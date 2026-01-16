//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE x86 CPU Emulation Module
//  Provides x86 virtual CPU emulation for CALL ABSOLUTE and CALL INTERRUPT
//  Note: This emulation is experimental and limited
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"
#include "cpu_emu.h"
#include "graphics.h"      // For img_struct
#include "qbs.h"           // For qbs struct (string)
#include "error_handle.h"  // For error(), is_error_pending()

#include "../../os.h"      // For int32, uint8 type definitions
#include "../../common.h"  // For byte_element_struct

#include <cstring>  // For memset
#include <cstdlib>  // For exit()

// ============================================================================
// EXTERNAL REFERENCES
// ============================================================================

// Conventional memory emulation (from qbx.cpp)
extern uint8 cmem[1114099];

// Default segment pointer (from generated code)
extern uint8 *defseg;

// Call absolute offsets array (from qbx.cpp)
extern uint16 call_absolute_offsets[256];

// GUI alert for error messages
extern void gui_alert(const char *message, const char *title, const char *type);

// Mouse functions (from mouse.cpp)
extern void sub__mouseshow(qbs *style, int32 passed);
extern void sub__mousehide();
extern float func__mousex();
extern float func__mousey();

// Display page (from libqb.cpp)
extern img_struct *display_page;

// Font height array (from libqb.cpp)
extern int32 *fontheight;

// Unknown opcode message string (from libqb.cpp)
extern qbs *unknown_opcode_mess;

// Mouse message queue (from libqb.cpp)
struct mouse_message {
    int32 x;
    int32 y;
    int32 movementX;
    int32 movementY;
    int32 buttons;
    int32 wheel;
};

struct mouse_message_queue_struct {
    mouse_message *queue;
    int32 lastIndex;
    int32 current;
    int32 last;
};

extern mouse_message_queue_struct mouse_message_queue;

// byte_element_struct is defined in common.h

// ============================================================================
// CPU STATE DEFINITION
// ============================================================================

cpu_struct cpu;

// Instruction pointer (into conventional memory)
static uint8 *ip;

// Default segment (DS unless overridden)
static uint8 *seg;

// Segment for BP access (SS unless overridden)
static uint8 *seg_bp;

// Register pointer arrays for opcode decoding
static uint8 *reg8[8];
static uint16 *reg16[8];
static uint32 *reg32[8];
static uint16 *segreg[8];

// Address size override (32-bit addressing)
static int32 a32;

// Operand size override (32-bit operands)
static int32 b32;

// Segment base pointers
static uint8 *seg_es_ptr;
static uint8 *seg_cs_ptr;
static uint8 *seg_ss_ptr;
static uint8 *seg_ds_ptr;
static uint8 *seg_fs_ptr;
static uint8 *seg_gs_ptr;

// Segment register indices
#define seg_es 0
#define seg_cs 1
#define seg_ss 2
#define seg_ds 3
#define seg_fs 4
#define seg_gs 5

// Macro to extract register from opcode
#define op_r i & 7

// ============================================================================
// REGISTER INITIALIZATION
// ============================================================================

// Initialize register pointer arrays (called once at startup via init function)
static bool registers_initialized = false;

static void init_cpu_registers() {
    if (registers_initialized) return;

    reg8[0] = &cpu.al;
    reg8[1] = &cpu.cl;
    reg8[2] = &cpu.dl;
    reg8[3] = &cpu.bl;
    reg8[4] = &cpu.ah;
    reg8[5] = &cpu.ch;
    reg8[6] = &cpu.dh;
    reg8[7] = &cpu.bh;

    reg16[0] = &cpu.ax;
    reg16[1] = &cpu.cx;
    reg16[2] = &cpu.dx;
    reg16[3] = &cpu.bx;
    reg16[4] = &cpu.sp;
    reg16[5] = &cpu.bp;
    reg16[6] = &cpu.si;
    reg16[7] = &cpu.di;

    reg32[0] = &cpu.eax;
    reg32[1] = &cpu.ecx;
    reg32[2] = &cpu.edx;
    reg32[3] = &cpu.ebx;
    reg32[4] = &cpu.esp;
    reg32[5] = &cpu.ebp;
    reg32[6] = &cpu.esi;
    reg32[7] = &cpu.edi;

    segreg[0] = &cpu.es;
    segreg[1] = &cpu.cs;
    segreg[2] = &cpu.ss;
    segreg[3] = &cpu.ds;
    segreg[4] = &cpu.fs;
    segreg[5] = &cpu.gs;

    registers_initialized = true;
}

// ============================================================================
// SIB (Scale-Index-Base) BYTE DECODING
// ============================================================================

static uint32 sib() {
    static uint32 i;
    i = *ip++;
    switch (i >> 6) {
    case 0:
        return *reg32[i & 7] + *reg32[i >> 3 & 7];
        break;
    case 1:
        return *reg32[i & 7] + (*reg32[i >> 3 & 7] << 1);
        break;
    case 2:
        return *reg32[i & 7] + (*reg32[i >> 3 & 7] << 2);
        break;
    case 3:
        return *reg32[i & 7] + (*reg32[i >> 3 & 7] << 3);
        break;
    }
    return 0;
}

static uint32 sib_mod0() {
    // Note: Called when top 2 bits of rm byte before sib byte were 0, base register is ignored
    //      and replaced with an int32 following the sib byte
    static uint32 i;
    i = *ip++;
    if ((i & 7) == 5) {
        switch (i >> 6) {
        case 0:
            return (*(uint32 *)((ip += 4) - 4)) + *reg32[i >> 3 & 7];
            break;
        case 1:
            return (*(uint32 *)((ip += 4) - 4)) + (*reg32[i >> 3 & 7] << 1);
            break;
        case 2:
            return (*(uint32 *)((ip += 4) - 4)) + (*reg32[i >> 3 & 7] << 2);
            break;
        case 3:
            return (*(uint32 *)((ip += 4) - 4)) + (*reg32[i >> 3 & 7] << 3);
            break;
        }
    }
    switch (i >> 6) {
    case 0:
        return *reg32[i & 7] + *reg32[i >> 3 & 7];
        break;
    case 1:
        return *reg32[i & 7] + (*reg32[i >> 3 & 7] << 1);
        break;
    case 2:
        return *reg32[i & 7] + (*reg32[i >> 3 & 7] << 2);
        break;
    case 3:
        return *reg32[i & 7] + (*reg32[i >> 3 & 7] << 3);
        break;
    }
    return 0;
}

// ============================================================================
// R/M BYTE DECODING (8/16/32-bit)
// ============================================================================

static uint8 *rm8() {
    static uint32 i;
    i = *ip++;
    switch (i >> 6) {
    case 3:
        return reg8[i & 7];
        break;
    case 0:
        if (a32) {
            switch (i & 7) {
            case 0:
                return seg + cpu.ax;
                break;
            case 1:
                return seg + cpu.cx;
                break;
            case 2:
                return seg + cpu.dx;
                break;
            case 3:
                return seg + cpu.bx;
                break;
            case 4:
                return seg + (uint16)sib_mod0();
                break;
            case 5:
                return seg + (*(uint16 *)((ip += 4) - 4));
                break;
            case 6:
                return seg + cpu.si;
                break;
            case 7:
                return seg + cpu.di;
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return seg + ((uint16)(cpu.bx + cpu.si));
                break;
            case 1:
                return seg + ((uint16)(cpu.bx + cpu.di));
                break;
            case 2:
                return seg_bp + ((uint16)(cpu.bp + cpu.si));
                break;
            case 3:
                return seg_bp + ((uint16)(cpu.bp + cpu.di));
                break;
            case 4:
                return seg + cpu.si;
                break;
            case 5:
                return seg + cpu.di;
                break;
            case 6:
                return seg + (*(uint16 *)((ip += 2) - 2));
                break;
            case 7:
                return seg + cpu.bx;
                break;
            }
        }
        break;
    case 1:
        if (a32) {
            switch (i & 7) {
            case 0:
                return seg + ((uint16)(cpu.eax + *(int8 *)ip++));
                break;
            case 1:
                return seg + ((uint16)(cpu.ecx + *(int8 *)ip++));
                break;
            case 2:
                return seg + ((uint16)(cpu.edx + *(int8 *)ip++));
                break;
            case 3:
                return seg + ((uint16)(cpu.ebx + *(int8 *)ip++));
                break;
            case 4:
                i = sib();
                return seg + ((uint16)(i + *(int8 *)ip++));
                break;
            case 5:
                return seg_bp + ((uint16)(cpu.ebp + *(int8 *)ip++));
                break;
            case 6:
                return seg + ((uint16)(cpu.esi + *(int8 *)ip++));
                break;
            case 7:
                return seg + ((uint16)(cpu.edi + *(int8 *)ip++));
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return seg + ((uint16)(cpu.bx + cpu.si + *(int8 *)ip++));
                break;
            case 1:
                return seg + ((uint16)(cpu.bx + cpu.di + *(int8 *)ip++));
                break;
            case 2:
                return seg_bp + ((uint16)(cpu.bp + cpu.si + *(int8 *)ip++));
                break;
            case 3:
                return seg_bp + ((uint16)(cpu.bp + cpu.di + *(int8 *)ip++));
                break;
            case 4:
                return seg + ((uint16)(cpu.si + *(int8 *)ip++));
                break;
            case 5:
                return seg + ((uint16)(cpu.di + *(int8 *)ip++));
                break;
            case 6:
                return seg_bp + ((uint16)(cpu.bp + *(int8 *)ip++));
                break;
            case 7:
                return seg + ((uint16)(cpu.bx + *(int8 *)ip++));
                break;
            }
        }
        break;
    case 2:
        if (a32) {
            switch (i & 7) {
            case 0:
                return seg + ((uint16)(cpu.eax + *(uint32 *)((ip += 4) - 4)));
                break;
            case 1:
                return seg + ((uint16)(cpu.ecx + *(uint32 *)((ip += 4) - 4)));
                break;
            case 2:
                return seg + ((uint16)(cpu.edx + *(uint32 *)((ip += 4) - 4)));
                break;
            case 3:
                return seg + ((uint16)(cpu.ebx + *(uint32 *)((ip += 4) - 4)));
                break;
            case 4:
                i = sib();
                return seg + ((uint16)(i + *(uint32 *)((ip += 4) - 4)));
                break;
            case 5:
                return seg_bp + ((uint16)(cpu.ebp + *(uint32 *)((ip += 4) - 4)));
                break;
            case 6:
                return seg + ((uint16)(cpu.esi + *(uint32 *)((ip += 4) - 4)));
                break;
            case 7:
                return seg + ((uint16)(cpu.edi + *(uint32 *)((ip += 4) - 4)));
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return seg + ((uint16)(cpu.bx + cpu.si + *(uint16 *)((ip += 2) - 2)));
                break;
            case 1:
                return seg + ((uint16)(cpu.bx + cpu.di + *(uint16 *)((ip += 2) - 2)));
                break;
            case 2:
                return seg_bp + ((uint16)(cpu.bp + cpu.si + *(uint16 *)((ip += 2) - 2)));
                break;
            case 3:
                return seg_bp + ((uint16)(cpu.bp + cpu.di + *(uint16 *)((ip += 2) - 2)));
                break;
            case 4:
                return seg + ((uint16)(cpu.si + *(uint16 *)((ip += 2) - 2)));
                break;
            case 5:
                return seg + ((uint16)(cpu.di + *(uint16 *)((ip += 2) - 2)));
                break;
            case 6:
                return seg_bp + ((uint16)(cpu.bp + *(uint16 *)((ip += 2) - 2)));
                break;
            case 7:
                return seg + ((uint16)(cpu.bx + *(uint16 *)((ip += 2) - 2)));
                break;
            }
        }
        break;
    }
    return nullptr;
}

static uint16 *rm16() {
    static int32 i;
    i = *ip;
    switch (i >> 6) {
    case 3:
        ip++;
        return reg16[i & 7];
        break;
    case 0:
        ip++;
        if (a32) {
            switch (i & 7) {
            case 0:
                return (uint16 *)(seg + cpu.ax);
                break;
            case 1:
                return (uint16 *)(seg + cpu.cx);
                break;
            case 2:
                return (uint16 *)(seg + cpu.dx);
                break;
            case 3:
                return (uint16 *)(seg + cpu.bx);
                break;
            case 4:
                return (uint16 *)(seg + (uint16)sib_mod0());
                break;
            case 5:
                return (uint16 *)(seg + (*(uint16 *)((ip += 4) - 4)));
                break;
            case 6:
                return (uint16 *)(seg + cpu.si);
                break;
            case 7:
                return (uint16 *)(seg + cpu.di);
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return (uint16 *)(seg + ((uint16)(cpu.bx + cpu.si)));
                break;
            case 1:
                return (uint16 *)(seg + ((uint16)(cpu.bx + cpu.di)));
                break;
            case 2:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + cpu.si)));
                break;
            case 3:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + cpu.di)));
                break;
            case 4:
                return (uint16 *)(seg + cpu.si);
                break;
            case 5:
                return (uint16 *)(seg + cpu.di);
                break;
            case 6:
                return (uint16 *)(seg + (*(uint16 *)((ip += 2) - 2)));
                break;
            case 7:
                return (uint16 *)(seg + cpu.bx);
                break;
            }
        }
        break;
    case 1:
        ip++;
        if (a32) {
            switch (i & 7) {
            case 0:
                return (uint16 *)(seg + ((uint16)(cpu.eax + *(int8 *)ip++)));
                break;
            case 1:
                return (uint16 *)(seg + ((uint16)(cpu.ecx + *(int8 *)ip++)));
                break;
            case 2:
                return (uint16 *)(seg + ((uint16)(cpu.edx + *(int8 *)ip++)));
                break;
            case 3:
                return (uint16 *)(seg + ((uint16)(cpu.ebx + *(int8 *)ip++)));
                break;
            case 4:
                i = sib();
                return (uint16 *)(seg + ((uint16)(i + *(int8 *)ip++)));
                break;
            case 5:
                return (uint16 *)(seg_bp + ((uint16)(cpu.ebp + *(int8 *)ip++)));
                break;
            case 6:
                return (uint16 *)(seg + ((uint16)(cpu.esi + *(int8 *)ip++)));
                break;
            case 7:
                return (uint16 *)(seg + ((uint16)(cpu.edi + *(int8 *)ip++)));
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return (uint16 *)(seg + ((uint16)(cpu.bx + cpu.si + *(int8 *)ip++)));
                break;
            case 1:
                return (uint16 *)(seg + ((uint16)(cpu.bx + cpu.di + *(int8 *)ip++)));
                break;
            case 2:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + cpu.si + *(int8 *)ip++)));
                break;
            case 3:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + cpu.di + *(int8 *)ip++)));
                break;
            case 4:
                return (uint16 *)(seg + ((uint16)(cpu.si + *(int8 *)ip++)));
                break;
            case 5:
                return (uint16 *)(seg + ((uint16)(cpu.di + *(int8 *)ip++)));
                break;
            case 6:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + *(int8 *)ip++)));
                break;
            case 7:
                return (uint16 *)(seg + ((uint16)(cpu.bx + *(int8 *)ip++)));
                break;
            }
        }
        break;
    case 2:
        ip++;
        if (a32) {
            switch (i & 7) {
            case 0:
                return (uint16 *)(seg + ((uint16)(cpu.eax + *(uint32 *)((ip += 4) - 4))));
                break;
            case 1:
                return (uint16 *)(seg + ((uint16)(cpu.ecx + *(uint32 *)((ip += 4) - 4))));
                break;
            case 2:
                return (uint16 *)(seg + ((uint16)(cpu.edx + *(uint32 *)((ip += 4) - 4))));
                break;
            case 3:
                return (uint16 *)(seg + ((uint16)(cpu.ebx + *(uint32 *)((ip += 4) - 4))));
                break;
            case 4:
                i = sib();
                return (uint16 *)(seg + ((uint16)(i + *(uint32 *)((ip += 4) - 4))));
                break;
            case 5:
                return (uint16 *)(seg_bp + ((uint16)(cpu.ebp + *(uint32 *)((ip += 4) - 4))));
                break;
            case 6:
                return (uint16 *)(seg + ((uint16)(cpu.esi + *(uint32 *)((ip += 4) - 4))));
                break;
            case 7:
                return (uint16 *)(seg + ((uint16)(cpu.edi + *(uint32 *)((ip += 4) - 4))));
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return (uint16 *)(seg + ((uint16)(cpu.bx + cpu.si + *(uint16 *)((ip += 2) - 2))));
                break;
            case 1:
                return (uint16 *)(seg + ((uint16)(cpu.bx + cpu.di + *(uint16 *)((ip += 2) - 2))));
                break;
            case 2:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + cpu.si + *(uint16 *)((ip += 2) - 2))));
                break;
            case 3:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + cpu.di + *(uint16 *)((ip += 2) - 2))));
                break;
            case 4:
                return (uint16 *)(seg + ((uint16)(cpu.si + *(uint16 *)((ip += 2) - 2))));
                break;
            case 5:
                return (uint16 *)(seg + ((uint16)(cpu.di + *(uint16 *)((ip += 2) - 2))));
                break;
            case 6:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + *(uint16 *)((ip += 2) - 2))));
                break;
            case 7:
                return (uint16 *)(seg + ((uint16)(cpu.bx + *(uint16 *)((ip += 2) - 2))));
                break;
            }
        }
        break;
    }
    return nullptr;
}

static uint32 *rm32() {
    static int32 i;
    i = *ip;
    switch (i >> 6) {
    case 3:
        ip++;
        return reg32[i & 7];
        break;
    case 0:
        ip++;
        if (a32) {
            switch (i & 7) {
            case 0:
                return (uint32 *)(seg + cpu.ax);
                break;
            case 1:
                return (uint32 *)(seg + cpu.cx);
                break;
            case 2:
                return (uint32 *)(seg + cpu.dx);
                break;
            case 3:
                return (uint32 *)(seg + cpu.bx);
                break;
            case 4:
                return (uint32 *)(seg + (uint16)sib_mod0());
                break;
            case 5:
                return (uint32 *)(seg + (*(uint16 *)((ip += 4) - 4)));
                break;
            case 6:
                return (uint32 *)(seg + cpu.si);
                break;
            case 7:
                return (uint32 *)(seg + cpu.di);
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return (uint32 *)(seg + ((uint16)(cpu.bx + cpu.si)));
                break;
            case 1:
                return (uint32 *)(seg + ((uint16)(cpu.bx + cpu.di)));
                break;
            case 2:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + cpu.si)));
                break;
            case 3:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + cpu.di)));
                break;
            case 4:
                return (uint32 *)(seg + cpu.si);
                break;
            case 5:
                return (uint32 *)(seg + cpu.di);
                break;
            case 6:
                return (uint32 *)(seg + (*(uint16 *)((ip += 2) - 2)));
                break;
            case 7:
                return (uint32 *)(seg + cpu.bx);
                break;
            }
        }
        break;
    case 1:
        ip++;
        if (a32) {
            switch (i & 7) {
            case 0:
                return (uint32 *)(seg + ((uint16)(cpu.eax + *(int8 *)ip++)));
                break;
            case 1:
                return (uint32 *)(seg + ((uint16)(cpu.ecx + *(int8 *)ip++)));
                break;
            case 2:
                return (uint32 *)(seg + ((uint16)(cpu.edx + *(int8 *)ip++)));
                break;
            case 3:
                return (uint32 *)(seg + ((uint16)(cpu.ebx + *(int8 *)ip++)));
                break;
            case 4:
                i = sib();
                return (uint32 *)(seg + ((uint16)(i + *(int8 *)ip++)));
                break;
            case 5:
                return (uint32 *)(seg_bp + ((uint16)(cpu.ebp + *(int8 *)ip++)));
                break;
            case 6:
                return (uint32 *)(seg + ((uint16)(cpu.esi + *(int8 *)ip++)));
                break;
            case 7:
                return (uint32 *)(seg + ((uint16)(cpu.edi + *(int8 *)ip++)));
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return (uint32 *)(seg + ((uint16)(cpu.bx + cpu.si + *(int8 *)ip++)));
                break;
            case 1:
                return (uint32 *)(seg + ((uint16)(cpu.bx + cpu.di + *(int8 *)ip++)));
                break;
            case 2:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + cpu.si + *(int8 *)ip++)));
                break;
            case 3:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + cpu.di + *(int8 *)ip++)));
                break;
            case 4:
                return (uint32 *)(seg + ((uint16)(cpu.si + *(int8 *)ip++)));
                break;
            case 5:
                return (uint32 *)(seg + ((uint16)(cpu.di + *(int8 *)ip++)));
                break;
            case 6:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + *(int8 *)ip++)));
                break;
            case 7:
                return (uint32 *)(seg + ((uint16)(cpu.bx + *(int8 *)ip++)));
                break;
            }
        }
        break;
    case 2:
        ip++;
        if (a32) {
            switch (i & 7) {
            case 0:
                return (uint32 *)(seg + ((uint16)(cpu.eax + *(uint32 *)((ip += 4) - 4))));
                break;
            case 1:
                return (uint32 *)(seg + ((uint16)(cpu.ecx + *(uint32 *)((ip += 4) - 4))));
                break;
            case 2:
                return (uint32 *)(seg + ((uint16)(cpu.edx + *(uint32 *)((ip += 4) - 4))));
                break;
            case 3:
                return (uint32 *)(seg + ((uint16)(cpu.ebx + *(uint32 *)((ip += 4) - 4))));
                break;
            case 4:
                i = sib();
                return (uint32 *)(seg + ((uint16)(i + *(uint32 *)((ip += 4) - 4))));
                break;
            case 5:
                return (uint32 *)(seg_bp + ((uint16)(cpu.ebp + *(uint32 *)((ip += 4) - 4))));
                break;
            case 6:
                return (uint32 *)(seg + ((uint16)(cpu.esi + *(uint32 *)((ip += 4) - 4))));
                break;
            case 7:
                return (uint32 *)(seg + ((uint16)(cpu.edi + *(uint32 *)((ip += 4) - 4))));
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return (uint32 *)(seg + ((uint16)(cpu.bx + cpu.si + *(uint16 *)((ip += 2) - 2))));
                break;
            case 1:
                return (uint32 *)(seg + ((uint16)(cpu.bx + cpu.di + *(uint16 *)((ip += 2) - 2))));
                break;
            case 2:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + cpu.si + *(uint16 *)((ip += 2) - 2))));
                break;
            case 3:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + cpu.di + *(uint16 *)((ip += 2) - 2))));
                break;
            case 4:
                return (uint32 *)(seg + ((uint16)(cpu.si + *(uint16 *)((ip += 2) - 2))));
                break;
            case 5:
                return (uint32 *)(seg + ((uint16)(cpu.di + *(uint16 *)((ip += 2) - 2))));
                break;
            case 6:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + *(uint16 *)((ip += 2) - 2))));
                break;
            case 7:
                return (uint32 *)(seg + ((uint16)(cpu.bx + *(uint16 *)((ip += 2) - 2))));
                break;
            }
        }
        break;
    }
    return nullptr;
}

// ============================================================================
// INTERRUPT HANDLER
// ============================================================================

void call_int(int32 i) {
    if (i == 0x33) {
        if (cpu.ax == 0) {
            cpu.ax = 0xFFFF; // mouse installed
            cpu.bx = 2;
            return;
        }

        if (cpu.ax == 1) {
            sub__mouseshow(NULL, 0);
            return;
        }
        if (cpu.ax == 2) {
            sub__mousehide();
            return;
        }
        if (cpu.ax == 3) {
            // return the current mouse status
            mouse_message_queue_struct *queue = &mouse_message_queue;

            // buttons
            cpu.bx = queue->queue[queue->last].buttons & 1;
            if (queue->queue[queue->last].buttons & 4)
                cpu.bx += 2;

            // x,y offsets
            static float mx, my;

            // temp override current message index to the most recent event
            static int32 current_mouse_message_backup;
            current_mouse_message_backup = queue->current;
            queue->current = queue->last;

            mx = func__mousex();
            my = func__mousey();

            // restore "current" message index
            queue->current = current_mouse_message_backup;

            cpu.cx = mx;
            cpu.dx = my;
            // double x-axis value for modes 1,7,13
            if ((display_page->compatible_mode == 1) || (display_page->compatible_mode == 7) || (display_page->compatible_mode == 13))
                cpu.cx *= 2;
            if (display_page->text) {
                // note: a range from 0 to columns*8-1 is returned regardless of the number of actual pixels
                cpu.cx = (mx - 0.5) * 8.0;
                if (cpu.cx >= (display_page->width * 8))
                    cpu.cx = (display_page->width * 8) - 1;
                // note: a range from 0 to rows*8-1 is returned regardless of the number of actual pixels
                cpu.dx = (my - 0.5) * 8.0;
                if (cpu.dx >= (display_page->height * 8))
                    cpu.dx = (display_page->height * 8) - 1;
            }
            return;
        }

        if (cpu.ax == 7) { // horizontal min/max
            return;
        }
        if (cpu.ax == 8) { // vertical min/max
            return;
        }

        return;
    }
}

// ============================================================================
// CPU EXECUTION
// ============================================================================

void cpu_call() {
    // Ensure registers are initialized
    init_cpu_registers();

    static int32 i, i2;
    static uint8 *uint8p;
    static uint16 *uint16p;
    static uint32 *uint32p;
    static int32 r;

    ip = (uint8 *)&cmem[cpu.cs * 16 + cpu.ip];

    seg_es_ptr = (uint8 *)cmem + cpu.es * 16;
    seg_cs_ptr = (uint8 *)cmem + cpu.cs * 16;
    seg_ss_ptr = (uint8 *)cmem + cpu.ss * 16;
    seg_ds_ptr = (uint8 *)cmem + cpu.ds * 16;
    seg_fs_ptr = (uint8 *)cmem + cpu.fs * 16;
    seg_gs_ptr = (uint8 *)cmem + cpu.gs * 16;

next_opcode:
    b32 = 0;
    a32 = 0;
    seg = seg_ds_ptr;
    seg_bp = seg_ss_ptr;

    i = *ip++;

    // read any prefixes
    if (i == 0x66) {
        b32 = 1;
        i = *ip++;
    }
    if (i == 0x26) {
        seg_bp = seg = seg_es_ptr;
        i = *ip++;
    }
    if (i == 0x2E) {
        seg_bp = seg = seg_cs_ptr;
        i = *ip++;
    }
    if (i == 0x36) {
        seg = seg_ss_ptr;
        i = *ip++;
    }
    if (i == 0x3E) {
        seg_bp = seg_ds_ptr;
        i = *ip++;
    }
    if (i == 0x64) {
        seg_bp = seg = seg_fs_ptr;
        i = *ip++;
    }
    if (i == 0x65) {
        seg_bp = seg = seg_gs_ptr;
        i = *ip++;
    }
    if (i == 0x67) {
        a32 = 1;
        i = *ip++;
    }

    if (i == 0x0F)
        goto opcode_0F;

    r = *ip >> 3 & 7;

    // mov
    if (i != 0x8D) {
        if (i >= 0x88 && i <= 0x8E) {
            switch (i) {
            case 0x88: // /r r/m8,r8
                *rm8() = *reg8[r];
                break;
            case 0x89: // /r r/m16(32),r16(32)
                if (b32)
                    *rm32() = *reg32[r];
                else
                    *rm16() = *reg16[r];
                break;
            case 0x8A: // /r r8,r/m8
                *reg8[r] = *rm8();
                break;
            case 0x8B: // /r r16(32),r/m16(32)
                if (b32)
                    *reg32[r] = *rm32();
                else
                    *reg16[r] = *rm16();
                break;
            case 0x8C: // /r r/m16,Sreg
                *rm16() = *segreg[r];
                break;
            case 0x8E: // /r Sreg,r/m16
                *segreg[r] = *rm16();
                if (r == 0)
                    seg_es_ptr = (uint8 *)cmem + *segreg[r] * 16;
                // CS (r==1) cannot be set
                if (r == 2)
                    seg_ss_ptr = (uint8 *)cmem + *segreg[r] * 16;
                if (r == 3)
                    seg_ds_ptr = (uint8 *)cmem + *segreg[r] * 16;
                if (r == 4)
                    seg_fs_ptr = (uint8 *)cmem + *segreg[r] * 16;
                if (r == 5)
                    seg_gs_ptr = (uint8 *)cmem + *segreg[r] * 16;
                break;
            }
            goto done;
        }
    }
    if (i >= 0xA0 && i <= 0xA3) {
        switch (i) {
        case 0xA0: // al,moffs8
            cpu.al = *(seg + *(uint16 *)ip);
            ip += 2;
            break;
        case 0xA1: // (e)ax,moffs16(32)
            if (b32) {
                cpu.eax = *(uint32 *)(seg + *(uint16 *)ip);
                ip += 2;
            } else {
                cpu.ax = *(uint16 *)(seg + *(uint16 *)ip);
                ip += 2;
            }
            break;
        case 0xA2: // moffs8,al
            *(seg + *(uint16 *)ip) = cpu.al;
            ip += 2;
            break;
        case 0xA3: // moffs16(32),(e)ax
            if (b32) {
                *(uint32 *)(seg + *(uint16 *)ip) = cpu.eax;
                ip += 2;
            } else {
                *(uint16 *)(seg + *(uint16 *)ip) = cpu.ax;
                ip += 2;
            }
            break;
        }
        goto done;
    }
    if (i >= 0xB0 && i <= 0xB7) { // +rb reg8,imm8
        *reg8[op_r] = *ip++;
        goto done;
    }
    if (i >= 0xB8 && i <= 0xBF) { // +rw(rd) reg16(32),imm16(32)
        if (b32) {
            *reg32[op_r] = *(uint32 *)ip;
            ip += 4;
        } else {
            *reg16[op_r] = *(uint16 *)ip;
            ip += 2;
        }
        goto done;
    }
    if (i == 0xC6) { // r/m8,imm8
        uint8p = rm8();
        *uint8p = *ip++;
        goto done;
    }
    if (i == 0xC7) { // r/m16(32),imm16(32)
        if (b32) {
            uint32p = rm32();
            *uint32p = *(uint32 *)ip;
            ip += 4;
        } else {
            uint16p = rm16();
            *uint16p = *(uint16 *)ip;
            ip += 2;
        }
        goto done;
    }

    // RET instruction handler
    if (i == 0xCB) { // far return
        return;
    }
    if (i == 0xCA) { // imm16 (far)
        return;
    }

    // INT instruction handler
    if (i == 0xCD) {
        call_int(*ip++);
        goto done;
    }

    // push
    if (i == 0xFF) {
        if (b32) {
            *((uint32 *)(seg_ss_ptr + (cpu.sp -= 4))) = *rm32();
        } else {
            *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *rm16();
        }
        goto done;
    }
    if (i >= 0x50 && i <= 0x57) { //+ /r r16(32)
        if (b32) {
            *((uint32 *)(seg_ss_ptr + (cpu.sp -= 4))) = *reg32[op_r];
        } else {
            *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *reg16[op_r];
        }
        goto done;
    }
    if (i == 0x6A) { // imm8 (sign extended to 16 bits)
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = ((int8)*ip++);
        goto done;
    }
    if (i == 0x68) { // imm16(32)
        if (b32) {
            *((uint32 *)(seg_ss_ptr + (cpu.sp -= 4))) = *(uint32 *)ip;
            ip += 4;
        } else {
            *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *(uint16 *)ip;
            ip += 2;
        }
        goto done;
    }
    if (i == 0x0E) { // CS
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *segreg[seg_cs];
        goto done;
    }
    if (i == 0x16) { // SS
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *segreg[seg_ss];
        goto done;
    }
    if (i == 0x1E) { // DS
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *segreg[seg_ds];
        goto done;
    }
    if (i == 0x06) { // ES
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *segreg[seg_es];
        goto done;
    }

    // pop
    if (i == 0x8F) {
        if (b32) {
            *rm32() = *((uint32 *)(seg_ss_ptr - 4 + (cpu.sp += 4)));
        } else {
            *rm16() = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        }
        goto done;
    }
    if (i >= 0x58 && i <= 0x5F) { //+rw(d) r16(32)
        if (b32) {
            *reg32[op_r] = *((uint32 *)(seg_ss_ptr - 4 + (cpu.sp += 4)));
        } else {
            *reg16[op_r] = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        }
        goto done;
    }
    if (i == 0x1F) { // DS
        *segreg[seg_ds] = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        goto done;
    }
    if (i == 0x07) { // ES
        *segreg[seg_es] = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        goto done;
    }
    if (i == 0x17) { // SS
        *segreg[seg_ss] = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        goto done;
    }

    goto skip_0F_opcodes;
opcode_0F:
    i = *ip++;
    r = *ip >> 3 & 7;

    // push
    if (i == 0xA0) {
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *segreg[seg_fs];
        goto done;
    }
    if (i == 0xA8) {
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *segreg[seg_gs];
        goto done;
    }

    // pop
    if (i == 0xA1) { // FS
        *segreg[seg_fs] = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        goto done;
    }
    if (i == 0xA9) { // GS
        *segreg[seg_gs] = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        goto done;
    }

skip_0F_opcodes:

    i2 = ((i >> 4) & 15);
    if (i2 <= 9)
        i2 += 48;
    else
        i2 = i2 - 10 + 65;
    unknown_opcode_mess->chr[16] = i2;
    i2 = i & 15;
    if (i2 <= 9)
        i2 += 48;
    else
        i2 = i2 - 10 + 65;
    unknown_opcode_mess->chr[17] = i2;
    gui_alert((const char *)unknown_opcode_mess->chr, "X86 Error", "ok");
    exit(86);
done:
    if (*ip)
        goto next_opcode;

    exit(cmem[0]);
}

// ============================================================================
// PUBLIC API FUNCTIONS
// ============================================================================

void call_absolute(int32 args, uint16 offset) {
    memset(&cpu, 0, sizeof(cpu_struct));
    cpu.cs = ((defseg - cmem) >> 4);
    cpu.ip = offset;
    cpu.ss = 0xFFFF;
    cpu.sp = 0; // sp "loops" to <65536 after first push
    cpu.ds = 80;
    // push (near) arg offsets
    static int32 i;
    for (i = 0; i < args; i++) {
        cpu.sp -= 2;
        *(uint16 *)(cmem + cpu.ss * 16 + cpu.sp) = call_absolute_offsets[i];
    }
    // push ret segment, then push ret offset (both 0xFFFF to return control to QB64)
    cpu.sp -= 4;
    *(uint32 *)(cmem + cpu.ss * 16 + cpu.sp) = 0xFFFFFFFF;
    cpu_call();
}

void call_interrupt(int32 intno, void *inregs, void *outregs) {
    if (is_error_pending())
        return;
    static byte_element_struct *ele;
    static uint16 *sp;

    // error checking
    ele = (byte_element_struct *)outregs;
    if (ele->length < 16) {
        error(5);
        return;
    }
    ele = (byte_element_struct *)inregs;
    if (ele->length < 16) {
        error(5);
        return;
    }
    // load virtual registers
    sp = (uint16 *)(ele->offset);
    cpu.ax = sp[0];
    cpu.bx = sp[1];
    cpu.cx = sp[2];
    cpu.dx = sp[3];
    cpu.bp = sp[4];
    cpu.si = sp[5];
    cpu.di = sp[6];
    // note: flags ignored (revise)
    call_int(intno);
    // save virtual registers
    ele = (byte_element_struct *)outregs;
    sp = (uint16 *)(ele->offset);
    sp[0] = cpu.ax;
    sp[1] = cpu.bx;
    sp[2] = cpu.cx;
    sp[3] = cpu.dx;
    sp[4] = cpu.bp;
    sp[5] = cpu.si;
    sp[6] = cpu.di;
    // note: flags ignored (revise)
    return;
}

void call_interruptx(int32 intno, void *inregs, void *outregs) {
    if (is_error_pending())
        return;
    static byte_element_struct *ele;
    static uint16 *sp;

    // error checking
    ele = (byte_element_struct *)outregs;
    if (ele->length < 20) {
        error(5);
        return;
    }
    ele = (byte_element_struct *)inregs;
    if (ele->length < 20) {
        error(5);
        return;
    }
    // load virtual registers
    sp = (uint16 *)(ele->offset);
    cpu.ax = sp[0];
    cpu.bx = sp[1];
    cpu.cx = sp[2];
    cpu.dx = sp[3];
    cpu.bp = sp[4];
    cpu.si = sp[5];
    cpu.di = sp[6];
    // note: flags ignored (revise)
    cpu.ds = sp[8];
    cpu.es = sp[9];
    call_int(intno);
    // save virtual registers
    ele = (byte_element_struct *)outregs;
    sp = (uint16 *)(ele->offset);
    sp[0] = cpu.ax;
    sp[1] = cpu.bx;
    sp[2] = cpu.cx;
    sp[3] = cpu.dx;
    sp[4] = cpu.bp;
    sp[5] = cpu.si;
    sp[6] = cpu.di;
    // note: flags ignored (revise)
    sp[8] = cpu.ds;
    sp[9] = cpu.es;
    return;
}
