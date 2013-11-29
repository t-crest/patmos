onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -radix hexadecimal /Patmos_tb/clk
add wave -noupdate -radix hexadecimal /Patmos_tb/reset
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/io_led
add wave -noupdate /Patmos_tb/io_led
add wave -noupdate /Patmos_tb/io_uartPins_rx
add wave -noupdate /Patmos_tb/io_uartPins_tx
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_0
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_1
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_2
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_3
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_4
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_5
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_6
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_7
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_8
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_9
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_10
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_11
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_12
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_13
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_14
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_15
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_16
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_17
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_18
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_19
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_20
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_21
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_22
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_23
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_24
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_25
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_26
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_27
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_28
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_29
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_30
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/io_rfDebug_31
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/fetch/io_exfe_branchPc
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/fetch/io_exfe_doBranch
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/fetch/io_fedec_instr_a
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/fetch/io_fedec_instr_b
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/fetch/io_fedec_pc
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {897966400 ps} 0}
configure wave -namecolwidth 506
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
WaveRestoreZoom {0 ps} {1682650900 ps}
