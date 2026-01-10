'
' Integration Test: Dependency Detection
'
' Tests that the compiler correctly detects which runtime features
' are needed and sets appropriate DEP_* flags.
'

' Test that graphics dependency is detected
SCREEN _NEWIMAGE(640, 480, 32)

' Test that audio dependency is detected (if audio functions are used)
' _SNDPLAY would trigger DEP_AUDIO_MINIAUDIO

' Test that HTTP dependency is detected (if HTTP functions are used)
' HTTP functions would trigger DEP_HTTP

' This test verifies that the compiler correctly identifies dependencies
' Expected: Compilation should succeed with appropriate DEP_* flags set

PRINT "Dependency detection test"
