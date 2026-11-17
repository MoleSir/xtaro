.SUBCKT precharge bl br en_bar vdd
MM1 vdd en_bar bl vdd PMOS_VTG w=0.27u l=0.05u
MM2 vdd en_bar br vdd PMOS_VTG w=0.27u l=0.05u
MM3 bl en_bar br vdd PMOS_VTG w=0.27u l=0.05u
.ENDS precharge
