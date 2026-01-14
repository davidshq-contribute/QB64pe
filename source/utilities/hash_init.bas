
'$INCLUDEONCE

' Hash table initialization
' This file contains executable code to initialize hash tables and must be
' included in Phase 2 (main program section) after hash_declarations.bi

'Initialize hash lookup tables (moved from hash.bi)
FOR x = 1 TO 26
    hash1char(64 + x) = x
    hash1char(96 + x) = x
NEXT
hash1char(95) = 27 '_
hash1char(48) = 28 '0
hash1char(49) = 29 '1
hash1char(50) = 30 '2
hash1char(51) = 31 '3
hash1char(52) = 23 '4 'note: x, y, z and beginning alphabet letters avoided because of common usage (eg. a2, y3)
hash1char(53) = 22 '5
hash1char(54) = 20 '6
hash1char(55) = 19 '7
hash1char(56) = 18 '8
hash1char(57) = 17 '9
FOR c1 = 0 TO 255
    FOR c2 = 0 TO 255
        hash2char(c1 + c2 * 256) = hash1char(c1) + hash1char(c2) * 32
    NEXT
NEXT

'Initialize hash list
HashListSize = 65536
HashListNext = 1
HashListFreeSize = 1024
HashListFreeLast = 0
' All REDIM SHARED declarations moved to qb64pe.bas

'Initialize CONST support arrays
constmax = 100
constlast = -1
' All REDIM SHARED declarations moved to qb64pe.bas

