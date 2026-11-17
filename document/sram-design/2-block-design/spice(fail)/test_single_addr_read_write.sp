
.include "./models.sp"
.include "./32x8_sram.sp"

Vvdd vdd 0 3.300000
.TEMP 25

# .subckt _32x8_sram A[0] A[1] A[2] A[3] A[4] clk csb din0 din1 din2 din3 din4 din5 din6 din7 we dout0 dout1 dout2 dout3 dout4 dout5 dout6 dout7
Xsram addr0 addr1 addr2 addr3 addr4 clk csb din0 din1 din2 din3 din4 din5 din6 din7 we dout0 dout1 dout2 dout3 dout4 dout5 dout6 dout7 _32x8_sram

Vclk clk 0 PULSE(0.000000 3.300000 0.000000n 0.100000n 0.100000n 4.900000n 10.000000n)

Vaddr0 addr0 0 PWL (0.000000n 0.000000v )
Vaddr1 addr1 0 PWL (0.000000n 0.000000v )
Vaddr2 addr2 0 PWL (0.000000n 0.000000v )
Vaddr3 addr3 0 PWL (0.000000n 0.000000v )
Vaddr4 addr4 0 PWL (0.000000n 0.000000v )
Vdin0 din0 0 PWL (0.000000n 3.300000v )
Vdin1 din1 0 PWL (0.000000n 0.000000v )
Vdin2 din2 0 PWL (0.000000n 3.300000v )
Vdin3 din3 0 PWL (0.000000n 0.000000v )
Vdin4 din4 0 PWL (0.000000n 3.300000v )
Vdin5 din5 0 PWL (0.000000n 0.000000v )
Vdin6 din6 0 PWL (0.000000n 3.300000v )
Vdin7 din7 0 PWL (0.000000n 0.000000v )
Vwe we 0 PWL (0.000000n 3.300000v 4.900000n 3.300000v 5.100000n 3.300000v 14.900000n 3.300000v 15.100000n 0.000000v )
Vcsb csb 0 PWL (0.000000n 3.300000v 2.900000n 3.300000v 3.100000n 0.000000v )
.TRAN 10.000000p 30.000000n 0.000000n
