set IDLE 10#0
set WR 10#1
set RD 10#2

proc step {name res msg} {
    for {set i 0} {$i <= 20} {incr i} {
        run 9.9 ns
        if {"[examine -value $name]"==$res} {
            run 0.1 ns
            return
        }
        run 0.1 ns
        #echo $msg
    }
    run 0.1 ns
    echo "-- Step timed out"
    return
}

proc ocpBurstWrite {addr data1 data2 data3 data4} {
    echo "-- OCP Burst Write"
    global IDLE
    global WR
    force io_ocpPort_M_Cmd $WR
    force io_ocpPort_M_Addr $addr
    force io_ocpPort_M_Data $data1
    force io_ocpPort_M_DataByteEn 1111
    force io_ocpPort_M_DataValid 1
    step io_ocpPort_S_CmdAccept "St1" "-- SRamCtrl not accepting ocp command"

    force io_ocpPort_M_Data $data2
    step io_ocpPort_S_DataAccept "St1" "-- SRamCtrl not accepting ocp data"

    force io_ocpPort_M_Data $data3
    step io_ocpPort_S_DataAccept "St1" "-- SRamCtrl not accepting ocp data"

    force io_ocpPort_M_Data $data4
    step io_ocpPort_S_DataAccept "St1" "-- SRamCtrl not accepting ocp data"

    force io_ocpPort_M_Cmd $IDLE
    force io_ocpPort_M_Addr 10#0
    force io_ocpPort_M_Data 10#0
    force io_ocpPort_M_DataByteEn 10#0
    force io_ocpPort_M_DataValid 0

    step io_ocpPort_S_Resp "01" "-- Waiting for slave response"
}

proc ocpBurstRead {addr} {
    echo "-- OCP Burst Read"
    global IDLE
    global RD
    force io_ocpPort_M_Cmd $RD
    force io_ocpPort_M_Addr $addr

    step io_ocpPort_S_CmdAccept "St1" "-- SRamCtrl not accepting ocp command"

    force io_ocpPort_M_Cmd $IDLE
    force io_ocpPort_M_Addr 10#0

    step io_ocpPort_S_Resp "01" "-- Waiting for slave response"
    step io_ocpPort_S_Resp "01" "-- Waiting for slave response"
    step io_ocpPort_S_Resp "01" "-- Waiting for slave response"
    step io_ocpPort_S_Resp "01" "-- Waiting for slave response"
}

proc setClkReset {} {
    force clk 1 0, 0 5000 -repeat 10000
    force reset 1 0, 0 40000
}

restart -force -nowave
#add wave -hexadecimal
do sram.do
setClkReset
run 30 ns

force io_ramIn_din 10#0 0
force io_ocpPort_M_DataValid 0 0
force io_ocpPort_M_DataByteEn 10#0 0
force io_ocpPort_M_Data 10#0 0
force io_ocpPort_M_Cmd $IDLE 0
force io_ocpPort_M_Addr 10#0 0

run 20 ns

ocpBurstWrite 10#20 10#1001 10#1002 10#1003 10#1004

run 20 ns

ocpBurstRead 10#20

run 20 ns
