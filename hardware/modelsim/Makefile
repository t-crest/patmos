# Use Wine on OSX
# I would like to use a better way, but some shell variables
# are not set within make.... Don't know why...
ifeq ($(TERM_PROGRAM),Apple_Terminal)
	WINE=wine
else
	WINE=
endif

all: sim

sim:
	rm -rf work
	$(WINE) vlib work
	$(WINE) vcom -quiet +acc gen_utils.vhd conversions.vhd sim_ssram_512x36.vhd
	$(WINE) vlog -quiet +acc ../build/Patmos.v Patmos_tb.v
#	$(WINE) vlog -quiet +acc CY7C2263KV18/CY7C2263KV18.v ../build/Patmos.v Patmos_Qdr_tb.v
	$(WINE) vsim -i -do sim.do Patmos_tb

sim_de2-115-mc:
	rm -rf work
	$(WINE) vlib work
	$(WINE) vcom -quiet +acc ../../../aegean/vhdl/packages/test.vhd
	$(WINE) vcom -quiet +acc CY7C10612DV33/package_utility.vhd CY7C10612DV33/package_timing.vhd CY7C10612DV33/cy7c10612dv33.vhd
	$(WINE) vcom -quiet +acc ../vhdl/altera/pll_sim.vhd gen_utils.vhd conversions.vhd sim_ssram_512x36.vhd ../vhdl/patmos_de2-115-mc.vhdl
	$(WINE) vlog -quiet +acc ../vhdl/motor_control/ssg_emb_pwm.v 
	$(WINE) vlog -quiet +acc ../build/Patmos.v 
	$(WINE) vcom -quiet +acc patmos_top_tb_sram16.vhd
	$(WINE) vsim -i -do sim.do patmos_tb_sram16
	
sim_pat:
	rm -rf work
	$(WINE) vlib work
	$(WINE) vlog -quiet +acc ../build/Patmos.v
	$(WINE) vlog +acc ../quartus/altde2-115-mc/DOC_Axis_Periphs_patmos/synthesis/submodules/*.v
	$(WINE) vcom +acc /uhome/rboso/rbosodata/altera/14.0/quartus/eda/sim_lib/altera_mf_components.vhd
	$(WINE) vcom +acc /uhome/rboso/rbosodata/altera/14.0/quartus/eda/sim_lib/altera_mf.vhd
	$(WINE) vsim -i -do sim.do work.altera_avalon_mm_clock_crossing_bridge work.Patmos


sim_pat-mc:
	rm -rf work
	$(WINE) vlib work
	$(WINE) vlog -quiet +acc ../build/Patmos.v
	$(WINE) vcom -quiet +acc /uhome/rboso/rbosodata/altera/14.0/quartus/eda/sim_lib/altera_mf_components.vhd
	$(WINE) vcom -quiet +acc /uhome/rboso/rbosodata/altera/14.0/quartus/eda/sim_lib/altera_mf.vhd
	$(WINE) vlog +acc ../quartus/altde2-115-mc/DOC_Monitor/synthesis/submodules/*.v
	$(WINE) vlog +acc ../quartus/altde2-115-mc/DOC_Monitor/synthesis/submodules/*.sv
	$(WINE) vlog +acc ../quartus/altde2-115-mc/DOC_Axis_Periphs_patmos/synthesis/submodules/*.v
	$(WINE) vlog +acc ../quartus/altde2-115-mc/DOC_Axis_Periphs_patmos/synthesis/submodules/*.sv
	$(WINE) vcom +acc ../quartus/altde2-115-mc/DOC_Axis_Periphs_patmos/synthesis/submodules/*.vhd
	$(WINE) vlog -quiet +acc ../quartus/altde2-115-mc/DEFINES_DE2115.v
	$(WINE) vcom -quiet +acc -2008 ../vhdl/patmos_de2-115-mc.vhdl
	$(WINE) vcom -quiet +acc -2008 altpll_patmos.vhd
	$(WINE) vcom -quiet +acc -2008 ../quartus/altde2-115-mc/DOC_Axis_Periphs_patmos/synthesis/DOC_Axis_Periphs_patmos.vhd
	$(WINE) vcom -quiet +acc -2008 ../quartus/altde2-115-mc/DOC_Monitor/synthesis/DOC_Monitor.vhd
	$(WINE) vsim -novopt -i -t 1ns -do sim_pat-mc.do patmos_top