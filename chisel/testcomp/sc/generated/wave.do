onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /sc_top/t/func_gen
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/rdData
add wave -noupdate /sc_top/t/sc_simple/clk
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/reset
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scCpuInOut_M_Cmd
add wave -noupdate -radix unsigned -childformat {{{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[31]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[30]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[29]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[28]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[27]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[26]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[25]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[24]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[23]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[22]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[21]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[20]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[19]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[18]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[17]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[16]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[15]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[14]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[13]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[12]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[11]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[10]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[9]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[8]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[7]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[6]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[5]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[4]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[3]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[2]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[1]} -radix unsigned} {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[0]} -radix unsigned}} -subitemconfig {{/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[31]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[30]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[29]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[28]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[27]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[26]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[25]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[24]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[23]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[22]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[21]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[20]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[19]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[18]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[17]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[16]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[15]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[14]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[13]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[12]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[11]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[10]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[9]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[8]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[7]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[6]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[5]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[4]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[3]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[2]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[1]} {-height 17 -radix unsigned} {/sc_top/t/sc_simple/io_scCpuInOut_M_Addr[0]} {-height 17 -radix unsigned}} /sc_top/t/sc_simple/io_scCpuInOut_M_Addr
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scCpuInOut_M_Data
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scCpuInOut_M_ByteEn
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scCpuInOut_S_Resp
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scCpuInOut_S_Data
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_M_Cmd
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_M_Addr
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_M_Data
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_M_DataValid
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_M_DataByteEn
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_S_Resp
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_S_Data
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_S_CmdAccept
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_scMemInOut_S_DataAccept
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_spill
add wave -noupdate /sc_top/t/sc_simple/n_spill
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_fill
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_free
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_sc_top
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_m_top
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_n_spill
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_n_fill
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/io_stall
add wave -noupdate /sc_top/t/sc_simple/n_fill
add wave -noupdate -radix unsigned /sc_top/t/sc_simple/m_top
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 3} {260 ps} 0}
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
WaveRestoreZoom {6645 ps} {12251 ps}
