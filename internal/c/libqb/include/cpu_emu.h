//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE x86 CPU Emulation Module
//  Provides x86 virtual CPU emulation for CALL ABSOLUTE and CALL INTERRUPT
//  Note: This emulation is experimental and limited
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_CPU_EMU_H
#define INCLUDE_LIBQB_CPU_EMU_H

#include <stdint.h>

// ============================================================================
// CPU STATE STRUCTURE
// ============================================================================

/// x86 Virtual CPU state structure
/// Emulates 16-bit and 32-bit x86 registers with unions for different access widths
struct cpu_struct {
    // al,ah,ax,eax (unsigned & signed)
    union {
        struct {
            union {
                uint8_t al;
                int8_t al_signed;
            };
            union {
                uint8_t ah;
                int8_t ah_signed;
            };
        };
        uint16_t ax;
        int16_t ax_signed;
        uint32_t eax;
        int32_t eax_signed;
    };

    // bl,bh,bx,ebx (unsigned & signed)
    union {
        struct {
            union {
                uint8_t bl;
                int8_t bl_signed;
            };
            union {
                uint8_t bh;
                int8_t bh_signed;
            };
        };
        uint16_t bx;
        int16_t bx_signed;
        uint32_t ebx;
        int32_t ebx_signed;
    };

    // cl,ch,cx,ecx (unsigned & signed)
    union {
        struct {
            union {
                uint8_t cl;
                int8_t cl_signed;
            };
            union {
                uint8_t ch;
                int8_t ch_signed;
            };
        };
        uint16_t cx;
        int16_t cx_signed;
        uint32_t ecx;
        int32_t ecx_signed;
    };

    // dl,dh,dx,edx (unsigned & signed)
    union {
        struct {
            union {
                uint8_t dl;
                int8_t dl_signed;
            };
            union {
                uint8_t dh;
                int8_t dh_signed;
            };
        };
        uint16_t dx;
        int16_t dx_signed;
        uint32_t edx;
        int32_t edx_signed;
    };

    // si,esi (unsigned & signed)
    union {
        uint16_t si;
        int16_t si_signed;
        uint32_t esi;
        int32_t esi_signed;
    };

    // di,edi (unsigned & signed)
    union {
        uint16_t di;
        int16_t di_signed;
        uint32_t edi;
        int32_t edi_signed;
    };

    // bp,ebp (unsigned & signed)
    union {
        uint16_t bp;
        int16_t bp_signed;
        uint32_t ebp;
        int32_t ebp_signed;
    };

    // sp,esp (unsigned & signed)
    union {
        uint16_t sp;
        int16_t sp_signed;
        uint32_t esp;
        int32_t esp_signed;
    };

    // Segment registers: cs,ss,ds,es,fs,gs
    union {
        uint16_t cs;
        uint16_t cs_signed;
    };
    union {
        uint16_t ss;
        uint16_t ss_signed;
    };
    union {
        uint16_t ds;
        uint16_t ds_signed;
    };
    union {
        uint16_t es;
        uint16_t es_signed;
    };
    union {
        uint16_t fs;
        uint16_t fs_signed;
    };
    union {
        uint16_t gs;
        uint16_t gs_signed;
    };

    // ip,eip (unsigned & signed)
    union {
        uint16_t ip;
        uint16_t ip_signed;
        uint32_t eip;
        uint32_t eip_signed;
    };

    // CPU flags
    uint8_t overflow_flag;
    uint8_t direction_flag;
    uint8_t interrupt_flag;
    uint8_t trap_flag;
    uint8_t sign_flag;
    uint8_t zero_flag;
    uint8_t auxiliary_flag;
    uint8_t parity_flag;
    uint8_t carry_flag;
};

// ============================================================================
// PUBLIC API
// ============================================================================

/// Global CPU state (accessible for register manipulation)
extern cpu_struct cpu;

/// Execute x86 code starting at current CS:IP
void cpu_call();

/// Execute a software interrupt
/// @param i Interrupt number (e.g., 0x33 for mouse)
void call_int(int32_t i);

/// Execute CALL ABSOLUTE with arguments
/// @param args Number of arguments pushed to stack
/// @param offset Offset within current segment to call
void call_absolute(int32_t args, uint16_t offset);

/// Execute CALL INTERRUPT (RegType structure)
/// @param intno Interrupt number
/// @param inregs Input registers (byte_element pointing to RegType)
/// @param outregs Output registers (byte_element pointing to RegType)
void call_interrupt(int32_t intno, void *inregs, void *outregs);

/// Execute CALL INTERRUPTX (RegTypeX structure with segment registers)
/// @param intno Interrupt number
/// @param inregs Input registers (byte_element pointing to RegTypeX)
/// @param outregs Output registers (byte_element pointing to RegTypeX)
void call_interruptx(int32_t intno, void *inregs, void *outregs);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides x86 CPU emulation for legacy QB/QBasic programs that use:
// - CALL ABSOLUTE: Direct machine code execution
// - CALL INTERRUPT: Software interrupt invocation (INT 33h mouse, etc.)
//
// The emulation is LIMITED and EXPERIMENTAL:
// - Only a subset of x86 opcodes are implemented
// - Primarily supports MOV, PUSH, POP, and basic control flow
// - INT 33h (mouse) has partial support
// - Other interrupts may not be fully implemented
//
// Dependencies:
// - cmem[] array for conventional memory emulation
// - Mouse functions for INT 33h support
// - GUI alert for error reporting

#endif // INCLUDE_LIBQB_CPU_EMU_H
