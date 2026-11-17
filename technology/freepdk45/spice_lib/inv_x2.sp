.SUBCKT inv_x2 A Z vdd gnd
Minv_nmos
+ Z A gnd gnd
+ NMOS_VTG w=0.20u l=0.05u
Minv_pmos
+ Z A vdd vdd
+ PMOS_VTG w=0.40u l=0.05u
.ENDS inv_x2
