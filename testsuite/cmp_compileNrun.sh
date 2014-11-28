#!/bin/bash
# Alters DE2-115 on Helena
#COM_PORT=/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A700aiK5-if00-port0
# Xilinx ML605 on Helena
COM_PORT=/dev/serial/by-id/usb-Silicon_Labs_CP2103_USB_to_UART_Bridge_Controller_0001-if00-port0

cd ~/t-crest/patmos
numfailures=0
failures=""
numnotcomp=0
notcomp=""
numexclude=0

exclude_app="memtest_cmp memtest_cmp2 nocinit16 nocinit4"

echo "==> Configuring FPGA";
echo
make -C ../aegean config > /dev/null

for f in `git ls-files ./c/cmp/*.c`; do
	s=${f##*/};
	app=${s%.*};
	if [[ " ${exclude_app} " =~ " ${app} " ]]; then
		numexclude=$((numexclude+1));
		continue;
	fi
	echo ""
	echo "==> Compiling ${app}";
	make comp APP=${app} > /dev/null
	comp=$?
	if (( "comp" > "0" )); then
		echo ""
		echo "==> ${app} Did not compile";
		numnotcomp=$((numnotcomp+1));
		notcomp+=" ${app}"
		continue;
	fi
	echo "==> Executing ${app}";
	echo ""
	patserdow ${COM_PORT} tmp/${app}.elf;
	res=$?;
	if (( "$res" > "0" )); then
		numfailures=$((numfailures+1));
		failures+=" ${app}"
		echo ""
		echo "==> ${app} failed"
	fi
done

numtests=$(git ls-files ./c/cmp/*.c | wc -l)

echo ""
echo "==================================="
echo "== ${numtests} Tests found"
echo "== ${numexclude} Tests excluded"
if (( "$numnotcomp" > "0" )); then
	echo "== $numnotcomp Tests not compiling"
	echo "== Not compiling: $notcomp"
else
	echo "== All test compiled"
fi
if (( "$numfailures" > "0" )); then
	echo "== $numfailures Tests failing"
	echo "== Failing: $failures"
else
	echo "== All test passed"
fi
echo "==================================="