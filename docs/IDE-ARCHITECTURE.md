# IDE-ARCHITECTURE.md

Detailed architecture of the QB64pe Integrated Development Environment.

## Overview

The QB64pe IDE is a ~21,800 line QB64 application providing a complete development environment with syntax highlighting, debugging, help integration, and code export capabilities.

## Core Files

| File | Lines | Purpose |
|------|-------|---------|
| `source/ide/ide_methods.bas` | 20,885 | Main IDE implementation |
| `source/ide/ide_global.bas` | 243 | Global variable declarations |
| `source/ide/ide_export.bas` | 663 | Code export functionality |
| `source/ide/config/cfg_methods.bas` | 651 | Configuration management |
| `source/ide/config/cfg_global.bas` | 75 | Configuration variables |
| `source/ide/wiki/wiki_methods.bas` | 1,357 | Help system |
| `source/ide/wiki/wiki_global.bas` | 156 | Wiki variables |

## Main Event Loop (ide2)

The `ide2()` function is the heart of the IDE, containing ~4,500 lines of event handling code.

### Loop Structure

```
ide2() FUNCTION
├── Initialization Phase (idelaunched = 0)
│   ├── Menu system setup (12 menus)
│   ├── Window dimensions and display
│   ├── Font configuration
│   ├── Codepage initialization (27 codepages)
│   └── Autosave recovery
│
├── Main Event Loop (ideloop:)
│   ├── Resize handling (75+ lines)
│   ├── Input processing via GetInput
│   ├── Text editor operations
│   ├── Menu interaction
│   ├── Help system rendering
│   ├── Error/warning display
│   └── Screen update via ideshowtext
│
├── Menu System (startmenu:)
│   ├── Menu bar rendering
│   ├── Menu navigation
│   └── Submenu display (showmenu:)
│
└── Exit Handling (ideexit flag)
```

### Event Processing Sequence

1. Check for compiler commands (`idecommand$`)
2. Handle resize events (1.5-second debounce)
3. Call `GetInput` for keyboard/mouse input
4. Process text editor operations
5. Check menu activation
6. Render screen via `ideshowtext`
7. Display help window (if active)
8. Update status bar
9. Copy screen pages (`PCOPY 3, 0`)

## Screen Buffer System

The IDE uses multiple screen pages for efficient rendering:

| Page | Purpose |
|------|---------|
| 0 | Display output |
| 1 | Working buffer |
| 2 | Dialog/backup buffer |
| 3 | Main edit buffer (virtual screen) |

### Display Layout

```
Row 1:          Menu bar (COLOR 0,7)
Row 2:          Editor top border
Rows 3-N:       Editor area (idewy-5 rows)
Row N+1:        Editor bottom border
Row N+2:        Search bar (if IdeSystem=2)
Row N+3:        Status area (COLOR 0,3)
Row N+4+:       Help window (if idehelp=1)
```

## Syntax Highlighting

### Color Scheme

| Color Index | Variable | Purpose |
|-------------|----------|---------|
| 8 | IDENumbersColor | Numbers, hex, binary |
| 10 | IDEMetaCommandColor | $IF, $INCLUDE, etc. |
| 11 | IDECommentColor | REM and ' comments |
| 12 | IDEKeywordColor | QB64 keywords |
| 13 | IDETextColor | Regular text |
| 14 | IDEQuoteColor | String literals |
| 5 | IDEBracketHighlightColor | Matching brackets |
| 6 | IDEBackgroundColor2 | Line number background |

### Highlighting Pipeline

The `ideshowtext` SUB (~700 lines) processes each line:

1. **Quote/Comment Detection** via `FindQuoteComment`
   - Track opening/closing quotes
   - Identify REM and `'` markers
   - Handle line continuations (`_`)

2. **Keyword Recognition**
   - Built-in QB64 keyword database
   - Custom keyword dictionary support

3. **Special Token Handling**
   - Numbers (decimal, &H hex, &O octal, &B binary)
   - Metacommands ($IF, $ELSE, etc.)
   - String escapes

4. **Bracket Matching**
   - Find matching parentheses/brackets
   - Highlight both with color 5

5. **Selection Highlighting**
   - Multi-selection: highlight all matching words
   - User selection: inverted colors

## Configuration System

### File Locations

All configuration stored in `./settings/` relative to QB64pe:

| File | Purpose |
|------|---------|
| `config.ini` | Main settings |
| `debug.ini` | Debug session data |
| `bookmarks.bin` | Bookmark positions |
| `recent.bin` | Recent file list |
| `searched.bin` | Search history |
| `autosave*.bin` | Per-instance autosave |
| `undo*.bin` | Per-instance undo/redo |

### Configuration Sections

**[IDE WINDOW]**
- IDE_Width, IDE_Height
- IDE_X, IDE_Y (window position)

**[IDE COLOR SETTINGS]**
- All syntax highlighting colors
- 14+ built-in color schemes

**[GENERAL SETTINGS]**
- DisableSyntaxHighlighter
- PasteCursorAtEnd
- AutoCloseBrackets
- SaveExeWithSource
- DefaultExeSaveFolder

**[IDE DISPLAY SETTINGS]**
- IDEAutoLayout, IDEAutoIndent
- IDECustomFont, IDECustomFontFile$
- ShowLineNumbers, BracketHighlight
- MultiHighlight, KeywordHighlight

**[COMPILER SETTINGS]**
- MaxParallelProcesses
- ExtraCppFlags$, ExtraLinkerFlags$
- OptimizeCppProgram
- StripDebugSymbols

### Key Functions

| Function | Purpose |
|----------|---------|
| `ReadInitialConfig` | Load settings on startup |
| `ReadConfigSetting` | Read individual setting |
| `WriteConfigSetting` | Write/update setting |
| `CopyFromOther` | Import from another installation |

## Help System (Wiki)

### Architecture

The help system caches wiki pages locally and renders them in-IDE.

**Cache Location:** `internal/help/`
**Wiki URL:** `https://qb64phoenix.com/` (configurable via `wikiBaseAddress$`)

### Markup Support

```
[[page]]              - Internal wiki link
[[page|text]]         - Link with custom text
[http://...]          - External URL
'''bold'''            - Bold text
''italic''            - Italic text
= Heading =           - Section heading
* Bullet              - Bullet list
# Number              - Numbered list
<source>code</source> - Code block
{{template}}          - Template expansion
{| table |}           - Table markup
```

### Navigation

- Back history: `Back$()` array stores page stack
- 6 recent links shown in `IdeRecentLink` array
- Search: `IdeSystem = 3` activates help search mode
- Tab key cycles through search matches

## Export Functionality

The `ExportCodeAs(docFormat$)` function supports multiple formats:

### Supported Formats

| Format | Output | Colors |
|--------|--------|--------|
| HTML (.htm) | File | IDE colors |
| RTF (.rtf) | File | IDE colors |
| Discord | Clipboard | Fixed blue |
| Forum BBCode | Clipboard | Fixed blue |
| Wiki | Clipboard | Fixed blue |

### Processing Pipeline

**Pass 1: Source Preparation**
- Get selected text or entire document
- Normalize line endings

**Pass 2: Token-by-Token Parsing**
- Character-by-character analysis
- Quote/comment/metacommand detection
- Keyword identification
- Wiki link generation for keywords

### Processing Flags

| Flag | Purpose |
|------|---------|
| `co%` | Comment processing |
| `me%` | Metacommand processing |
| `kw%` | Keyword processing |
| `nu%` | Number processing |
| `qu%` | Quote processing |
| `fu%` | Function identifier |
| `bo%` | Boolean operator |

## IDE/Compiler Communication

### Command Protocol

The IDE and compiler communicate via `idecommand$` and `idereturn$`:

**Compiler → IDE:**

| Code | Purpose |
|------|---------|
| 1 | Load file (startup) |
| 3 | Request next source line |
| 6 | Compilation OK |
| 7 | Rewind for repass |
| 8 | Error occurred |
| 10 | Request specific line |
| 11 | ".EXE file created" message |
| 12 | Report exe name |
| 100 | Simplified next line request |
| 254 | Launch debug interface |
| 255 | QB error in IDE |

**IDE → Compiler:**

| Code | Purpose |
|------|---------|
| 2 | Begin new compilation |
| 4 | Return next line |
| 5 | No more lines |
| 9 | C++ compile and run |

### Compilation Flow

1. IDE tracks `idecompiledline` (current line to send)
2. Compiler calls `ide(0)` with code 3
3. IDE returns formatted line in `idereturn$`
4. IDE applies auto-indentation if enabled
5. Progress shown: `INT((idecompiledline * 100) / (iden * 2))`
6. On error: IDE highlights error line, shows message

## Key Global Variables

### Editor State

| Variable | Purpose |
|----------|---------|
| `idet$(1..N)` | Array of text lines |
| `idel` | Current display line |
| `iden` | Total line count |
| `idesx, idesy` | Scroll position |
| `idecx, idecy` | Cursor position |
| `ideselect` | Selection active flag |
| `ideselectx1, ideselecty1` | Selection start |

### Undo/Redo

| Variable | Purpose |
|----------|---------|
| `ideundopos` | Current undo position |
| `ideundobase` | Base undo position |
| `ideundoflag` | Undo/redo mode |
| `ideundocombo` | Combine consecutive actions |

### File Management

| Variable | Purpose |
|----------|---------|
| `ideprogname$` | Current filename |
| `idepath$` | Working directory |
| `ideroot$` | QB64pe installation root |
| `ideunsaved` | Unsaved changes flag |
| `idecompiling` | Compilation in progress |

### Input State

| Variable | Purpose |
|----------|---------|
| `KB` | _KEYHIT value |
| `K$` | INKEY$ equivalent |
| `KSHIFT, KCTRL, KALT` | Modifier states |
| `mX, mY` | Mouse position |
| `mB, mB2` | Mouse button states |
| `mWHEEL` | Mouse wheel delta |

### Debug Integration

| Variable | Purpose |
|----------|---------|
| `IdeDebugMode` | Debugger active |
| `debugnextline` | Next line to execute |
| `idefocusline` | Error line to highlight |
| `host&, debugClient&` | TCP connections |
| `variableWatchList$` | Watch expressions |
| `callStackLength` | Call stack depth |

## Codepage Support

The IDE supports 27 codepages for international character sets:

- `idecpindex` - Active codepage index (0-27)
- `idecpname$(1..27)` - Codepage names
- `idecp$(1..27)` - Mapping tables (hex encoded)

## Menu System

| Variable | Purpose |
|----------|---------|
| `menu$(1..12, 0..20)` | Menu item text |
| `menuDesc$(12, 20)` | Menu tooltips |
| `menusize(1..12)` | Items per menu |
| `menus` | Total menu count |
| `menubar$` | Combined menu bar string |
| `MenuLocations` | Position indices |

## Performance Considerations

1. **Selective Rendering**: Only redraw changed portions
2. **Screen Page Buffering**: Work on page 3, copy to page 0
3. **Debounced Resize**: 1.5-second delay prevents rapid redraws
4. **Lazy Syntax Highlighting**: Only visible lines processed
5. **Cached Wiki Pages**: Avoid repeated downloads
