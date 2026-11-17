.SUBCKT inv_x3 A Z vdd gnd
Minv_nmos
+ Z A gnd gnd
+ NMOS_VTG w=0.30u l=0.05u
Minv_pmos
+ Z A vdd vdd
+ PMOS_VTG w=0.60u l=0.05u
.ENDS inv_x3
