'Simple test to verify test harness works
$CONSOLE:ONLY

PRINT "Testing simple output"
PRINT "1 + 1 = "; 1 + 1

IF 1 + 1 = 2 THEN
    PRINT "PASS: Basic math works"
    SYSTEM 0
ELSE
    PRINT "FAIL: Basic math broken"
    SYSTEM 1
END IF
