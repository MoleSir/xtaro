
.SUBCKT and_3_x1 A0 A1 A2 Z vdd gnd

Mnand_pmos0
+ Z_bar A0 vdd vdd
+ PMOS_VTG w=0.20u l=0.05u
Mnand_pmos1
+ Z_bar A1 vdd vdd
+ PMOS_VTG w=0.20u l=0.05u
Mnand_pmos2
+ Z_bar A2 vdd vdd
+ PMOS_VTG w=0.20u l=0.05u
Mnand_nmos0
+ Z_bar A0 net0 gnd
+ NMOS_VTG w=0.10u l=0.05u
Mnand_nmos1
+ net0 A1 net1 gnd
+ NMOS_VTG w=0.10u l=0.05u
Mnand_nmos2
+ net1 A2 gnd gnd
+ NMOS_VTG w=0.10u l=0.05u

Minv_nmos
+ Z Z_bar gnd gnd
+ NMOS_VTG w=0.10u l=0.05u
Minv_pmos
+ Z Z_bar vdd vdd
+ PMOS_VTG w=0.20u l=0.05u

.ENDS nand_3_x1
