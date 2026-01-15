
' Hash function: Computes a hash value for string lookup in symbol table
' Hash value bit layout: [5(first)][5(second)][5(last)][5(2nd-last)][3(length AND 7)][1(first char is underscore)]
' This provides fast O(1) average-case lookup by encoding key string characteristics
FUNCTION HashValue& (a$) 'returns the hash table value of a string
    '[5(first)][5(second)][5(last)][5(2nd-last)][3(length AND 7)][1(first char is underscore)]
    l = LEN(a$)
    IF l = 0 THEN EXIT FUNCTION 'an (invalid) NULL string equates to 0
    a = ASC(a$)
    ' Handle identifiers that don't start with underscore (normal case)
    ' Hash encodes: first 2 chars, last 2 chars, length (mod 8), and underscore flag
    IF a <> 95 THEN 'does not begin with underscore
        SELECT CASE l
            CASE 1
                ' Single char: hash1char value + length offset (1048576 = 2^20, length slot)
                HashValue& = hash1char(a) + 1048576
                EXIT FUNCTION
            CASE 2
                ' Two chars: hash2char of both + length offset (2097152 = 2*2^20)
                HashValue& = hash2char(CVI(a$)) + 2097152
                EXIT FUNCTION
            CASE 3
                ' Three chars: first 2 via hash2char, 3rd via hash1char, length offset (3145728 = 3*2^20)
                HashValue& = hash2char(CVI(a$)) + hash1char(ASC(a$, 3)) * 1024 + 3145728
                EXIT FUNCTION
            CASE ELSE
                ' Four+ chars: first 2, last 2, length (mod 8) in upper bits
                ' hash2char encodes pairs of characters, 1024 multiplier shifts to next 5-bit slot
                HashValue& = hash2char(CVI(a$)) + hash2char(ASC(a$, l) + ASC(a$, l - 1) * 256) * 1024 + (l AND 7) * 1048576
                EXIT FUNCTION
        END SELECT
    ELSE 'does begin with underscore
        ' Identifiers starting with underscore: add 8388608 (2^23) to mark underscore flag
        ' Skip first char (underscore) in hash calculation to avoid collisions
        SELECT CASE l
            CASE 1
                ' Just underscore: length offset + underscore flag
                HashValue& = (1048576 + 8388608): EXIT FUNCTION 'note: underscore only is illegal in QB64 but supported by hash
            CASE 2
                ' Underscore + 1 char: hash1char of 2nd char + length + underscore flag
                HashValue& = hash1char(ASC(a$, 2)) + (2097152 + 8388608)
                EXIT FUNCTION
            CASE 3
                ' Underscore + 2 chars: hash2char of chars 2-3 + length + underscore flag
                HashValue& = hash2char(ASC(a$, 2) + ASC(a$, 3) * 256) + (3145728 + 8388608)
                EXIT FUNCTION
            CASE 4
                ' Underscore + 3 chars: hash2char of chars 2-3, hash1char of char 4, length + underscore flag
                ' CVL extracts 4 bytes, mask extracts middle 2 bytes (chars 2-3)
                HashValue& = hash2char((CVL(a$) AND &HFFFF00) \ 256) + hash1char(ASC(a$, 4)) * 1024 + (4194304 + 8388608)
                EXIT FUNCTION
            CASE ELSE
                ' Underscore + 4+ chars: chars 2-3, last 2 chars, length (mod 8), underscore flag
                HashValue& = hash2char((CVL(a$) AND &HFFFF00) \ 256) + hash2char(ASC(a$, l) + ASC(a$, l - 1) * 256) * 1024 + (l AND 7) * 1048576 + 8388608
                EXIT FUNCTION
        END SELECT
    END IF
END FUNCTION

' Add a symbol to the hash table with collision chaining
' Uses a free list for efficient memory reuse of removed entries
SUB HashAdd (a$, flags, reference)

    ' Find the index to use: prefer free list (reuse removed entries) over new allocation
    IF HashListFreeLast > 0 THEN
        'take from free list (LIFO - Last In First Out for cache efficiency)
        i = HashListFree(HashListFreeLast)
        HashListFreeLast = HashListFreeLast - 1
    ELSE
        ' No free entries: allocate new slot, grow table if needed
        IF HashListNext > HashListSize THEN
            'double hash list size (exponential growth for amortized O(1) performance)
            HashListSize = HashListSize * 2
            REDIM _PRESERVE HashList(1 TO HashListSize) AS HashListItem
            REDIM _PRESERVE HashListName(1 TO HashListSize) AS STRING * 256
        END IF
        i = HashListNext
        HashListNext = HashListNext + 1
    END IF

    ' Setup collision chain links: hash table uses chaining to handle collisions
    ' Each hash bucket points to a linked list of entries with the same hash value
    x = HashValue(a$)
    i2 = HashTable(x)
    IF i2 THEN
        ' Collision: add to end of existing chain
        ' LastItem tracks the tail for O(1) append
        i3 = HashList(i2).LastItem
        HashList(i2).LastItem = i
        HashList(i3).NextItem = i
        HashList(i).PrevItem = i3
    ELSE
        ' No collision: this is the first entry in this hash bucket
        HashTable(x) = i
        HashList(i).PrevItem = 0
        HashList(i).LastItem = i
    END IF
    HashList(i).NextItem = 0

    ' Set common hashlist values: flags identify symbol type, reference points to actual data
    HashList(i).Flags = flags
    HashList(i).Reference = reference
    HashListName(i) = UCASE$(a$)

END SUB

FUNCTION HashFind (a$, searchflags, resultflags, resultreference)
    '(0,1,2)z=hashfind[rev]("RUMI",Hashflag_label,resflag,resref)
    '0=doesn't exist
    '1=found, no more items to scan
    '2=found, more items still to scan
    ' Look up hash bucket and traverse collision chain
    i = HashTable(HashValue(a$))
    IF i THEN
        ' Pad name to fixed length for fast comparison (avoids length checks)
        ua$ = UCASE$(a$) + SPACE$(256 - LEN(a$))
        hashfind_next:
        f = HashList(i).Flags
        ' Check if this entry matches the requested symbol type (flags must overlap)
        IF searchflags AND f THEN 'flags in common
            ' Name match: found the symbol
            IF HashListName(i) = ua$ THEN
                resultflags = f
                resultreference = HashList(i).Reference
                ' Check if more entries exist in collision chain for continuation
                i2 = HashList(i).NextItem
                IF i2 THEN
                    ' More matches available: save state for HashFindCont
                    HashFind = 2
                    HashFind_NextListItem = i2
                    HashFind_Reverse = 0
                    HashFind_SearchFlags = searchflags
                    HashFind_Name = ua$
                    HashRemove_LastFound = i
                    EXIT FUNCTION
                ELSE
                    ' Last match in chain
                    HashFind = 1
                    HashRemove_LastFound = i
                    EXIT FUNCTION
                END IF
            END IF
        END IF
        ' Continue searching collision chain
        i = HashList(i).NextItem
        IF i THEN GOTO hashfind_next
    END IF
END FUNCTION

FUNCTION HashFindRev (a$, searchflags, resultflags, resultreference)
    '(0,1,2)z=hashfind[rev]("RUMI",Hashflag_label,resflag,resref)
    '0=doesn't exist
    '1=found, no more items to scan
    '2=found, more items still to scan
    i = HashTable(HashValue(a$))
    IF i THEN
        i = HashList(i).LastItem
        ua$ = UCASE$(a$) + SPACE$(256 - LEN(a$))
        hashfindrev_next:
        f = HashList(i).Flags
        IF searchflags AND f THEN 'flags in common
            IF HashListName(i) = ua$ THEN
                resultflags = f
                resultreference = HashList(i).Reference
                i2 = HashList(i).PrevItem
                IF i2 THEN
                    HashFindRev = 2
                    HashFind_NextListItem = i2
                    HashFind_Reverse = 1
                    HashFind_SearchFlags = searchflags
                    HashFind_Name = ua$
                    HashRemove_LastFound = i
                    EXIT FUNCTION
                ELSE
                    HashFindRev = 1
                    HashRemove_LastFound = i
                    EXIT FUNCTION
                END IF
            END IF
        END IF
        i = HashList(i).PrevItem
        IF i THEN GOTO hashfindrev_next
    END IF
END FUNCTION

' Continue hash table search from previous HashFind/HashFindRev call
' Used to find additional matches in collision chain (e.g., overloaded functions)
' Returns: 0=no more items, 1=found (last match), 2=found (more matches available)
FUNCTION HashFindCont (resultflags, resultreference)
    '(0,1,2)z=hashfind[rev](resflag,resref)
    '0=no more items exist
    '1=found, no more items to scan
    '2=found, more items still to scan
    ' Check if we're searching forward (normal) or backward (reverse)
    IF HashFind_Reverse THEN
        ' Reverse search: traverse collision chain backward (PrevItem)
        ' Used when searching from end of chain (HashFindRev)
        i = HashFind_NextListItem
        hashfindrevc_next:
        f = HashList(i).Flags
        IF HashFind_SearchFlags AND f THEN 'flags in common
            IF HashListName(i) = HashFind_Name THEN
                resultflags = f
                resultreference = HashList(i).Reference
                i2 = HashList(i).PrevItem
                IF i2 THEN
                    HashFindCont = 2
                    HashFind_NextListItem = i2
                    HashRemove_LastFound = i
                    EXIT FUNCTION
                ELSE
                    HashFindCont = 1
                    HashRemove_LastFound = i
                    EXIT FUNCTION
                END IF
            END IF
        END IF
        i = HashList(i).PrevItem
        IF i THEN GOTO hashfindrevc_next
        EXIT FUNCTION

    ELSE
        ' Forward search: traverse collision chain forward (NextItem)
        ' Used when searching from start of chain (HashFind)
        i = HashFind_NextListItem
        hashfindc_next:
        f = HashList(i).Flags
        IF HashFind_SearchFlags AND f THEN 'flags in common
            IF HashListName(i) = HashFind_Name THEN
                resultflags = f
                resultreference = HashList(i).Reference
                i2 = HashList(i).NextItem
                IF i2 THEN
                    HashFindCont = 2
                    HashFind_NextListItem = i2
                    HashRemove_LastFound = i
                    EXIT FUNCTION
                ELSE
                    HashFindCont = 1
                    HashRemove_LastFound = i
                    EXIT FUNCTION
                END IF
            END IF
        END IF
        i = HashList(i).NextItem
        IF i THEN GOTO hashfindc_next
        EXIT FUNCTION

    END IF
END FUNCTION

' Remove a symbol from the hash table and unlink it from collision chain
' The entry to remove must have been found by HashFind/HashFindRev (stored in HashRemove_LastFound)
SUB HashRemove

    i = HashRemove_LastFound

    ' Add entry to free list for reuse (memory pool optimization)
    ' Free list uses LIFO (Last In First Out) for cache efficiency
    HashListFreeLast = HashListFreeLast + 1
    ' Grow free list if needed (exponential growth for amortized O(1) performance)
    IF HashListFreeLast > HashListFreeSize THEN
        HashListFreeSize = HashListFreeSize * 2
        REDIM _PRESERVE HashListFree(1 TO HashListFreeSize) AS LONG
    END IF
    HashListFree(HashListFreeLast) = i

    ' Unlink entry from collision chain: maintain doubly-linked list integrity
    ' Four cases: first item, last item, middle item, or only item
    i1 = HashList(i).PrevItem
    IF i1 THEN
        ' Not first item in chain: has a previous item
        i2 = HashList(i).NextItem
        IF i2 THEN
            ' Middle item: has both previous and next
            ' Link previous to next, bypassing current item
            HashList(i1).NextItem = i2
            ' Update LastItem pointer if this was the tail (LastItem points to head)
            HashList(i2).LastItem = i1
        ELSE
            ' Last item in chain: update head's LastItem pointer
            x = HashTable(HashValue(HashListName$(i)))
            HashList(x).LastItem = i1
            HashList(i1).NextItem = 0
        END IF
    ELSE
        ' First item in chain: update hash table bucket pointer
        x = HashTable(HashValue(HashListName$(i)))
        i2 = HashList(i).NextItem
        IF i2 THEN
            ' First but not last: make next item the new head
            HashTable(x) = i2
            HashList(i2).PrevItem = 0
            ' Preserve LastItem pointer (points to actual tail, not removed head)
            HashList(i2).LastItem = HashList(i).LastItem
        ELSE
            ' Only item in chain: clear hash bucket (no more entries)
            HashTable(x) = 0
        END IF
    END IF

END SUB

SUB HashDump 'used for debugging purposes
    fh = FREEFILE
    OPEN "hashdump.txt" FOR OUTPUT AS #fh
    b$ = "12345678901234567890123456789012}"
    FOR x = 0 TO 16777215
        IF HashTable(x) THEN

            PRINT #fh, "START HashTable("; x; "):"
            i = HashTable(x)

            'validate
            lasti = HashList(i).LastItem
            IF HashList(i).LastItem = 0 OR HashList(i).PrevItem <> 0 OR HashValue(HashListName(i)) <> x THEN GOTO corrupt

            PRINT #fh, "  HashList("; i; ").LastItem="; HashList(i).LastItem
            hashdumpnextitem:
            x$ = "  [" + _TOSTR$(i) + "]" + HashListName(i)

            f = HashList(i).Flags
            x$ = x$ + ",.Flags=" + _TOSTR$(f) + "{"
            FOR z = 1 TO 32
                ASC(b$, z) = (f AND 1) + 48
                f = f \ 2
            NEXT
            x$ = x$ + b$

            x$ = x$ + ",.Reference=" + _TOSTR$(HashList(i).Reference)

            PRINT #fh, x$

            'validate
            i1 = HashList(i).PrevItem
            i2 = HashList(i).NextItem
            IF i1 THEN
                IF HashList(i1).NextItem <> i THEN GOTO corrupt
            END IF
            IF i2 THEN
                IF HashList(i2).PrevItem <> i THEN GOTO corrupt
            END IF
            IF i2 = 0 THEN
                IF lasti <> i THEN GOTO corrupt
            END IF

            i = HashList(i).NextItem
            IF i THEN GOTO hashdumpnextitem

            PRINT #fh, "END HashTable("; x; ")"
        END IF
    NEXT
    CLOSE #fh

    EXIT SUB
    corrupt:
    PRINT #fh, "HASH TABLE CORRUPT!" 'should never happen
    CLOSE #fh

END SUB

SUB HashClear 'clear entire hash table

    HashListSize = 65536
    HashListNext = 1
    HashListFreeSize = 1024
    HashListFreeLast = 0
    REDIM HashList(1 TO HashListSize) AS HashListItem
    REDIM HashListName(1 TO HashListSize) AS STRING * 256
    REDIM HashListFree(1 TO HashListFreeSize) AS LONG
    REDIM HashTable(16777215) AS LONG '64MB lookup table with indexes to the hashlist

    HashFind_NextListItem = 0
    HashFind_Reverse = 0
    HashFind_SearchFlags = 0
    HashFind_Name = ""
    HashRemove_LastFound = 0

END SUB

