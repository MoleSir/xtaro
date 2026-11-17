# `wire`

wire 类继承 wire_path。

wire_path 是帮助 layout 对象，在给定坐标、layer 上添加路径（若干 rectangle）

而 wire 是给定一个 layer_stack（layer1, via, layer2），以及系列坐标，也是首先把路径调整为只有直线，然后在每个拐角处增加 via（layer1 -> layer2），再对路径上的每一条直线，如果是水平线，就在 水平 layer 层添加 rectangle，否则在垂直层添加 rectangle。