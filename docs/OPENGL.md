# OPENGL.md

Documentation of the QB64pe OpenGL integration system.

## Overview

QB64pe provides direct OpenGL access through the `_GL` SUB mechanism, allowing hardware-accelerated 3D graphics. The compiler parses OpenGL headers to generate function bindings, enabling use of standard GL functions from BASIC code.

## _GL SUB

```basic
SUB _GL
    ' OpenGL rendering code here
    ' Called automatically each frame
END SUB
```

### Invocation

The `_GL` SUB is called automatically by the display system:

```c
// internal/c/libqb.cpp
void sub__glrender(int32 mode) {
    if (mode == 1) {  // _BEHIND
        // Render GL before software rendering
    } else {          // _ONTOP (default)
        // Render GL after software rendering
    }
}
```

## _GLRENDER Statement

```basic
_GLRENDER _BEHIND    ' GL renders behind software graphics
_GLRENDER _ONTOP     ' GL renders on top (default)
_GLRENDER _ONLY      ' GL only, no software rendering
```

## GL Function Binding

### Header Scanning

The compiler scans `internal/c/parts/core/gl_header_for_parsing/gl.h` to generate bindings:

```basic
' In source/qb64pe.bas
SUB gl_scan_header
    ' Parse gl.h for function declarations
    ' Generate DECLARE statements
    ' Map GL types to QB64 types
END SUB
```

### Type Mapping

| GL Type | QB64 Type |
|---------|-----------|
| `GLint` | `LONG` |
| `GLuint` | `_UNSIGNED LONG` |
| `GLfloat` | `SINGLE` |
| `GLdouble` | `DOUBLE` |
| `GLboolean` | `_BYTE` |
| `GLenum` | `_UNSIGNED LONG` |
| `GLsizei` | `LONG` |
| `GLvoid*` | `_OFFSET` |

### Generated Declarations

```basic
' Auto-generated from gl.h
DECLARE LIBRARY
    SUB _glClear ALIAS "glClear" (BYVAL mask AS _UNSIGNED LONG)
    SUB _glClearColor ALIAS "glClearColor" (BYVAL r AS SINGLE, ...)
    SUB _glBegin ALIAS "glBegin" (BYVAL mode AS _UNSIGNED LONG)
    SUB _glEnd ALIAS "glEnd" ()
    SUB _glVertex3f ALIAS "glVertex3f" (BYVAL x!, BYVAL y!, BYVAL z!)
    ' ... hundreds more
END DECLARE
```

## GL Constants

OpenGL constants are available with `_GL` prefix:

```basic
_glClear _GL_COLOR_BUFFER_BIT OR _GL_DEPTH_BUFFER_BIT
_glBegin _GL_TRIANGLES
_glEnable _GL_DEPTH_TEST
_glBlendFunc _GL_SRC_ALPHA, _GL_ONE_MINUS_SRC_ALPHA
```

## Display Synchronization

### Software to GL Coordinates

```basic
' Software screen (0,0 top-left)
' GL coordinates (-1,-1 bottom-left to 1,1 top-right)

' Convert screen coords to GL
glX! = (screenX / _WIDTH) * 2 - 1
glY! = 1 - (screenY / _HEIGHT) * 2
```

### Frame Timing

```basic
_DISPLAY         ' Sync software display
' _GL SUB called after _DISPLAY
```

## Texture Loading

```basic
' Load image as texture
img& = _LOADIMAGE("texture.png", 32)
texID~& = 0
_glGenTextures 1, _OFFSET(texID~&)
_glBindTexture _GL_TEXTURE_2D, texID~&

' Get image data
DIM m AS _MEM
m = _MEMIMAGE(img&)
_glTexImage2D _GL_TEXTURE_2D, 0, _GL_RGBA, _WIDTH(img&), _HEIGHT(img&), _
              0, _GL_BGRA_EXT, _GL_UNSIGNED_BYTE, m.OFFSET
_MEMFREE m
_FREEIMAGE img&
```

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | GL header scanning, _GL SUB parsing |
| `internal/c/parts/core/gl_header_for_parsing/gl.h` | GL function declarations |
| `internal/c/libqb.cpp` | `sub__glrender` implementation |
| `internal/c/parts/core/src/freeglut_*.c` | FreeGLUT windowing |
| `internal/c/parts/core/src/glew.c` | GL extension wrangling |

## Dependencies

| Component | Purpose |
|-----------|---------|
| FreeGLUT | Window/context management |
| GLEW | OpenGL extension loading |
| OpenGL | System graphics driver |

## Example Usage

### Basic 3D Triangle

```basic
SCREEN _NEWIMAGE(800, 600, 32)

DO
    _LIMIT 60
LOOP

SUB _GL
    STATIC angle!

    _glClear _GL_COLOR_BUFFER_BIT OR _GL_DEPTH_BUFFER_BIT

    _glMatrixMode _GL_MODELVIEW
    _glLoadIdentity
    _glTranslatef 0, 0, -3
    _glRotatef angle!, 0, 1, 0

    _glBegin _GL_TRIANGLES
        _glColor3f 1, 0, 0: _glVertex3f 0, 1, 0
        _glColor3f 0, 1, 0: _glVertex3f -1, -1, 0
        _glColor3f 0, 0, 1: _glVertex3f 1, -1, 0
    _glEnd

    angle! = angle! + 1
END SUB
```

### Mixed Software and GL

```basic
SCREEN _NEWIMAGE(800, 600, 32)
_GLRENDER _BEHIND  ' GL behind software

DO
    CLS
    PRINT "Software text on top"
    _DISPLAY
    _LIMIT 60
LOOP

SUB _GL
    ' GL renders behind the text
    _glClearColor 0.2, 0.2, 0.4, 1
    _glClear _GL_COLOR_BUFFER_BIT
END SUB
```

### Perspective Setup

```basic
SUB _GL
    STATIC setup AS _BYTE
    IF NOT setup THEN
        setup = -1
        _glMatrixMode _GL_PROJECTION
        _glLoadIdentity
        aspect! = _WIDTH / _HEIGHT
        gluPerspective 60, aspect!, 0.1, 100
        _glMatrixMode _GL_MODELVIEW
        _glEnable _GL_DEPTH_TEST
    END IF

    ' Render scene...
END SUB
```

## Limitations

- GL context managed by QB64pe (no direct context creation)
- Single GL context per program
- `_GL` SUB cannot use PRINT or other software graphics
- Must use `_OFFSET()` for pointer parameters
