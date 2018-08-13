`timescale 1ns / 1ps

module imu_mpu(
    // inputs:
    address,
    clk,
    reset,
    sda_in,

    // outputs:
    readdata_0,
    readdata_1,
    readdata_2,
    readdata_3,
    readdata_4,
    readdata_5,
    readdata_6,
    readdata_7,
    readdata_8,
    readdata_9,
    scl_out,
    sda_out,
    we_out
);

    output  [ 31: 0] readdata_0;
    output  [ 31: 0] readdata_1;
    output  [ 31: 0] readdata_2;
    output  [ 31: 0] readdata_3;
    output  [ 31: 0] readdata_4;
    output  [ 31: 0] readdata_5;

    output  [ 31: 0] readdata_6;  //For future
    output  [ 31: 0] readdata_7;  //For future
    output  [ 31: 0] readdata_8;  //For future
    output  [ 31: 0] readdata_9;  //For future

    input   [  1: 0] address;
    input            clk;
    input            reset;

    output           scl_out;
    input          	 sda_in;
    output         	 sda_out;
    output 		 we_out;


    wire             clk_en;
    wire             data_in;
    wire             read_mux_out;


    reg     [ 31: 0] readdata_0;
    reg     [ 31: 0] readdata_1;
    reg     [ 31: 0] readdata_2;
    reg     [ 31: 0] readdata_3;
    reg     [ 31: 0] readdata_4;
    reg     [ 31: 0] readdata_5;

    reg     [ 31: 0] readdata_6;  //For future
    reg     [ 31: 0] readdata_7;  //For future
    reg     [ 31: 0] readdata_8;  //For future
    reg     [ 31: 0] readdata_9;  //For future


    parameter clock_in_KHz = 200;

    reg [7:0] full_clk = 50000/clock_in_KHz; // 250 in Default case.
    reg [7:0] half_clk = 25000/ clock_in_KHz;
    reg [7:0] qtr_clk = 12500/clock_in_KHz;


    reg running = 0;

    parameter dev_add = 7'b1101000;  // 0x68
    parameter reg_add = 8'b00111011; //0x3B

    reg [18:0] tx_buffer_1 = 19'b0110100001001110111; //start + dev_address + write + recvr_ack + reg_add + recvr_ack =0,1101000,0,1,00111011,1
    reg [9:0] tx_buffer_2 = 10'b0110100011;// start + dev_addr + read + slave_ack

    reg [9:0] clk_tick = 0;
    reg scl = 1;
    reg sda = 1'bz;
    reg [3:0] state = 0;
    reg tristate_en = 0;
    reg [2:0] byte;


    reg start = 0;
    reg [7:0] scl_clk = 0;


    integer i = 0;
    integer j = 0;

    reg [15:0] data1;
    reg [15:0] data2;
    reg [15:0] data3;
    reg [15:0] data4;
    reg [15:0] data5;
    reg [15:0] data6;
    reg [15:0] data7;
    reg [111:0] big_data;


    always @(posedge clk or posedge reset)
        begin
            if (reset == 1)
                begin
                    state <= 0;
                    clk_tick <= 0;
                    tristate_en <= 0;
                    sda <= 1'b1;
                    i = 0;
                    data1 <= 0;
                    start = 0;
                    readdata_0 <= 0;
                    readdata_1 <= 0;
                    readdata_2 <= 0;
                    readdata_3 <= 0;
                    readdata_4 <= 0;
                    readdata_5 <= 0;
                    big_data <= 0;
                end
            else
                begin
                    clk_tick <= clk_tick + 1;
                    if(state == 0 && clk_tick == 8'b1)
                        begin
                            state <= 1;
                            tristate_en <=1;
                            clk_tick <= 0;
                        end

                    else if(state == 1 && clk_tick == full_clk)
                        begin
                            start = 1;
                            i = i+1;
                            sda <= tx_buffer_1[18-(i-1)]?1'b1:1'b0;
                            clk_tick <= 0;
                            if(i == 19)
                                begin
                                    state<= 2;
                                    i = 0;
                                end
                        end
                    else if(state == 2 && clk_tick == full_clk)
                        begin
                            sda <= 0;
                            clk_tick <= 0;
                            state <= 3;
                        end
                    else if(state == 3 && clk_tick == half_clk)
                        begin
                            sda<= 1'b1;
                            start = 0;
                            clk_tick <= 0;
                            state <= 4;
                        end
                    else if(state == 4 && clk_tick == full_clk)
                        begin
                            start = 1;
                            i = i+1;
                            sda <= tx_buffer_2[9-(i-1)]?1'b1:1'b0;
                            clk_tick <= 0;
                            if(i == 10)
                                begin
                                    state<= 5;
                                    i = 0;
                                end
                        end
                    else if(state == 5 && clk_tick == (full_clk + half_clk))
                        begin
                            tristate_en <= 0;
                            i = i+ 1;
                            clk_tick <= half_clk;
                            big_data = {big_data[110:0], sda_in};
                            if( i == 8 || i == 16 || i == 24 || i == 32 || i == 40 || i == 48 || i == 56 || i == 64 || i == 72 || i == 80 || i == 88 || i == 96 || i == 104 )
                                begin
                                    state <= 6;
                                    clk_tick <= 0;
                                end
                            else if(i ==112)
                                begin
                                    state <= 8;
                                    tristate_en <= 0;
                                    sda <= 0;
                                    readdata_0 <= {16'b0,big_data[111:96]};
                                    readdata_1 <= {16'b0,big_data[95:80]};
                                    readdata_2 <= {16'b0,big_data[79:64]};

                                    readdata_3 <= {16'b0,big_data[47:32]};
                                    readdata_4 <= {16'b0,big_data[31:16]};
                                    readdata_5 <= {16'b0,big_data[15:0]};

                                    big_data <= 0;
                                    i <=0;
                                    clk_tick <= 0;
                                end

                        end

                    else if(state == 6 && clk_tick == half_clk)
                        begin
                            tristate_en <= 1;
                            sda <= 0;
                            clk_tick <= 0;
                            state <= 7;
                        end

                    else if(state == 7 && clk_tick == full_clk)
                        begin
                            tristate_en <= 0;
                            clk_tick <= full_clk;
                            state <= 5;
                        end

                    else if(state == 8 && clk_tick == full_clk)
                        begin

                            clk_tick <= 0;
                            state <= 9;
                        end

                    else if(state == 9 && clk_tick == half_clk)
                        begin
                            sda <=1'b1;
                            start <= 0;
                            clk_tick <= 0;
                            state <= 0;
                        end

                end
        end



    always @(posedge clk or posedge reset)
        begin
            if (reset == 1)
                begin

                    running = 0;
                    scl_clk <= 0;
                    scl <= 1;
                end
            else
                begin

                    if (start == 1)
                        begin
                            scl_clk <= scl_clk + 1;
                            if(running == 1 && scl_clk == half_clk)
                                begin
                                    scl <= 0;
                                end
                            else if (running == 1 && scl_clk == full_clk)
                                begin
                                    scl <= 1;
                                    scl_clk <= 0;
                                end
                            else if (running == 0 && scl_clk == qtr_clk)
                                begin
                                    running <= 1;
                                    scl_clk <= 0;
                                end

                        end
                    else
                        begin
                            scl <= 1;
                            scl_clk <= 0;
                            running <= 0;
                        end
                end
        end





    assign scl_out = scl;
    assign sda_out = sda;
    assign we_out = tristate_en;


endmodule