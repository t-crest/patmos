module DC_1_way(input clk, input reset,
    input  io_rd,
    input  io_wr,
    input [31:0] io_data_in,
    output[31:0] io_data_out,
    input [31:0] io_mem_data_in,
    output[31:0] io_mem_data_out,
    input [31:0] io_address,
    output io_stall);

  wire T0;
  reg[1:0] state;
  wire T1;
  wire T2;
  wire T3;
  wire[4:0] T4;
  reg[4:0] burst_count;
  wire T5;
  wire[4:0] T6;
  wire[4:0] T7;
  wire[4:0] T8;
  wire[4:0] T9;
  wire[4:0] T10;
  wire T11;
  wire T12;
  reg[0:0] rd_reg;
  wire T13;
  wire T14;
  wire T15;
  reg[19:0] tag_dout;
  wire T16;
  wire T17;
  wire T18;
  wire[19:0] T19;
  wire[19:0] T20;
  reg [19:0] tag [1023:0];
  wire[19:0] T21;
  wire[19:0] T22;
  wire[19:0] T23;
  reg[31:0] address_reg;
  wire[31:0] T24;
  reg[9:0] index_number_reg;
  wire[9:0] T25;
  wire[9:0] T26;
  wire[9:0] index_number;
  wire[9:0] T27;
  wire[9:0] T28;
  wire T29;
  wire[9:0] T30;
  wire[19:0] T31;
  wire[19:0] T32;
  wire[19:0] T33;
  wire T34;
  wire T35;
  reg[0:0] wr_reg;
  wire T36;
  wire[9:0] T37;
  wire[5:0] T38;
  wire[19:0] T39;
  wire[3:0] T40;
  wire[9:0] T41;
  wire T42;
  reg[0:0] init;
  wire T43;
  wire[5:0] T44;
  wire[19:0] T45;
  wire[3:0] T46;
  wire[9:0] T47;
  wire[5:0] T48;
  wire[19:0] T49;
  wire[3:0] T50;
  wire[9:0] T51;
  wire[5:0] T52;
  wire[19:0] T53;
  wire[3:0] T54;
  wire[9:0] T55;
  wire[5:0] T56;
  wire[19:0] T57;
  wire[3:0] T58;
  wire[9:0] T59;
  wire[5:0] T60;
  wire[19:0] T61;
  wire[3:0] T62;
  wire[9:0] T63;
  wire[4:0] T64;
  wire[19:0] T65;
  wire[3:0] T66;
  wire[9:0] T67;
  wire[4:0] T68;
  wire[19:0] T69;
  wire[3:0] T70;
  wire[9:0] T71;
  wire[19:0] T72;
  wire T73;
  reg[0:0] valid_dout;
  wire T74;
  wire T75;
  reg [0:0] valid [1023:0];
  wire[9:0] T76;
  wire T77;
  wire[9:0] T78;
  wire T79;
  wire[9:0] T80;
  wire[5:0] T81;
  wire T82;
  wire[9:0] T83;
  wire[5:0] T84;
  wire T85;
  wire[9:0] T86;
  wire[5:0] T87;
  wire T88;
  wire[9:0] T89;
  wire[5:0] T90;
  wire T91;
  wire[9:0] T92;
  wire[5:0] T93;
  wire T94;
  wire[9:0] T95;
  wire[5:0] T96;
  wire T97;
  wire[9:0] T98;
  wire[4:0] T99;
  wire T100;
  wire[9:0] T101;
  wire[4:0] T102;
  wire T103;
  wire[9:0] T104;
  wire[1:0] T105;
  wire[1:0] T106;
  wire[31:0] T107;
  wire[31:0] T108;
  wire[31:0] T109;
  reg[31:0] data_in_reg;
  wire[31:0] T110;
  wire T111;
  wire T112;
  wire T113;
  wire T114;
  wire T115;
  wire[19:0] T116;
  wire T117;
  wire T118;
  wire[31:0] T119;
  wire[31:0] T120;
  wire[31:0] T121;
  reg[31:0] data_dout;
  wire T122;
  wire[31:0] T123;
  wire[31:0] T124;
  reg [31:0] data [1023:0];
  wire[31:0] T125;
  wire[31:0] T126;
  reg[31:0] mem_data_in_reg;
  wire[31:0] T127;
  wire[31:0] T128;
  wire[31:0] T129;
  wire[31:0] T130;
  wire[31:0] T131;
  wire[9:0] T132;
  wire T133;
  wire T134;

  assign io_stall = T0;
  assign T0 = state == 2'h1/* 1*/;
  assign T1 = T11 || T2;
  assign T2 = T0 && T3;
  assign T3 = burst_count == T4;
  assign T4 = {4'h0/* 0*/, 1'h0/* 0*/};
  assign T5 = T0 || T2;
  assign T6 = T2 ? T10 : T7;
  assign T7 = 1'h1/* 1*/ ? T8 : burst_count;
  assign T8 = burst_count - T9;
  assign T9 = {4'h0/* 0*/, 1'h1/* 1*/};
  assign T10 = {2'h0/* 0*/, 3'h5/* 5*/};
  assign T11 = T14 && T12;
  assign T12 = rd_reg == 1'h1/* 1*/;
  assign T13 = 1'h1/* 1*/ ? io_rd : rd_reg;
  assign T14 = T73 || T15;
  assign T15 = T72 != tag_dout;
  assign T16 = T18 || T17;
  assign T17 = io_wr == 1'h1/* 1*/;
  assign T18 = io_rd == 1'h1/* 1*/;
  assign T19 = 1'h1/* 1*/ ? T20 : tag_dout;
  assign T20 = tag[index_number];
  assign T22 = T23;
  assign T23 = address_reg[5'h1f/* 31*/:4'hc/* 12*/];
  assign T24 = 1'h1/* 1*/ ? io_address : address_reg;
  assign T25 = T0 ? T27 : T26;
  assign T26 = 1'h1/* 1*/ ? index_number : index_number_reg;
  assign index_number = io_address[4'hb/* 11*/:2'h2/* 2*/];
  assign T27 = T30 + T28;
  assign T28 = {9'h0/* 0*/, T29};
  assign T29 = 1'h1/* 1*/;
  assign T30 = index_number_reg;
  assign T32 = T33;
  assign T33 = address_reg[5'h1f/* 31*/:4'hc/* 12*/];
  assign T34 = T14 && T35;
  assign T35 = wr_reg == 1'h1/* 1*/;
  assign T36 = 1'h1/* 1*/ ? io_wr : wr_reg;
  assign T39 = {16'h0/* 0*/, T40};
  assign T40 = 4'ha/* 10*/;
  assign T41 = {4'h0/* 0*/, 6'h2a/* 42*/};
  assign T42 = init == 1'h1/* 1*/;
  assign T43 = 1'h1/* 1*/ ? 1'h0/* 0*/ : init;
  assign T45 = {16'h0/* 0*/, T46};
  assign T46 = 4'ha/* 10*/;
  assign T47 = {4'h0/* 0*/, 6'h29/* 41*/};
  assign T49 = {16'h0/* 0*/, T50};
  assign T50 = 4'ha/* 10*/;
  assign T51 = {4'h0/* 0*/, 6'h28/* 40*/};
  assign T53 = {16'h0/* 0*/, T54};
  assign T54 = 4'ha/* 10*/;
  assign T55 = {4'h0/* 0*/, 6'h27/* 39*/};
  assign T57 = {16'h0/* 0*/, T58};
  assign T58 = 4'ha/* 10*/;
  assign T59 = {4'h0/* 0*/, 6'h26/* 38*/};
  assign T61 = {16'h0/* 0*/, T62};
  assign T62 = 4'ha/* 10*/;
  assign T63 = {4'h0/* 0*/, 6'h25/* 37*/};
  assign T65 = {16'h0/* 0*/, T66};
  assign T66 = 4'ha/* 10*/;
  assign T67 = {5'h0/* 0*/, 5'h1b/* 27*/};
  assign T69 = {16'h0/* 0*/, T70};
  assign T70 = 4'ha/* 10*/;
  assign T71 = {5'h0/* 0*/, 5'h19/* 25*/};
  assign T72 = address_reg[5'h1f/* 31*/:4'hc/* 12*/];
  assign T73 = valid_dout != 1'h1/* 1*/;
  assign T74 = 1'h1/* 1*/ ? T75 : valid_dout;
  assign T75 = valid[index_number];
  assign T77 = 1'h1/* 1*/;
  assign T79 = 1'h1/* 1*/;
  assign T82 = 1'h0/* 0*/;
  assign T83 = {4'h0/* 0*/, 6'h2a/* 42*/};
  assign T85 = 1'h0/* 0*/;
  assign T86 = {4'h0/* 0*/, 6'h29/* 41*/};
  assign T88 = 1'h0/* 0*/;
  assign T89 = {4'h0/* 0*/, 6'h28/* 40*/};
  assign T91 = 1'h0/* 0*/;
  assign T92 = {4'h0/* 0*/, 6'h27/* 39*/};
  assign T94 = 1'h0/* 0*/;
  assign T95 = {4'h0/* 0*/, 6'h26/* 38*/};
  assign T97 = 1'h0/* 0*/;
  assign T98 = {4'h0/* 0*/, 6'h25/* 37*/};
  assign T100 = 1'h0/* 0*/;
  assign T101 = {5'h0/* 0*/, 5'h1b/* 27*/};
  assign T103 = 1'h0/* 0*/;
  assign T104 = {5'h0/* 0*/, 5'h19/* 25*/};
  assign T105 = T2 ? 2'h2/* 2*/ : T106;
  assign T106 = 1'h1/* 1*/ ? 2'h1/* 1*/ : state;
  assign io_mem_data_out = T107;
  assign T107 = T111 ? data_in_reg : T108;
  assign T108 = T34 ? data_in_reg : T109;
  assign T109 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign T110 = 1'h1/* 1*/ ? io_data_in : data_in_reg;
  assign T111 = T113 && T112;
  assign T112 = wr_reg == 1'h1/* 1*/;
  assign T113 = T118 && T114;
  assign T114 = T117 && T115;
  assign T115 = T116 == tag_dout;
  assign T116 = address_reg[5'h1f/* 31*/:4'hc/* 12*/];
  assign T117 = valid_dout == 1'h1/* 1*/;
  assign T118 = ! T14;
  assign io_data_out = T119;
  assign T119 = T0 ? mem_data_in_reg : T120;
  assign T120 = T133 ? data_dout : T121;
  assign T121 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign T122 = io_rd == 1'h1/* 1*/;
  assign T123 = 1'h1/* 1*/ ? T124 : data_dout;
  assign T124 = data[index_number];
  assign T126 = mem_data_in_reg;
  assign T127 = 1'h1/* 1*/ ? io_mem_data_in : mem_data_in_reg;
  assign T129 = data_in_reg;
  assign T131 = data_in_reg;
  assign T133 = T113 && T134;
  assign T134 = rd_reg == 1'h1/* 1*/;

  always @(posedge clk) begin
    if(reset) begin
      state <= 2'h0/* 0*/;
    end else if(T1) begin
      state <= T105;
    end
    if(reset) begin
      burst_count <= 5'h5/* 5*/;
    end else if(T5) begin
      burst_count <= T6;
    end
    rd_reg <= T13;
    if(T16) begin
      tag_dout <= T19;
    end
    if (T0)
      tag[index_number_reg] <= T22;
    address_reg <= T24;
    index_number_reg <= T25;
    if (T34)
      tag[index_number_reg] <= T32;
    wr_reg <= T36;
    if (T42)
      tag[T41] <= T39;
    if(reset) begin
      init <= 1'h1/* 1*/;
    end else if(T42) begin
      init <= T43;
    end
    if (T42)
      tag[T47] <= T45;
    if (T42)
      tag[T51] <= T49;
    if (T42)
      tag[T55] <= T53;
    if (T42)
      tag[T59] <= T57;
    if (T42)
      tag[T63] <= T61;
    if (T42)
      tag[T67] <= T65;
    if (T42)
      tag[T71] <= T69;
    if(T16) begin
      valid_dout <= T74;
    end
    if (T0)
      valid[index_number_reg] <= T77;
    if (T34)
      valid[index_number_reg] <= T79;
    if (T42)
      valid[T83] <= T82;
    if (T42)
      valid[T86] <= T85;
    if (T42)
      valid[T89] <= T88;
    if (T42)
      valid[T92] <= T91;
    if (T42)
      valid[T95] <= T94;
    if (T42)
      valid[T98] <= T97;
    if (T42)
      valid[T101] <= T100;
    if (T42)
      valid[T104] <= T103;
    data_in_reg <= T110;
    if(T122) begin
      data_dout <= T123;
    end
    if (T0)
      data[index_number_reg] <= T126;
    mem_data_in_reg <= T127;
    if (T111)
      data[index_number_reg] <= T129;
    if (T34)
      data[index_number_reg] <= T131;
  end
endmodule

module Test_dc(input clk, input reset,
    output io_led);

  reg[31:0] address;
  wire T0;
  wire T1;
  reg[3:0] state;
  wire T2;
  wire T3;
  wire T4;
  wire T5;
  wire dc_io_stall;
  wire T6;
  wire T7;
  wire T8;
  wire T9;
  wire T10;
  wire T11;
  wire T12;
  wire T13;
  wire T14;
  wire T15;
  wire[3:0] T16;
  wire[3:0] T17;
  wire[3:0] T18;
  wire[3:0] T19;
  wire[3:0] T20;
  wire[3:0] T21;
  wire[3:0] T22;
  wire T23;
  wire T24;
  wire T25;
  wire T26;
  wire[31:0] T27;
  wire[31:0] T28;
  wire[31:0] T29;
  wire[31:0] T30;
  wire[31:0] T31;
  wire[31:0] T32;
  wire[31:0] T33;
  wire[31:0] T34;
  wire[31:0] T35;
  wire[31:0] T36;
  wire[31:0] T37;
  wire[31:0] T38;
  wire[31:0] T39;
  reg [31:0] mem [149999:0];
  wire[31:0] T40;
  wire[17:0] T41;
  wire T42;
  wire[17:0] T43;
  wire[17:0] T44;
  wire[7:0] T45;
  wire[31:0] T46;
  wire[7:0] T47;
  wire[17:0] T48;
  wire T49;
  reg[0:0] init;
  wire T50;
  wire[7:0] T51;
  wire[31:0] T52;
  wire[7:0] T53;
  wire[17:0] T54;
  wire[7:0] T55;
  wire[31:0] T56;
  wire[7:0] T57;
  wire[17:0] T58;
  wire[7:0] T59;
  wire[31:0] T60;
  wire[7:0] T61;
  wire[17:0] T62;
  wire[7:0] T63;
  wire[31:0] T64;
  wire[7:0] T65;
  wire[17:0] T66;
  wire[7:0] T67;
  wire[31:0] T68;
  wire[7:0] T69;
  wire[17:0] T70;
  wire[14:0] T71;
  wire[31:0] T72;
  wire[7:0] T73;
  wire[17:0] T74;
  wire[6:0] T75;
  wire[31:0] T76;
  wire[6:0] T77;
  wire[17:0] T78;
  wire[17:0] T79;
  reg[31:0] data_in;
  wire T80;
  wire T81;
  wire[31:0] T82;
  wire[31:0] T83;
  wire[31:0] T84;
  wire[31:0] T85;
  wire[31:0] T86;
  wire[31:0] T87;
  reg[0:0] wr;
  wire T88;
  wire T89;
  wire T90;
  wire T91;
  wire T92;
  wire T93;
  wire T94;
  wire T95;
  wire T96;
  reg[0:0] rd;
  wire T97;
  wire T98;
  wire T99;
  wire T100;
  wire T101;
  wire T102;
  wire T103;

  assign T0 = T23 || T1;
  assign T1 = state == 4'h8/* 8*/;
  assign T2 = T3 || T1;
  assign T3 = T7 || T4;
  assign T4 = T6 && T5;
  assign T5 = dc_io_stall == 1'h0/* 0*/;
  assign T6 = state == 4'h2/* 2*/;
  assign T7 = T9 || T8;
  assign T8 = state == 4'h7/* 7*/;
  assign T9 = T11 || T10;
  assign T10 = state == 4'h3/* 3*/;
  assign T11 = T13 || T12;
  assign T12 = state == 4'h5/* 5*/;
  assign T13 = T15 || T14;
  assign T14 = state == 4'h4/* 4*/;
  assign T15 = state == 4'h0/* 0*/;
  assign T16 = T1 ? 4'h6/* 6*/ : T17;
  assign T17 = T4 ? 4'h8/* 8*/ : T18;
  assign T18 = T8 ? 4'h2/* 2*/ : T19;
  assign T19 = T10 ? 4'h7/* 7*/ : T20;
  assign T20 = T12 ? 4'h3/* 3*/ : T21;
  assign T21 = T14 ? 4'h5/* 5*/ : T22;
  assign T22 = 1'h1/* 1*/ ? 4'h4/* 4*/ : state;
  assign T23 = T24 || T6;
  assign T24 = T25 || T10;
  assign T25 = T26 || T12;
  assign T26 = T15 || T14;
  assign T27 = T1 ? T38 : T28;
  assign T28 = T6 ? T37 : T29;
  assign T29 = T10 ? T36 : T30;
  assign T30 = T12 ? T35 : T31;
  assign T31 = T14 ? T34 : T32;
  assign T32 = 1'h1/* 1*/ ? T33 : address;
  assign T33 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign T34 = {25'h0/* 0*/, 7'h64/* 100*/};
  assign T35 = {25'h0/* 0*/, 7'h6e/* 110*/};
  assign T36 = {25'h0/* 0*/, 7'h64/* 100*/};
  assign T37 = {24'h0/* 0*/, 8'h96/* 150*/};
  assign T38 = {24'h0/* 0*/, 8'ha2/* 162*/};
  assign T39 = mem[T79];
  assign T41 = address[5'h11/* 17*/:1'h0/* 0*/];
  assign T42 = T43 < 18'h249f0/* 150000*/;
  assign T43 = T44;
  assign T44 = address[5'h11/* 17*/:1'h0/* 0*/];
  assign T46 = {24'h0/* 0*/, T47};
  assign T47 = 8'hae/* 174*/;
  assign T48 = {10'h0/* 0*/, 8'hae/* 174*/};
  assign T49 = init == 1'h1/* 1*/;
  assign T50 = 1'h1/* 1*/ ? 1'h0/* 0*/ : init;
  assign T52 = {24'h0/* 0*/, T53};
  assign T53 = 8'haa/* 170*/;
  assign T54 = {10'h0/* 0*/, 8'haa/* 170*/};
  assign T56 = {24'h0/* 0*/, T57};
  assign T57 = 8'ha6/* 166*/;
  assign T58 = {10'h0/* 0*/, 8'ha6/* 166*/};
  assign T60 = {24'h0/* 0*/, T61};
  assign T61 = 8'ha2/* 162*/;
  assign T62 = {10'h0/* 0*/, 8'ha2/* 162*/};
  assign T64 = {24'h0/* 0*/, T65};
  assign T65 = 8'h9e/* 158*/;
  assign T66 = {10'h0/* 0*/, 8'h9e/* 158*/};
  assign T68 = {24'h0/* 0*/, T69};
  assign T69 = 8'h9a/* 154*/;
  assign T70 = {10'h0/* 0*/, 8'h9a/* 154*/};
  assign T72 = {24'h0/* 0*/, T73};
  assign T73 = 8'h82/* 130*/;
  assign T74 = {3'h0/* 0*/, 15'h4064/* 16484*/};
  assign T76 = {25'h0/* 0*/, T77};
  assign T77 = 7'h78/* 120*/;
  assign T78 = {11'h0/* 0*/, 7'h6e/* 110*/};
  assign T79 = address[5'h11/* 17*/:1'h0/* 0*/];
  assign T80 = T81 || T12;
  assign T81 = T15 || T14;
  assign T82 = T12 ? T87 : T83;
  assign T83 = T14 ? T86 : T84;
  assign T84 = 1'h1/* 1*/ ? T85 : data_in;
  assign T85 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign T86 = {25'h0/* 0*/, 7'h64/* 100*/};
  assign T87 = {25'h0/* 0*/, 7'h6e/* 110*/};
  assign T88 = T89 || T8;
  assign T89 = T90 || T10;
  assign T90 = T91 || T12;
  assign T91 = T15 || T14;
  assign T92 = T8 ? 1'h0/* 0*/ : T93;
  assign T93 = T10 ? 1'h0/* 0*/ : T94;
  assign T94 = T12 ? 1'h1/* 1*/ : T95;
  assign T95 = T14 ? 1'h1/* 1*/ : T96;
  assign T96 = 1'h1/* 1*/ ? 1'h0/* 0*/ : wr;
  assign T97 = T98 || T1;
  assign T98 = T99 || T6;
  assign T99 = T10 || T8;
  assign T100 = T1 ? 1'h1/* 1*/ : T101;
  assign T101 = T6 ? 1'h1/* 1*/ : T102;
  assign T102 = T8 ? 1'h0/* 0*/ : T103;
  assign T103 = 1'h1/* 1*/ ? 1'h1/* 1*/ : rd;
  assign io_led = 1'h1/* 1*/;
  DC_1_way dc(.clk(clk), .reset(reset),
       .io_rd( rd ),
       .io_wr( wr ),
       .io_data_in( data_in ),
       .io_data_out(  ),
       .io_mem_data_in( T39 ),
       .io_mem_data_out(  ),
       .io_address( address ),
       .io_stall( dc_io_stall ));

  always @(posedge clk) begin
    if(reset) begin
      address <= 32'h0/* 0*/;
    end else if(T0) begin
      address <= T27;
    end
    if(reset) begin
      state <= 4'h0/* 0*/;
    end else if(T2) begin
      state <= T16;
    end
    if (T49)
      mem[T48] <= T46;
    if(reset) begin
      init <= 1'h1/* 1*/;
    end else if(T49) begin
      init <= T50;
    end
    if (T49)
      mem[T54] <= T52;
    if (T49)
      mem[T58] <= T56;
    if (T49)
      mem[T62] <= T60;
    if (T49)
      mem[T66] <= T64;
    if (T49)
      mem[T70] <= T68;
    if (T49)
      mem[T74] <= T72;
    if (T49)
      mem[T78] <= T76;
    if(reset) begin
      data_in <= 32'h0/* 0*/;
    end else if(T80) begin
      data_in <= T82;
    end
    if(reset) begin
      wr <= 1'h0/* 0*/;
    end else if(T88) begin
      wr <= T92;
    end
    if(reset) begin
      rd <= 1'h0/* 0*/;
    end else if(T97) begin
      rd <= T100;
    end
  end
endmodule

