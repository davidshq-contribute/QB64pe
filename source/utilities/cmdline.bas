'
' Command-line argument parsing module
' Contains functions for parsing command-line arguments and settings
'
' This module contains command-line parsing functions extracted from qb64pe.bas for better modularity.
' All functions depend on global variables and functions defined in qb64pe.bas and other modules.

FUNCTION ParseCMDLineArgs$ ()
    DIM tmpKwCap AS LONG, tmpKwLow AS LONG, i AS LONG
    DIM token$, setting$, dexf$
    DIM delim AS LONG, eos AS LONG
    'Recall that COMMAND$ is a concatenation of argv[] elements, so we don't have
    'to worry about more than one space between things (unless they used quotes,
    'in which case they're simply asking for trouble).
    tmpKwCap = -5: tmpKwLow = -5 'mutual exclusive options, both invalidated here
    FOR i = 1 TO _COMMANDCOUNT
        token$ = COMMAND$(i)
        IF LCASE$(token$) = "/?" OR LCASE$(token$) = "/h" OR LCASE$(token$) = "--help" OR LCASE$(token$) = "/help" THEN token$ = "-?"
        SELECT CASE LCASE$(LEFT$(token$, 2))
            CASE "-?", "-h" 'Command-line help
                _DEST _CONSOLE
                IF NOT QB64VersionPrinted THEN QB64VersionPrinted = _TRUE: PRINT "QB64-PE Compiler V" + Version$
                PRINT
                PRINT "USAGE: qb64pe [options] <source file> [-o <output file>]"
                PRINT
                PRINT "Info Options (no files required):"
                PRINT "  -?, -h, --help       Show this help text"
                PRINT "  -v                   Show version information"
                PRINT
                PRINT "File specifications:"
                PRINT "  <source file>        Source file to load into IDE, to format or compile" '    '80 columns
                PRINT "  -o <output file>     Write result to <output file>"
                PRINT "                         - optionally override the default executable name"
                PRINT "                         - is mandatory for code formatting (-y option)"
                PRINT
                PRINT "IDE Options:"
                PRINT "  -l:<line number>     Load <source file> into the IDE and move cursor to"
                PRINT "                       the given <line number>, if possible"
                PRINT
                PRINT "Compiler Options (no IDE):"
                PRINT "  -c                   Compile <source file> (show progress in own window)"
                PRINT "  -x                   Like -c, but progress goes to console (no own window)"
                PRINT "  -y                   Output (re)formatted <source file> to -o <output file>"
                PRINT "  -z                   Generate C code from <source file> without compiling"
                PRINT "                       the executable (C code output goes to internal\temp)"
                PRINT "                         - may be used to quickly check for syntax errors"
                PRINT
                PRINT "Extended Compiler Options:"
                PRINT "  -p                   Purge all pre-compiled content first"
                PRINT "  -e                   Enforce variable declaration even if no OPTION _EXPLICIT"
                PRINT "                       was used in the <source file>"
                PRINT "                         - per compilation, doesn't change the <source file>"
                PRINT "  -s[:setting=value]   View and/or edit & save compiler settings permanently"
                PRINT
                PRINT "Temporary Compiler Options:"
                PRINT "  -f[:setting=value]   Compiler and/or formatting settings to use"
                PRINT "                         - per compilation, doesn't change global defaults"
                PRINT
                PRINT "Reporting Options:"
                PRINT "  -w                   Show warnings (such as unused variables etc.)"
                PRINT "  -q                   Quiet mode (no progress, but warnings/errors, if any)"
                PRINT "  -m                   Do not colorize compiler outputs (monochrome mode)"
                PRINT
                PRINT "     ----------------------------------------------------------------------"
                PRINT
                CMDLineSettingsHelp
                PRINT
                PRINT "     ----------------------------------------------------------------------"
                PRINT
                CMDLineTemporarySettingsHelp
                SYSTEM

            CASE "-v" ' Print version
                _DEST _CONSOLE
                IF NOT QB64VersionPrinted THEN QB64VersionPrinted = _TRUE: PRINT "QB64-PE Compiler V" + Version$
                SYSTEM

            CASE "-o" 'Specify an output file
                IF LEN(COMMAND$(i + 1)) > 0 THEN CMDLineOutFile$ = COMMAND$(i + 1): i = i + 1
                CMDLineSwitch = _TRUE

            CASE "-l" 'goto line (ide mode only); -l:<line number>
                IF MID$(token$, 3, 1) = ":" THEN IDEStartAtLine = VAL(MID$(token$, 4))
                CMDLineSwitch = _TRUE

            CASE "-c" 'Compile instead of edit
                NoIDEMode = _TRUE
                CMDLineSwitch = _TRUE

            CASE "-x" 'Use the console
                ConsoleMode = _TRUE
                NoIDEMode = _TRUE
                CMDLineSwitch = _TRUE

            CASE "-y" 'Format
                FormatMode = _TRUE
                ConsoleMode = _TRUE
                NoIDEMode = _TRUE
                QuietMode = _TRUE
                CMDLineSwitch = _TRUE

            CASE "-z" 'Not compiling C code
                NoCCompileMode = _TRUE
                ConsoleMode = _TRUE
                NoIDEMode = _TRUE
                CMDLineSwitch = _TRUE

            CASE "-p" 'Purge
                PurgeTemporaryBuildFiles (os$), (MacOSX)
                CMDLineSwitch = _TRUE

            CASE "-e" 'Option Explicit
                ForceOptExpl = _TRUE
                CMDLineSwitch = _TRUE

            CASE "-s" 'Settings
                SettingsMode = _TRUE
                setting$ = MID$(token$, 3)
                _DEST _CONSOLE
                IF NOT QB64VersionPrinted THEN QB64VersionPrinted = _TRUE: PRINT "QB64-PE Compiler V" + Version$: PRINT

                delim = INSTR(setting$, "="): eos = 0
                IF delim = 0 THEN delim = LEN(setting$) + 1: eos = -1 'end of string
                SELECT CASE LCASE$(LEFT$(setting$, delim - 1))
                    CASE ""
                        PRINT "DebugInfo     = "; BoolToTFString$(IncludeDebugInfo)
                        PRINT "ExeWithSource = "; BoolToTFString$(SaveExeWithSource)
                        PRINT "ExeDefaultDir = "; _IIF(LEN(DefaultExeSaveFolder$), DefaultExeSaveFolder$, "unset (= qb64pe folder)")
                        SYSTEM
                    CASE ":"
                        CMDLineSettingsError "Missing setting specification: " + token$, 0, 0
                    CASE ":debuginfo"
                        IF NOT eos THEN
                            IF NOT ParseBooleanSetting&(token$, IncludeDebugInfo) THEN CMDLineSettingsError token$, 1, 0
                            WriteConfigSetting compilerSettingsSection$, "IncludeDebugInfo", BoolToTFString$(IncludeDebugInfo)
                            PurgeTemporaryBuildFiles (os$), (MacOSX)
                        END IF
                        PRINT "DebugInfo = "; BoolToTFString$(IncludeDebugInfo)
                    CASE ":exewithsource"
                        IF NOT eos THEN
                            IF NOT ParseBooleanSetting&(token$, SaveExeWithSource) THEN CMDLineSettingsError token$, 1, 0
                            WriteConfigSetting generalSettingsSection$, "SaveExeWithSource", BoolToTFString$(SaveExeWithSource)
                        END IF
                        PRINT "ExeWithSource = "; BoolToTFString$(SaveExeWithSource)
                    CASE ":exedefaultdir"
                        IF NOT eos THEN
                            IF NOT ParseStringSetting&(token$, dexf$) THEN CMDLineSettingsError token$, 3, 0
                            IF LEN(dexf$) THEN
                                IF _DIREXISTS(_STARTDIR$ + dexf$) THEN dexf$ = _STARTDIR$ + dexf$
                                IF _DIREXISTS(dexf$) THEN
                                    dexf$ = _FULLPATH$(dexf$)
                                    WriteConfigSetting generalSettingsSection$, "DefaultExeSaveFolder", dexf$
                                    IF RIGHT$(dexf$, 1) <> pathsep$ THEN dexf$ = dexf$ + pathsep$
                                    DefaultExeSaveFolder$ = dexf$: dexf$ = "-1"
                                END IF
                            END IF
                            IF dexf$ <> "-1" THEN CMDLineSettingsError "Path not found: " + token$, 0, 0
                        END IF
                        PRINT "ExeDefaultDir = "; _IIF(LEN(DefaultExeSaveFolder$), DefaultExeSaveFolder$, "unset (= qb64pe folder)")
                    CASE ELSE
                        CMDLineSettingsError "Unsupported setting: " + token$, 0, 0
                END SELECT
                IF eos THEN SYSTEM 'was just a settings query
                _DEST 0

            CASE "-f" 'temporary setting
                SettingsMode = _TRUE
                setting$ = MID$(token$, 3)

                delim = INSTR(setting$, "=")
                IF delim = 0 THEN delim = LEN(setting$) + 1
                SELECT CASE LCASE$(LEFT$(setting$, delim - 1))
                    CASE ""
                        _DEST _CONSOLE
                        IF NOT QB64VersionPrinted THEN QB64VersionPrinted = _TRUE: PRINT "QB64-PE Compiler V" + Version$: PRINT
                        CMDLineTemporarySettingsHelp
                        SYSTEM
                    CASE ":"
                        CMDLineSettingsError "Missing setting specification: " + token$, 0, 1
                    CASE ":optimizecppprogram"
                        IF NOT ParseBooleanSetting&(token$, OptimizeCppProgram) THEN CMDLineSettingsError token$, 1, 1
                    CASE ":stripdebugsymbols"
                        IF NOT ParseBooleanSetting&(token$, StripDebugSymbols) THEN CMDLineSettingsError token$, 1, 1
                    CASE ":extracppflags"
                        IF NOT ParseStringSetting&(token$, ExtraCppFlags$) THEN CMDLineSettingsError token$, 3, 1
                    CASE ":extralinkerflags"
                        IF NOT ParseStringSetting&(token$, ExtraLinkerFlags$) THEN CMDLineSettingsError token$, 3, 1
                    CASE ":maxcompilerprocesses"
                        IF NOT ParseLongSetting&(token$, MaxParallelProcesses) THEN CMDLineSettingsError token$, 2, 1
                        IF MaxParallelProcesses < 1 OR MaxParallelProcesses > 128 THEN CMDLineSettingsError "MaxCompilerProcesses must be in range 1-128.", 0, 1
                    CASE ":generatelicensefile"
                        IF NOT ParseBooleanSetting&(token$, GenerateLicenseFile) THEN CMDLineSettingsError token$, 1, 1
                    CASE ":usesystemcompiler"
                        IF NOT ParseBooleanSetting&(token$, UseSystemMinGW) THEN CMDLineSettingsError token$, 1, 1
                    CASE ":autoindent"
                        IF NOT ParseBooleanSetting&(token$, IDEAutoIndent) THEN CMDLineSettingsError token$, 1, 1
                        DEFAutoIndent = IDEAutoIndent 'for restoring after '$FORMAT:OFF
                    CASE ":autoindentsize"
                        IF NOT ParseLongSetting&(token$, IDEAutoIndentSize) THEN CMDLineSettingsError token$, 2, 1
                        IF IDEAutoIndentSize < 1 OR IDEAutoIndentSize > 64 THEN CMDLineSettingsError "AutoIndentSize must be in range 1-64.", 0, 1
                    CASE ":indentsubs"
                        IF NOT ParseBooleanSetting&(token$, IDEIndentSubs) THEN CMDLineSettingsError token$, 1, 1
                    CASE ":autolayout"
                        IF NOT ParseBooleanSetting&(token$, IDEAutoLayout) THEN CMDLineSettingsError token$, 1, 1
                        DEFAutoLayout = IDEAutoLayout 'for restoring after '$FORMAT:OFF
                    CASE ":keywordcapitals"
                        IF NOT ParseBooleanSetting&(token$, tmpKwCap) THEN CMDLineSettingsError token$, 1, 1
                    CASE ":keywordlowercase"
                        IF NOT ParseBooleanSetting&(token$, tmpKwLow) THEN CMDLineSettingsError token$, 1, 1
                    CASE ELSE
                        CMDLineSettingsError "Unsupported setting: " + token$, 0, 1
                END SELECT

            CASE "-w" 'Show warnings
                ShowWarnings = _TRUE
                CMDLineSwitch = _TRUE

            CASE "-q" 'Quiet mode
                QuietMode = _TRUE
                CMDLineSwitch = _TRUE

            CASE "-m" 'Monochrome mode
                MonochromeLoggingMode = _TRUE
                CMDLineSwitch = _TRUE

            CASE "-u" 'Invoke "Update all pages" to populate internal/help files (hidden CI build option)
                Help_Recaching = 2: Help_IgnoreCache = 1
                IF ideupdatehelpbox THEN
                    _DEST _CONSOLE
                    PRINT "Update failed: Can't make connection to Wiki."
                    SYSTEM 1
                END IF
                SYSTEM

            CASE ELSE 'Something we don't recognise, assume it's a filename
                IF PassedFileName$ = "" THEN PassedFileName$ = token$
        END SELECT
    NEXT i
    'check if mutual exclusive options were given and accordingly adjust setting
    IF tmpKwCap > -5 OR tmpKwLow > -5 THEN 'any of it given? - No: stay with default
        IF tmpKwCap = tmpKwLow THEN 'both set or unset = CaMeL case
            IDEAutoLayoutKwStyle = _EQUAL
        ELSEIF tmpKwCap = _TRUE THEN '= UPPER case
            IDEAutoLayoutKwStyle = _GREATER
        ELSEIF tmpKwLow = _TRUE THEN '= lower case
            IDEAutoLayoutKwStyle = _LESS
        END IF
    END IF
    'don't leak force option into the IDE or the code formatter
    IF NOT NoIDEMode THEN ForceOptExpl = _FALSE

    IF FormatMode AND LEN(CMDLineOutFile$) = 0 THEN
        _DEST _CONSOLE
        PRINT "Formatting requires specifying output file with -o option"
        SYSTEM 1
    END IF

    IF LEN(PassedFileName$) THEN
        ParseCMDLineArgs$ = PassedFileName$
    ELSE
        IF NOT CMDLineSwitch AND SettingsMode THEN SYSTEM
    END IF
END FUNCTION

SUB CMDLineSettingsError (errstr$, typ%, tmp%)
    'errstr$ = the token$ only for types 1-3, else any custom message
    'typ% = 1 (invalid bool) / = 2 (missing integer) / = 3 (missing string) / else (custom message)
    'tmp% <> 0 (-f temp setting) / = 0 (-s permanent setting)
    _DEST _CONSOLE
    IF NOT QB64VersionPrinted THEN QB64VersionPrinted = _TRUE: PRINT "QB64-PE Compiler V" + Version$: PRINT

    SELECT CASE typ%
        CASE 1
            PRINT "Invalid boolean value for "; _IIF(tmp%, "temporary (-f)", "(-s)"); " setting: "; errstr$
            PRINT "   enable: "; _IIF(tmp%, "-f", "-s"); ":setting=[true|on|yes|1|-1]"
            PRINT "  disable: "; _IIF(tmp%, "-f", "-s"); ":setting=[false|off|no|0]"
        CASE 2, 3
            PRINT "Missing value for "; _IIF(tmp%, "temporary (-f)", "(-s)"); " setting: "; errstr$
            PRINT "  expected: "; _IIF(tmp%, "-f", "-s"); ":setting=["; _IIF(typ% = 2, "number", "string"); "]"
        CASE ELSE
            PRINT errstr$
    END SELECT
    PRINT

    IF tmp% THEN CMDLineTemporarySettingsHelp ELSE CMDLineSettingsHelp
    SYSTEM 1
END SUB

SUB CMDLineSettingsHelp
    PRINT "Supported (-s) Compiler settings:"
    PRINT "  -s                              Show the current state of all settings"
    PRINT "  -s:DebugInfo=[true|false]       Embed C++ debug info into executable"
    PRINT "  -s:ExeWithSource=[true|false]   Save executable in the source folder"
    PRINT "  -s:ExeDefaultDir=[string]       Save executables here per default,"
    PRINT "                                  if not saving with source"
    PRINT "      You may specify a setting without equal sign and value to"
    PRINT "      show the current state of that specific setting only."
END SUB

SUB CMDLineTemporarySettingsHelp
    PRINT "Note:" '                                                                        '80 columns
    PRINT "  Defaults for the following settings can be set via the IDE Options menu,"
    PRINT "  any values given here are just temporary overrides per compilation."
    PRINT
    PRINT "Supported (-f) Compiler settings:"
    PRINT "  -f                                    Show this list of supported settings"
    PRINT "  -f:OptimizeCppProgram=[true|false]    Compile with C++ Optimization flag"
    PRINT "  -f:StripDebugSymbols=[true|false]     Strip C++ Symbols from executable"
    PRINT "  -f:ExtraCppFlags=[string]             Extra flags for the C++ Compiler"
    PRINT "  -f:ExtraLinkerFlags=[string]          Extra flags for the Linker"
    PRINT "  -f:MaxCompilerProcesses=[integer]     Max C++ Compiler processes to use"
    PRINT "  -f:GenerateLicenseFile=[true|false]   Produce a license.txt file for program"
    PRINT "  -f:UseSystemCompiler=[true|false]     Use the system C++ compiler instead of"
    PRINT "                                        the bundled one (Windows only)"
    PRINT
    PRINT "Supported (-f) Layout settings:"
    PRINT "  -f:AutoIndent=[true|false]            Auto Indent lines"
    PRINT "      The next two also require the above to be enabled or they will have"
    PRINT "      no effect, unless AutoIndent is enabled per default in the IDE."
    PRINT "  -f:AutoIndentSize=[integer]           Indent Spacing per indent level"
    PRINT "  -f:IndentSubs=[true|false]            Indent SUBs and FUNCTIONs"
    PRINT "  -f:AutoLayout=[true|false]            Auto Single-spacing of code elements"
    PRINT "      The next two work together, if both are given with the same state"
    PRINT "      it's CaMeL case, otherwise the enabled one determines the case,"
    PRINT "      hence no need to specify both if you just want UPPER or lower case."
    PRINT "      If none is given the default as set in the IDE is used."
    PRINT "  -f:KeywordCapitals=[true|false]       Make keywords to ALL CAPITALS"
    PRINT "  -f:KeywordLowercase=[true|false]      Make keywords to ALL lower case"
END SUB
