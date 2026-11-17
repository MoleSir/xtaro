.SUBCKT inv_x0 A Z vdd gnd
Minv_nmos
+ Z A gnd gnd
+ NMOS_VTG w=0.05u l=0.05u
Minv_pmos
+ Z A vdd vdd
+ PMOS_VTG w=0.05u l=0.05u
.ENDS inv_x0
