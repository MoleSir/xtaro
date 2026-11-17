
.SUBCKT nand_2_x2
+ A0 A1 Z vdd gnd
Mnand_pmos0
+ Z A0 vdd vdd
+ PMOS_VTG w=0.40u l=0.05u
Mnand_pmos1
+ Z A1 vdd vdd
+ PMOS_VTG w=0.40u l=0.05u
Mnand_nmos0
+ Z A0 net0 gnd
+ NMOS_VTG w=0.20u l=0.05u
Mnand_nmos1
+ net0 A1 gnd gnd
+ NMOS_VTG w=0.20u l=0.05u
.ENDS nand_2_x2
