'
' Main Code Generation Test Suite
'
' Includes and runs all code generation tests.
'
' Note: test_framework.bi is included by test_runner.bas
'$INCLUDE:'test_code_emission.bas'
'$INCLUDE:'test_code_structure.bas'

' Run all code generation tests
SUB RunCodeGenerationTests
    RunCodeEmissionTests
    RunCodeStructureTests
END SUB
