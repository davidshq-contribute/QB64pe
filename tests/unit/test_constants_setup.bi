'
' Test Constants Setup
'
' Provides minimal constants needed for test compilation
' This file defines constants that are normally in settings.bas and other files
' but aren't available in the test context
'

' Debug constant (from source/global/settings.bas)
' Used by constants.bas for debug output formatting
CONST Debug = 0

' Note: _OS$ is a built-in QB64 function, so it should be available
' If it's not, we may need to define it, but typically it's available
