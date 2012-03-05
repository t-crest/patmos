onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -radix hexadecimal /pc_tester/clk
add wave -noupdate -radix hexadecimal /pc_tester/rst
add wave -noupdate -radix hexadecimal /pc_tester/instruction_word
add wave -noupdate -radix hexadecimal /pc_tester/pc
add wave -noupdate -radix hexadecimal /pc_tester/dec/inst_type
add wave -noupdate -radix hexadecimal /pc_tester/reg_file/read_address1
add wave -noupdate -radix hexadecimal /pc_tester/reg_file/read_address2
add wave -noupdate -radix hexadecimal /pc_tester/reg_file/write_address
add wave -noupdate -radix hexadecimal /pc_tester/reg_file/read_data1
add wave -noupdate -radix hexadecimal /pc_tester/reg_file/read_data2
add wave -noupdate -radix hexadecimal /pc_tester/reg_file/write_data
add wave -noupdate -radix hexadecimal /pc_tester/reg_file/write_enable
add wave -noupdate -radix hexadecimal /pc_tester/reg_file/reg_bank
add wave -noupdate -radix hexadecimal /pc_tester/exec/rs
add wave -noupdate -radix hexadecimal /pc_tester/exec/rt
add wave -noupdate -radix hexadecimal /pc_tester/exec/rd
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {0 ps} 0}
configure wave -namecolwidth 150
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
WaveRestoreZoom {0 ps} {105 ns}
