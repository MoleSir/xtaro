# Cell Property

对于库单元：

- bitcell
- precharge
- column_tri_gate
- dff
- sense_amp
- write_driver
- 数字单元
    - inv
    - and
    - or
    - nand
    - nor

由于每个工艺库定义的端口名称可能不同（端口顺序目前稳定），所以需要每个工艺库自己指定！这里为了兼容，工艺库可以提供一份表示库单元引脚名称的配置文件。如果不提供则采用默认的方式：

- bitcell：bl br wl vdd gnd
- precharge：bl br en_bar vdd
- column_tri_gate：bl br bl_out br_out sel vdd gnd
- dff：D Q clk vdd gnd
- sense_amp：bl br dout en vdd gnd
- write_driver：din bl br en vdd gnd
- 数字单元
    - inv：A Z vdd gnd
    - and：A0 A1 ... Z vdd gnd
    - or：A0 A1 Z ... vdd gnd
    - nand：A0 A1 Z ... vdd gnd
    - nor：A0 A1 Z ... vdd gnd