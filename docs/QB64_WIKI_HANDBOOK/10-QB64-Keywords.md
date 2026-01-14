# QB64-Specific Keywords

This chapter contains all QB64-specific keywords that begin with an underscore (_). These keywords are not available in traditional QBasic and provide modern functionality for enhanced programming capabilities.

## Important Note

**All QB64 specific keywords must use the underscore prefix as listed below.** This distinguishes them from traditional QBasic keywords and ensures compatibility.

---

## _A (QB64)

### _ACCEPTFILEDROP (statement)
**Purpose**: Turns a program window into a valid drop destination for dragging files from Windows Explorer.

**Syntax**:
```basic
_ACCEPTFILEDROP
```

**Example**:
```basic
_ACCEPTFILEDROP
PRINT "Drag files onto this window..."
DO
    _LIMIT 30
LOOP
```

### _ACOS (function)
**Purpose**: Arccosine function returns the angle in radians based on an input COSine value range from -1 to 1.

**Syntax**:
```basic
result! = _ACOS(numeric_value)
```

**Parameters**:
- `numeric_value`: A value between -1 and 1

**Returns**: Angle in radians

**Example**:
```basic
angle! = _ACOS(0.5)  ' Returns approximately 1.0472 radians (60 degrees)
PRINT "Arccosine of 0.5 ="; angle!
```

### _ACOSH (function)
**Purpose**: Returns the nonnegative arc hyperbolic cosine of x, expressed in radians.

**Syntax**:
```basic
result! = _ACOSH(numeric_value)
```

**Parameters**:
- `numeric_value`: Value ≥ 1

**Returns**: Arc hyperbolic cosine in radians

### _ADLER32 (function)
**Purpose**: Returns the Adler-32 checksum of any arbitrary string.

**Syntax**:
```basic
checksum& = _ADLER32(string_expression)
```

**Example**:
```basic
checksum& = _ADLER32("Hello World")
PRINT "Adler-32 checksum:"; checksum&
```

### _ALLOWFULLSCREEN (statement)
**Purpose**: Allows setting the behavior of the ALT+ENTER combo.

**Syntax**:
```basic
_ALLOWFULLSCREEN {ON|OFF|_SMOOTH}
```

**Options**:
- `ON`: Enable fullscreen toggle
- `OFF`: Disable fullscreen toggle  
- `_SMOOTH`: Enable with smoothing

### _ALPHA (function)
**Purpose**: Returns the alpha channel transparency level of a color value used on a screen page or image.

**Syntax**:
```basic
alpha% = _ALPHA(color_value)
```

**Returns**: Alpha channel value (0-255)

### _ALPHA32 (function)
**Purpose**: Returns the alpha channel transparency level of a color value used on a 32-bit screen page or image.

**Syntax**:
```basic
alpha% = _ALPHA32(color_value)
```

**Returns**: Alpha channel value (0-255)

### _ANDALSO (logical operator)
**Purpose**: Performs short-circuiting logical conjunction on two expressions.

**Syntax**:
```basic
result = expression1 _ANDALSO expression2
```

**Note**: Short-circuits - if expression1 is false, expression2 is not evaluated

### _ARCCOT (function)
**Purpose**: Is the inverse function of the cotangent.

**Syntax**:
```basic
result! = _ARCCOT(numeric_value)
```

### _ARCCSC (function)
**Purpose**: Is the inverse function of the cosecant.

**Syntax**:
```basic
result! = _ARCCSC(numeric_value)
```

### _ARCSEC (function)
**Purpose**: Is the inverse function of the secant.

**Syntax**:
```basic
result! = _ARCSEC(numeric_value)
```

### _ASIN (function)
**Purpose**: Returns the principal value of the arc sine of x, expressed in radians.

**Syntax**:
```basic
result! = _ASIN(numeric_value)
```

**Parameters**:
- `numeric_value`: Value between -1 and 1

### _ASINH (function)
**Purpose**: Returns the arc hyperbolic sine of x, expressed in radians.

**Syntax**:
```basic
result! = _ASINH(numeric_value)
```

### _ASSERT (statement)
**Purpose**: Performs debug tests.

**Syntax**:
```basic
_ASSERT condition
```

**Note**: Only active when $ASSERTS metacommand is enabled

### _ATAN2 (function)
**Purpose**: Returns the principal value of the arc tangent of y/x, expressed in radians.

**Syntax**:
```basic
result! = _ATAN2(y_value, x_value)
```

**Returns**: Angle in radians (-π to π)

### _ATANH (function)
**Purpose**: Returns the arc hyperbolic tangent of x, expressed in radians.

**Syntax**:
```basic
result! = _ATANH(numeric_value)
```

### _AUTODISPLAY (statement)
**Purpose**: Enables the automatic display of the screen image changes previously disabled by _DISPLAY.

**Syntax**:
```basic
_AUTODISPLAY
```

### _AUTODISPLAY (function)
**Purpose**: Returns the current display mode as true (-1) if automatic or false (0) if per request using _DISPLAY.

**Syntax**:
```basic
mode% = _AUTODISPLAY
```

**Returns**: -1 if automatic, 0 if manual

### _AXIS (function)
**Purpose**: Returns a SINGLE value between -1 and 1 indicating the maximum distance from the device axis center, 0.

**Syntax**:
```basic
value! = _AXIS(device_number, axis_number)
```

**Parameters**:
- `device_number`: Input device number
- `axis_number`: Axis number to read

---

## _B (QB64)

### _BACKGROUNDCOLOR (function)
**Purpose**: Returns the current background color for an image handle or page.

**Syntax**:
```basic
color& = _BACKGROUNDCOLOR(image_handle)
```

### _BASE64DECODE$ (function)
**Purpose**: Decodes a Base64-encoded string.

**Syntax**:
```basic
decoded$ = _BASE64DECODE$(encoded_string)
```

### _BASE64ENCODE$ (function)
**Purpose**: Encodes a string into Base64 format.

**Syntax**:
```basic
encoded$ = _BASE64ENCODE$(string_expression)
```

### _BIT (numerical type)
**Purpose**: Can return only signed values of 0 (bit off) and -1 (bit on). Unsigned 0 or 1.

**Syntax**:
```basic
DIM variable AS _BIT
```

**Range**: -128 to 127 (signed), 0 to 255 (unsigned)

### _BIN$ (function)
**Purpose**: Returns the binary (base 2) STRING representation of the INTEGER part of any value.

**Syntax**:
```basic
binary$ = _BIN$(numeric_value)
```

**Example**:
```basic
PRINT _BIN$(10)  ' Outputs: 1010
```

### _BLEND (statement)
**Purpose**: Statement turns on 32-bit alpha blending for the current image or screen mode and is default.

**Syntax**:
```basic
_BLEND
```

### _BLEND (function)
**Purpose**: Returns -1 if enabled or 0 if disabled by _DONTBLEND statement.

**Syntax**:
```basic
status% = _BLEND
```

### _BLINK (statement)
**Purpose**: Statement turns blinking colors on/off in SCREEN 0.

**Syntax**:
```basic
_BLINK {ON|OFF}
```

### _BLINK (function)
**Purpose**: Returns -1 if enabled or 0 if disabled by _BLINK statement.

**Syntax**:
```basic
status% = _BLINK
```

### _BLUE (function)
**Purpose**: Function returns the palette or the blue component intensity of a 32-bit image color.

**Syntax**:
```basic
blue% = _BLUE(color_value)
```

### _BLUE32 (function)
**Purpose**: Returns the blue component intensity of a 32-bit color value.

**Syntax**:
```basic
blue% = _BLUE32(color_value)
```

### _BRIGHTNESS32 (function)
**Purpose**: Returns the brightness value (HSB colorspace) of a given 32-bit ARGB color.

**Syntax**:
```basic
brightness% = _BRIGHTNESS32(color_value)
```

### _BUTTON (function)
**Purpose**: Returns -1 when a controller device button is pressed and 0 when button is released.

**Syntax**:
```basic
status% = _BUTTON(device_number, button_number)
```

### _BUTTONCHANGE (function)
**Purpose**: Returns -1 when a device button has been pressed and 1 when released. Zero indicates no change.

**Syntax**:
```basic
change% = _BUTTONCHANGE(device_number, button_number)
```

### _BYTE (numerical type)
**Purpose**: Can hold signed values from -128 to 127 (one byte or _BIT * 8). Unsigned from 0 to 255.

**Syntax**:
```basic
DIM variable AS _BYTE
```

**Range**: -128 to 127 (signed), 0 to 255 (unsigned)

---

## _C (QB64)

### _CAPSLOCK (function)
**Purpose**: Returns -1 when Caps Lock is on.

**Syntax**:
```basic
status% = _CAPSLOCK
```

### _CAPSLOCK (statement)
**Purpose**: Sets Caps Lock key state.

**Syntax**:
```basic
_CAPSLOCK {ON|OFF}
```

### _CAST (function)
**Purpose**: Performs a C-like cast of a numerical value to a specified numerical type.

**Syntax**:
```basic
result = _CAST(numeric_value AS target_type)
```

**Example**:
```basic
result% = _CAST(3.14 AS INTEGER)  ' Returns 3
```

### _CEIL (function)
**Purpose**: Rounds x upward, returning the smallest integral value that is not less than x.

**Syntax**:
```basic
result = _CEIL(numeric_value)
```

**Example**:
```basic
PRINT _CEIL(3.14)  ' Returns 4
PRINT _CEIL(-2.7)  ' Returns -2
```

### _CINP (function)
**Purpose**: Returns a key code from $CONSOLE input.

**Syntax**:
```basic
key_code% = _CINP
```

### _CLAMP (function)
**Purpose**: Forces the given numeric value into a specific range.

**Syntax**:
```basic
result = _CLAMP(value, minimum, maximum)
```

**Example**:
```basic
result% = _CLAMP(150, 0, 100)  ' Returns 100
```

### _CLEARCOLOR (function)
**Purpose**: Returns the current transparent color of an image.

**Syntax**:
```basic
color& = _CLEARCOLOR(image_handle)
```

### _CLEARCOLOR (statement)
**Purpose**: Sets a specific color index of an image to be transparent.

**Syntax**:
```basic
_CLEARCOLOR image_handle, color_value
```

### _CLIP (PUT graphics option)
**Purpose**: Allows placement of an image partially off of the screen.

**Syntax**:
```basic
PUT (x, y), image_handle, _CLIP
```

### _CLIPBOARD$ (function)
**Purpose**: Returns the operating system's clipboard contents as a STRING.

**Syntax**:
```basic
clipboard$ = _CLIPBOARD$
```

### _CLIPBOARD$ (statement)
**Purpose**: Sets and overwrites the STRING value in the operating system's clipboard.

**Syntax**:
```basic
_CLIPBOARD$ = string_expression
```

### _CLIPBOARDIMAGE (function)
**Purpose**: Pastes an image from the clipboard into a new QB64 image in memory.

**Syntax**:
```basic
image_handle = _CLIPBOARDIMAGE
```

### _CLIPBOARDIMAGE (statement)
**Purpose**: Copies a valid QB64 image to the clipboard.

**Syntax**:
```basic
_CLIPBOARDIMAGE image_handle
```

### _COLORCHOOSERDIALOG (function)
**Purpose**: Displays a standard color picker dialog box and returns a 32-bit RGBA color selected by the user.

**Syntax**:
```basic
color& = _COLORCHOOSERDIALOG
```

### _COMMANDCOUNT (function)
**Purpose**: Returns the number of arguments passed to the compiled program from the command line.

**Syntax**:
```basic
count% = _COMMANDCOUNT
```

### _CONNECTED (function)
**Purpose**: Returns the status of a TCP/IP connection handle.

**Syntax**:
```basic
status% = _CONNECTED(connection_handle)
```

**Returns**: -1 if connected, 0 if not connected

### _CONNECTIONADDRESS$ (TCP/IP function)
**Purpose**: Returns a connected user's STRING IP address value using the handle.

**Syntax**:
```basic
ip_address$ = _CONNECTIONADDRESS$(connection_handle)
```

### _CONSOLE (statement)
**Purpose**: Used to turn a console window OFF or ON or to designate _DEST _CONSOLE for output.

**Syntax**:
```basic
_CONSOLE {ON|OFF}
_DEST _CONSOLE
```

### _CONSOLEINPUT (function)
**Purpose**: Fetches input data from a $CONSOLE window to be read later (both mouse and keyboard).

**Syntax**:
```basic
input_data$ = _CONSOLEINPUT
```

### _CONSOLECURSOR (statement)
**Purpose**: Is used to switch the text cursor in Console Windows off/on or change its size.

**Syntax**:
```basic
_CONSOLECURSOR {ON|OFF|size%}
```

### _CONSOLEFONT (statement)
**Purpose**: Is used to change the text font used in Console Windows or change its size.

**Syntax**:
```basic
_CONSOLEFONT font_name$, size%
```

### _CONSOLETITLE (statement)
**Purpose**: Creates the title of the console window using a literal or variable STRING string.

**Syntax**:
```basic
_CONSOLETITLE title_string$
```

### _CONTINUE (statement)
**Purpose**: Skips the remaining lines in a control block (DO/LOOP, FOR/NEXT or WHILE/WEND).

**Syntax**:
```basic
_CONTINUE
```

**Example**:
```basic
FOR i = 1 TO 10
    IF i = 5 THEN _CONTINUE  ' Skip when i = 5
    PRINT i
NEXT i
```

### _CONTROLCHR (statement)
**Purpose**: OFF allows the control characters to be used as text characters. ON (default) can use them as commands.

**Syntax**:
```basic
_CONTROLCHR {ON|OFF}
```

### _CONTROLCHR (function)
**Purpose**: Returns the current state of _CONTROLCHR as 1 when OFF and 0 when ON.

**Syntax**:
```basic
state% = _CONTROLCHR
```

### _COPYIMAGE (function)
**Purpose**: Copies an image handle value to a new designated handle.

**Syntax**:
```basic
new_handle = _COPYIMAGE(source_handle)
```

### _COPYPALETTE (statement)
**Purpose**: Copies the color palette intensities from one 4 or 8 BPP image to another image.

**Syntax**:
```basic
_COPYPALETTE source_handle, destination_handle
```

### _COT (function)
**Purpose**: The mathematical function cotangent defined by 1/TAN.

**Syntax**:
```basic
result! = _COT(angle_in_radians)
```

### _COTH (function)
**Purpose**: Returns the hyperbolic cotangent.

**Syntax**:
```basic
result! = _COTH(numeric_value)
```

### _COSH (function)
**Purpose**: Returns the hyperbolic cosine of x radians.

**Syntax**:
```basic
result! = _COSH(numeric_value)
```

### _CRC32 (function)
**Purpose**: Returns the Crc-32 checksum of any arbitrary string.

**Syntax**:
```basic
checksum& = _CRC32(string_expression)
```

### _CSC (function)
**Purpose**: The mathematical function cosecant defined by 1/SIN.

**Syntax**:
```basic
result! = _CSC(angle_in_radians)
```

### _CSCH (function)
**Purpose**: Returns the hyperbolic cosecant.

**Syntax**:
```basic
result! = _CSCH(numeric_value)
```

### _CV (function)
**Purpose**: Converts any _MK$ STRING value to the designated numerical type value.

**Syntax**:
```basic
result = _CV(string_value AS target_type)
```

### _CWD$ (function)
**Purpose**: Returns the current working directory as a STRING value.

**Syntax**:
```basic
directory$ = _CWD$
```

---

## _D (QB64)

### _D2G (function)
**Purpose**: Converts degrees to gradient angle values.

**Syntax**:
```basic
gradient! = _D2G(degrees)
```

### _D2R (function)
**Purpose**: Converts degrees to radian angle values.

**Syntax**:
```basic
radians! = _D2R(degrees)
```

### _DECODEURL$ (function)
**Purpose**: Returns the decoded plain text URL of the given encoded URL.

**Syntax**:
```basic
decoded$ = _DECODEURL$(encoded_url)
```

### _DEFAULTCOLOR (function)
**Purpose**: Returns the current default (text/drawing) color for an image handle or page.

**Syntax**:
```basic
color& = _DEFAULTCOLOR(image_handle)
```

### _DEFINE (statement)
**Purpose**: Defines a range of variable names according to their first character as a data type.

**Syntax**:
```basic
_DEFINE start_letter-end_letter AS data_type
```

**Example**:
```basic
_DEFINE A-Z AS INTEGER  ' All variables default to INTEGER
```

### _DEFLATE$ (function)
**Purpose**: Compresses a string.

**Syntax**:
```basic
compressed$ = _DEFLATE$(string_expression)
```

### _DELAY (statement)
**Purpose**: Suspends program execution for a SINGLE number of seconds.

**Syntax**:
```basic
_DELAY seconds!
```

**Example**:
```basic
_DELAY 2.5  ' Wait 2.5 seconds
```

### _DEPTHBUFFER (statement)
**Purpose**: Enables, disables, locks or clears depth buffering.

**Syntax**:
```basic
_DEPTHBUFFER {ON|OFF|LOCK|CLEAR}
```

### _DESKTOPHEIGHT (function)
**Purpose**: Returns the height of the desktop (not program window).

**Syntax**:
```basic
height% = _DESKTOPHEIGHT
```

### _DESKTOPWIDTH (function)
**Purpose**: Returns the width of the desktop (not program window).

**Syntax**:
```basic
width% = _DESKTOPWIDTH
```

### _DEST (statement)
**Purpose**: Sets the current write image or SCREEN page destination for prints or graphics.

**Syntax**:
```basic
_DEST image_handle_or_page_number
```

### _DEST (function)
**Purpose**: Returns the current destination screen page or image handle value.

**Syntax**:
```basic
current_dest = _DEST
```

### _DEVICE$ (function)
**Purpose**: Returns a STRING expression listing a designated numbered input device name and types of input.

**Syntax**:
```basic
device_info$ = _DEVICE$(device_number)
```

### _DEVICEINPUT (function)
**Purpose**: Returns the _DEVICES number of an _AXIS, _BUTTON or _WHEEL event.

**Syntax**:
```basic
device_num% = _DEVICEINPUT
```

### _DEVICES (function)
**Purpose**: Returns the number of input devices found on a computer system including the keyboard and mouse.

**Syntax**:
```basic
device_count% = _DEVICES
```

### _DIR$ (function)
**Purpose**: Returns common paths in Windows only, like My Documents, My Pictures, My Music, Desktop.

**Syntax**:
```basic
path$ = _DIR$(path_type)
```

### _DIREXISTS (function)
**Purpose**: Returns -1 if the Directory folder name STRING parameter exists. Zero if it does not.

**Syntax**:
```basic
exists% = _DIREXISTS(directory_path$)
```

### _DISPLAY (statement)
**Purpose**: Turns off the _AUTODISPLAY automatic display while only displaying the screen changes when called.

**Syntax**:
```basic
_DISPLAY
```

### _DISPLAY (function)
**Purpose**: Returns the handle of the current image that is displayed on the screen.

**Syntax**:
```basic
display_handle = _DISPLAY
```

### _DISPLAYORDER (statement)
**Purpose**: Designates the order to render software, hardware and custom-opengl-code.

**Syntax**:
```basic
_DISPLAYORDER software%, hardware%, custom%
```

### _DONTBLEND (statement)
**Purpose**: Statement turns off default _BLEND 32-bit _ALPHA alpha blending for the current image or screen.

**Syntax**:
```basic
_DONTBLEND
```

### _DONTWAIT (SHELL action)
**Purpose**: Specifies that the program should not wait until the shelled command/program is finished.

**Syntax**:
```basic
SHELL command$, _DONTWAIT
```

### _DROPPEDFILE (function)
**Purpose**: Returns the list of items (files or folders) dropped in a program's window after _ACCEPTFILEDROP is enabled.

**Syntax**:
```basic
file_list$ = _DROPPEDFILE
```

---

*Continue to next section...*
