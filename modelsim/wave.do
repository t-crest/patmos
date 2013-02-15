onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /patmos_testbench/clk
add wave -noupdate /patmos_testbench/led
add wave -noupdate /patmos_testbench/txd
add wave -noupdate /patmos_testbench/rxd
add wave -noupdate -group SRAM /patmos_testbench/oSRAM_A
add wave -noupdate -group SRAM /patmos_testbench/SRAM_DQ
add wave -noupdate -group SRAM /patmos_testbench/oSRAM_CE1_N
add wave -noupdate -group SRAM /patmos_testbench/oSRAM_OE_N
add wave -noupdate -group SRAM /patmos_testbench/oSRAM_BE_N
add wave -noupdate -group SRAM /patmos_testbench/oSRAM_WE_N
add wave -noupdate -group SRAM /patmos_testbench/oSRAM_GW_N
add wave -noupdate -group SRAM /patmos_testbench/oSRAM_CLK
add wave -noupdate -group SRAM /patmos_testbench/oSRAM_ADSC_N
add wave -noupdate -group SRAM /patmos_testbench/oSRAM_ADSP_N
add wave -noupdate -group SRAM /patmos_testbench/oSRAM_ADV_N
add wave -noupdate -group SRAM /patmos_testbench/oSRAM_CE2
add wave -noupdate -group SRAM /patmos_testbench/oSRAM_CE3_N
add wave -noupdate /patmos_testbench/internal_rst
add wave -noupdate /patmos_testbench/core/clk
add wave -noupdate /patmos_testbench/core/led
add wave -noupdate /patmos_testbench/core/txd
add wave -noupdate /patmos_testbench/core/rxd
add wave -noupdate /patmos_testbench/core/mem_write
add wave -noupdate /patmos_testbench/core/mem_data_out_muxed
add wave -noupdate /patmos_testbench/core/pat_rst
add wave -noupdate /patmos_testbench/core/data_mem_data_out
add wave -noupdate /patmos_testbench/core/execute_dout
add wave -noupdate /patmos_testbench/core/core/clk
add wave -noupdate /patmos_testbench/core/core/rst
add wave -noupdate /patmos_testbench/core/core/mem_write
add wave -noupdate /patmos_testbench/core/core/mem_data_out_muxed
add wave -noupdate /patmos_testbench/core/core/data_mem_data_out
add wave -noupdate /patmos_testbench/core/core/execute_dout_core
add wave -noupdate /patmos_testbench/core/core/fetch_dout
add wave -noupdate /patmos_testbench/core/core/fetch_reg1
add wave -noupdate /patmos_testbench/core/core/fetch_reg2
add wave -noupdate /patmos_testbench/core/core/decode_din
add wave -noupdate /patmos_testbench/core/core/decode_dout
add wave -noupdate /patmos_testbench/core/core/mem_dout
add wave -noupdate /patmos_testbench/core/core/fet/clk
add wave -noupdate /patmos_testbench/core/core/fet/rst
add wave -noupdate /patmos_testbench/core/core/fet/decout
add wave -noupdate /patmos_testbench/core/core/fet/exout
add wave -noupdate /patmos_testbench/core/core/fet/reg1
add wave -noupdate /patmos_testbench/core/core/fet/reg2
add wave -noupdate /patmos_testbench/core/core/fet/dout
add wave -noupdate /patmos_testbench/core/core/fet/pc
add wave -noupdate /patmos_testbench/core/core/fet/pc_next
add wave -noupdate /patmos_testbench/core/core/fet/pc_add
add wave -noupdate /patmos_testbench/core/core/fet/evn_next
add wave -noupdate /patmos_testbench/core/core/fet/addr_evn
add wave -noupdate /patmos_testbench/core/core/fet/addr_odd
add wave -noupdate /patmos_testbench/core/core/fet/feout
add wave -noupdate /patmos_testbench/core/core/fet/data_evn
add wave -noupdate /patmos_testbench/core/core/fet/data_odd
add wave -noupdate /patmos_testbench/core/core/fet/instr_a
add wave -noupdate /patmos_testbench/core/core/fet/instr_b
add wave -noupdate /patmos_testbench/core/core/fet/dout_feout
add wave -noupdate /patmos_testbench/core/core/fet/rom_evn/address
add wave -noupdate /patmos_testbench/core/core/fet/rom_evn/q
add wave -noupdate /patmos_testbench/core/core/fet/rom_odd/address
add wave -noupdate /patmos_testbench/core/core/fet/rom_odd/q
add wave -noupdate /patmos_testbench/core/core/reg_file/clk
add wave -noupdate /patmos_testbench/core/core/reg_file/rst
add wave -noupdate /patmos_testbench/core/core/reg_file/read_address1
add wave -noupdate /patmos_testbench/core/core/reg_file/read_address2
add wave -noupdate /patmos_testbench/core/core/reg_file/write_address
add wave -noupdate /patmos_testbench/core/core/reg_file/read_data1
add wave -noupdate /patmos_testbench/core/core/reg_file/read_data2
add wave -noupdate /patmos_testbench/core/core/reg_file/write_data
add wave -noupdate /patmos_testbench/core/core/reg_file/write_enable
add wave -noupdate /patmos_testbench/core/core/reg_file/ram
add wave -noupdate /patmos_testbench/core/core/reg_file/wr_addr_reg
add wave -noupdate /patmos_testbench/core/core/reg_file/wr_data_reg
add wave -noupdate /patmos_testbench/core/core/reg_file/wr_en_reg
add wave -noupdate /patmos_testbench/core/core/reg_file/rd_addr_reg1
add wave -noupdate /patmos_testbench/core/core/reg_file/rd_addr_reg2
add wave -noupdate /patmos_testbench/core/core/reg_file/fwd1
add wave -noupdate /patmos_testbench/core/core/reg_file/fwd2
add wave -noupdate /patmos_testbench/core/core/dec/clk
add wave -noupdate /patmos_testbench/core/core/dec/rst
add wave -noupdate /patmos_testbench/core/core/dec/din
add wave -noupdate /patmos_testbench/core/core/dec/dout
add wave -noupdate /patmos_testbench/core/core/dec/alu_func
add wave -noupdate /patmos_testbench/core/core/alu/clk
add wave -noupdate /patmos_testbench/core/core/alu/rst
add wave -noupdate /patmos_testbench/core/core/alu/decdout
add wave -noupdate /patmos_testbench/core/core/alu/memdout
add wave -noupdate /patmos_testbench/core/core/alu/rd
add wave -noupdate /patmos_testbench/core/core/alu/rd1
add wave -noupdate /patmos_testbench/core/core/alu/rd2
add wave -noupdate /patmos_testbench/core/core/alu/adrs
add wave -noupdate /patmos_testbench/core/core/alu/cmp_equal
add wave -noupdate /patmos_testbench/core/core/alu/cmp_result
add wave -noupdate /patmos_testbench/core/core/alu/predicate
add wave -noupdate /patmos_testbench/core/core/alu/predicate_reg
add wave -noupdate /patmos_testbench/core/core/alu/rs1
add wave -noupdate /patmos_testbench/core/core/alu/rs2
add wave -noupdate /patmos_testbench/core/core/alu/doutex_alu_result_reg
add wave -noupdate /patmos_testbench/core/core/alu/doutex_alu_adrs_reg
add wave -noupdate /patmos_testbench/core/core/alu/doutex_write_back_reg
add wave -noupdate /patmos_testbench/core/core/alu/doutex_reg_write
add wave -noupdate /patmos_testbench/core/core/alu/din_rs1
add wave -noupdate /patmos_testbench/core/core/alu/din_rs2
add wave -noupdate /patmos_testbench/core/core/alu/alu_src2
add wave -noupdate /patmos_testbench/core/core/alu/shamt
add wave -noupdate /patmos_testbench/core/core/alu/shifted_arg
add wave -noupdate /patmos_testbench/core/core/alu/pc
add wave -noupdate /patmos_testbench/core/core/alu/doutex_lm_write
add wave -noupdate /patmos_testbench/core/core/alu/doutex_reg_write_reg
add wave -noupdate /patmos_testbench/core/core/alu/doutex_lm_read
add wave -noupdate /patmos_testbench/core/core/alu/predicate_checked
add wave -noupdate /patmos_testbench/core/core/memory_stage/clk
add wave -noupdate /patmos_testbench/core/core/memory_stage/rst
add wave -noupdate /patmos_testbench/core/core/memory_stage/mem_write
add wave -noupdate /patmos_testbench/core/core/memory_stage/mem_data_out_muxed
add wave -noupdate /patmos_testbench/core/core/memory_stage/dout
add wave -noupdate /patmos_testbench/core/core/memory_stage/decdout
add wave -noupdate /patmos_testbench/core/core/memory_stage/ldt_type
add wave -noupdate /patmos_testbench/core/core/memory_stage/datain
add wave -noupdate /patmos_testbench/core/core/memory_stage/ld_half
add wave -noupdate /patmos_testbench/core/core/memory_stage/ld_byte
add wave -noupdate /patmos_testbench/core/core/memory_stage/s_u
add wave -noupdate /patmos_testbench/core/core/memory_stage/spill
add wave -noupdate /patmos_testbench/core/core/memory_stage/fill
add wave -noupdate /patmos_testbench/core/core/memory_stage/stall
add wave -noupdate /patmos_testbench/core/core/memory_stage/gm_stall
add wave -noupdate /patmos_testbench/core/core/memory_stage/gm_is_read
add wave -noupdate /patmos_testbench/core/core/memory_stage/gm_is_write
add wave -noupdate /patmos_testbench/core/core/memory_stage/gm_read_done
add wave -noupdate /patmos_testbench/core/core/memory_stage/gm_write_done
add wave -noupdate /patmos_testbench/core/core/memory_stage/gm_en_spill
add wave -noupdate /patmos_testbench/core/core/memory_stage/gm_spill
add wave -noupdate -group hide /patmos_testbench/core/wrapper/clk
add wave -noupdate -group hide /patmos_testbench/core/wrapper/pat_rst
add wave -noupdate -group hide /patmos_testbench/core/wrapper/mem_write
add wave -noupdate -group hide /patmos_testbench/core/wrapper/data_mem_data_out
add wave -noupdate -group hide /patmos_testbench/core/wrapper/mem_data_out_muxed
add wave -noupdate -group hide /patmos_testbench/core/wrapper/execute_dout
add wave -noupdate -group hide /patmos_testbench/core/wrapper/led
add wave -noupdate -group hide /patmos_testbench/core/wrapper/txd
add wave -noupdate -group hide /patmos_testbench/core/wrapper/rxd
add wave -noupdate -group hide /patmos_testbench/core/wrapper/memdin_reg
add wave -noupdate -group hide /patmos_testbench/core/wrapper/mem_data_out_uart
add wave -noupdate -group hide /patmos_testbench/core/wrapper/int_res
add wave -noupdate -group hide /patmos_testbench/core/wrapper/res_cnt
add wave -noupdate -group hide /patmos_testbench/core/wrapper/rst
add wave -noupdate -group hide /patmos_testbench/core/wrapper/led_reg
add wave -noupdate -group hide /patmos_testbench/core/wrapper/counter
add wave -noupdate -group hide /patmos_testbench/core/wrapper/cntus
add wave -noupdate -group hide /patmos_testbench/core/wrapper/cnt_div
add wave -noupdate -group hide /patmos_testbench/core/wrapper/io_next
add wave -noupdate -group hide /patmos_testbench/core/wrapper/io_reg
add wave -noupdate -group hide /patmos_testbench/core/wrapper/uart_rd
add wave -noupdate -group hide /patmos_testbench/core/wrapper/uart_wr
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/clk
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/reset
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/address
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/wr_data
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rd
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/wr
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rd_data
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/txd
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rxd
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/ua_dout
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/ua_wr
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tdre
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/ua_rd
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rdrf
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/uart_tx_state
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf_dout
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf_rd
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf_empty
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf_full
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tsr
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tx_clk
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/uart_rx_state
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf_wr
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf_empty
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf_full
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rxd_reg
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rx_buf
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rx_d
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rsr
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rx_clk
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rx_clk_ena
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/clk
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/reset
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/din
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/dout
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/rd
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/wr
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/empty
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/full
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/r
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/w
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/rp
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/f
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/di
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/do
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/g1(0)/f1/clk
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/g1(0)/f1/reset
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/g1(0)/f1/din
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/g1(0)/f1/dout
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/g1(0)/f1/rd
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/g1(0)/f1/wr
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/g1(0)/f1/rd_prev
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/g1(0)/f1/full
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/g1(0)/f1/buf
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/tf/g1(0)/f1/f
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/clk
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/reset
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/din
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/dout
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/rd
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/wr
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/empty
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/full
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/r
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/w
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/rp
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/f
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/di
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/do
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/g1(0)/f1/clk
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/g1(0)/f1/reset
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/g1(0)/f1/din
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/g1(0)/f1/dout
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/g1(0)/f1/rd
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/g1(0)/f1/wr
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/g1(0)/f1/rd_prev
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/g1(0)/f1/full
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/g1(0)/f1/buf
add wave -noupdate -group hide /patmos_testbench/core/wrapper/ua/rf/g1(0)/f1/f
add wave -noupdate -expand -group SDRAM /patmos_testbench/sdram_sa
add wave -noupdate -expand -group SDRAM /patmos_testbench/sdram_ba
add wave -noupdate -expand -group SDRAM /patmos_testbench/sdram_cs_n
add wave -noupdate -expand -group SDRAM /patmos_testbench/sdram_cke
add wave -noupdate -expand -group SDRAM /patmos_testbench/sdram_ras_n
add wave -noupdate -expand -group SDRAM /patmos_testbench/sdram_cas_n
add wave -noupdate -expand -group SDRAM /patmos_testbench/sdram_we_n
add wave -noupdate -expand -group SDRAM /patmos_testbench/sdram_dq
add wave -noupdate -expand -group SDRAM /patmos_testbench/sdram_dqm
add wave -noupdate -expand -group SDRAM /patmos_testbench/sdram_dq_out
add wave -noupdate -expand -group SDRAM /patmos_testbench/sdram_dq_dir
add wave -noupdate -childformat {{/patmos_testbench/gm_master.MCmd -radix hexadecimal -childformat {{/patmos_testbench/gm_master.MCmd(2) -radix hexadecimal} {/patmos_testbench/gm_master.MCmd(1) -radix hexadecimal} {/patmos_testbench/gm_master.MCmd(0) -radix hexadecimal}}} {/patmos_testbench/gm_master.MAddr -radix hexadecimal} {/patmos_testbench/gm_master.MData -radix hexadecimal}} -expand -subitemconfig {/patmos_testbench/gm_master.MCmd {-height 17 -radix hexadecimal -childformat {{/patmos_testbench/gm_master.MCmd(2) -radix hexadecimal} {/patmos_testbench/gm_master.MCmd(1) -radix hexadecimal} {/patmos_testbench/gm_master.MCmd(0) -radix hexadecimal}}} /patmos_testbench/gm_master.MCmd(2) {-height 17 -radix hexadecimal} /patmos_testbench/gm_master.MCmd(1) {-height 17 -radix hexadecimal} /patmos_testbench/gm_master.MCmd(0) {-height 17 -radix hexadecimal} /patmos_testbench/gm_master.MAddr {-height 17 -radix hexadecimal} /patmos_testbench/gm_master.MData {-height 17 -radix hexadecimal}} /patmos_testbench/gm_master
add wave -noupdate -childformat {{/patmos_testbench/gm_slave.SData -radix hexadecimal}} -expand -subitemconfig {/patmos_testbench/gm_slave.SData {-height 17 -radix hexadecimal}} /patmos_testbench/gm_slave
add wave -noupdate /patmos_testbench/sdr_sdram_inst/state_r
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {509818 ps} 0}
configure wave -namecolwidth 380
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {169397 ps} {579393 ps}
