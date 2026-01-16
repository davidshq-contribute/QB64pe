'QB64 Compiler Parser Module - Interface
'Contains utility functions for parsing and tokenization

DECLARE FUNCTION RemoveTrailingSpaces$ (text$)
DECLARE FUNCTION IsValidIdentifierChar%% (c AS LONG)

'String utilities (migrated from qb64pe.bas)
DECLARE FUNCTION str_nth$ (x AS LONG)

'Element counting (migrated from qb64pe.bas)
DECLARE FUNCTION countelements& (a$)

'Unique number generation (migrated from qb64pe.bas)
DECLARE FUNCTION uniquenumber&

'Name validation (migrated from qb64pe.bas)
DECLARE FUNCTION validname (a$)

'Line formatting (migrated from qb64pe.bas)
DECLARE FUNCTION lineformat$ (a$)
