
TEST_SETTINGS_FILE="testing_settings.h"
RESULT_FILE="cont-test-results.csv"
REPEAT=20

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
		output=$(make APP=cont-test config download)
		# Output last line (results) to file
		RESULT_LINE=$(echo "$output" | tail -n 1) 
		echo "$CONFIG_NAME,$RESULT_LINE" >> "c/apps/cont-test/$RESULT_FILE"
		COUNTER=$(( $COUNTER + 1 ))
		
		cd c/apps/cont-test
	done
}


echo "#define CORES_RUNNING 1" > $TEST_SETTINGS_FILE
CONFIG_NAME="patmos-default-1"
run_benchmark


echo "#define CORES_RUNNING 4" > $TEST_SETTINGS_FILE
CONFIG_NAME="patmos-default-4"
run_benchmark
