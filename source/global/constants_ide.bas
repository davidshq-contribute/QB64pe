' IDE Constants
' This file contains all IDE-related constant declarations
' to avoid SUB/FUNCTION boundary issues in main files

'setup optional codepages
CONST idecpnum& = 27

'Dependency management constants
CONST DEPENDENCY_LOADFONT = 1
CONST DEPENDENCY_MINIAUDIO = 2
CONST DEPENDENCY_GL = 3
CONST DEPENDENCY_IMAGE_CODEC = 4
CONST DEPENDENCY_CONSOLE_ONLY = 5
CONST DEPENDENCY_SOCKETS = 6
CONST DEPENDENCY_PRINTER = 7
CONST DEPENDENCY_ICON = 8
CONST DEPENDENCY_SCREENIMAGE = 9
CONST DEPENDENCY_DEVICEINPUT = 10
CONST DEPENDENCY_ZLIB = 11
CONST DEPENDENCY_EMBED = 12

'$EMBED metacommand constants
CONST eflLine = 0
CONST eflUsed = 1
CONST eflFile = 2
CONST eflHand = 3

'$USELIBRARY metacommand constants
CONST ullName = 0
CONST ullNeedy = 1
CONST ullTop = 2
CONST ullMain = 3
CONST ullBottom = 4

'IDE TYPE definitions moved to qb64pe.bas
