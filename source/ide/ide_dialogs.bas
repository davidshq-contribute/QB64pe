'================================================================================
' IDE Dialog Functions Module
'================================================================================
' This module contains all dialog box functions for the QB64 IDE.
' Extracted from ide_methods.bas as part of the IDE modernization plan.
'
' Functions:
'   - ideinputbox$      : Generic input dialog box
'   - idefiledialog$    : File open/save dialog box
'   - ideLayoutBox      : Code layout settings dialog
'   - ideCompilerSettingsBox : Compiler settings dialog
'   - idemessagebox     : Generic message box dialog
'   - ideDisplayBox     : Display settings dialog
'   - idechoosecolorsbox : IDE color scheme selection dialog
'================================================================================

FUNCTION ideinputbox$ (title$, caption$, initialvalue$, validinput$, boxwidth, maxlength, ok)


    '-------- generic dialog box header --------
    PCOPY 0, 2
    PCOPY 0, 1
    SCREEN , , 1, 0
    focus = 1
    DIM p AS idedbptype
    DIM o(1 TO 100) AS idedbotype
    DIM sep AS STRING * 1
    sep = CHR$(0)
    '-------- end of generic dialog box header --------

    '-------- init --------

    i = 0
    ok = 0 'will be set to true if "OK" or Enter are used to close the dialog

    idepar p, boxwidth, 5, title$

    i = i + 1
    PrevFocus = 1
    o(i).typ = 1
    o(i).y = 2
    o(i).nam = idenewtxt(caption$)
    o(i).txt = idenewtxt(initialvalue$)
    IF LEN(initialvalue$) > 0 THEN o(i).issel = -1
    o(i).sx1 = 0
    o(i).v1 = LEN(initialvalue$)

    i = i + 1
    o(i).typ = 3
    o(i).y = 5
    o(i).txt = idenewtxt("#OK" + sep + "#Cancel")
    o(i).dft = 1
    '-------- end of init --------

    '-------- generic init --------
    FOR i = 1 TO 100: o(i).par = p: NEXT 'set parent info of objects
    '-------- end of generic init --------

    DO 'main loop


        '-------- generic display dialog box & objects --------
        idedrawpar p
        f = 1: cx = 0: cy = 0
        FOR i = 1 TO 100
            IF o(i).typ THEN

                'prepare object
                o(i).foc = focus - f 'focus offset
                o(i).cx = 0: o(i).cy = 0
                idedrawobj o(i), f 'display object
                IF o(i).cx THEN cx = o(i).cx: cy = o(i).cy
            END IF
        NEXT i
        lastfocus = f - 1
        '-------- end of generic display dialog box & objects --------

        '-------- custom display changes --------
        '-------- end of custom display changes --------

        'update visual page and cursor position
        PCOPY 1, 0
        IF cx THEN SCREEN , , 0, 0: LOCATE cy, cx, 1: SCREEN , , 1, 0

        '-------- read input --------
        change = 0
        DO
            GetInput
            IF mWHEEL THEN change = 1
            IF KB THEN change = 1
            IF mCLICK THEN mousedown = 1: change = 1
            IF mRELEASE THEN mouseup = 1: change = 1
            IF mB THEN change = 1
            alt = KALT: IF alt <> oldalt THEN change = 1
            oldalt = alt
            _LIMIT 100
        LOOP UNTIL change
        IF alt AND NOT KCTRL THEN idehl = 1 ELSE idehl = 0
        'convert "alt+letter" scancode to letter's ASCII character
        altletter$ = ""
        IF alt AND NOT KCTRL THEN
            IF LEN(K$) = 1 THEN
                k = ASC(UCASE$(K$))
                IF k >= 65 AND k <= 90 THEN altletter$ = CHR$(k)
            END IF
        END IF
        SCREEN , , 0, 0: LOCATE , , 0: SCREEN , , 1, 0
        '-------- end of read input --------

        '-------- generic input response --------
        info = 0
        IF K$ = "" THEN K$ = CHR$(255)
        IF KSHIFT = 0 AND K$ = CHR$(9) THEN focus = focus + 1
        IF (KSHIFT AND K$ = CHR$(9)) OR (INSTR(_OS$, "MAC") AND K$ = CHR$(25)) THEN focus = focus - 1: K$ = ""
        IF focus < 1 THEN focus = lastfocus
        IF focus > lastfocus THEN focus = 1
        f = 1
        FOR i = 1 TO 100
            t = o(i).typ
            IF t THEN
                focusoffset = focus - f
                ideobjupdate o(i), focus, f, focusoffset, K$, altletter$, mB, mousedown, mouseup, mX, mY, info, mWHEEL
            END IF
        NEXT
        '-------- end of generic input response --------

        'specific post controls
        IF focus <> PrevFocus THEN
            'Always start with TextBox values selected upon getting focus
            PrevFocus = focus
            IF focus = 1 THEN
                o(focus).v1 = LEN(idetxt(o(focus).txt))
                IF o(focus).v1 > 0 THEN o(focus).issel = -1
                o(focus).sx1 = 0
            END IF
        END IF

        IF LEN(validinput$) THEN
            a$ = idetxt(o(1).txt)
            tempA$ = ""
            FOR i = 1 TO LEN(a$)
                IF INSTR(validinput$, MID$(a$, i, 1)) > 0 THEN
                    tempA$ = tempA$ + MID$(a$, i, 1)
                END IF
            NEXT
            idetxt(o(1).txt) = tempA$
        END IF

        IF maxlength THEN
            idetxt(o(1).txt) = LEFT$(idetxt(o(1).txt), maxlength)
        END IF

        IF K$ = CHR$(27) OR (focus = 3 AND info <> 0) THEN
            ClearMouse
            EXIT FUNCTION
        END IF

        IF K$ = CHR$(13) OR (focus = 2 AND info <> 0) THEN
            ideinputbox$ = idetxt(o(1).txt)
            ok = -1
            ClearMouse
            _KEYCLEAR
            EXIT FUNCTION
        END IF
        'end of custom controls

        mousedown = 0
        mouseup = 0
    LOOP

END FUNCTION
FUNCTION idefiledialog$ (programname$, mode AS _BYTE)
    STATIC AllFiles

    '-------- generic dialog box header --------
    PCOPY 0, 2
    PCOPY 0, 1
    SCREEN , , 1, 0
    focus = 1
    DIM p AS idedbptype
    DIM o(1 TO 100) AS idedbotype
    DIM sep AS STRING * 1
    sep = CHR$(0)
    '-------- end of generic dialog box header --------

    '-------- init --------
    path$ = idepath$
    IF mode = 3 THEN 'font selector
        path$ = _DIR$("Fonts"): AllFiles = 1
        path$ = LEFT$(path$, LEN(path$) - 1)
    ELSEIF mode = 4 THEN 'logfile selector
        oprg$ = programname$: AllFiles = 1
        path$ = ideztakepath$(programname$)
    END IF
    IF RIGHT$(path$, 1) = "/" OR RIGHT$(path$, 1) = "\" THEN
        path$ = LEFT$(path$, LEN(path$) - 1)
    END IF
    filelist$ = idezfilelist$(path$, AllFiles, "")
    pathlist$ = idezpathlist$(path$)

    i = 0
    IF mode = 1 THEN
        idepar p, 70, idewy + idesubwindow - 7, "Open"
    ELSEIF mode = 2 THEN
        idepar p, 70, idewy + idesubwindow - 7, "Save As"
    ELSEIF mode = 3 THEN
        idepar p, 70, idewy + idesubwindow - 7, "Choose a custom font"
    ELSEIF mode = 4 THEN
        idepar p, 70, idewy + idesubwindow - 7, "Save logging to"
    END IF
    i = i + 1
    PrevFocus = 1
    o(i).typ = 1
    o(i).y = 2
    o(i).nam = idenewtxt("File #Name")
    IF mode > 1 THEN
        o(i).txt = idenewtxt(programname$)
        o(i).issel = -1
        o(i).sx1 = 0
        o(i).v1 = LEN(programname$)
    END IF

    i = i + 1
    o(i).typ = 2
    o(i).y = 5
    o(i).w = 32: o(i).h = idewy + idesubwindow - 14
    o(i).nam = idenewtxt("#Files")
    o(i).txt = idenewtxt(filelist$): filelist$ = ""
    i = i + 1
    o(i).typ = 2
    o(i).x = 37: o(i).y = 5
    o(i).w = 31: o(i).h = idewy + idesubwindow - 16
    o(i).nam = idenewtxt("#Paths")
    o(i).txt = idenewtxt(pathlist$): pathlist$ = ""
    i = i + 1
    o(i).typ = 4 'check box
    o(i).x = 37
    o(i).y = idewy + idesubwindow - 9
    o(i).nam = idenewtxt(".#BAS Only")
    IF AllFiles THEN o(i).sel = 0 ELSE o(i).sel = 1
    prevBASOnly = o(i).sel
    i = i + 1
    o(i).typ = 3
    o(i).x = 56
    o(i).y = idewy + idesubwindow - 9
    o(i).txt = idenewtxt("Ne#w Folder")
    i = i + 1
    o(i).typ = 3
    o(i).y = idewy + idesubwindow - 7
    o(i).txt = idenewtxt("#OK" + sep + "#Cancel")
    o(i).dft = 1
    '-------- end of init --------

    '-------- generic init --------
    FOR i = 1 TO 100: o(i).par = p: NEXT 'set parent info of objects
    '-------- end of generic init --------

    IF mode = 1 AND LEN(IdeOpenFile) > 0 THEN f$ = IdeOpenFile: GOTO DirectLoad
    IF mode = 3 AND (INSTR(programname$, "?") > 0 OR INSTR(programname$, "*") > 0) THEN
        f$ = programname$: GOTO wildcardsearch
    END IF

    DO 'main loop

        '-------- generic display dialog box & objects --------
        idedrawpar p
        f = 1: cx = 0: cy = 0
        FOR i = 1 TO 100
            IF o(i).typ THEN
                'prepare object
                o(i).foc = focus - f 'focus offset
                o(i).cx = 0: o(i).cy = 0
                idedrawobj o(i), f 'display object
                IF o(i).cx THEN cx = o(i).cx: cy = o(i).cy
            END IF
        NEXT i
        lastfocus = f - 1
        '-------- end of generic display dialog box & objects --------

        '-------- custom display changes --------
        COLOR 0, 7: _PRINTSTRING (p.x + 2, p.y + 4), "Path: "
        a$ = path$
        IF LEN(a$) = 2 AND RIGHT$(a$, 1) = ":" THEN a$ = a$ + "\"
        w = p.w - 8
        IF LEN(a$) > w - 3 THEN a$ = STRING$(3, 250) + RIGHT$(a$, w - 3)
        _PRINTSTRING (p.x + 2 + 6, p.y + 4), a$
        '-------- end of custom display changes --------


        'update visual page and cursor position
        PCOPY 1, 0
        IF cx THEN SCREEN , , 0, 0: LOCATE cy, cx, 1: SCREEN , , 1, 0

        '-------- read input --------
        change = 0
        DO
            GetInput
            IF mWHEEL THEN change = 1
            IF KB THEN change = 1
            IF mCLICK THEN mousedown = 1: change = 1
            IF mRELEASE THEN mouseup = 1: change = 1
            IF mB THEN change = 1
            alt = KALT: IF alt <> oldalt THEN change = 1
            oldalt = alt

            IF mode = 1 THEN
                IF _TOTALDROPPEDFILES > 0 THEN
                    idetxt(o(1).txt) = _DROPPEDFILE$(1)
                    o(1).v1 = LEN(idetxt(o(1).txt))
                    focus = 1
                    _FINISHDROP
                    change = 1
                END IF
            END IF

            _LIMIT 100
        LOOP UNTIL change
        IF alt AND NOT KCTRL THEN idehl = 1 ELSE idehl = 0
        'convert "alt+letter" scancode to letter's ASCII character
        altletter$ = ""
        IF alt AND NOT KCTRL THEN
            IF LEN(K$) = 1 THEN
                k = ASC(UCASE$(K$))
                IF k >= 65 AND k <= 90 THEN altletter$ = CHR$(k)
            END IF
        END IF
        SCREEN , , 0, 0: LOCATE , , 0: SCREEN , , 1, 0
        '-------- end of read input --------

        '-------- generic input response --------
        info = 0
        IF K$ = "" THEN K$ = CHR$(255)
        IF KSHIFT = 0 AND K$ = CHR$(9) THEN focus = focus + 1
        IF (KSHIFT AND K$ = CHR$(9)) OR (INSTR(_OS$, "MAC") AND K$ = CHR$(25)) THEN focus = focus - 1: K$ = ""
        IF focus < 1 THEN focus = lastfocus
        IF focus > lastfocus THEN focus = 1
        f = 1
        FOR i = 1 TO 100
            t = o(i).typ
            IF t THEN
                focusoffset = focus - f
                ideobjupdate o(i), focus, f, focusoffset, K$, altletter$, mB, mousedown, mouseup, mX, mY, info, mWHEEL
            END IF
        NEXT
        '-------- end of generic input response --------










        'specific post controls
        IF focus <> PrevFocus THEN
            'Always start with TextBox values selected upon getting focus
            PrevFocus = focus
            IF focus = 1 THEN
                o(focus).v1 = LEN(idetxt(o(focus).txt))
                IF o(focus).v1 > 0 THEN o(focus).issel = -1
                o(focus).sx1 = 0
            END IF
        END IF

        IF o(4).sel <> prevBASOnly THEN
            prevBASOnly = o(4).sel
            IF o(4).sel = 0 THEN AllFiles = 1 ELSE AllFiles = 0
            idetxt(o(2).txt) = idezfilelist$(path$, AllFiles, "")
            o(2).sel = -1
            GOTO ideopenloop
        END IF

        IF focus = 5 AND info <> 0 THEN
            'create new folder
            newpath$ = idenewfolder(path$)
            IF LEN(newpath$) THEN
                f$ = RemoveDoubleSlashes$(newpath$)
                GOTO changepath
            ELSE
                GOTO ideopenloop
            END IF
        END IF

        IF K$ = CHR$(27) OR (focus = 7 AND info <> 0) THEN
            idefiledialog$ = "C"
            IF mode = 4 THEN 'logfile mode
                programname$ = oprg$ 'restore to avoid argument side effect
            END IF
            EXIT FUNCTION
        END IF

        IF focus = 2 AND o(2).sel <> prevFileBoxSel THEN
            prevFileBoxSel = o(2).sel
            idetxt(o(1).txt) = idetxt(o(2).stx)
            o(1).issel = 0
        END IF

        IF focus = 3 THEN
            IF (K$ = CHR$(13) OR info = 1) AND o(3).sel >= 1 THEN
                newpath$ = RemoveDoubleSlashes$(idetxt(o(3).stx))
                IF newpath$ = "" THEN
                    newpath$ = ".."
                    f$ = newpath$
                    GOTO changepath
                ELSE
                    path$ = RemoveDoubleSlashes$(idezchangepath(path$, newpath$))
                    idetxt(o(2).txt) = idezfilelist$(path$, AllFiles, "")
                    idetxt(o(3).txt) = idezpathlist$(path$)

                    o(2).sel = -1
                    o(3).sel = 1
                    IF info = 1 THEN o(3).sel = -1
                    IF mode = 3 AND (INSTR(programname$, "?") > 0 OR INSTR(programname$, "*") > 0) THEN
                        f$ = programname$: GOTO wildcardsearch
                    END IF
                    GOTO ideopenloop
                END IF
            END IF
        END IF

        'load or save file
        IF K$ = CHR$(13) OR (info = 1 AND focus = 2) OR (focus = 6 AND info <> 0) THEN
            f$ = idetxt(o(1).txt)

            IF _FILEEXISTS(f$) THEN GOTO DirectLoad

            IF f$ = "" AND focus = 1 AND K$ = CHR$(13) THEN
                'reset filters
                idetxt(o(2).txt) = idezfilelist$(path$, AllFiles, "")
                o(2).sel = -1
                GOTO ideopenloop
            ELSEIF f$ = "" AND focus = 6 AND info <> 0 THEN
                GOTO ideopenloop
            END IF

            'change path?
            changepath:
            IF _DIREXISTS(path$ + idepathsep$ + f$) THEN
                'check/acquire file path
                path$ = RemoveDoubleSlashes$(idezgetfilepath$(path$, f$ + idepathsep$)) 'note: path ending with pathsep needn't contain a file
                IF ideerror > 1 THEN EXIT FUNCTION

                IF LEN(newpath$) = 0 THEN
                    idetxt(o(1).txt) = ""
                    focus = 1
                ELSE
                    newpath$ = ""
                END IF
                idetxt(o(2).txt) = idezfilelist$(path$, AllFiles, "")
                o(2).sel = -1
                idetxt(o(3).txt) = idezpathlist$(path$)
                o(3).sel = -1
                IF mode = 3 AND (INSTR(programname$, "?") > 0 OR INSTR(programname$, "*") > 0) THEN
                    f$ = programname$: GOTO wildcardsearch
                END IF
                GOTO ideopenloop
            END IF

            'wildcards search
            wildcardsearch:
            IF INSTR(f$, "?") > 0 OR INSTR(f$, "*") > 0 THEN
                IF INSTR(f$, "/") > 0 OR INSTR(f$, "\") > 0 THEN
                    'path + wildcards
                    path$ = RemoveDoubleSlashes$(idezgetfilepath$(path$, f$)) 'note: path ending with pathsep needn't contain a file
                    IF ideerror > 1 THEN EXIT FUNCTION
                    idetxt(o(3).txt) = idezpathlist$(path$)
                    o(3).sel = -1
                END IF
                idetxt(o(1).txt) = f$
                idetxt(o(2).txt) = idezfilelist$(path$, 2, f$)
                o(2).sel = -1
                o(1).v1 = LEN(idetxt(o(1).txt))
                o(1).issel = -1
                o(1).sx1 = 0
                IF LCASE$(RIGHT$(f$, 4)) <> ".bas" THEN
                    AllFiles = 0
                    o(4).sel = 0
                    prevBASOnly = o(4).sel
                END IF
                GOTO ideopenloop
            END IF

            DirectLoad:
            path$ = RemoveDoubleSlashes$(idezgetfilepath$(path$, f$)) 'repeat in case of DirectLoad
            IF ideerror > 1 THEN EXIT FUNCTION

            IF mode = 1 THEN 'load mode
                IF _FILEEXISTS(path$ + idepathsep$ + f$) = 0 THEN
                    'add .bas if not given
                    IF (LCASE$(RIGHT$(f$, 4)) <> ".bas") AND AllFiles = 0 THEN f$ = f$ + ".bas"
                END IF

                'check file exists
                ideerror = 2
                IF _FILEEXISTS(path$ + idepathsep$ + f$) = 0 THEN EXIT FUNCTION

                IF BinaryFormatCheck%(path$, idepathsep$, f$) > 0 THEN
                    IF LEN(IdeOpenFile) THEN
                        idefiledialog$ = "C"
                        EXIT FUNCTION
                    ELSE
                        info = 0: GOTO ideopenloop
                    END IF
                END IF

                'load file
                ideerror = 3
                idet$ = MKL$(0) + MKL$(0): idel = 1: ideli = 1: iden = 1: IdeBmkN = 0
                idesx = 1
                idesy = 1
                idecx = 1
                idecy = 1
                ideselect = 0
                idefocusline = 0
                lineinput3load path$ + idepathsep$ + f$
                idet$ = SPACE$(LEN(lineinput3buffer) * 8)
                i2 = 1
                n = 0
                chrtab$ = CHR$(9)
                space1$ = " ": space2$ = "  ": space3$ = "   ": space4$ = "    "
                chr7$ = CHR$(7): chr11$ = CHR$(11): chr12$ = CHR$(12): chr28$ = CHR$(28): chr29$ = CHR$(29): chr30$ = CHR$(30): chr31$ = CHR$(31)
                DO
                    a$ = lineinput3$
                    l = LEN(a$)
                    IF l THEN asca = ASC(a$) ELSE asca = -1
                    IF asca <> 13 THEN
                        IF asca <> -1 THEN
                            'fix tabs
                            ideopenfixtabs:
                            x = INSTR(a$, chrtab$)
                            IF x THEN
                                x2 = (x - 1) MOD 4
                                IF x2 = 0 THEN a$ = LEFT$(a$, x - 1) + space4$ + RIGHT$(a$, l - x): l = l + 3: GOTO ideopenfixtabs
                                IF x2 = 1 THEN a$ = LEFT$(a$, x - 1) + space3$ + RIGHT$(a$, l - x): l = l + 2: GOTO ideopenfixtabs
                                IF x2 = 2 THEN a$ = LEFT$(a$, x - 1) + space2$ + RIGHT$(a$, l - x): l = l + 1: GOTO ideopenfixtabs
                                IF x2 = 3 THEN a$ = LEFT$(a$, x - 1) + space1$ + RIGHT$(a$, l - x): GOTO ideopenfixtabs
                            END IF
                        END IF 'asca<>-1
                        MID$(idet$, i2, l + 8) = MKL$(l) + a$ + MKL$(l): i2 = i2 + l + 8: n = n + 1
                    END IF
                LOOP UNTIL asca = 13
                lineinput3buffer = ""
                iden = n: IF n = 0 THEN idet$ = MKL$(0) + MKL$(0): iden = 1 ELSE idet$ = LEFT$(idet$, i2 - 1)
                REDIM IdeBreakpoints(iden) AS _BYTE
                REDIM IdeSkipLines(iden) AS _BYTE
                variableWatchList$ = ""
                backupVariableWatchList$ = "": REDIM backupUsedVariableList(1000) AS usedVarList
                backupTypeDefinitions$ = ""
                callstacklist$ = "": callStackLength = 0

                ideerror = 1
                ideprogname = f$: _TITLE ideprogname + " - " + WindowTitle
                listOfCustomKeywords$ = LEFT$(listOfCustomKeywords$, customKeywordsLength)
                idepath$ = path$
                AddToHistory "RECENT", idepath$ + idepathsep$ + ideprogname$
                IdeImportBookmarks idepath$ + idepathsep$ + ideprogname$
                EXIT FUNCTION
            ELSEIF mode = 2 THEN 'save mode
                IF FileHasExtension(f$) = 0 THEN f$ = f$ + ".bas"

                ideerror = 3
                OPEN path$ + idepathsep$ + f$ FOR BINARY AS #150
                ideerror = 1
                IF LOF(150) THEN
                    CLOSE #150
                    a$ = idefileexists(f$)
                    IF a$ = "N" THEN
                        idefiledialog$ = "C"
                        EXIT FUNCTION 'user didn't agree to overwrite
                    END IF
                ELSE
                    CLOSE #150
                END IF
                ideprogname$ = f$: _TITLE ideprogname + " - " + WindowTitle
                idesave path$ + idepathsep$ + f$
                idepath$ = path$
                AddToHistory "RECENT", idepath$ + idepathsep$ + ideprogname$
                IdeSaveBookmarks idepath$ + idepathsep$ + ideprogname$
                EXIT FUNCTION
            ELSEIF mode = 3 THEN 'font mode
                idefiledialog$ = path$ + idepathsep$ + f$
                EXIT FUNCTION
            ELSEIF mode = 4 THEN 'logfile mode
                idefiledialog$ = path$ + idepathsep$ + f$
                programname$ = oprg$ 'restore to avoid argument side effect
                EXIT FUNCTION
            END IF
        END IF

        ideopenloop:

        'end of custom controls
        mousedown = 0
        mouseup = 0
    LOOP
END FUNCTION
FUNCTION ideLayoutBox

    '-------- generic dialog box header --------
    PCOPY 0, 2
    PCOPY 0, 1
    SCREEN , , 1, 0
    focus = 1
    DIM p AS idedbptype
    DIM o(1 TO 100) AS idedbotype
    DIM sep AS STRING * 1
    sep = CHR$(0)
    '-------- end of generic dialog box header --------

    '-------- init dialog box & objects --------
    i = 0
    idepar p, 39, 13, "Code Layout"

    i = i + 1: aiChk = i
    o(i).typ = 4 'check box
    o(i).y = 2
    o(i).nam = idenewtxt("Auto #Indent lines")
    o(i).sel = ABS(IDEAutoIndent)
    i = i + 1: aisBox = i
    o(i).typ = 1 'text box
    o(i).x = 9: o(i).y = 3
    o(i).nam = idenewtxt("Indent #Spacing"): a2$ = _TOSTR$(IDEAutoIndentSize)
    o(i).txt = idenewtxt(a2$): o(i).v1 = LEN(a2$): o(i).blk = 6
    i = i + 1: aisSymUp = i
    o(i).typ = 5 'symbol button
    o(i).x = 32: o(i).y = 3
    o(i).txt = idenewtxt(CHR$(30)): o(i).rpt = 10
    i = i + 1: aisSymDn = i
    o(i).typ = 5 'symbol button
    o(i).x = 35: o(i).y = 3
    o(i).txt = idenewtxt(CHR$(31)): o(i).rpt = 10
    i = i + 1: isChk = i
    o(i).typ = 4 'check box
    o(i).x = 6: o(i).y = 5
    o(i).nam = idenewtxt("Indent SUBs and #FUNCTIONs")
    o(i).sel = ABS(IDEIndentSubs)

    i = i + 1: alChk = i
    o(i).typ = 4 'check box
    o(i).y = 7
    o(i).nam = idenewtxt("#Auto Single-spacing code elements")
    o(i).sel = ABS(IDEAutoLayout)

    i = i + 1: kuChk = i
    o(i).typ = 4 'check box
    o(i).x = 4: o(i).y = 11
    o(i).nam = idenewtxt("#UPPER")
    o(i).sel = ABS(IDEAutoLayoutKwStyle = _GREATER)
    i = i + 1: kcChk = i
    o(i).typ = 4 'check box
    o(i).x = 16: o(i).y = 11
    o(i).nam = idenewtxt("Ca#MeL")
    o(i).sel = ABS(IDEAutoLayoutKwStyle = _EQUAL)
    i = i + 1: klChk = i
    o(i).typ = 4 'check box
    o(i).x = 28: o(i).y = 11
    o(i).nam = idenewtxt("#lower")
    o(i).sel = ABS(IDEAutoLayoutKwStyle = _LESS)

    i = i + 1: okBut = i: caBut = i + 1
    o(i).typ = 3 'action buttons
    o(i).y = 13
    o(i).txt = idenewtxt("#OK" + sep + "#Cancel"): o(i).dft = 1
    '-------- end of init dialog box & objects --------

    '-------- generic init --------
    FOR i = 1 TO 100: o(i).par = p: NEXT 'set parent info of objects
    '-------- end of generic init --------

    '-------- custom variables init --------
    '-------- end of custom variables init --------

    DO 'main loop

        '-------- generic display dialog box & objects --------
        idedrawpar p
        f = 1: cx = 0: cy = 0
        FOR i = 1 TO 100
            IF o(i).typ THEN
                'prepare object
                o(i).foc = focus - f 'focus offset
                o(i).cx = 0: o(i).cy = 0 'clear cursor pos
                IF i = focus _ANDALSO focus <> oldfocus THEN
                    oldfocus = focus
                    IF o(i).typ = 1 THEN 'if text box
                        'start with values selected upon getting focus
                        o(i).v1 = LEN(idetxt(o(i).txt)) 'selection len
                        IF o(i).v1 > 0 THEN o(i).issel = -1 ELSE o(i).issel = 0
                        o(focus).sx1 = 0 'selection start
                    END IF
                END IF
                idedrawobj o(i), f 'display object
                IF o(i).cx THEN cx = o(i).cx: cy = o(i).cy 'get new cursor pos
            END IF
        NEXT i
        lastfocus = f - 1
        '-------- end of generic display dialog box & objects --------

        '-------- custom display changes --------
        _PRINTSTRING (p.x, p.y + 9), CHR$(195) + STRING$(p.w, 196) + CHR$(180)
        _PRINTSTRING (p.x + 11, p.y + 9), " Show Keywords as "
        '-------- end of custom display changes --------

        'update visual page and cursor position
        PCOPY 1, 0
        IF cx THEN SCREEN , , 0, 0: LOCATE cy, cx, 1: SCREEN , , 1, 0

        '-------- read input --------
        change = 0
        DO
            GetInput
            IF mWHEEL THEN change = 1
            IF KB THEN change = 1
            IF mCLICK THEN mousedown = 1: change = 1
            IF mRELEASE THEN mouseup = 1: change = 1
            IF mB THEN change = 1
            alt = KALT: IF alt <> oldalt THEN change = 1
            oldalt = alt
            _LIMIT 100
        LOOP UNTIL change
        IF alt AND NOT KCTRL THEN idehl = 1 ELSE idehl = 0
        'convert "alt+letter" scancode to letter's ASCII character
        altletter$ = ""
        IF alt AND NOT KCTRL THEN
            IF LEN(K$) = 1 THEN
                k = ASC(UCASE$(K$))
                IF k >= 65 AND k <= 90 THEN altletter$ = CHR$(k)
            END IF
        END IF
        SCREEN , , 0, 0: LOCATE , , 0: SCREEN , , 1, 0
        '-------- end of read input --------

        '-------- generic input response --------
        info = 0: invdata = 0
        IF K$ = "" THEN K$ = CHR$(255)
        IF KSHIFT = 0 AND K$ = CHR$(9) THEN focus = focus + 1
        IF (KSHIFT AND K$ = CHR$(9)) OR (INSTR(_OS$, "MAC") AND K$ = CHR$(25)) THEN focus = focus - 1: K$ = ""
        IF focus < 1 THEN focus = lastfocus
        IF focus > lastfocus THEN focus = 1
        f = 1
        FOR i = 1 TO 100
            IF o(i).typ THEN
                focusoffset = focus - f
                ideobjupdate o(i), focus, f, focusoffset, K$, altletter$, mB, mousedown, mouseup, mX, mY, info, mWHEEL
            END IF
        NEXT
        '-------- end of generic input response --------

        '-------- custom input response --------
        'auto indent check box
        IF focus = aiChk AND o(aiChk).sel = 0 THEN 'goes off?
            o(isChk).sel = 0 'indent SUBs off
            idetxt(o(aisBox).txt) = "4": o(aisBox).v1 = 1 'reset indent spacing
        END IF
        'auto indent size spinners
        IF focus = aisSymUp AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(aisBox).txt)) + 1, 1, 64))
            idetxt(o(aisBox).txt) = a$: o(aisBox).v1 = LEN(a$)
            o(aiChk).sel = 1 'implies auto indent on
        END IF
        IF focus = aisSymDn AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(aisBox).txt)) - 1, 1, 64))
            idetxt(o(aisBox).txt) = a$: o(aisBox).v1 = LEN(a$)
            o(aiChk).sel = 1 'implies auto indent on
        END IF
        'auto indent size text box (valid data check)
        a$ = idetxt(o(aisBox).txt): o(aisBox).inv = 1
        IF isuinteger(a$) _ANDALSO (VAL(a$) >= 1 AND VAL(a$) <= 64) THEN o(aisBox).inv = 0
        IF o(aisBox).inv THEN invdata = 1 'block confirmation, as long as invalid
        IF focus = aisBox THEN
            IF o(aisBox).inv = 0 THEN o(aiChk).sel = 1 'manual input implies auto indent on, if valid
        END IF
        'indent SUBs check box
        IF focus = isChk AND o(isChk).sel = 1 THEN 'goes on?
            o(aiChk).sel = 1 'implies auto indent on
        END IF

        'auto layout check box (no checks required)

        'keyword UPPER check box
        IF focus = kuChk AND o(kuChk).sel = 1 THEN 'goes on?
            o(kcChk).sel = 0 'implies CaMeL case off
            o(klChk).sel = 0 'implies lower case off
        END IF
        'keyword CaMeL check box
        IF focus = kcChk AND o(kcChk).sel = 1 THEN 'goes on?
            o(kuChk).sel = 0 'implies UPPER case off
            o(klChk).sel = 0 'implies lower case off
        END IF
        'keyword lower check box
        IF focus = klChk AND o(klChk).sel = 1 THEN 'goes on?
            o(kuChk).sel = 0 'implies UPPER case off
            o(kcChk).sel = 0 'implies CaMeL case off
        END IF

        'ok & cancel buttons
        IF K$ = CHR$(27) OR (focus = caBut AND info <> 0) THEN EXIT FUNCTION
        IF K$ = CHR$(13) OR (focus = okBut AND info <> 0) THEN
            'blocked?
            IF invdata THEN
                retval = idemessagebox("Warning", "Confirmation has been blocked due to invalid settings.\nPlease check your inputs, look for highlighted boxes.", "#OK")
                PCOPY 2, 1: _CONTINUE
            END IF

            optChg% = _FALSE 'reset changed options indicator

            'adjust runtime variables
            v% = o(aiChk).sel: IF v% <> 0 THEN v% = _TRUE
            IF IDEAutoIndent <> v% THEN IDEAutoIndent = v%: optChg% = _TRUE
            v% = VAL(idetxt(o(aisBox).txt))
            IF IDEAutoIndentSize <> v% THEN
                IDEAutoIndentSize = v%
                IF IDEAutoIndent <> 0 THEN optChg% = _TRUE
            END IF
            v% = o(isChk).sel: IF v% <> 0 THEN v% = _TRUE
            IF IDEIndentSubs <> v% THEN IDEIndentSubs = v%: optChg% = _TRUE

            v% = o(alChk).sel: IF v% <> 0 THEN v% = _TRUE
            IF IDEAutoLayout <> v% THEN IDEAutoLayout = v%: optChg% = _TRUE

            'only one of these checkboxes can be selected
            IF o(kuChk).sel <> 0 THEN v% = 1
            IF o(kcChk).sel <> 0 THEN v% = 0
            IF o(klChk).sel <> 0 THEN v% = -1
            IF IDEAutoLayoutKwStyle <> v% THEN IDEAutoLayoutKwStyle = v%: optChg% = _TRUE

            IF optChg% THEN
                'update default values for restoring after '$FORMAT:OFF
                DEFAutoIndent = IDEAutoIndent: DEFAutoLayout = IDEAutoLayout
                'save changes
                WriteConfigSetting displaySettingsSection$, "IDE_AutoIndent", BoolToTFString$(IDEAutoIndent)
                WriteConfigSetting displaySettingsSection$, "IDE_IndentSize", _TOSTR$(IDEAutoIndentSize)
                WriteConfigSetting displaySettingsSection$, "IDE_IndentSUBs", BoolToTFString$(IDEIndentSubs)

                WriteConfigSetting displaySettingsSection$, "IDE_AutoFormat", BoolToTFString$(IDEAutoLayout)

                WriteConfigSetting displaySettingsSection$, "IDE_KeywordCapital", BoolToTFString$(IDEAutoLayoutKwStyle = _GREATER)
                WriteConfigSetting displaySettingsSection$, "IDE_KeywordLowercase", BoolToTFString$(IDEAutoLayoutKwStyle = _LESS)

                ideLayoutBox = 1
            END IF
            EXIT FUNCTION
        END IF
        '-------- end of custom input response --------

        mousedown = 0
        mouseup = 0
    LOOP
END FUNCTION
FUNCTION ideCompilerSettingsBox

    '-------- generic dialog box header --------
    PCOPY 0, 2
    PCOPY 0, 1
    SCREEN , , 1, 0
    focus = 1
    DIM p AS idedbptype
    DIM o(1 TO 100) AS idedbotype
    DIM sep AS STRING * 1
    sep = CHR$(0)
    '-------- end of generic dialog box header --------

    '-------- init dialog box & objects --------
    i = 0
    idepar p, 48, _IIF(os$ = "WIN", 16, 15), "Compiler Settings"

    i = i + 1: ocpChk = i
    o(i).typ = 4 'check box
    o(i).y = 2
    o(i).nam = idenewtxt("Compile #program with C++ optimization flag")
    o(i).sel = ABS(OptimizeCppProgram)
    i = i + 1: sdsChk = i
    o(i).typ = 4 'check box
    o(i).y = 3
    o(i).nam = idenewtxt("#Strip C++ symbols from executable")
    o(i).sel = ABS(StripDebugSymbols)
    i = i + 1: idiChk = i
    o(i).typ = 4 'check box
    o(i).y = 4
    o(i).nam = idenewtxt("#Add C++ Debug Information")
    o(i).sel = ABS(IncludeDebugInfo)

    i = i + 1: ecfBox = i
    o(i).typ = 1 'text box
    o(i).y = 6
    o(i).nam = idenewtxt("C++ Compiler #Flags"): a2$ = ExtraCppFlags$
    o(i).txt = idenewtxt(a2$): o(i).v1 = LEN(a2$)
    i = i + 1: elfBox = i
    o(i).typ = 1 'text box
    o(i).y = 9
    o(i).nam = idenewtxt("C++ #Linker Flags"): a2$ = ExtraLinkerFlags$
    o(i).txt = idenewtxt(a2$): o(i).v1 = LEN(a2$)

    i = i + 1: mppBox = i
    o(i).typ = 1 'text box
    o(i).y = 12
    o(i).nam = idenewtxt("#Max C++ Compiler Processes"): a2$ = _TOSTR$(MaxParallelProcesses)
    o(i).txt = idenewtxt(a2$): o(i).v1 = LEN(a2$): o(i).blk = 6
    i = i + 1: mppSymUp = i
    o(i).typ = 5 'symbol button
    o(i).x = 41: o(i).y = 12
    o(i).txt = idenewtxt(CHR$(30)): o(i).rpt = 10
    i = i + 1: mppSymDn = i
    o(i).typ = 5 'symbol button
    o(i).x = 44: o(i).y = 12
    o(i).txt = idenewtxt(CHR$(31)): o(i).rpt = 10

    IF os$ = "WIN" THEN
        i = i + 1: uscChk = i
        o(i).typ = 4 'check box
        o(i).y = 14
        o(i).nam = idenewtxt("#Use system C++ compiler")
        o(i).sel = ABS(UseSystemMinGW)
    END IF

    i = i + 1: okBut = i: caBut = i + 1
    o(i).typ = 3 'action buttons
    o(i).y = _IIF(os$ = "WIN", 16, 15)
    o(i).txt = idenewtxt("#OK" + sep + "#Cancel"): o(i).dft = 1
    '-------- end of init dialog box & objects --------

    '-------- generic init --------
    FOR i = 1 TO 100: o(i).par = p: NEXT 'set parent info of objects
    '-------- end of generic init --------

    '-------- custom variables init --------
    '-------- end of custom variables init --------

    DO 'main loop

        '-------- generic display dialog box & objects --------
        idedrawpar p
        f = 1: cx = 0: cy = 0
        FOR i = 1 TO 100
            IF o(i).typ THEN
                'prepare object
                o(i).foc = focus - f 'focus offset
                o(i).cx = 0: o(i).cy = 0 'clear cursor pos
                IF i = focus _ANDALSO focus <> oldfocus THEN
                    oldfocus = focus
                    IF o(i).typ = 1 THEN 'if text box
                        'start with values selected upon getting focus
                        o(i).v1 = LEN(idetxt(o(i).txt)) 'selection len
                        IF o(i).v1 > 0 THEN o(i).issel = -1 ELSE o(i).issel = 0
                        o(focus).sx1 = 0 'selection start
                    END IF
                END IF
                idedrawobj o(i), f 'display object
                IF o(i).cx THEN cx = o(i).cx: cy = o(i).cy 'get new cursor pos
            END IF
        NEXT i
        lastfocus = f - 1
        '-------- end of generic display dialog box & objects --------

        '-------- custom display changes --------
        '-------- end of custom display changes --------

        'update visual page and cursor position
        PCOPY 1, 0
        IF cx THEN SCREEN , , 0, 0: LOCATE cy, cx, 1: SCREEN , , 1, 0

        '-------- read input --------
        change = 0
        DO
            GetInput
            IF mWHEEL THEN change = 1
            IF KB THEN change = 1
            IF mCLICK THEN mousedown = 1: change = 1
            IF mRELEASE THEN mouseup = 1: change = 1
            IF mB THEN change = 1
            alt = KALT: IF alt <> oldalt THEN change = 1
            oldalt = alt
            _LIMIT 100
        LOOP UNTIL change
        IF alt AND NOT KCTRL THEN idehl = 1 ELSE idehl = 0
        'convert "alt+letter" scancode to letter's ASCII character
        altletter$ = ""
        IF alt AND NOT KCTRL THEN
            IF LEN(K$) = 1 THEN
                k = ASC(UCASE$(K$))
                IF k >= 65 AND k <= 90 THEN altletter$ = CHR$(k)
            END IF
        END IF
        SCREEN , , 0, 0: LOCATE , , 0: SCREEN , , 1, 0
        '-------- end of read input --------

        '-------- generic input response --------
        info = 0: invdata = 0
        IF K$ = "" THEN K$ = CHR$(255)
        IF KSHIFT = 0 AND K$ = CHR$(9) THEN focus = focus + 1
        IF (KSHIFT AND K$ = CHR$(9)) OR (INSTR(_OS$, "MAC") AND K$ = CHR$(25)) THEN focus = focus - 1: K$ = ""
        IF focus < 1 THEN focus = lastfocus
        IF focus > lastfocus THEN focus = 1
        f = 1
        FOR i = 1 TO 100
            IF o(i).typ THEN
                focusoffset = focus - f
                ideobjupdate o(i), focus, f, focusoffset, K$, altletter$, mB, mousedown, mouseup, mX, mY, info, mWHEEL
            END IF
        NEXT
        '-------- end of generic input response --------

        '-------- custom input response --------
        'max. processes spinners
        IF focus = mppSymUp AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(mppBox).txt)) + 1, 1, 128))
            idetxt(o(mppBox).txt) = a$: o(mppBox).v1 = LEN(a$)
        END IF
        IF focus = mppSymDn AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(mppBox).txt)) - 1, 1, 128))
            idetxt(o(mppBox).txt) = a$: o(mppBox).v1 = LEN(a$)
        END IF
        'max. processes text box (valid data check)
        a$ = idetxt(o(mppBox).txt): o(mppBox).inv = 1
        IF isuinteger(a$) _ANDALSO (VAL(a$) >= 1 AND VAL(a$) <= 128) THEN o(mppBox).inv = 0
        IF o(mppBox).inv THEN invdata = 1 'block confirmation, as long as invalid

        'ok & cancel buttons
        IF K$ = CHR$(27) OR (focus = caBut AND info <> 0) THEN EXIT FUNCTION
        IF K$ = CHR$(13) OR (focus = okBut AND info <> 0) THEN
            'blocked?
            IF invdata THEN
                retval = idemessagebox("Warning", "Confirmation has been blocked due to invalid settings.\nPlease check your inputs, look for highlighted boxes.", "#OK")
                PCOPY 2, 1: _CONTINUE
            END IF

            optChg% = _FALSE 'reset changed options indicator

            'adjust runtime variables
            v% = o(ocpChk).sel: IF v% <> 0 THEN v% = _TRUE
            IF OptimizeCppProgram <> v% THEN OptimizeCppProgram = v%: optChg% = _TRUE
            v% = o(sdsChk).sel: IF v% <> 0 THEN v% = _TRUE
            IF StripDebugSymbols <> v% THEN StripDebugSymbols = v%: optChg% = _TRUE
            v% = o(idiChk).sel: IF v% <> 0 THEN v% = _TRUE
            IF IncludeDebugInfo <> v% THEN IncludeDebugInfo = v%: optChg% = _TRUE

            v$ = idetxt(o(ecfBox).txt)
            IF ExtraCppFlags$ <> v$ THEN ExtraCppFlags$ = v$: optChg% = _TRUE
            v$ = idetxt(o(elfBox).txt)
            IF ExtraLinkerFlags$ <> v$ THEN ExtraLinkerFlags$ = v$: optChg% = _TRUE

            v% = VAL(idetxt(o(mppBox).txt))
            IF MaxParallelProcesses <> v% THEN MaxParallelProcesses = v%: optChg% = _TRUE

            IF os$ = "WIN" THEN
                v% = o(uscChk).sel: IF v% <> 0 THEN v% = _TRUE
                IF UseSystemMinGW <> v% THEN UseSystemMinGW = v%: optChg% = _TRUE
            END IF

            IF optChg% THEN
                'save changes
                WriteConfigSetting compilerSettingsSection$, "OptimizeCppProgram", BoolToTFString$(OptimizeCppProgram)
                WriteConfigSetting compilerSettingsSection$, "StripDebugSymbols", BoolToTFString$(StripDebugSymbols)
                WriteConfigSetting compilerSettingsSection$, "IncludeDebugInfo", BoolToTFString$(IncludeDebugInfo)

                WriteConfigSetting compilerSettingsSection$, "ExtraCppFlags", ExtraCppFlags$
                WriteConfigSetting compilerSettingsSection$, "ExtraLinkerFlags", ExtraLinkerFlags$

                WriteConfigSetting compilerSettingsSection$, "MaxParallelProcesses", _TOSTR$(MaxParallelProcesses)

                IF os$ = "WIN" THEN
                    WriteConfigSetting compilerSettingsSection$, "UseSystemMinGW", BoolToTFString$(UseSystemMinGW)
                    IF UseSystemMinGW THEN
                        retval = idemessagebox("Warning", "Using the system MinGW compiler may cause problems.", "#OK")
                        PCOPY 2, 1
                    END IF
                END IF

                'clean compiled files, since they may change due to the different settings
                PurgeTemporaryBuildFiles (os$), (MacOSX)

                ideCompilerSettingsBox = 1
            END IF
            EXIT FUNCTION
        END IF
        '-------- end of custom input response --------

        mousedown = 0
        mouseup = 0
    LOOP
END FUNCTION
FUNCTION idemessagebox (titlestr$, messagestr$, buttons$)

    '-------- generic dialog box header --------
    PCOPY 0, 2
    PCOPY 0, 1
    SCREEN , , 1, 0
    focus = 1
    DIM p AS idedbptype
    DIM o(1 TO 100) AS idedbotype
    DIM sep AS STRING * 1
    sep = CHR$(0)
    '-------- end of generic dialog box header --------

    '-------- init --------
    messagestr$ = StrReplace$(messagestr$, "\n", CHR$(10))
    MessageLines = 1
    DIM FullMessage$(1 TO 9)
    PrevScan = 1
    DO
        NextScan = INSTR(NextScan + 1, messagestr$, CHR$(10))
        IF NextScan > 0 THEN
            FullMessage$(MessageLines) = MID$(messagestr$, PrevScan, NextScan - PrevScan)
            tw = LEN(FullMessage$(MessageLines)) + 2
            IF tw > w THEN w = tw
            PrevScan = NextScan + 1
            IF MessageLines = UBOUND(FullMessage$) THEN EXIT DO
            MessageLines = MessageLines + 1
        ELSE
            FullMessage$(MessageLines) = MID$(messagestr$, PrevScan)
            tw = LEN(FullMessage$(MessageLines)) + 2
            IF tw > w THEN w = tw
            EXIT DO
        END IF
    LOOP

    IF buttons$ = "" THEN buttons$ = "#OK"
    totalButtons = 1
    FOR i = 1 TO LEN(buttons$)
        IF ASC(buttons$, i) = 59 THEN totalButtons = totalButtons + 1
    NEXT
    buttonsLen = LEN(buttons$) + totalButtons * 6

    i = 0
    w2 = LEN(titlestr$) + 4
    IF w < w2 THEN w = w2
    IF w < buttonsLen THEN w = buttonsLen
    IF w > idewx - 4 THEN w = idewx - 4
    idepar p, w, 3 + MessageLines, titlestr$

    i = i + 1
    o(i).typ = 3
    o(i).y = 3 + MessageLines
    o(i).txt = idenewtxt(StrReplace$(buttons$, ";", sep))
    o(i).dft = 1
    '-------- end of init --------

    '-------- generic init --------
    FOR i = 1 TO 100: o(i).par = p: NEXT 'set parent info of objects
    '-------- end of generic init --------

    DO 'main loop


        '-------- generic display dialog box & objects --------
        idedrawpar p
        f = 1: cx = 0: cy = 0
        FOR i = 1 TO 100
            IF o(i).typ THEN

                'prepare object
                o(i).foc = focus - f 'focus offset
                o(i).cx = 0: o(i).cy = 0
                idedrawobj o(i), f 'display object
                IF o(i).cx THEN cx = o(i).cx: cy = o(i).cy
            END IF
        NEXT i
        lastfocus = f - 1
        '-------- end of generic display dialog box & objects --------

        '-------- custom display changes --------
        COLOR 0, 7
        FOR i = 1 TO MessageLines
            IF LEN(FullMessage$(i)) > p.w - 2 THEN
                FullMessage$(i) = LEFT$(FullMessage$(i), p.w - 5) + STRING$(3, 250)
            END IF
            _PRINTSTRING (p.x + (w \ 2 - LEN(FullMessage$(i)) \ 2) + 1, p.y + 1 + i), FullMessage$(i)
        NEXT i
        '-------- end of custom display changes --------

        'update visual page and cursor position
        PCOPY 1, 0
        IF cx THEN SCREEN , , 0, 0: LOCATE cy, cx, 1: SCREEN , , 1, 0

        '-------- read input --------
        change = 0
        DO
            GetInput
            IF mWHEEL THEN change = 1
            IF KB THEN change = 1
            IF mCLICK THEN mousedown = 1: change = 1
            IF mRELEASE THEN mouseup = 1: change = 1
            IF mB THEN change = 1
            alt = KALT: IF alt <> oldalt THEN change = 1
            oldalt = alt
            _LIMIT 100
        LOOP UNTIL change
        IF alt AND NOT KCTRL THEN idehl = 1 ELSE idehl = 0
        'convert "alt+letter" scancode to letter's ASCII character
        altletter$ = ""
        IF alt AND NOT KCTRL THEN
            IF LEN(K$) = 1 THEN
                k = ASC(UCASE$(K$))
                IF k >= 65 AND k <= 90 THEN altletter$ = CHR$(k)
            END IF
        END IF
        SCREEN , , 0, 0: LOCATE , , 0: SCREEN , , 1, 0
        '-------- end of read input --------

        '-------- generic input response --------
        info = 0

        IF UCASE$(K$) >= "A" AND UCASE$(K$) <= "Z" THEN altletter$ = UCASE$(K$)

        IF K$ = "" THEN K$ = CHR$(255)
        IF KSHIFT = 0 AND K$ = CHR$(9) THEN focus = focus + 1
        IF (KSHIFT AND K$ = CHR$(9)) OR (INSTR(_OS$, "MAC") AND K$ = CHR$(25)) THEN focus = focus - 1: K$ = ""
        IF focus < 1 THEN focus = lastfocus
        IF focus > lastfocus THEN focus = 1
        f = 1
        FOR i = 1 TO 100
            t = o(i).typ
            IF t THEN
                focusoffset = focus - f
                ideobjupdate o(i), focus, f, focusoffset, K$, altletter$, mB, mousedown, mouseup, mX, mY, info, mWHEEL
            END IF
        NEXT
        '-------- end of generic input response --------

        'specific post controls
        IF K$ = CHR$(27) THEN EXIT FUNCTION

        IF K$ = CHR$(13) OR (info <> 0) THEN
            idemessagebox = focus
            ClearMouse
            EXIT FUNCTION
        END IF
        'end of custom controls

        mousedown = 0
        mouseup = 0
    LOOP
END FUNCTION
FUNCTION ideDisplayBox

    '-------- generic dialog box header --------
    PCOPY 0, 2
    PCOPY 0, 1
    SCREEN , , 1, 0
    focus = 1
    DIM p AS idedbptype
    DIM o(1 TO 100) AS idedbotype
    DIM sep AS STRING * 1
    sep = CHR$(0)
    '-------- end of generic dialog box header --------

    '-------- init dialog box & objects --------
    i = 0
    idepar p, 61, 18, "Display Settings"
    'manually adjust winpos in case display was set too large by accident
    p.x = (80 \ 2) - p.w \ 2: p.y = (25 \ 2) - p.h \ 2

    i = i + 1: wwBox = i
    o(i).typ = 1 'text box
    o(i).x = 3: o(i).y = 2: o(i).w = 10
    o(i).nam = idenewtxt("Window #width"): a2$ = _TOSTR$(idewx)
    o(i).txt = idenewtxt(a2$): o(i).v1 = LEN(a2$): o(i).blk = 6
    i = i + 1: wwSymUp = i
    o(i).typ = 5 'symbol button
    o(i).x = 24: o(i).y = 2
    o(i).txt = idenewtxt(CHR$(30)): o(i).rpt = 10
    i = i + 1: wwSymDn = i
    o(i).typ = 5 'symbol button
    o(i).x = 27: o(i).y = 2
    o(i).txt = idenewtxt(CHR$(31)): o(i).rpt = 10
    i = i + 1: whBox = i
    o(i).typ = 1 'text box
    o(i).x = 2: o(i).y = 5: o(i).w = 10
    o(i).nam = idenewtxt("Window #height"): a2$ = _TOSTR$(idewy + idesubwindow)
    o(i).txt = idenewtxt(a2$): o(i).v1 = LEN(a2$): o(i).blk = 6
    i = i + 1: whSymUp = i
    o(i).typ = 5 'symbol button
    o(i).x = 24: o(i).y = 5
    o(i).txt = idenewtxt(CHR$(30)): o(i).rpt = 10
    i = i + 1: whSymDn = i
    o(i).typ = 5 'symbol button
    o(i).x = 27: o(i).y = 5
    o(i).txt = idenewtxt(CHR$(31)): o(i).rpt = 10

    i = i + 1: rpChk = i
    o(i).typ = 4 'check box
    o(i).y = 7
    IF INSTR(_OS$, "WIN") > 0 OR INSTR(_OS$, "MAC") > 0 THEN
        o(i).nam = idenewtxt("#Remember position + size")
    ELSE
        o(i).nam = idenewtxt("#Remember size")
    END IF
    o(i).sel = ABS(IDEAutoPosition)

    i = i + 1: csBox = i
    o(i).typ = 1 'text box
    o(i).x = 33: o(i).y = 2
    o(i).nam = idenewtxt("Cursor #start"): a2$ = _TOSTR$(IDENormalCursorStart)
    o(i).txt = idenewtxt(a2$): o(i).v1 = LEN(a2$): o(i).blk = 6
    i = i + 1: csSymUp = i
    o(i).typ = 5 'symbol button
    o(i).x = 54: o(i).y = 2
    o(i).txt = idenewtxt(CHR$(30)): o(i).rpt = 10
    i = i + 1: csSymDn = i
    o(i).typ = 5 'symbol button
    o(i).x = 57: o(i).y = 2
    o(i).txt = idenewtxt(CHR$(31)): o(i).rpt = 10
    i = i + 1: ceBox = i
    o(i).typ = 1 'text box
    o(i).x = 35: o(i).y = 5
    o(i).nam = idenewtxt("Cursor #end"): a2$ = _TOSTR$(IDENormalCursorEnd)
    o(i).txt = idenewtxt(a2$): o(i).v1 = LEN(a2$): o(i).blk = 6
    i = i + 1: ceSymUp = i
    o(i).typ = 5 'symbol button
    o(i).x = 54: o(i).y = 5
    o(i).txt = idenewtxt(CHR$(30)): o(i).rpt = 10
    i = i + 1: ceSymDn = i
    o(i).typ = 5 'symbol button
    o(i).x = 57: o(i).y = 5
    o(i).txt = idenewtxt(CHR$(31)): o(i).rpt = 10

    i = i + 1: f8Chk = i
    o(i).typ = 4 'check box
    o(i).y = 9
    o(i).nam = idenewtxt("#Use _FONT 8")
    o(i).sel = ABS(IDEUseFont8)
    i = i + 1: cfChk = i
    o(i).typ = 4 'check box
    o(i).y = 10
    o(i).nam = idenewtxt("Use monospace #TTF, TTC, OTF, FNT, FON, PCF, BDF font:")
    o(i).sel = ABS(IDECustomFont)

    i = i + 1: cfBox = i
    o(i).typ = 1 'text box
    o(i).x = 7: o(i).y = 12
    o(i).nam = idenewtxt("#Font file"): a2$ = IDECustomFontFile$
    o(i).txt = idenewtxt(a2$): o(i).v1 = LEN(a2$): o(i).blk = 3
    i = i + 1: cfSymL = i
    o(i).typ = 5 'symbol button
    o(i).x = 57: o(i).y = 12
    o(i).txt = idenewtxt(CHR$(240))
    i = i + 1: cfsBox = i
    o(i).typ = 1 'text box
    o(i).x = 7: o(i).y = 15
    o(i).nam = idenewtxt("Font size in #pixels"): a2$ = _TOSTR$(IDECustomFontHeight)
    o(i).txt = idenewtxt(a2$): o(i).v1 = LEN(a2$): o(i).blk = 6
    i = i + 1: cfsSymUp = i
    o(i).typ = 5 'symbol button
    o(i).x = 54: o(i).y = 15
    o(i).txt = idenewtxt(CHR$(30)): o(i).rpt = 10
    i = i + 1: cfsSymDn = i
    o(i).typ = 5 'symbol button
    o(i).x = 57: o(i).y = 15
    o(i).txt = idenewtxt(CHR$(31)): o(i).rpt = 10

    i = i + 1: okBut = i: caBut = i + 1
    o(i).typ = 3 'action buttons
    o(i).y = 18
    o(i).txt = idenewtxt("#OK" + sep + "#Cancel"): o(i).dft = 1
    '-------- end of init dialog box & objects --------

    '-------- generic init --------
    FOR i = 1 TO 100: o(i).par = p: NEXT 'set parent info of objects
    '-------- end of generic init --------

    '-------- custom variables init --------
    tmpNormalCursorStart = IDENormalCursorStart
    tmpNormalCursorEnd = IDENormalCursorEnd
    '-------- end of custom variables init --------

    DO 'main loop

        '-------- generic display dialog box & objects --------
        idedrawpar p
        f = 1: cx = 0: cy = 0
        FOR i = 1 TO 100
            IF o(i).typ THEN
                'prepare object
                o(i).foc = focus - f 'focus offset
                o(i).cx = 0: o(i).cy = 0 'clear cursor pos
                IF i = focus _ANDALSO focus <> oldfocus THEN
                    oldfocus = focus
                    IF o(i).typ = 1 THEN 'if text box
                        'start with values selected upon getting focus
                        o(i).v1 = LEN(idetxt(o(i).txt)) 'selection len
                        IF o(i).v1 > 0 THEN o(i).issel = -1 ELSE o(i).issel = 0
                        o(focus).sx1 = 0 'selection start
                    END IF
                END IF
                idedrawobj o(i), f 'display object
                IF o(i).cx THEN cx = o(i).cx: cy = o(i).cy 'get new cursor pos
            END IF
        NEXT i
        lastfocus = f - 1
        '-------- end of generic display dialog box & objects --------

        '-------- custom display changes --------
        LOCATE , , , tmpNormalCursorStart, tmpNormalCursorEnd
        '-------- end of custom display changes --------

        'update visual page and cursor position
        PCOPY 1, 0
        IF cx THEN SCREEN , , 0, 0: LOCATE cy, cx, 1: SCREEN , , 1, 0

        '-------- read input --------
        change = 0
        DO
            GetInput
            IF mWHEEL THEN change = 1
            IF KB THEN change = 1
            IF mCLICK THEN mousedown = 1: change = 1
            IF mRELEASE THEN mouseup = 1: change = 1
            IF mB THEN change = 1
            alt = KALT: IF alt <> oldalt THEN change = 1
            oldalt = alt
            _LIMIT 100
        LOOP UNTIL change
        IF alt AND NOT KCTRL THEN idehl = 1 ELSE idehl = 0
        'convert "alt+letter" scancode to letter's ASCII character
        altletter$ = ""
        IF alt AND NOT KCTRL THEN
            IF LEN(K$) = 1 THEN
                k = ASC(UCASE$(K$))
                IF k >= 65 AND k <= 90 THEN altletter$ = CHR$(k)
            END IF
        END IF
        SCREEN , , 0, 0: LOCATE , , 0: SCREEN , , 1, 0
        '-------- end of read input --------

        '-------- generic input response --------
        info = 0: invdata = 0
        IF K$ = "" THEN K$ = CHR$(255)
        IF KSHIFT = 0 AND K$ = CHR$(9) THEN focus = focus + 1
        IF (KSHIFT AND K$ = CHR$(9)) OR (INSTR(_OS$, "MAC") AND K$ = CHR$(25)) THEN focus = focus - 1: K$ = ""
        IF focus < 1 THEN focus = lastfocus
        IF focus > lastfocus THEN focus = 1
        f = 1
        FOR i = 1 TO 100
            IF o(i).typ THEN
                focusoffset = focus - f
                ideobjupdate o(i), focus, f, focusoffset, K$, altletter$, mB, mousedown, mouseup, mX, mY, info, mWHEEL
            END IF
        NEXT
        '-------- end of generic input response --------

        '-------- custom input response --------
        'width spinners
        IF focus = wwSymUp AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(wwBox).txt)) + 1, 80, 999))
            idetxt(o(wwBox).txt) = a$: o(wwBox).v1 = LEN(a$)
        END IF
        IF focus = wwSymDn AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(wwBox).txt)) - 1, 80, 999))
            idetxt(o(wwBox).txt) = a$: o(wwBox).v1 = LEN(a$)
        END IF
        'width text box (valid data check)
        a$ = idetxt(o(wwBox).txt): o(wwBox).inv = 1
        IF isuinteger(a$) _ANDALSO (VAL(a$) >= 80 AND VAL(a$) <= 999) THEN o(wwBox).inv = 0
        IF o(wwBox).inv THEN invdata = 1 'block confirmation, as long as invalid
        'height spinners
        IF focus = whSymUp AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(whBox).txt)) + 1, 25, 999))
            idetxt(o(whBox).txt) = a$: o(whBox).v1 = LEN(a$)
        END IF
        IF focus = whSymDn AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(whBox).txt)) - 1, 25, 999))
            idetxt(o(whBox).txt) = a$: o(whBox).v1 = LEN(a$)
        END IF
        'height text box (valid data check)
        a$ = idetxt(o(whBox).txt): o(whBox).inv = 1
        IF isuinteger(a$) _ANDALSO (VAL(a$) >= 25 AND VAL(a$) <= 999) THEN o(whBox).inv = 0
        IF o(whBox).inv THEN invdata = 1 'block confirmation, as long as invalid

        'cursor start spinners
        IF focus = csSymUp AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(csBox).txt)) + 1, 0, 31))
            idetxt(o(csBox).txt) = a$: o(csBox).v1 = LEN(a$)
        END IF
        IF focus = csSymDn AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(csBox).txt)) - 1, 0, 31))
            idetxt(o(csBox).txt) = a$: o(csBox).v1 = LEN(a$)
        END IF
        'cursor start text box (valid data check)
        a$ = idetxt(o(csBox).txt): o(csBox).inv = 1
        IF isuinteger(a$) _ANDALSO (VAL(a$) >= 0 AND VAL(a$) <= 31) THEN o(csBox).inv = 0
        IF o(csBox).inv THEN
            invdata = 1 'block confirmation, as long as invalid
        ELSE
            tmpNormalCursorStart = VAL(a$)
        END IF
        'cursor end spinners
        IF focus = ceSymUp AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(ceBox).txt)) + 1, 0, 31))
            idetxt(o(ceBox).txt) = a$: o(ceBox).v1 = LEN(a$)
        END IF
        IF focus = ceSymDn AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(ceBox).txt)) - 1, 0, 31))
            idetxt(o(ceBox).txt) = a$: o(ceBox).v1 = LEN(a$)
        END IF
        'cursor end text box (valid data check)
        a$ = idetxt(o(ceBox).txt): o(ceBox).inv = 1
        IF isuinteger(a$) _ANDALSO (VAL(a$) >= 0 AND VAL(a$) <= 31) THEN o(ceBox).inv = 0
        IF o(ceBox).inv THEN
            invdata = 1 'block confirmation, as long as invalid
        ELSE
            tmpNormalCursorEnd = VAL(a$)
        END IF

        'font 8 check box
        IF focus = f8Chk AND o(f8Chk).sel = 1 THEN 'goes on?
            o(cfChk).sel = 0 'implies custom font off
        END IF
        'custom font check box
        IF focus = cfChk AND o(cfChk).sel = 1 THEN 'goes on?
            o(f8Chk).sel = 0 'implies font 8 off
        END IF

        'custom font selector
        IF focus = cfSymL AND info <> 0 THEN
            a$ = idefiledialog$("*.tt*", 3)
            IF a$ <> "C" THEN
                a$ = RemoveDoubleSlashes$(a$)
                idetxt(o(cfBox).txt) = a$: o(cfBox).v1 = LEN(a$)
            END IF
            PCOPY 2, 1: K$ = ""
            o(f8Chk).sel = 0 'implies font 8 off
            o(cfChk).sel = 1 'and custom font on
        END IF
        'custom font text box (valid data check)
        a$ = idetxt(o(cfBox).txt): o(cfBox).inv = 1
        IF LEN(a$) >= 1 AND LEN(a$) <= 1024 _ANDALSO _FILEEXISTS(a$) THEN o(cfBox).inv = 0
        IF o(cfBox).inv THEN invdata = 1 'block confirmation, as long as invalid
        IF focus = cfBox THEN
            IF o(cfBox).inv = 0 THEN 'if valid, then
                o(f8Chk).sel = 0 'manual input implies font 8 off
                o(cfChk).sel = 1 'and custom font on
            END IF
        END IF
        'custom font size spinners
        IF focus = cfsSymUp AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(cfsBox).txt)) + 1, 8, 99))
            idetxt(o(cfsBox).txt) = a$: o(cfsBox).v1 = LEN(a$)
            o(f8Chk).sel = 0 'implies font 8 off
            o(cfChk).sel = 1 'and custom font on
        END IF
        IF focus = cfsSymDn AND info <> 0 THEN
            a$ = _TOSTR$(_CLAMP(VAL(idetxt(o(cfsBox).txt)) - 1, 8, 99))
            idetxt(o(cfsBox).txt) = a$: o(cfsBox).v1 = LEN(a$)
            o(f8Chk).sel = 0 'implies font 8 off
            o(cfChk).sel = 1 'and custom font on
        END IF
        'custom font size text box (valid data check)
        a$ = idetxt(o(cfsBox).txt): o(cfsBox).inv = 1
        IF isuinteger(a$) _ANDALSO (VAL(a$) >= 8 AND VAL(a$) <= 99) THEN o(cfsBox).inv = 0
        IF o(cfsBox).inv THEN invdata = 1 'block confirmation, as long as invalid
        IF focus = cfsBox THEN
            IF o(cfsBox).inv = 0 THEN 'if valid, then
                o(f8Chk).sel = 0 'manual input implies font 8 off
                o(cfChk).sel = 1 'and custom font on
            END IF
        END IF

        'ok & cancel buttons
        IF K$ = CHR$(27) OR (focus = caBut AND info <> 0) THEN EXIT FUNCTION
        IF K$ = CHR$(13) OR (focus = okBut AND info <> 0) THEN
            'blocked?
            IF invdata THEN
                retval = idemessagebox("Warning", "Confirmation has been blocked due to invalid settings.\nPlease check your inputs, look for highlighted boxes.", "#OK")
                PCOPY 2, 1: _CONTINUE
            END IF

            optChg% = _FALSE 'reset changed options indicator
            fonChg% = _FALSE 'reset changed custom font indicator

            'adjust runtime variables
            v% = VAL(idetxt(o(wwBox).txt))
            IF idewx <> v% THEN idewx = v%: optChg% = _TRUE
            v% = VAL(idetxt(o(whBox).txt))
            IF idewy <> v% - idesubwindow THEN idewy = v% - idesubwindow: optChg% = _TRUE

            v% = o(rpChk).sel: IF v% <> 0 THEN v% = _TRUE
            IF IDEAutoPosition <> v% THEN IDEAutoPosition = v%: optChg% = _TRUE

            v% = VAL(idetxt(o(csBox).txt))
            IF IDENormalCursorStart <> v% THEN IDENormalCursorStart = v%: optChg% = _TRUE
            v% = VAL(idetxt(o(ceBox).txt))
            IF IDENormalCursorEnd <> v% THEN IDENormalCursorEnd = v%: optChg% = _TRUE

            v% = o(f8Chk).sel: IF v% <> 0 THEN v% = _TRUE
            IF IDEUseFont8 <> v% THEN IDEUseFont8 = v%: optChg% = _TRUE
            v% = o(cfChk).sel: IF v% <> 0 THEN v% = _TRUE
            IF IDECustomFont <> v% THEN IDECustomFont = v%: fonChg% = _TRUE: optChg% = _TRUE

            v$ = idetxt(o(cfBox).txt)
            IF IDECustomFontFile$ <> v$ THEN
                IDECustomFontFile$ = v$
                IF IDECustomFont THEN fonChg% = _TRUE: optChg% = _TRUE
            END IF
            v% = VAL(idetxt(o(cfsBox).txt))
            IF IDECustomFontHeight <> v% THEN
                IDECustomFontHeight = v%
                IF IDECustomFont THEN fonChg% = _TRUE: optChg% = _TRUE
            END IF

            IF fonChg% THEN
                IF o(cfChk).sel = 0 THEN 'custom font now off?
                    IF IDEUseFont8 THEN _FONT 8 ELSE _FONT 16
                    IF IDECustomFontHandle > 0 THEN
                        _FREEFONT IDECustomFontHandle
                        IDECustomFontHandle = 0
                    END IF
                ELSE 'custom font now on, or changed
                    oldhandle = IDECustomFontHandle
                    IDECustomFontHandle = _LOADFONT(IDECustomFontFile$, IDECustomFontHeight, "MONOSPACE")
                    IF IDECustomFontHandle < 1 THEN
                        retval = idemessagebox("Custom font not found!", "Your desired font was not found at the specified\nlocation, is not usable as monospace or is of\nunsupported format. Please check your inputs.", "#OK")
                        IDECustomFontHandle = oldhandle 'old handle remains active
                        PCOPY 2, 1: _CONTINUE
                    ELSE
                        _FONT IDECustomFontHandle
                        IF oldhandle > 0 THEN _FREEFONT oldhandle
                    END IF
                END IF
            END IF

            IF optChg% THEN
                'save changes
                WriteConfigSetting windowSettingsSection$, "IDE_Width", _TOSTR$(idewx)
                WriteConfigSetting windowSettingsSection$, "IDE_Height", _TOSTR$(idewy)

                WriteConfigSetting displaySettingsSection$, "IDE_AutoPosition", BoolToTFString$(IDEAutoPosition)

                WriteConfigSetting displaySettingsSection$, "IDE_NormalCursorStart", _TOSTR$(IDENormalCursorStart)
                WriteConfigSetting displaySettingsSection$, "IDE_NormalCursorEnd", _TOSTR$(IDENormalCursorEnd)

                WriteConfigSetting displaySettingsSection$, "IDE_UseFont8", BoolToTFString$(IDEUseFont8)
                WriteConfigSetting displaySettingsSection$, "IDE_CustomFont", BoolToTFString$(IDECustomFont)

                WriteConfigSetting displaySettingsSection$, "IDE_CustomFont$", IDECustomFontFile$
                WriteConfigSetting displaySettingsSection$, "IDE_CustomFontSize", _TOSTR$(IDECustomFontHeight)

                ideDisplayBox = 1
            END IF
            EXIT FUNCTION
        END IF
        '-------- end of custom input response --------

        mousedown = 0
        mouseup = 0
    LOOP
END FUNCTION
FUNCTION idechoosecolorsbox
    DIM bkpIDECommentColor AS _UNSIGNED LONG, bkpIDEMetaCommandColor AS _UNSIGNED LONG
    DIM bkpIDEQuoteColor AS _UNSIGNED LONG, bkpIDETextColor AS _UNSIGNED LONG
    DIM bkpIDEBackgroundColor AS _UNSIGNED LONG, bkpIDEKeywordColor AS _UNSIGNED LONG
    DIM bkpIDEBackgroundColor2 AS _UNSIGNED LONG, bkpIDENumbersColor AS _UNSIGNED LONG
    DIM bkpIDEBracketHighlightColor AS _UNSIGNED LONG, bkpIDEChromaColor AS _UNSIGNED LONG

    TotalItems = 10
    DIM SelectionIndicator$(1 TO TotalItems)
    bkpIDECommentColor = IDECommentColor
    bkpIDEMetaCommandColor = IDEMetaCommandColor
    bkpIDEQuoteColor = IDEQuoteColor
    bkpIDETextColor = IDETextColor
    bkpIDEKeywordColor = IDEKeywordColor
    bkpIDENumbersColor = IDENumbersColor
    bkpIDEBackgroundColor = IDEBackgroundColor
    bkpIDEBackgroundColor2 = IDEBackgroundColor2
    bkpIDEBracketHighlightColor = IDEBracketHighlightColor
    bkpIDEChromaColor = IDEChromaColor

    '-------- generic dialog box header --------
    PCOPY 0, 2
    PCOPY 0, 1
    SCREEN , , 1, 0
    focus = 1
    DIM p AS idedbptype
    DIM o(1 TO 100) AS idedbotype
    DIM sep AS STRING * 1
    sep = CHR$(0)
    '-------- end of generic dialog box header --------

    '-------- init --------
    LoadColorSchemes
    _PALETTECOLOR 5, &HFF00A800, 0 'Original green may have been changed by the Help System, so 5 is now green

    i = 0
    idepar p, 73, 20, "IDE Colors"

    l$ = CHR$(16) + "Normal Text"
    l$ = l$ + sep + " Keywords"
    l$ = l$ + sep + " Numbers"
    l$ = l$ + sep + " Strings"
    l$ = l$ + sep + " Metacommand/custom keywords"
    l$ = l$ + sep + " Comments"
    l$ = l$ + sep + " Background"
    l$ = l$ + sep + " Current line background"
    l$ = l$ + sep + " Bracket/selection highlight"
    l$ = l$ + sep + " Menus and dialogs"
    i = i + 1
    o(i).typ = 2
    o(i).y = 4
    o(i).w = 30: o(i).h = 10
    o(i).txt = idenewtxt(l$)
    o(i).sel = 1
    SelectedITEM = 1
    PrevFocus = 1
    o(i).nam = idenewtxt("#Item:")

    a2$ = _TOSTR$(_RED32(IDETextColor))
    i = i + 1
    o(i).typ = 1
    o(i).x = 66
    o(i).y = 5
    o(i).txt = idenewtxt(a2$)
    o(i).v1 = LEN(a2$)
    o(i).issel = -1
    o(i).sx1 = 0

    a2$ = _TOSTR$(_GREEN32(IDETextColor))
    i = i + 1
    o(i).typ = 1
    o(i).x = 66
    o(i).y = 8
    o(i).txt = idenewtxt(a2$)
    o(i).v1 = LEN(a2$)
    o(i).issel = -1
    o(i).sx1 = 0

    a2$ = _TOSTR$(_BLUE32(IDETextColor))
    i = i + 1
    o(i).typ = 1
    o(i).x = 66
    o(i).y = 11
    o(i).txt = idenewtxt(a2$)
    o(i).v1 = LEN(a2$)
    o(i).issel = -1
    o(i).sx1 = 0

    i = i + 1
    o(i).typ = 4 'check box
    o(i).y = 16
    o(i).nam = idenewtxt("#Highlight brackets")
    IF BracketHighlight THEN o(i).sel = 1

    i = i + 1
    o(i).typ = 4 'check box
    o(i).y = 17
    o(i).nam = idenewtxt("#Multi-highlight (selection)")
    IF MultiHighlight THEN o(i).sel = 1

    i = i + 1
    o(i).typ = 4 'check box
    o(i).y = 18
    o(i).nam = idenewtxt("Highlight #keywords and numbers")
    IF KeywordHighlight THEN o(i).sel = 1

    i = i + 1
    o(i).typ = 3
    o(i).y = 20
    o(i).txt = idenewtxt("#OK" + sep + "Restore #Defaults" + sep + "#Cancel")
    o(i).dft = 1

    result = ReadConfigSetting(colorSettingsSection$, "SchemeID", value$)
    SchemeID = VAL(value$)
    IF SchemeID > TotalColorSchemes THEN SchemeID = 0

    IF SchemeID = 0 THEN
        a2$ = "User-defined"
    ELSE
        'Validate this scheme first
        FoundPipe = INSTR(ColorSchemes$(SchemeID), "|")
        IF FoundPipe > 0 THEN
            IF LEN(MID$(ColorSchemes$(SchemeID), FoundPipe + 1)) = 90 THEN
                a2$ = LEFT$(ColorSchemes$(SchemeID), FoundPipe - 1)
            ELSE
                SchemeID = 0
                a2$ = "User-defined"
            END IF
        ELSE
            SchemeID = 0
            a2$ = "User-defined"
        END IF
    END IF
    i = i + 1
    o(i).typ = 1
    o(i).x = 9
    o(i).y = 2
    o(i).w = 38
    o(i).nam = idenewtxt("#Scheme")
    o(i).txt = idenewtxt(a2$)
    o(i).v1 = LEN(a2$)

    '-------- end of init --------

    '-------- generic init --------
    FOR i = 1 TO 100: o(i).par = p: NEXT 'set parent info of objects
    '-------- end of generic init --------

    DO 'main loop

        '-------- generic display dialog box & objects --------
        idedrawpar p
        f = 1: cx = 0: cy = 0
        FOR i = 1 TO 100
            IF o(i).typ THEN

                'prepare object
                o(i).foc = focus - f 'focus offset
                o(i).cx = 0: o(i).cy = 0
                idedrawobj o(i), f 'display object
                IF o(i).cx THEN cx = o(i).cx: cy = o(i).cy
            END IF
        NEXT i
        lastfocus = f - 1
        '-------- end of generic display dialog box & objects --------

        '-------- custom display changes --------
        'Color scheme selection arrows:
        LOCATE p.y + 2, p.x + 2
        IF mY = p.y + 2 AND mX >= p.x + 2 AND mX <= p.x + 4 THEN COLOR 15, 0 ELSE COLOR 15, 2
        IF SchemeID <= 1 THEN COLOR 7, 2
        PRINT " " + CHR$(17) + " ";
        IF mY = p.y + 2 AND mX >= p.x + 5 AND mX <= p.x + 7 THEN COLOR 15, 0 ELSE COLOR 15, 2
        IF SchemeID = LastValidColorScheme THEN COLOR 7, 2
        PRINT " " + CHR$(16) + " ";

        'Color scheme Save and Erase buttons:
        LOCATE p.y + 2, p.x + 60
        IF mY = p.y + 2 AND mX >= p.x + 60 AND mX <= p.x + 65 THEN COLOR 15, 0 ELSE COLOR 15, 2
        IF SchemeID > 0 AND SchemeID <= PresetColorSchemes THEN COLOR 7, 2 'Disable if preset scheme
        PRINT " Save ";
        IF mY = p.y + 2 AND mX >= p.x + 66 AND mX <= p.x + 72 THEN COLOR 15, 0 ELSE COLOR 15, 2
        IF SchemeID <= PresetColorSchemes THEN COLOR 7, 2 'Disable if preset scheme or unsaved user-defined
        PRINT " Erase ";

        COLOR , 7

        _PALETTECOLOR 1, IDEBackgroundColor, 0
        _PALETTECOLOR 2, _RGB32(84, 84, 84), 0 'dark gray - help system and interface details
        _PALETTECOLOR 4, IDEErrorColor, 0
        _PALETTECOLOR 6, IDEBackgroundColor2, 0
        _PALETTECOLOR 7, IDEChromaColor, 0
        _PALETTECOLOR 8, IDENumbersColor, 0
        _PALETTECOLOR 10, IDEMetaCommandColor, 0
        _PALETTECOLOR 11, IDECommentColor, 0
        _PALETTECOLOR 12, IDEKeywordColor, 0
        _PALETTECOLOR 13, IDETextColor, 0
        _PALETTECOLOR 14, IDEQuoteColor, 0

        COLOR 0: LOCATE p.y + 5, p.x + 36: PRINT "R: ";
        COLOR 4: PRINT STRING$(26, 196);
        slider$ = CHR$(197)
        T = VAL(idetxt(o(2).txt)): r = ((T / 255) * 26)
        IF T = 0 THEN slider$ = CHR$(195)
        IF T = 255 THEN slider$ = CHR$(180)
        _PRINTSTRING (p.x + 39 + r, p.y + 5), slider$

        COLOR 0: LOCATE p.y + 8, p.x + 36: PRINT "G: ";
        COLOR 5: PRINT STRING$(26, 196);
        slider$ = CHR$(197)
        T = VAL(idetxt(o(3).txt)): r = ((T / 255) * 26)
        IF T = 0 THEN slider$ = CHR$(195)
        IF T = 255 THEN slider$ = CHR$(180)
        _PRINTSTRING (p.x + 39 + r, p.y + 8), slider$

        COLOR 0: LOCATE p.y + 11, p.x + 36: PRINT "B: ";
        COLOR 9: PRINT STRING$(26, 196);
        slider$ = CHR$(197)
        T = VAL(idetxt(o(4).txt)): r = ((T / 255) * 26)
        IF T = 0 THEN slider$ = CHR$(195)
        IF T = 255 THEN slider$ = CHR$(180)
        _PRINTSTRING (p.x + 39 + r, p.y + 11), slider$

        SELECT EVERYCASE SelectedITEM
            CASE 1 TO 9
                COLOR 7, 1
                _PRINTSTRING (p.x + 39, p.y + 13), CHR$(218) + STRING$(25, 196)
                _PRINTSTRING (p.x + 39, p.y + 14), CHR$(179) + SPACE$(25)
                _PRINTSTRING (p.x + 39, p.y + 15), CHR$(179) + SPACE$(25)
            CASE 1: COLOR 13, 1: SampleText$ = "myVar% = " 'Normal text
            CASE 2: COLOR 12, 1: SampleText$ = "CLS: PRINT" 'Keywords
            CASE 3: COLOR 13, 1: SampleText$ = "myVar% = " 'Normal text
            CASE 4: COLOR 14, 1: SampleText$ = SPACE$(6) + CHR$(34) + "Hello, world!" + CHR$(34) 'Strings
            CASE 5: COLOR 10, 1: SampleText$ = "'$DYNAMIC" 'Metacommands
            CASE 6: COLOR 11, 1: SampleText$ = "'TODO: review this block" 'Comments
            CASE 7: COLOR 1, 1: SampleText$ = "" 'Background
            CASE 8: COLOR 6, 6: SampleText$ = SPACE$(25) 'Current line background
            CASE 9
                COLOR 6, 6: SampleText$ = "" 'Bracket highlight
                _PALETTECOLOR 6, IDEBracketHighlightColor, 0
            CASE 10
                COLOR 0, 7
                _PRINTSTRING (p.x + 39, p.y + 13), CHR$(218) + STRING$(24, 196) + CHR$(191)
                _PRINTSTRING (p.x + 39, p.y + 14), CHR$(179) + SPACE$(24) + CHR$(179)
                _PRINTSTRING (p.x + 39, p.y + 15), CHR$(192) + STRING$(24, 196) + CHR$(217)
                SampleText$ = " Open...         Ctrl+O "
        END SELECT

        _PRINTSTRING (p.x + 40, p.y + 14), SampleText$
        IF SelectedITEM = 1 OR SelectedITEM = 3 THEN
            COLOR 8, 1
            _PRINTSTRING (p.x + 49, p.y + 14), "5"
        ELSEIF SelectedITEM = 2 THEN
            COLOR 13, 1
            _PRINTSTRING (p.x + 51, p.y + 14), "myVar%"
        ELSEIF SelectedITEM = 4 THEN
            COLOR 12, 1
            _PRINTSTRING (p.x + 40, p.y + 14), "PRINT"
        ELSEIF SelectedITEM = 5 THEN
            COLOR 11, 1
            _PRINTSTRING (p.x + 40, p.y + 14), "'"
        ELSEIF SelectedITEM = 9 THEN
            LOCATE p.y + 14, p.x + 40
            COLOR 13, 1: PRINT "myVar% = ";
            COLOR 12: PRINT "INT RND";
            LOCATE p.y + 14, p.x + 52
            COLOR 13, 6: PRINT "(";
            LOCATE p.y + 14, p.x + 56
            PRINT ")";
        ELSEIF SelectedITEM = 10 THEN
            COLOR 15, 7
            _PRINTSTRING (p.x + 41, p.y + 14), "O"
        END IF
        '-------- end of custom display changes --------

        'update visual page and cursor position
        PCOPY 1, 0
        IF cx THEN SCREEN , , 0, 0: LOCATE cy, cx, 1: SCREEN , , 1, 0

        '-------- read input --------
        change = 0
        DO
            GetInput
            IF mWHEEL THEN change = 1
            IF KB THEN change = 1
            IF mCLICK THEN mousedown = 1: change = 1
            IF mRELEASE THEN mouseup = 1: change = 1
            IF mB THEN change = 1
            IF mX <> prev.mX OR mY <> prev.mY THEN change = 1: prev.mX = mX: prev.mY = mY
            alt = KALT: IF alt <> oldalt THEN change = 1
            oldalt = alt
            _LIMIT 100
        LOOP UNTIL change
        IF alt AND NOT KCTRL THEN idehl = 1 ELSE idehl = 0
        'convert "alt+letter" scancode to letter's ASCII character
        altletter$ = ""
        IF alt AND NOT KCTRL THEN
            IF LEN(K$) = 1 THEN
                k = ASC(UCASE$(K$))
                IF k >= 65 AND k <= 90 THEN altletter$ = CHR$(k)
            END IF
        END IF
        SCREEN , , 0, 0: LOCATE , , 0: SCREEN , , 1, 0
        '-------- end of read input --------

        '-------- generic input response --------
        info = 0
        IF K$ = "" THEN K$ = CHR$(255)
        IF KSHIFT = 0 AND K$ = CHR$(9) THEN focus = focus + 1
        IF (KSHIFT AND K$ = CHR$(9)) OR (INSTR(_OS$, "MAC") AND K$ = CHR$(25)) THEN focus = focus - 1: K$ = ""
        IF focus < 1 THEN focus = lastfocus
        IF focus > lastfocus THEN focus = 1
        f = 1
        FOR i = 1 TO 100
            T = o(i).typ
            IF T THEN
                focusoffset = focus - f
                ideobjupdate o(i), focus, f, focusoffset, K$, altletter$, mB, mousedown, mouseup, mX, mY, info, mWHEEL
            END IF
        NEXT
        '-------- end of generic input response --------

        'specific post controls
        IF focus <> PrevFocus THEN
            'Always start with RGB values AND scheme name selected upon getting focus
            PrevFocus = focus
            IF (focus >= 2 AND focus <= 4) OR focus = 11 THEN
                IF focus = 11 THEN tfocus = 9 ELSE tfocus = focus
                o(tfocus).v1 = LEN(idetxt(o(tfocus).txt))
                IF o(tfocus).v1 > 0 THEN o(tfocus).issel = -1
                o(tfocus).sx1 = 0
                IF (tfocus >= 2 AND tfocus <= 4) THEN prevTB.value$ = idetxt(o(tfocus).txt)
            END IF
        ELSEIF focus = PrevFocus THEN
            'Check if new values have been entered into textboxes
            IF focus >= 2 AND focus <= 4 THEN
                IF prevTB.value$ <> idetxt(o(focus).txt) THEN
                    GOSUB NewUserScheme
                    prevTB.value$ = idetxt(o(focus).txt)
                END IF
            END IF
        END IF

        'Save and Erase color scheme (Buttons):
        IF (SchemeID = 0 OR SchemeID > PresetColorSchemes) AND mCLICK THEN
            IF mY = p.y + 2 AND mX >= p.x + 60 AND mX <= p.x + 65 THEN
                'Save
                IF SchemeID = 0 THEN
                    SaveNew:
                    SchemeString$ = LTRIM$(RTRIM$(idetxt(o(9).txt)))
                    IF LEN(SchemeString$) = 0 THEN SchemeString$ = "User-defined"
                    'Find the next free scheme index
                    i = 0
                    DO
                        i = i + 1
                        result = ReadConfigSetting(colorSchemesSection$, "Scheme" + _TOSTR$(i) + "$", value$)
                        IF value$ = "" OR value$ = "0" THEN EXIT DO
                    LOOP

                    'Build scheme string
                    SchemeString$ = SchemeString$ + "|"
                    FOR j = 1 TO 10
                        CurrentColor~& = GetCurrentColor(j)
                        r$ = _TOSTR$(_RED32(CurrentColor~&)): r$ = STRING$(3 - LEN(r$), "0") + r$
                        g$ = _TOSTR$(_GREEN32(CurrentColor~&)): g$ = STRING$(3 - LEN(g$), "0") + g$
                        b$ = _TOSTR$(_BLUE32(CurrentColor~&)): b$ = STRING$(3 - LEN(b$), "0") + b$
                        SchemeString$ = SchemeString$ + r$ + g$ + b$
                    NEXT j

                    'Save user scheme
                    WriteConfigSetting colorSchemesSection$, "Scheme" + _TOSTR$(i) + "$", SchemeString$
                    LoadColorSchemes
                    SchemeID = PresetColorSchemes + i
                    ChangedScheme = _TRUE
                    GOTO ApplyScheme
                ELSE
                    FoundPipe = INSTR(ColorSchemes$(SchemeID), "|")
                    SchemeString$ = LEFT$(ColorSchemes$(SchemeID), FoundPipe - 1)

                    IF SchemeString$ <> LTRIM$(RTRIM$(idetxt(o(9).txt))) THEN
                        'User wants to save the current SchemeID under a different name
                        GOTO SaveNew
                    END IF

                    i = SchemeID - PresetColorSchemes
                    SchemeString$ = SchemeString$ + "|"

                    'Build scheme string
                    FOR j = 1 TO 10
                        CurrentColor~& = GetCurrentColor(j)
                        r$ = _TOSTR$(_RED32(CurrentColor~&)): r$ = STRING$(3 - LEN(r$), "0") + r$
                        g$ = _TOSTR$(_GREEN32(CurrentColor~&)): g$ = STRING$(3 - LEN(g$), "0") + g$
                        b$ = _TOSTR$(_BLUE32(CurrentColor~&)): b$ = STRING$(3 - LEN(b$), "0") + b$
                        SchemeString$ = SchemeString$ + r$ + g$ + b$
                    NEXT j

                    'Save user scheme
                    WriteConfigSetting colorSchemesSection$, "Scheme" + _TOSTR$(i) + "$", SchemeString$
                    LoadColorSchemes
                    SchemeID = PresetColorSchemes + i
                    ChangedScheme = _TRUE
                    GOTO ApplyScheme
                END IF
                o(9).v1 = LEN(idetxt(o(9).txt))
                o(9).issel = -1
                o(9).sx1 = 0
            ELSEIF mY = p.y + 2 AND mX >= p.x + 66 AND mX <= p.x + 72 THEN
                'Erase
                IF SchemeID > PresetColorSchemes THEN
                    what$ = ideyesnobox("Erase color scheme", "This cannot be undone. Erase scheme?")
                    K$ = ""
                    IF what$ = "Y" THEN
                        i = SchemeID - PresetColorSchemes
                        WriteConfigSetting colorSchemesSection$, "Scheme" + _TOSTR$(i) + "$", "0"
                        LoadColorSchemes
                        SchemeID = SchemeID - 1
                        ChangedScheme = _TRUE
                        SchemeArrow = -1
                        GOTO ValidateScheme
                    END IF
                END IF
            END IF
        END IF

        'Scheme selection arrows:
        ChangedScheme = _FALSE
        SchemeArrow = 0
        IF (mCLICK AND mY = p.y + 2 AND mX >= p.x + 2 AND mX <= p.x + 4) OR _
           (K$ = CHR$(0) + CHR$(75) AND (focus = 1)) THEN
            SchemeArrow = -1
            IF SchemeID = 0 THEN
                ChangedScheme = _TRUE
                GOTO LoadDefaultScheme
            ELSE
                IF SchemeID > 1 THEN SchemeID = SchemeID - 1: ChangedScheme = _TRUE
            END IF
        ELSEIF (mCLICK AND mY = p.y + 2 AND mX >= p.x + 5 AND mX <= p.x + 7) OR _
               (K$ = CHR$(0) + CHR$(77) AND (focus = 1)) THEN
            SchemeArrow = 1
            IF SchemeID = 0 THEN
                ChangedScheme = _TRUE
                GOTO LoadDefaultScheme
            ELSE
                IF SchemeID < TotalColorSchemes THEN SchemeID = SchemeID + 1: ChangedScheme = _TRUE
            END IF
        END IF

        IF ChangedScheme THEN
            'Validate this scheme first
            IF SchemeArrow = 0 THEN SchemeArrow = 1
            ValidateScheme:
            FoundPipe = INSTR(ColorSchemes$(SchemeID), "|")
            IF FoundPipe > 0 THEN
                IF LEN(MID$(ColorSchemes$(SchemeID), FoundPipe + 1)) = 90 THEN
                    a2$ = LEFT$(ColorSchemes$(SchemeID), FoundPipe - 1)
                ELSE
                    SchemeID = SchemeID + SchemeArrow
                    IF SchemeID > TotalColorSchemes THEN SchemeID = TotalColorSchemes: SchemeArrow = -1
                    IF SchemeID < 1 THEN SchemeID = 1
                    GOTO ValidateScheme
                END IF
            ELSE
                SchemeID = SchemeID + SchemeArrow
                IF SchemeID > TotalColorSchemes THEN SchemeID = TotalColorSchemes: SchemeArrow = -1
                IF SchemeID < 1 THEN SchemeID = 1
                GOTO ValidateScheme
            END IF
            ApplyScheme:
            FoundPipe = INSTR(ColorSchemes$(SchemeID), "|")
            idetxt(o(9).txt) = LEFT$(ColorSchemes$(SchemeID), FoundPipe - 1)
            o(9).v1 = LEN(idetxt(o(9).txt))
            o(9).issel = -1
            o(9).sx1 = 0
            ColorData$ = RIGHT$(ColorSchemes$(SchemeID), 90)
            i = 1
            r$ = MID$(ColorData$, i, 3): i = i + 3: g$ = MID$(ColorData$, i, 3): i = i + 3: b$ = MID$(ColorData$, i, 3): i = i + 3
            IDETextColor = _RGB32(VAL(r$), VAL(g$), VAL(b$))
            r$ = MID$(ColorData$, i, 3): i = i + 3: g$ = MID$(ColorData$, i, 3): i = i + 3: b$ = MID$(ColorData$, i, 3): i = i + 3
            IDEKeywordColor = _RGB32(VAL(r$), VAL(g$), VAL(b$))
            r$ = MID$(ColorData$, i, 3): i = i + 3: g$ = MID$(ColorData$, i, 3): i = i + 3: b$ = MID$(ColorData$, i, 3): i = i + 3
            IDENumbersColor = _RGB32(VAL(r$), VAL(g$), VAL(b$))
            r$ = MID$(ColorData$, i, 3): i = i + 3: g$ = MID$(ColorData$, i, 3): i = i + 3: b$ = MID$(ColorData$, i, 3): i = i + 3
            IDEQuoteColor = _RGB32(VAL(r$), VAL(g$), VAL(b$))
            r$ = MID$(ColorData$, i, 3): i = i + 3: g$ = MID$(ColorData$, i, 3): i = i + 3: b$ = MID$(ColorData$, i, 3): i = i + 3
            IDEMetaCommandColor = _RGB32(VAL(r$), VAL(g$), VAL(b$))
            r$ = MID$(ColorData$, i, 3): i = i + 3: g$ = MID$(ColorData$, i, 3): i = i + 3: b$ = MID$(ColorData$, i, 3): i = i + 3
            IDECommentColor = _RGB32(VAL(r$), VAL(g$), VAL(b$))
            r$ = MID$(ColorData$, i, 3): i = i + 3: g$ = MID$(ColorData$, i, 3): i = i + 3: b$ = MID$(ColorData$, i, 3): i = i + 3
            IDEBackgroundColor = _RGB32(VAL(r$), VAL(g$), VAL(b$))
            r$ = MID$(ColorData$, i, 3): i = i + 3: g$ = MID$(ColorData$, i, 3): i = i + 3: b$ = MID$(ColorData$, i, 3): i = i + 3
            IDEBackgroundColor2 = _RGB32(VAL(r$), VAL(g$), VAL(b$))
            r$ = MID$(ColorData$, i, 3): i = i + 3: g$ = MID$(ColorData$, i, 3): i = i + 3: b$ = MID$(ColorData$, i, 3): i = i + 3
            IDEBracketHighlightColor = _RGB32(VAL(r$), VAL(g$), VAL(b$))
            r$ = MID$(ColorData$, i, 3): i = i + 3: g$ = MID$(ColorData$, i, 3): i = i + 3: b$ = MID$(ColorData$, i, 3): i = i + 3
            IDEChromaColor = _RGB32(VAL(r$), VAL(g$), VAL(b$))
            GOTO ChangeTextBoxes
        END IF

        IF mB AND mY = p.y + 5 AND mX >= p.x + 39 AND mX <= p.x + 39 + 26 THEN
            newValue = (mX - p.x - 39) * (255 / 26)
            idetxt(o(2).txt) = _TOSTR$(newValue)
            IF _KEYDOWN(100305) OR _KEYDOWN(100306) THEN
                idetxt(o(3).txt) = _TOSTR$(newValue)
                idetxt(o(4).txt) = _TOSTR$(newValue)
            END IF
            focus = 2
            o(focus).v1 = LEN(idetxt(o(focus).txt))
            o(focus).issel = -1
            o(focus).sx1 = 0
            GOSUB NewUserScheme
        END IF

        IF mB AND mY = p.y + 8 AND mX >= p.x + 39 AND mX <= p.x + 39 + 26 THEN
            newValue = (mX - p.x - 39) * (255 / 26)
            idetxt(o(3).txt) = _TOSTR$(newValue)
            IF _KEYDOWN(100305) OR _KEYDOWN(100306) THEN
                idetxt(o(2).txt) = _TOSTR$(newValue)
                idetxt(o(4).txt) = _TOSTR$(newValue)
            END IF
            focus = 3
            o(focus).v1 = LEN(idetxt(o(focus).txt))
            o(focus).issel = -1
            o(focus).sx1 = 0
            GOSUB NewUserScheme
        END IF

        IF mB AND mY = p.y + 11 AND mX >= p.x + 39 AND mX <= p.x + 39 + 26 THEN
            newValue = (mX - p.x - 39) * (255 / 26)
            idetxt(o(4).txt) = _TOSTR$(newValue)
            IF _KEYDOWN(100305) OR _KEYDOWN(100306) THEN
                idetxt(o(2).txt) = _TOSTR$(newValue)
                idetxt(o(3).txt) = _TOSTR$(newValue)
            END IF
            focus = 4
            o(focus).v1 = LEN(idetxt(o(focus).txt))
            o(focus).issel = -1
            o(focus).sx1 = 0
            GOSUB NewUserScheme
        END IF

        ChangedWithKeys = 0
        IF K$ = CHR$(0) + CHR$(72) AND (focus = 2 OR focus = 3 OR focus = 4) THEN 'Up
            idetxt(o(focus).txt) = _TOSTR$(VAL(idetxt(o(focus).txt)) + 1)
            o(focus).issel = -1: o(focus).sx1 = 0: o(focus).v1 = LEN(idetxt(o(focus).txt))
            ChangedWithKeys = -1
            GOSUB NewUserScheme
        END IF

        IF K$ = CHR$(0) + CHR$(80) AND (focus = 2 OR focus = 3 OR focus = 4) THEN 'Down
            idetxt(o(focus).txt) = _TOSTR$(VAL(idetxt(o(focus).txt)) - 1)
            o(focus).issel = -1: o(focus).sx1 = 0: o(focus).v1 = LEN(idetxt(o(focus).txt))
            ChangedWithKeys = -1
            GOSUB NewUserScheme
        END IF

        IF SelectedITEM <> o(1).sel AND o(1).sel > 0 THEN
            SelectedITEM = o(1).sel
            FOR i = 1 TO 10: SelectionIndicator$(i) = " ": NEXT i
            SelectionIndicator$(SelectedITEM) = CHR$(16)

            i = 10 'total number of selection indicators
            l$ = SelectionIndicator$(1) + "Normal Text"
            l$ = l$ + sep + SelectionIndicator$(2) + "Keywords"
            l$ = l$ + sep + SelectionIndicator$(3) + "Numbers"
            l$ = l$ + sep + SelectionIndicator$(4) + "Strings"
            l$ = l$ + sep + SelectionIndicator$(5) + "Metacommand/custom keywords"
            l$ = l$ + sep + SelectionIndicator$(6) + "Comments"
            l$ = l$ + sep + SelectionIndicator$(7) + "Background"
            l$ = l$ + sep + SelectionIndicator$(8) + "Current line background"
            l$ = l$ + sep + SelectionIndicator$(9) + "Bracket/selection highlight"
            l$ = l$ + sep + SelectionIndicator$(10) + "Menus and dialogs"
            idetxt(o(1).txt) = l$

            ChangeTextBoxes:
            CurrentColor~& = GetCurrentColor(SelectedITEM)
            idetxt(o(2).txt) = _TOSTR$(_RED32(CurrentColor~&))
            idetxt(o(3).txt) = _TOSTR$(_GREEN32(CurrentColor~&))
            idetxt(o(4).txt) = _TOSTR$(_BLUE32(CurrentColor~&))
            IF focus >= 2 AND focus <= 4 AND ChangedScheme THEN
                prevTB.value$ = idetxt(o(focus).txt)
            END IF
        END IF

        'Check RGB values range (0-255)
        FOR checkRGB = 2 TO 4
            a$ = idetxt(o(checkRGB).txt)
            IF LEN(a$) > 3 THEN a$ = LEFT$(a$, 3) '3 character limit
            FOR i = 1 TO LEN(a$)
                a = ASC(a$, i)
                IF i = 2 AND ASC(a$, 1) = 48 THEN a$ = "0": EXIT FOR
                IF a < 48 OR a > 57 THEN a$ = "": EXIT FOR
            NEXT
            IF LEN(a$) THEN
                a = VAL(a$)
                IF a > 255 THEN a$ = "255"
                IF a < 0 THEN a$ = "0"
            ELSE
                IF ChangedWithKeys = -1 THEN a$ = "0"
            END IF
            idetxt(o(checkRGB).txt) = a$
        NEXT checkRGB

        'Check for valid scheme name
        FoundPipe = INSTR(idetxt(o(9).txt), "|")
        IF FoundPipe > 0 THEN
            a2$ = LEFT$(idetxt(o(9).txt), FoundPipe - 1) + MID$(idetxt(o(9).txt), FoundPipe + 1)
            idetxt(o(9).txt) = a2$
            IF o(9).v1 >= FoundPipe THEN o(9).v1 = o(9).v1 - 1
        END IF

        IF SchemeID > 0 THEN
            FoundPipe = INSTR(ColorSchemes$(SchemeID), "|")
            IF RTRIM$(LTRIM$(idetxt(o(9).txt))) <> LEFT$(ColorSchemes$(SchemeID), FoundPipe - 1) THEN
                'A different scheme name is the beginning of editing a new one
                SchemeID = 0
            END IF
        END IF

        CurrentColor~& = _RGB32(VAL(idetxt(o(2).txt)), VAL(idetxt(o(3).txt)), VAL(idetxt(o(4).txt)))

        SELECT CASE SelectedITEM
            CASE 1: IDETextColor = CurrentColor~& 'Normal text
            CASE 2: IDEKeywordColor = CurrentColor~& 'Keywords
            CASE 3: IDENumbersColor = CurrentColor~& 'Numbers
            CASE 4: IDEQuoteColor = CurrentColor~& 'Strings
            CASE 5: IDEMetaCommandColor = CurrentColor~& 'Metacommands
            CASE 6: IDECommentColor = CurrentColor~& 'Comments
            CASE 7: IDEBackgroundColor = CurrentColor~& 'Background
            CASE 8: IDEBackgroundColor2 = CurrentColor~& 'Current line background
            CASE 9: IDEBracketHighlightColor = CurrentColor~& 'Bracket highlight
            CASE 10: IDEChromaColor = CurrentColor~&
        END SELECT

        IF K$ = CHR$(27) OR (focus = 10 AND info <> 0) THEN
            IDECommentColor = bkpIDECommentColor
            IDEMetaCommandColor = bkpIDEMetaCommandColor
            IDEQuoteColor = bkpIDEQuoteColor
            IDETextColor = bkpIDETextColor
            IDEKeywordColor = bkpIDEKeywordColor
            IDENumbersColor = bkpIDENumbersColor
            IDEBackgroundColor = bkpIDEBackgroundColor
            IDEBackgroundColor2 = bkpIDEBackgroundColor2
            IDEBracketHighlightColor = bkpIDEBracketHighlightColor
            IDEChromaColor = bkpIDEChromaColor
            EXIT FUNCTION
        END IF

        IF (focus = 9 AND info <> 0) THEN
            LoadDefaultScheme:
            GOSUB enableHighlighter
            SchemeID = 1
            FoundPipe = INSTR(ColorSchemes$(SchemeID), "|")
            idetxt(o(9).txt) = LEFT$(ColorSchemes$(SchemeID), FoundPipe - 1)
            info = 0
            GOTO ApplyScheme
        END IF

    IF (focus = 8 AND info <> 0) OR _
       (focus = 1 AND K$ = CHR$(13)) OR _
       (focus = 2 AND K$ = CHR$(13)) OR _
       (focus = 3 AND K$ = CHR$(13)) OR _
       (focus = 4 AND K$ = CHR$(13)) OR _
       (focus = 5 AND K$ = CHR$(13)) OR _
       (focus = 6 AND K$ = CHR$(13)) OR _
       (focus = 7 AND K$ = CHR$(13)) OR _
       (focus = 11 AND K$ = CHR$(13)) THEN
            'save changes
            GOSUB enableHighlighter

            WriteConfigSetting colorSettingsSection$, "SchemeID", _TOSTR$(SchemeID)
            FOR i = 1 TO 10
                CurrentColor~& = GetCurrentColor(i)
                SELECT CASE i
                    CASE 1: colorid$ = "TextColor"
                    CASE 2: colorid$ = "KeywordColor"
                    CASE 3: colorid$ = "NumbersColor"
                    CASE 4: colorid$ = "QuoteColor"
                    CASE 5: colorid$ = "MetaCommandColor"
                    CASE 6: colorid$ = "CommentColor"
                    CASE 7: colorid$ = "BackgroundColor"
                    CASE 8: colorid$ = "BackgroundColor2"
                    CASE 9: colorid$ = "HighlightColor"
                    CASE 10: colorid$ = "ChromaColor"
                END SELECT


                WriteConfigSetting colorSettingsSection$, colorid$, rgbs$(CurrentColor~&)
            NEXT i

            v% = o(5).sel
            IF v% <> 0 THEN v% = -1
            BracketHighlight = v%

            IF BracketHighlight THEN
                WriteConfigSetting generalSettingsSection$, "BracketHighlight", "True"
            ELSE
                WriteConfigSetting generalSettingsSection$, "BracketHighlight", "False"
            END IF

            v% = o(6).sel
            IF v% <> 0 THEN v% = -1
            MultiHighlight = v%

            IF MultiHighlight THEN
                WriteConfigSetting generalSettingsSection$, "MultiHighlight", "True"
            ELSE
                WriteConfigSetting generalSettingsSection$, "MultiHighlight", "False"
            END IF

            v% = o(7).sel
            IF v% <> 0 THEN v% = -1
            KeywordHighlight = v%

            IF KeywordHighlight THEN
                WriteConfigSetting generalSettingsSection$, "KeywordHighlight", "True"
            ELSE
                WriteConfigSetting generalSettingsSection$, "KeywordHighlight", "False"
            END IF

            EXIT FUNCTION
        END IF

        'end of custom controls

        mousedown = 0
        mouseup = 0
    LOOP

    idechoosecolorsbox = 0

    EXIT FUNCTION
    NewUserScheme:
    IF SchemeID > 0 AND SchemeID <= PresetColorSchemes THEN
        'If one of the preset schemes is currently selected,
        'create a new one. User-defined types can be freely
        'edited.
        SchemeID = 0
        idetxt(o(9).txt) = "User-defined"
    END IF
    RETURN

    enableHighlighter:
    IF DisableSyntaxHighlighter THEN
        DisableSyntaxHighlighter = _FALSE
        WriteConfigSetting generalSettingsSection$, "DisableSyntaxHighlighter", "False"
        menu$(OptionsMenuID, OptionsMenuDisableSyntax) = CHR$(7) + "Syntax #Highlighter"
    END IF
    RETURN
END FUNCTION
