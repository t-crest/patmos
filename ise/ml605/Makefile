DESIGN_NAME = patmos
UCF_FILE    = ml605.ucf
TOP_NAME = patmos_core

# file list taken from ../../quartus/altde2-70/patmos.qsf

SRCS_SYNTH = \
  ../../vhdl/io/uart.vhd \
  ../../vhdl/core/sc_pack.vhd \
  ../../vhdl/core/sc_ssram32.vhd \
  ../../vhdl/generated/patmos_rom.vhd \
  ../../vhdl/core/patmos_types.vhd \
  ../../vhdl/core/patmos_type_package.vhd \
  ../../vhdl/core/patmos_stack_cache_ctrl.vhd \
  ../../vhdl/core/patmos_stack_cache.vhd \
  ../../vhdl/core/patmos_register_file.vhd \
  ../../vhdl/core/patmos_mux_32.vhd \
  ../../vhdl/core/patmos_mux_3x32.vhd \
  ../../vhdl/core/patmos_mem_stage.vhd \
  ../../vhdl/core/patmos_instruction_memory.vhd \
  ../../vhdl/core/patmos_forward.vhd \
  ../../vhdl/core/patmos_fetch.vhd \
  ../../vhdl/core/patmos_dual_port_ram.vhd \
  ../../vhdl/core/patmos_decode.vhd \
  ../../vhdl/core/patmos_data_memory.vhd \
  ../../vhdl/core/patmos_core.vhd \
  ../../vhdl/core/patmos_alu.vhd \
  ../../vhdl/core/patmos_adder.vhd 

PART=xc6vlx240t-1-ff1156

MAP_FLAGS = $(INTSTYLE) -w -logic_opt off -ol high -t 1 -xt 0 -register_duplication off -r 4 -global_opt off -mt off -ir off -pr off -lc off -power off
# -w overwrite, -ol effort-level, -mt multi-threaded
PAR_FLAGS = $(INTSTYLE) -w -ol std -mt 4

 

#w============= END OF CONFIG ======================
UTILITY_DIR := $(PWD)/../ise-make-system/
include $(UTILITY_DIR)/xilinx_rules.mk
