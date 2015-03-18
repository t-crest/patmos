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
	