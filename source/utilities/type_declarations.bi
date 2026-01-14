
'$INCLUDEONCE

' Type system flags (declarations only)
' All DIM SHARED declarations moved to qb64pe.bas
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

' Global id variable used by id2fulltypename$ and id2shorttypename$ functions
' All DIM SHARED declarations moved to qb64pe.bas
