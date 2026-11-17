.SUBCKT tri_gate in out en en_bar vdd gnd
M1000 in en_bar out vdd pmos_vtg w=180.0n l=50.0n
M1002 in en out gnd nmos_vtg w=90.0n l=50.0n
.ENDS
