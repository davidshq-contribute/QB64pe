//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Audio Engine Module
//  Audio processing and sound management powered by miniaudio (https://miniaud.io/)
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_AUDIO_H
#define INCLUDE_LIBQB_AUDIO_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include "logging.h"
#include <stdint.h>

// Audio logging macros
#define audio_log_trace(...) libqb_log_with_scope_trace(logscope::Audio, __VA_ARGS__)
#define audio_log_info(...) libqb_log_with_scope_info(logscope::Audio, __VA_ARGS__)
#define audio_log_warn(...) libqb_log_with_scope_warn(logscope::Audio, __VA_ARGS__)
#define audio_log_error(...) libqb_log_with_scope_error(logscope::Audio, __VA_ARGS__)

#define AUDIO_DEBUG_CHECK(_exp_)                                                                                                                               \
    do {                                                                                                                                                       \
        if (!(_exp_))                                                                                                                                          \
            audio_log_warn("Condition (%s) failed", #_exp_);                                                                                                   \
    } while (0)

// Forward declarations
struct qbs;
struct mem_block;

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

// Basic sound functions
void sub_beep();
double func_play(uint32_t voice, int32_t passed);
void sub_play(const qbs *str1, const qbs *str2, const qbs *str3, const qbs *str4, int32_t passed);
void sub_sound(float frequency, float lengthInClockTicks, float volume, float panPosition, int32_t waveform, float waveformParam, uint32_t voice,
               int32_t option, int32_t passed);
void sub__wave(uint32_t voice, void *waveDefinition, uint32_t frameCount, int32_t passed);

// Sound file operations
int32_t func__sndrate();
int32_t func__sndopen(qbs *qbsFileName, qbs *qbsRequirements, int32_t passed);
void sub__sndclose(int32_t handle);
int32_t func__sndcopy(int32_t src_handle);
void sub__sndplay(int32_t handle);
void sub__sndplaycopy(int32_t src_handle, float volume, float x, float y, float z, int32_t passed);
void sub__sndplayfile(qbs *fileName, int32_t sync, float volume, int32_t passed);

// Sound control functions
void sub__sndpause(int32_t handle);
int32_t func__sndplaying(int32_t handle);
int32_t func__sndpaused(int32_t handle);
void sub__sndvol(int32_t handle, float volume);
void sub__sndloop(int32_t handle);
void sub__sndbal(int32_t handle, float x, float y, float z, int32_t channel, int32_t passed);
double func__sndlen(int32_t handle);
double func__sndgetpos(int32_t handle);
void sub__sndsetpos(int32_t handle, double seconds);
void sub__sndlimit(int32_t handle, double limit);
void sub__sndstop(int32_t handle);

// Raw audio functions
int32_t func__sndopenraw();
void sub__sndraw(float left, float right, int32_t handle, int32_t passed);
void sub__sndrawbatch(void *sampleFrameArray, int32_t channels, int32_t handle, uint32_t frameCount, int32_t passed);
static inline void sub__sndrawdone(int32_t handle, int32_t passed) {
    // Dummy function that does nothing
    (void)handle;
    (void)passed;
}
double func__sndrawlen(int32_t handle, int32_t passed);

// Memory and sound creation
mem_block func__memsound(int32_t handle, int32_t targetChannel, int32_t passed);
int32_t func__sndnew(uint32_t frames, int32_t channels, int32_t bits, uint32_t sampleRate, int32_t passed);
void sub__midisoundbank(qbs *qbsFileName, qbs *qbsRequirements, int32_t passed);

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

// Audio system update (called by main loop)
void snd_update();

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides comprehensive audio functionality including:
// - Basic sound generation (beep, play, sound)
// - Sound file loading and playback (WAV, MP3, etc.)
// - Real-time sound manipulation and control
// - Raw audio buffer management
// - 3D positional audio support
// - MIDI sound bank support
//
// Uses miniaudio library for cross-platform audio processing

#endif // INCLUDE_LIBQB_AUDIO_H
