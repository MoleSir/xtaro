# Schedule

## 2024/12/09

1. TSMC 前端 pdk 做完，尝试将其前端跑通 
2. 后端 pdk 放大器 + Mux 版图画完（Bitcell 不知道能不能画）



## 2025/1/6

1. 把各个主要部件的版图写完
    - 目前已经完成的：Decoder、BitcellArray、WriteWriterArray、SenseArray、MuxArray
    - 剩下比较困难的：ControlLogic、ColumnDecoder、TreeBuf（）
2. 设计 Wordline Driver（Netlist + Layout）



## 2025/01/20

单个 Core 的 SRAM 版图

- 需要规划好摆放位置：寄存器、收入输出
- rbl 的处理（）
- VDD/GND



## 2025/02/03

多个 Core 的 SRAM 版图

- 还是怎么摆放的问题
- SRAM Selector 
- 输入输出端口放在哪里合适
- 各个 Core 共享的地址、数据输出怎么连（Buf）



## 2025/02/10

用 TSMC28



ASICON
