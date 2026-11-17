* block symbol definitions
.subckt _32x8_sram A[0] A[1] A[2] A[3] A[4] clk csb din0 din1 din2 din3 din4 din5 din6 din7 we dout0 dout1 dout2 dout3 dout4 dout5 dout6 dout7
XX1 N035 N037 N039 N041 NC_01 NC_02 NC_03 NC_04 N052 N051 NC_05 NC_06 NC_07 NC_08 N004 N003 N006 N005 N008 N007 N010 N009 N012 N011 N014 N043 N045 N047 N049 N054 N053 N056 N055 N058 N057 N060 N059 N062 N061 N064 N063 N066 N065 N013 N016 N015 N018 N017 8x8_array
XX2 N146 N148 N150 N152 NC_09 NC_10 NC_11 NC_12 N167 N166 NC_13 NC_14 NC_15 NC_16 N115 N114 N117 N116 N119 N118 N121 N120 N123 N122 N125 N154 N156 N158 N160 N169 N168 N171 N170 N173 N172 N175 N174 N177 N176 N179 N178 N181 N180 N124 N127 N126 N129 N128 8x8_array
XX3 NC_17 NC_18 NC_19 NC_20 N036 N038 N040 N042 N069 N068 N044 N046 N048 N050 N020 N019 N022 N021 N024 N023 N026 N025 N028 N027 N030 NC_21 NC_22 NC_23 NC_24 N071 N070 N073 N072 N075 N074 N077 N076 N079 N078 N081 N080 N083 N082 N029 N032 N031 N034 N033 8x8_array
XX4 NC_25 NC_26 NC_27 NC_28 N147 N149 N151 N153 N184 N183 N155 N157 N159 N161 N131 N130 N133 N132 N135 N134 N137 N136 N139 N138 N141 NC_29 NC_30 NC_31 NC_32 N186 N185 N188 N187 N190 N189 N192 N191 N194 N193 N196 N195 N198 N197 N140 N143 N142 N145 N144 8x8_array
XX5 N051 N053 N055 N057 N059 N061 N063 N065 N052 N054 N056 N058 N060 N062 N064 N066 NC_33 N091 N095 N099 N103 N093 N097 NC_34 N105 N086 N084 NC_35 column_mux
XX6 N114 N116 N118 N120 N122 N124 N126 N128 N115 N117 N119 N121 N123 N125 N127 N129 NC_36 N091 N095 N099 N103 N093 N097 N101 N105 N110 N112 NC_37 column_mux
XX7 N001 N003 N004 N005 N006 N007 N008 N009 N010 N011 N012 N013 N014 N015 N016 N017 N018 N002 precharge_array
XX8 N201 N166 N167 N168 N169 N170 N171 N172 N173 N174 N175 N176 N177 N178 N179 N180 N181 N202 precharge_array
XX9 NC_38 N206 N207 N208 N209 N210 N211 N212 N213 N091 N093 N095 N097 N099 N101 N103 N105 dout0 dout1 dout2 dout3 N204 sense_amp_array
XX11 N163 N164 N165 N147 N149 N151 N153 N155 N157 N159 N161 N162 N146 N148 N150 N152 N154 N156 N158 N160 row_decoder
XX12 N163 N164 N165 N036 N038 N040 N042 N044 N046 N048 N050 N067 N035 N037 N039 N041 N043 N045 N047 N049 row_decoder
XX13 N068 N070 N072 N074 N076 N078 N080 N082 N069 N071 N073 N075 N077 N079 N081 N083 N087 N092 N096 N100 N104 N094 N098 NC_39 N106 NC_40 NC_41 N085 column_mux
XX14 N130 N132 N134 N136 N138 N140 N142 N144 N131 N133 N135 N137 N139 N141 N143 N145 N111 N092 N096 N100 N104 N094 N098 N102 N106 NC_42 NC_43 N113 column_mux
XX15 N205 N214 N215 N216 N217 N218 N219 N220 N221 N092 N094 N096 N098 N100 N102 N104 N106 dout4 dout5 dout6 dout7 NC_44 sense_amp_array
XX16 N233 N234 N235 N236 N223 N214 N216 N218 N220 N215 N217 N219 N221 NC_45 write_driver_array
XX17 N202 N183 N184 N185 N186 N187 N188 N189 N190 N191 N192 N193 N194 N195 N196 N197 N198 NC_46 precharge_array
XX18 N002 N019 N020 N021 N022 N023 N024 N025 N026 N027 N028 N029 N030 N031 N032 N033 N034 NC_47 precharge_array
XX19 N001 N089 N090 N088 replica_array
XX20 N201 N108 N107 N109 replica_array
XX21 NC_48 NC_49 NC_50 NC_51 replica_array
XX22 NC_52 NC_53 NC_54 NC_55 replica_array
XX23 N203 N231 N200 N199 N086 N002 N232 N182 N088 N109 N084 N112 N089 N108 N067 N107 N090 N110 N085 N087 N113 N111 N162 N202 array_seclector
XX24 N237 din0 din1 din2 din3 N224 N225 N226 N227 NC_56 din_dffs
XX25 NC_57 din4 din5 din6 din7 N233 N234 N235 N236 N238 din_dffs
XX26 N228 N229 N182 N230 N203 N204 N200 N199 N205 N222 N223 logic_control
XX27 A[0] A[1] A[2] A[3] A[4] clk csb we N232 N231 N165 N164 N163 N229 N230 N237 N238 N228 control_dffs
XX10 N224 N225 N226 N227 NC_58 N206 N208 N210 N212 N207 N209 N211 N213 N222 write_driver_array
.ends _32x8_sram

.subckt 8x8_array wl0 wl1 wl2 wl3 wl0 wl1 wl2 wl3 br0 bl0 wl4 wl5 wl6 wl7 br0 bl0 br1 bl1 br2 bl2 br3 bl3 br4 bl4 br5 wl4 wl5 wl6 wl7 br1 bl1 br2 bl2 br3 bl3 br4 bl4 br5 bl5 br6 bl6 br7 bl7 bl5 br6 bl6 br7 bl7
XX1 wl0 bl0 br0 cell_1rw
XX2 wl0 bl1 br1 cell_1rw
XX3 wl0 bl2 br2 cell_1rw
XX4 wl0 bl3 br3 cell_1rw
XX5 wl0 bl4 br4 cell_1rw
XX6 wl0 bl5 br5 cell_1rw
XX7 wl0 bl6 br6 cell_1rw
XX8 wl0 bl7 br7 cell_1rw
XX9 wl1 bl0 br0 cell_1rw
XX10 wl1 bl1 br1 cell_1rw
XX11 wl1 bl2 br2 cell_1rw
XX12 wl1 bl3 br3 cell_1rw
XX13 wl1 bl4 br4 cell_1rw
XX14 wl1 bl5 br5 cell_1rw
XX15 wl1 bl6 br6 cell_1rw
XX16 wl1 bl7 br7 cell_1rw
XX17 wl2 bl0 br0 cell_1rw
XX18 wl2 bl1 br1 cell_1rw
XX19 wl2 bl2 br2 cell_1rw
XX20 wl2 bl3 br3 cell_1rw
XX21 wl2 bl4 br4 cell_1rw
XX22 wl2 bl5 br5 cell_1rw
XX23 wl2 bl6 br6 cell_1rw
XX24 wl2 bl7 br7 cell_1rw
XX25 wl3 bl0 br0 cell_1rw
XX26 wl3 bl1 br1 cell_1rw
XX27 wl3 bl2 br2 cell_1rw
XX28 wl3 bl3 br3 cell_1rw
XX29 wl3 bl4 br4 cell_1rw
XX30 wl3 bl5 br5 cell_1rw
XX31 wl3 bl6 br6 cell_1rw
XX32 wl3 bl7 br7 cell_1rw
XX33 wl4 bl0 br0 cell_1rw
XX34 wl4 bl1 br1 cell_1rw
XX35 wl4 bl2 br2 cell_1rw
XX36 wl4 bl3 br3 cell_1rw
XX37 wl4 bl4 br4 cell_1rw
XX38 wl4 bl5 br5 cell_1rw
XX39 wl4 bl6 br6 cell_1rw
XX40 wl4 bl7 br7 cell_1rw
XX41 wl5 bl0 br0 cell_1rw
XX42 wl5 bl1 br1 cell_1rw
XX43 wl5 bl2 br2 cell_1rw
XX44 wl5 bl3 br3 cell_1rw
XX45 wl5 bl4 br4 cell_1rw
XX46 wl5 bl5 br5 cell_1rw
XX47 wl5 bl6 br6 cell_1rw
XX48 wl5 bl7 br7 cell_1rw
XX49 wl6 bl0 br0 cell_1rw
XX50 wl6 bl1 br1 cell_1rw
XX51 wl6 bl2 br2 cell_1rw
XX52 wl6 bl3 br3 cell_1rw
XX53 wl6 bl4 br4 cell_1rw
XX54 wl6 bl5 br5 cell_1rw
XX55 wl6 bl6 br6 cell_1rw
XX56 wl6 bl7 br7 cell_1rw
XX57 wl7 bl0 br0 cell_1rw
XX58 wl7 bl1 br1 cell_1rw
XX59 wl7 bl2 br2 cell_1rw
XX60 wl7 bl3 br3 cell_1rw
XX61 wl7 bl4 br4 cell_1rw
XX62 wl7 bl5 br5 cell_1rw
XX63 wl7 bl6 br6 cell_1rw
XX64 wl7 bl7 br7 cell_1rw
.ends 8x8_array

.subckt column_mux bl_0 bl_1 bl_2 bl_3 bl_4 bl_5 bl_6 bl_7 br_0 br_1 br_2 br_3 br_4 br_5 br_6 br_7 sel_0 blo_0 blo_1 blo_2 blo_3 bro_0 bro_1 bro_2 bro_3 sel_0 en en
XX1 bl_0 bl_1 br_0 br_1 sel_0 blo_0 bro_0 en mux21
XX2 bl_2 bl_3 br_2 br_3 sel_0 blo_1 bro_1 en mux21
XX3 bl_4 bl_5 br_4 br_5 sel_0 blo_2 bro_2 en mux21
XX4 bl_6 bl_7 br_6 br_7 sel_0 blo_3 bro_3 en mux21
.ends column_mux

.subckt precharge_array en_bar bl0 br0 bl1 br1 bl2 br2 bl3 br3 bl4 br4 bl5 br5 bl6 br6 bl7 br7 en_bar
XX1 en_bar bl0 br0 precharge
XX2 en_bar bl1 br1 precharge
XX3 en_bar bl2 br2 precharge
XX4 en_bar bl3 br3 precharge
XX5 en_bar bl4 br4 precharge
XX6 en_bar bl5 br5 precharge
XX7 en_bar bl6 br6 precharge
XX8 en_bar bl7 br7 precharge
.ends precharge_array

.subckt sense_amp_array en bl0 br0 bl1 br1 bl2 br2 bl3 br3 bl0 br0 bl1 br1 bl2 br2 bl3 br3 dout0 dout1 dout2 dout3 en
XX1 bl0 br0 en dout0 sense_amp
XX2 bl1 br1 en dout1 sense_amp
XX3 bl2 br2 en dout2 sense_amp
XX4 bl3 br3 en dout3 sense_amp
.ends sense_amp_array

.subckt row_decoder in_2 in_1 in_0 out_0 out_1 out_2 out_3 out_4 out_5 out_6 out_7 wl_en out_0 out_1 out_2 out_3 out_4 out_5 out_6 out_7
XX1 in_0 in_1 in_2 N001 N002 N003 N004 N005 N006 N007 N008 decoder38
XX2 wl_en N001 out_0 and2
XX3 wl_en N002 out_1 and2
XX4 wl_en N003 out_2 and2
XX5 wl_en N004 out_3 and2
XX6 wl_en N005 out_4 and2
XX7 wl_en N006 out_5 and2
XX8 wl_en N007 out_6 and2
XX9 wl_en N008 out_7 and2
.ends row_decoder

.subckt write_driver_array din0 din1 din2 din3 en bl0 bl1 bl2 bl3 br0 br1 br2 br3 en
XX1 bl1 br1 din1 en write_driver
XX2 bl3 br3 din3 en write_driver
XX3 bl2 br2 din2 en write_driver
XX4 bl0 br0 din0 en write_driver
.ends write_driver_array

.subckt replica_array p_en_bar we wl rbl
XX1 0 rbl N001 cell_1rw
XX2 0 rbl N001 cell_1rw
XX3 0 rbl N001 cell_1rw
XX4 0 rbl N001 cell_1rw
XX5 0 rbl N001 cell_1rw
XX6 wl rbl N001 cell_1rw
XX7 wl rbl N001 cell_1rw
XX8 wl rbl N001 cell_1rw
XX9 p_en_bar rbl N001 precharge
XX10 rbl N001 0 we write_driver
.ends replica_array

.subckt array_seclector p_en_bar sel_A[0] we_en wl_en A[0] p_en_bar_0 A[0] rbl rbl_0 rbl_1 sl_en_0 sl_en_1 we_en_0 we_en_1 wl_en_0 wl_en_1 wl_en_0 A[0] sl_en_0 A[0] sl_en_1 A[0] wl_en_1 p_en_bar_1
XX1 p_en_bar sl_en_0 p_en_bar_0 and2
XX2 p_en_bar sel_A[0] p_en_bar_1 and2
XX3 wl_en sl_en_0 wl_en_0 and2
XX4 wl_en sel_A[0] wl_en_1 and2
XX5 we_en sl_en_0 we_en_0 and2
XX6 we_en sel_A[0] we_en_1 and2
XX7 sl_en_0 sel_A[0] inv
XX8 rbl sel_A[0] rbl_0 rbl_1 tri_gate
XX9 sl_en_0 sl_en_1 inv
.ends array_seclector

.subckt din_dffs clk din0 din1 din2 din3 din0_r din1_r din2_r din3_r clk
XX1 clk din0 din0_r dff
XX2 clk din1 din1_r dff
XX3 clk din2 din2_r dff
XX4 clk din3 din3_r dff
.ends din_dffs

.subckt logic_control clk csb rbl we p_bar_en sa_en we_en wl_en sa_en we_en we_en
XX2 clk N002 inv
XX3 we N003 inv
XX4 csb N004 inv
XX8 we N004 we_en and2
XX1 clk N003 N004 p_bar_en nand3
XX5 rbl N001 inv
XX9 N003 N004 N005 and2
XX11 N001 N002 N006 and2
XX12 N006 N005 sa_en and2
XX6 we N004 N007 and2
XX7 N008 N007 wl_en nor2
XX10 rbl N002 N004 N008 and3
.ends logic_control

.subckt control_dffs A[0] A[1] A[2] A[3] A[4] clk csb we A[0]_r A[1]_r A[2]_r A[3]_r A[4]_r csb_r we_r clk clk clk
XX1 clk csb csb_r dff
XX2 clk we we_r dff
XX3 clk A[4] A[4]_r dff
XX4 clk A[3] A[3]_r dff
XX5 clk A[2] A[2]_r dff
XX6 clk A[1] A[1]_r dff
XX7 clk A[0] A[0]_r dff
.ends control_dffs

.subckt cell_1rw wl bl br
M1000 vdd q qr vdd p l=0.8u w=0.6u
M1001 vdd qr q vdd p l=0.8u w=0.6u
M1002 qr q 0 0 n l=0.4u w=1.6u
M1 q qr 0 0 n l=0.4u w=1.6u
vdd vdd 0 3.3
M1004 br wl qr 0 n l=0.4u w=0.8u
M2 bl wl q 0 n l=0.4u w=0.8u
.include models.sp
.ends cell_1rw

.subckt mux21 bl_0 bl_1 br_0 br_1 sel_0 bl br en
vdd vdd 0 3.3
XX1 sel_0 N001 inv
XX2 sb s bl_0 bl tri_gate
XX3 sb s br br_0 tri_gate
XX4 s sb br_1 br tri_gate
XX5 s sb bl bl_1 tri_gate
XX6 en sel_0 s and2
XX7 N001 en sb nand2
.include models.sp
.ends mux21

.subckt precharge en_bar bl br
Mlower_pmos br en_bar bl vdd p l=0.4u w=1.6u
Mlower_pmos1 vdd en_bar br vdd p l=0.4u w=9.6uu
Mlower_pmos2 vdd en_bar bl vdd p l=0.4u w=9.6u
vdd vdd 0 3.3
.include models.sp
.ends precharge

.subckt sense_amp bl br en dout
M1000 N002 en 0 0 n l=0.4u w=1.8u
M1001 dint N001 N002 0 n l=0.4u w=1.8u
M1002 N001 dint N002 0 n l=0.4u w=1.8u
M1008 dout_bar dint 0 0 n l=0.4u w=0.8u
M1010 dout dout_bar 0 0 n l=0.4u w=2.4u
M1003 vdd N001 dint vdd p l=0.4u w=3.6u
M1004 vdd dint N001 vdd p l=0.4u w=3.6u
M1009 vdd dout_bar dout vdd p l=0.4u w=4.8u
M1007 vdd dint dout_bar vdd p l=0.4u w=1.6u
vdd vdd 0 3.3
XX1 enb en dint bl tri_gate
XX2 enb en br N001 tri_gate
XX3 en enb inv
.include models.sp
.ends sense_amp

.subckt decoder38 in_0 in_1 in_2 out_0 out_1 out_2 out_3 out_4 out_5 out_6 out_7
XX1 N003 N002 N001 out_0 and3
XX2 in_0 N002 N001 out_1 and3
XX3 N003 in_1 N001 out_2 and3
XX4 in_0 in_1 N001 out_3 and3
XX5 N003 N002 in_2 out_4 and3
XX6 in_0 N002 in_2 out_5 and3
XX7 N003 in_1 in_2 out_6 and3
XX8 in_0 in_1 in_2 out_7 and3
XX9 in_2 N001 inv
XX10 in_1 N002 inv
XX11 in_0 N003 inv
.ends decoder38

.subckt and2 A B Z
XX2 N001 Z inv
XX1 A B N001 nand2
.ends and2

.subckt write_driver bl br data we
vdd vdd 0 3.3
XX5 we web inv
XX6 data N001 inv
XX7 N001 N002 inv
XX1 we web br N001 tri_gate
XX2 we web N002 bl tri_gate
.include models.sp
.ends write_driver

.subckt inv A Z
M1 Z A 0 0 n l=0.4u w=1.6u
Mpinv_pmos vdd A Z vdd p l=0.4u w=3.2u
vdd vdd 0 3.3
.include models.sp
.ends inv

.subckt tri_gate en enb x y
M1 x enb y vdd p l=0.4u w=4.8u
M2 x en y 0 n l=0.4u w=2.4u
vdd vdd 0 3.3
.include models.sp
.ends tri_gate

.subckt dff clk D Q
XX1 N002 clk N004 N007 tri_gate
XX2 clk N002 N004 N003 tri_gate
XX3 N001 N003 inv
XX4 N004 N001 inv
XX5 D N007 inv
XX6 clk N002 N006 N008 tri_gate
XX7 N002 clk N006 N005 tri_gate
XX8 Q N005 inv
XX9 N006 Q inv
XX10 N001 N008 inv
XX11 clk N002 inv
.ends dff

.subckt nand3 A B C Z
Mpnand3_pmos1 vdd A Z vdd p l=0.4u w=3.2u
Mpnand3_pmos2 vdd B Z vdd p l=0.4u w=3.2u
Mpnand3_pmos3 vdd C Z vdd p l=0.4u w=3.2u
Mpnand3_nmos1 Z A N001 0 n l=0.4u w=1.6u
Mpnand3_nmos2 N001 B N002 0 n l=0.4u w=1.6u
Mpnand3_nmos3 N002 C 0 0 n l=0.4u w=1.6u
vdd vdd 0 3.3
.include models.sp
.ends nand3

.subckt nor2 A B Z
vdd vdd 0 3.3
Mpnand1 Z A 0 0 n l=0.4u w=1.6u
Mpnand2 Z B 0 0 n l=0.4u w=1.6u
Mpnand7 N001 B Z vdd p l=0.4u w=3.2u
Mpnand8 vdd A N001 vdd p l=0.4u w=3.2u
.include models.sp
.ends nor2

.subckt and3 A B C Z
XX1 A B C N001 nand3
XX2 N001 Z inv
.ends and3

.subckt nand2 A B Z
Mpnand3_pmos1 vdd A Z vdd p l=0.4u w=3.2u
Mpnand3_pmos2 vdd B Z vdd p l=0.4u w=3.2u
Mpnand3_nmos1 Z A N001 0 n l=0.4u w=1.6u
Mpnand3_nmos2 N001 B 0 0 n l=0.4u w=1.6u
vdd vdd 0 3.3
.include models.sp
.ends nand2

