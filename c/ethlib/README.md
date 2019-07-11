# Ethernet Support on the DE2-115

This C code uses the Ethernet chip on the DE2-115 board.

Note that: The Ethernet controller supports MII mode, but not RGMII mode,
which means that jumper JP1 on the DE2-115 board needs to be placed
over pin 2 and 3. Also make sure that the Ethernet Interface on your PC
is set to 10 Mbits/s or 100 Mbits/s.

See also: http://orbit.dtu.dk/files/110841187/tr15_02_Pezzarossa_L.pdf
