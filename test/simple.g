#
5
S -> VAR ( ) { SEQ }
SEQ -> EXPR ; SEQ
EXPR -> TYPE VAR = INTEGER ;
EXPR -> print ( VAR )
EXPR -> return VAR
