//----------------------------------------------------------------------------------------------------------------------
//    ___  ___   __ _ _  ___ ___     _          _ _       ___           _
//   / _ \| _ ) / /| | || _ \ __|   /_\ _  _ __| (_)___  | __|_ _  __ _(_)_ _  ___
//  | (_) | _ \/ _ \_  _|  _/ _|   / _ \ || / _` | / _ \ | _|| ' \/ _` | | ' \/ -_)
//   \__\_\___/\___/ |_||_| |___| /_/ \_\_,_\__,_|_\___/ |___|_||_\__, |_|_||_\___|
//                                                                |___/
//
//  QB64-PE Audio Engine powered by miniaudio (https://miniaud.io/)
//
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "logging.h"
#include <stdint.h>
#include <stdio.h>

/**
 * @file audio.h
 * @brief Audio functions for QB64-PE
 * 
 * This header provides functions for playing sounds, managing audio handles,
 * and controlling audio playback. The audio engine is powered by miniaudio.
 */

/**
 * @name Audio Logging Macros
 * @brief Macros for logging audio-related messages
 */
///@{
#define audio_log_trace(...) libqb_log_with_scope_trace(logscope::Audio, __VA_ARGS__)
#define audio_log_info(...) libqb_log_with_scope_info(logscope::Audio, __VA_ARGS__)
#define audio_log_warn(...) libqb_log_with_scope_warn(logscope::Audio, __VA_ARGS__)
#define audio_log_error(...) libqb_log_with_scope_error(logscope::Audio, __VA_ARGS__)
///@}

/**
 * @brief Debug check macro for audio operations
 * @param _exp_ Expression to check
 * @note Logs a warning if the expression evaluates to false
 */
#define AUDIO_DEBUG_CHECK(_exp_)                                                                                                                               \
    do {                                                                                                                                                       \
        if (!(_exp_))                                                                                                                                          \
            audio_log_warn("Condition (%s) failed", #_exp_);                                                                                                   \
    } while (0)

struct qbs;
struct mem_block;

/**
 * @brief Plays a beep sound (QB64 BEEP statement)
 * @note Plays the system beep sound
 */
void sub_beep();

/**
 * @brief Gets the current position of a playing voice (QB64 PLAY function)
 * @param voice Voice number to query
 * @param passed Flag indicating if voice parameter was provided
 * @return Current position in the music sequence, or 0 if not playing
 * @note Returns the current position for music playback using the PLAY statement
 */
double func_play(uint32_t voice, int32_t passed);

/**
 * @brief Plays music using the PLAY statement (QB64 PLAY statement)
 * @param str1 First music string parameter
 * @param str2 Second music string parameter
 * @param str3 Third music string parameter
 * @param str4 Fourth music string parameter
 * @param passed Number of parameters provided
 * @note Plays music using QB64 PLAY statement syntax
 */
void sub_play(const qbs *str1, const qbs *str2, const qbs *str3, const qbs *str4, int32_t passed);

/**
 * @brief Plays a sound with specified parameters (QB64 SOUND statement)
 * @param frequency Sound frequency in Hz
 * @param lengthInClockTicks Duration in clock ticks
 * @param volume Volume level (0.0 to 1.0)
 * @param panPosition Pan position (-1.0 to 1.0, where 0.0 is center)
 * @param waveform Waveform type (0=sine, 1=square, 2=sawtooth, 3=triangle, etc.)
 * @param waveformParam Additional waveform parameter
 * @param voice Voice number to use
 * @param option Additional option flags
 * @param passed Number of parameters provided
 * @note Plays a sound with the specified characteristics
 */
void sub_sound(float frequency, float lengthInClockTicks, float volume, float panPosition, int32_t waveform, float waveformParam, uint32_t voice,
               int32_t option, int32_t passed);

/**
 * @brief Plays a custom wave definition (QB64 _WAVE statement)
 * @param voice Voice number to use
 * @param waveDefinition Pointer to wave definition data
 * @param frameCount Number of audio frames in the wave definition
 * @param passed Flag indicating if parameters were provided
 * @note Plays a custom waveform defined by the provided data
 */
void sub__wave(uint32_t voice, void *waveDefinition, uint32_t frameCount, int32_t passed);

/**
 * @brief Gets the current audio sample rate (QB64 _SNDRATE function)
 * @return Sample rate in Hz
 * @note Returns the current audio output sample rate
 */
int32_t func__sndrate();

/**
 * @brief Opens a sound file (QB64 _SNDOPEN function)
 * @param qbsFileName qbs string containing the sound file path
 * @param qbsRequirements qbs string containing audio requirements (optional)
 * @param passed Flag indicating if requirements parameter was provided
 * @return Sound handle on success, 0 on failure
 * @note Opens a sound file and returns a handle for playback operations
 */
int32_t func__sndopen(qbs *qbsFileName, qbs *qbsRequirements, int32_t passed);

/**
 * @brief Closes a sound handle (QB64 _SNDCLOSE statement)
 * @param handle Sound handle to close
 * @note Releases resources associated with the sound handle
 */
void sub__sndclose(int32_t handle);

/**
 * @brief Creates a copy of a sound handle (QB64 _SNDCOPY function)
 * @param src_handle Source sound handle to copy
 * @return New sound handle that is a copy of the source, or 0 on failure
 * @note Creates an independent copy that can be played separately
 */
int32_t func__sndcopy(int32_t src_handle);

/**
 * @brief Starts playing a sound (QB64 _SNDPLAY statement)
 * @param handle Sound handle to play
 * @note Begins playback of the sound. If already playing, restarts from the beginning.
 */
void sub__sndplay(int32_t handle);

/**
 * @brief Plays a copy of a sound with 3D positioning (QB64 _SNDPLAYCOPY statement)
 * @param src_handle Source sound handle to copy and play
 * @param volume Volume level (0.0 to 1.0)
 * @param x X position for 3D audio
 * @param y Y position for 3D audio
 * @param z Z position for 3D audio
 * @param passed Number of parameters provided
 * @note Creates a copy of the sound and plays it with 3D positioning
 */
void sub__sndplaycopy(int32_t src_handle, float volume, float x, float y, float z, int32_t passed);

/**
 * @brief Plays a sound file directly (QB64 _SNDPLAYFILE statement)
 * @param fileName qbs string containing the sound file path
 * @param sync Synchronous flag (non-zero to wait for playback to complete)
 * @param volume Volume level (0.0 to 1.0)
 * @param passed Number of parameters provided
 * @note Opens and plays a sound file directly without creating a handle
 */
void sub__sndplayfile(qbs *fileName, int32_t sync, float volume, int32_t passed);

/**
 * @brief Pauses a playing sound (QB64 _SNDPAUSE statement)
 * @param handle Sound handle to pause
 * @note Pauses playback. Use sub__sndplay() to resume.
 */
void sub__sndpause(int32_t handle);

/**
 * @brief Checks if a sound is currently playing (QB64 _SNDPLAYING function)
 * @param handle Sound handle to check
 * @return Non-zero if playing, 0 if not playing
 * @note Returns true if the sound is currently playing
 */
int32_t func__sndplaying(int32_t handle);

/**
 * @brief Checks if a sound is currently paused (QB64 _SNDPAUSED function)
 * @param handle Sound handle to check
 * @return Non-zero if paused, 0 if not paused
 * @note Returns true if the sound is currently paused
 */
int32_t func__sndpaused(int32_t handle);

/**
 * @brief Sets the volume of a sound (QB64 _SNDVOL statement)
 * @param handle Sound handle to modify
 * @param volume Volume level (0.0 to 1.0, where 1.0 is maximum)
 * @note Sets the playback volume for the specified sound handle
 */
void sub__sndvol(int32_t handle, float volume);

/**
 * @brief Sets a sound to loop continuously (QB64 _SNDLOOP statement)
 * @param handle Sound handle to set to loop
 * @note Makes the sound automatically restart when it reaches the end
 */
void sub__sndloop(int32_t handle);

/**
 * @brief Sets 3D audio balance/positioning (QB64 _SNDBAL statement)
 * @param handle Sound handle to modify
 * @param x X position for 3D audio
 * @param y Y position for 3D audio
 * @param z Z position for 3D audio
 * @param channel Channel number (optional)
 * @param passed Number of parameters provided
 * @note Sets the 3D spatial position of the sound for positional audio
 */
void sub__sndbal(int32_t handle, float x, float y, float z, int32_t channel, int32_t passed);

/**
 * @brief Gets the length of a sound in seconds (QB64 _SNDLEN function)
 * @param handle Sound handle to query
 * @return Length of the sound in seconds, or 0.0 on error
 * @note Returns the total duration of the sound file
 */
double func__sndlen(int32_t handle);

/**
 * @brief Gets the current playback position in seconds (QB64 _SNDGETPOS function)
 * @param handle Sound handle to query
 * @return Current position in seconds, or 0.0 on error
 * @note Returns how far into the sound playback has progressed
 */
double func__sndgetpos(int32_t handle);

/**
 * @brief Sets the playback position in seconds (QB64 _SNDSETPOS statement)
 * @param handle Sound handle to modify
 * @param seconds Position to seek to in seconds
 * @note Seeks to the specified position in the sound file
 */
void sub__sndsetpos(int32_t handle, double seconds);

/**
 * @brief Sets a playback time limit (QB64 _SNDLIMIT statement)
 * @param handle Sound handle to modify
 * @param limit Maximum playback time in seconds
 * @note Limits how long the sound will play before automatically stopping
 */
void sub__sndlimit(int32_t handle, double limit);

/**
 * @brief Stops a playing sound (QB64 _SNDSTOP statement)
 * @param handle Sound handle to stop
 * @note Stops playback and resets position to the beginning
 */
void sub__sndstop(int32_t handle);

/**
 * @brief Opens a raw audio stream for writing (QB64 _SNDOPENRAW function)
 * @return Sound handle for raw audio output, or 0 on failure
 * @note Opens a handle for writing raw audio samples directly
 */
int32_t func__sndopenraw();

/**
 * @brief Writes raw audio samples to a sound handle (QB64 _SNDRAW statement)
 * @param left Left channel sample value (-1.0 to 1.0)
 * @param right Right channel sample value (-1.0 to 1.0)
 * @param handle Sound handle opened with func__sndopenraw()
 * @param passed Number of parameters provided
 * @note Writes a single stereo sample frame to the raw audio stream
 */
void sub__sndraw(float left, float right, int32_t handle, int32_t passed);

/**
 * @brief Writes a batch of raw audio samples (QB64 _SNDRAW statement)
 * @param sampleFrameArray Pointer to array of sample frames
 * @param channels Number of audio channels
 * @param handle Sound handle opened with func__sndopenraw()
 * @param frameCount Number of frames to write
 * @param passed Number of parameters provided
 * @note Writes multiple sample frames at once for efficient batch processing
 */
void sub__sndrawbatch(void *sampleFrameArray, int32_t channels, int32_t handle, uint32_t frameCount, int32_t passed);

/**
 * @brief Marks raw audio stream as complete (QB64 _SNDRAWDONE statement)
 * @param handle Sound handle
 * @param passed Flag indicating if parameter was provided
 * @note Dummy function that does nothing. Reserved for future use.
 */
static inline void sub__sndrawdone(int32_t handle, int32_t passed) {
    // Dummy function that does nothing
    (void)handle;
    (void)passed;
}

/**
 * @brief Gets the length of buffered raw audio (QB64 _SNDRAWLEN function)
 * @param handle Sound handle opened with func__sndopenraw()
 * @param passed Flag indicating if parameter was provided
 * @return Length of buffered audio in seconds
 * @note Returns how much audio is currently buffered in the raw stream
 */
double func__sndrawlen(int32_t handle, int32_t passed);

/**
 * @brief Gets a memory block for accessing sound data (QB64 _MEMSOUND function)
 * @param handle Sound handle
 * @param targetChannel Target channel to access (0 for all channels)
 * @param passed Flag indicating if targetChannel parameter was provided
 * @return mem_block structure for accessing the sound's audio data
 * @note Returns a memory block that can be used to read or modify sound data directly
 */
mem_block func__memsound(int32_t handle, int32_t targetChannel, int32_t passed);

/**
 * @brief Creates a new sound with specified format (QB64 _SNDNEW function)
 * @param frames Number of audio frames
 * @param channels Number of audio channels (1=mono, 2=stereo)
 * @param bits Bits per sample (8, 16, 24, or 32)
 * @param sampleRate Sample rate in Hz
 * @param passed Number of parameters provided
 * @return Sound handle on success, 0 on failure
 * @note Creates a new empty sound buffer with the specified format
 */
int32_t func__sndnew(uint32_t frames, int32_t channels, int32_t bits, uint32_t sampleRate, int32_t passed);

/**
 * @brief Loads a MIDI sound bank (QB64 _MIDISOUNDBANK statement)
 * @param qbsFileName qbs string containing the sound bank file path
 * @param qbsRequirements qbs string containing requirements (optional)
 * @param passed Flag indicating if requirements parameter was provided
 * @note Loads a sound bank file for MIDI playback
 */
void sub__midisoundbank(qbs *qbsFileName, qbs *qbsRequirements, int32_t passed);

/**
 * @brief Updates the audio system
 * @note Should be called periodically to process audio events and update playback state
 */
void snd_update();
