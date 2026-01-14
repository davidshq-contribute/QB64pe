
'$INCLUDEONCE

'hash table data
TYPE HashListItem
    Flags AS LONG
    Reference AS LONG
    NextItem AS LONG
    PrevItem AS LONG
    LastItem AS LONG 'note: this value is only valid on the first item in the list
    'note: name is stored in a separate array of strings
END TYPE

' Hash flag constants (must be declared before executable code)
CONST HASHFLAG_LABEL = 2
CONST HASHFLAG_TYPE = 4
CONST HASHFLAG_RESERVED = 8
CONST HASHFLAG_OPERATOR = 16
CONST HASHFLAG_CUSTOMSYNTAX = 32
CONST HASHFLAG_SUB = 64
CONST HASHFLAG_FUNCTION = 128
CONST HASHFLAG_UDT = 256
CONST HASHFLAG_UDTELEMENT = 512
CONST HASHFLAG_CONSTANT = 1024
CONST HASHFLAG_VARIABLE = 2048
CONST HASHFLAG_ARRAY = 4096
CONST HASHFLAG_XELEMENTNAME = 8192
CONST HASHFLAG_XTYPENAME = 16384

' All DIM SHARED declarations moved to qb64pe.bas

