//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Print Using Module
//  Handles PRINT USING string formatting for numeric and string values
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "print_using.h"
#include "error_handle.h"

#include "../../os.h"  // For int32, uint8 type definitions

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <inttypes.h>

// ============================================================================
// MODULE-PRIVATE STATE
// ============================================================================

static uint8_t pu_dig[1024]; // digits (left justified)
static int32_t pu_ndig;      // number of digits
static int32_t pu_dp;        // decimal place modifier
// note: if dp=0, the number is an integer and can be read as is
//      if dp=1 the number is itself*10
//      if dp=-1 the number is itself/10
static int32_t pu_neg;
static uint8_t pu_buf[1024];     // a buffer for preprocessing
static uint8_t pu_exp_char = 69; //"E"

// ============================================================================
// PRINT USING IMPLEMENTATION
// ============================================================================

int32 print_using(qbs *f, int32 s2, qbs *dest, qbs *pu_str) {
    // type: 1=numeric, 2=string
    if (is_error_pending())
        return 0;

    static int32 x, z, z2, z3, z4, ii;
    // x  - current format string read position
    // z - used as a temp variable for various calculations and loops
    // z2  - used for various calculations involving exponent digits
    // z3 - used as a temp variable for various calculations and loops
    // z4 - number of 0s between . and digits after point
    // ii  - used as a counter for writing the output
    static uint8 c;
    static int32 stage, len, chrsleft, type, s;
    static int32 leading_plus, dollar_sign, asterisk_spaces, digits_before_point, commas;
    static int32 decimal_point, digits_after_point, trailing_plus, exponent_digits, trailing_minus;
    static int32 cant_fit, extra_sign_space, rounded, digits_and_commas_before_point, leading_zero;
    static qbs *qbs1 = NULL;

    if (qbs1 == NULL)
        qbs1 = qbs_new(1, 0);

    if (pu_str)
        type = 2;
    else
        type = 1;

    s = s2;
    len = f->len;

scan:
    rounded = 0;
rounded_repass:

    x = s - 1; // subtract one to counter pre-increment later

    leading_plus = 0;
    dollar_sign = 0;
    asterisk_spaces = 0;
    digits_before_point = 0;
    commas = 0;
    decimal_point = 0;
    digits_after_point = 0;
    trailing_plus = 0;
    exponent_digits = 0;
    trailing_minus = 0;
    digits_and_commas_before_point = 0;
    leading_zero = 0;
    stage = 0;

nextchar:
    x++;
    if (x < len) {
        c = f->chr[x];
        chrsleft = len - x;

        if ((stage >= 2) && (stage <= 4)) {

            if (c == 43) { //+
                trailing_plus = 1;
                x++;
                goto numeric_spacer;
            }

            if (c == 45) { //-
                trailing_minus = 1;
                x++;
                goto numeric_spacer;
            }

        } // stage>=2 & stage<=4

        if ((stage >= 2) && (stage <= 3)) {

            if (chrsleft >= 5) {
                if ((c == 94) && (f->chr[x + 1] == 94) && (f->chr[x + 2] == 94) && (f->chr[x + 3] == 94) && (f->chr[x + 4] == 94)) { //^^^^^
                    exponent_digits = 3;
                    stage = 4;
                    x += 4;
                    goto nextchar;
                }
            } // 5

            if (chrsleft >= 4) {
                if ((c == 94) && (f->chr[x + 1] == 94) && (f->chr[x + 2] == 94) && (f->chr[x + 3] == 94)) { //^^^^
                    exponent_digits = 2;
                    stage = 4;
                    x += 3;
                    goto nextchar;
                }
            } // 4

        } // stage>=2 & stage<=3

        if (stage == 3) {

            if (c == 35) { // #
                digits_after_point++;
                goto nextchar;
            }

        } // stage==3

        if (stage == 2) {

            if (c == 44) { //,
                commas = 1;
                digits_before_point++;
                goto nextchar;
            }

        } // stage==2

        if (stage <= 2) {

            if (c == 35) { // #
                digits_before_point++;
                stage = 2;
                goto nextchar;
            }

            if (c == 46) { //.
                decimal_point = 1;
                stage = 3;
                goto nextchar;
            }

        } // stage<=2

        if (stage <= 1) {

            if (chrsleft >= 3) {
                if ((c == 42) && (f->chr[x + 1] == 42) && (f->chr[x + 2] == 36)) { //**$
                    asterisk_spaces = 1;
                    digits_before_point = 2;
                    dollar_sign = 1;
                    stage = 2;
                    x += 2;
                    goto nextchar;
                }
            } // 3

            if (chrsleft >= 2) {
                if ((c == 42) && (f->chr[x + 1] == 42)) { //**
                    asterisk_spaces = 1;
                    digits_before_point = 2;
                    stage = 2;
                    x++;
                    goto nextchar;
                }
                if ((c == 36) && (f->chr[x + 1] == 36)) { //$$
                    dollar_sign = 1;
                    digits_before_point = 1;
                    stage = 2;
                    x++;
                    goto nextchar;
                }
            } // 2

        } // stage 1

        if (stage == 0) {

            if (c == 43) { //+
                leading_plus = 1;
                stage = 1;
                goto nextchar;
            }

        } // stage 0

        // spacer/end encountered
    } // x<len
numeric_spacer:

    // valid numeric format?
    if (stage <= 1)
        goto invalid_numeric_format;
    if ((digits_before_point == 0) && (digits_after_point == 0))
        goto invalid_numeric_format;

    if (type == 0)
        return s;    // s is the beginning of a new format but item has already been added to dest
    if (type == 2) { // expected string format, not numeric format
        error(13);   // type mismatch
        return 0;
    }

    // reduce digits before point appropriately
    extra_sign_space = 0;
    if (exponent_digits) {
        if ((leading_plus == 0) && (trailing_plus == 0) && (trailing_minus == 0)) {
            digits_before_point--;
            if (digits_before_point == -1) {
                digits_after_point--;
                digits_before_point = 0;
                if (digits_after_point == 0) {
                    decimal_point = 0;
                    digits_before_point++;
                }
            }
            extra_sign_space = 1;
        }
    } else {
        // the following doesn't occur if using an exponent
        if (pu_neg) {
            if ((leading_plus == 0) && (trailing_plus == 0) && (trailing_minus == 0)) {
                digits_before_point--;
                extra_sign_space = 1;
            }
        }
        if (commas) {
            digits_and_commas_before_point = digits_before_point;
            ii = digits_before_point / 4; // for every 4 digits, one digit will be used up by a comma
            digits_before_point -= ii;
        }
    }

    //'0'->'.0' exception (for when format doesn't allow for any digits_before_point)
    if (digits_before_point == 0) { // no digits allowed before decimal point
        // note: pu_ndig=256, pu_dp=-255
        if ((pu_ndig + pu_dp) == 1) { // 1 digit exists in front of the decimal point
            if (pu_dig[0] == 48) {    // is it 0?
                pu_dp--;              // moves decimal point left one position
            } // 0
        }
    }

    // will number fit? if it can't then adjustments will be made
    cant_fit = 0;
    if (exponent_digits) {
        // give back extra_sign_space?
        if (extra_sign_space) {
            if (!pu_neg) {
                if (digits_before_point <= 0) {
                    extra_sign_space = 0;
                    digits_before_point++; // will become 0 or 1
                    // force 0 in recovered digit?
                    if ((digits_before_point == 1) && (digits_after_point > 0)) {
                        digits_before_point--;
                        extra_sign_space = 2; // 2=put 0 instead of blank space
                    }
                }
            }
        }
        if ((digits_before_point == 0) && (digits_after_point == 0)) {
            cant_fit = 1;
            digits_before_point = 1; // give back removed (for extra sign space) digit
        }
        // but does the exponent fit?
        z2 = pu_ndig + pu_dp - 1; // calc exponent of most significant digit
        // 1.0  = 0
        // 10.0 = 1
        // 0.1  = -1
        // calc exponent of format's most significant position
        if (digits_before_point)
            z3 = digits_before_point - 1;
        else
            z3 = -1;
        z = z2 - z3; // combine to calculate actual exponent which will be "printed"
        z3 = abs(z);
        z2 = snprintf((char *)pu_buf, sizeof(pu_buf), "%u", z3); // use pu_buf to convert exponent to a string
        if (z2 > exponent_digits) {
            cant_fit = 1;
            exponent_digits = z2;
        }
    } else {
        z2 = 0;
        z = pu_ndig + pu_dp; // calc number of digits required before decimal places
        if (digits_before_point < z) {
            digits_before_point = z;
            cant_fit = 1;
            if (commas)
                digits_and_commas_before_point = digits_before_point + (digits_before_point - 1) / 3;
        }
    }

    static int32 buf_size; // buf_size is an estimation of size required
    static uint8 *cp, *buf = NULL;
    static int32 count;
    if (buf)
        free(buf);
    buf_size = 256; // 256 bytes to account for calc overflow (such as exponent digits)
    buf_size += 9;  //%(1)+-(1)$(1)???.(1)???exponent(5)
    buf_size += digits_before_point;
    if (commas)
        buf_size += ((digits_before_point / 3) + 2);
    buf_size += digits_after_point;
    buf = (uint8 *)malloc(buf_size);
    cp = buf;
    count = 0; // char count
    ii = 0;

    if (asterisk_spaces)
        asterisk_spaces = 42;
    else
        asterisk_spaces = 32; // character to fill blanks with

    if (cant_fit) {
        *cp++ = 37;
        count++;
    } //%

    // leading +/-
    if (leading_plus) {
        if (pu_neg)
            *cp++ = 45;
        else
            *cp++ = 43;
        count++;
    }

    if (exponent_digits) {
        z4 = 0;
        // add $?
        if (dollar_sign) {
            *cp++ = 36;
            count++;
        } //$
        // add - sign? (as sign space was not specified)
        if (extra_sign_space) {
            if (pu_neg) {
                *cp++ = 45;
            } else {
                if (extra_sign_space == 2)
                    *cp++ = 48;
                else
                    *cp++ = 32;
            }
            count++;
        }
        // add digits left of decimal point
        for (z3 = 0; z3 < digits_before_point; z3++) {
            if (ii < pu_ndig)
                *cp++ = pu_dig[ii++];
            else
                *cp++ = 48;
            count++;
        }
        // add decimal point
        if (decimal_point) {
            *cp++ = 46;
            count++;
        }
        // add digits right of decimal point
        for (z3 = 0; z3 < digits_after_point; z3++) {
            if (ii < pu_ndig)
                *cp++ = pu_dig[ii++];
            else
                *cp++ = 48;
            count++;
        }
        // round last digit (requires a repass)
        if (!rounded) {
            if (ii < pu_ndig) {
                if (pu_dig[ii] >= 53) { //>="5" (round 5 up)
                    z = ii - 1;
                    // round up pu (by adding 1 from digit at character position z)
                    // note: pu_dig is rebuilt one character to the right so highest digit can flow over into next character
                    rounded = 1;
                    memmove(&pu_dig[1], &pu_dig[0], pu_ndig);
                    pu_dig[0] = 48;
                    z++;
                puround2:
                    pu_dig[z]++;
                    if (pu_dig[z] > 57) {
                        pu_dig[z] = 48;
                        z--;
                        goto puround2;
                    }
                    if (pu_dig[0] != 48) { // was extra character position necessary?
                        pu_ndig++;         // note: pu_dp does not require any changes
                    } else {
                        memmove(&pu_dig[0], &pu_dig[1], pu_ndig);
                    }
                    goto rounded_repass;
                }
            }
        }
        // add exponent...
        *cp++ = pu_exp_char;
        count++; // add exponent D/E/F (set and restored by calling function as necessary)
        if (z >= 0) {
            *cp++ = 43;
            count++;
        } else {
            *cp++ = 45;
            count++;
        } //+/- exponent's sign
        // add exponent's leading 0s (if any)
        for (z3 = 0; z3 < (exponent_digits - z2); z3++) {
            *cp++ = 48;
            count++;
        }
        // add exponent's value
        for (z3 = 0; z3 < z2; z3++) {
            *cp++ = pu_buf[z3];
            count++;
        }
    } else {
        //"print" everything before the point
        // calculate digit spaces before the point in number
        if (!commas)
            digits_and_commas_before_point = digits_before_point;
        z = pu_ndig + pu_dp; // num of character whole portion of number requires
        z4 = 0;
        if (z < 0)
            z4 = -z; // number of 0s between . and digits after point
        if (commas)
            z = z + (z - 1) / 3; // including appropriate amount of commas
        if (z < 0)
            z = 0;
        z2 = digits_and_commas_before_point - z;
        if ((z == 0) && (z2 > 0)) {
            leading_zero = 1;
            z2--;
        } // change .1 to 0.1 if possible
        for (z3 = 0; z3 < z2; z3++) {
            *cp++ = asterisk_spaces;
            count++;
        }
        // add - sign? (as sign space was not specified)
        if (extra_sign_space) {
            *cp++ = 45;
            count++;
        }
        // add $?
        if (dollar_sign) {
            *cp++ = 36;
            count++;
        } //$
        // leading 0?
        if (leading_zero) {
            *cp++ = 48;
            count++;
        } // 0
        // add digits left of decimal point
        for (z3 = 0; z3 < z; z3++) {
            if ((commas != 0) && (((z - z3) & 3) == 0)) {
                *cp++ = 44;
            } else {
                if (ii < pu_ndig)
                    *cp++ = pu_dig[ii++];
                else
                    *cp++ = 48;
            }
            count++;
        }
        // add decimal point
        if (decimal_point) {
            *cp++ = 46;
            count++;
        }
        // add digits right of decimal point
        for (z3 = 0; z3 < digits_after_point; z3++) {
            if (z4) {
                z4--;
                *cp++ = 48;
            } else {
                if (ii < pu_ndig)
                    *cp++ = pu_dig[ii++];
                else
                    *cp++ = 48;
            }
            count++;
        }
        // round last digit (requires a repass)
        if (!rounded) {
            if (ii < pu_ndig) {
                if (pu_dig[ii] >= 53) { //>="5" (round 5 up)
                    z = ii - 1;
                    // round up pu (by adding 1 from digit at character position z)
                    // note: pu_dig is rebuilt one character to the right so highest digit can flow over into next character
                    rounded = 1;
                    memmove(&pu_dig[1], &pu_dig[0], pu_ndig);
                    pu_dig[0] = 48;
                    z++;
                puround1:
                    pu_dig[z]++;
                    if (pu_dig[z] > 57) {
                        pu_dig[z] = 48;
                        z--;
                        goto puround1;
                    }
                    if (pu_dig[0] != 48) { // was extra character position necessary?
                        pu_ndig++;         // note: pu_dp does not require any changes
                    } else {
                        memmove(&pu_dig[0], &pu_dig[1], pu_ndig);
                    }
                    goto rounded_repass;
                }
            }
        }
    } // exponent_digits

    // add trailing sign?
    // trailing +/-
    if (trailing_plus) {
        if (pu_neg)
            *cp++ = 45;
        else
            *cp++ = 43;
        count++;
    }
    // trailing -
    if (trailing_minus) {
        if (pu_neg)
            *cp++ = 45;
        else
            *cp++ = 32;
        count++;
    }

    qbs_set(dest, qbs_add(dest, qbs_new_txt_len((char *)buf, count)));

    s = x;
    type = 0; // passed type added
    if (s >= len)
        return 0; // end of format line encountered and passed item added
    goto scan;

invalid_numeric_format:
    // string format
    static int32 string_size;

    x = s;
    if (x < len) {
        c = f->chr[x];
        string_size = 0; // invalid
        if (c == 38)
            string_size = -1; //"&" (all of string)
        if (c == 33)
            string_size = 1; //"!" (first character only)
        if (c == 92) {       //"\" first n characters
            z = 1;
            x++;
        get_str_fmt:
            if (x >= len)
                goto invalid_string_format;
            c = f->chr[x];
            z++;
            if (c == 32) {
                x++;
                goto get_str_fmt;
            }
            if (c != 92)
                goto invalid_string_format;
            string_size = z;
        } // c==47
        if (string_size) {
            if (type == 0)
                return s;    // s is the beginning of a new format but item has already been added to dest
            if (type == 1) { // expected numeric format, not string format
                error(13);   // type mismatch
                return 0;
            }
            if (string_size != -1) {
                s += string_size;
                for (z = 0; z < string_size; z++) {
                    if (z < pu_str->len)
                        qbs1->chr[0] = pu_str->chr[z];
                    else
                        qbs1->chr[0] = 32;
                    qbs_set(dest, qbs_add(dest, qbs1));
                } // z
            } else {
                qbs_set(dest, qbs_add(dest, pu_str));
                s++;
            }
            type = 0; // passed type added
            if (s >= len)
                return 0; // end of format line encountered and passed item added
            goto scan;
        } // string_size
    } // x<len
invalid_string_format:

    // add literal?
    if ((f->chr[s] == 95) && (s < (len - 1))) { // trailing single _ in format is treated as a literal _
        s++;
    }
    // add non-format character
    qbs1->chr[0] = f->chr[s];
    qbs_set(dest, qbs_add(dest, qbs1));

    s++;
    if (s >= len) {
        s = 0;
        if (type == 0)
            return s; // end of format line encountered and passed item added
        // illegal format? (format has been passed from start (s2=0) to end and has no numeric/string identifiers
        if (s2 == 0) {
            error(5); // illegal function call
            return 0;
        }
    }
    goto scan;

    return 0;
}

int32 print_using_integer64(qbs *format, int64 value, int32 start, qbs *output) {
    if (is_error_pending())
        return 0;
    pu_ndig = snprintf((char *)pu_buf, sizeof(pu_buf), "% " PRId64, value);
    if (pu_buf[0] == 45)
        pu_neg = 1;
    else
        pu_neg = 0;
    pu_ndig--; // remove sign
    memcpy(pu_dig, &pu_buf[1], pu_ndig);
    pu_dp = 0;
    start = print_using(format, start, output, NULL);
    return start;
}

int32 print_using_uinteger64(qbs *format, uint64 value, int32 start, qbs *output) {
    if (is_error_pending())
        return 0;
    pu_ndig = snprintf((char *)pu_dig, sizeof(pu_dig), "%" PRIu64, value);
    pu_neg = 0;
    pu_dp = 0;
    start = print_using(format, start, output, NULL);
    return start;
}

int32 print_using_single(qbs *format, float value, int32 start, qbs *output) {
    if (is_error_pending())
        return 0;
    static int32 i, len, neg_exp;
    static uint8 c;
    static int64 exp;
    len = snprintf((char *)&pu_buf, sizeof(pu_buf), "% .255E", value); // 256 character limit ([1].[255])
    pu_dp = 0;
    pu_ndig = 0;
    // 1. sign
    if (pu_buf[0] == 45)
        pu_neg = 1;
    else
        pu_neg = 0;
    i = 1;
// 2. digits before decimal place
getdigits:
    if (i >= len) {
        error(5);
        return 0;
    }
    c = pu_buf[i];
    if ((c >= 48) && (c <= 57)) {
        pu_dig[pu_ndig++] = c;
        i++;
        goto getdigits;
    }
    // 3. decimal place
    if (c != 46) {
        error(5);
        return 0;
    } // expected decimal point
    i++;
// 4. digits after decimal place
getdigits2:
    if (i >= len) {
        error(5);
        return 0;
    }
    c = pu_buf[i];
    if ((c >= 48) && (c <= 57)) {
        pu_dig[pu_ndig++] = c;
        pu_dp--;
        i++;
        goto getdigits2;
    }
    // assume random character signifying an exponent
    i++;
    // optional exponent sign
    neg_exp = 0;
    if (i >= len) {
        error(5);
        return 0;
    }
    c = pu_buf[i];
    if (c == 45) {
        neg_exp = 1;
        i++;
    } //-
    if (c == 43)
        i++; //+
    // assume remaining characters are an exponent
    exp = 0;
getdigits3:
    if (i < len) {
        c = pu_buf[i];
        if ((c < 48) || (c > 57)) {
            error(5);
            return 0;
        }
        exp = exp * 10;
        exp = exp + c - 48;
        i++;
        goto getdigits3;
    }
    if (neg_exp)
        exp = -exp;
    pu_dp += exp;
    start = print_using(format, start, output, NULL);
    return start;
}

int32 print_using_double(qbs *format, double value, int32 start, qbs *output) {
    if (is_error_pending())
        return 0;
    static int32 i, len, neg_exp;
    static uint8 c;
    static int64 exp;
    len = snprintf((char *)&pu_buf, sizeof(pu_buf), "% .255E", value); // 256 character limit ([1].[255])
    pu_dp = 0;
    pu_ndig = 0;
    // 1. sign
    if (pu_buf[0] == 45)
        pu_neg = 1;
    else
        pu_neg = 0;
    i = 1;
// 2. digits before decimal place
getdigits:
    if (i >= len) {
        error(5);
        return 0;
    }
    c = pu_buf[i];
    if ((c >= 48) && (c <= 57)) {
        pu_dig[pu_ndig++] = c;
        i++;
        goto getdigits;
    }
    // 3. decimal place
    if (c != 46) {
        error(5);
        return 0;
    } // expected decimal point
    i++;
// 4. digits after decimal place
getdigits2:
    if (i >= len) {
        error(5);
        return 0;
    }
    c = pu_buf[i];
    if ((c >= 48) && (c <= 57)) {
        pu_dig[pu_ndig++] = c;
        pu_dp--;
        i++;
        goto getdigits2;
    }
    // assume random character signifying an exponent
    i++;
    // optional exponent sign
    neg_exp = 0;
    if (i >= len) {
        error(5);
        return 0;
    }
    c = pu_buf[i];
    if (c == 45) {
        neg_exp = 1;
        i++;
    } //-
    if (c == 43)
        i++; //+
    // assume remaining characters are an exponent
    exp = 0;
getdigits3:
    if (i < len) {
        c = pu_buf[i];
        if ((c < 48) || (c > 57)) {
            error(5);
            return 0;
        }
        exp = exp * 10;
        exp = exp + c - 48;
        i++;
        goto getdigits3;
    }
    if (neg_exp)
        exp = -exp;
    pu_dp += exp;
    pu_exp_char = 68; //"D"
    start = print_using(format, start, output, NULL);
    pu_exp_char = 69; //"E"
    return start;
}

// WARNING: DUE TO MINGW NOT SUPPORTING PRINTF long double, VALUES ARE CONVERTED TO A DOUBLE
//         BUT PRINTED AS IF THEY WERE A long double
int32 print_using_float(qbs *format, long double value, int32 start, qbs *output) {
    if (is_error_pending())
        return 0;
    static int32 i, len, neg_exp;
    static uint8 c;
    static int64 exp;
// len=sprintf((char*)&pu_buf,"% .255E",value);//256 character limit ([1].[255])
#ifdef QB64_MINGW
    len = __mingw_snprintf((char *)&pu_buf, sizeof(pu_buf), "% .255Lf", value); // 256 character limit ([1].[255])
#else
    len = snprintf((char *)&pu_buf, sizeof(pu_buf), "% .255Lf", value); // 256 character limit ([1].[255])
#endif

    // qbs_print(qbs_new_txt((char*)&pu_buf),1);

    pu_dp = 0;
    pu_ndig = 0;
    // 1. sign
    if (pu_buf[0] == 45)
        pu_neg = 1;
    else
        pu_neg = 0;
    i = 1;
// 2. digits before decimal place
getdigits:
    if (i >= len) {
        error(5);
        return 0;
    }
    c = pu_buf[i];
    if ((c >= 48) && (c <= 57)) {
        pu_dig[pu_ndig++] = c;
        i++;
        goto getdigits;
    }
    // 3. decimal place
    if (c != 46) {
        error(5);
        return 0;
    } // expected decimal point
    i++;
// 4. digits after decimal place
getdigits2:
    if (i >= len) {
        // no exponent information has been provided
        neg_exp = 0;
        exp = 0;
        goto no_exponent_provided;
        // error(5); return 0;
    }
    c = pu_buf[i];
    if ((c >= 48) && (c <= 57)) {
        pu_dig[pu_ndig++] = c;
        pu_dp--;
        i++;
        goto getdigits2;
    }
    // assume random character signifying an exponent
    i++;
    // optional exponent sign
    neg_exp = 0;
    if (i >= len) {
        error(5);
        return 0;
    }
    c = pu_buf[i];
    if (c == 45) {
        neg_exp = 1;
        i++;
    } //-
    if (c == 43)
        i++; //+
    // assume remaining characters are an exponent
    exp = 0;
getdigits3:
    if (i < len) {
        c = pu_buf[i];
        if ((c < 48) || (c > 57)) {
            error(5);
            return 0;
        }
        exp = exp * 10;
        exp = exp + c - 48;
        i++;
        goto getdigits3;
    }
    if (neg_exp)
        exp = -exp;
    pu_dp += exp;
no_exponent_provided:
    pu_exp_char = 70; //"F"
    start = print_using(format, start, output, NULL);
    pu_exp_char = 69; //"E"
    return start;
}
