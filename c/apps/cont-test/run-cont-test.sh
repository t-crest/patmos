
TEST_SETTINGS_FILE="testing_settings.h"
RESULT_FILE="cont-test-results.csv"
REPEAT=10

# Prepare output file
echo "config,core0,core1,core2,core3" > $RESULT_FILE

function run_benchmark {
	COUNTER=0
	while [ $COUNTER -ne $REPEAT ]
	do
		# Compile new version of program
		make
		
		cd ../../..
		# Run on FPGA
		#output=$(make APP=cont-test config download)
		# Run on emulator
		output=$(timeout --kill-after=5s 30s patemu tmp/cont-test.elf)
		
		if [ $? -eq 124 ]; then
			echo "Timeout"
			# Timed out, just show zeros
			RESULT_LINE="0,0,0,0"
		else
			# Output last line (results) to file
			RESULT_LINE=$(echo "$output" | tail -n 1) 
		fi
		echo "$CONFIG_NAME,$RESULT_LINE" >> "c/apps/cont-test/$RESULT_FILE"
		COUNTER=$(( $COUNTER + 1 ))
		
		cd c/apps/cont-test
	done
}

echo "#define CORES_RUNNING 1" > $TEST_SETTINGS_FILE
echo "#define CONTENTION_LIMIT 0" >> $TEST_SETTINGS_FILE
CONFIG_NAME="patmos-default-1-0"
run_benchmark

echo "#define CORES_RUNNING 4" > $TEST_SETTINGS_FILE
echo "#define CONTENTION_LIMIT 0" >> $TEST_SETTINGS_FILE
CONFIG_NAME="patmos-default-4-0"
run_benchmark

echo "#define CORES_RUNNING 4" > $TEST_SETTINGS_FILE
echo "#define CONTENTION_LIMIT 1" >> $TEST_SETTINGS_FILE
CONFIG_NAME="patmos-cont-4-1"
run_benchmark

echo "#define CORES_RUNNING 4" > $TEST_SETTINGS_FILE
echo "#define CONTENTION_LIMIT 100" >> $TEST_SETTINGS_FILE
CONFIG_NAME="patmos-cont-4-100"
run_benchmark

echo "#define CORES_RUNNING 4" > $TEST_SETTINGS_FILE
echo "#define CONTENTION_LIMIT 500" >> $TEST_SETTINGS_FILE
CONFIG_NAME="patmos-cont-4-500"
run_benchmark

echo "#define CORES_RUNNING 4" > $TEST_SETTINGS_FILE
echo "#define CONTENTION_LIMIT 1000" >> $TEST_SETTINGS_FILE
CONFIG_NAME="patmos-cont-4-1k"
run_benchmark

echo "#define CORES_RUNNING 4" > $TEST_SETTINGS_FILE
echo "#define CONTENTION_LIMIT 2000" >> $TEST_SETTINGS_FILE
CONFIG_NAME="patmos-cont-4-2k"
run_benchmark
