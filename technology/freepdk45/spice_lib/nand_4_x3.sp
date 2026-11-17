
.SUBCKT nand_4_x3
+ A0 A1 A2 A3 Z vdd gnd
Mnand_pmos0
+ Z A0 vdd vdd
+ PMOS_VTG w=0.60u l=0.05u
Mnand_pmos1
+ Z A1 vdd vdd
+ PMOS_VTG w=0.60u l=0.05u
Mnand_pmos2
+ Z A2 vdd vdd
+ PMOS_VTG w=0.60u l=0.05u
Mnand_pmos3
+ Z A3 vdd vdd
+ PMOS_VTG w=0.60u l=0.05u
Mnand_nmos0
+ Z A0 net0 gnd
+ NMOS_VTG w=0.30u l=0.05u
Mnand_nmos1
+ net0 A1 net1 gnd
+ NMOS_VTG w=0.30u l=0.05u
Mnand_nmos2
+ net1 A2 net2 gnd
+ NMOS_VTG w=0.30u l=0.05u
Mnand_nmos3
+ net2 A3 gnd gnd
+ NMOS_VTG w=0.30u l=0.05u
.ENDS nand_4_x3
