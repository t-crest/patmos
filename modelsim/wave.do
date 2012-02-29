onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -radix hexadecimal /pc_tester/clk
add wave -noupdate -radix hexadecimal /pc_tester/rst
add wave -noupdate -radix hexadecimal /pc_tester/instruction_word
add wave -noupdate -radix hexadecimal /pc_tester/pc
add wave -noupdate -radix hexadecimal /pc_tester/pc_next
add wave -noupdate -radix hexadecimal /pc_tester/rs
add wave -noupdate -radix hexadecimal /pc_tester/rt
add wave -noupdate -radix hexadecimal /pc_tester/rd
add wave -noupdate -radix hexadecimal /pc_tester/func
add wave -noupdate -radix hexadecimal /pc_tester/read_data1
add wave -noupdate -radix hexadecimal /pc_tester/read_data2
add wave -noupdate -radix hexadecimal /pc_tester/write_data
add wave -noupdate -radix hexadecimal /pc_tester/operation1
add wave -noupdate -radix hexadecimal /pc_tester/operation2
add wave -noupdate -radix hexadecimal /pc_tester/write_enable
add wave -noupdate -radix hexadecimal /pc_tester/inst_type
add wave -noupdate -radix hexadecimal /pc_tester/alui_function_type
add wave -noupdate -radix hexadecimal /pc_tester/alu_function_type
add wave -noupdate -radix hexadecimal /pc_tester/alu_instruction_type
add wave -noupdate -radix hexadecimal /pc_tester/alui_immediate
add wave -noupdate -radix hexadecimal /pc_tester/inst_type_reg
add wave -noupdate -radix hexadecimal /pc_tester/alui_function_type_reg
add wave -noupdate -radix hexadecimal /pc_tester/alu_function_type_reg
add wave -noupdate -radix hexadecimal /pc_tester/alu_instruction_type_reg
add wave -noupdate -radix hexadecimal /pc_tester/alui_immediate_reg
add wave -noupdate -radix hexadecimal /pc_tester/pc_ctrl_gen
add wave -noupdate -radix hexadecimal /pc_tester/multiplexer_a_ctrl
add wave -noupdate -radix hexadecimal /pc_tester/multiplexer_b_ctrl
add wave -noupdate -radix hexadecimal /pc_tester/alu_we
add wave -noupdate -radix hexadecimal /pc_tester/wb_we
add wave -noupdate -radix hexadecimal /pc_tester/rs1
add wave -noupdate -radix hexadecimal /pc_tester/rs2
add wave -noupdate -radix hexadecimal /pc_tester/predicate_bit
add wave -noupdate -radix hexadecimal /pc_tester/pd
add wave -noupdate -radix hexadecimal /pc_tester/wb_we_out_exec
add wave -noupdate -radix hexadecimal /pc_tester/wa1
add wave -noupdate -radix hexadecimal /pc_tester/wa2
add wave -noupdate -radix hexadecimal /pc_tester/write_address_reg_file
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {84126 ps} 0}
configure wave -namecolwidth 195
configure wave -valuecolwidth 40
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
WaveRestoreZoom {0 ps} {108400 ps}
