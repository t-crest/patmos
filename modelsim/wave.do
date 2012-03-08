onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -radix hexadecimal /patmos_testbench/clk
add wave -noupdate -radix hexadecimal /patmos_testbench/rst
add wave -noupdate -radix hexadecimal /patmos_testbench/fet/pc
add wave -noupdate -radix hexadecimal /patmos_testbench/reg_file/reg_bank
add wave -noupdate -radix hexadecimal /patmos_testbench/exec/rs
add wave -noupdate -radix hexadecimal /patmos_testbench/exec/rt
add wave -noupdate -radix hexadecimal /patmos_testbench/exec/pd
add wave -noupdate -radix hexadecimal /patmos_testbench/exec/rd
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {105000 ps} 0}
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
WaveRestoreZoom {100250 ps} {106522 ps}
