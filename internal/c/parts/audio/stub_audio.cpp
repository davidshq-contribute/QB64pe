#include "audio.h"
#include "../../libqb.h"
#include "mem.h"
#include "libqb-common.h"

// Stub implementations for audio functions when audio library is not available
// These allow programs to compile and run without actual audio functionality
//
// Note: sub_beep() is NOT provided here because it's already implemented in
// gui.cpp using tinyfd_beep(). This avoids duplicate symbol errors when both
// gui.o and stub_audio.o are linked together (e.g., for console-only programs).

double func_play(uint32_t voice, int32_t passed) {
    (void)voice;
    (void)passed;
    return 0.0;
}

void sub_play(const qbs *str1, const qbs *str2, const qbs *str3, const qbs *str4, int32_t passed) {
    (void)str1;
    (void)str2;
    (void)str3;
    (void)str4;
    (void)passed;
    // Stub: No-op play
}

void sub_sound(float frequency, float lengthInClockTicks, float volume, float panPosition, int32_t waveform, float waveformParam, uint32_t voice,
               int32_t option, int32_t passed) {
    (void)frequency;
    (void)lengthInClockTicks;
    (void)volume;
    (void)panPosition;
    (void)waveform;
    (void)waveformParam;
    (void)voice;
    (void)option;
    (void)passed;
    // Stub: No-op sound
}

void sub__wave(uint32_t voice, void *waveDefinition, uint32_t frameCount, int32_t passed) {
    (void)voice;
    (void)waveDefinition;
    (void)frameCount;
    (void)passed;
    // Stub: No-op wave
}

int32_t func__sndrate() {
    // Return a default sample rate (44100 Hz)
    return 44100;
}

int32_t func__sndopen(qbs *qbsFileName, qbs *qbsRequirements, int32_t passed) {
    (void)qbsFileName;
    (void)qbsRequirements;
    (void)passed;
    // Return 0 to indicate failure (no audio available)
    return 0;
}

void sub__sndclose(int32_t handle) {
    (void)handle;
    // Stub: No-op close
}

int32_t func__sndcopy(int32_t src_handle) {
    (void)src_handle;
    // Return 0 to indicate failure (no audio available)
    return 0;
}

void sub__sndplay(int32_t handle) {
    (void)handle;
    // Stub: No-op play
}

void sub__sndplaycopy(int32_t src_handle, float volume, float x, float y, float z, int32_t passed) {
    (void)src_handle;
    (void)volume;
    (void)x;
    (void)y;
    (void)z;
    (void)passed;
    // Stub: No-op play copy
}

void sub__sndplayfile(qbs *fileName, int32_t sync, float volume, int32_t passed) {
    (void)fileName;
    (void)sync;
    (void)volume;
    (void)passed;
    // Stub: No-op play file
}

void sub__sndpause(int32_t handle) {
    (void)handle;
    // Stub: No-op pause
}

int32_t func__sndplaying(int32_t handle) {
    (void)handle;
    // Return false (not playing)
    return QB_FALSE;
}

int32_t func__sndpaused(int32_t handle) {
    (void)handle;
    // Return false (not paused)
    return QB_FALSE;
}

void sub__sndvol(int32_t handle, float volume) {
    (void)handle;
    (void)volume;
    // Stub: No-op volume
}

void sub__sndloop(int32_t handle) {
    (void)handle;
    // Stub: No-op loop
}

void sub__sndbal(int32_t handle, float x, float y, float z, int32_t channel, int32_t passed) {
    (void)handle;
    (void)x;
    (void)y;
    (void)z;
    (void)channel;
    (void)passed;
    // Stub: No-op balance
}

double func__sndlen(int32_t handle) {
    (void)handle;
    // Return 0.0 (no length)
    return 0.0;
}

double func__sndgetpos(int32_t handle) {
    (void)handle;
    // Return 0.0 (no position)
    return 0.0;
}

void sub__sndsetpos(int32_t handle, double seconds) {
    (void)handle;
    (void)seconds;
    // Stub: No-op set position
}

void sub__sndlimit(int32_t handle, double limit) {
    (void)handle;
    (void)limit;
    // Stub: No-op limit
}

void sub__sndstop(int32_t handle) {
    (void)handle;
    // Stub: No-op stop
}

int32_t func__sndopenraw() {
    // Return 0 (no handle available)
    return 0;
}

void sub__sndraw(float left, float right, int32_t handle, int32_t passed) {
    (void)left;
    (void)right;
    (void)handle;
    (void)passed;
    // Stub: No-op raw sound
}

void sub__sndrawbatch(void *sampleFrameArray, int32_t channels, int32_t handle, uint32_t frameCount, int32_t passed) {
    (void)sampleFrameArray;
    (void)channels;
    (void)handle;
    (void)frameCount;
    (void)passed;
    // Stub: No-op raw sound batch
}

double func__sndrawlen(int32_t handle, int32_t passed) {
    (void)handle;
    (void)passed;
    // Return 0.0 (no length)
    return 0.0;
}

mem_block func__memsound(int32_t handle, int32_t targetChannel, int32_t passed) {
    (void)handle;
    (void)targetChannel;
    (void)passed;
    // Return an invalid mem_block to indicate failure
    mem_block mb = {};
    mb.lock_offset = (intptr_t)mem_lock_base;
    mb.lock_id = INVALID_MEM_LOCK;
    mb.type = MEM_TYPE_SOUND;
    mb.sound = 0;
    return mb;
}

int32_t func__sndnew(uint32_t frames, int32_t channels, int32_t bits, uint32_t sampleRate, int32_t passed) {
    (void)frames;
    (void)channels;
    (void)bits;
    (void)sampleRate;
    (void)passed;
    // Return 0 to indicate failure (no audio available)
    return 0;
}

void sub__midisoundbank(qbs *qbsFileName, qbs *qbsRequirements, int32_t passed) {
    (void)qbsFileName;
    (void)qbsRequirements;
    (void)passed;
    // Stub: No-op MIDI sound bank
}

void snd_update() {
    // Stub: No-op update
}
