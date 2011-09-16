// $ANTLR 3.3 Nov 30, 2010 12:50:56 java/src/grammar/PatGram.g 2011-09-16 23:23:11
package patmos.asm.generated;

import org.antlr.runtime.*;
import java.util.Stack;
import java.util.List;
import java.util.ArrayList;

public class PatGramLexer extends Lexer {
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

    public PatGramLexer() {;} 
    public PatGramLexer(CharStream input) {
        this(input, new RecognizerSharedState());
    }
    public PatGramLexer(CharStream input, RecognizerSharedState state) {
        super(input,state);

    }
    public String getGrammarFileName() { return "java/src/grammar/PatGram.g"; }

    // $ANTLR start "T__12"
    public final void mT__12() throws RecognitionException {
        try {
            int _type = T__12;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:5:7: ( ':' )
            // java/src/grammar/PatGram.g:5:9: ':'
            {
            match(':'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__12"

    // $ANTLR start "T__13"
    public final void mT__13() throws RecognitionException {
        try {
            int _type = T__13;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:6:7: ( '.start' )
            // java/src/grammar/PatGram.g:6:9: '.start'
            {
            match(".start"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__13"

    // $ANTLR start "T__14"
    public final void mT__14() throws RecognitionException {
        try {
            int _type = T__14;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:7:7: ( ',' )
            // java/src/grammar/PatGram.g:7:9: ','
            {
            match(','); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__14"

    // $ANTLR start "T__15"
    public final void mT__15() throws RecognitionException {
        try {
            int _type = T__15;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:8:7: ( 'cmp' )
            // java/src/grammar/PatGram.g:8:9: 'cmp'
            {
            match("cmp"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__15"

    // $ANTLR start "T__16"
    public final void mT__16() throws RecognitionException {
        try {
            int _type = T__16;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:9:7: ( 'br' )
            // java/src/grammar/PatGram.g:9:9: 'br'
            {
            match("br"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__16"

    // $ANTLR start "T__17"
    public final void mT__17() throws RecognitionException {
        try {
            int _type = T__17;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:10:7: ( 'addi' )
            // java/src/grammar/PatGram.g:10:9: 'addi'
            {
            match("addi"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__17"

    // $ANTLR start "T__18"
    public final void mT__18() throws RecognitionException {
        try {
            int _type = T__18;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:11:7: ( 'ori' )
            // java/src/grammar/PatGram.g:11:9: 'ori'
            {
            match("ori"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__18"

    // $ANTLR start "T__19"
    public final void mT__19() throws RecognitionException {
        try {
            int _type = T__19;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:12:7: ( 'andi' )
            // java/src/grammar/PatGram.g:12:9: 'andi'
            {
            match("andi"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__19"

    // $ANTLR start "T__20"
    public final void mT__20() throws RecognitionException {
        try {
            int _type = T__20;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:13:7: ( 'xori' )
            // java/src/grammar/PatGram.g:13:9: 'xori'
            {
            match("xori"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__20"

    // $ANTLR start "T__21"
    public final void mT__21() throws RecognitionException {
        try {
            int _type = T__21;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:14:7: ( 'add' )
            // java/src/grammar/PatGram.g:14:9: 'add'
            {
            match("add"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__21"

    // $ANTLR start "T__22"
    public final void mT__22() throws RecognitionException {
        try {
            int _type = T__22;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:15:7: ( 'sub' )
            // java/src/grammar/PatGram.g:15:9: 'sub'
            {
            match("sub"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__22"

    // $ANTLR start "T__23"
    public final void mT__23() throws RecognitionException {
        try {
            int _type = T__23;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:16:7: ( 'or' )
            // java/src/grammar/PatGram.g:16:9: 'or'
            {
            match("or"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__23"

    // $ANTLR start "T__24"
    public final void mT__24() throws RecognitionException {
        try {
            int _type = T__24;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:17:7: ( 'and' )
            // java/src/grammar/PatGram.g:17:9: 'and'
            {
            match("and"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__24"

    // $ANTLR start "T__25"
    public final void mT__25() throws RecognitionException {
        try {
            int _type = T__25;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:18:7: ( 'xor' )
            // java/src/grammar/PatGram.g:18:9: 'xor'
            {
            match("xor"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__25"

    // $ANTLR start "T__26"
    public final void mT__26() throws RecognitionException {
        try {
            int _type = T__26;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:19:7: ( '==' )
            // java/src/grammar/PatGram.g:19:9: '=='
            {
            match("=="); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__26"

    // $ANTLR start "T__27"
    public final void mT__27() throws RecognitionException {
        try {
            int _type = T__27;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:20:7: ( '!=' )
            // java/src/grammar/PatGram.g:20:9: '!='
            {
            match("!="); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__27"

    // $ANTLR start "T__28"
    public final void mT__28() throws RecognitionException {
        try {
            int _type = T__28;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:21:7: ( '>=' )
            // java/src/grammar/PatGram.g:21:9: '>='
            {
            match(">="); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__28"

    // $ANTLR start "T__29"
    public final void mT__29() throws RecognitionException {
        try {
            int _type = T__29;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:22:7: ( '<=' )
            // java/src/grammar/PatGram.g:22:9: '<='
            {
            match("<="); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__29"

    // $ANTLR start "T__30"
    public final void mT__30() throws RecognitionException {
        try {
            int _type = T__30;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:23:7: ( '>' )
            // java/src/grammar/PatGram.g:23:9: '>'
            {
            match('>'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__30"

    // $ANTLR start "T__31"
    public final void mT__31() throws RecognitionException {
        try {
            int _type = T__31;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:24:7: ( '<' )
            // java/src/grammar/PatGram.g:24:9: '<'
            {
            match('<'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__31"

    // $ANTLR start "INT"
    public final void mINT() throws RecognitionException {
        try {
            int _type = INT;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:200:5: ( ( '0' .. '9' )+ )
            // java/src/grammar/PatGram.g:200:9: ( '0' .. '9' )+
            {
            // java/src/grammar/PatGram.g:200:9: ( '0' .. '9' )+
            int cnt1=0;
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( ((LA1_0>='0' && LA1_0<='9')) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // java/src/grammar/PatGram.g:200:9: '0' .. '9'
            	    {
            	    matchRange('0','9'); 

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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "INT"

    // $ANTLR start "REG"
    public final void mREG() throws RecognitionException {
        try {
            int _type = REG;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:201:4: ( 'r' INT )
            // java/src/grammar/PatGram.g:201:6: 'r' INT
            {
            match('r'); 
            mINT(); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "REG"

    // $ANTLR start "PRD"
    public final void mPRD() throws RecognitionException {
        try {
            int _type = PRD;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:202:4: ( 'p' INT )
            // java/src/grammar/PatGram.g:202:6: 'p' INT
            {
            match('p'); 
            mINT(); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "PRD"

    // $ANTLR start "TO"
    public final void mTO() throws RecognitionException {
        try {
            int _type = TO;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:203:3: ( '->' )
            // java/src/grammar/PatGram.g:203:5: '->'
            {
            match("->"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "TO"

    // $ANTLR start "ID"
    public final void mID() throws RecognitionException {
        try {
            int _type = ID;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:205:3: ( ( 'a' .. 'z' | 'A' .. 'Z' | '_' ) ( 'a' .. 'z' | 'A' .. 'Z' | '_' | '0' .. '9' )* )
            // java/src/grammar/PatGram.g:205:5: ( 'a' .. 'z' | 'A' .. 'Z' | '_' ) ( 'a' .. 'z' | 'A' .. 'Z' | '_' | '0' .. '9' )*
            {
            if ( (input.LA(1)>='A' && input.LA(1)<='Z')||input.LA(1)=='_'||(input.LA(1)>='a' && input.LA(1)<='z') ) {
                input.consume();

            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;}

            // java/src/grammar/PatGram.g:205:29: ( 'a' .. 'z' | 'A' .. 'Z' | '_' | '0' .. '9' )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( ((LA2_0>='0' && LA2_0<='9')||(LA2_0>='A' && LA2_0<='Z')||LA2_0=='_'||(LA2_0>='a' && LA2_0<='z')) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // java/src/grammar/PatGram.g:
            	    {
            	    if ( (input.LA(1)>='0' && input.LA(1)<='9')||(input.LA(1)>='A' && input.LA(1)<='Z')||input.LA(1)=='_'||(input.LA(1)>='a' && input.LA(1)<='z') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop2;
                }
            } while (true);


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "ID"

    // $ANTLR start "COMMENT"
    public final void mCOMMENT() throws RecognitionException {
        try {
            int _type = COMMENT;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:207:8: ( '//' (~ ( '\\n' | '\\r' ) )* )
            // java/src/grammar/PatGram.g:207:10: '//' (~ ( '\\n' | '\\r' ) )*
            {
            match("//"); 

            // java/src/grammar/PatGram.g:207:15: (~ ( '\\n' | '\\r' ) )*
            loop3:
            do {
                int alt3=2;
                int LA3_0 = input.LA(1);

                if ( ((LA3_0>='\u0000' && LA3_0<='\t')||(LA3_0>='\u000B' && LA3_0<='\f')||(LA3_0>='\u000E' && LA3_0<='\uFFFF')) ) {
                    alt3=1;
                }


                switch (alt3) {
            	case 1 :
            	    // java/src/grammar/PatGram.g:207:15: ~ ( '\\n' | '\\r' )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop3;
                }
            } while (true);


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "COMMENT"

    // $ANTLR start "NEWLINE"
    public final void mNEWLINE() throws RecognitionException {
        try {
            int _type = NEWLINE;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:209:8: ( ( '\\r' )? '\\n' )
            // java/src/grammar/PatGram.g:209:10: ( '\\r' )? '\\n'
            {
            // java/src/grammar/PatGram.g:209:10: ( '\\r' )?
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( (LA4_0=='\r') ) {
                alt4=1;
            }
            switch (alt4) {
                case 1 :
                    // java/src/grammar/PatGram.g:209:10: '\\r'
                    {
                    match('\r'); 

                    }
                    break;

            }

            match('\n'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "NEWLINE"

    // $ANTLR start "WHITSPACE"
    public final void mWHITSPACE() throws RecognitionException {
        try {
            int _type = WHITSPACE;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // java/src/grammar/PatGram.g:210:10: ( ( ' ' | '\\t' )+ )
            // java/src/grammar/PatGram.g:210:14: ( ' ' | '\\t' )+
            {
            // java/src/grammar/PatGram.g:210:14: ( ' ' | '\\t' )+
            int cnt5=0;
            loop5:
            do {
                int alt5=2;
                int LA5_0 = input.LA(1);

                if ( (LA5_0=='\t'||LA5_0==' ') ) {
                    alt5=1;
                }


                switch (alt5) {
            	case 1 :
            	    // java/src/grammar/PatGram.g:
            	    {
            	    if ( input.LA(1)=='\t'||input.LA(1)==' ' ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    if ( cnt5 >= 1 ) break loop5;
                        EarlyExitException eee =
                            new EarlyExitException(5, input);
                        throw eee;
                }
                cnt5++;
            } while (true);

            skip();

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "WHITSPACE"

    public void mTokens() throws RecognitionException {
        // java/src/grammar/PatGram.g:1:8: ( T__12 | T__13 | T__14 | T__15 | T__16 | T__17 | T__18 | T__19 | T__20 | T__21 | T__22 | T__23 | T__24 | T__25 | T__26 | T__27 | T__28 | T__29 | T__30 | T__31 | INT | REG | PRD | TO | ID | COMMENT | NEWLINE | WHITSPACE )
        int alt6=28;
        alt6 = dfa6.predict(input);
        switch (alt6) {
            case 1 :
                // java/src/grammar/PatGram.g:1:10: T__12
                {
                mT__12(); 

                }
                break;
            case 2 :
                // java/src/grammar/PatGram.g:1:16: T__13
                {
                mT__13(); 

                }
                break;
            case 3 :
                // java/src/grammar/PatGram.g:1:22: T__14
                {
                mT__14(); 

                }
                break;
            case 4 :
                // java/src/grammar/PatGram.g:1:28: T__15
                {
                mT__15(); 

                }
                break;
            case 5 :
                // java/src/grammar/PatGram.g:1:34: T__16
                {
                mT__16(); 

                }
                break;
            case 6 :
                // java/src/grammar/PatGram.g:1:40: T__17
                {
                mT__17(); 

                }
                break;
            case 7 :
                // java/src/grammar/PatGram.g:1:46: T__18
                {
                mT__18(); 

                }
                break;
            case 8 :
                // java/src/grammar/PatGram.g:1:52: T__19
                {
                mT__19(); 

                }
                break;
            case 9 :
                // java/src/grammar/PatGram.g:1:58: T__20
                {
                mT__20(); 

                }
                break;
            case 10 :
                // java/src/grammar/PatGram.g:1:64: T__21
                {
                mT__21(); 

                }
                break;
            case 11 :
                // java/src/grammar/PatGram.g:1:70: T__22
                {
                mT__22(); 

                }
                break;
            case 12 :
                // java/src/grammar/PatGram.g:1:76: T__23
                {
                mT__23(); 

                }
                break;
            case 13 :
                // java/src/grammar/PatGram.g:1:82: T__24
                {
                mT__24(); 

                }
                break;
            case 14 :
                // java/src/grammar/PatGram.g:1:88: T__25
                {
                mT__25(); 

                }
                break;
            case 15 :
                // java/src/grammar/PatGram.g:1:94: T__26
                {
                mT__26(); 

                }
                break;
            case 16 :
                // java/src/grammar/PatGram.g:1:100: T__27
                {
                mT__27(); 

                }
                break;
            case 17 :
                // java/src/grammar/PatGram.g:1:106: T__28
                {
                mT__28(); 

                }
                break;
            case 18 :
                // java/src/grammar/PatGram.g:1:112: T__29
                {
                mT__29(); 

                }
                break;
            case 19 :
                // java/src/grammar/PatGram.g:1:118: T__30
                {
                mT__30(); 

                }
                break;
            case 20 :
                // java/src/grammar/PatGram.g:1:124: T__31
                {
                mT__31(); 

                }
                break;
            case 21 :
                // java/src/grammar/PatGram.g:1:130: INT
                {
                mINT(); 

                }
                break;
            case 22 :
                // java/src/grammar/PatGram.g:1:134: REG
                {
                mREG(); 

                }
                break;
            case 23 :
                // java/src/grammar/PatGram.g:1:138: PRD
                {
                mPRD(); 

                }
                break;
            case 24 :
                // java/src/grammar/PatGram.g:1:142: TO
                {
                mTO(); 

                }
                break;
            case 25 :
                // java/src/grammar/PatGram.g:1:145: ID
                {
                mID(); 

                }
                break;
            case 26 :
                // java/src/grammar/PatGram.g:1:148: COMMENT
                {
                mCOMMENT(); 

                }
                break;
            case 27 :
                // java/src/grammar/PatGram.g:1:156: NEWLINE
                {
                mNEWLINE(); 

                }
                break;
            case 28 :
                // java/src/grammar/PatGram.g:1:164: WHITSPACE
                {
                mWHITSPACE(); 

                }
                break;

        }

    }


    protected DFA6 dfa6 = new DFA6(this);
    static final String DFA6_eotS =
        "\4\uffff\6\22\2\uffff\1\36\1\40\1\uffff\2\22\5\uffff\1\22\1\44\2"+
        "\22\1\50\2\22\4\uffff\1\53\1\54\1\55\1\uffff\1\57\1\61\1\62\1\uffff"+
        "\1\64\1\65\3\uffff\1\66\1\uffff\1\67\2\uffff\1\70\5\uffff";
    static final String DFA6_eofS =
        "\71\uffff";
    static final String DFA6_minS =
        "\1\11\3\uffff\1\155\1\162\1\144\1\162\1\157\1\165\2\uffff\2\75\1"+
        "\uffff\2\60\5\uffff\1\160\1\60\2\144\1\60\1\162\1\142\4\uffff\3"+
        "\60\1\uffff\3\60\1\uffff\2\60\3\uffff\1\60\1\uffff\1\60\2\uffff"+
        "\1\60\5\uffff";
    static final String DFA6_maxS =
        "\1\172\3\uffff\1\155\1\162\1\156\1\162\1\157\1\165\2\uffff\2\75"+
        "\1\uffff\2\71\5\uffff\1\160\1\172\2\144\1\172\1\162\1\142\4\uffff"+
        "\3\172\1\uffff\3\172\1\uffff\2\172\3\uffff\1\172\1\uffff\1\172\2"+
        "\uffff\1\172\5\uffff";
    static final String DFA6_acceptS =
        "\1\uffff\1\1\1\2\1\3\6\uffff\1\17\1\20\2\uffff\1\25\2\uffff\1\30"+
        "\1\31\1\32\1\33\1\34\7\uffff\1\21\1\23\1\22\1\24\3\uffff\1\5\3\uffff"+
        "\1\14\2\uffff\1\26\1\27\1\4\1\uffff\1\12\1\uffff\1\15\1\7\1\uffff"+
        "\1\16\1\13\1\6\1\10\1\11";
    static final String DFA6_specialS =
        "\71\uffff}>";
    static final String[] DFA6_transitionS = {
            "\1\25\1\24\2\uffff\1\24\22\uffff\1\25\1\13\12\uffff\1\3\1\21"+
            "\1\2\1\23\12\16\1\1\1\uffff\1\15\1\12\1\14\2\uffff\32\22\4\uffff"+
            "\1\22\1\uffff\1\6\1\5\1\4\13\22\1\7\1\20\1\22\1\17\1\11\4\22"+
            "\1\10\2\22",
            "",
            "",
            "",
            "\1\26",
            "\1\27",
            "\1\30\11\uffff\1\31",
            "\1\32",
            "\1\33",
            "\1\34",
            "",
            "",
            "\1\35",
            "\1\37",
            "",
            "\12\41",
            "\12\42",
            "",
            "",
            "",
            "",
            "",
            "\1\43",
            "\12\22\7\uffff\32\22\4\uffff\1\22\1\uffff\32\22",
            "\1\45",
            "\1\46",
            "\12\22\7\uffff\32\22\4\uffff\1\22\1\uffff\10\22\1\47\21\22",
            "\1\51",
            "\1\52",
            "",
            "",
            "",
            "",
            "\12\41\7\uffff\32\22\4\uffff\1\22\1\uffff\32\22",
            "\12\42\7\uffff\32\22\4\uffff\1\22\1\uffff\32\22",
            "\12\22\7\uffff\32\22\4\uffff\1\22\1\uffff\32\22",
            "",
            "\12\22\7\uffff\32\22\4\uffff\1\22\1\uffff\10\22\1\56\21\22",
            "\12\22\7\uffff\32\22\4\uffff\1\22\1\uffff\10\22\1\60\21\22",
            "\12\22\7\uffff\32\22\4\uffff\1\22\1\uffff\32\22",
            "",
            "\12\22\7\uffff\32\22\4\uffff\1\22\1\uffff\10\22\1\63\21\22",
            "\12\22\7\uffff\32\22\4\uffff\1\22\1\uffff\32\22",
            "",
            "",
            "",
            "\12\22\7\uffff\32\22\4\uffff\1\22\1\uffff\32\22",
            "",
            "\12\22\7\uffff\32\22\4\uffff\1\22\1\uffff\32\22",
            "",
            "",
            "\12\22\7\uffff\32\22\4\uffff\1\22\1\uffff\32\22",
            "",
            "",
            "",
            "",
            ""
    };

    static final short[] DFA6_eot = DFA.unpackEncodedString(DFA6_eotS);
    static final short[] DFA6_eof = DFA.unpackEncodedString(DFA6_eofS);
    static final char[] DFA6_min = DFA.unpackEncodedStringToUnsignedChars(DFA6_minS);
    static final char[] DFA6_max = DFA.unpackEncodedStringToUnsignedChars(DFA6_maxS);
    static final short[] DFA6_accept = DFA.unpackEncodedString(DFA6_acceptS);
    static final short[] DFA6_special = DFA.unpackEncodedString(DFA6_specialS);
    static final short[][] DFA6_transition;

    static {
        int numStates = DFA6_transitionS.length;
        DFA6_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA6_transition[i] = DFA.unpackEncodedString(DFA6_transitionS[i]);
        }
    }

    class DFA6 extends DFA {

        public DFA6(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 6;
            this.eot = DFA6_eot;
            this.eof = DFA6_eof;
            this.min = DFA6_min;
            this.max = DFA6_max;
            this.accept = DFA6_accept;
            this.special = DFA6_special;
            this.transition = DFA6_transition;
        }
        public String getDescription() {
            return "1:1: Tokens : ( T__12 | T__13 | T__14 | T__15 | T__16 | T__17 | T__18 | T__19 | T__20 | T__21 | T__22 | T__23 | T__24 | T__25 | T__26 | T__27 | T__28 | T__29 | T__30 | T__31 | INT | REG | PRD | TO | ID | COMMENT | NEWLINE | WHITSPACE );";
        }
    }
 

}