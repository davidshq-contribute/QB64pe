'
' Integration Test: Large File Handling
'
' Tests that the compiler can handle large source files
' without performance issues or memory problems.
'

' Generate a large program with many statements
DIM i AS LONG
FOR i = 1 TO 1000
    DIM var AS LONG
    var = i
    PRINT "Line "; i; ": "; var
NEXT

' Expected: Compilation should complete successfully
' Performance: Should compile in reasonable time
