
grammar PatGram;

@header {
import java.util.HashMap;
}

@members {
/** Map variable name to Integer object holding value */
HashMap memory = new HashMap();
}

prog: statement+;

statement: (label)? (directive | instruction)? NEWLINE;

directive: '.start';

instruction: opcode register {System.out.println("Instr found" + $opcode.text + $opcode.opc+ $register.text);};

label:  ID ':';

opcode returns [String opc]: 'add' {$opc = "1100";}| 'sub' {$opc = "1111";};

register: ('R'|'r') INT;

INT :   '0'..'9'+ ;
ID: ('a'..'z'|'A'..'Z')+;

NEWLINE:'\r'? '\n' ;
WHITSPACE  :   (' '|'\t')+ {skip();} ;

