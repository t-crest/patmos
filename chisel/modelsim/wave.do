onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -radix hexadecimal /Patmos_tb/clk
add wave -noupdate -radix hexadecimal /Patmos_tb/reset
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/io_led
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/fetch/addrEvenReg
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/fetch/addrOddReg
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/fetch/ispm_even
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/fetch/ispm_odd
add wave -noupdate /Patmos_tb/patmos/fetch/selIspm
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/fetch/data_even
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/fetch/data_odd
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/fetch/instr_a
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/fetch/instr_b
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/fetch/pcReg
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/fetch/pc_next
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/decode/addrImm
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/decode/decReg_instr_b
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/decode/decReg_pc
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/decode/func
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/decode/isMem
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/decode/longImm
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/decode/shamt
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {578800 ps} 0}
configure wave -namecolwidth 255
configure wave -valuecolwidth 156
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
WaveRestoreZoom {0 ps} {10500 ns}
