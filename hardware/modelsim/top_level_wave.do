onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /patmos_tb_sram16/patmos_inst/clk
add wave -noupdate /patmos_tb_sram16/patmos_inst/comp/reset
add wave -noupdate -radix unsigned /patmos_tb_sram16/patmos_inst/comp/core/fetch/pcReg
add wave -noupdate -radix hexadecimal /patmos_tb_sram16/patmos_inst/comp/core/fetch/io_fedec_instr_a
add wave -noupdate -radix hexadecimal /patmos_tb_sram16/patmos_inst/comp/core/fetch/io_fedec_instr_b
add wave -noupdate /patmos_tb_sram16/patmos_inst/oLedsPins_led
add wave -noupdate /patmos_tb_sram16/patmos_inst/iKeysPins_key
add wave -noupdate /patmos_tb_sram16/patmos_inst/oUartPins_txd
add wave -noupdate /patmos_tb_sram16/patmos_inst/iUartPins_rxd
add wave -noupdate /patmos_tb_sram16/patmos_inst/u_h
add wave -noupdate /patmos_tb_sram16/patmos_inst/u_l
add wave -noupdate /patmos_tb_sram16/patmos_inst/v_h
add wave -noupdate /patmos_tb_sram16/patmos_inst/v_l
add wave -noupdate /patmos_tb_sram16/patmos_inst/w_h
add wave -noupdate /patmos_tb_sram16/patmos_inst/w_l
add wave -noupdate /patmos_tb_sram16/patmos_inst/oSRAM_A
add wave -noupdate /patmos_tb_sram16/patmos_inst/SRAM_DQ
add wave -noupdate /patmos_tb_sram16/patmos_inst/oSRAM_CE_N
add wave -noupdate /patmos_tb_sram16/patmos_inst/oSRAM_OE_N
add wave -noupdate /patmos_tb_sram16/patmos_inst/oSRAM_WE_N
add wave -noupdate /patmos_tb_sram16/patmos_inst/oSRAM_LB_N
add wave -noupdate /patmos_tb_sram16/patmos_inst/oSRAM_UB_N
add wave -noupdate /patmos_tb_sram16/patmos_inst/avs_write_n
add wave -noupdate /patmos_tb_sram16/patmos_inst/avs_read_n
add wave -noupdate -radix hexadecimal /patmos_tb_sram16/patmos_inst/avs_address
add wave -noupdate -radix hexadecimal /patmos_tb_sram16/patmos_inst/avs_writedata
add wave -noupdate -radix hexadecimal /patmos_tb_sram16/patmos_inst/avs_readdata
add wave -noupdate /patmos_tb_sram16/patmos_inst/pwm/reset_n
add wave -noupdate -childformat {{{/patmos_tb_sram16/patmos_inst/pwm/pwm_reg[0]} -radix hexadecimal} {{/patmos_tb_sram16/patmos_inst/pwm/pwm_reg[1]} -radix hexadecimal} {{/patmos_tb_sram16/patmos_inst/pwm/pwm_reg[2]} -radix hexadecimal}} -expand -subitemconfig {{/patmos_tb_sram16/patmos_inst/pwm/pwm_reg[0]} {-radix hexadecimal} {/patmos_tb_sram16/patmos_inst/pwm/pwm_reg[1]} {-radix hexadecimal} {/patmos_tb_sram16/patmos_inst/pwm/pwm_reg[2]} {-radix hexadecimal}} /patmos_tb_sram16/patmos_inst/pwm/pwm_reg
add wave -noupdate /patmos_tb_sram16/patmos_inst/pwm/max_reg
add wave -noupdate /patmos_tb_sram16/patmos_inst/pwm/block_reg
add wave -noupdate /patmos_tb_sram16/patmos_inst/pwm/trigger_up_reg
add wave -noupdate /patmos_tb_sram16/patmos_inst/pwm/trigger_down_reg
add wave -noupdate /patmos_tb_sram16/patmos_inst/pwm/compare
add wave -noupdate /patmos_tb_sram16/patmos_inst/pwm/en_lower
add wave -noupdate /patmos_tb_sram16/patmos_inst/pwm/en_upper
add wave -noupdate -radix hexadecimal /patmos_tb_sram16/patmos_inst/pwm/counter
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {2950 ns} 0}
quietly wave cursor active 1
configure wave -namecolwidth 147
configure wave -valuecolwidth 156
configure wave -justifyvalue left
configure wave -signalnamewidth 1
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
WaveRestoreZoom {0 ns} {3150 ns}
