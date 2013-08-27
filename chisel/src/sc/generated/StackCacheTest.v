module MainMemory(input clk, input reset,
    input [2:0] io_mmInOut_M_Cmd,
    input [31:0] io_mmInOut_M_Addr,
    input [31:0] io_mmInOut_M_Data,
    input  io_mmInOut_M_DataValid,
    input [3:0] io_mmInOut_M_DataByteEn,
    output[1:0] io_mmInOut_S_Resp,
    output[31:0] io_mmInOut_S_Data,
    output io_mmInOut_S_DataAccept);

  wire T0;
  wire T1;
  wire T2;
  wire T3;
  wire T4;
  wire T5;
  wire T6;
  reg[2:0] mem_delay_cnt;
  wire T7;
  wire T8;
  wire T9;
  reg[1:0] burst_count;
  wire T10;
  wire T11;
  wire T12;
  reg[2:0] state;
  wire T13;
  wire T14;
  wire T15;
  wire T16;
  wire T17;
  wire T18;
  wire T19;
  wire T20;
  wire T21;
  wire T22;
  wire T23;
  wire T24;
  wire T25;
  reg[2:0] cmd;
  wire[2:0] T26;
  wire T27;
  wire T28;
  wire T29;
  wire[2:0] T30;
  wire[2:0] T31;
  wire[2:0] T32;
  wire[2:0] T33;
  wire[2:0] T34;
  wire[2:0] T35;
  wire T36;
  wire[1:0] T37;
  wire[1:0] T38;
  wire[1:0] T39;
  wire[1:0] T40;
  wire[1:0] T41;
  wire[1:0] T42;
  wire[1:0] T43;
  wire[1:0] T44;
  wire[1:0] T45;
  wire[1:0] T46;
  wire T47;
  wire T48;
  wire T49;
  reg[0:0] mem_delay;
  wire T50;
  wire T51;
  wire T52;
  wire T53;
  wire T54;
  wire T55;
  wire T56;
  wire[2:0] T57;
  wire[2:0] T58;
  wire[2:0] T59;
  wire[2:0] T60;
  wire[2:0] T61;
  wire[2:0] T62;
  wire[2:0] T63;
  wire[2:0] T64;
  wire[2:0] T65;
  wire T66;
  wire[31:0] T67;
  wire[31:0] T68;
  wire[31:0] T69;
  wire[31:0] T70;
  reg [31:0] mem [511:0];
  wire[31:0] T71;
  wire[8:0] T72;
  reg[31:0] rd_addr;
  wire T73;
  wire[31:0] T74;
  wire[31:0] T75;
  wire[31:0] T76;
  wire[31:0] T77;
  wire[31:0] T78;
  wire[31:0] T79;
  wire[8:0] T80;
  wire[9:0] T81;
  wire[31:0] T82;
  wire[9:0] T83;
  wire T84;
  reg[0:0] init;
  wire T85;
  wire[8:0] T86;
  wire[31:0] T87;
  wire[8:0] T88;
  wire[8:0] T89;
  wire[31:0] T90;
  wire[8:0] T91;
  wire[8:0] T92;
  wire[31:0] T93;
  wire[8:0] T94;
  wire[8:0] T95;
  wire[31:0] T96;
  wire[8:0] T97;
  wire[8:0] T98;
  wire[31:0] T99;
  wire[8:0] T100;
  wire[8:0] T101;
  wire[31:0] T102;
  wire[8:0] T103;
  wire[8:0] T104;
  wire[31:0] T105;
  wire[8:0] T106;
  wire[8:0] T107;
  wire[31:0] T108;
  wire[8:0] T109;
  wire[8:0] T110;
  wire[31:0] T111;
  wire[8:0] T112;
  wire[8:0] T113;
  wire[31:0] T114;
  wire[8:0] T115;
  wire[8:0] T116;
  wire[31:0] T117;
  wire[8:0] T118;
  wire[8:0] T119;
  wire[31:0] T120;
  wire[8:0] T121;
  wire[1:0] T122;
  wire[1:0] T123;
  wire[1:0] T124;
  wire[1:0] T125;
  wire T126;

  assign io_mmInOut_S_DataAccept = T0;
  assign T0 = T8 ? 1'h0/* 0*/ : T1;
  assign T1 = T12 ? 1'h0/* 0*/ : T2;
  assign T2 = T15 ? 1'h0/* 0*/ : T3;
  assign T3 = T17 ? 1'h1/* 1*/ : T4;
  assign T4 = T19 ? 1'h0/* 0*/ : T5;
  assign T5 = T66 && T6;
  assign T6 = mem_delay_cnt == 3'h7/* 7*/;
  assign T7 = T47 || T8;
  assign T8 = T12 && T9;
  assign T9 = burst_count == 2'h3/* 3*/;
  assign T10 = T11 || T8;
  assign T11 = T36 || T12;
  assign T12 = state == 3'h2/* 2*/;
  assign T13 = T14 || T8;
  assign T14 = T18 || T15;
  assign T15 = T17 && T16;
  assign T16 = burst_count == 2'h3/* 3*/;
  assign T17 = state == 3'h1/* 1*/;
  assign T18 = T22 || T19;
  assign T19 = T21 && T20;
  assign T20 = mem_delay_cnt == 3'h7/* 7*/;
  assign T21 = state == 3'h4/* 4*/;
  assign T22 = T23 || T5;
  assign T23 = T28 || T24;
  assign T24 = T27 && T25;
  assign T25 = cmd == 3'b010/* 0*/;
  assign T26 = 1'h1/* 1*/ ? io_mmInOut_M_Cmd : cmd;
  assign T27 = state == 3'h0/* 0*/;
  assign T28 = T27 && T29;
  assign T29 = cmd == 3'b101/* 0*/;
  assign T30 = T8 ? 3'h0/* 0*/ : T31;
  assign T31 = T15 ? 3'h0/* 0*/ : T32;
  assign T32 = T19 ? 3'h2/* 2*/ : T33;
  assign T33 = T5 ? 3'h1/* 1*/ : T34;
  assign T34 = T24 ? 3'h4/* 4*/ : T35;
  assign T35 = 1'h1/* 1*/ ? 3'h3/* 3*/ : state;
  assign T36 = T17 || T15;
  assign T37 = T8 ? T46 : T38;
  assign T38 = T12 ? T44 : T39;
  assign T39 = T15 ? T43 : T40;
  assign T40 = 1'h1/* 1*/ ? T41 : burst_count;
  assign T41 = burst_count + T42;
  assign T42 = {1'h0/* 0*/, 1'h1/* 1*/};
  assign T43 = {1'h0/* 0*/, 1'h0/* 0*/};
  assign T44 = burst_count + T45;
  assign T45 = {1'h0/* 0*/, 1'h1/* 1*/};
  assign T46 = {1'h0/* 0*/, 1'h0/* 0*/};
  assign T47 = T48 || T15;
  assign T48 = T49 || T27;
  assign T49 = mem_delay == 1'h1/* 1*/;
  assign T50 = T51 || T8;
  assign T51 = T52 || T15;
  assign T52 = T28 || T24;
  assign T53 = T8 ? 1'h1/* 1*/ : T54;
  assign T54 = T15 ? 1'h1/* 1*/ : T55;
  assign T55 = T24 ? 1'h1/* 1*/ : T56;
  assign T56 = 1'h1/* 1*/ ? 1'h1/* 1*/ : mem_delay;
  assign T57 = T8 ? T65 : T58;
  assign T58 = T15 ? T64 : T59;
  assign T59 = T27 ? T63 : T60;
  assign T60 = 1'h1/* 1*/ ? T61 : mem_delay_cnt;
  assign T61 = mem_delay_cnt + T62;
  assign T62 = {2'h0/* 0*/, 1'h1/* 1*/};
  assign T63 = {2'h0/* 0*/, 1'h0/* 0*/};
  assign T64 = {2'h0/* 0*/, 1'h0/* 0*/};
  assign T65 = {2'h0/* 0*/, 1'h0/* 0*/};
  assign T66 = state == 3'h3/* 3*/;
  assign io_mmInOut_S_Data = T67;
  assign T67 = T12 ? T120 : T68;
  assign T68 = T21 ? T70 : T69;
  assign T69 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign T70 = mem[T119];
  assign T72 = rd_addr[4'h8/* 8*/:1'h0/* 0*/];
  assign T73 = T21 || T12;
  assign T74 = T12 ? T76 : T75;
  assign T75 = 1'h1/* 1*/ ? io_mmInOut_M_Addr : rd_addr;
  assign T76 = T78 + T77;
  assign T77 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign T78 = rd_addr;
  assign T80 = io_mmInOut_M_Addr[4'h8/* 8*/:1'h0/* 0*/];
  assign T82 = {22'h0/* 0*/, T83};
  assign T83 = 10'h200/* 512*/;
  assign T84 = init == 1'h1/* 1*/;
  assign T85 = 1'h1/* 1*/ ? 1'h0/* 0*/ : init;
  assign T87 = {23'h0/* 0*/, T88};
  assign T88 = 9'h1ff/* 511*/;
  assign T90 = {23'h0/* 0*/, T91};
  assign T91 = 9'h1fe/* 510*/;
  assign T93 = {23'h0/* 0*/, T94};
  assign T94 = 9'h1fd/* 509*/;
  assign T96 = {23'h0/* 0*/, T97};
  assign T97 = 9'h1fc/* 508*/;
  assign T99 = {23'h0/* 0*/, T100};
  assign T100 = 9'h1fb/* 507*/;
  assign T102 = {23'h0/* 0*/, T103};
  assign T103 = 9'h1fa/* 506*/;
  assign T105 = {23'h0/* 0*/, T106};
  assign T106 = 9'h1f9/* 505*/;
  assign T108 = {23'h0/* 0*/, T109};
  assign T109 = 9'h1f8/* 504*/;
  assign T111 = {23'h0/* 0*/, T112};
  assign T112 = 9'h1f7/* 503*/;
  assign T114 = {23'h0/* 0*/, T115};
  assign T115 = 9'h1f6/* 502*/;
  assign T117 = {23'h0/* 0*/, T118};
  assign T118 = 9'h1f5/* 501*/;
  assign T119 = io_mmInOut_M_Addr[4'h8/* 8*/:1'h0/* 0*/];
  assign T120 = mem[T121];
  assign T121 = rd_addr[4'h8/* 8*/:1'h0/* 0*/];
  assign io_mmInOut_S_Resp = T122;
  assign T122 = T8 ? 2'b01/* 0*/ : T123;
  assign T123 = T12 ? 2'b01/* 0*/ : T124;
  assign T124 = T15 ? 2'b01/* 0*/ : T125;
  assign T125 = {1'h0/* 0*/, T126};
  assign T126 = T19;

  always @(posedge clk) begin
    if(reset) begin
      mem_delay_cnt <= 3'h0/* 0*/;
    end else if(T7) begin
      mem_delay_cnt <= T57;
    end
    if(reset) begin
      burst_count <= 2'h0/* 0*/;
    end else if(T10) begin
      burst_count <= T37;
    end
    if(reset) begin
      state <= 3'h0/* 0*/;
    end else if(T13) begin
      state <= T30;
    end
    cmd <= reset ? 3'h0/* 0*/ : T26;
    if(reset) begin
      mem_delay <= 1'h0/* 0*/;
    end else if(T50) begin
      mem_delay <= T53;
    end
    if(reset) begin
      rd_addr <= 32'h0/* 0*/;
    end else if(T73) begin
      rd_addr <= T74;
    end
    if (T84)
      mem[9'h0/* 0*/] <= T82;
    if(reset) begin
      init <= 1'h1/* 1*/;
    end else if(T84) begin
      init <= T85;
    end
    if (T84)
      mem[9'h1ff/* 511*/] <= T87;
    if (T84)
      mem[9'h1fe/* 510*/] <= T90;
    if (T84)
      mem[9'h1fd/* 509*/] <= T93;
    if (T84)
      mem[9'h1fc/* 508*/] <= T96;
    if (T84)
      mem[9'h1fb/* 507*/] <= T99;
    if (T84)
      mem[9'h1fa/* 506*/] <= T102;
    if (T84)
      mem[9'h1f9/* 505*/] <= T105;
    if (T84)
      mem[9'h1f8/* 504*/] <= T108;
    if (T84)
      mem[9'h1f7/* 503*/] <= T111;
    if (T84)
      mem[9'h1f6/* 502*/] <= T114;
    if (T84)
      mem[9'h1f5/* 501*/] <= T117;
  end
endmodule

module StackCache(input clk, input reset,
    input [2:0] io_scCpuInOut_M_Cmd,
    input [31:0] io_scCpuInOut_M_Addr,
    input [31:0] io_scCpuInOut_M_Data,
    input [3:0] io_scCpuInOut_M_ByteEn,
    output[1:0] io_scCpuInOut_S_Resp,
    output[31:0] io_scCpuInOut_S_Data,
    output[2:0] io_scMemInOut_M_Cmd,
    output[31:0] io_scMemInOut_M_Addr,
    output[31:0] io_scMemInOut_M_Data,
    output io_scMemInOut_M_DataValid,
    output[3:0] io_scMemInOut_M_DataByteEn,
    input [1:0] io_scMemInOut_S_Resp,
    input [31:0] io_scMemInOut_S_Data,
    input  io_scMemInOut_S_DataAccept,
    input  io_spill,
    input  io_fill,
    input  io_free,
    input [31:0] io_sc_top,
    output[31:0] io_m_top,
    input [7:0] io_n_spill,
    input [7:0] io_n_fill,
    output io_stall);

  wire T0;
  wire T1;
  wire T2;
  wire T3;
  wire T4;
  wire T5;
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
  reg[2:0] state;
  wire T16;
  wire T17;
  wire T18;
  wire T19;
  wire T20;
  wire T21;
  wire T22;
  wire T23;
  wire T24;
  wire T25;
  wire T26;
  wire T27;
  wire T28;
  wire T29;
  wire T30;
  wire T31;
  wire T32;
  wire T33;
  wire T34;
  wire T35;
  wire T36;
  wire T37;
  wire T38;
  wire T39;
  wire T40;
  wire T41;
  wire T42;
  wire signed  T43;
  wire[2:0] T44;
  wire signed [2:0] T45;
  wire[2:0] T46;
  reg signed [2:0] burst_count;
  wire T47;
  wire T48;
  wire T49;
  wire[2:0] T50;
  wire[2:0] T51;
  wire signed [2:0] T52;
  wire[2:0] T53;
  wire[2:0] T54;
  wire T55;
  wire T56;
  wire T57;
  wire T58;
  wire signed  T59;
  wire[7:0] T60;
  wire signed [7:0] T61;
  wire[7:0] T62;
  reg signed [7:0] n_fill;
  wire T63;
  wire T64;
  wire T65;
  wire T66;
  wire T67;
  wire T68;
  wire T69;
  wire[8:0] T70;
  wire[8:0] T71;
  wire[8:0] T72;
  wire[7:0] T73;
  wire[7:0] T74;
  wire signed [7:0] T75;
  wire[7:0] T76;
  wire signed [1:0] T77;
  wire signed [7:0] T78;
  wire[7:0] T79;
  wire signed [1:0] T80;
  wire[8:0] T81;
  wire[8:0] T82;
  wire[8:0] T83;
  wire[8:0] T84;
  wire T85;
  wire T86;
  wire T87;
  wire signed  T88;
  wire[7:0] T89;
  wire signed [7:0] T90;
  wire[7:0] T91;
  reg signed [7:0] n_spill;
  wire T92;
  wire T93;
  wire T94;
  wire T95;
  wire T96;
  wire T97;
  wire T98;
  wire T99;
  wire T100;
  wire T101;
  wire T102;
  wire T103;
  wire T104;
  wire[8:0] T105;
  wire[8:0] T106;
  wire[8:0] T107;
  wire[8:0] T108;
  wire[8:0] T109;
  wire T110;
  wire T111;
  wire[8:0] T112;
  wire[8:0] T113;
  wire[8:0] T114;
  wire signed [7:0] T115;
  wire[7:0] T116;
  wire signed [1:0] T117;
  wire T118;
  wire T119;
  wire[8:0] T120;
  wire signed [7:0] T121;
  wire[7:0] T122;
  wire signed [1:0] T123;
  wire T124;
  wire T125;
  wire[8:0] T126;
  wire[8:0] T127;
  wire T128;
  wire T129;
  wire T130;
  wire T131;
  wire T132;
  wire T133;
  wire T134;
  wire T135;
  wire T136;
  wire T137;
  wire T138;
  wire T139;
  wire T140;
  wire T141;
  wire T142;
  wire[2:0] T143;
  wire[2:0] T144;
  wire[2:0] T145;
  wire[2:0] T146;
  wire[2:0] T147;
  wire[2:0] T148;
  wire[2:0] T149;
  wire[2:0] T150;
  wire[2:0] T151;
  wire[2:0] T152;
  wire[2:0] T153;
  wire[2:0] T154;
  wire[2:0] T155;
  wire[2:0] T156;
  reg[31:0] m_top;
  wire T157;
  wire T158;
  wire T159;
  wire T160;
  wire T161;
  wire T162;
  wire[31:0] T163;
  wire[31:0] T164;
  wire[31:0] T165;
  wire[31:0] T166;
  wire[31:0] T167;
  wire[31:0] T168;
  wire[31:0] T169;
  wire[31:0] T170;
  wire[31:0] T171;
  wire[31:0] T172;
  wire[31:0] T173;
  wire[31:0] T174;
  wire[31:0] T175;
  wire[3:0] T176;
  wire[3:0] T177;
  wire[3:0] T178;
  wire[3:0] T179;
  wire T180;
  wire signed  T181;
  wire[7:0] T182;
  wire signed [1:0] T183;
  wire signed [7:0] T184;
  wire[7:0] T185;
  wire T186;
  wire T187;
  wire signed  T188;
  wire[7:0] T189;
  wire signed [1:0] T190;
  wire signed [7:0] T191;
  wire[7:0] T192;
  wire signed [2:0] T193;
  wire T194;
  wire T195;
  wire T196;
  wire signed  T197;
  wire[7:0] T198;
  wire signed [1:0] T199;
  wire signed [7:0] T200;
  wire[7:0] T201;
  wire signed [2:0] T202;
  wire T203;
  wire T204;
  wire T205;
  wire T206;
  wire signed  T207;
  wire[7:0] T208;
  wire signed [1:0] T209;
  wire signed [7:0] T210;
  wire[7:0] T211;
  wire signed [1:0] T212;
  wire T213;
  wire T214;
  wire T215;
  wire T216;
  wire[31:0] T217;
  wire[31:0] T218;
  wire[31:0] T219;
  wire[31:0] T220;
  wire[31:0] T221;
  wire[31:0] rdDataSpill;
  wire[7:0] T222;
  reg [7:0] sc0 [63:0];
  wire[31:0] T223;
  wire[7:0] T224;
  wire[7:0] T225;
  wire[5:0] T226;
  wire[31:0] mem_addr_masked;
  wire[31:0] T227;
  wire[31:0] T228;
  wire[31:0] T229;
  wire T230;
  wire T231;
  wire T232;
  wire[3:0] fill_en;
  wire T233;
  wire[31:0] T234;
  wire[5:0] T235;
  wire[31:0] cpu_addr_masked;
  wire[31:0] T236;
  wire[31:0] T237;
  wire[5:0] T238;
  wire[31:0] T239;
  wire[7:0] T240;
  wire[7:0] T241;
  wire[5:0] T242;
  wire T243;
  wire T244;
  wire[3:0] sc_en;
  wire T245;
  wire[5:0] T246;
  wire[7:0] T247;
  reg [7:0] sc1 [63:0];
  wire[31:0] T248;
  wire[7:0] T249;
  wire[7:0] T250;
  wire[5:0] T251;
  wire T252;
  wire T253;
  wire T254;
  wire[31:0] T255;
  wire[5:0] T256;
  wire[31:0] T257;
  wire[5:0] T258;
  wire[31:0] T259;
  wire[7:0] T260;
  wire[7:0] T261;
  wire[5:0] T262;
  wire T263;
  wire T264;
  wire[5:0] T265;
  wire[7:0] T266;
  reg [7:0] sc2 [63:0];
  wire[31:0] T267;
  wire[7:0] T268;
  wire[7:0] T269;
  wire[5:0] T270;
  wire T271;
  wire T272;
  wire T273;
  wire[31:0] T274;
  wire[5:0] T275;
  wire[31:0] T276;
  wire[5:0] T277;
  wire[31:0] T278;
  wire[7:0] T279;
  wire[7:0] T280;
  wire[5:0] T281;
  wire T282;
  wire T283;
  wire[5:0] T284;
  wire[7:0] T285;
  reg [7:0] sc3 [63:0];
  wire[31:0] T286;
  wire[7:0] T287;
  wire[7:0] T288;
  wire[5:0] T289;
  wire T290;
  wire T291;
  wire T292;
  wire[31:0] T293;
  wire[5:0] T294;
  wire[31:0] T295;
  wire[5:0] T296;
  wire[31:0] T297;
  wire[7:0] T298;
  wire[7:0] T299;
  wire[5:0] T300;
  wire T301;
  wire T302;
  wire[5:0] T303;
  wire[31:0] T304;
  wire[31:0] T305;
  wire[31:0] T306;
  wire[31:0] T307;
  wire[31:0] T308;
  wire[31:0] T309;
  wire[31:0] T310;
  reg[31:0] mem_addr_reg;
  wire[31:0] T311;
  wire[31:0] T312;
  wire[31:0] T313;
  wire[31:0] T314;
  wire[31:0] T315;
  wire[2:0] T316;
  wire[2:0] T317;
  wire[2:0] T318;
  wire[2:0] T319;
  wire[2:0] T320;
  wire[2:0] T321;
  wire[2:0] T322;
  wire[2:0] T323;
  wire[2:0] T324;
  wire[2:0] T325;
  wire[2:0] T326;
  wire[31:0] rdData;
  wire[7:0] T327;
  wire[5:0] T328;
  wire[7:0] T329;
  wire[5:0] T330;
  wire[7:0] T331;
  wire[5:0] T332;
  wire[7:0] T333;
  wire[5:0] T334;
  wire[1:0] T335;
  wire T336;
  wire T337;
  wire T338;
  wire T339;

  assign io_stall = T0;
  assign T0 = T31 ? 1'h1/* 1*/ : T1;
  assign T1 = T36 ? 1'h1/* 1*/ : T2;
  assign T2 = T22 ? 1'h0/* 0*/ : T3;
  assign T3 = T57 ? 1'h0/* 0*/ : T4;
  assign T4 = T66 ? 1'h1/* 1*/ : T5;
  assign T5 = T48 ? 1'h1/* 1*/ : T6;
  assign T6 = T86 ? 1'h0/* 0*/ : T7;
  assign T7 = T94 ? 1'h1/* 1*/ : T8;
  assign T8 = T100 ? 1'h1/* 1*/ : T9;
  assign T9 = T69 ? 1'h1/* 1*/ : T10;
  assign T10 = T104 ? 1'h1/* 1*/ : T11;
  assign T11 = T133 ? 1'h0/* 0*/ : T12;
  assign T12 = T140 ? 1'h1/* 1*/ : T13;
  assign T13 = T15 && T14;
  assign T14 = io_spill == 1'h1/* 1*/;
  assign T15 = state == 3'h0/* 0*/;
  assign T16 = T23 || T17;
  assign T17 = T22 && T18;
  assign T18 = ! T19;
  assign T19 = T21 || T20;
  assign T20 = io_spill == 1'h1/* 1*/;
  assign T21 = io_fill == 1'h1/* 1*/;
  assign T22 = state == 3'h3/* 3*/;
  assign T23 = T30 || T24;
  assign T24 = T27 && T25;
  assign T25 = ! T26;
  assign T26 = io_scMemInOut_S_DataAccept == 1'h1/* 1*/;
  assign T27 = T22 && T28;
  assign T28 = T29 && T20;
  assign T29 = ! T21;
  assign T30 = T32 || T31;
  assign T31 = T27 && T26;
  assign T32 = T37 || T33;
  assign T33 = T36 && T34;
  assign T34 = ! T35;
  assign T35 = io_scMemInOut_S_Resp == 2'b01/* 0*/;
  assign T36 = T22 && T21;
  assign T37 = T39 || T38;
  assign T38 = T36 && T35;
  assign T39 = T56 || T40;
  assign T40 = T49 && T41;
  assign T41 = ! T42;
  assign T42 = T55 && T43;
  assign T43 = $signed(T45) >= $signed(T44);
  assign T44 = {1'h0/* 0*/, 2'h0/* 0*/};
  assign T45 = $signed(burst_count) - $signed(T46);
  assign T46 = {1'h0/* 0*/, 2'h1/* 1*/};
  assign T47 = T48 || T40;
  assign T48 = T49 && T42;
  assign T49 = state == 3'h2/* 2*/;
  assign T50 = T40 ? T54 : T51;
  assign T51 = 1'h1/* 1*/ ? T52 : burst_count;
  assign T52 = $signed(burst_count) - $signed(T53);
  assign T53 = {1'h0/* 0*/, 2'h1/* 1*/};
  assign T54 = 3'h3/* 3*/;
  assign T55 = io_scMemInOut_S_Resp == 2'b01/* 0*/;
  assign T56 = T85 || T57;
  assign T57 = T48 && T58;
  assign T58 = ! T59;
  assign T59 = $signed(T61) >= $signed(T60);
  assign T60 = {6'h0/* 0*/, 2'h0/* 0*/};
  assign T61 = $signed(n_fill) - $signed(T62);
  assign T62 = {6'h0/* 0*/, 2'h1/* 1*/};
  assign T63 = T64 || T36;
  assign T64 = T65 || T38;
  assign T65 = T67 || T66;
  assign T66 = T48 && T59;
  assign T67 = T69 && T68;
  assign T68 = io_scMemInOut_S_Resp == 2'b01/* 0*/;
  assign T69 = state == 3'h5/* 5*/;
  assign T70 = T36 ? T83 : T71;
  assign T71 = T38 ? T81 : T72;
  assign T72 = {1'h0/* 0*/, T73};
  assign T73 = T66 ? T78 : T74;
  assign T74 = 1'h1/* 1*/ ? T75 : n_fill;
  assign T75 = $signed(n_fill) - $signed(T76);
  assign T76 = {6'h0/* 0*/, T77};
  assign T77 = 2'h1/* 1*/;
  assign T78 = $signed(n_fill) - $signed(T79);
  assign T79 = {6'h0/* 0*/, T80};
  assign T80 = 2'h1/* 1*/;
  assign T81 = T82;
  assign T82 = {1'h0/* 0*/, io_n_fill};
  assign T83 = T84;
  assign T84 = {1'h0/* 0*/, io_n_fill};
  assign T85 = T128 || T86;
  assign T86 = T95 && T87;
  assign T87 = ! T88;
  assign T88 = $signed(T90) >= $signed(T89);
  assign T89 = {6'h0/* 0*/, 2'h0/* 0*/};
  assign T90 = $signed(n_spill) - $signed(T91);
  assign T91 = {6'h0/* 0*/, 2'h1/* 1*/};
  assign T92 = T93 || T31;
  assign T93 = T101 || T94;
  assign T94 = T95 && T88;
  assign T95 = T100 && T96;
  assign T96 = T98 && T97;
  assign T97 = io_scMemInOut_S_DataAccept == 1'h1/* 1*/;
  assign T98 = ! T99;
  assign T99 = io_scMemInOut_S_DataAccept == 1'h0/* 0*/;
  assign T100 = state == 3'h1/* 1*/;
  assign T101 = T13 || T102;
  assign T102 = T104 && T103;
  assign T103 = io_scMemInOut_S_DataAccept == 1'h1/* 1*/;
  assign T104 = state == 3'h4/* 4*/;
  assign T105 = T31 ? T126 : T106;
  assign T106 = T94 ? T120 : T107;
  assign T107 = T102 ? T114 : T108;
  assign T108 = 1'h1/* 1*/ ? T112 : T109;
  assign T109 = {T110, n_spill};
  assign T110 = {1'h1/* 1*/{T111}};
  assign T111 = n_spill[3'h7/* 7*/:3'h7/* 7*/];
  assign T112 = T113;
  assign T113 = {1'h0/* 0*/, io_n_spill};
  assign T114 = {T118, T115};
  assign T115 = $signed(n_spill) - $signed(T116);
  assign T116 = {6'h0/* 0*/, T117};
  assign T117 = 2'h1/* 1*/;
  assign T118 = {1'h1/* 1*/{T119}};
  assign T119 = T115[3'h7/* 7*/:3'h7/* 7*/];
  assign T120 = {T124, T121};
  assign T121 = $signed(n_spill) - $signed(T122);
  assign T122 = {6'h0/* 0*/, T123};
  assign T123 = 2'h1/* 1*/;
  assign T124 = {1'h1/* 1*/{T125}};
  assign T125 = T121[3'h7/* 7*/:3'h7/* 7*/];
  assign T126 = T127;
  assign T127 = {1'h0/* 0*/, io_n_spill};
  assign T128 = T130 || T129;
  assign T129 = T100 && T99;
  assign T130 = T131 || T67;
  assign T131 = T132 || T102;
  assign T132 = T139 || T133;
  assign T133 = T15 && T134;
  assign T134 = T136 && T135;
  assign T135 = io_free == 1'h1/* 1*/;
  assign T136 = ! T137;
  assign T137 = T14 || T138;
  assign T138 = io_fill == 1'h1/* 1*/;
  assign T139 = T13 || T140;
  assign T140 = T15 && T141;
  assign T141 = T142 && T138;
  assign T142 = ! T14;
  assign T143 = T17 ? 3'h0/* 0*/ : T144;
  assign T144 = T24 ? 3'h4/* 4*/ : T145;
  assign T145 = T31 ? 3'h1/* 1*/ : T146;
  assign T146 = T33 ? 3'h5/* 5*/ : T147;
  assign T147 = T38 ? 3'h2/* 2*/ : T148;
  assign T148 = T40 ? 3'h5/* 5*/ : T149;
  assign T149 = T57 ? 3'h0/* 0*/ : T150;
  assign T150 = T86 ? 3'h0/* 0*/ : T151;
  assign T151 = T129 ? 3'h4/* 4*/ : T152;
  assign T152 = T67 ? 3'h2/* 2*/ : T153;
  assign T153 = T102 ? 3'h1/* 1*/ : T154;
  assign T154 = T133 ? 3'h3/* 3*/ : T155;
  assign T155 = T140 ? 3'h2/* 2*/ : T156;
  assign T156 = 1'h1/* 1*/ ? 3'h1/* 1*/ : state;
  assign io_m_top = m_top;
  assign T157 = T160 || T158;
  assign T158 = T22 && T159;
  assign T159 = io_sc_top > m_top;
  assign T160 = T161 || T66;
  assign T161 = T162 || T94;
  assign T162 = T102 || T67;
  assign T163 = T158 ? io_sc_top : T164;
  assign T164 = T66 ? T174 : T165;
  assign T165 = T94 ? T172 : T166;
  assign T166 = T67 ? T170 : T167;
  assign T167 = 1'h1/* 1*/ ? T168 : m_top;
  assign T168 = m_top - T169;
  assign T169 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign T170 = m_top + T171;
  assign T171 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign T172 = m_top - T173;
  assign T173 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign T174 = m_top + T175;
  assign T175 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign io_scMemInOut_M_DataByteEn = T176;
  assign T176 = T205 ? 4'b1000/* 0*/ : T177;
  assign T177 = T195 ? 4'b1100/* 0*/ : T178;
  assign T178 = T186 ? 4'b1110/* 0*/ : T179;
  assign T179 = T180 ? 4'b1111/* 0*/ : 4'b0000/* 0*/;
  assign T180 = T95 && T181;
  assign T181 = $signed(T184) >= $signed(T182);
  assign T182 = {6'h3f/* 63*/, T183};
  assign T183 = 2'h2/* 2*/;
  assign T184 = $signed(n_spill) - $signed(T185);
  assign T185 = {4'h0/* 0*/, 4'h4/* 4*/};
  assign T186 = T95 && T187;
  assign T187 = T194 && T188;
  assign T188 = $signed(T191) == $signed(T189);
  assign T189 = {6'h0/* 0*/, T190};
  assign T190 = 2'h0/* 0*/;
  assign T191 = $signed(n_spill) - $signed(T192);
  assign T192 = {5'h0/* 0*/, T193};
  assign T193 = 3'h3/* 3*/;
  assign T194 = ! T181;
  assign T195 = T95 && T196;
  assign T196 = T203 && T197;
  assign T197 = $signed(T200) == $signed(T198);
  assign T198 = {6'h0/* 0*/, T199};
  assign T199 = 2'h0/* 0*/;
  assign T200 = $signed(n_spill) - $signed(T201);
  assign T201 = {5'h0/* 0*/, T202};
  assign T202 = 3'h2/* 2*/;
  assign T203 = ! T204;
  assign T204 = T181 || T188;
  assign T205 = T95 && T206;
  assign T206 = T213 && T207;
  assign T207 = $signed(T210) == $signed(T208);
  assign T208 = {6'h0/* 0*/, T209};
  assign T209 = 2'h0/* 0*/;
  assign T210 = $signed(n_spill) - $signed(T211);
  assign T211 = {6'h0/* 0*/, T212};
  assign T212 = 2'h1/* 1*/;
  assign T213 = ! T214;
  assign T214 = T204 || T197;
  assign io_scMemInOut_M_DataValid = T215;
  assign T215 = T94 ? 1'h1/* 1*/ : T216;
  assign T216 = T69 ? 1'h0/* 0*/ : T104;
  assign io_scMemInOut_M_Data = T217;
  assign T217 = T94 ? rdDataSpill : T218;
  assign T218 = T100 ? rdDataSpill : T219;
  assign T219 = T104 ? rdDataSpill : T220;
  assign T220 = T13 ? rdDataSpill : T221;
  assign T221 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign rdDataSpill = {T285, T266, T247, T222};
  assign T222 = sc0[T246];
  assign T224 = T225;
  assign T225 = io_scMemInOut_S_Data[3'h7/* 7*/:1'h0/* 0*/];
  assign T226 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign mem_addr_masked = T228 & T227;
  assign T227 = {24'h0/* 0*/, 8'hff/* 255*/};
  assign T228 = m_top - T229;
  assign T229 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign T230 = T48 && T231;
  assign T231 = T232;
  assign T232 = fill_en[1'h0/* 0*/:1'h0/* 0*/];
  assign fill_en = T233 ? 4'b1111/* 0*/ : 4'b0000/* 0*/;
  assign T233 = state == 3'h2/* 2*/;
  assign T235 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign cpu_addr_masked = io_scCpuInOut_M_Addr & T236;
  assign T236 = {24'h0/* 0*/, 8'hff/* 255*/};
  assign T238 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T240 = T241;
  assign T241 = io_scCpuInOut_M_Data[3'h7/* 7*/:1'h0/* 0*/];
  assign T242 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T243 = T244;
  assign T244 = sc_en[1'h0/* 0*/:1'h0/* 0*/];
  assign sc_en = T245 ? io_scCpuInOut_M_ByteEn : 4'b0000/* 0*/;
  assign T245 = io_scCpuInOut_M_Cmd == 3'b001/* 0*/;
  assign T246 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T247 = sc1[T265];
  assign T249 = T250;
  assign T250 = io_scMemInOut_S_Data[4'hf/* 15*/:4'h8/* 8*/];
  assign T251 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T252 = T48 && T253;
  assign T253 = T254;
  assign T254 = fill_en[1'h1/* 1*/:1'h1/* 1*/];
  assign T256 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T258 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T260 = T261;
  assign T261 = io_scCpuInOut_M_Data[4'hf/* 15*/:4'h8/* 8*/];
  assign T262 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T263 = T264;
  assign T264 = sc_en[1'h1/* 1*/:1'h1/* 1*/];
  assign T265 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T266 = sc2[T284];
  assign T268 = T269;
  assign T269 = io_scMemInOut_S_Data[5'h17/* 23*/:5'h10/* 16*/];
  assign T270 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T271 = T48 && T272;
  assign T272 = T273;
  assign T273 = fill_en[2'h2/* 2*/:2'h2/* 2*/];
  assign T275 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T277 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T279 = T280;
  assign T280 = io_scCpuInOut_M_Data[5'h17/* 23*/:5'h10/* 16*/];
  assign T281 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T282 = T283;
  assign T283 = sc_en[2'h2/* 2*/:2'h2/* 2*/];
  assign T284 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T285 = sc3[T303];
  assign T287 = T288;
  assign T288 = io_scMemInOut_S_Data[5'h1f/* 31*/:5'h18/* 24*/];
  assign T289 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T290 = T48 && T291;
  assign T291 = T292;
  assign T292 = fill_en[2'h3/* 3*/:2'h3/* 3*/];
  assign T294 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T296 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T298 = T299;
  assign T299 = io_scCpuInOut_M_Data[5'h1f/* 31*/:5'h18/* 24*/];
  assign T300 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T301 = T302;
  assign T302 = sc_en[2'h3/* 3*/:2'h3/* 3*/];
  assign T303 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign io_scMemInOut_M_Addr = T304;
  assign T304 = T94 ? T314 : T305;
  assign T305 = T100 ? mem_addr_reg : T306;
  assign T306 = T69 ? m_top : T307;
  assign T307 = T104 ? mem_addr_reg : T308;
  assign T308 = T140 ? mem_addr_reg : T309;
  assign T309 = T13 ? mem_addr_reg : T310;
  assign T310 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign T311 = 1'h1/* 1*/ ? T312 : mem_addr_reg;
  assign T312 = m_top - T313;
  assign T313 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign T314 = m_top - T315;
  assign T315 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign io_scMemInOut_M_Cmd = T316;
  assign T316 = T38 ? 3'b010/* 0*/ : T317;
  assign T317 = T57 ? 3'b000/* 0*/ : T318;
  assign T318 = T66 ? 3'b000/* 0*/ : T319;
  assign T319 = T48 ? 3'b000/* 0*/ : T320;
  assign T320 = T86 ? 3'b000/* 0*/ : T321;
  assign T321 = T94 ? 3'b000/* 0*/ : T322;
  assign T322 = T100 ? 3'b000/* 0*/ : T323;
  assign T323 = T69 ? 3'b010/* 0*/ : T324;
  assign T324 = T104 ? 3'b101/* 0*/ : T325;
  assign T325 = T140 ? 3'b010/* 0*/ : T326;
  assign T326 = T13 ? 3'b101/* 0*/ : 3'b000/* 0*/;
  assign io_scCpuInOut_S_Data = rdData;
  assign rdData = {T333, T331, T329, T327};
  assign T327 = sc0[T328];
  assign T328 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T329 = sc1[T330];
  assign T330 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T331 = sc2[T332];
  assign T332 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T333 = sc3[T334];
  assign T334 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign io_scCpuInOut_S_Resp = T335;
  assign T335 = {1'h0/* 0*/, T336};
  assign T336 = T337;
  assign T337 = T339 || T338;
  assign T338 = io_scCpuInOut_M_Cmd == 3'b010/* 0*/;
  assign T339 = io_scCpuInOut_M_Cmd == 3'b101/* 0*/;

  always @(posedge clk) begin
    if(reset) begin
      state <= 3'h0/* 0*/;
    end else if(T16) begin
      state <= T143;
    end
    if(reset) begin
      burst_count <= 3'h3/* 3*/;
    end else if(T47) begin
      burst_count <= T50;
    end
    if(reset) begin
      n_fill <= 8'h0/* 0*/;
    end else if(T63) begin
      n_fill <= T70;
    end
    if(reset) begin
      n_spill <= 8'h0/* 0*/;
    end else if(T92) begin
      n_spill <= T105;
    end
    if(reset) begin
      m_top <= 32'h200/* 512*/;
    end else if(T157) begin
      m_top <= T163;
    end
    if (T230)
      sc0[T226] <= T224;
    if (T243)
      sc0[T242] <= T240;
    if (T252)
      sc1[T251] <= T249;
    if (T263)
      sc1[T262] <= T260;
    if (T271)
      sc2[T270] <= T268;
    if (T282)
      sc2[T281] <= T279;
    if (T290)
      sc3[T289] <= T287;
    if (T301)
      sc3[T300] <= T298;
    mem_addr_reg <= reset ? 32'h0/* 0*/ : T311;
  end
endmodule

module SC_ex(input clk, input reset,
    input [1:0] io_sc_func_type,
    input [31:0] io_imm,
    output io_spill,
    output io_fill,
    output io_free,
    input  io_stall,
    input [31:0] io_m_top,
    output[7:0] io_n_spill,
    output[7:0] io_n_fill,
    output[31:0] io_sc_top);

  wire[31:0] T0;
  wire[31:0] T1;
  reg[31:0] sc_top;
  wire T2;
  wire T3;
  wire T4;
  wire T5;
  reg[0:0] stall;
  wire T6;
  wire T7;
  wire T8;
  wire[1:0] T9;
  wire[31:0] T10;
  wire[31:0] T11;
  wire[31:0] T12;
  wire[31:0] T13;
  wire[31:0] T14;
  wire[31:0] T15;
  wire[7:0] T16;
  wire[31:0] T17;
  wire[31:0] T18;
  wire[31:0] T19;
  wire[31:0] ensure_size;
  wire[31:0] T20;
  wire T21;
  wire T22;
  wire[31:0] T23;
  wire T24;
  wire T25;
  wire[1:0] T26;
  wire[31:0] T27;
  wire T28;
  wire T29;
  wire[7:0] T30;
  wire[31:0] T31;
  wire[31:0] T32;
  wire[31:0] T33;
  wire[31:0] reserve_size;
  wire[31:0] T34;
  wire[31:0] T35;
  wire[31:0] T36;
  wire T37;
  wire T38;
  wire[31:0] T39;
  wire[31:0] T40;
  wire T41;
  wire T42;
  wire T43;
  wire T44;
  wire T45;
  wire T46;
  wire T47;
  wire T48;
  wire T49;
  wire T50;

  assign io_sc_top = T0;
  assign T0 = T3 ? T15 : T1;
  assign T1 = T7 ? T14 : sc_top;
  assign T2 = T7 || T3;
  assign T3 = T5 && T4;
  assign T4 = io_sc_func_type == 2'h2/* 2*/;
  assign T5 = stall == 1'h0/* 0*/;
  assign T6 = 1'h1/* 1*/ ? io_stall : stall;
  assign T7 = T5 && T8;
  assign T8 = io_sc_func_type == T9;
  assign T9 = {1'h0/* 0*/, 1'h0/* 0*/};
  assign T10 = T3 ? T13 : T11;
  assign T11 = 1'h1/* 1*/ ? T12 : sc_top;
  assign T12 = sc_top - io_imm;
  assign T13 = sc_top + io_imm;
  assign T14 = sc_top - io_imm;
  assign T15 = sc_top + io_imm;
  assign io_n_fill = T16;
  assign T16 = T17[3'h7/* 7*/:1'h0/* 0*/];
  assign T17 = T28 ? T27 : T18;
  assign T18 = T21 ? ensure_size : T19;
  assign T19 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign ensure_size = T20 + sc_top;
  assign T20 = io_imm - io_m_top;
  assign T21 = T24 && T22;
  assign T22 = ensure_size > T23;
  assign T23 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign T24 = T5 && T25;
  assign T25 = io_sc_func_type == T26;
  assign T26 = {1'h0/* 0*/, 1'h1/* 1*/};
  assign T27 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign T28 = T5 && T29;
  assign T29 = io_sc_func_type == 2'h3/* 3*/;
  assign io_n_spill = T30;
  assign T30 = T31[3'h7/* 7*/:1'h0/* 0*/];
  assign T31 = T28 ? T40 : T32;
  assign T32 = T37 ? reserve_size : T33;
  assign T33 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign reserve_size = T34 + io_imm;
  assign T34 = T36 - T35;
  assign T35 = {23'h0/* 0*/, 9'h100/* 256*/};
  assign T36 = io_m_top - sc_top;
  assign T37 = T7 && T38;
  assign T38 = reserve_size > T39;
  assign T39 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign T40 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign io_free = T3;
  assign io_fill = T41;
  assign T41 = T28 ? 1'h0/* 0*/ : T42;
  assign T42 = T3 ? 1'h0/* 0*/ : T43;
  assign T43 = T44 ? 1'h0/* 0*/ : T21;
  assign T44 = T24 && T45;
  assign T45 = ! T22;
  assign io_spill = T46;
  assign T46 = T28 ? 1'h0/* 0*/ : T47;
  assign T47 = T3 ? 1'h0/* 0*/ : T48;
  assign T48 = T49 ? 1'h0/* 0*/ : T37;
  assign T49 = T7 && T50;
  assign T50 = ! T38;

  always @(posedge clk) begin
    if(reset) begin
      sc_top <= 32'h200/* 512*/;
    end else if(T2) begin
      sc_top <= T10;
    end
    stall <= reset ? 1'h0/* 0*/ : T6;
  end
endmodule

module StackCacheTest(input clk, input reset,
    output io_led);

  wire[31:0] sc_simple_io_m_top;
  wire sc_simple_io_stall;
  wire[31:0] T0;
  wire[8:0] T1;
  wire[8:0] T2;
  wire[8:0] T3;
  wire[8:0] T4;
  wire[8:0] T5;
  wire T6;
  wire[4:0] T7;
  reg[4:0] func_gen;
  wire T8;
  wire T9;
  reg[0:0] sc_init;
  wire T10;
  wire T11;
  wire T12;
  wire T13;
  reg[3:0] count;
  wire[3:0] T14;
  wire[3:0] T15;
  wire[3:0] T16;
  wire T17;
  wire T18;
  wire[4:0] T19;
  wire[4:0] T20;
  wire[4:0] T21;
  wire[8:0] T22;
  wire T23;
  wire[4:0] T24;
  wire[8:0] T25;
  wire T26;
  wire[4:0] T27;
  wire T28;
  wire[4:0] T29;
  wire[1:0] T30;
  wire[1:0] T31;
  wire[1:0] T32;
  wire[1:0] T33;
  wire[1:0] T34;
  wire[1:0] T35;
  wire T36;
  wire[4:0] T37;
  wire[1:0] T38;
  wire[1:0] T39;
  wire[7:0] sc_ex_io_n_fill;
  wire[7:0] sc_ex_io_n_spill;
  wire[31:0] sc_ex_io_sc_top;
  wire sc_ex_io_free;
  wire sc_ex_io_fill;
  wire sc_ex_io_spill;
  wire mm_io_mmInOut_S_DataAccept;
  wire[31:0] mm_io_mmInOut_S_Data;
  wire[1:0] mm_io_mmInOut_S_Resp;
  wire[3:0] T40;
  wire[3:0] T41;
  wire[3:0] T42;
  wire[3:0] T43;
  wire[3:0] T44;
  wire T45;
  wire[4:0] T46;
  wire[31:0] T47;
  wire[8:0] T48;
  wire[8:0] T49;
  wire[8:0] T50;
  wire[8:0] T51;
  wire[8:0] T52;
  wire[31:0] T53;
  wire[8:0] T54;
  wire[8:0] T55;
  wire[8:0] T56;
  wire[8:0] T57;
  wire[8:0] T58;
  wire[8:0] T59;
  wire[8:0] T60;
  wire T61;
  wire[4:0] T62;
  wire T63;
  wire[4:0] T64;
  wire[2:0] T65;
  wire[2:0] T66;
  wire[2:0] T67;
  wire[2:0] T68;
  wire[2:0] T69;
  wire T70;
  wire[3:0] sc_simple_io_scMemInOut_M_DataByteEn;
  wire sc_simple_io_scMemInOut_M_DataValid;
  wire[31:0] sc_simple_io_scMemInOut_M_Data;
  wire[31:0] sc_simple_io_scMemInOut_M_Addr;
  wire[2:0] sc_simple_io_scMemInOut_M_Cmd;
  wire T71;
  wire T72;
  wire[31:0] sc_simple_io_scCpuInOut_S_Data;
  wire T73;
  reg[2:0] mem_delay_cnt;
  wire T74;
  reg[0:0] mem_delay;
  wire T75;
  wire T76;
  wire[2:0] T77;
  wire[2:0] T78;
  wire[2:0] T79;

  assign T0 = {23'h0/* 0*/, T1};
  assign T1 = T28 ? 9'h100/* 256*/ : T2;
  assign T2 = T26 ? T25 : T3;
  assign T3 = T23 ? T22 : T4;
  assign T4 = T6 ? T5 : 9'h100/* 256*/;
  assign T5 = {5'h0/* 0*/, 4'ha/* 10*/};
  assign T6 = func_gen == T7;
  assign T7 = {3'h0/* 0*/, 2'h2/* 2*/};
  assign T8 = T18 && T9;
  assign T9 = sc_init == 1'h0/* 0*/;
  assign T10 = ! T11;
  assign T11 = T13 && T12;
  assign T12 = sc_init == 1'h1/* 1*/;
  assign T13 = count <= 4'ha/* 10*/;
  assign T14 = 1'h1/* 1*/ ? T15 : count;
  assign T15 = count + T16;
  assign T16 = {3'h0/* 0*/, 1'h1/* 1*/};
  assign T17 = 1'h1/* 1*/ ? 1'h0/* 0*/ : sc_init;
  assign T18 = sc_simple_io_stall == 1'h0/* 0*/;
  assign T19 = 1'h1/* 1*/ ? T20 : func_gen;
  assign T20 = func_gen + T21;
  assign T21 = {4'h0/* 0*/, 1'h1/* 1*/};
  assign T22 = {5'h0/* 0*/, 4'ha/* 10*/};
  assign T23 = func_gen == T24;
  assign T24 = {3'h0/* 0*/, 2'h3/* 3*/};
  assign T25 = {5'h0/* 0*/, 4'ha/* 10*/};
  assign T26 = func_gen == T27;
  assign T27 = {2'h0/* 0*/, 3'h6/* 6*/};
  assign T28 = func_gen == T29;
  assign T29 = {2'h0/* 0*/, 3'h7/* 7*/};
  assign T30 = T28 ? T39 : T31;
  assign T31 = T26 ? 2'h2/* 2*/ : T32;
  assign T32 = T23 ? 2'h3/* 3*/ : T33;
  assign T33 = T6 ? T38 : T34;
  assign T34 = T36 ? T35 : 2'h3/* 3*/;
  assign T35 = {1'h0/* 0*/, 1'h0/* 0*/};
  assign T36 = func_gen == T37;
  assign T37 = {4'h0/* 0*/, 1'h1/* 1*/};
  assign T38 = {1'h0/* 0*/, 1'h0/* 0*/};
  assign T39 = {1'h0/* 0*/, 1'h1/* 1*/};
  assign T40 = T45 ? T44 : T41;
  assign T41 = T23 ? 4'hf/* 15*/ : T42;
  assign T42 = T11 ? 4'hf/* 15*/ : T43;
  assign T43 = {3'h0/* 0*/, 1'h0/* 0*/};
  assign T44 = {3'h0/* 0*/, 1'h0/* 0*/};
  assign T45 = func_gen == T46;
  assign T46 = {2'h0/* 0*/, 3'h4/* 4*/};
  assign T47 = {23'h0/* 0*/, T48};
  assign T48 = T23 ? 9'h131/* 305*/ : T49;
  assign T49 = T11 ? T51 : T50;
  assign T50 = {8'h0/* 0*/, 1'h0/* 0*/};
  assign T51 = 9'h1ff/* 511*/ - T52;
  assign T52 = {5'h0/* 0*/, count};
  assign T53 = {23'h0/* 0*/, T54};
  assign T54 = T63 ? 9'h1f7/* 503*/ : T55;
  assign T55 = T61 ? 9'h1f7/* 503*/ : T56;
  assign T56 = T23 ? 9'h1f7/* 503*/ : T57;
  assign T57 = T11 ? T59 : T58;
  assign T58 = {8'h0/* 0*/, 1'h0/* 0*/};
  assign T59 = 9'h1ff/* 511*/ - T60;
  assign T60 = {5'h0/* 0*/, count};
  assign T61 = func_gen == T62;
  assign T62 = {2'h0/* 0*/, 3'h5/* 5*/};
  assign T63 = func_gen == T64;
  assign T64 = {1'h0/* 0*/, 4'h8/* 8*/};
  assign T65 = T63 ? 3'b010/* 0*/ : T66;
  assign T66 = T61 ? 3'b010/* 0*/ : T67;
  assign T67 = T45 ? 3'b000/* 0*/ : T68;
  assign T68 = T23 ? 3'b001/* 0*/ : T69;
  assign T69 = {2'h0/* 0*/, T70};
  assign T70 = T11;
  assign io_led = T71;
  assign T71 = T73 ? 1'h1/* 1*/ : T72;
  assign T72 = sc_simple_io_scCpuInOut_S_Data[1'h0/* 0*/:1'h0/* 0*/];
  assign T73 = mem_delay_cnt == 3'h7/* 7*/;
  assign T74 = mem_delay == 1'h1/* 1*/;
  assign T75 = sc_simple_io_scMemInOut_M_Cmd == 3'b010/* 0*/;
  assign T76 = 1'h1/* 1*/ ? 1'h1/* 1*/ : mem_delay;
  assign T77 = 1'h1/* 1*/ ? T78 : mem_delay_cnt;
  assign T78 = mem_delay_cnt + T79;
  assign T79 = {2'h0/* 0*/, 1'h1/* 1*/};
  MainMemory mm(.clk(clk), .reset(reset),
       .io_mmInOut_M_Cmd( sc_simple_io_scMemInOut_M_Cmd ),
       .io_mmInOut_M_Addr( sc_simple_io_scMemInOut_M_Addr ),
       .io_mmInOut_M_Data( sc_simple_io_scMemInOut_M_Data ),
       .io_mmInOut_M_DataValid( sc_simple_io_scMemInOut_M_DataValid ),
       .io_mmInOut_M_DataByteEn( sc_simple_io_scMemInOut_M_DataByteEn ),
       .io_mmInOut_S_Resp( mm_io_mmInOut_S_Resp ),
       .io_mmInOut_S_Data( mm_io_mmInOut_S_Data ),
       .io_mmInOut_S_DataAccept( mm_io_mmInOut_S_DataAccept ));
  StackCache sc_simple(.clk(clk), .reset(reset),
       .io_scCpuInOut_M_Cmd( T65 ),
       .io_scCpuInOut_M_Addr( T53 ),
       .io_scCpuInOut_M_Data( T47 ),
       .io_scCpuInOut_M_ByteEn( T40 ),
       .io_scCpuInOut_S_Resp(  ),
       .io_scCpuInOut_S_Data( sc_simple_io_scCpuInOut_S_Data ),
       .io_scMemInOut_M_Cmd( sc_simple_io_scMemInOut_M_Cmd ),
       .io_scMemInOut_M_Addr( sc_simple_io_scMemInOut_M_Addr ),
       .io_scMemInOut_M_Data( sc_simple_io_scMemInOut_M_Data ),
       .io_scMemInOut_M_DataValid( sc_simple_io_scMemInOut_M_DataValid ),
       .io_scMemInOut_M_DataByteEn( sc_simple_io_scMemInOut_M_DataByteEn ),
       .io_scMemInOut_S_Resp( mm_io_mmInOut_S_Resp ),
       .io_scMemInOut_S_Data( mm_io_mmInOut_S_Data ),
       .io_scMemInOut_S_DataAccept( mm_io_mmInOut_S_DataAccept ),
       .io_spill( sc_ex_io_spill ),
       .io_fill( sc_ex_io_fill ),
       .io_free( sc_ex_io_free ),
       .io_sc_top( sc_ex_io_sc_top ),
       .io_m_top( sc_simple_io_m_top ),
       .io_n_spill( sc_ex_io_n_spill ),
       .io_n_fill( sc_ex_io_n_fill ),
       .io_stall( sc_simple_io_stall ));
  SC_ex sc_ex(.clk(clk), .reset(reset),
       .io_sc_func_type( T30 ),
       .io_imm( T0 ),
       .io_spill( sc_ex_io_spill ),
       .io_fill( sc_ex_io_fill ),
       .io_free( sc_ex_io_free ),
       .io_stall( sc_simple_io_stall ),
       .io_m_top( sc_simple_io_m_top ),
       .io_n_spill( sc_ex_io_n_spill ),
       .io_n_fill( sc_ex_io_n_fill ),
       .io_sc_top( sc_ex_io_sc_top ));

  always @(posedge clk) begin
    if(reset) begin
      func_gen <= 5'h0/* 0*/;
    end else if(T8) begin
      func_gen <= T19;
    end
    if(reset) begin
      sc_init <= 1'h1/* 1*/;
    end else if(T10) begin
      sc_init <= T17;
    end
    count <= reset ? 4'h0/* 0*/ : T14;
    if(reset) begin
      mem_delay_cnt <= 3'h0/* 0*/;
    end else if(T74) begin
      mem_delay_cnt <= T77;
    end
    if(reset) begin
      mem_delay <= 1'h0/* 0*/;
    end else if(T75) begin
      mem_delay <= T76;
    end
  end
endmodule

