

set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]

set_property CLOCK_DEDICATED_ROUTE BACKBONE [get_nets clk_manager_inst_0/inst/clk_in_clk_manager]
#set_property CLOCK_DEDICATED_ROUTE BACKBONE [get_nets u_clk_manager/inst/clk_in_clk_manager]

## Clock Signal
set_property -dict {PACKAGE_PIN AA3 IOSTANDARD LVDS} [get_ports clk_in_n]
set_property -dict {PACKAGE_PIN AA2 IOSTANDARD LVDS} [get_ports clk_in_p]

## Reset button (BTN4)
set_property -dict {PACKAGE_PIN AA8 IOSTANDARD LVCMOS18} [get_ports reset]

##DDR3 SDRAM
set_property -dict {PACKAGE_PIN AE5 IOSTANDARD SSTL15_T_DCI} [get_ports ddr3_dq[0]]
set_property -dict {PACKAGE_PIN AE3 IOSTANDARD SSTL15_T_DCI} [get_ports ddr3_dq[1]]
set_property -dict {PACKAGE_PIN AD4 IOSTANDARD SSTL15_T_DCI} [get_ports ddr3_dq[2]]
set_property -dict {PACKAGE_PIN AF3 IOSTANDARD SSTL15_T_DCI} [get_ports ddr3_dq[3]]
set_property -dict {PACKAGE_PIN AE1 IOSTANDARD SSTL15_T_DCI} [get_ports ddr3_dq[4]]
set_property -dict {PACKAGE_PIN AF2 IOSTANDARD SSTL15_T_DCI} [get_ports ddr3_dq[5]]
set_property -dict {PACKAGE_PIN AD1 IOSTANDARD SSTL15_T_DCI} [get_ports ddr3_dq[6]]
set_property -dict {PACKAGE_PIN AE2 IOSTANDARD SSTL15_T_DCI} [get_ports ddr3_dq[7]]
set_property -dict {PACKAGE_PIN Y3 IOSTANDARD SSTL15} [get_ports ddr3_addr[0]]
set_property -dict {PACKAGE_PIN Y2 IOSTANDARD SSTL15} [get_ports ddr3_addr[1]]
set_property -dict {PACKAGE_PIN W3 IOSTANDARD SSTL15} [get_ports ddr3_addr[2]]
set_property -dict {PACKAGE_PIN W5 IOSTANDARD SSTL15} [get_ports ddr3_addr[3]]
set_property -dict {PACKAGE_PIN AB2 IOSTANDARD SSTL15} [get_ports ddr3_addr[4]]
set_property -dict {PACKAGE_PIN W1 IOSTANDARD SSTL15} [get_ports ddr3_addr[5]]
set_property -dict {PACKAGE_PIN AC2 IOSTANDARD SSTL15} [get_ports ddr3_addr[6]]
set_property -dict {PACKAGE_PIN U2 IOSTANDARD SSTL15} [get_ports ddr3_addr[7]]
set_property -dict {PACKAGE_PIN AB1 IOSTANDARD SSTL15} [get_ports ddr3_addr[8]]
set_property -dict {PACKAGE_PIN V1 IOSTANDARD SSTL15} [get_ports ddr3_addr[9]]
set_property -dict {PACKAGE_PIN AD6 IOSTANDARD SSTL15} [get_ports ddr3_addr[10]]
set_property -dict {PACKAGE_PIN Y1 IOSTANDARD SSTL15} [get_ports ddr3_addr[11]]
set_property -dict {PACKAGE_PIN AC3 IOSTANDARD SSTL15} [get_ports ddr3_addr[12]]
set_property -dict {PACKAGE_PIN V2 IOSTANDARD SSTL15} [get_ports ddr3_addr[13]]
set_property -dict {PACKAGE_PIN AC1 IOSTANDARD SSTL15} [get_ports ddr3_addr[14]]
set_property -dict {PACKAGE_PIN AD5 IOSTANDARD SSTL15} [get_ports ddr3_addr[15]]
set_property -dict {PACKAGE_PIN AA5 IOSTANDARD SSTL15} [get_ports ddr3_ba[0]]
set_property -dict {PACKAGE_PIN AC4 IOSTANDARD SSTL15} [get_ports ddr3_ba[1]]
set_property -dict {PACKAGE_PIN V4 IOSTANDARD SSTL15} [get_ports ddr3_ba[2]]
set_property -dict {PACKAGE_PIN Y6 IOSTANDARD SSTL15} [get_ports ddr3_ras_n]
set_property -dict {PACKAGE_PIN Y5 IOSTANDARD SSTL15} [get_ports ddr3_cas_n]
set_property -dict {PACKAGE_PIN U5 IOSTANDARD SSTL15} [get_ports ddr3_we_n]
set_property -dict {PACKAGE_PIN U1 IOSTANDARD LVCMOS15} [get_ports ddr3_reset_n]
set_property -dict {PACKAGE_PIN AB5 IOSTANDARD SSTL15} [get_ports ddr3_cke]
set_property -dict {PACKAGE_PIN U7 IOSTANDARD SSTL15} [get_ports ddr3_odt]
set_property -dict {PACKAGE_PIN U6 IOSTANDARD SSTL15} [get_ports ddr3_cs_n]
set_property -dict {PACKAGE_PIN AE6 IOSTANDARD SSTL15} [get_ports ddr3_dm]
set_property -dict {PACKAGE_PIN AF5 IOSTANDARD DIFF_SSTL15_T_DCI} [get_ports ddr3_dqs_p]
set_property -dict {PACKAGE_PIN AF4 IOSTANDARD DIFF_SSTL15_T_DCI} [get_ports ddr3_dqs_n]
set_property -dict {PACKAGE_PIN AA4 IOSTANDARD DIFF_SSTL15} [get_ports ddr3_dqs_p]
set_property -dict {PACKAGE_PIN AB4 IOSTANDARD DIFF_SSTL15} [get_ports ddr3_dqs_n]

#QDRII+
set_property -dict {PACKAGE_PIN V8 IOSTANDARD HSTL_I} [get_ports qdriip_d[0]]
set_property -dict {PACKAGE_PIN V7 IOSTANDARD HSTL_I} [get_ports qdriip_d[1]]
set_property -dict {PACKAGE_PIN W9 IOSTANDARD HSTL_I} [get_ports qdriip_d[2]]
set_property -dict {PACKAGE_PIN Y11 IOSTANDARD HSTL_I} [get_ports qdriip_d[3]]
set_property -dict {PACKAGE_PIN Y8 IOSTANDARD HSTL_I} [get_ports qdriip_d[4]]
set_property -dict {PACKAGE_PIN Y7 IOSTANDARD HSTL_I} [get_ports qdriip_d[5]]
set_property -dict {PACKAGE_PIN W10 IOSTANDARD HSTL_I} [get_ports qdriip_d[6]]
set_property -dict {PACKAGE_PIN Y10 IOSTANDARD HSTL_I} [get_ports qdriip_d[7]]
set_property -dict {PACKAGE_PIN V9 IOSTANDARD HSTL_I} [get_ports qdriip_d[8]]
set_property -dict {PACKAGE_PIN AF8 IOSTANDARD HSTL_I} [get_ports qdriip_d[9]]
set_property -dict {PACKAGE_PIN AE8 IOSTANDARD HSTL_I} [get_ports qdriip_d[10]]
set_property -dict {PACKAGE_PIN AF9 IOSTANDARD HSTL_I} [get_ports qdriip_d[11]]
set_property -dict {PACKAGE_PIN AF10 IOSTANDARD HSTL_I} [get_ports qdriip_d[12]]
set_property -dict {PACKAGE_PIN AE10 IOSTANDARD HSTL_I} [get_ports qdriip_d[13]]
set_property -dict {PACKAGE_PIN AD10 IOSTANDARD HSTL_I} [get_ports qdriip_d[14]]
set_property -dict {PACKAGE_PIN AD11 IOSTANDARD HSTL_I} [get_ports qdriip_d[15]]
set_property -dict {PACKAGE_PIN AF13 IOSTANDARD HSTL_I} [get_ports qdriip_d[16]]
set_property -dict {PACKAGE_PIN AE13 IOSTANDARD HSTL_I} [get_ports qdriip_d[17]]
set_property -dict {PACKAGE_PIN AA14 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[0]]
set_property -dict {PACKAGE_PIN AD14 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[1]]
set_property -dict {PACKAGE_PIN Y15 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[2]]
set_property -dict {PACKAGE_PIN AA15 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[3]]
set_property -dict {PACKAGE_PIN AC14 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[4]]
set_property -dict {PACKAGE_PIN AB14 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[5]]
set_property -dict {PACKAGE_PIN Y16 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[6]]
set_property -dict {PACKAGE_PIN AB15 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[7]]
set_property -dict {PACKAGE_PIN AC16 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[8]]
set_property -dict {PACKAGE_PIN AE20 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[9]]
set_property -dict {PACKAGE_PIN AD19 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[10]]
set_property -dict {PACKAGE_PIN AD18 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[11]]
set_property -dict {PACKAGE_PIN AC19 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[12]]
set_property -dict {PACKAGE_PIN AB20 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[13]]
set_property -dict {PACKAGE_PIN AA20 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[14]]
set_property -dict {PACKAGE_PIN AD20 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[15]]
set_property -dict {PACKAGE_PIN AC17 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[16]]
set_property -dict {PACKAGE_PIN AB17 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_q[17]]
set_property -dict {PACKAGE_PIN AC9 IOSTANDARD HSTL_I} [get_ports qdriip_sa[0]]
set_property -dict {PACKAGE_PIN AF7 IOSTANDARD HSTL_I} [get_ports qdriip_sa[1]]
set_property -dict {PACKAGE_PIN AA9 IOSTANDARD HSTL_I} [get_ports qdriip_sa[2]]
set_property -dict {PACKAGE_PIN AD8 IOSTANDARD HSTL_I} [get_ports qdriip_sa[3]]
set_property -dict {PACKAGE_PIN AC8 IOSTANDARD HSTL_I} [get_ports qdriip_sa[4]]
set_property -dict {PACKAGE_PIN AB7 IOSTANDARD HSTL_I} [get_ports qdriip_sa[5]]
set_property -dict {PACKAGE_PIN AB12 IOSTANDARD HSTL_I} [get_ports qdriip_sa[6]]
set_property -dict {PACKAGE_PIN AD13 IOSTANDARD HSTL_I} [get_ports qdriip_sa[7]]
set_property -dict {PACKAGE_PIN AC11 IOSTANDARD HSTL_I} [get_ports qdriip_sa[8]]
set_property -dict {PACKAGE_PIN AC12 IOSTANDARD HSTL_I} [get_ports qdriip_sa[9]]
set_property -dict {PACKAGE_PIN Y12 IOSTANDARD HSTL_I} [get_ports qdriip_sa[10]]
set_property -dict {PACKAGE_PIN AB11 IOSTANDARD HSTL_I} [get_ports qdriip_sa[11]]
set_property -dict {PACKAGE_PIN AB10 IOSTANDARD HSTL_I} [get_ports qdriip_sa[12]]
set_property -dict {PACKAGE_PIN AA13 IOSTANDARD HSTL_I} [get_ports qdriip_sa[13]]
set_property -dict {PACKAGE_PIN AC13 IOSTANDARD HSTL_I} [get_ports qdriip_sa[14]]
set_property -dict {PACKAGE_PIN Y13 IOSTANDARD HSTL_I} [get_ports qdriip_sa[15]]
set_property -dict {PACKAGE_PIN AA12 IOSTANDARD HSTL_I} [get_ports qdriip_sa[16]]
set_property -dict {PACKAGE_PIN AA10 IOSTANDARD HSTL_I} [get_ports qdriip_sa[17]]
set_property -dict {PACKAGE_PIN AB9 IOSTANDARD HSTL_I} [get_ports qdriip_sa[18]]
set_property -dict {PACKAGE_PIN AD9 IOSTANDARD HSTL_I} [get_ports qdriip_w_n]
set_property -dict {PACKAGE_PIN AE7 IOSTANDARD HSTL_I} [get_ports qdriip_r_n]
set_property -dict {PACKAGE_PIN AC7 IOSTANDARD HSTL_I} [get_ports qdriip_dll_off_n]
set_property -dict {PACKAGE_PIN W11 IOSTANDARD HSTL_I} [get_ports qdriip_bw_n[0]]
set_property -dict {PACKAGE_PIN V11 IOSTANDARD HSTL_I} [get_ports qdriip_bw_n[1]]
set_property -dict {PACKAGE_PIN AB16 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_cq_p]
set_property -dict {PACKAGE_PIN AC18 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_cq_n]
set_property -dict {PACKAGE_PIN AA19 IOSTANDARD HSTL_I_DCI} [get_ports qdriip_qvld]
set_property -dict {PACKAGE_PIN AE12 IOSTANDARD DIFF_HSTL_I} [get_ports qdriip_k_p]
set_property -dict {PACKAGE_PIN AF12 IOSTANDARD DIFF_HSTL_I} [get_ports qdriip_k_n]

#BPI Flash
set_property -dict {PACKAGE_PIN C8 IOSTANDARD LVCMOS33} [get_ports bpi_clk_out]
set_property -dict {PACKAGE_PIN L18 IOSTANDARD LVCMOS33} [get_ports bpi_we_n]
set_property -dict {PACKAGE_PIN M17 IOSTANDARD LVCMOS33} [get_ports bpi_oe_n]
set_property -dict {PACKAGE_PIN C23 IOSTANDARD LVCMOS33} [get_ports bpi_ce_n]
set_property -dict {PACKAGE_PIN D20 IOSTANDARD LVCMOS33} [get_ports bpi_adv]
set_property -dict {PACKAGE_PIN J23 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[0]]
set_property -dict {PACKAGE_PIN K23 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[1]]
set_property -dict {PACKAGE_PIN K22 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[2]]
set_property -dict {PACKAGE_PIN L22 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[3]]
set_property -dict {PACKAGE_PIN J25 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[4]]
set_property -dict {PACKAGE_PIN J24 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[5]]
set_property -dict {PACKAGE_PIN H22 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[6]]
set_property -dict {PACKAGE_PIN H24 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[7]]
set_property -dict {PACKAGE_PIN H23 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[8]]
set_property -dict {PACKAGE_PIN G21 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[9]]
set_property -dict {PACKAGE_PIN H21 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[10]]
set_property -dict {PACKAGE_PIN H26 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[11]]
set_property -dict {PACKAGE_PIN J26 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[12]]
set_property -dict {PACKAGE_PIN E26 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[13]]
set_property -dict {PACKAGE_PIN F25 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[14]]
set_property -dict {PACKAGE_PIN G26 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[15]]
set_property -dict {PACKAGE_PIN K17 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[16]]
set_property -dict {PACKAGE_PIN K16 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[17]]
set_property -dict {PACKAGE_PIN L20 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[18]]
set_property -dict {PACKAGE_PIN J19 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[19]]
set_property -dict {PACKAGE_PIN J18 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[20]]
set_property -dict {PACKAGE_PIN J20 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[21]]
set_property -dict {PACKAGE_PIN K20 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[22]]
set_property -dict {PACKAGE_PIN G20 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[23]]
set_property -dict {PACKAGE_PIN H19 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[24]]
set_property -dict {PACKAGE_PIN E20 IOSTANDARD LVCMOS33} [get_ports bpi_addr_cmd[25]]
set_property -dict {PACKAGE_PIN B24 IOSTANDARD LVCMOS33} [get_ports bpi_data[0]]
set_property -dict {PACKAGE_PIN A25 IOSTANDARD LVCMOS33} [get_ports bpi_data[1]]
set_property -dict {PACKAGE_PIN B22 IOSTANDARD LVCMOS33} [get_ports bpi_data[2]]
set_property -dict {PACKAGE_PIN A22 IOSTANDARD LVCMOS33} [get_ports bpi_data[3]]
set_property -dict {PACKAGE_PIN A23 IOSTANDARD LVCMOS33} [get_ports bpi_data[4]]
set_property -dict {PACKAGE_PIN A24 IOSTANDARD LVCMOS33} [get_ports bpi_data[5]]
set_property -dict {PACKAGE_PIN D26 IOSTANDARD LVCMOS33} [get_ports bpi_data[6]]
set_property -dict {PACKAGE_PIN C26 IOSTANDARD LVCMOS33} [get_ports bpi_data[7]]
set_property -dict {PACKAGE_PIN C24 IOSTANDARD LVCMOS33} [get_ports bpi_data[8]]
set_property -dict {PACKAGE_PIN D21 IOSTANDARD LVCMOS33} [get_ports bpi_data[9]]
set_property -dict {PACKAGE_PIN C22 IOSTANDARD LVCMOS33} [get_ports bpi_data[10]]
set_property -dict {PACKAGE_PIN B20 IOSTANDARD LVCMOS33} [get_ports bpi_data[11]]
set_property -dict {PACKAGE_PIN A20 IOSTANDARD LVCMOS33} [get_ports bpi_data[12]]
set_property -dict {PACKAGE_PIN E22 IOSTANDARD LVCMOS33} [get_ports bpi_data[13]]
set_property -dict {PACKAGE_PIN C21 IOSTANDARD LVCMOS33} [get_ports bpi_data[14]]
set_property -dict {PACKAGE_PIN B21 IOSTANDARD LVCMOS33} [get_ports bpi_data[15]]

#SD Card Connector
set_property -dict {PACKAGE_PIN AE15 IOSTANDARD LVCMOS18  PULLUP true } [get_ports { sd_cd }];
set_property -dict {PACKAGE_PIN AF15 IOSTANDARD LVCMOS18  PULLUP true } [get_ports { sd_wp }];
set_property -dict {PACKAGE_PIN AA18 IOSTANDARD LVCMOS18} [get_ports { sd_cclk }];
set_property -dict {PACKAGE_PIN AF18 IOSTANDARD LVCMOS18} [get_ports { sd_cmd }];
set_property -dict {PACKAGE_PIN AE17 IOSTANDARD LVCMOS18} [get_ports { sd_d[0]}];
set_property -dict {PACKAGE_PIN AF17 IOSTANDARD LVCMOS18} [get_ports { sd_d[1]}];
set_property -dict {PACKAGE_PIN AD15 IOSTANDARD LVCMOS18} [get_ports { sd_d[2]}];
set_property -dict {PACKAGE_PIN AE18 IOSTANDARD LVCMOS18} [get_ports { sd_d[3]}];

#PCI Express
set_property -dict {PACKAGE_PIN H2} [get_ports { pcie_rx_p[0]}];
set_property -dict {PACKAGE_PIN J4} [get_ports { pcie_tx_p[0]}];
set_property -dict {PACKAGE_PIN H1} [get_ports { pcie_rx_n[0]}];
set_property -dict {PACKAGE_PIN J3} [get_ports { pcie_tx_n[0]}];
set_property -dict {PACKAGE_PIN K2} [get_ports { pcie_rx_p[1]}];
set_property -dict {PACKAGE_PIN L4} [get_ports { pcie_tx_p[1]}];
set_property -dict {PACKAGE_PIN K1} [get_ports { pcie_rx_n[1]}];
set_property -dict {PACKAGE_PIN L3} [get_ports { pcie_tx_n[1]}];
set_property -dict {PACKAGE_PIN M2} [get_ports { pcie_rx_p[2]}];
set_property -dict {PACKAGE_PIN N4} [get_ports { pcie_tx_p[2]}];
set_property -dict {PACKAGE_PIN M1} [get_ports { pcie_rx_n[2]}];
set_property -dict {PACKAGE_PIN N3} [get_ports { pcie_tx_n[2]}];
set_property -dict {PACKAGE_PIN P2} [get_ports { pcie_rx_p[3]}];
set_property -dict {PACKAGE_PIN R4} [get_ports { pcie_tx_p[3]}];
set_property -dict {PACKAGE_PIN P1} [get_ports { pcie_rx_n[3]}];
set_property -dict {PACKAGE_PIN R3} [get_ports { pcie_tx_n[3]}];
set_property -dict {PACKAGE_PIN L17 IOSTANDARD LVCMOS33  PULLUP true NODELAY} [get_ports { pcie_perstn }];
set_property -dict {PACKAGE_PIN K18 IOSTANDARD LVCMOS33} [get_ports { pcie_wake}];
set_property -dict {PACKAGE_PIN AA7 IOSTANDARD LVCMOS18} [get_ports { pcie_prsnt}];

#Ethernet PHYS
set_property -dict {PACKAGE_PIN V13 IOSTANDARD LVCMOS18} [get_ports { mdc}];
set_property -dict {PACKAGE_PIN W13 IOSTANDARD LVCMOS18} [get_ports { mdio}];
set_property -dict {PACKAGE_PIN D18 IOSTANDARD LVCMOS33} [get_ports { phy_rstn_1}];
set_property -dict {PACKAGE_PIN E25 IOSTANDARD LVCMOS33} [get_ports { phy_rstn_2}];
set_property -dict {PACKAGE_PIN K21 IOSTANDARD LVCMOS33} [get_ports { phy_rstn_3}];
set_property -dict {PACKAGE_PIN L23 IOSTANDARD LVCMOS33} [get_ports { phy_rstn_4}];
set_property -dict {PACKAGE_PIN J8 IOSTANDARD LVCMOS18  PULLUP true } [get_ports { phy_intrn_1 }];
set_property -dict {PACKAGE_PIN J14 IOSTANDARD LVCMOS18  PULLUP true } [get_ports { phy_intrn_2 }];
set_property -dict {PACKAGE_PIN K15 IOSTANDARD LVCMOS18  PULLUP true } [get_ports { phy_intrn_3 }];
set_property -dict {PACKAGE_PIN M16 IOSTANDARD LVCMOS18  PULLUP true } [get_ports { phy_intrn_4 }];
set_property -dict {PACKAGE_PIN B11 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_1[0]}];
set_property -dict {PACKAGE_PIN A10 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_1[1]}];
set_property -dict {PACKAGE_PIN B10 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_1[2]}];
set_property -dict {PACKAGE_PIN A9 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_1[3]}];
set_property -dict {PACKAGE_PIN A8 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_1[0]}];
set_property -dict {PACKAGE_PIN D8 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_1[1]}];
set_property -dict {PACKAGE_PIN G9 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_1[2]}];
set_property -dict {PACKAGE_PIN H9 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_1[3]}];
set_property -dict {PACKAGE_PIN B12 IOSTANDARD LVCMOS18} [get_ports { rgmii_rx_ctl_1}];
set_property -dict {PACKAGE_PIN E10 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxc_1}];
set_property -dict {PACKAGE_PIN H8 IOSTANDARD LVCMOS18} [get_ports { rgmii_tx_ctl_1}];
set_property -dict {PACKAGE_PIN B9 IOSTANDARD LVCMOS18} [get_ports { rgmii_txc_1}];
set_property -dict {PACKAGE_PIN A13 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_2[0]}];
set_property -dict {PACKAGE_PIN C9 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_2[1]}];
set_property -dict {PACKAGE_PIN D11 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_2[2]}];
set_property -dict {PACKAGE_PIN C11 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_2[3]}];
set_property -dict {PACKAGE_PIN D10 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_2[0]}];
set_property -dict {PACKAGE_PIN G10 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_2[1]}];
set_property -dict {PACKAGE_PIN D9 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_2[2]}];
set_property -dict {PACKAGE_PIN F9 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_2[3]}];
set_property -dict {PACKAGE_PIN A12 IOSTANDARD LVCMOS18} [get_ports { rgmii_rx_ctl_2}];
set_property -dict {PACKAGE_PIN C12 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxc_2}];
set_property -dict {PACKAGE_PIN F8 IOSTANDARD LVCMOS18} [get_ports { rgmii_tx_ctl_2}];
set_property -dict {PACKAGE_PIN J10 IOSTANDARD LVCMOS18} [get_ports { rgmii_txc_2}];
set_property -dict {PACKAGE_PIN A14 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_3[0]}];
set_property -dict {PACKAGE_PIN B14 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_3[1]}];
set_property -dict {PACKAGE_PIN E12 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_3[2]}];
set_property -dict {PACKAGE_PIN D13 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_3[3]}];
set_property -dict {PACKAGE_PIN G12 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_3[0]}];
set_property -dict {PACKAGE_PIN F13 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_3[1]}];
set_property -dict {PACKAGE_PIN F12 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_3[2]}];
set_property -dict {PACKAGE_PIN H11 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_3[3]}];
set_property -dict {PACKAGE_PIN C13 IOSTANDARD LVCMOS18} [get_ports { rgmii_rx_ctl_3}];
set_property -dict {PACKAGE_PIN E11 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxc_3}];
set_property -dict {PACKAGE_PIN F10 IOSTANDARD LVCMOS18} [get_ports { rgmii_tx_ctl_3}];
set_property -dict {PACKAGE_PIN E13 IOSTANDARD LVCMOS18} [get_ports { rgmii_txc_3}];
set_property -dict {PACKAGE_PIN B15 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_4[0]}];
set_property -dict {PACKAGE_PIN F14 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_4[1]}];
set_property -dict {PACKAGE_PIN C14 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_4[2]}];
set_property -dict {PACKAGE_PIN H12 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxd_4[3]}];
set_property -dict {PACKAGE_PIN J13 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_4[0]}];
set_property -dict {PACKAGE_PIN G14 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_4[1]}];
set_property -dict {PACKAGE_PIN H14 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_4[2]}];
set_property -dict {PACKAGE_PIN H13 IOSTANDARD LVCMOS18} [get_ports { rgmii_txd_4[3]}];
set_property -dict {PACKAGE_PIN A15 IOSTANDARD LVCMOS18} [get_ports { rgmii_rx_ctl_4}];
set_property -dict {PACKAGE_PIN G11 IOSTANDARD LVCMOS18} [get_ports { rgmii_rxc_4}];
set_property -dict {PACKAGE_PIN J11 IOSTANDARD LVCMOS18} [get_ports { rgmii_tx_ctl_4}];
set_property -dict {PACKAGE_PIN D14 IOSTANDARD LVCMOS18} [get_ports { rgmii_txc_4}];

#PIC Interface
set_property -dict {PACKAGE_PIN AA17 IOSTANDARD LVCMOS18} [get_ports { pic2fpga_sck}];
set_property -dict {PACKAGE_PIN V16 IOSTANDARD LVCMOS18} [get_ports { pic2fpga_sdo}];
set_property -dict {PACKAGE_PIN W16 IOSTANDARD LVCMOS18} [get_ports { pic2fpga_ss_n}];
set_property -dict {PACKAGE_PIN W18 IOSTANDARD LVCMOS18} [get_ports { pic2fpga_gpi00}];
set_property -dict {PACKAGE_PIN V17 IOSTANDARD LVCMOS18} [get_ports { pic2fpga_gpi01}];
set_property -dict {PACKAGE_PIN W15 IOSTANDARD LVCMOS18} [get_ports { pic2fpga_sdi}];
set_property -dict {PACKAGE_PIN W14 IOSTANDARD LVCMOS18} [get_ports { fpga2pic_sck}];
set_property -dict {PACKAGE_PIN V14 IOSTANDARD LVCMOS18} [get_ports { fpga2pic_sdi}];
set_property -dict {PACKAGE_PIN V18 IOSTANDARD LVCMOS18} [get_ports { fpga2pic_ss_n}];
set_property -dict {PACKAGE_PIN V19 IOSTANDARD LVCMOS18} [get_ports { fpga2pic_sdo}];

#LEDs and Button I/O
set_property -dict {PACKAGE_PIN E17 IOSTANDARD LVCMOS33} [get_ports { led[0]}];
set_property -dict {PACKAGE_PIN AF14 IOSTANDARD LVCMOS18} [get_ports { led[1]}];
set_property -dict {PACKAGE_PIN F17 IOSTANDARD LVCMOS33} [get_ports { led[2]}];
set_property -dict {PACKAGE_PIN W19 IOSTANDARD LVCMOS18} [get_ports { led[3]}];

set_property -dict {PACKAGE_PIN W6 IOSTANDARD LVCMOS15} [get_ports { btn[0]}];
set_property -dict {PACKAGE_PIN E18 IOSTANDARD LVCMOS33} [get_ports { btn[1]}];
set_property -dict {PACKAGE_PIN AC6 IOSTANDARD LVCMOS15} [get_ports { btn[2]}];
set_property -dict {PACKAGE_PIN AB6 IOSTANDARD LVCMOS15} [get_ports { btn[3]}];

#PMOD Ports
set_property -dict {PACKAGE_PIN D19 IOSTANDARD LVCMOS33} [get_ports { pmod_ja_1}];
set_property -dict {PACKAGE_PIN E23 IOSTANDARD LVCMOS33} [get_ports { oUartPins_txd}]; #pmod_ja_2
set_property -dict {PACKAGE_PIN D25 IOSTANDARD LVCMOS33} [get_ports { iUartPins_rxd}]; #pmod_ja_3
set_property -dict {PACKAGE_PIN F23 IOSTANDARD LVCMOS33} [get_ports { pmod_ja_4}];
set_property -dict {PACKAGE_PIN F19 IOSTANDARD LVCMOS33} [get_ports { pmod_ja_7}];
set_property -dict {PACKAGE_PIN G22 IOSTANDARD LVCMOS33} [get_ports { pmod_ja_8}];
set_property -dict {PACKAGE_PIN D24 IOSTANDARD LVCMOS33} [get_ports { pmod_ja_9}];
set_property -dict {PACKAGE_PIN E21 IOSTANDARD LVCMOS33} [get_ports { pmod_ja_10}];
set_property -dict {PACKAGE_PIN F20 IOSTANDARD LVCMOS33} [get_ports { pmod_jb_1}];
set_property -dict {PACKAGE_PIN E15 IOSTANDARD LVCMOS33} [get_ports { pmod_jb_2}];
set_property -dict {PACKAGE_PIN H18 IOSTANDARD LVCMOS33} [get_ports { pmod_jb_3}];
set_property -dict {PACKAGE_PIN G19 IOSTANDARD LVCMOS33} [get_ports { pmod_jb_4}];
set_property -dict {PACKAGE_PIN H17 IOSTANDARD LVCMOS33} [get_ports { pmod_jb_7}];
set_property -dict {PACKAGE_PIN J21 IOSTANDARD LVCMOS33} [get_ports { pmod_jb_8}];
set_property -dict {PACKAGE_PIN L19 IOSTANDARD LVCMOS33} [get_ports { pmod_jb_9}];
set_property -dict {PACKAGE_PIN F18 IOSTANDARD LVCMOS33} [get_ports { pmod_jb_10}];

#FMC Connector
set_property -dict {PACKAGE_PIN AD16 IOSTANDARD LVCMOS18} [get_ports { VADJ_EN}];
set_property -dict {PACKAGE_PIN AF19 IOSTANDARD LVCMOS18} [get_ports { SET_VADJ1}];
set_property -dict {PACKAGE_PIN AF20 IOSTANDARD LVCMOS18} [get_ports { SET_VADJ2}];
set_property -dict {PACKAGE_PIN AF20 } [get_ports {FMC_LA00_N}];
set_property -dict {PACKAGE_PIN AA22 } [get_ports {FMC_LA00_N}]; 
set_property -dict {PACKAGE_PIN N21  } [get_ports {FMC_LA01_P}];
set_property -dict {PACKAGE_PIN N22  } [get_ports {FMC_LA01_N}];
set_property -dict {PACKAGE_PIN AB22 } [get_ports {FMC_LA02_P}]; 
set_property -dict {PACKAGE_PIN AC22 } [get_ports {FMC_LA02_N}]; 
set_property -dict {PACKAGE_PIN AF24 } [get_ports {FMC_LA03_P}]; 
set_property -dict {PACKAGE_PIN AF25 } [get_ports {FMC_LA03_N}]; 
set_property -dict {PACKAGE_PIN AA25 } [get_ports {FMC_LA04_P}]; 
set_property -dict {PACKAGE_PIN AB25 } [get_ports {FMC_LA04_N}]; 
set_property -dict {PACKAGE_PIN AE23 } [get_ports {FMC_LA05_P}]; 
set_property -dict {PACKAGE_PIN AF23 } [get_ports {FMC_LA05_N}]; 
set_property -dict {PACKAGE_PIN W20  } [get_ports {FMC_LA06_P}];
set_property -dict {PACKAGE_PIN Y21  } [get_ports {FMC_LA06_N}];
set_property -dict {PACKAGE_PIN AB26 } [get_ports {FMC_LA07_P}]; 
set_property -dict {PACKAGE_PIN AC26 } [get_ports {FMC_LA07_N}]; 
set_property -dict {PACKAGE_PIN AD26 } [get_ports {FMC_LA08_P}]; 
set_property -dict {PACKAGE_PIN AE26 } [get_ports {FMC_LA08_N}]; 
set_property -dict {PACKAGE_PIN Y25  } [get_ports {FMC_LA09_P}];
set_property -dict {PACKAGE_PIN Y26  } [get_ports {FMC_LA09_N}];
set_property -dict {PACKAGE_PIN W21  } [get_ports {FMC_LA10_P}];
set_property -dict {PACKAGE_PIN V21  } [get_ports {FMC_LA10_N}];
set_property -dict {PACKAGE_PIN W25  } [get_ports {FMC_LA11_P}];
set_property -dict {PACKAGE_PIN W26  } [get_ports {FMC_LA11_N}];
set_property -dict {PACKAGE_PIN W23  } [get_ports {FMC_LA12_P}];
set_property -dict {PACKAGE_PIN W24  } [get_ports {FMC_LA12_N}];
set_property -dict {PACKAGE_PIN U22  } [get_ports {FMC_LA13_P}];
set_property -dict {PACKAGE_PIN V22  } [get_ports {FMC_LA13_N}];
set_property -dict {PACKAGE_PIN R26  } [get_ports {FMC_LA14_P}];
set_property -dict {PACKAGE_PIN P26  } [get_ports {FMC_LA14_N}];
set_property -dict {PACKAGE_PIN T24  } [get_ports {FMC_LA15_P}];
set_property -dict {PACKAGE_PIN T25  } [get_ports {FMC_LA15_N}];
set_property -dict {PACKAGE_PIN V23  } [get_ports {FMC_LA16_P}];
set_property -dict {PACKAGE_PIN V24  } [get_ports {FMC_LA16_N}];
set_property -dict {PACKAGE_PIN R22  } [get_ports {FMC_LA17_P}];
set_property -dict {PACKAGE_PIN R23  } [get_ports {MC_LA17_N }]
set_property -dict {PACKAGE_PIN P23  } [get_ports {FMC_LA18_P}];
set_property -dict {PACKAGE_PIN N23  } [get_ports {FMC_LA18_N}];
set_property -dict {PACKAGE_PIN T22  } [get_ports {FMC_LA19_P}];
set_property -dict {PACKAGE_PIN T23  } [get_ports {FMC_LA19_N}];
set_property -dict {PACKAGE_PIN R25  } [get_ports {FMC_LA20_P}];
set_property -dict {PACKAGE_PIN P25  } [get_ports {FMC_LA20_N}];
set_property -dict {PACKAGE_PIN M24  } [get_ports {FMC_LA21_P}];
set_property -dict {PACKAGE_PIN L24  } [get_ports {FMC_LA21_N}];
set_property -dict {PACKAGE_PIN M25  } [get_ports {FMC_LA22_P}];
set_property -dict {PACKAGE_PIN L25  } [get_ports {FMC_LA22_N}];
set_property -dict {PACKAGE_PIN P24  } [get_ports {FMC_LA23_P}];
set_property -dict {PACKAGE_PIN N24  } [get_ports {FMC_LA23_N}];
set_property -dict {PACKAGE_PIN U17  } [get_ports {FMC_LA24_P}];
set_property -dict {PACKAGE_PIN T17  } [get_ports {FMC_LA24_N}];
set_property -dict {PACKAGE_PIN T18  } [get_ports {FMC_LA25_P}];
set_property -dict {PACKAGE_PIN T19  } [get_ports {FMC_LA25_N}];
set_property -dict {PACKAGE_PIN M21  } [get_ports {FMC_LA26_P}];
set_property -dict {PACKAGE_PIN M22 } [get_ports {FMC_LA26_N}];
set_property -dict {PACKAGE_PIN N26 } [get_ports {FMC_LA27_P}];
set_property -dict {PACKAGE_PIN M26 } [get_ports {FMC_LA27_N}];
set_property -dict {PACKAGE_PIN R16 } [get_ports {FMC_LA28_P}];
set_property -dict {PACKAGE_PIN R17 } [get_ports {FMC_LA28_N}];
set_property -dict {PACKAGE_PIN K25 } [get_ports {FMC_LA29_P}];
set_property -dict {PACKAGE_PIN K26 } [get_ports {FMC_LA29_N}];
set_property -dict {PACKAGE_PIN N19 } [get_ports {FMC_LA30_P}];
set_property -dict {PACKAGE_PIN M20 } [get_ports {FMC_LA30_N}];
set_property -dict {PACKAGE_PIN P19 } [get_ports {FMC_LA31_P}];
set_property -dict {PACKAGE_PIN P20 } [get_ports {FMC_LA31_N}];
set_property -dict {PACKAGE_PIN P16 } [get_ports {FMC_LA32_P}];
set_property -dict {PACKAGE_PIN N17 } [get_ports {FMC_LA32_N}];
set_property -dict {PACKAGE_PIN N18 } [get_ports {FMC_LA33_P}];
set_property -dict {PACKAGE_PIN M19 } [get_ports {FMC_LA33_N}];
set_property -dict {PACKAGE_PIN U19 } [get_ports {FMC_HA00_P}];
set_property -dict {PACKAGE_PIN U20 } [get_ports {FMC_HA00_N}];
set_property -dict {PACKAGE_PIN T20 } [get_ports {FMC_HA01_P}];
set_property -dict {PACKAGE_PIN R20 } [get_ports {FMC_HA01_N}];
set_property -dict {PACKAGE_PIN AD2 } [get_ports {FMC_HA02_P}];
set_property -dict {PACKAGE_PIN AD2 } [get_ports {FMC_HA02_N}];
set_property -dict {PACKAGE_PIN AB2 } [get_ports {FMC_HA03_P}];
set_property -dict {PACKAGE_PIN AC2 } [get_ports {FMC_HA03_N}];
set_property -dict {PACKAGE_PIN U24 } [get_ports {FMC_HA04_P}];
set_property -dict {PACKAGE_PIN U25 } [get_ports {FMC_HA04_N}];
set_property -dict {PACKAGE_PIN V26 } [get_ports {FMC_HA05_P}];
set_property -dict {PACKAGE_PIN U26 } [get_ports {FMC_HA05_N}];
set_property -dict {PACKAGE_PIN AD2 } [get_ports {FMC_HA06_P}];
set_property -dict {PACKAGE_PIN AE2 } [get_ports {FMC_HA06_N}];
set_property -dict {PACKAGE_PIN AD2 } [get_ports {FMC_HA07_P}];
set_property -dict {PACKAGE_PIN AE2 } [get_ports {FMC_HA07_N}];
set_property -dict {PACKAGE_PIN AE2 } [get_ports {FMC_HA08_P}];
set_property -dict {PACKAGE_PIN AF2 } [get_ports {FMC_HA08_N}];
set_property -dict {PACKAGE_PIN R18 } [get_ports {FMC_HA09_P}];
set_property -dict {PACKAGE_PIN P18 } [get_ports {FMC_HA09_N}];
set_property -dict {PACKAGE_PIN U16 } [get_ports {FMC_HA10_P}];
set_property -dict {PACKAGE_PIN N16 } [get_ports {FMC_HA10_N}];
set_property -dict {PACKAGE_PIN Y20 } [get_ports {FMC_HA11_P}];
set_property -dict {PACKAGE_PIN U21 } [get_ports {FMC_HA11_N}];
set_property -dict {PACKAGE_PIN P21 } [get_ports {FMC_CLK0_M2C_N}];
set_property -dict {PACKAGE_PIN R21 } [get_ports {FMC_CLK0_M2C_P}];
set_property -dict {PACKAGE_PIN AC24} [get_ports {FMC_CLK1_M2C_N}]; 
set_property -dict {PACKAGE_PIN AC23} [get_ports {FMC_CLK1_M2C_P}]; 
set_property -dict {PACKAGE_PIN AB24} [get_ports {FMC_CLK2_M2C_N}]; 
set_property -dict {PACKAGE_PIN AA23} [get_ports {FMC_CLK2_M2C_P}]; 
set_property -dict {PACKAGE_PIN AA24} [get_ports {FMC_CLK3_M2C_N}]; 
set_property -dict {PACKAGE_PIN Y23 } [get_ports {FMC_CLK3_M2C_P}];
set_property -dict {PACKAGE_PIN C3  } [get_ports {FMC_DP0_M2C_N}]; 
set_property -dict {PACKAGE_PIN C4  } [get_ports {FMC_DP0_M2C_P}]; 
set_property -dict {PACKAGE_PIN A3  } [get_ports {FMC_DP0_C2M_N}]; 
set_property -dict {PACKAGE_PIN A4  } [get_ports {FMC_DP0_C2M_P}]; 
set_property -dict {PACKAGE_PIN E3  } [get_ports {FMC_DP1_M2C_N}]; 
set_property -dict {PACKAGE_PIN E4  } [get_ports {FMC_DP1_M2C_P}]; 
set_property -dict {PACKAGE_PIN D1  } [get_ports {FMC_DP1_C2M_N}];
set_property -dict {PACKAGE_PIN D2  } [get_ports {FMC_DP1_C2M_P}];
set_property -dict {PACKAGE_PIN B5  } [get_ports {FMC_DP2_M2C_N}];
set_property -dict {PACKAGE_PIN B6  } [get_ports {FMC_DP2_M2C_P}];
set_property -dict {PACKAGE_PIN A3  } [get_ports {FMC_DP2_C2M_N}];
set_property -dict {PACKAGE_PIN A4  } [get_ports {FMC_DP2_C2M_P}];
set_property -dict {PACKAGE_PIN G3  } [get_ports {FMC_DP3_M2C_N}];
set_property -dict {PACKAGE_PIN G4  } [get_ports {FMC_DP3_M2C_P}];
set_property -dict {PACKAGE_PIN F1  } [get_ports {FMC_DP3_C2M_N}];
set_property -dict {PACKAGE_PIN F2  } [get_ports {FMC_DP3_C2M_P}];
set_property -dict {PACKAGE_PIN F5  } [get_ports {FMC_GBTCLK0_M2C_N}];
set_property -dict {PACKAGE_PIN F6  } [get_ports {FMC_GBTCLK0_M2C_P}];
set_property -dict {PACKAGE_PIN D5  } [get_ports {FMC_GBTCLK1_M2C_N}];
set_property -dict {PACKAGE_PIN D6  } [get_ports {FMC_GBTCLK1_M2C_P}];