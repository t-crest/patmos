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
    output[3:0] io_scMemInOut_M_ByteEn,
    input [1:0] io_scMemInOut_S_Resp,
    input [31:0] io_scMemInOut_S_Data,
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
  wire T26;
  wire T27;
  wire T28;
  wire T29;
  wire T30;
  wire T31;
  wire T32;
  wire T33;
  wire signed  T34;
  wire[7:0] T35;
  wire signed [7:0] T36;
  wire[7:0] T37;
  reg signed [7:0] n_fill;
  wire T38;
  wire T39;
  wire T40;
  wire T41;
  wire T42;
  wire T43;
  wire T44;
  wire T45;
  wire[8:0] T46;
  wire[8:0] T47;
  wire[8:0] T48;
  wire[8:0] T49;
  wire T50;
  wire T51;
  wire[8:0] T52;
  wire[8:0] T53;
  wire[8:0] T54;
  wire signed [7:0] T55;
  wire[7:0] T56;
  wire signed [1:0] T57;
  wire T58;
  wire T59;
  wire[8:0] T60;
  wire[8:0] T61;
  wire T62;
  wire T63;
  wire T64;
  wire signed  T65;
  wire[7:0] T66;
  wire signed [7:0] T67;
  wire[7:0] T68;
  reg signed [7:0] n_spill;
  wire T69;
  wire T70;
  wire T71;
  wire T72;
  wire[8:0] T73;
  wire[8:0] T74;
  wire[8:0] T75;
  wire[8:0] T76;
  wire T77;
  wire T78;
  wire[8:0] T79;
  wire[8:0] T80;
  wire[8:0] T81;
  wire signed [7:0] T82;
  wire[7:0] T83;
  wire signed [1:0] T84;
  wire T85;
  wire T86;
  wire[8:0] T87;
  wire[8:0] T88;
  wire T89;
  wire T90;
  wire T91;
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
  wire[2:0] T105;
  wire[2:0] T106;
  wire[2:0] T107;
  wire[2:0] T108;
  wire[2:0] T109;
  wire[2:0] T110;
  wire[2:0] T111;
  wire[2:0] T112;
  wire[2:0] T113;
  wire[2:0] T114;
  wire[2:0] T115;
  reg[31:0] m_top;
  wire T116;
  wire T117;
  wire T118;
  wire T119;
  wire[31:0] T120;
  wire[31:0] T121;
  wire[31:0] T122;
  wire[31:0] T123;
  wire[31:0] T124;
  wire[31:0] T125;
  wire[31:0] T126;
  wire[3:0] T127;
  wire[31:0] T128;
  wire[31:0] T129;
  wire[31:0] rdDataSpill;
  wire[7:0] T130;
  reg [7:0] sc0 [63:0];
  wire[31:0] T131;
  wire[7:0] T132;
  wire[7:0] T133;
  wire[5:0] T134;
  reg[31:0] mem_addr_masked_reg;
  wire[31:0] T135;
  wire[31:0] mem_addr_masked;
  wire[31:0] T136;
  wire T137;
  wire[31:0] T138;
  wire[5:0] T139;
  wire[31:0] cpu_addr_masked;
  wire[31:0] T140;
  wire[31:0] T141;
  wire[5:0] T142;
  wire[31:0] T143;
  wire[7:0] T144;
  wire[7:0] T145;
  wire[5:0] T146;
  wire T147;
  wire T148;
  wire[3:0] sc_en;
  wire T149;
  wire[5:0] T150;
  wire[7:0] T151;
  reg [7:0] sc1 [63:0];
  wire[31:0] T152;
  wire[7:0] T153;
  wire[7:0] T154;
  wire[5:0] T155;
  wire[31:0] T156;
  wire[5:0] T157;
  wire[31:0] T158;
  wire[5:0] T159;
  wire[31:0] T160;
  wire[7:0] T161;
  wire[7:0] T162;
  wire[5:0] T163;
  wire T164;
  wire T165;
  wire[5:0] T166;
  wire[7:0] T167;
  reg [7:0] sc2 [63:0];
  wire[31:0] T168;
  wire[7:0] T169;
  wire[7:0] T170;
  wire[5:0] T171;
  wire[31:0] T172;
  wire[5:0] T173;
  wire[31:0] T174;
  wire[5:0] T175;
  wire[31:0] T176;
  wire[7:0] T177;
  wire[7:0] T178;
  wire[5:0] T179;
  wire T180;
  wire T181;
  wire[5:0] T182;
  wire[7:0] T183;
  reg [7:0] sc3 [63:0];
  wire[31:0] T184;
  wire[7:0] T185;
  wire[7:0] T186;
  wire[5:0] T187;
  wire[31:0] T188;
  wire[5:0] T189;
  wire[31:0] T190;
  wire[5:0] T191;
  wire[31:0] T192;
  wire[7:0] T193;
  wire[7:0] T194;
  wire[5:0] T195;
  wire T196;
  wire T197;
  wire[5:0] T198;
  wire[31:0] T199;
  wire[31:0] T200;
  wire[31:0] T201;
  wire[31:0] T202;
  wire[31:0] T203;
  wire[31:0] T204;
  wire[31:0] T205;
  wire[31:0] T206;
  wire[31:0] T207;
  wire[31:0] T208;
  wire[2:0] T209;
  wire[2:0] T210;
  wire[2:0] T211;
  wire[2:0] T212;
  wire[2:0] T213;
  wire[2:0] T214;
  wire[2:0] T215;
  wire[2:0] T216;
  wire T217;
  wire[31:0] rdData;
  wire[7:0] T218;
  wire[5:0] T219;
  wire[7:0] T220;
  wire[5:0] T221;
  wire[7:0] T222;
  wire[5:0] T223;
  wire[7:0] T224;
  wire[5:0] T225;
  wire[1:0] T226;
  wire T227;
  wire T228;
  wire T229;
  wire T230;

  assign io_stall = T0;
  assign T0 = T21 ? 1'h1/* 1*/ : T1;
  assign T1 = T28 ? 1'h1/* 1*/ : T2;
  assign T2 = T19 ? 1'h0/* 0*/ : T3;
  assign T3 = T32 ? 1'h0/* 0*/ : T4;
  assign T4 = T40 ? 1'h1/* 1*/ : T5;
  assign T5 = T63 ? 1'h0/* 0*/ : T6;
  assign T6 = T71 ? 1'h1/* 1*/ : T7;
  assign T7 = T92 ? 1'h1/* 1*/ : T8;
  assign T8 = T94 ? 1'h0/* 0*/ : T9;
  assign T9 = T42 ? 1'h1/* 1*/ : T10;
  assign T10 = T12 && T11;
  assign T11 = io_spill == 1'h1/* 1*/;
  assign T12 = state == 3'h0/* 0*/;
  assign T13 = T20 || T14;
  assign T14 = T19 && T15;
  assign T15 = ! T16;
  assign T16 = T18 || T17;
  assign T17 = io_spill == 1'h1/* 1*/;
  assign T18 = io_fill == 1'h1/* 1*/;
  assign T19 = state == 3'h3/* 3*/;
  assign T20 = T24 || T21;
  assign T21 = T19 && T22;
  assign T22 = T23 && T17;
  assign T23 = ! T18;
  assign T24 = T29 || T25;
  assign T25 = T28 && T26;
  assign T26 = ! T27;
  assign T27 = io_scMemInOut_S_Resp == 2'b01/* 0*/;
  assign T28 = T19 && T18;
  assign T29 = T31 || T30;
  assign T30 = T28 && T27;
  assign T31 = T62 || T32;
  assign T32 = T41 && T33;
  assign T33 = ! T34;
  assign T34 = $signed(T36) >= $signed(T35);
  assign T35 = {6'h0/* 0*/, 2'h0/* 0*/};
  assign T36 = $signed(n_fill) - $signed(T37);
  assign T37 = {6'h0/* 0*/, 2'h1/* 1*/};
  assign T38 = T39 || T28;
  assign T39 = T42 || T40;
  assign T40 = T41 && T34;
  assign T41 = state == 3'h2/* 2*/;
  assign T42 = T12 && T43;
  assign T43 = T45 && T44;
  assign T44 = io_fill == 1'h1/* 1*/;
  assign T45 = ! T11;
  assign T46 = T28 ? T60 : T47;
  assign T47 = T40 ? T54 : T48;
  assign T48 = 1'h1/* 1*/ ? T52 : T49;
  assign T49 = {T50, n_fill};
  assign T50 = {1'h1/* 1*/{T51}};
  assign T51 = n_fill[3'h7/* 7*/:3'h7/* 7*/];
  assign T52 = T53;
  assign T53 = {1'h0/* 0*/, io_n_fill};
  assign T54 = {T58, T55};
  assign T55 = $signed(n_fill) - $signed(T56);
  assign T56 = {6'h0/* 0*/, T57};
  assign T57 = 2'h1/* 1*/;
  assign T58 = {1'h1/* 1*/{T59}};
  assign T59 = T55[3'h7/* 7*/:3'h7/* 7*/];
  assign T60 = T61;
  assign T61 = {1'h0/* 0*/, io_n_fill};
  assign T62 = T89 || T63;
  assign T63 = T72 && T64;
  assign T64 = ! T65;
  assign T65 = $signed(T67) >= $signed(T66);
  assign T66 = {6'h0/* 0*/, 2'h0/* 0*/};
  assign T67 = $signed(n_spill) - $signed(T68);
  assign T68 = {6'h0/* 0*/, 2'h1/* 1*/};
  assign T69 = T70 || T21;
  assign T70 = T10 || T71;
  assign T71 = T72 && T65;
  assign T72 = state == 3'h1/* 1*/;
  assign T73 = T21 ? T87 : T74;
  assign T74 = T71 ? T81 : T75;
  assign T75 = 1'h1/* 1*/ ? T79 : T76;
  assign T76 = {T77, n_spill};
  assign T77 = {1'h1/* 1*/{T78}};
  assign T78 = n_spill[3'h7/* 7*/:3'h7/* 7*/];
  assign T79 = T80;
  assign T80 = {1'h0/* 0*/, io_n_spill};
  assign T81 = {T85, T82};
  assign T82 = $signed(n_spill) - $signed(T83);
  assign T83 = {6'h0/* 0*/, T84};
  assign T84 = 2'h1/* 1*/;
  assign T85 = {1'h1/* 1*/{T86}};
  assign T86 = T82[3'h7/* 7*/:3'h7/* 7*/];
  assign T87 = T88;
  assign T88 = {1'h0/* 0*/, io_n_fill};
  assign T89 = T93 || T90;
  assign T90 = T92 && T91;
  assign T91 = io_scMemInOut_S_Resp == 2'b01/* 0*/;
  assign T92 = state == 3'h4/* 4*/;
  assign T93 = T99 || T94;
  assign T94 = T12 && T95;
  assign T95 = T97 && T96;
  assign T96 = io_free == 1'h1/* 1*/;
  assign T97 = ! T98;
  assign T98 = T11 || T44;
  assign T99 = T103 || T100;
  assign T100 = T42 && T101;
  assign T101 = ! T102;
  assign T102 = io_scMemInOut_S_Resp == 2'b01/* 0*/;
  assign T103 = T10 || T104;
  assign T104 = T42 && T102;
  assign T105 = T14 ? 3'h0/* 0*/ : T106;
  assign T106 = T21 ? 3'h1/* 1*/ : T107;
  assign T107 = T25 ? 3'h4/* 4*/ : T108;
  assign T108 = T30 ? 3'h2/* 2*/ : T109;
  assign T109 = T32 ? 3'h0/* 0*/ : T110;
  assign T110 = T63 ? 3'h0/* 0*/ : T111;
  assign T111 = T90 ? 3'h2/* 2*/ : T112;
  assign T112 = T94 ? 3'h3/* 3*/ : T113;
  assign T113 = T100 ? 3'h4/* 4*/ : T114;
  assign T114 = T104 ? 3'h2/* 2*/ : T115;
  assign T115 = 1'h1/* 1*/ ? 3'h1/* 1*/ : state;
  assign io_m_top = m_top;
  assign T116 = T119 || T117;
  assign T117 = T19 && T118;
  assign T118 = io_sc_top > m_top;
  assign T119 = T71 || T40;
  assign T120 = T117 ? io_sc_top : T121;
  assign T121 = T40 ? T125 : T122;
  assign T122 = 1'h1/* 1*/ ? T123 : m_top;
  assign T123 = m_top - T124;
  assign T124 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign T125 = m_top + T126;
  assign T126 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign io_scMemInOut_M_ByteEn = T127;
  assign T127 = T71 ? 4'b1111/* 0*/ : 4'b0000/* 0*/;
  assign io_scMemInOut_M_Data = T128;
  assign T128 = T71 ? rdDataSpill : T129;
  assign T129 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign rdDataSpill = {T183, T167, T151, T130};
  assign T130 = sc0[T150];
  assign T132 = T133;
  assign T133 = io_scMemInOut_S_Data[3'h7/* 7*/:1'h0/* 0*/];
  assign T134 = mem_addr_masked_reg[3'h5/* 5*/:1'h0/* 0*/];
  assign T135 = 1'h1/* 1*/ ? mem_addr_masked : mem_addr_masked_reg;
  assign mem_addr_masked = io_scMemInOut_M_Addr & T136;
  assign T136 = {24'h0/* 0*/, 8'hff/* 255*/};
  assign T137 = state == 3'h2/* 2*/;
  assign T139 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign cpu_addr_masked = io_scCpuInOut_M_Addr & T140;
  assign T140 = {24'h0/* 0*/, 8'hff/* 255*/};
  assign T142 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T144 = T145;
  assign T145 = io_scCpuInOut_M_Data[3'h7/* 7*/:1'h0/* 0*/];
  assign T146 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T147 = T148;
  assign T148 = sc_en[1'h0/* 0*/:1'h0/* 0*/];
  assign sc_en = T149 ? io_scCpuInOut_M_ByteEn : 4'b0000/* 0*/;
  assign T149 = io_scCpuInOut_M_Cmd == 3'b001/* 0*/;
  assign T150 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T151 = sc1[T166];
  assign T153 = T154;
  assign T154 = io_scMemInOut_S_Data[4'hf/* 15*/:4'h8/* 8*/];
  assign T155 = mem_addr_masked_reg[3'h5/* 5*/:1'h0/* 0*/];
  assign T157 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T159 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T161 = T162;
  assign T162 = io_scCpuInOut_M_Data[4'hf/* 15*/:4'h8/* 8*/];
  assign T163 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T164 = T165;
  assign T165 = sc_en[1'h1/* 1*/:1'h1/* 1*/];
  assign T166 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T167 = sc2[T182];
  assign T169 = T170;
  assign T170 = io_scMemInOut_S_Data[5'h17/* 23*/:5'h10/* 16*/];
  assign T171 = mem_addr_masked_reg[3'h5/* 5*/:1'h0/* 0*/];
  assign T173 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T175 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T177 = T178;
  assign T178 = io_scCpuInOut_M_Data[5'h17/* 23*/:5'h10/* 16*/];
  assign T179 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T180 = T181;
  assign T181 = sc_en[2'h2/* 2*/:2'h2/* 2*/];
  assign T182 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T183 = sc3[T198];
  assign T185 = T186;
  assign T186 = io_scMemInOut_S_Data[5'h1f/* 31*/:5'h18/* 24*/];
  assign T187 = mem_addr_masked_reg[3'h5/* 5*/:1'h0/* 0*/];
  assign T189 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T191 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T193 = T194;
  assign T194 = io_scCpuInOut_M_Data[5'h1f/* 31*/:5'h18/* 24*/];
  assign T195 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T196 = T197;
  assign T197 = sc_en[2'h3/* 3*/:2'h3/* 3*/];
  assign T198 = mem_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign io_scMemInOut_M_Addr = T199;
  assign T199 = T40 ? T207 : T200;
  assign T200 = T71 ? T205 : T201;
  assign T201 = T92 ? T203 : T202;
  assign T202 = {31'h0/* 0*/, 1'h0/* 0*/};
  assign T203 = m_top + T204;
  assign T204 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign T205 = m_top - T206;
  assign T206 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign T207 = m_top + T208;
  assign T208 = {31'h0/* 0*/, 1'h1/* 1*/};
  assign io_scMemInOut_M_Cmd = T209;
  assign T209 = T30 ? 3'b010/* 0*/ : T210;
  assign T210 = T32 ? 3'b000/* 0*/ : T211;
  assign T211 = T40 ? 3'b010/* 0*/ : T212;
  assign T212 = T63 ? 3'b000/* 0*/ : T213;
  assign T213 = T71 ? 3'b001/* 0*/ : T214;
  assign T214 = T92 ? 3'b010/* 0*/ : T215;
  assign T215 = T42 ? 3'b010/* 0*/ : T216;
  assign T216 = {2'h0/* 0*/, T217};
  assign T217 = T10;
  assign io_scCpuInOut_S_Data = rdData;
  assign rdData = {T224, T222, T220, T218};
  assign T218 = sc0[T219];
  assign T219 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T220 = sc1[T221];
  assign T221 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T222 = sc2[T223];
  assign T223 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign T224 = sc3[T225];
  assign T225 = cpu_addr_masked[3'h5/* 5*/:1'h0/* 0*/];
  assign io_scCpuInOut_S_Resp = T226;
  assign T226 = {1'h0/* 0*/, T227};
  assign T227 = T228;
  assign T228 = T230 || T229;
  assign T229 = io_scCpuInOut_M_Cmd == 3'b010/* 0*/;
  assign T230 = io_scCpuInOut_M_Cmd == 3'b101/* 0*/;

  always @(posedge clk) begin
    if(reset) begin
      state <= 3'h0/* 0*/;
    end else if(T13) begin
      state <= T105;
    end
    if(reset) begin
      n_fill <= 8'h0/* 0*/;
    end else if(T38) begin
      n_fill <= T46;
    end
    if(reset) begin
      n_spill <= 8'h0/* 0*/;
    end else if(T69) begin
      n_spill <= T73;
    end
    if(reset) begin
      m_top <= 32'h200/* 512*/;
    end else if(T116) begin
      m_top <= T120;
    end
    if (T137)
      sc0[T134] <= T132;
    mem_addr_masked_reg <= reset ? 32'h0/* 0*/ : T135;
    if (T147)
      sc0[T146] <= T144;
    if (T137)
      sc1[T155] <= T153;
    if (T164)
      sc1[T163] <= T161;
    if (T137)
      sc2[T171] <= T169;
    if (T180)
      sc2[T179] <= T177;
    if (T137)
      sc3[T187] <= T185;
    if (T196)
      sc3[T195] <= T193;
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
  wire[31:0] T40;
  reg [31:0] mem [511:0];
  wire[31:0] T41;
  wire[31:0] T42;
  wire[31:0] sc_simple_io_scMemInOut_M_Data;
  wire[8:0] T43;
  wire[31:0] T44;
  wire[8:0] T45;
  wire[31:0] sc_simple_io_scMemInOut_M_Addr;
  wire[8:0] T46;
  wire[1:0] T47;
  wire T48;
  wire T49;
  reg[2:0] mem_delay_cnt;
  wire T50;
  reg[0:0] mem_delay;
  wire T51;
  wire[2:0] sc_simple_io_scMemInOut_M_Cmd;
  wire T52;
  wire[2:0] T53;
  wire[2:0] T54;
  wire[2:0] T55;
  wire[3:0] T56;
  wire[3:0] T57;
  wire[3:0] T58;
  wire[3:0] T59;
  wire[3:0] T60;
  wire T61;
  wire[4:0] T62;
  wire[31:0] T63;
  wire[8:0] T64;
  wire[8:0] T65;
  wire[8:0] T66;
  wire[8:0] T67;
  wire[8:0] T68;
  wire[31:0] T69;
  wire[8:0] T70;
  wire[8:0] T71;
  wire[8:0] T72;
  wire[8:0] T73;
  wire[8:0] T74;
  wire[8:0] T75;
  wire[8:0] T76;
  wire T77;
  wire[4:0] T78;
  wire T79;
  wire[4:0] T80;
  wire[2:0] T81;
  wire[2:0] T82;
  wire[2:0] T83;
  wire[2:0] T84;
  wire[2:0] T85;
  wire T86;
  wire T87;
  wire T88;
  wire[31:0] sc_simple_io_scCpuInOut_S_Data;

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
  assign T40 = mem[T46];
  assign T42 = sc_simple_io_scMemInOut_M_Data;
  assign T43 = sc_simple_io_m_top[4'h8/* 8*/:1'h0/* 0*/];
  assign T45 = sc_simple_io_scMemInOut_M_Addr[4'h8/* 8*/:1'h0/* 0*/];
  assign T46 = sc_simple_io_scMemInOut_M_Addr[4'h8/* 8*/:1'h0/* 0*/];
  assign T47 = {1'h0/* 0*/, T48};
  assign T48 = T49;
  assign T49 = mem_delay_cnt == 3'h7/* 7*/;
  assign T50 = mem_delay == 1'h1/* 1*/;
  assign T51 = sc_simple_io_scMemInOut_M_Cmd == 3'b010/* 0*/;
  assign T52 = 1'h1/* 1*/ ? 1'h1/* 1*/ : mem_delay;
  assign T53 = 1'h1/* 1*/ ? T54 : mem_delay_cnt;
  assign T54 = mem_delay_cnt + T55;
  assign T55 = {2'h0/* 0*/, 1'h1/* 1*/};
  assign T56 = T61 ? T60 : T57;
  assign T57 = T23 ? 4'hf/* 15*/ : T58;
  assign T58 = T11 ? 4'hf/* 15*/ : T59;
  assign T59 = {3'h0/* 0*/, 1'h0/* 0*/};
  assign T60 = {3'h0/* 0*/, 1'h0/* 0*/};
  assign T61 = func_gen == T62;
  assign T62 = {2'h0/* 0*/, 3'h4/* 4*/};
  assign T63 = {23'h0/* 0*/, T64};
  assign T64 = T23 ? 9'h131/* 305*/ : T65;
  assign T65 = T11 ? T67 : T66;
  assign T66 = {8'h0/* 0*/, 1'h0/* 0*/};
  assign T67 = 9'h1ff/* 511*/ - T68;
  assign T68 = {5'h0/* 0*/, count};
  assign T69 = {23'h0/* 0*/, T70};
  assign T70 = T79 ? 9'h1f7/* 503*/ : T71;
  assign T71 = T77 ? 9'h1f9/* 505*/ : T72;
  assign T72 = T23 ? 9'h1f7/* 503*/ : T73;
  assign T73 = T11 ? T75 : T74;
  assign T74 = {8'h0/* 0*/, 1'h0/* 0*/};
  assign T75 = 9'h1ff/* 511*/ - T76;
  assign T76 = {5'h0/* 0*/, count};
  assign T77 = func_gen == T78;
  assign T78 = {2'h0/* 0*/, 3'h5/* 5*/};
  assign T79 = func_gen == T80;
  assign T80 = {1'h0/* 0*/, 4'h8/* 8*/};
  assign T81 = T79 ? 3'b010/* 0*/ : T82;
  assign T82 = T77 ? 3'b010/* 0*/ : T83;
  assign T83 = T61 ? 3'b000/* 0*/ : T84;
  assign T84 = T23 ? 3'b001/* 0*/ : T85;
  assign T85 = {2'h0/* 0*/, T86};
  assign T86 = T11;
  assign io_led = T87;
  assign T87 = T49 ? 1'h1/* 1*/ : T88;
  assign T88 = sc_simple_io_scCpuInOut_S_Data[1'h0/* 0*/:1'h0/* 0*/];
  StackCache sc_simple(.clk(clk), .reset(reset),
       .io_scCpuInOut_M_Cmd( T81 ),
       .io_scCpuInOut_M_Addr( T69 ),
       .io_scCpuInOut_M_Data( T63 ),
       .io_scCpuInOut_M_ByteEn( T56 ),
       .io_scCpuInOut_S_Resp(  ),
       .io_scCpuInOut_S_Data( sc_simple_io_scCpuInOut_S_Data ),
       .io_scMemInOut_M_Cmd( sc_simple_io_scMemInOut_M_Cmd ),
       .io_scMemInOut_M_Addr( sc_simple_io_scMemInOut_M_Addr ),
       .io_scMemInOut_M_Data( sc_simple_io_scMemInOut_M_Data ),
       .io_scMemInOut_M_ByteEn(  ),
       .io_scMemInOut_S_Resp( T47 ),
       .io_scMemInOut_S_Data( T40 ),
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
    if (1'h1/* 1*/)
      mem[T43] <= T42;
    if(reset) begin
      mem_delay_cnt <= 3'h0/* 0*/;
    end else if(T50) begin
      mem_delay_cnt <= T53;
    end
    if(reset) begin
      mem_delay <= 1'h0/* 0*/;
    end else if(T51) begin
      mem_delay <= T52;
    end
  end
endmodule

