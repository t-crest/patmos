onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -radix hexadecimal /Patmos_tb/clk
add wave -noupdate -radix hexadecimal /Patmos_tb/reset
add wave -noupdate /Patmos_tb/io_led
add wave -noupdate /Patmos_tb/io_uartPins_rx
add wave -noupdate /Patmos_tb/io_uartPins_tx
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/fetch/io_fedec_instr_a
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/fetch/io_fedec_instr_b
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/fetch/io_fedec_pc
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_0
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_1
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_2
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_3
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_4
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_5
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_6
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_7
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_8
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_9
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_10
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_11
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_12
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_13
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_14
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_15
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_16
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_17
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_18
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_19
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_20
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_21
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_22
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_23
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_24
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_25
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_26
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_27
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_28
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_29
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_30
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/decode/rf/rf_31
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/io_globalInOut_M_Addr
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/io_globalInOut_M_AddrSpace
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/io_globalInOut_M_ByteEn
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/io_globalInOut_M_Cmd
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/io_globalInOut_M_Data
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/io_globalInOut_S_Data
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/io_globalInOut_S_Resp
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/io_localInOut_M_Addr
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/io_localInOut_M_ByteEn
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/io_localInOut_M_Cmd
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/io_localInOut_M_Data
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/io_localInOut_S_Data
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/io_localInOut_S_Resp
add wave -noupdate -radix hexadecimal -childformat {{{/Patmos_tb/patmos/core/io_memPort_M_Addr[20]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[19]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[18]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[17]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[16]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[15]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[14]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[13]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[12]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[11]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[10]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[9]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[8]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[7]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[6]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[5]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[4]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[3]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[2]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[1]} -radix hexadecimal} {{/Patmos_tb/patmos/core/io_memPort_M_Addr[0]} -radix hexadecimal}} -subitemconfig {{/Patmos_tb/patmos/core/io_memPort_M_Addr[20]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[19]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[18]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[17]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[16]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[15]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[14]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[13]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[12]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[11]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[10]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[9]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[8]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[7]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[6]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[5]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[4]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[3]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[2]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[1]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/io_memPort_M_Addr[0]} {-height 16 -radix hexadecimal}} /Patmos_tb/patmos/core/io_memPort_M_Addr
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/io_memPort_M_Cmd
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/io_memPort_M_Data
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/io_memPort_M_DataByteEn
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/io_memPort_M_DataValid
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/io_memPort_S_CmdAccept
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/io_memPort_S_Data
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/io_memPort_S_DataAccept
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/io_memPort_S_Resp
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/iocomp/Uart/io_ocp_M_Addr
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/iocomp/Uart/io_ocp_M_ByteEn
add wave -noupdate -radix hexadecimal -childformat {{{/Patmos_tb/patmos/core/iocomp/Uart/io_ocp_M_Cmd[2]} -radix hexadecimal} {{/Patmos_tb/patmos/core/iocomp/Uart/io_ocp_M_Cmd[1]} -radix hexadecimal} {{/Patmos_tb/patmos/core/iocomp/Uart/io_ocp_M_Cmd[0]} -radix hexadecimal}} -expand -subitemconfig {{/Patmos_tb/patmos/core/iocomp/Uart/io_ocp_M_Cmd[2]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/iocomp/Uart/io_ocp_M_Cmd[1]} {-height 16 -radix hexadecimal} {/Patmos_tb/patmos/core/iocomp/Uart/io_ocp_M_Cmd[0]} {-height 16 -radix hexadecimal}} /Patmos_tb/patmos/core/iocomp/Uart/io_ocp_M_Cmd
add wave -noupdate -radix ascii /Patmos_tb/patmos/core/iocomp/Uart/io_ocp_M_Data
add wave -noupdate -radix ascii /Patmos_tb/patmos/core/iocomp/Uart/io_ocp_S_Data
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/iocomp/Uart/io_ocp_S_Resp
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_ocp_M_Addr
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_ocp_M_Cmd
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_ocp_M_Data
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_ocp_M_DataByteEn
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_ocp_M_DataValid
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_ocp_S_CmdAccept
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_ocp_S_Data
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_ocp_S_DataAccept
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_ocp_S_Resp
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramIn_din
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramOut_addr
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramOut_ce2
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramOut_dout
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramOut_doutEna
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramOut_nadsc
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramOut_nadsp
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramOut_nadv
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramOut_nbw
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramOut_nbwe
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramOut_nce1
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramOut_nce3
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramOut_ngw
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/sramCtrl/io_sSRam32CtrlPins_ramOut_noe
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/memReg_mem_trap
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/memReg_mem_xcall
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/memReg_mem_xret
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/memory/memReg_mem_xsrc
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/exc
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/excAddr
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/intr
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/io_excdec_addr
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/io_excdec_exc
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/io_excdec_excAddr
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/io_excdec_intr
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/io_excdec_src
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/io_memexc_call
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/io_memexc_exc
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/io_memexc_excAddr
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/io_memexc_ret
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/io_memexc_src
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/mask
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/sleepReg
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/source
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/src
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/srcReg
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/status
add wave -noupdate -radix hexadecimal /Patmos_tb/patmos/core/exc/vec
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {1635498900 ps} 0}
quietly wave cursor active 1
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
WaveRestoreZoom {269902 ns} {2091057800 ps}
