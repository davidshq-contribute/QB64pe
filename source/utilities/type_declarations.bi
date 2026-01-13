
'$INCLUDEONCE

' Type system flags (declarations only)
DIM SHARED ISSTRING AS LONG
DIM SHARED ISFLOAT AS LONG
DIM SHARED ISUNSIGNED AS LONG
DIM SHARED ISPOINTER AS LONG
DIM SHARED ISFIXEDLENGTH AS LONG
DIM SHARED ISINCONVENTIONALMEMORY AS LONG
DIM SHARED ISOFFSETINBITS AS LONG
DIM SHARED ISARRAY AS LONG
DIM SHARED ISREFERENCE AS LONG
DIM SHARED ISUDT AS LONG
DIM SHARED ISOFFSET AS LONG

' Type constants (declarations only)
DIM SHARED STRINGTYPE AS LONG
DIM SHARED BITTYPE AS LONG
DIM SHARED UBITTYPE AS LONG
DIM SHARED BYTETYPE AS LONG
DIM SHARED UBYTETYPE AS LONG
DIM SHARED INTEGERTYPE AS LONG
DIM SHARED UINTEGERTYPE AS LONG
DIM SHARED LONGTYPE AS LONG
DIM SHARED ULONGTYPE AS LONG
DIM SHARED INTEGER64TYPE AS LONG
DIM SHARED UINTEGER64TYPE AS LONG
DIM SHARED SINGLETYPE AS LONG
DIM SHARED DOUBLETYPE AS LONG
DIM SHARED FLOATTYPE AS LONG
DIM SHARED OFFSETTYPE AS LONG
DIM SHARED UOFFSETTYPE AS LONG
DIM SHARED UDTTYPE AS LONG

' UDT arrays (declarations only - will be REDIM'd in type_init.bas)
DIM SHARED udtxname() AS STRING * 256
DIM SHARED udtxcname() AS STRING * 256
DIM SHARED udtxsize() AS LONG
DIM SHARED udtxnext() AS LONG
DIM SHARED udtxvariable() AS INTEGER
'elements
DIM SHARED udtename() AS STRING * 256
DIM SHARED udtecname() AS STRING * 256
DIM SHARED udtesize() AS LONG
DIM SHARED udtetype() AS LONG
DIM SHARED udtetypesize() AS LONG
DIM SHARED udtearrayelements() AS LONG
DIM SHARED udtenext() AS LONG

' idstruct TYPE - needed for id2fulltypename$ and id2shorttypename$ functions
TYPE idstruct
    n AS STRING * 256 'name
    cn AS STRING * 256 'case sensitive version of n

    arraytype AS LONG 'similar to t
    arrayelements AS INTEGER
    staticarray AS INTEGER 'set for arrays declared in the main module with static elements

    mayhave AS STRING * 8 'mayhave and musthave are exclusive of each other
    musthave AS STRING * 8
    t AS LONG 'type

    tsize AS LONG

    subfunc AS INTEGER 'if function=1, sub=2 (max 100 arguments)
    Dependency AS INTEGER
    internal_subfunc AS INTEGER

    callname AS STRING * 256
    ccall AS INTEGER
    overloaded AS _BYTE
    args AS INTEGER
    minargs AS INTEGER
    arg AS STRING * 400 'similar to t
    argsize AS STRING * 400 'similar to tsize (used for fixed length strings)
    specialformat AS STRING * 256
    secondargmustbe AS STRING * 256
    secondargcantbe AS STRING * 256
    ret AS LONG 'the value it returns if it is a function (again like t)

    insubfunc AS STRING * 256
    insubfuncn AS LONG

    share AS INTEGER
    nele AS STRING * 100
    nelereq AS STRING * 100
    linkid AS LONG
    linkarg AS INTEGER
    staticscope AS INTEGER
    'For variables which are arguments passed to a sub/function
    sfid AS LONG 'id number of variable's parent sub/function
    sfarg AS INTEGER 'argument/parameter # within call (1=first)

    hr_syntax AS STRING
END TYPE

' Global id variable used by id2fulltypename$ and id2shorttypename$ functions
DIM SHARED id AS idstruct

