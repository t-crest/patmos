// $ANTLR 3.3 Nov 30, 2010 12:50:56 java/src/grammar/PatGram.g 2011-09-16 23:23:10

package patmos.asm.generated;

import java.util.HashMap;
import java.util.List;



import org.antlr.runtime.*;
import java.util.Stack;
import java.util.List;
import java.util.ArrayList;

public class PatGramParser extends Parser {
    public static final String[] tokenNames = new String[] {
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "COMMENT", "NEWLINE", "ID", "TO", "REG", "PRD", "INT", "WHITSPACE", "':'", "'.start'", "','", "'cmp'", "'br'", "'addi'", "'ori'", "'andi'", "'xori'", "'add'", "'sub'", "'or'", "'and'", "'xor'", "'=='", "'!='", "'>='", "'<='", "'>'", "'<'"
    };
    public static final int EOF=-1;
    public static final int T__12=12;
    public static final int T__13=13;
    public static final int T__14=14;
    public static final int T__15=15;
    public static final int T__16=16;
    public static final int T__17=17;
    public static final int T__18=18;
    public static final int T__19=19;
    public static final int T__20=20;
    public static final int T__21=21;
    public static final int T__22=22;
    public static final int T__23=23;
    public static final int T__24=24;
    public static final int T__25=25;
    public static final int T__26=26;
    public static final int T__27=27;
    public static final int T__28=28;
    public static final int T__29=29;
    public static final int T__30=30;
    public static final int T__31=31;
    public static final int COMMENT=4;
    public static final int NEWLINE=5;
    public static final int ID=6;
    public static final int TO=7;
    public static final int REG=8;
    public static final int PRD=9;
    public static final int INT=10;
    public static final int WHITSPACE=11;

    // delegates
    // delegators


        public PatGramParser(TokenStream input) {
            this(input, new RecognizerSharedState());
        }
        public PatGramParser(TokenStream input, RecognizerSharedState state) {
            super(input, state);
             
        }
        

    public String[] getTokenNames() { return PatGramParser.tokenNames; }
    public String getGrammarFileName() { return "java/src/grammar/PatGram.g"; }


    /** Map symbol to Integer object holding the value or address */
    HashMap symbols = new HashMap();
    int pc = 0;
    int code[];
    boolean pass2 = false;

    public static String niceHex(int val) {
    	String s = Integer.toHexString(val);
    	while (s.length() < 8) {
    		s = "0"+s;
    	}
    	s = "0x"+s;
    	return s;
    }



    // $ANTLR start "pass1"
    // java/src/grammar/PatGram.g:63:1: pass1 : ( statement )+ ;
    public final void pass1() throws RecognitionException {
        try {
            // java/src/grammar/PatGram.g:63:6: ( ( statement )+ )
            // java/src/grammar/PatGram.g:63:8: ( statement )+
            {
            // java/src/grammar/PatGram.g:63:8: ( statement )+
            int cnt1=0;
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( ((LA1_0>=COMMENT && LA1_0<=ID)||LA1_0==PRD||LA1_0==13||(LA1_0>=15 && LA1_0<=25)) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // java/src/grammar/PatGram.g:63:8: statement
            	    {
            	    pushFollow(FOLLOW_statement_in_pass139);
            	    statement();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    if ( cnt1 >= 1 ) break loop1;
                        EarlyExitException eee =
                            new EarlyExitException(1, input);
                        throw eee;
                }
                cnt1++;
            } while (true);


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end "pass1"


    // $ANTLR start "dump"
    // java/src/grammar/PatGram.g:65:1: dump : ;
    public final void dump() throws RecognitionException {
        try {
            // java/src/grammar/PatGram.g:65:5: ()
            // java/src/grammar/PatGram.g:65:7: 
            {
            System.out.println(symbols);

            }

        }
        finally {
        }
        return ;
    }
    // $ANTLR end "dump"


    // $ANTLR start "pass2"
    // java/src/grammar/PatGram.g:68:1: pass2 returns [List mem] : ( statement )+ ;
    public final List pass2() throws RecognitionException {
        List mem = null;


        	System.out.println(pc+" "+symbols);
        	code = new int[pc];
        	pc = 0;
        	pass2 = true;

        try {
            // java/src/grammar/PatGram.g:75:2: ( ( statement )+ )
            // java/src/grammar/PatGram.g:75:4: ( statement )+
            {
            // java/src/grammar/PatGram.g:75:4: ( statement )+
            int cnt2=0;
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( ((LA2_0>=COMMENT && LA2_0<=ID)||LA2_0==PRD||LA2_0==13||(LA2_0>=15 && LA2_0<=25)) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // java/src/grammar/PatGram.g:75:4: statement
            	    {
            	    pushFollow(FOLLOW_statement_in_pass265);
            	    statement();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    if ( cnt2 >= 1 ) break loop2;
                        EarlyExitException eee =
                            new EarlyExitException(2, input);
                        throw eee;
                }
                cnt2++;
            } while (true);


            	mem = new ArrayList(pc);
            	for (int i=0; i<pc; ++i) {
            		mem.add(new Integer(code[i]));
            	}
            	

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return mem;
    }
    // $ANTLR end "pass2"


    // $ANTLR start "statement"
    // java/src/grammar/PatGram.g:82:1: statement : ( label )? ( directive | bundle )? ( COMMENT )? NEWLINE ;
    public final void statement() throws RecognitionException {
        try {
            // java/src/grammar/PatGram.g:82:10: ( ( label )? ( directive | bundle )? ( COMMENT )? NEWLINE )
            // java/src/grammar/PatGram.g:82:12: ( label )? ( directive | bundle )? ( COMMENT )? NEWLINE
            {
            // java/src/grammar/PatGram.g:82:12: ( label )?
            int alt3=2;
            int LA3_0 = input.LA(1);

            if ( (LA3_0==ID) ) {
                alt3=1;
            }
            switch (alt3) {
                case 1 :
                    // java/src/grammar/PatGram.g:82:13: label
                    {
                    pushFollow(FOLLOW_label_in_statement77);
                    label();

                    state._fsp--;


                    }
                    break;

            }

            // java/src/grammar/PatGram.g:82:21: ( directive | bundle )?
            int alt4=3;
            int LA4_0 = input.LA(1);

            if ( (LA4_0==13) ) {
                alt4=1;
            }
            else if ( (LA4_0==PRD||(LA4_0>=15 && LA4_0<=25)) ) {
                alt4=2;
            }
            switch (alt4) {
                case 1 :
                    // java/src/grammar/PatGram.g:82:22: directive
                    {
                    pushFollow(FOLLOW_directive_in_statement82);
                    directive();

                    state._fsp--;


                    }
                    break;
                case 2 :
                    // java/src/grammar/PatGram.g:82:34: bundle
                    {
                    pushFollow(FOLLOW_bundle_in_statement86);
                    bundle();

                    state._fsp--;


                    }
                    break;

            }

            // java/src/grammar/PatGram.g:82:43: ( COMMENT )?
            int alt5=2;
            int LA5_0 = input.LA(1);

            if ( (LA5_0==COMMENT) ) {
                alt5=1;
            }
            switch (alt5) {
                case 1 :
                    // java/src/grammar/PatGram.g:82:44: COMMENT
                    {
                    match(input,COMMENT,FOLLOW_COMMENT_in_statement91); 

                    }
                    break;

            }

            match(input,NEWLINE,FOLLOW_NEWLINE_in_statement95); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end "statement"


    // $ANTLR start "label"
    // java/src/grammar/PatGram.g:84:1: label : ID ':' ;
    public final void label() throws RecognitionException {
        Token ID1=null;

        try {
            // java/src/grammar/PatGram.g:84:6: ( ID ':' )
            // java/src/grammar/PatGram.g:84:9: ID ':'
            {
            ID1=(Token)match(input,ID,FOLLOW_ID_in_label103); 
            match(input,12,FOLLOW_12_in_label105); 
            symbols.put((ID1!=null?ID1.getText():null), new Integer(pc));

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end "label"


    // $ANTLR start "directive"
    // java/src/grammar/PatGram.g:87:1: directive : '.start' ;
    public final void directive() throws RecognitionException {
        try {
            // java/src/grammar/PatGram.g:87:10: ( '.start' )
            // java/src/grammar/PatGram.g:87:12: '.start'
            {
            match(input,13,FOLLOW_13_in_directive115); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end "directive"


    // $ANTLR start "bundle"
    // java/src/grammar/PatGram.g:90:1: bundle : instruction ;
    public final void bundle() throws RecognitionException {
        try {
            // java/src/grammar/PatGram.g:90:7: ( instruction )
            // java/src/grammar/PatGram.g:90:9: instruction
            {
            pushFollow(FOLLOW_instruction_in_bundle123);
            instruction();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end "bundle"


    // $ANTLR start "instruction"
    // java/src/grammar/PatGram.g:92:1: instruction : ( alu_imm | alu | compare | branch );
    public final void instruction() throws RecognitionException {
        try {
            // java/src/grammar/PatGram.g:92:12: ( alu_imm | alu | compare | branch )
            int alt6=4;
            switch ( input.LA(1) ) {
            case PRD:
                {
                switch ( input.LA(2) ) {
                case 17:
                case 18:
                case 19:
                case 20:
                    {
                    alt6=1;
                    }
                    break;
                case 21:
                case 22:
                case 23:
                case 24:
                case 25:
                    {
                    alt6=2;
                    }
                    break;
                case 15:
                    {
                    alt6=3;
                    }
                    break;
                case 16:
                    {
                    alt6=4;
                    }
                    break;
                default:
                    NoViableAltException nvae =
                        new NoViableAltException("", 6, 1, input);

                    throw nvae;
                }

                }
                break;
            case 17:
            case 18:
            case 19:
            case 20:
                {
                alt6=1;
                }
                break;
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
                {
                alt6=2;
                }
                break;
            case 15:
                {
                alt6=3;
                }
                break;
            case 16:
                {
                alt6=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("", 6, 0, input);

                throw nvae;
            }

            switch (alt6) {
                case 1 :
                    // java/src/grammar/PatGram.g:92:14: alu_imm
                    {
                    pushFollow(FOLLOW_alu_imm_in_instruction130);
                    alu_imm();

                    state._fsp--;


                    }
                    break;
                case 2 :
                    // java/src/grammar/PatGram.g:92:24: alu
                    {
                    pushFollow(FOLLOW_alu_in_instruction134);
                    alu();

                    state._fsp--;


                    }
                    break;
                case 3 :
                    // java/src/grammar/PatGram.g:92:30: compare
                    {
                    pushFollow(FOLLOW_compare_in_instruction138);
                    compare();

                    state._fsp--;


                    }
                    break;
                case 4 :
                    // java/src/grammar/PatGram.g:92:40: branch
                    {
                    pushFollow(FOLLOW_branch_in_instruction142);
                    branch();

                    state._fsp--;


                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end "instruction"


    // $ANTLR start "alu_imm"
    // java/src/grammar/PatGram.g:94:1: alu_imm returns [int opc] : ( pred )? op_imm rs1 ',' imm_val TO rd ;
    public final int alu_imm() throws RecognitionException {
        int opc = 0;

        PatGramParser.op_imm_return op_imm2 = null;

        int pred3 = 0;

        int rs14 = 0;

        int rd5 = 0;

        int imm_val6 = 0;


        try {
            // java/src/grammar/PatGram.g:95:2: ( ( pred )? op_imm rs1 ',' imm_val TO rd )
            // java/src/grammar/PatGram.g:95:4: ( pred )? op_imm rs1 ',' imm_val TO rd
            {
            // java/src/grammar/PatGram.g:95:4: ( pred )?
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( (LA7_0==PRD) ) {
                alt7=1;
            }
            switch (alt7) {
                case 1 :
                    // java/src/grammar/PatGram.g:95:5: pred
                    {
                    pushFollow(FOLLOW_pred_in_alu_imm156);
                    pred3=pred();

                    state._fsp--;


                    }
                    break;

            }

            pushFollow(FOLLOW_op_imm_in_alu_imm160);
            op_imm2=op_imm();

            state._fsp--;

            pushFollow(FOLLOW_rs1_in_alu_imm162);
            rs14=rs1();

            state._fsp--;

            match(input,14,FOLLOW_14_in_alu_imm164); 
            pushFollow(FOLLOW_imm_val_in_alu_imm166);
            imm_val6=imm_val();

            state._fsp--;

            match(input,TO,FOLLOW_TO_in_alu_imm168); 
            pushFollow(FOLLOW_rd_in_alu_imm170);
            rd5=rd();

            state._fsp--;


            		opc = (op_imm2!=null?op_imm2.opc:0) + (pred3<<22) +
            			(rs14<<12) + (rd5<<17) +
            			imm_val6;
            		System.out.println(pc+" "+niceHex(opc)+
            			" p"+pred3+" "+(op_imm2!=null?input.toString(op_imm2.start,op_imm2.stop):null));
            		if (pass2) { code[pc] = opc; }
            		++pc;
            	

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return opc;
    }
    // $ANTLR end "alu_imm"


    // $ANTLR start "alu"
    // java/src/grammar/PatGram.g:106:1: alu returns [int opc] : ( pred )? op_alu rs1 ',' rs2 TO rd ;
    public final int alu() throws RecognitionException {
        int opc = 0;

        int pred7 = 0;

        int rs18 = 0;

        int rs29 = 0;

        int rd10 = 0;

        PatGramParser.op_alu_return op_alu11 = null;


        try {
            // java/src/grammar/PatGram.g:107:2: ( ( pred )? op_alu rs1 ',' rs2 TO rd )
            // java/src/grammar/PatGram.g:107:4: ( pred )? op_alu rs1 ',' rs2 TO rd
            {
            // java/src/grammar/PatGram.g:107:4: ( pred )?
            int alt8=2;
            int LA8_0 = input.LA(1);

            if ( (LA8_0==PRD) ) {
                alt8=1;
            }
            switch (alt8) {
                case 1 :
                    // java/src/grammar/PatGram.g:107:5: pred
                    {
                    pushFollow(FOLLOW_pred_in_alu187);
                    pred7=pred();

                    state._fsp--;


                    }
                    break;

            }

            pushFollow(FOLLOW_op_alu_in_alu191);
            op_alu11=op_alu();

            state._fsp--;

            pushFollow(FOLLOW_rs1_in_alu193);
            rs18=rs1();

            state._fsp--;

            match(input,14,FOLLOW_14_in_alu195); 
            pushFollow(FOLLOW_rs2_in_alu197);
            rs29=rs2();

            state._fsp--;

            match(input,TO,FOLLOW_TO_in_alu199); 
            pushFollow(FOLLOW_rd_in_alu201);
            rd10=rd();

            state._fsp--;


            		opc = (0x05<<26) + (pred7<<22) +
            			(rs18<<12) + (rs29<<7) + (rd10<<17) +
            			(op_alu11!=null?op_alu11.func:0);
            		System.out.println(pc+" "+niceHex(opc)+
            			" p"+pred7+" "+(op_alu11!=null?input.toString(op_alu11.start,op_alu11.stop):null));
            		if (pass2) { code[pc] = opc; }
            		++pc;
            	

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return opc;
    }
    // $ANTLR end "alu"


    // $ANTLR start "compare"
    // java/src/grammar/PatGram.g:118:1: compare returns [int opc] : ( pred )? 'cmp' rs1 op_cmp rs2 TO pdest ;
    public final int compare() throws RecognitionException {
        int opc = 0;

        int pred12 = 0;

        int rs113 = 0;

        int rs214 = 0;

        int pdest15 = 0;

        PatGramParser.op_cmp_return op_cmp16 = null;


        try {
            // java/src/grammar/PatGram.g:119:2: ( ( pred )? 'cmp' rs1 op_cmp rs2 TO pdest )
            // java/src/grammar/PatGram.g:119:4: ( pred )? 'cmp' rs1 op_cmp rs2 TO pdest
            {
            // java/src/grammar/PatGram.g:119:4: ( pred )?
            int alt9=2;
            int LA9_0 = input.LA(1);

            if ( (LA9_0==PRD) ) {
                alt9=1;
            }
            switch (alt9) {
                case 1 :
                    // java/src/grammar/PatGram.g:119:5: pred
                    {
                    pushFollow(FOLLOW_pred_in_compare218);
                    pred12=pred();

                    state._fsp--;


                    }
                    break;

            }

            match(input,15,FOLLOW_15_in_compare222); 
            pushFollow(FOLLOW_rs1_in_compare224);
            rs113=rs1();

            state._fsp--;

            pushFollow(FOLLOW_op_cmp_in_compare226);
            op_cmp16=op_cmp();

            state._fsp--;

            pushFollow(FOLLOW_rs2_in_compare228);
            rs214=rs2();

            state._fsp--;

            match(input,TO,FOLLOW_TO_in_compare230); 
            pushFollow(FOLLOW_pdest_in_compare232);
            pdest15=pdest();

            state._fsp--;


            		opc = (0x07<<26) + (pred12<<22) +
            			(rs113<<12) + (rs214<<7) + (pdest15<<17) +
            			(op_cmp16!=null?op_cmp16.func:0);
            		System.out.println(pc+" "+niceHex(opc)+
            			" p"+pred12+" cmp "+(op_cmp16!=null?input.toString(op_cmp16.start,op_cmp16.stop):null)+" pd"+pdest15);
            		if (pass2) { code[pc] = opc; }
            		++pc;
            	

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return opc;
    }
    // $ANTLR end "compare"


    // $ANTLR start "branch"
    // java/src/grammar/PatGram.g:130:1: branch returns [int opc] : ( pred )? 'br' ID ;
    public final int branch() throws RecognitionException {
        int opc = 0;

        Token ID17=null;
        int pred18 = 0;


        try {
            // java/src/grammar/PatGram.g:131:2: ( ( pred )? 'br' ID )
            // java/src/grammar/PatGram.g:131:4: ( pred )? 'br' ID
            {
            // java/src/grammar/PatGram.g:131:4: ( pred )?
            int alt10=2;
            int LA10_0 = input.LA(1);

            if ( (LA10_0==PRD) ) {
                alt10=1;
            }
            switch (alt10) {
                case 1 :
                    // java/src/grammar/PatGram.g:131:5: pred
                    {
                    pushFollow(FOLLOW_pred_in_branch249);
                    pred18=pred();

                    state._fsp--;


                    }
                    break;

            }

            match(input,16,FOLLOW_16_in_branch253); 
            ID17=(Token)match(input,ID,FOLLOW_ID_in_branch255); 

            		int off = 0;
            		if (pass2) {
            			Integer v = (Integer) symbols.get((ID17!=null?ID17.getText():null));
                    		if ( v!=null ) {
            				off = v.intValue();
            		        } else {
            				throw new Error("Undefined label "+(ID17!=null?ID17.getText():null));
            			}
            			off = off - pc;
            			// TODO test maximum offset
            			// at the moment 22 bits offset
            			off &= 0x3fffff;
            		}
            		opc = (0x06<<26) + (pred18<<22) + off;
            		System.out.println(pc+" "+niceHex(opc)+
            			" p"+pred18+" br "+((off<<10)>>10));
            		if (pass2) { code[pc] = opc; }
            		++pc;
            	

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return opc;
    }
    // $ANTLR end "branch"


    // $ANTLR start "rs1"
    // java/src/grammar/PatGram.g:153:1: rs1 returns [int value] : register ;
    public final int rs1() throws RecognitionException {
        int value = 0;

        int register19 = 0;


        try {
            // java/src/grammar/PatGram.g:153:24: ( register )
            // java/src/grammar/PatGram.g:153:26: register
            {
            pushFollow(FOLLOW_register_in_rs1269);
            register19=register();

            state._fsp--;

            value = register19;

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return value;
    }
    // $ANTLR end "rs1"


    // $ANTLR start "rs2"
    // java/src/grammar/PatGram.g:154:1: rs2 returns [int value] : register ;
    public final int rs2() throws RecognitionException {
        int value = 0;

        int register20 = 0;


        try {
            // java/src/grammar/PatGram.g:154:24: ( register )
            // java/src/grammar/PatGram.g:154:26: register
            {
            pushFollow(FOLLOW_register_in_rs2281);
            register20=register();

            state._fsp--;

            value = register20;

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return value;
    }
    // $ANTLR end "rs2"


    // $ANTLR start "rd"
    // java/src/grammar/PatGram.g:155:1: rd returns [int value] : register ;
    public final int rd() throws RecognitionException {
        int value = 0;

        int register21 = 0;


        try {
            // java/src/grammar/PatGram.g:155:23: ( register )
            // java/src/grammar/PatGram.g:155:25: register
            {
            pushFollow(FOLLOW_register_in_rd293);
            register21=register();

            state._fsp--;

            value = register21;

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return value;
    }
    // $ANTLR end "rd"


    // $ANTLR start "register"
    // java/src/grammar/PatGram.g:158:1: register returns [int value] : REG ;
    public final int register() throws RecognitionException {
        int value = 0;

        Token REG22=null;

        try {
            // java/src/grammar/PatGram.g:159:2: ( REG )
            // java/src/grammar/PatGram.g:159:4: REG
            {
            REG22=(Token)match(input,REG,FOLLOW_REG_in_register309); 
            value = Integer.parseInt((REG22!=null?REG22.getText():null).substring(1));
            		if (value<0 || value>31) throw new Error("Wrong register name");

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return value;
    }
    // $ANTLR end "register"


    // $ANTLR start "pred"
    // java/src/grammar/PatGram.g:162:1: pred returns [int value] : PRD ;
    public final int pred() throws RecognitionException {
        int value = 0;

        Token PRD23=null;

        try {
            // java/src/grammar/PatGram.g:163:2: ( PRD )
            // java/src/grammar/PatGram.g:163:4: PRD
            {
            PRD23=(Token)match(input,PRD,FOLLOW_PRD_in_pred324); 
            value = Integer.parseInt((PRD23!=null?PRD23.getText():null).substring(1));
            		if (value<0 || value>15) throw new Error("Wrong predicate name");

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return value;
    }
    // $ANTLR end "pred"


    // $ANTLR start "pdest"
    // java/src/grammar/PatGram.g:166:1: pdest returns [int value] : PRD ;
    public final int pdest() throws RecognitionException {
        int value = 0;

        Token PRD24=null;

        try {
            // java/src/grammar/PatGram.g:167:2: ( PRD )
            // java/src/grammar/PatGram.g:167:4: PRD
            {
            PRD24=(Token)match(input,PRD,FOLLOW_PRD_in_pdest339); 
            value = Integer.parseInt((PRD24!=null?PRD24.getText():null).substring(1));
            		if (value<0 || value>15) throw new Error("Wrong predicate name");

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return value;
    }
    // $ANTLR end "pdest"


    // $ANTLR start "imm_val"
    // java/src/grammar/PatGram.g:170:1: imm_val returns [int value] : INT ;
    public final int imm_val() throws RecognitionException {
        int value = 0;

        Token INT25=null;

        try {
            // java/src/grammar/PatGram.g:171:5: ( INT )
            // java/src/grammar/PatGram.g:171:7: INT
            {
            INT25=(Token)match(input,INT,FOLLOW_INT_in_imm_val357); 
            value = Integer.parseInt((INT25!=null?INT25.getText():null));
            		if (value<-2048 || value>2047) throw new Error("Wrong immediate");

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return value;
    }
    // $ANTLR end "imm_val"

    public static class op_imm_return extends ParserRuleReturnScope {
        public int opc;
    };

    // $ANTLR start "op_imm"
    // java/src/grammar/PatGram.g:174:1: op_imm returns [int opc] : ( 'addi' | 'ori' | 'andi' | 'xori' );
    public final PatGramParser.op_imm_return op_imm() throws RecognitionException {
        PatGramParser.op_imm_return retval = new PatGramParser.op_imm_return();
        retval.start = input.LT(1);

        try {
            // java/src/grammar/PatGram.g:174:25: ( 'addi' | 'ori' | 'andi' | 'xori' )
            int alt11=4;
            switch ( input.LA(1) ) {
            case 17:
                {
                alt11=1;
                }
                break;
            case 18:
                {
                alt11=2;
                }
                break;
            case 19:
                {
                alt11=3;
                }
                break;
            case 20:
                {
                alt11=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("", 11, 0, input);

                throw nvae;
            }

            switch (alt11) {
                case 1 :
                    // java/src/grammar/PatGram.g:175:2: 'addi'
                    {
                    match(input,17,FOLLOW_17_in_op_imm372); 
                    retval.opc = 0<<26;

                    }
                    break;
                case 2 :
                    // java/src/grammar/PatGram.g:176:2: 'ori'
                    {
                    match(input,18,FOLLOW_18_in_op_imm379); 
                    retval.opc =  1<<26;

                    }
                    break;
                case 3 :
                    // java/src/grammar/PatGram.g:177:2: 'andi'
                    {
                    match(input,19,FOLLOW_19_in_op_imm386); 
                    retval.opc = 2<<26;

                    }
                    break;
                case 4 :
                    // java/src/grammar/PatGram.g:178:2: 'xori'
                    {
                    match(input,20,FOLLOW_20_in_op_imm393); 
                    retval.opc = 3<<26;

                    }
                    break;

            }
            retval.stop = input.LT(-1);

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return retval;
    }
    // $ANTLR end "op_imm"

    public static class op_alu_return extends ParserRuleReturnScope {
        public int func;
    };

    // $ANTLR start "op_alu"
    // java/src/grammar/PatGram.g:181:1: op_alu returns [int func] : ( 'add' | 'sub' | 'or' | 'and' | 'xor' );
    public final PatGramParser.op_alu_return op_alu() throws RecognitionException {
        PatGramParser.op_alu_return retval = new PatGramParser.op_alu_return();
        retval.start = input.LT(1);

        try {
            // java/src/grammar/PatGram.g:181:26: ( 'add' | 'sub' | 'or' | 'and' | 'xor' )
            int alt12=5;
            switch ( input.LA(1) ) {
            case 21:
                {
                alt12=1;
                }
                break;
            case 22:
                {
                alt12=2;
                }
                break;
            case 23:
                {
                alt12=3;
                }
                break;
            case 24:
                {
                alt12=4;
                }
                break;
            case 25:
                {
                alt12=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("", 12, 0, input);

                throw nvae;
            }

            switch (alt12) {
                case 1 :
                    // java/src/grammar/PatGram.g:182:2: 'add'
                    {
                    match(input,21,FOLLOW_21_in_op_alu410); 
                    retval.func = 0;

                    }
                    break;
                case 2 :
                    // java/src/grammar/PatGram.g:183:2: 'sub'
                    {
                    match(input,22,FOLLOW_22_in_op_alu417); 
                    retval.func = 1;

                    }
                    break;
                case 3 :
                    // java/src/grammar/PatGram.g:184:2: 'or'
                    {
                    match(input,23,FOLLOW_23_in_op_alu424); 
                    retval.func = 2;

                    }
                    break;
                case 4 :
                    // java/src/grammar/PatGram.g:185:2: 'and'
                    {
                    match(input,24,FOLLOW_24_in_op_alu431); 
                    retval.func = 3;

                    }
                    break;
                case 5 :
                    // java/src/grammar/PatGram.g:186:2: 'xor'
                    {
                    match(input,25,FOLLOW_25_in_op_alu438); 
                    retval.func = 4;

                    }
                    break;

            }
            retval.stop = input.LT(-1);

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return retval;
    }
    // $ANTLR end "op_alu"

    public static class op_cmp_return extends ParserRuleReturnScope {
        public int func;
    };

    // $ANTLR start "op_cmp"
    // java/src/grammar/PatGram.g:189:1: op_cmp returns [int func] : ( '==' | '!=' | '>=' | '<=' | '>' | '<' );
    public final PatGramParser.op_cmp_return op_cmp() throws RecognitionException {
        PatGramParser.op_cmp_return retval = new PatGramParser.op_cmp_return();
        retval.start = input.LT(1);

        try {
            // java/src/grammar/PatGram.g:189:26: ( '==' | '!=' | '>=' | '<=' | '>' | '<' )
            int alt13=6;
            switch ( input.LA(1) ) {
            case 26:
                {
                alt13=1;
                }
                break;
            case 27:
                {
                alt13=2;
                }
                break;
            case 28:
                {
                alt13=3;
                }
                break;
            case 29:
                {
                alt13=4;
                }
                break;
            case 30:
                {
                alt13=5;
                }
                break;
            case 31:
                {
                alt13=6;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("", 13, 0, input);

                throw nvae;
            }

            switch (alt13) {
                case 1 :
                    // java/src/grammar/PatGram.g:190:2: '=='
                    {
                    match(input,26,FOLLOW_26_in_op_cmp454); 
                    retval.func = 0;

                    }
                    break;
                case 2 :
                    // java/src/grammar/PatGram.g:191:2: '!='
                    {
                    match(input,27,FOLLOW_27_in_op_cmp461); 
                    retval.func = 1;

                    }
                    break;
                case 3 :
                    // java/src/grammar/PatGram.g:192:2: '>='
                    {
                    match(input,28,FOLLOW_28_in_op_cmp468); 
                    retval.func = 2;

                    }
                    break;
                case 4 :
                    // java/src/grammar/PatGram.g:193:2: '<='
                    {
                    match(input,29,FOLLOW_29_in_op_cmp475); 
                    retval.func = 3;

                    }
                    break;
                case 5 :
                    // java/src/grammar/PatGram.g:194:2: '>'
                    {
                    match(input,30,FOLLOW_30_in_op_cmp482); 
                    retval.func = 4;

                    }
                    break;
                case 6 :
                    // java/src/grammar/PatGram.g:195:2: '<'
                    {
                    match(input,31,FOLLOW_31_in_op_cmp489); 
                    retval.func = 5;

                    }
                    break;

            }
            retval.stop = input.LT(-1);

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return retval;
    }
    // $ANTLR end "op_cmp"

    // Delegated rules


 

    public static final BitSet FOLLOW_statement_in_pass139 = new BitSet(new long[]{0x0000000003FFA272L});
    public static final BitSet FOLLOW_statement_in_pass265 = new BitSet(new long[]{0x0000000003FFA272L});
    public static final BitSet FOLLOW_label_in_statement77 = new BitSet(new long[]{0x0000000003FFA230L});
    public static final BitSet FOLLOW_directive_in_statement82 = new BitSet(new long[]{0x0000000000000030L});
    public static final BitSet FOLLOW_bundle_in_statement86 = new BitSet(new long[]{0x0000000000000030L});
    public static final BitSet FOLLOW_COMMENT_in_statement91 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_NEWLINE_in_statement95 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ID_in_label103 = new BitSet(new long[]{0x0000000000001000L});
    public static final BitSet FOLLOW_12_in_label105 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_directive115 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_instruction_in_bundle123 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_alu_imm_in_instruction130 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_alu_in_instruction134 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_compare_in_instruction138 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_branch_in_instruction142 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_pred_in_alu_imm156 = new BitSet(new long[]{0x00000000001E0200L});
    public static final BitSet FOLLOW_op_imm_in_alu_imm160 = new BitSet(new long[]{0x0000000000000100L});
    public static final BitSet FOLLOW_rs1_in_alu_imm162 = new BitSet(new long[]{0x0000000000004000L});
    public static final BitSet FOLLOW_14_in_alu_imm164 = new BitSet(new long[]{0x0000000000000400L});
    public static final BitSet FOLLOW_imm_val_in_alu_imm166 = new BitSet(new long[]{0x0000000000000080L});
    public static final BitSet FOLLOW_TO_in_alu_imm168 = new BitSet(new long[]{0x0000000000000100L});
    public static final BitSet FOLLOW_rd_in_alu_imm170 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_pred_in_alu187 = new BitSet(new long[]{0x0000000003E00200L});
    public static final BitSet FOLLOW_op_alu_in_alu191 = new BitSet(new long[]{0x0000000000000100L});
    public static final BitSet FOLLOW_rs1_in_alu193 = new BitSet(new long[]{0x0000000000004000L});
    public static final BitSet FOLLOW_14_in_alu195 = new BitSet(new long[]{0x0000000000000100L});
    public static final BitSet FOLLOW_rs2_in_alu197 = new BitSet(new long[]{0x0000000000000080L});
    public static final BitSet FOLLOW_TO_in_alu199 = new BitSet(new long[]{0x0000000000000100L});
    public static final BitSet FOLLOW_rd_in_alu201 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_pred_in_compare218 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_15_in_compare222 = new BitSet(new long[]{0x0000000000000100L});
    public static final BitSet FOLLOW_rs1_in_compare224 = new BitSet(new long[]{0x00000000FC000000L});
    public static final BitSet FOLLOW_op_cmp_in_compare226 = new BitSet(new long[]{0x0000000000000100L});
    public static final BitSet FOLLOW_rs2_in_compare228 = new BitSet(new long[]{0x0000000000000080L});
    public static final BitSet FOLLOW_TO_in_compare230 = new BitSet(new long[]{0x0000000000000200L});
    public static final BitSet FOLLOW_pdest_in_compare232 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_pred_in_branch249 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_16_in_branch253 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_ID_in_branch255 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_register_in_rs1269 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_register_in_rs2281 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_register_in_rd293 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_REG_in_register309 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_PRD_in_pred324 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_PRD_in_pdest339 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_INT_in_imm_val357 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_op_imm372 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_op_imm379 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_op_imm386 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_op_imm393 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_op_alu410 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_op_alu417 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_op_alu424 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_op_alu431 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_op_alu438 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_op_cmp454 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_op_cmp461 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_op_cmp468 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_op_cmp475 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_op_cmp482 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_op_cmp489 = new BitSet(new long[]{0x0000000000000002L});

}