/* LZSS encoder, based on implementation by Haruhiko Okumura
 *
 * Modified to use a 1k ring buffer (instead of the original 4k), and
 * 6 bits to encode the length (instead of the original 4 bits) to
 * compensate for the fewer index bits. Also modified for compressing
 * a stream instead of encoding in one pass.
 *
 * Adapted by Wolfgang Puffitsch 
 */

/***************************************************************
	4/6/1989 Haruhiko Okumura
	Use, distribute, and modify this program freely.
	Please send me your improved versions.
		PC-VAN		SCIENCE
		NIFTY-Serve	PAF01022
		CompuServe	74050,1022
**************************************************************/

package patserdow;

import java.io.IOException;
import java.io.OutputStream;
import java.io.FilterOutputStream;

public class CompressionOutputStream extends FilterOutputStream
{
    static final int INDEX_BITS = 10;                      /* bits to encode index */
    static final int LENGTH_BITS = (16-INDEX_BITS);        /* bits to encode length */
    static final int THRESHOLD = 2; /* encode string into position and length if
                                       match_length is greater than this */

    static final int N = (1 << INDEX_BITS);                /* size of ring buffer */
    static final int F = ((1 << LENGTH_BITS) + THRESHOLD); /* lookahead buffer size */

    static final int NIL = N;

    /* ring buffer of size N, with extra F-1 bytes to facilitate string comparison */
    byte textBuf [] = new byte[N + F - 1];
    /* position and length of longest match. These are set by the InsertNode() procedure. */
    int matchPosition, matchLength, lastMatchLength;
    /* left & right children & parents. These constitute binary search trees. */
    int lchild [] = new int[N + 1];
    int rchild [] = new int[N + 257];
    int parent [] = new int[N + 1];

    int codeBufPtr;
    byte codeBuf [] = new byte[17];
    byte codeMask;

    int currLen;
    int currPos, killPos;
    int writeCount; 

    public CompressionOutputStream(OutputStream out) {
        super(out);

        initTree(); /* initialize trees */
        
        codeBuf[0] = 0; /* code_buf[1..16] saves eight units of code,
                           and code_buf[0] works as eight flags, "1"
                           representing that the unit is an unencoded
                           letter (1 byte), "0" a position-and-length
                           pair (2 bytes).  Thus, eight units require
                           at most 16 bytes of code. */
        codeBufPtr = 1;
        codeMask = 1;

        currPos = N - F;
        currLen = 0;
        for (int i = 0; i < currPos; i++) {
            textBuf[i] = ' '; /* Clear the buffer with any character
                                 that will appear often. */
        }
        killPos = 0;
    }

    /* initialize trees */
    private void initTree()  {
        /* For i = 0 to N - 1, rchild[i] and lchild[i] will be the right and
           left children of node i.  These nodes need not be initialized.
           Also, parent[i] is the parent of node i.  These are initialized to
           NIL (= N), which stands for 'not used.'
           For i = 0 to 255, rchild[N + i + 1] is the root of the tree
           for strings that begin with character i.  These are initialized
           to NIL.  Note there are 256 trees. */
        for (int i = N + 1; i <= N + 256; i++) {
            rchild[i] = NIL;
        }
        for (int i = 0; i < N; i++) {
            parent[i] = NIL;
        }
    }

    /* insert node */
    private void insertNode(int r) {
        /* Inserts string of length F, textBuf[r..r+F-1], into one of the
           trees (textBuf[r]'th tree) and returns the longest-match position
           and length via the global variables matchPosition and matchLength.
           If matchLength = F, then removes the old node in favor of the new
           one, because the old one will be deleted sooner.
           Note r plays double role, as tree node and position in buffer. */
        int cmp = 1;
        int keyPos = r;
        int p = N + 1 + (textBuf[keyPos] & 0xff);
        rchild[r] = NIL;
        lchild[r] = NIL;
        matchLength = 0;
        for (;;) {
            if (cmp >= 0) {
                if (rchild[p] != NIL) {
                    p = rchild[p];
                } else {
                    rchild[p] = r;
                    parent[r] = p;
                    return;
                }
            } else {
                if (lchild[p] != NIL) {
                    p = lchild[p];
                } else {
                    lchild[p] = r;
                    parent[r] = p;
                    return;
                }
            }
            int i;
            for (i = 1; i < F; i++) {
                cmp = textBuf[keyPos + i] - textBuf[p + i];
                if (cmp != 0) {
                    break;
                }
            }
            if (i > matchLength) {
                matchPosition = p;
                matchLength = i;
                if (matchLength >= F) {
                    break;
                }
            }
        }
        parent[r] = parent[p];
        lchild[r] = lchild[p];
        rchild[r] = rchild[p];
        parent[lchild[p]] = r;
        parent[rchild[p]] = r;
        if (rchild[parent[p]] == p) {
            rchild[parent[p]] = r;
        } else {
            lchild[parent[p]] = r;
        }
        parent[p] = NIL;  /* remove p */
    }

    /* delete node p from tree */
    private void deleteNode(int p) {
        int  q;
    
        if (parent[p] == NIL) {
            return;  /* not in tree */
        }
        if (rchild[p] == NIL) {
            q = lchild[p];
        } else if (lchild[p] == NIL) {
            q = rchild[p];
        } else {
            q = lchild[p];
            if (rchild[q] != NIL) {
                do {
                    q = rchild[q];
                } while (rchild[q] != NIL);
                rchild[parent[q]] = lchild[q];
                parent[lchild[q]] = parent[q];
                lchild[q] = lchild[p];
                parent[lchild[p]] = q;
            }
            rchild[q] = rchild[p];
            parent[rchild[p]] = q;
        }
        parent[q] = parent[p];
        if (rchild[parent[p]] == p) {
            rchild[parent[p]] = q;
        } else {
            lchild[parent[p]] = q;
        }
        parent[p] = NIL;
    }

    private void encode() throws IOException {
        /* match_length may be spuriously long near the end of text. */
        if (matchLength > currLen) {
            matchLength = currLen;
        }
        
        if (matchLength <= THRESHOLD) {
            matchLength = 1;  /* Not long enough match.  Send one byte. */
            codeBuf[0] |= codeMask;  /* 'send one byte' flag */
            codeBuf[codeBufPtr++] = textBuf[currPos];  /* Send uncoded. */
        } else {
            codeBuf[codeBufPtr++] = (byte) matchPosition;
            /* Send position and length pair. Note match_length > THRESHOLD. */
            codeBuf[codeBufPtr++] = (byte)
                (((matchPosition >> (8 - LENGTH_BITS)) & (0xff << LENGTH_BITS) & 0xff)
                 | (matchLength - (THRESHOLD + 1)));
        }
        codeMask <<= 1;  /* Shift mask left one bit. */
        if (codeMask == 0) {
            flushBuffer();
        }
        lastMatchLength = matchLength;
    }

    private void flushBuffer() throws IOException {
        /* Send at most 8 units of code together, if buffer if not empty */
        if (codeBufPtr > 1) {
            out.write(codeBuf, 0, codeBufPtr);
            codeSize += codeBufPtr;
            codeBuf[0] = 0;
            codeBufPtr = 1;
            codeMask = 1;
        }
    }
    
    @Override
    public void write(int b) throws IOException {
        textSize++;
        if (currLen < F) {
            /* Read F bytes into the last F bytes of the buffer */
            textBuf[currPos + currLen] = (byte)b;
            currLen++;
            if (currLen == F) { 
                /* Insert the F strings, each of which begins with one
                   or more 'space' characters.  Note the order in
                   which these strings are inserted. This way,
                   degenerate trees will be less likely to occur. */
                for (int i = 1; i <= F; i++) {
                    insertNode(currPos - i);
                }
                /* Finally, insert the whole string just read.  The
                   global variables match_length and match_position
                   are set. */
                insertNode(currPos);
                encode();
            }
        } else {
            deleteNode(killPos);        /* Delete old strings and */
            textBuf[killPos] = (byte)b; /* read new bytes */
            if (killPos < F - 1) {
                /* If the position is near the end of buffer, extend
                   the buffer to make string comparison easier. */
                textBuf[killPos + N] = (byte)b;
            }
            /* Since this is a ring buffer, increment the position modulo N. */
            killPos = (killPos + 1) & (N - 1);
            currPos = (currPos + 1) & (N - 1);
            insertNode(currPos);  /* Register the string in textBuf[r..r+F-1] */
            writeCount++;
            if (writeCount == lastMatchLength) {
                encode();
                writeCount = 0;
            }
        }
    }

    public void finish() throws IOException {
        /* After the end of text, no need to read, but buffer may not
           be empty. */
        while (currLen > 0) {
            while (writeCount++ < lastMatchLength) {
                deleteNode(killPos);
                killPos = (killPos + 1) & (N - 1);
                currPos = (currPos + 1) & (N - 1);
                currLen--;
                if (currLen > 0) {
                    insertNode(currPos);
                }
            }
            encode();
            writeCount = 0;
        }
        flushBuffer();
        out.flush();
    }

    private long codeSize = 0;
    private long textSize = 0;

    public long getTextSize() {
        return textSize;
    }
    public long getCodeSize() {
        return codeSize;
    }
}
