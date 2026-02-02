# SOUND-LEGACY.md

Documentation of the QB64pe legacy sound system (BEEP, SOUND, PLAY).

## Overview

QB64pe provides legacy QBasic sound compatibility through BEEP, SOUND, and PLAY statements, using synthesized waveforms via the miniaudio PSG (Programmable Sound Generator).

## BEEP Statement

```basic
BEEP
```

Produces a simple system beep.

### Implementation

- Delegates to `tinyfd_beep()` (TinyFileDialogs)
- Uses native OS beep mechanism
- No parameters, fixed tone

## SOUND Statement

```basic
SOUND frequency!, duration!
SOUND frequency!, duration!, volume!, pan!, waveform%, param!, voice%, option%
```

### Parameters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| frequency | 20-32767 Hz | 440 | Tone frequency |
| duration | clock ticks | - | Length (18.2 ticks/sec) |
| volume | 0.0-1.0 | 0.5 | Volume level |
| pan | -1.0 to 1.0 | 0 | Stereo position |
| waveform | 1-9 | 1 | Waveform type |
| param | 0.0-1.0 | 0.5 | Pulse duty cycle |
| voice | 0-3 | 0 | Voice channel |
| option | 1-2 | - | Wait/Resume |

### Waveform Types

| Value | Type | Description |
|-------|------|-------------|
| 1 | SQUARE | Classic PC speaker |
| 2 | SAWTOOTH | Bright, buzzy |
| 3 | TRIANGLE | Soft, mellow |
| 4 | SINE | Pure tone |
| 5 | WHITE NOISE | All frequencies |
| 6 | PINK NOISE | Lower emphasis |
| 7 | BROWN NOISE | Even lower |
| 8 | LFSR NOISE | Retro game noise |
| 9 | PULSE | Variable duty cycle |

### Examples

```basic
' Simple beep (440 Hz, 1 second)
SOUND 440, 18.2

' Low bass note
SOUND 100, 36.4

' White noise burst
SOUND 1000, 18.2, 0.5, 0, 5

' Stereo sweep
FOR f = 200 TO 800 STEP 50
    SOUND f, 2, 0.3, (f - 500) / 300
NEXT
```

## PLAY Statement

```basic
PLAY commandString$
PLAY voice1$, voice2$, voice3$, voice4$
```

### Music Macro Language (MML)

#### Notes

| Command | Description |
|---------|-------------|
| A-G | Note names |
| # or + | Sharp |
| - | Flat |
| . | Dotted note (1.5x duration) |

#### Timing

| Command | Range | Description |
|---------|-------|-------------|
| L | 1-64 | Note length (1=whole, 4=quarter) |
| T | 32-255 | Tempo in BPM (default 120) |
| P or R | - | Rest/pause |

#### Octave

| Command | Description |
|---------|-------------|
| O | 0-7 | Set octave (default 4) |
| < | Octave down |
| > | Octave up |

#### Articulation

| Command | Description |
|---------|-------------|
| ML | Legato (notes connected) |
| MN | Normal (7/8 duration) |
| MS | Staccato (3/4 duration) |

#### Mode

| Command | Description |
|---------|-------------|
| MF | Foreground (wait for completion) |
| MB | Background (continue execution) |

#### Volume and Effects

| Command | Range | Description |
|---------|-------|-------------|
| V | 0-100 | Volume percentage |
| @ or W | 1-9 | Waveform type |
| S | 0-100 | Pan (0=left, 50=center, 100=right) |
| Q | 0-100 | Volume envelope |

#### ADSR Envelope

| Command | Range | Description |
|---------|-------|-------------|
| / | 0-100 | Attack time % |
| \ | 0-100 | Decay time % |
| ^ | 0-100 | Sustain level % |
| _ | 0-100 | Release time % |

#### Advanced

| Command | Description |
|---------|-------------|
| N | 0-84 | Direct note number |
| X | VARPTR$ | Include substring |
| = | VARPTR$ | Numeric variable |
| Y | 0-100 | Waveform parameter |

### Examples

```basic
' Simple melody
PLAY "C D E F G A B > C"

' With tempo and octave
PLAY "T120 O4 L4 C E G > C"

' Background music
PLAY "MB T140 L8 C D E F G A B > C"
PRINT "Music playing in background"

' Chords (multi-voice)
PLAY "O3 C", "O3 E", "O3 G"

' Staccato with volume
PLAY "MS V50 L16 C C C C"
```

## MB (Background) vs MF (Foreground)

### Foreground (MF) - Default

```basic
PLAY "MF C D E F"   ' Program waits
PRINT "After music" ' Prints when done
```

- Program execution pauses
- Waits for notes to complete
- Traditional sequential playback

### Background (MB)

```basic
PLAY "MB C D E F"   ' Returns immediately
PRINT "During music" ' Prints while playing
```

- Control returns immediately
- Music plays concurrently
- Enables game music loops

## Comparison: Legacy vs Modern Audio

| Feature | SOUND/PLAY | _SNDOPEN |
|---------|------------|----------|
| Source | Synthesized | File-based |
| Formats | Waveforms | WAV, MP3, OGG, etc. |
| Control | Frequency/duration | Position/loop |
| Voices | 4 | Unlimited |
| MML | Yes | No |
| Real-time | Yes | Pre-loaded |

## PSG Architecture

The Programmable Sound Generator provides:

- 4 simultaneous voices
- Real-time waveform synthesis
- ADSR envelope support
- Stereo panning
- Multiple noise types

### Voice Channels

```basic
' Use specific voice
SOUND 440, 18, , , , , 0   ' Voice 0
SOUND 550, 18, , , , , 1   ' Voice 1 (simultaneous)
```

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/parts/gui/gui.cpp` | BEEP (line 337) |
| `internal/c/parts/audio/audio.cpp` | SOUND (2766-2800) |
| `internal/c/parts/audio/audio.cpp` | PLAY (1415-2142) |
| `internal/c/libqb/include/audio.h` | Declarations |

## Example Usage

### Classic PC Speaker Sound

```basic
' Ascending tones
FOR f = 100 TO 1000 STEP 100
    SOUND f, 2
NEXT
```

### Game Sound Effects

```basic
SUB Explosion
    FOR i = 500 TO 50 STEP -25
        SOUND i, 0.5, 0.8, 0, 5  ' White noise descending
    NEXT
END SUB

SUB Laser
    FOR i = 2000 TO 500 STEP -100
        SOUND i, 0.2, 0.5, 0, 1  ' Square wave sweep
    NEXT
END SUB
```

### Background Music Loop

```basic
music$ = "MB T120 L8 O4 "
music$ = music$ + "C E G > C < B G E C "
music$ = music$ + "D F A > D < C A F D "

DO
    PLAY music$
    ' Game logic here
    _LIMIT 60
LOOP
```

### Multi-Voice Harmony

```basic
' Play a C major chord
PLAY "O3 L1 C", "O3 L1 E", "O3 L1 G", "O4 L1 C"
```

### ADSR Envelope

```basic
' Plucky sound (fast attack, quick decay)
PLAY "/10 \30 ^20 _40 C D E F G"

' Pad sound (slow attack, long sustain)
PLAY "/50 \10 ^80 _30 L1 C"
```
