'
' Main Code Generation Test Suite
'
' Includes and runs all code generation tests.
'

'$INCLUDE:'../test_framework.bi'
'$INCLUDE:'test_code_emission.bas'
'$INCLUDE:'test_code_structure.bas'

' Run all code generation tests
SUB RunCodeGenerationTests
    RunCodeEmissionTests
    RunCodeStructureTests
END SUB
