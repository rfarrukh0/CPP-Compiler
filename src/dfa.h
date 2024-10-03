#include <string>

std::string DFAstring = R"(
.STATES
start
ID!
NUM!
LPAREN!
RPAREN!
LBRACE!
RBRACE!
LBRACK!
RBRACK!
BECOMES!
PLUS!
MINUS!
STAR!
SLASH!
PCT!
AMP!
COMMA!
SEMI!
LT!
GT!
LE!
GE!
LBRACK!
RBRACK!
EQ!
NE!
ZERO!
EXCLAMATION
?WHITESPACE!
?COMMENT!
.TRANSITIONS
start a-z A-Z ID
ID a-z A-Z 0-9 ID
start 1-9 NUM
NUM 0-9 NUM
start 0 ZERO
ZERO 0 ZERO 
ZERO 1-9 NUM 
start ( LPAREN
start ) RPAREN
start { LBRACE 
start } RBRACE 
start = BECOMES
start [ LBRACK
start ] RBRACK
BECOMES = EQ
start + PLUS 
start - MINUS 
start * STAR 
start / SLASH
start % PCT
start & AMP
start , COMMA
start ; SEMI 
start < LT
start > GT
LT = LE 
GT = GE 
start ! EXCLAMATION
EXCLAMATION = NE 
start \s \t \r \n ?WHITESPACE
?WHITESPACE \s \t \r \n ?WHITESPACE
SLASH / ?COMMENT
?COMMENT \x00-\x09 \x0B \x0C \x0E-\x7F ?COMMENT
)";