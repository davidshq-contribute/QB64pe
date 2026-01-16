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

// ============================================================================
// BASIC SOUND FUNCTIONS
// ============================================================================

/// Plays a simple system beep sound
/// Generates a default beep tone using the system speaker
void sub_beep();

/// Plays background music or sound on specified voice
/// Manages background music playback with voice control
/// @param voice Voice number to play on (0-15)
/// @param passed Parameter passing flags (for QB64 compatibility)
/// @return Current playback position or status
double func_play(uint32_t voice, int32_t passed);

/// Plays music using string notation (QB-style PLAY statement)
/// Plays musical notes using QB64 string notation format
/// @param str1 Music string or note sequence
/// @param str2 Additional music string (optional)
/// @param str3 Additional music string (optional)
/// @param str4 Additional music string (optional)
/// @param passed Parameter passing flags (for QB64 compatibility)
void sub_play(const qbs *str1, const qbs *str2, const qbs *str3, const qbs *str4, int32_t passed);

/// Generates a sound with specified frequency and properties
/// Creates a sound wave with customizable parameters
/// @param frequency Frequency in Hz (20-20000)
/// @param lengthInClockTicks Duration in system clock ticks
/// @param volume Volume level (0.0-1.0)
/// @param panPosition Stereo panning (-1.0 left to 1.0 right)
/// @param waveform Waveform type (0=sine, 1=square, 2=sawtooth, 3=triangle, 4=noise)
/// @param waveformParam Additional waveform parameter
/// @param voice Voice number to use (0-15)
/// @param option Additional options
/// @param passed Parameter passing flags (for QB64 compatibility)
void sub_sound(float frequency, float lengthInClockTicks, float volume, float panPosition, int32_t waveform, float waveformParam, uint32_t voice,
               int32_t option, int32_t passed);

/// Plays a custom waveform from memory
/// Plays a user-defined waveform from memory buffer
/// @param voice Voice number to play on (0-15)
/// @param waveDefinition Pointer to waveform data
/// @param frameCount Number of frames in waveform
/// @param passed Parameter passing flags (for QB64 compatibility)
void sub__wave(uint32_t voice, void *waveDefinition, uint32_t frameCount, int32_t passed);

// ============================================================================
// SOUND FILE OPERATIONS
// ============================================================================

/// Gets the current audio sample rate
/// Returns the system audio sample rate in Hz
/// @return Sample rate in samples per second
int32_t func__sndrate();

/// Opens a sound file for playback
/// Loads and prepares a sound file (WAV, MP3, etc.) for playback
/// @param qbsFileName Path to sound file
/// @param qbsRequirements Requirements string (optional)
/// @param passed Parameter passing flags (for QB64 compatibility)
/// @return Sound handle number, or -1 on error
int32_t func__sndopen(qbs *qbsFileName, qbs *qbsRequirements, int32_t passed);

/// Closes a sound file and releases resources
/// Stops playback and frees memory associated with sound handle
/// @param handle Sound handle to close
void sub__sndclose(int32_t handle);

/// Creates a copy of a sound handle
/// Duplicates a sound handle for independent control
/// @param src_handle Source sound handle to copy
/// @return New sound handle, or -1 on error
int32_t func__sndcopy(int32_t src_handle);

/// Starts playback of a loaded sound
/// Begins playback of sound from current position
/// @param handle Sound handle to play
void sub__sndplay(int32_t handle);

/// Plays a sound copy with 3D positioning
/// Plays a copy of sound with spatial audio positioning
/// @param src_handle Source sound handle to copy and play
/// @param volume Volume level (0.0-1.0)
/// @param x X position in 3D space
/// @param y Y position in 3D space
/// @param z Z position in 3D space
/// @param passed Parameter passing flags (for QB64 compatibility)
void sub__sndplaycopy(int32_t src_handle, float volume, float x, float y, float z, int32_t passed);

/// Plays a sound file directly from disk
/// Loads and plays a sound file without creating persistent handle
/// @param fileName Path to sound file
/// @param sync 0=async, 1=wait for completion
/// @param volume Volume level (0.0-1.0)
/// @param passed Parameter passing flags (for QB64 compatibility)
void sub__sndplayfile(qbs *fileName, int32_t sync, float volume, int32_t passed);

// ============================================================================
// SOUND CONTROL FUNCTIONS
// ============================================================================

/// Pauses playback of a sound
/// Temporarily stops playback while maintaining current position
/// @param handle Sound handle to pause
void sub__sndpause(int32_t handle);

/// Checks if a sound is currently playing
/// Determines if sound is actively playing (not paused or stopped)
/// @param handle Sound handle to check
/// @return Non-zero if playing, 0 if not playing
int32_t func__sndplaying(int32_t handle);

/// Checks if a sound is currently paused
/// Determines if sound is in paused state
/// @param handle Sound handle to check
/// @return Non-zero if paused, 0 if not paused
int32_t func__sndpaused(int32_t handle);

/// Sets the volume of a sound
/// Adjusts playback volume for specified sound
/// @param handle Sound handle to adjust
/// @param volume Volume level (0.0-1.0)
void sub__sndvol(int32_t handle, float volume);

/// Enables looping for a sound
/// Sets sound to continuously loop when reaching end
/// @param handle Sound handle to set looping for
void sub__sndloop(int32_t handle);

/// Sets 3D audio balance and positioning
/// Positions sound in 3D space with channel control
/// @param handle Sound handle to position
/// @param x X position in 3D space
/// @param y Y position in 3D space
/// @param z Z position in 3D space
/// @param channel Audio channel to use
/// @param passed Parameter passing flags (for QB64 compatibility)
void sub__sndbal(int32_t handle, float x, float y, float z, int32_t channel, int32_t passed);

/// Gets the length of a sound in seconds
/// Returns total duration of loaded sound
/// @param handle Sound handle to query
/// @return Length in seconds
double func__sndlen(int32_t handle);

/// Gets current playback position
/// Returns current position within sound playback
/// @param handle Sound handle to query
/// @return Current position in seconds
double func__sndgetpos(int32_t handle);

/// Sets playback position
/// Seeks to specified position within sound
/// @param handle Sound handle to modify
/// @param seconds New position in seconds
void sub__sndsetpos(int32_t handle, double seconds);

/// Sets playback limit for sound
/// Limits playback to specified duration
/// @param handle Sound handle to limit
/// @param limit Maximum playback duration in seconds
void sub__sndlimit(int32_t handle, double limit);

/// Stops playback of a sound
/// Stops playback and resets position to beginning
/// @param handle Sound handle to stop
void sub__sndstop(int32_t handle);

// ============================================================================
// RAW AUDIO FUNCTIONS
// ============================================================================

/// Opens a raw audio stream for real-time audio generation
/// Creates a handle for streaming raw audio data
/// @return Raw audio handle, or -1 on error
int32_t func__sndopenraw();

/// Sends raw audio samples to audio stream
/// Streams individual audio samples in real-time
/// @param left Left channel sample value (-1.0 to 1.0)
/// @param right Right channel sample value (-1.0 to 1.0)
/// @param handle Raw audio handle
/// @param passed Parameter passing flags (for QB64 compatibility)
void sub__sndraw(float left, float right, int32_t handle, int32_t passed);

/// Sends batch of raw audio samples to audio stream
/// Streams multiple audio frames efficiently
/// @param sampleFrameArray Array of audio sample frames
/// @param channels Number of audio channels (1=mono, 2=stereo)
/// @param handle Raw audio handle
/// @param frameCount Number of frames to send
/// @param passed Parameter passing flags (for QB64 compatibility)
void sub__sndrawbatch(void *sampleFrameArray, int32_t channels, int32_t handle, uint32_t frameCount, int32_t passed);

/// Marks raw audio stream as complete
/// Signals end of raw audio streaming (dummy function)
/// @param handle Raw audio handle
/// @param passed Parameter passing flags (for QB64 compatibility)
static inline void sub__sndrawdone(int32_t handle, int32_t passed) {
    // Dummy function that does nothing
    (void)handle;
    (void)passed;
}

/// Gets length of raw audio buffer
/// Returns current length of raw audio data
/// @param handle Raw audio handle
/// @param passed Parameter passing flags (for QB64 compatibility)
/// @return Length in seconds
double func__sndrawlen(int32_t handle, int32_t passed);

// ============================================================================
// MEMORY AND SOUND CREATION
// ============================================================================

/// Creates memory block from sound data
/// Converts sound handle to memory block for direct access
/// @param handle Sound handle to convert
/// @param targetChannel Target audio channel
/// @param passed Parameter passing flags (for QB64 compatibility)
/// @return Memory block containing sound data
mem_block func__memsound(int32_t handle, int32_t targetChannel, int32_t passed);

/// Creates new sound buffer in memory
/// Allocates empty sound buffer for custom audio generation
/// @param frames Number of audio frames
/// @param channels Number of audio channels (1=mono, 2=stereo)
/// @param bits Bits per sample (8, 16, 24, 32)
/// @param sampleRate Sample rate in Hz
/// @param passed Parameter passing flags (for QB64 compatibility)
/// @return Sound handle for new buffer
int32_t func__sndnew(uint32_t frames, int32_t channels, int32_t bits, uint32_t sampleRate, int32_t passed);

/// Loads MIDI sound bank
/// Loads MIDI instrument bank for music playback
/// @param qbsFileName Path to MIDI sound bank file
/// @param qbsRequirements Requirements string (optional)
/// @param passed Parameter passing flags (for QB64 compatibility)
void sub__midisoundbank(qbs *qbsFileName, qbs *qbsRequirements, int32_t passed);

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

/// Updates audio system state
/// Processes audio engine updates (called by main loop)
/// Should be called regularly to maintain audio processing
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
