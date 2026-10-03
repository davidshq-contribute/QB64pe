# AUDIO.md

Documentation of the QB64pe audio system implementation.

## Overview

QB64pe uses the miniaudio library for cross-platform audio playback with support for multiple formats, streaming, and real-time audio synthesis.

## Core Architecture

### miniaudio Integration

**Location:** `internal/c/parts/audio/`

```c
ma_engine audioEngine;           // Main audio engine
ma_sound *sounds[];              // Array of sound objects
int32 sound_count;               // Number of sound slots
```

### Sound Handle System

| Range | Type |
|-------|------|
| > 0 | Valid sound handle |
| 0 | Invalid/no sound |
| < 0 | Error indicator |

## Sound Loading

### _SNDOPEN

```c
int32 func__sndopen(qbs *filename, qbs *requirements, int32 passed);
```

**Supported formats:**
- WAV (uncompressed)
- MP3 (via dr_mp3)
- OGG Vorbis (via stb_vorbis)
- FLAC (via dr_flac)
- MIDI (via TinySoundFont)
- MOD/XM/IT (via libxmp-lite)

**Requirements string options:**
- `STREAM` - Stream from disk (large files)
- `MEMORY` - Load entirely to memory
- `NODECODE` - Don't pre-decode

### _SNDCLOSE

```c
void sub__sndclose(int32 handle);
```

Releases sound handle and associated resources.

### _SNDCOPY

```c
int32 func__sndcopy(int32 handle);
```

Creates independent copy of sound for simultaneous playback.

## Sound Playback

### _SNDPLAY

```c
void sub__sndplay(int32 handle);
```

Starts playback from current position.

### _SNDSTOP

```c
void sub__sndstop(int32 handle);
```

Stops playback and resets position to start.

### _SNDPAUSE

```c
void sub__sndpause(int32 handle);
```

Pauses without resetting position.

### _SNDLOOP

```c
void sub__sndloop(int32 handle);
```

Starts looping playback.

### _SNDPLAYCOPY

```c
int32 func__sndplaycopy(int32 handle, float volume, int32 passed);
```

Creates temporary copy, plays it, auto-frees when done.

### _SNDPLAYFILE

```c
void sub__sndplayfile(qbs *filename, int32 sync, float volume, int32 passed);
```

Quick one-shot playback without handle management.

## Sound Properties

### Position Control

```c
void sub__sndsetpos(int32 handle, double seconds);
double func__sndgetpos(int32 handle);
double func__sndlen(int32 handle);
```

### Volume Control

```c
void sub__sndvol(int32 handle, float volume);
```

Volume range: 0.0 (silent) to 1.0 (full)

### Balance/Panning

```c
void sub__sndbal(int32 handle, float pan, float x, float y, float z, int32 passed);
```

Pan range: -1.0 (left) to 1.0 (right)

### Playback Status

```c
int32 func__sndplaying(int32 handle);  // Returns -1 if playing
int32 func__sndpaused(int32 handle);   // Returns -1 if paused
```

## Raw Audio (_SNDRAW)

### Buffer System

```c
struct sndraw_struct {
    ma_pcm_rb buffer;            // Ring buffer
    int32 channels;              // 1=mono, 2=stereo
    int32 sample_rate;           // Default 44100
    ma_sound *sound;             // Playback object
};
```

### _SNDRAW

```c
void sub__sndraw(float left, float right, int32 handle, int32 passed);
```

Queues audio sample for real-time synthesis.

### _SNDRAWLEN

```c
double func__sndrawlen(int32 handle, int32 passed);
```

Returns seconds of queued audio remaining.

### _SNDRAWDONE

```c
void sub__sndrawdone(int32 handle, int32 passed);
```

Signals end of raw audio stream.

### _SNDOPENRAW

```c
int32 func__sndopenraw(int32 passed);
```

Creates handle for raw audio output.

## Legacy Sound (BEEP, SOUND)

### BEEP Statement

```c
void sub_beep();
```

Plays default system beep (platform-specific).

### SOUND Statement

```c
void sub_sound(double frequency, double duration);
```

**Parameters:**
- frequency: Hz (37-32767)
- duration: Timer ticks (18.2 ticks/second)

### Implementation

Uses PSG (Programmable Sound Generator) synthesis:

```c
struct psg_state {
    double frequency;
    double duration_samples;
    double phase;
    int32 active;
};
```

## PLAY Statement

### Syntax

```basic
PLAY "CDEFGAB"                  ' Notes
PLAY "O4 L4 T120"              ' Octave, Length, Tempo
PLAY "MN ML MS"                 ' Music Normal/Legato/Staccato
```

### Command Codes

| Code | Meaning |
|------|---------|
| A-G | Notes (with # or + for sharp, - for flat) |
| O | Octave (0-6) |
| L | Note length (1-64, 1=whole, 4=quarter) |
| T | Tempo (32-255 quarter notes/minute) |
| P/R | Pause/Rest |
| N | Note by number (0-84) |
| MN | Music Normal (7/8 duration) |
| ML | Music Legato (full duration) |
| MS | Music Staccato (3/4 duration) |
| MF | Music Foreground (blocking) |
| MB | Music Background (non-blocking) |
| > | Octave up |
| < | Octave down |

### PLAY Implementation

```c
void sub_play(qbs *str);
```

Parses MML (Music Macro Language) string and generates tones.

## MIDI Support

### _SNDOPEN with MIDI

```c
handle& = _SNDOPEN("file.mid")
```

Uses TinySoundFont for software MIDI synthesis.

### SoundFont Loading

Default soundfont: `internal/support/default_soundfont.sf2`

## Audio Device Management

### Device Enumeration

```c
int32 func__snddevicecount();
qbs *func__snddevice(int32 index);
```

### Device Selection

```c
void sub__snddevice(int32 index);
```

## Audio State

### Global State Variables

```c
ma_engine *audio_engine;         // Main engine
int32 audio_init_attempted;      // Init flag
int32 audio_init_successful;     // Success flag
ma_device_config device_config;  // Device configuration
```

### Initialization

```c
int32 audio_init() {
    ma_engine_config config = ma_engine_config_init();
    config.sampleRate = 44100;
    config.channels = 2;
    return ma_engine_init(&config, &audioEngine);
}
```

## Thread Safety

Audio operations use mutex protection:

```c
pthread_mutex_t audio_mutex;

void audio_lock() {
    pthread_mutex_lock(&audio_mutex);
}

void audio_unlock() {
    pthread_mutex_unlock(&audio_mutex);
}
```

## Error Handling

| Error | Cause |
|-------|-------|
| 5 | Illegal function call |
| 53 | File not found |
| 54 | Bad file mode |
| 258 | Invalid handle |
| 260 | Sound not initialized |

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/parts/audio/miniaudio.h` | miniaudio library |
| `internal/c/parts/audio/audio.cpp` | QB64 audio wrapper |
| `internal/c/parts/audio/extras/` | Format decoders |
| `internal/c/parts/audio/extras/libxmp-lite/` | MOD/XM support |
| `internal/c/libqb.cpp` | BEEP, SOUND, PLAY |

## Example Usage

### Basic Playback

```basic
song& = _SNDOPEN("music.mp3")
_SNDPLAY song&
DO WHILE _SNDPLAYING(song&)
    _LIMIT 60
LOOP
_SNDCLOSE song&
```

### Volume and Position

```basic
snd& = _SNDOPEN("effect.wav")
_SNDVOL snd&, 0.5               ' 50% volume
_SNDSETPOS snd&, 1.5            ' Start at 1.5 seconds
_SNDPLAY snd&
```

### Raw Audio Synthesis

```basic
DIM handle AS LONG
handle = _SNDOPENRAW
FOR t = 0 TO 44100              ' 1 second
    sample! = SIN(t * 440 * _PI / 22050)
    _SNDRAW sample!, sample!, handle
NEXT
DO WHILE _SNDRAWLEN(handle) > 0
    _LIMIT 60
LOOP
_SNDCLOSE handle
```

### PLAY Statement

```basic
PLAY "T120 O4 L4 CDEFGAB>C"     ' C major scale
PLAY "MBT180O3L8CEGCEGCEG"      ' Background arpeggio
```

### Sound Effects with Copy

```basic
shot& = _SNDOPEN("shot.wav")
' Play multiple overlapping shots
FOR i = 1 TO 5
    _SNDPLAYCOPY shot&
    _DELAY 0.1
NEXT
_SNDCLOSE shot&
```
