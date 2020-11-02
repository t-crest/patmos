
################################################################
# This is a generated script based on design: design_1
#
# Though there are limitations about the generated script,
# the main purpose of this utility is to make learning
# IP Integrator Tcl commands easier.
################################################################

namespace eval _tcl {
proc get_script_folder {} {
   set script_path [file normalize [info script]]
   set script_folder [file dirname $script_path]
   return $script_folder
}
}
variable script_folder
set script_folder [_tcl::get_script_folder]

################################################################
# Check if script is running in correct Vivado version.
################################################################
set scripts_vivado_version 2019.2
set current_vivado_version [version -short]

if { [string first $scripts_vivado_version $current_vivado_version] == -1 } {
   puts ""
   catch {common::send_msg_id "BD_TCL-109" "ERROR" "This script was generated using Vivado <$scripts_vivado_version> and is being run in <$current_vivado_version> of Vivado. Please run the script in Vivado <$scripts_vivado_version> then open the design in Vivado <$current_vivado_version>. Upgrade the design by running \"Tools => Report => Report IP Status...\", then run write_bd_tcl to create an updated script."}

   return 1
}

################################################################
# START
################################################################

# To test this script, run the following commands from Vivado Tcl console:
# source design_1_script.tcl


# The design that will be created by this Tcl script contains the following 
# module references:
# mii2rgmii, patmos_top

# Please add the sources of those modules before sourcing this Tcl script.

# If there is no project opened, this script will create a
# project, but make sure you do not have an existing project
# <./myproj/project_1.xpr> in the current working folder.

set list_projs [get_projects -quiet]
if { $list_projs eq "" } {
   create_project project_1 myproj -part xc7k325tffg676-1
}


# CHANGE DESIGN NAME HERE
variable design_name
set design_name design_1

# If you do not already have an existing IP Integrator design open,
# you can create a design using the following command:
#    create_bd_design $design_name

# Creating design if needed
set errMsg ""
set nRet 0

set cur_design [current_bd_design -quiet]
set list_cells [get_bd_cells -quiet]

if { ${design_name} eq "" } {
   # USE CASES:
   #    1) Design_name not set

   set errMsg "Please set the variable <design_name> to a non-empty value."
   set nRet 1

} elseif { ${cur_design} ne "" && ${list_cells} eq "" } {
   # USE CASES:
   #    2): Current design opened AND is empty AND names same.
   #    3): Current design opened AND is empty AND names diff; design_name NOT in project.
   #    4): Current design opened AND is empty AND names diff; design_name exists in project.

   if { $cur_design ne $design_name } {
      common::send_msg_id "BD_TCL-001" "INFO" "Changing value of <design_name> from <$design_name> to <$cur_design> since current design is empty."
      set design_name [get_property NAME $cur_design]
   }
   common::send_msg_id "BD_TCL-002" "INFO" "Constructing design in IPI design <$cur_design>..."

} elseif { ${cur_design} ne "" && $list_cells ne "" && $cur_design eq $design_name } {
   # USE CASES:
   #    5) Current design opened AND has components AND same names.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 1
} elseif { [get_files -quiet ${design_name}.bd] ne "" } {
   # USE CASES: 
   #    6) Current opened design, has components, but diff names, design_name exists in project.
   #    7) No opened design, design_name exists in project.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 2

} else {
   # USE CASES:
   #    8) No opened design, design_name not in project.
   #    9) Current opened design, has components, but diff names, design_name not in project.

   common::send_msg_id "BD_TCL-003" "INFO" "Currently there is no design <$design_name> in project, so creating one..."

   create_bd_design $design_name

   common::send_msg_id "BD_TCL-004" "INFO" "Making design <$design_name> as current_bd_design."
   current_bd_design $design_name

}

common::send_msg_id "BD_TCL-005" "INFO" "Currently the variable <design_name> is equal to \"$design_name\"."

if { $nRet != 0 } {
   catch {common::send_msg_id "BD_TCL-114" "ERROR" $errMsg}
   return $nRet
}

##################################################################
# DESIGN PROCs
##################################################################



# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_root_design { parentCell } {

  variable script_folder
  variable design_name

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports
  set gpio_rtl_0_tri_o [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:gpio_rtl:1.0 gpio_rtl_0_tri_o ]


  # Create ports
  set btn [ create_bd_port -dir I -from 3 -to 0 btn ]
  set clk_in_n [ create_bd_port -dir I -type clk -freq_hz 200000000 clk_in_n ]
  set clk_in_p [ create_bd_port -dir I -type clk -freq_hz 200000000 clk_in_p ]
  set iUartPins_rxd [ create_bd_port -dir O iUartPins_rxd ]
  set led [ create_bd_port -dir O -from 3 -to 0 led ]
  set oUartPins_txd [ create_bd_port -dir I oUartPins_txd ]
  set phy_rstn_1 [ create_bd_port -dir O -type rst phy_rstn_1 ]
  set rgmii_rx_ctl_1 [ create_bd_port -dir I rgmii_rx_ctl_1 ]
  set rgmii_rxc_1 [ create_bd_port -dir I rgmii_rxc_1 ]
  set rgmii_rxd_1 [ create_bd_port -dir I -from 3 -to 0 rgmii_rxd_1 ]
  set rgmii_tx_ctl_1 [ create_bd_port -dir O rgmii_tx_ctl_1 ]
  set rgmii_txc_1 [ create_bd_port -dir O rgmii_txc_1 ]
  set rgmii_txd_1 [ create_bd_port -dir O -from 3 -to 0 rgmii_txd_1 ]

  # Create instance: axi_ethernetlite_0, and set properties
  set axi_ethernetlite_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_ethernetlite:3.0 axi_ethernetlite_0 ]
  set_property -dict [ list \
   CONFIG.C_INCLUDE_INTERNAL_LOOPBACK {0} \
   CONFIG.C_INCLUDE_MDIO {0} \
   CONFIG.C_RX_PING_PONG {0} \
   CONFIG.C_TX_PING_PONG {0} \
 ] $axi_ethernetlite_0

  # Create instance: axi_gpio_0, and set properties
  set axi_gpio_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_0 ]
  set_property -dict [ list \
   CONFIG.C_ALL_OUTPUTS {1} \
   CONFIG.C_GPIO_WIDTH {4} \
 ] $axi_gpio_0

  # Create instance: clk_wiz_0, and set properties
  set clk_wiz_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz_0 ]
  set_property -dict [ list \
   CONFIG.CLKIN1_JITTER_PS {50.0} \
   CONFIG.CLKOUT1_JITTER {112.316} \
   CONFIG.CLKOUT1_PHASE_ERROR {89.971} \
   CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {100.000} \
   CONFIG.MMCM_CLKFBOUT_MULT_F {5.000} \
   CONFIG.MMCM_CLKIN1_PERIOD {5.000} \
   CONFIG.MMCM_CLKIN2_PERIOD {10.0} \
   CONFIG.MMCM_CLKOUT0_DIVIDE_F {10.000} \
   CONFIG.PRIM_IN_FREQ {200.000} \
   CONFIG.PRIM_SOURCE {Differential_clock_capable_pin} \
   CONFIG.USE_RESET {false} \
 ] $clk_wiz_0

  # Create instance: clk_wiz_1, and set properties
  set clk_wiz_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz_1 ]
  set_property -dict [ list \
   CONFIG.CLKIN1_JITTER_PS {100.0} \
   CONFIG.CLKOUT1_JITTER {181.828} \
   CONFIG.CLKOUT1_PHASE_ERROR {104.359} \
   CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {25.000} \
   CONFIG.CLKOUT1_USED {true} \
   CONFIG.CLKOUT2_JITTER {175.402} \
   CONFIG.CLKOUT2_PHASE_ERROR {98.575} \
   CONFIG.CLKOUT2_REQUESTED_OUT_FREQ {25.000} \
   CONFIG.CLKOUT2_USED {false} \
   CONFIG.CLK_OUT1_PORT {clk_25} \
   CONFIG.CLK_OUT2_PORT {clk_rgmii} \
   CONFIG.MMCM_CLKFBOUT_MULT_F {9.125} \
   CONFIG.MMCM_CLKIN1_PERIOD {10.000} \
   CONFIG.MMCM_CLKIN2_PERIOD {10.000} \
   CONFIG.MMCM_CLKOUT0_DIVIDE_F {36.500} \
   CONFIG.MMCM_CLKOUT1_DIVIDE {1} \
   CONFIG.NUM_OUT_CLKS {1} \
   CONFIG.PRIM_IN_FREQ {100.000} \
   CONFIG.PRIM_SOURCE {Single_ended_clock_capable_pin} \
   CONFIG.USE_RESET {false} \
 ] $clk_wiz_1

  # Create instance: mii2rgmii_0, and set properties
  set block_name mii2rgmii
  set block_cell_name mii2rgmii_0
  if { [catch {set mii2rgmii_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $mii2rgmii_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: patmos_top_0, and set properties
  set block_name patmos_top
  set block_cell_name patmos_top_0
  if { [catch {set patmos_top_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $patmos_top_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: patmos_top_0_axi_periph, and set properties
  set patmos_top_0_axi_periph [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 patmos_top_0_axi_periph ]
  set_property -dict [ list \
   CONFIG.NUM_MI {1} \
 ] $patmos_top_0_axi_periph

  # Create instance: rst_clk_wiz_0_100M, and set properties
  set rst_clk_wiz_0_100M [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 rst_clk_wiz_0_100M ]

  # Create interface connections
  connect_bd_intf_net -intf_net axi_gpio_0_GPIO [get_bd_intf_ports gpio_rtl_0_tri_o] [get_bd_intf_pins axi_gpio_0/GPIO]
  connect_bd_intf_net -intf_net patmos_top_0_axi_periph_M00_AXI [get_bd_intf_pins axi_gpio_0/S_AXI] [get_bd_intf_pins patmos_top_0_axi_periph/M00_AXI]
  connect_bd_intf_net -intf_net patmos_top_0_m_axi [get_bd_intf_pins patmos_top_0/m_axi] [get_bd_intf_pins patmos_top_0_axi_periph/S00_AXI]

  # Create port connections
  connect_bd_net -net axi_ethernetlite_0_phy_rst_n [get_bd_ports phy_rstn_1] [get_bd_pins axi_ethernetlite_0/phy_rst_n]
  connect_bd_net -net axi_ethernetlite_0_phy_tx_data [get_bd_pins axi_ethernetlite_0/phy_tx_data] [get_bd_pins mii2rgmii_0/mii_phy_tx_data]
  connect_bd_net -net axi_ethernetlite_0_phy_tx_en [get_bd_pins axi_ethernetlite_0/phy_tx_en] [get_bd_pins mii2rgmii_0/mii_phy_tx_en]
  connect_bd_net -net btn_1 [get_bd_ports btn] [get_bd_pins patmos_top_0/btn]
  connect_bd_net -net clk_in_n_1 [get_bd_ports clk_in_n] [get_bd_pins clk_wiz_0/clk_in1_n]
  connect_bd_net -net clk_in_p_1 [get_bd_ports clk_in_p] [get_bd_pins clk_wiz_0/clk_in1_p]
  connect_bd_net -net clk_wiz_0_clk_out1 [get_bd_pins axi_ethernetlite_0/s_axi_aclk] [get_bd_pins axi_gpio_0/s_axi_aclk] [get_bd_pins clk_wiz_0/clk_out1] [get_bd_pins clk_wiz_1/clk_in1] [get_bd_pins patmos_top_0/clk_int] [get_bd_pins patmos_top_0_axi_periph/ACLK] [get_bd_pins patmos_top_0_axi_periph/M00_ACLK] [get_bd_pins patmos_top_0_axi_periph/S00_ACLK] [get_bd_pins rst_clk_wiz_0_100M/slowest_sync_clk]
  connect_bd_net -net clk_wiz_0_locked [get_bd_pins clk_wiz_0/locked] [get_bd_pins patmos_top_0/locked] [get_bd_pins rst_clk_wiz_0_100M/dcm_locked]
  connect_bd_net -net clk_wiz_1_clk_25 [get_bd_pins axi_ethernetlite_0/phy_rx_clk] [get_bd_pins axi_ethernetlite_0/phy_tx_clk] [get_bd_pins clk_wiz_1/clk_25] [get_bd_pins mii2rgmii_0/rgmii_clk]
  connect_bd_net -net mii2rgmii_0_RGMII_TXD [get_bd_ports rgmii_txd_1] [get_bd_pins mii2rgmii_0/rgmii_phy_txd]
  connect_bd_net -net mii2rgmii_0_TXC [get_bd_ports rgmii_txc_1] [get_bd_pins mii2rgmii_0/rgmii_phy_txc]
  connect_bd_net -net mii2rgmii_0_TX_CTL [get_bd_ports rgmii_tx_ctl_1] [get_bd_pins mii2rgmii_0/rgmii_phy_tx_ctl]
  connect_bd_net -net mii2rgmii_0_phy_col [get_bd_pins axi_ethernetlite_0/phy_col] [get_bd_pins mii2rgmii_0/mii_phy_col]
  connect_bd_net -net mii2rgmii_0_phy_crs [get_bd_pins axi_ethernetlite_0/phy_crs] [get_bd_pins mii2rgmii_0/mii_phy_crs]
  connect_bd_net -net mii2rgmii_0_phy_dv [get_bd_pins axi_ethernetlite_0/phy_dv] [get_bd_pins mii2rgmii_0/mii_phy_dv]
  connect_bd_net -net mii2rgmii_0_phy_rx_data [get_bd_pins axi_ethernetlite_0/phy_rx_data] [get_bd_pins mii2rgmii_0/mii_phy_rx_data]
  connect_bd_net -net mii2rgmii_0_phy_rx_er [get_bd_pins axi_ethernetlite_0/phy_rx_er] [get_bd_pins mii2rgmii_0/mii_phy_rx_er]
  connect_bd_net -net oUartPins_txd_1 [get_bd_ports oUartPins_txd] [get_bd_pins patmos_top_0/iUartPins_rxd]
  connect_bd_net -net patmos_top_0_led [get_bd_ports led] [get_bd_pins patmos_top_0/led]
  connect_bd_net -net patmos_top_0_oUartPins_txd [get_bd_ports iUartPins_rxd] [get_bd_pins patmos_top_0/oUartPins_txd]
  connect_bd_net -net rgmii_rx_ctl_1_1 [get_bd_ports rgmii_rx_ctl_1] [get_bd_pins mii2rgmii_0/rgmii_phy_rx_ctl]
  connect_bd_net -net rgmii_rxc_1_1 [get_bd_ports rgmii_rxc_1] [get_bd_pins mii2rgmii_0/rgmii_phy_rxc]
  connect_bd_net -net rgmii_rxd_1_1 [get_bd_ports rgmii_rxd_1] [get_bd_pins mii2rgmii_0/rgmii_phy_rxd]
  connect_bd_net -net rst_clk_wiz_0_100M_peripheral_aresetn [get_bd_pins axi_ethernetlite_0/s_axi_aresetn] [get_bd_pins axi_gpio_0/s_axi_aresetn] [get_bd_pins patmos_top_0_axi_periph/ARESETN] [get_bd_pins patmos_top_0_axi_periph/M00_ARESETN] [get_bd_pins patmos_top_0_axi_periph/S00_ARESETN] [get_bd_pins rst_clk_wiz_0_100M/peripheral_aresetn]

  # Create address segments
  assign_bd_address -offset 0x00000000 -range 0x00001000 -target_address_space [get_bd_addr_spaces patmos_top_0/m_axi] [get_bd_addr_segs axi_gpio_0/S_AXI/Reg] -force


  # Restore current instance
  current_bd_instance $oldCurInst

  validate_bd_design
  save_bd_design
}
# End of create_root_design()


##################################################################
# MAIN FLOW
##################################################################

create_root_design ""


