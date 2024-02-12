LF_PROJECT_ROOT ?= "$(CURDIR)/../.."
LF_MAIN_TARGET ?= HelloWorld
LF_MAIN_TARGET_LC := $(shell echo $(LF_MAIN_TARGET) | tr '[:upper:]' '[:lower:]')

SERIAL?=/dev/ttyUSB0

INCS  := -I"$(LF_PROJECT_ROOT)/include" \
	-I"$(LF_PROJECT_ROOT)/include/api" \
	-I"$(LF_PROJECT_ROOT)/include/core" \
	-I"$(LF_PROJECT_ROOT)/include/core/platform" \
	-I"$(LF_PROJECT_ROOT)/include/core/modal_models" \
	-I"$(LF_PROJECT_ROOT)/include/core/utils" 
CC := patmos-clang 
CFLAGS := $(INCS) -DINITIAL_EVENT_QUEUE_SIZE=10 -DINITIAL_REACT_QUEUE_SIZE=10 -DLF_UNTHREADED=1 -DLF_REACTION_GRAPH_BREADTH=1
SRC_FILES := _$(LF_MAIN_TARGET_LC)_main.c $(LF_MAIN_TARGET).c lib/schedule.c _counter.c _multiplier.c _printer.c
SRC_FILES += core/reactor_common.c core/lf_token.c core/reactor.c core/tag.c core/environment.c
SRC_FILES += core/utils/util.c core/utils/vector.c core/utils/pqueue.c 
SRC_FILES += core/utils/hashset/hashset_itr.c core/utils/hashset/hashset.c 
SRC_FILES += core/platform/lf_patmos_support.c

OBJ_FILES := $(patsubst %.c,%.o,$(SRC_FILES))
EXE_NAME := $(LF_MAIN_TARGET).elf

.PHONY: all clean wcet

all: $(EXE_NAME)
$(EXE_NAME): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^ -O2 -mserialize=lf.pml
$(OBJ_FILES): %.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ_FILES) $(EXE_NAME)

wcet: $(EXE_NAME)
	platin wcet --disable-ait -i lf.pml -b $(EXE_NAME) -e _doublereaction_function_0 --report report.txt
	# platin visualize -i lf.pml -f main --show-timings=platin
	