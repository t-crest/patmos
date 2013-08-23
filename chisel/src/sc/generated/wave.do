onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /sc_top/t/sc_simple/clk
add wave -noupdate /sc_top/t/sc_simple/reset
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scCpuInOut_M_Cmd
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scCpuInOut_M_Addr
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scCpuInOut_M_Data
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scCpuInOut_M_ByteEn
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scCpuInOut_S_Resp
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scCpuInOut_S_Data
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_M_Cmd
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_M_Addr
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_M_Data
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_M_DataValid
add wave -noupdate -radix binary /sc_top/t/sc_simple/io_scMemInOut_S_Resp
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_S_Data
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_S_DataAccept
add wave -noupdate /sc_top/t/sc_simple/n_spill
add wave -noupdate /sc_top/t/sc_simple/n_fill
add wave -noupdate /sc_top/t/sc_simple/m_top
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_stall
add wave -noupdate /sc_top/t/sc_simple/burst_count
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 3} {1060 ps} 0}
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
WaveRestoreZoom {0 ps} {1222 ps}
