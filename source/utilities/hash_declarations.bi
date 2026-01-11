
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

DIM SHARED HashFind_NextListItem AS LONG
DIM SHARED HashFind_Reverse AS LONG
DIM SHARED HashFind_SearchFlags AS LONG
DIM SHARED HashFind_Name AS STRING
DIM SHARED HashRemove_LastFound AS LONG
DIM SHARED HashListSize AS LONG
DIM SHARED HashListNext AS LONG
DIM SHARED HashListFreeSize AS LONG
DIM SHARED HashListFreeLast AS LONG

'hash lookup tables (declarations only)
DIM SHARED hash1char(255) AS INTEGER
DIM SHARED hash2char(65535) AS INTEGER

'hash list arrays (will be REDIM'd in hash_init.bas)
' Note: These are declared as dynamic arrays here
DIM SHARED HashList() AS HashListItem
DIM SHARED HashListName() AS STRING * 256
DIM SHARED HashListFree() AS LONG
DIM SHARED HashTable() AS LONG

'CONST support arrays (declarations only)
DIM SHARED constmax AS LONG
DIM SHARED constlast AS LONG
DIM SHARED constname() AS STRING
DIM SHARED constcname() AS STRING
DIM SHARED constnamesymbol() AS STRING 'optional name symbol
DIM SHARED consttype() AS LONG 'variable type number
DIM SHARED constinteger() AS _INTEGER64
DIM SHARED constuinteger() AS _UNSIGNED _INTEGER64
DIM SHARED constfloat() AS _FLOAT
DIM SHARED conststring() AS STRING
DIM SHARED constsubfunc() AS LONG
DIM SHARED constdefined() AS LONG

