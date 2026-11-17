# Framework

## [app](./app/README.md)

### xtaro

利用其他各模块，解析参数，运行程序、提供系统功能接口。

### batch

批处理模式实现

### help

帮助信息

### interactive

交互模式实现

### version

版本信息

### main.cc

程序入口。



## [global](./global/README.md)

### config

- Option：解析用户给定是选项文件；
- Tech：解析指定工艺库信息；

### debug

- Logger：日志；
- Console：命令行输出；
- Debug：组合 Logger 与 Console，控制日志输出以及异常产生；

### utility

存在常用的一些工具函数。例如字符串格式化、文件相关操作、数学、系统函数等。



## [circuit](./circuit/README.md)

### base

存放表示各个电路模块所需要的类：Circuit、Instance、Net、Port；

### module

存放各个具体的模块类，继承自 Circuit

### factory

电路工厂，提供统一的接口生成电路对象，附带一个字符串池。



## [parse](./parse/README.md)

各种文件的解析模块：gdsii、json、lib 等；



## [character](./character/README.md)

### command

SPICE 的 .meas 命令的表示，方便产生 .meas 测量与获取结果。

### simulator

- Simulator：产生 spice 激励与测量命令；
- CircuitSimulator：产生针对指定电路对象的 spice  激励与测量命令；
- SRAMSimulator：产生针对 SRAM 的 spice  激励与测量命令；

### character

- FunctionSimulator：使用 SRAMSimulator 对 SRAM 的功能进行验证；
- DelaySimulator：使用 SRAMSimulator 测量 SRAM 的 delay 信息；
- SetupHoldSimulator：使用 SRAMSimulator 测量 SRAM 的 setup 与 hold 信息；



## sram-circuit

本设计采用的 SARM 电路结构
