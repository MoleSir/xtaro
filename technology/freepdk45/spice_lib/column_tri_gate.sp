.subckt column_tri_gate bl br bl_out br_out sel vdd gnd

* for bl
M1000 bl sel_bar bl_out vdd pmos_vtg w=180.0n l=50.0n
M1001 bl sel bl_out gnd nmos_vtg w=90.0n l=50.0n

* for br
M1002 br sel_bar br_out vdd pmos_vtg w=180.0n l=50.0n
M1003 br sel br_out gnd nmos_vtg w=90.0n l=50.0n

* inv
Minv_nmos sel_bar sel gnd gnd nmos_vtg w=0.10u l=0.05u
Minv_pmos sel_bar sel vdd vdd pmos_vtg w=0.20u l=0.05u

.ends column_tri_gate
