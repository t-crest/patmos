
grammar PatGram;

@header {
import java.util.HashMap;
}

@members {
/** Map symbol to Integer object holding the value or address */
HashMap memory = new HashMap();
}

prog: statement+;

statement: (label)? (directive | instruction)? (COMMENT)? NEWLINE;

label:  ID ':';

directive: '.start';

instruction: alu_imm | alu;

alu_imm: op_imm s1 ',' imm_val TO dest {System.out.println($op_imm.text + $op_imm.opc + $s1.text + $dest.text);};

alu: op_alu s1 ',' s2 TO dest {System.out.println($op_alu.text + $op_alu.opc+ $s1.text);};

s1: REG;
s2: REG;
dest: REG;

imm_val returns [int value]
    : INT {$value = Integer.parseInt($INT.text);};

op_imm returns [String opc]: 
	'add' {$opc = "00";} |
	'or' {$opc = "01";} |
	'and' {$opc = "10";} |
	'xor' {$opc = "11";}
	;

op_alu returns [String opc]: 
	'add' {$opc = "000";} |
	'sub' {$opc = "000";} |
	'or' {$opc = "001";} |
	'and' {$opc = "010";} |
	'xor' {$opc = "011";}
	;

/* Lexer rules (start with upper case) */

INT :   '0'..'9'+ ;
REG: ('R'|'r') INT;
// fragment REG_NUM: ('0'..'9') | ('1'..'2' '0..9') | ('3' '0'..'1');
TO: '->';

ID: ('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'_' | '0'..'9')*;

COMMENT: '//' ~('\n'|'\r')* ;

NEWLINE: '\r'? '\n' ;
WHITSPACE:   (' '|'\t')+ {skip();} ;

