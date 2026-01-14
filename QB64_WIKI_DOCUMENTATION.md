# QB64-PE Wiki System Documentation

## Overview

This document provides comprehensive information about the QB64-PE (Phoenix Edition) wiki system, which is an integrated help system that downloads and displays wiki content from the QB64-PE online wiki. The system is implemented in BASIC and provides a rich text display with support for various wiki markup elements.

## Architecture

### Core Components

The wiki system consists of two main files:

1. **wiki_global.bas** - Global variables, constants, and data structures
2. **wiki_methods.bas** - Core functionality for downloading, parsing, and rendering wiki content

### Data Structures

#### Help System Variables

```basic
DIM SHARED Cache_Folder AS STRING        ' Cache directory for downloaded pages
DIM SHARED Help_sx, Help_sy, Help_cx, Help_cy  ' Screen coordinates
DIM SHARED Help_Select, Help_cx1, Help_cy1   ' Selection variables
DIM SHARED Help_SelX1, Help_SelX2, Help_SelY1, Help_SelY2
DIM SHARED Help_MSelect                     ' Multi-selection flag
DIM SHARED Help_wx1, Help_wy1, Help_wx2, Help_wy2  ' Window boundaries
DIM SHARED Help_ww, Help_wh                ' Width and height of text region
DIM SHARED help_h, help_w                  ' Help dimensions
DIM SHARED Help_Txt$                      ' Text buffer [chr][col][link-byte1][link-byte2]
DIM SHARED Help_Txt_Len                   ' Length of text buffer
DIM SHARED Help_Pos, Help_Wrap_Pos         ' Current position and wrap position
DIM SHARED Help_Line$                     ' Line index information
DIM SHARED Help_Link$                     ' Link information [sep][type:]...[sep]
DIM SHARED Help_Link_Sep$                 ' Link separator (CHR$(13))
DIM SHARED Help_LinkN, Help_LinkL          ' Number of links, local link flag
```

#### Color Scheme

```basic
DIM SHARED Help_BG_Col                    ' Background color
DIM SHARED Help_Col_Normal: Help_Col_Normal = 7     ' Normal text color (white)
DIM SHARED Help_Col_Link: Help_Col_Link = 9         ' Link color (blue)
DIM SHARED Help_Col_Bold: Help_Col_Bold = 15        ' Bold text color (bright white)
DIM SHARED Help_Col_Italic: Help_Col_Italic = 3      ' Italic text color (cyan)
DIM SHARED Help_Col_Section: Help_Col_Section = 8    ' Section color (gray)
```

#### Text Formatting State

```basic
DIM SHARED Help_Bold, Help_Italic, Help_LinkTxt, Help_Heading
DIM SHARED Help_Underline, Help_ChkBlank
DIM SHARED Help_LockWrap, Help_LockParse
DIM SHARED Help_DList, Help_LIndent$
DIM SHARED Help_Center, Help_CIndent$
```

## Character Encoding Support

### Entity Replacements

The system includes a comprehensive entity replacement table for HTML entities:

```basic
TYPE wikiEntityReplace
    enti AS STRING * 8  ' Entity name (e.g., "&apos;")
    repl AS STRING * 8  ' Replacement character
END TYPE

DIM SHARED wpEntRepl(0 TO 10) AS wikiEntityReplace
```

Supported entities include:
- `&apos;` → `'` (apostrophe)
- `&#91;` → `[` (square bracket open)
- `&#93;` → `]` (square bracket close)
- `&#123;` → `{` (curly bracket open)
- `&#125;` → `}` (curly bracket close)
- `&pi;` → π (pi character)
- `&theta;` → θ (theta character)
- `&nbsp;` → non-breaking space

### UTF-8 Character Support

The system provides extensive UTF-8 character replacement for extended characters:

```basic
TYPE wikiUtf8Replace
    utf8 AS STRING * 4  ' UTF-8 sequence (little-endian)
    repl AS STRING * 8  ' Replacement character/string
END TYPE

DIM SHARED wpUtfRepl(0 TO 65) AS wikiUtf8Replace
```

Supported UTF-8 characters include:

#### 2-byte sequences:
- Pilcrow (¶), Section (§), Copyright (©)
- Various accented characters (á, â, ã, ä, å, æ, ç, è, ê, ë, í, ï, ñ, ó, ô, õ, ö)
- Currency symbols (¢, ¼, ¾)
- Mathematical symbols (π)

#### 3-byte sequences:
- Box drawing characters (corners, lines, crosses)
- Arrows (up, down, left, right, double-headed)
- Geometric shapes (triangles, circles, bullets)
- Card suits (hearts, diamonds, clubs, spades)
- Musical notes
- Special symbols (faces, gender signs, sun, rectangle)

#### 4-byte sequences:
- Emoticons (smiley `:)`, wink `;)`)

## Wiki Markup Support

### Text Formatting

| Markup | Description | Implementation |
|--------|-------------|----------------|
| `'''text'''` | Bold text | Toggles `Help_Bold` flag |
| `''text''` | Italic text | Toggles `Help_Italic` flag |
| `{{InlineCode}}text{{InlineCodeEnd}}` | Inline code block | Sets background color, locks parsing |
| `{{CodeStart}}text{{CodeEnd}}` | Code block | Creates bordered block with header |
| `{{OutputStart}}text{{OutputEnd}}` | Output block | Creates bordered block for program output |
| `{{TextStart}}text{{TextEnd}}` | Text block | Creates bordered block for notes |
| `{{PreStart}}text{{PreEnd}}` | Preformatted text | Preserves formatting with indentation |
| `{{FixedStart}}text{{FixedEnd}}` | Fixed block | Similar to pre but with border |

### Links

| Type | Syntax | Description |
|------|--------|-------------|
| Internal | `[[PageName]]` | Link to another wiki page |
| Internal with text | `[[PageName|Display Text]]` | Link with alternative display text |
| External | `[http://example.com Text]` | External HTTP link |
| Wikipedia | `[[Wikipedia:Article]]` | Link to Wikipedia article |
| Local anchor | `[[PageName#section]]` | Link to section within page |
| Template links | `{{Cb|Page}}`, `{{Cl|Page}}`, `{{Cm|Page}}` | Special wrapped links |

### Section Headings

| Markup | Level | Display |
|--------|-------|---------|
| `== Section ==` | H2 | Double underline, section color |
| `=== Section ===` | H3 | Single underline, section color |
| `==== Section ====` | H4 | No underline, section color |

### Lists

| Type | Syntax | Description |
|------|--------|-------------|
| Unordered | `* Item` | Bullet list |
| Unordered (nested) | `** Item` | Indented bullet list |
| Ordered | `# Item` | Numbered list |
| Ordered (nested) | `## Item` | Indented numbered list |
| Definition | `;Term: Description` | Definition list with bold term |
| Description only | `: Description` | Indented description |

### Other Elements

| Element | Syntax | Description |
|---------|--------|-------------|
| Horizontal rule | `----` or `<hr>` | Horizontal line |
| Centered text | `<center>text</center>` | Centered alignment |
| Gallery | `<gallery>...</gallery>` | Special image gallery handling |
| Comments | `<!-- comment -->` | HTML comments (ignored) |
| Line break | `<br>` or `<br />` | HTML line break |

## Template System

### Page Structure Templates

- `{{PageSyntax}}` - "Syntax:" section header
- `{{PageParameters}}` - "Parameters:" section header
- `{{PageDescription}}` - "Description:" section header
- `{{PageAvailability}}` - "Availability:" section header
- `{{PageExamples}}` - "Examples:" section header
- `{{PageSeeAlso}}` - "See also:" section header

### Special Templates

- `{{Parameter}}text` - Italic parameter text
- `{{Small}}text` - Small/centered text
- `{{PageNavigation}}` - Page navigation plugin
- `{{PageReferences}}` - Page references plugin
- `{{*Plugin}}` - Various plugin templates

### Internal Templates

- `{{PageInternalError}}` - Error message display
- `{{QBDLDATE:date}}` - Download date stamp
- `{{QBDLTIME:time}}` - Download time stamp
- `{{DISPLAYTITLE:title}}` - Custom page title

## Download and Caching System

### Cache Management

```basic
DIM SHARED Cache_Folder$ = "internal\help"  ' Default cache location
DIM SHARED Help_Recaching    ' Update mode flag
DIM SHARED Help_IgnoreCache  ' Temporary cache bypass flag
```

Notes:
- On Windows the cache folder is initialized as `internal\help`.
- On non-Windows the cache folder is initialized as `internal/help`.

### Cache Modes

- **Help_Recaching = 0**: Normal operation, use cache if available
- **Help_Recaching = 1**: Update all pages with GUI progress
- **Help_Recaching = 2**: Silent update for CI/build systems
- **Help_IgnoreCache = 1**: Temporarily bypass cache

In practice:
- **Update Current Page** sets `Help_IgnoreCache = 1`, re-downloads just the currently viewed page (`Back$(Help_Back_Pos)`), parses it again, then resets `Help_IgnoreCache = 0`.
- **Update All Pages...** sets `Help_Recaching = 1` and `Help_IgnoreCache = 1`, runs `ideupdatehelpbox`, then resets both flags back to `0`.

### Download Process

1. **URL Construction**: `wikiBaseAddress$ + "/index.php?title=" + PageName$ + "&action=edit"`
2. **Content Extraction**: Extract wikitext between `<textarea>` tags
3. **Entity Processing**: Convert HTML entities to characters
4. **Local Storage**: Save processed content to cache folder
5. **Error Handling**: Provide fallback messages for missing pages

### Security Features

- **HTTPS Support**: Primary secure connection method
- **HTTP Fallback**: Option for systems with outdated CA stores
- **External Curl**: Fallback to system curl tool
- **Timeout Handling**: Configurable download timeout
- **Error Recovery**: Graceful handling of network failures

## Navigation and History

### Back Navigation

```basic
TYPE Help_Back_Type
    sx AS LONG  ' Scroll X position
    sy AS LONG  ' Scroll Y position
    cx AS LONG  ' Cursor X position
    cy AS LONG  ' Cursor Y position
END TYPE

REDIM SHARED Help_Back(1) AS Help_Back_Type
DIM SHARED Back$(1)        ' Page names
DIM SHARED Back_Name$(1)   ' Display names
DIM SHARED Help_Back_Pos   ' Current position in history
```

### Page Name Abbreviation

The `Back2BackName$` function provides abbreviated names for display:

| Full Name | Abbreviation |
|-----------|--------------|
| "Base Comparisons" | "Base Compare" |
| "Bitwise Operators" | "Bitwise OPs" |
| "Downloading Files" | "Downloads" |
| "Function (explanatory)" | "FUNC expl." |
| "Keyboard scancodes" | "KB Scancodes" |
| "Mathematical Operations" | "Math OPs" |
| "Relational Operations" | "Relational OPs" |
| "Windows Environment" | "Win Env." |
| "Windows Libraries" | "Win Lib." |
| "Windows Registry Access" | "Win Reg." |

## Text Rendering Engine

### Word Wrapping

The system implements intelligent word wrapping:
- Wraps at spaces when possible
- Backtracks to last wrap position for long words
- Maintains link and formatting information across line breaks
- Handles different indentation levels for lists and blocks

### Color Management

- **Dynamic color calculation** based on current formatting state
- **Background colors** for different block types (code, output, text)
- **Link colors** with contrast adjustment for different backgrounds
- **Section colors** for headings and structural elements

### Link Processing

- **Link type detection**: PAGE, EXTL, SECT
- **Local anchor handling**: #toc and #top redirection
- **Wikipedia expansion**: Automatic external link generation
- **Category filtering**: Ignore category links
- **Link numbering**: Sequential assignment for click handling

## Special Features

### Search Integration

The system generates a `links.bin` file for the alphabetical keyword reference:
- Extracts all internal links from the keyword reference page
- Filters out non-keyword entries (GL functions, etc.)
- Creates comma-separated keyword-page pairs for search functionality

### Error Handling

- **Network timeouts**: Graceful fallback with user notification
- **Missing pages**: Informative error messages with suggestions
- **Encoding issues**: Warning messages for unknown entities/UTF-8 characters
- **Parse errors**: Robust error recovery during wiki markup processing

### Performance Optimizations

- **Prefetch array**: Look-ahead buffer for efficient token detection
- **String manipulation**: Optimized string processing for large documents
- **Memory management**: Dynamic buffer sizing for text content
- **Caching**: Local storage to minimize network requests

## Configuration

### Wiki Base Address

The wiki server URL is configurable:
```basic
DIM SHARED wikiBaseAddress$  ' Default: "https://qb64phoenix.com/qb64wiki"
```

The value is stored in the IDE config under:
- **Section**: `generalSettingsSection$`
- **Key**: `WikiBaseAddress`

### Display Settings

- **Help_ww**: Text width in characters
- **Help_wh**: Text height in lines
- **Color scheme**: Customizable through global variables

## Integration Points

### IDE Integration

The wiki system integrates with the QB64-PE IDE through:
- **Status bar updates** during downloads
- **Menu commands** for help navigation
- **Keyboard shortcuts** for help access
- **Window management** for help display

The main IDE entry points (Help menu) are:
- **View (Shift+F1)**: Opens the help window (it does not force navigation to a specific page by itself).
- **Contents Page**: Opens the `QB64 Help Menu` page.
- **Keywords Index**: Opens `Keyword Reference - Alphabetical`.
- **Keywords by Usage**: Opens `Keyword Reference - By usage`.
- **Metacommands**: Opens `Metacommand`.
- **Variable Types**: Opens `Variable Types`.
- **Update Current Page**: Re-downloads the current page and reparses it.
- **Update All Pages...**: Runs the full update routine (can take up to ~10 minutes).
- **View Current Page On Wiki**: Opens `wikiBaseAddress$ + "/index.php?title=" + Back$(Help_Back_Pos)` in the system browser.

### Build System Integration

The system supports command-line updates:
```bash
qb64pe -u  # Update wiki cache during build
```

Notes:
- `-u` is implemented as a hidden CI/build option.
- It sets `Help_Recaching = 2` and `Help_IgnoreCache = 1`, runs `ideupdatehelpbox`, prints an error to console and exits with code `1` on failure, otherwise exits normally.
- In `Help_Recaching = 2` mode, `ideupdatehelpbox` skips scanning the existing cache directory and runs its update routine non-interactively.

## File Structure

```
source/ide/wiki/
├── wiki_global.bas    # Global variables and data
└── wiki_methods.bas    # Core functionality

internal/help/          # Cache directory
├── *.txt             # Downloaded wiki pages
└── links.bin         # Search index file
```

## Development Notes

### Character Encoding

All UTF-8 values in replacement tables must be reversed (little-endian) for proper matching with the `MKI$` and `MKL$` functions.

### Parser States

The parser uses a state machine with multiple lock levels:
- **Help_LockParse = 0**: Normal parsing
- **Help_LockParse = 1**: Output block (soft lock)
- **Help_LockParse = 2**: Code block (hard lock)
- **Help_LockParse = -1**: Text block (soft lock)
- **Help_LockParse = -2**: Pre/Fixed block (soft lock)

### Memory Management

The system uses dynamic memory allocation for:
- Text content buffer (up to 1MB)
- Line index array (dynamic sizing)
- Link information array (dynamic sizing)
- History stack (dynamic sizing)

## Conclusion

The QB64-PE wiki system is a sophisticated help system that provides:
- Rich text rendering with wiki markup support
- Comprehensive character encoding handling
- Efficient caching and download management
- Robust error handling and recovery
- Seamless IDE integration
- Extensible template system

This documentation reflects the current implementation as found in the source code and provides a complete reference for understanding and maintaining the wiki system.
