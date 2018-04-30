#!/bin/bash

# Script to send simple UDP messages over Ethernet and report the turnaround time

if [ $# -lt 3 ]
  then
    echo "Missing or no arguments supplied"
    echo "ex: ./udpSend host port data"
else
    def_host=$1
    def_port=$2
    def_msg=$3

    start_at=$(date +%s,%N)
    _s1=$(echo $start_at | cut -d',' -f1)   # sec
    _s2=$(echo $start_at | cut -d',' -f2)   # nano sec

    echo -n "$def_msg" | nc -4u -w 1 $def_host $def_port

    end_at=$(date +%s,%N)
    _e1=$(echo $end_at | cut -d',' -f1)
    _e2=$(echo $end_at | cut -d',' -f2)

    time_cost=$(bc <<< "scale=3; $_e1 - $_s1 + ($_e2 -$_s2)/1000000000")

    echo "Turnaround time = $time_cost sec"

fi