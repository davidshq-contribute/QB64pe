'QB64 Compiler Parser Module - Interface
'Contains utility functions for parsing and tokenization

DECLARE FUNCTION RemoveTrailingSpaces$ (text$)
DECLARE FUNCTION IsValidIdentifierChar%% (c AS LONG)

'String utilities (migrated from qb64pe.bas)
DECLARE FUNCTION str_nth$ (x AS LONG)

'Element counting (migrated from qb64pe.bas)
DECLARE FUNCTION countelements& (a$)

'Note: The following functions are still in qb64pe.bas and will be migrated later:
'DECLARE FUNCTION validname (a$)
'DECLARE FUNCTION lineformat$ (a$)
