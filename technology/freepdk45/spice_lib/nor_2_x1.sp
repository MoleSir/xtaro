
.SUBCKT nor_2_x1
+ A0 A1 Z vdd gnd
Mnor_pmos0
+ Z A0 net0 vdd
+PMOS_VTG w=0.20u l=0.05u
Mnor_pmos1
+ net0 A1 vdd vdd
+PMOS_VTG w=0.20u l=0.05u
Mnor_nmos0
+ Z A0 gnd gnd
+NMOS_VTG w=0.10u l=0.05u
Mnor_nmos1
+ Z A1 gnd gnd
+NMOS_VTG w=0.10u l=0.05u
.ENDS nor_2_x1
