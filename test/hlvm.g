#
14
S -> main ( ) { SEQ }
SEQ -> EXPR ; SEQ
EXPR -> int VAR = VAR ;
EXPR -> int VAR = INTEGER ;
EXPR -> float VAR = VAR;
EXPR -> float VAR = FLOAT;
EXPR -> string VAR = VAR;
EXPR -> string VAR = STRING;
EXPR -> VAR = VAR
EXPR -> print ( VAR )
EXPR -> print ( INTEGER )
EXPR -> print ( FLOAT )
EXPR -> print ( STRING )
EXPR -> return
