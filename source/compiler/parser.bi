'QB64 Compiler Parser Module - Interface
'Contains utility functions for parsing and tokenization

DECLARE FUNCTION RemoveTrailingSpaces$ (text$)
DECLARE FUNCTION IsValidIdentifierChar%% (c AS LONG)

'Note: The following functions are still in qb64pe.bas and will be migrated later:
'DECLARE FUNCTION str_nth$ (x AS LONG)
'DECLARE FUNCTION countelements& (a$)
'DECLARE FUNCTION validname (a$)
