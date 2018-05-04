/*
 * Asynchronous 2-input arbiter used to build the asynchronous 
 * arbiter tree
 *
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */
module AsyncArbiter2 (ack, req1, req2, ack1, ack2, req);

input ack;
input req1;
input req2;

output ack1 /* synthesis keep */;
output ack2 /* synthesis keep */;
output req /* synthesis keep */;

wire g1;
wire g2;

wire y1 /* synthesis keep */;
wire y2 /* synthesis keep */;

AsyncMutex2 mutex(req1, req2, g1, g2);

assign y1 = g1 & ~ack2;
assign y2 = g2 & ~ack1;

assign req = (y1 & ~ack2) | (y2 & ~ack1);

assign ack1 = (ack & y1) | ack1 & (ack | y1);
assign ack2 = (ack & y2) | ack2 & (ack | y2);

endmodule
