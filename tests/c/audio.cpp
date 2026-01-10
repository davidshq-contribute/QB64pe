// Unit tests for audio module
// Tests audio functions that can be tested without audio hardware

#include "test.h"
#include "audio.h"
#include "qbs.h"
#include <stdint.h>

static void test_sub_beep() {
    // Test beep function (may not produce sound in test environment)
    // This should not crash even if audio is not available
    sub_beep();
    test_assert(1); // If we get here, beep didn't crash
}

static void test_func__sndrate() {
    // Test getting sample rate
    // This should return a valid sample rate even if audio is not initialized
    int32_t rate = func__sndrate();
    // Sample rate should be a reasonable value (typically 44100, 48000, etc.)
    test_assert(rate > 0 && rate <= 192000);
}

static void test_func__sndplaying_invalid_handle() {
    // Test checking if sound is playing with invalid handle
    int32_t result = func__sndplaying(-1);
    // Should return 0 (not playing) for invalid handle
    test_assert(result == 0);
}

static void test_func__sndpaused_invalid_handle() {
    // Test checking if sound is paused with invalid handle
    int32_t result = func__sndpaused(-1);
    // Should return 0 (not paused) for invalid handle
    test_assert(result == 0);
}

static void test_sub__sndclose_invalid_handle() {
    // Test closing invalid sound handle (should not crash)
    sub__sndclose(-1);
    test_assert(1); // If we get here, close didn't crash
}

static void test_sub__sndstop_invalid_handle() {
    // Test stopping invalid sound handle (should not crash)
    sub__sndstop(-1);
    test_assert(1); // If we get here, stop didn't crash
}

static void test_sub__sndpause_invalid_handle() {
    // Test pausing invalid sound handle (should not crash)
    sub__sndpause(-1);
    test_assert(1); // If we get here, pause didn't crash
}

static void test_sub__sndvol_invalid_handle() {
    // Test setting volume on invalid sound handle (should not crash)
    sub__sndvol(-1, 0.5);
    test_assert(1); // If we get here, volume set didn't crash
}

static void test_sub__sndloop_invalid_handle() {
    // Test looping invalid sound handle (should not crash)
    sub__sndloop(-1);
    test_assert(1); // If we get here, loop didn't crash
}

static void test_func__sndlen_invalid_handle() {
    // Test getting length of invalid sound handle
    double len = func__sndlen(-1);
    // Should return 0 or negative for invalid handle
    test_assert(len <= 0.0);
}

static void test_func__sndgetpos_invalid_handle() {
    // Test getting position of invalid sound handle
    double pos = func__sndgetpos(-1);
    // Should return 0 or negative for invalid handle
    test_assert(pos <= 0.0);
}

static void test_sub__sndsetpos_invalid_handle() {
    // Test setting position on invalid sound handle (should not crash)
    sub__sndsetpos(-1, 1.0);
    test_assert(1); // If we get here, setpos didn't crash
}

static void test_sub__sndlimit_invalid_handle() {
    // Test setting limit on invalid sound handle (should not crash)
    sub__sndlimit(-1, 5.0);
    test_assert(1); // If we get here, limit didn't crash
}

static void test_func__sndcopy_invalid_handle() {
    // Test copying invalid sound handle
    int32_t result = func__sndcopy(-1);
    // Should return 0 or negative for invalid handle
    test_assert(result <= 0);
}

static void test_sub__sndplay_invalid_handle() {
    // Test playing invalid sound handle (should not crash)
    sub__sndplay(-1);
    test_assert(1); // If we get here, play didn't crash
}

static void test_sub__sndplaycopy_invalid_handle() {
    // Test playing copy of invalid sound handle (should not crash)
    sub__sndplaycopy(-1, 1.0, 0.0, 0.0, 0.0, 0);
    test_assert(1); // If we get here, playcopy didn't crash
}

static void test_sub__sndplayfile_nonexistent() {
    // Test playing nonexistent file (should not crash)
    qbs *filename = qbs_new_txt("nonexistent_audio_file.wav");
    sub__sndplayfile(filename, 0, 1.0, 0);
    qbs_free(filename);
    test_assert(1); // If we get here, playfile didn't crash
}

static void test_sub__sndbal_invalid_handle() {
    // Test setting balance on invalid sound handle (should not crash)
    sub__sndbal(-1, 0.0, 0.0, 0.0, 0, 0);
    test_assert(1); // If we get here, balance didn't crash
}

static void test_func__sndopenraw() {
    // Test opening raw sound stream
    // This may fail if audio is not initialized, but should not crash
    int32_t handle = func__sndopenraw();
    // Handle can be valid or invalid depending on audio initialization
    test_assert(1); // If we get here, openraw didn't crash
}

static void test_sub__sndraw_invalid_handle() {
    // Test writing raw sound data to invalid handle (should not crash)
    sub__sndraw(0.5, 0.5, -1, 0);
    test_assert(1); // If we get here, sndraw didn't crash
}

static void test_func__sndrawlen_invalid_handle() {
    // Test getting raw sound length for invalid handle
    double len = func__sndrawlen(-1, 0);
    // Should return 0 or negative for invalid handle
    test_assert(len <= 0.0);
}

static void test_func__sndnew() {
    // Test creating new sound buffer
    // This may fail if audio is not initialized, but should not crash
    int32_t handle = func__sndnew(44100, 2, 16, 44100, 0);
    // Handle can be valid or invalid depending on audio initialization
    test_assert(1); // If we get here, sndnew didn't crash
}

static void test_sub__midisoundbank_nonexistent() {
    // Test loading nonexistent MIDI sound bank (should not crash)
    qbs *filename = qbs_new_txt("nonexistent_soundbank.sf2");
    qbs *requirements = qbs_new_txt("");
    sub__midisoundbank(filename, requirements, 0);
    qbs_free(filename);
    qbs_free(requirements);
    test_assert(1); // If we get here, midisoundbank didn't crash
}

int main() {
    struct unit_test tests[] = {
        {test_sub_beep, "sub_beep"},
        {test_func__sndrate, "func__sndrate"},
        {test_func__sndplaying_invalid_handle, "func__sndplaying_invalid_handle"},
        {test_func__sndpaused_invalid_handle, "func__sndpaused_invalid_handle"},
        {test_sub__sndclose_invalid_handle, "sub__sndclose_invalid_handle"},
        {test_sub__sndstop_invalid_handle, "sub__sndstop_invalid_handle"},
        {test_sub__sndpause_invalid_handle, "sub__sndpause_invalid_handle"},
        {test_sub__sndvol_invalid_handle, "sub__sndvol_invalid_handle"},
        {test_sub__sndloop_invalid_handle, "sub__sndloop_invalid_handle"},
        {test_func__sndlen_invalid_handle, "func__sndlen_invalid_handle"},
        {test_func__sndgetpos_invalid_handle, "func__sndgetpos_invalid_handle"},
        {test_sub__sndsetpos_invalid_handle, "sub__sndsetpos_invalid_handle"},
        {test_sub__sndlimit_invalid_handle, "sub__sndlimit_invalid_handle"},
        {test_func__sndcopy_invalid_handle, "func__sndcopy_invalid_handle"},
        {test_sub__sndplay_invalid_handle, "sub__sndplay_invalid_handle"},
        {test_sub__sndplaycopy_invalid_handle, "sub__sndplaycopy_invalid_handle"},
        {test_sub__sndplayfile_nonexistent, "sub__sndplayfile_nonexistent"},
        {test_sub__sndbal_invalid_handle, "sub__sndbal_invalid_handle"},
        {test_func__sndopenraw, "func__sndopenraw"},
        {test_sub__sndraw_invalid_handle, "sub__sndraw_invalid_handle"},
        {test_func__sndrawlen_invalid_handle, "func__sndrawlen_invalid_handle"},
        {test_func__sndnew, "func__sndnew"},
        {test_sub__midisoundbank_nonexistent, "sub__midisoundbank_nonexistent"},
    };
    
    return run_tests("audio", tests, sizeof(tests) / sizeof(tests[0]));
}
