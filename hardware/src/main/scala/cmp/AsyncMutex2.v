/*
 * Asynchronous mutex
 *
 * Author: Torur Biskopsto Strom (torur.strom@gmail.com)
 *
 */
module AsyncMutex2 (reqin1, reqin2, reqout1, reqout2);

input reqin1;
input reqin2;

output reqout1 /* synthesis keep */;
output reqout2 /* synthesis keep */;

wire o1 /* synthesis keep */;
wire o2 /* synthesis keep */;

assign o1 = ~(reqin1 & o2);
assign o2 = ~(reqin2 & o1);

assign reqout1 = ~o1 & o2;
assign reqout2 = ~o2 & o1;

endmodule
