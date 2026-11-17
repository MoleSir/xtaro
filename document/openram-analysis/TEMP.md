VLSILayout 类布局原理

wire_path 类。只是一个辅助类，不是版图中的对象（例如 pin_layout），将路径传入，其帮助 layout 创建各类图形表示这个路径。

- 将一个坐标列表给 wire_path。
- 首先对所有两两相邻坐标，如果二者 x、y 都不同，插入一个中间坐标，使得这个 path 只走直线；
- 遍历新的坐标列表，找到所有直接拐角，计算这个拐角的信息，调用 layout.add_rect，创建 corner
- 对直线，调用 layout.add_line 添加

layout 类中的 pin_map 类型为：`dict[str, set[pin_layout]]`

route_vertical_pins 函数将指定实例中名为 name 的 pin_layout 连接起来（在指定 layer），垂直连接：

1. 遍历所有实例，获取实例名为 name 的 pin_layout（实例的 pin_layout 是将其指向电路的 pin_layout 进行坐标处理得到（处理参数保存在这个实例中））；
2. 对所有 pin_layout，以 x 坐标分类。产生一个字典：`dict[int, [(instance, pin_layout)]`，把 x 坐标相同的 pin 分到一组；
3. 因为按垂直连接。对字典的每一项，确定一 x 值，还有一系列 pin，对每个 pin，在 pin 位置创建从 layer 到 pin.layer 的 via。再计算所有 pin 中 y 最大与最小者，在 layer 层创建一条导线即可把所有 pin 连接



## GDSII

在 GDSII 文件中，包含了一系列 Structure，每个 Structure 对应一个实际电路。

而在 Structure 中可以包含很多类型的 Element，比如 Path 可以表示路径（其中含有一系列坐标点）、Boundary 可以表示矩形、Text 表示文字、Sref 表示引用的其他 Structure 等。其中可以存在多个 Sref，每个 Sref 意味着本 Structure 中引用了另一个 Structure，被引用的名称包含在了 Sref 描述中，而因为这是版图，Sref 还包含被引用 Structure 的位置、镜像信息以及位置。而这个被引用的 Structure 实际上也是全局 Structure 中的一员。

所以通常一个 GDSII 文件，包含多个 Structure，即多个独立的电路，但其中有一个是跟 Structure，它包含若干 Sref 结构，指向了这个文件中其他 Structure，表示子模块关系。而被包含的 Strucutrue 通常也还会包含若干 Sref，形成了树状结构。

遍历所有 Structure、以及其 Sref 中引用的名称，只出现一次的就是这个文件的根 Structure。

可以看出，GDSII 文件描述电路的方式与 SPICE 还是比较相似的。SPICE 也是在一个文件中存在多个 .SUBCKT 描述，每个 .SUBCKT 还可以包含其他的 .SUBCKT。只不过，在 SPICE 描述中，主要关注的是电路的连接关系，但在 GDSII 中，这个连接关系被淡化了，没有专门用来描述连接关系的 Element，而需要用 Path、Box 等结构自己组装。

例如在 SPICE 中，一个 .SUBCKT 的子 .SUBCKT 已经被描述好了，那么只要在实例化时使用相同的节点名称表示连接关系即可。

但在 GDSII 中，一个 Structure 所需要的子 Structure 都准备好了，那么在创建各个 Sref 时还需要规则每个子 Structure 的拜访位置。并且子 Structure 的连接关系需要使用 Path 等其他 Element 构建，这比较困难（还有各种分层关系）。



## 写 GDSII

在 OpenRAM 中，输出某个电路的 Spice 网表需要先准备一个空白文件，准备一个空集合。再调用这个电路对象的 write_spice_file 方法，传入这两个信息，对每个电路对象，首先判断其是否位于集合中，是说明被写过，直接退出，否则先调用其子电路对象的 write_spice_file，再根据自己电路连接关系写入一个 .SUBCKT。

这样的流程保证：子模块总是在父模块之前被写入，并且不会重复相同的电路模块。

而 OpenRAM 实现的写 GDSII 流程有些不同。

在电路对象创建阶段，每个电路对象会保存：其中的子实例对象（子实例对象指向一个电路对象，并且保存电路在这个父电路中的位置信息，如 Sref 中所需要的）、各种图形对象（矩形、文本、路径，用来构建电路连接关系的）以及 pin_layout。

并且 OpenRAM 创建了一个模块专门来读写 GDSII 文件。其中的 VLSILayout 类可以表示一个 GDSII 文件。每个 VLSILayout 包含多个 GDSIIStructure 类对象，但只有一个是根对象。可以理解是一个电路对象，最后可以对应到一个 VLSILayout 对象。

在电路对象中保存的各种对象都会实现一个 write_layout_file 的方法，将自身的信息写入到一个指定的 VLSILayout 对象中。

- 子实例对象。调用子实例对象的 write_spice_file 方法，会先判断这个子实例指向的子电路对象是否已经创建了 VLSILayout 对象。如果没有，单独为这个电路创建 VLSILayout 对象。创建好后，调用父电路对象此时还没创建好的 VLSILayout 对象的 addInstance 方法，将新创建的 VLSILayout 作为实例添加。

  添加的过程是： 把新 VLSILayout 中所有的 Structure 复制到父电路 VLSILayout 中，为父电路 VLSILayout 对象的根 Structure 增加一个 Sref，其引用的名称为新 VLSILayout 对象的根 Structure 名称，其信息即为子实例对象的位置信息。

- 图形对象：根据自身的情况，给 VLSILayout 对象中的根 Strucuture 添加 Path、Box 等；

- pin_layout：pin_layout 类似图形对象，给根 Strucuture 添加 Box、Text 等，表示一个引脚。这里需要注意，在 SPICE 中，用相同的名称表示连接关系。但 GDSII 需要实际位置，所以相同的 pin 可能出现在多个地方，将其他连接即表示了 SPICE 中的连接关系。

当需要输出电路对象的 GDSII 文件，也即为一个电路对象创建 VLSILayout。首先创建一个空的 VLSILayout，再调用输出其中各个对象的 write_layout_file 方法，不断填充这个 VLSILayout。这个过程中，其下各个子电路对象的 VLSILayout 对象也会被依次创建。

最后利用 GDSII 库中的 GDSIIWriter，将 VLSILayout 对象输出为 GDSII 文件。



## Contact

相邻的 via：两层层 0,0 -> 800,  800 夹着中间 via 层的 200,200 -> 600,600

m1 与 active 之间的 via 比较复杂，还需要 nimplant、 nwell



## 特别的模块

sram_1rw0r0w_2_16_scn4m_subm_cr_3：channel_route

layout 类中有：

- create_vertical_channel_route
- create_horizontal_channel_route

用来创建 channel_route。

Bank、PortData 以及 SRAM 会创建这个 channel route 模块。

channel_route 主要用来辅助连接 pin。需要传入一个 list[tuple[pin_layout]]，每个  tuple[pin_layout] 中的 pin_layout 是需要被连接起来的 pin_layout 对象。

并且可以指定布线层次与方向。channel_route 会使用一系列算法，在不违反 DRC 的情况下，在指定的 layer 上，以指定的方向（垂直/水平），把每组 pin_layout 连接。

算法大致的思路是把需要连接的一组 pin_layout 视为一条 net，设置出这个 net 范围。所有 net 两两判断是否违反 DRC 检查。然后取出没有与其他任何违反 DRC 的 net，进行布线，剩下的 net 会进行一定的处理，直到不违反 DRC 后，再布线。不断重复这个过程。

布线的过程比较直观，首先需要确定是垂直布线还是水平布线，以水平为例，找出这组 pin_layout 的最大 x 与最小 x，给定布线的 y 坐标，绘制处于 self.vertical_layer 层上 min_x,y => max_x,y 的 path。再对每个引脚，首先做 self.vertical_layer 层上引脚与这条 path 的垂直线，再在引脚位置的 layer 与 self.vertical_layer 打 via，就连接上了



## Ques

layout 对象中的 {layer}_pitch

在半导体工艺设计中，DRC（Design Rule Check）是一种用于检查芯片设计中是否符合制造工艺规则的技术。而在 DRC 中，"pitch" 是一个重要的概念，它指的是芯片中元件之间的间距或排列的密度。

具体来说，"pitch" 通常用于描述晶体管、金属线、连线等元件之间的间距或排列的密度。在芯片设计中，DRC 会检查这些元件之间的 pitch 是否满足制造工艺所规定的最小间距要求，以确保芯片在实际制造过程中能够正常工作并避免因为元件之间距离过小而导致的电气问题或制造工艺上的困难。

通常，DRC 中的 pitch 规则会根据具体的工艺节点和设计要求而有所不同，因此设计工程师需要根据芯片设计的具体情况来合理设置元件之间的间距或密度，以确保设计满足 DRC 规则。





## Layout 

1. `setup_drc_constants`：从 drc 中读取信息，加入到 layout 中
2. `setup_contacts`：创建 contact 对象（使用 factory），赋值给 pwell_contact、nwell_contact、{layer}_contact 等对象中
3. `setup_layer_constants`：计算各层的 pitch





## Bitcell Array

bank => capped_replica_bitcell_array => replica_bitcell_array => bitcell_array

````
        if OPTS.control_logic != "control_logic_delay":
            self.has_rbl = True
            rbl = [1, 1 if len(self.all_ports)>1 else 0]
            left_rbl = [0]
            right_rbl = [1] if len(self.all_ports)>1 else []
        else:
            self.has_rbl = False
            rbl = [0, 0]
            left_rbl = []
            right_rbl = []
````

根据时序控制单位的类型，控制是否使用 rbl，如果不使用 delay chain：

- has_rbl = True
- rbl = [1, 0] 单端口、[1, 1] 多端口
- left_rbl = [0]
- right_rbl = [1] 多端口、[] 单端口

在 bank 模块确定，不断向下传递到 replica_bitcell_array



## 工厂

工厂创建电路

````python
obj = mod(name=module_name, **kwargs)
````



OPTS：

````python
bitcell = "bitcell"
````



创建一个 bitecll 

````
self.cell = factory.create(module_type=OPTS.bitcell)
````



初始化 OpenRAM 的时候，设置 OPTS.bitcell：

````python
OPTS.bitcell = "bitcell_{}port".format(OPTS.num_ports)
````

如果只有一个端口，就是 OPTS.bitcell = "bitcell_1port"

调用的就是：

````
obj = bitcell_1port(name="bitcell_1port")
````

在 bitcell_1port 构造时调用：

````python
bitcell_base.__init__(self, "bitcell_1port", prop=props.bitcell_1port)
````

再调用 

````python
design.__init__(self, name="bitcell_1port", cell_name=None, prop=props.bitcell_1port)
````

在 design 的构造函数中执行：

````python
        if name in props.names:
            if type(props.names[name]) is list:
                num_ports = OPTS.num_rw_ports + OPTS.num_r_ports + OPTS.num_w_ports - 1
                cell_name = props.names[name][num_ports]
            else:
                cell_name = props.names[name]

        elif not cell_name:
            cell_name = name
            
````

此时 name = "bitcell_1port"，props 是 tech.py 中的一个对象：

````python
cell_properties = d.cell_properties()
---

class cell_properties():
    def __init__(self):
        self.names = {}
        self.names["bitcell_1port"] = "cell_1rw"
        self.names["bitcell_2port"] = "cell_2rw"
        self.names["dummy_bitcell_1port"] = "dummy_cell_1rw"
        self.names["dummy_bitcell_2port"] = "dummy_cell_2rw"
        self.names["replica_bitcell_1port"] = "replica_cell_1rw"
        self.names["replica_bitcell_2port"] = "replica_cell_2rw"
        self.names["col_cap_bitcell_1port"] = "col_cap_cell_1rw"
        self.names["col_cap_bitcell_2port"] = "col_cap_cell_2rw"
        self.names["row_cap_bitcell_1port"] = "row_cap_cell_1rw"
        self.names["row_cap_bitcell_2port"] = "row_cap_cell_2rw"
        self.names["internal"] = "internal"
````

这样可以查到 `name = "bitcell_1port"` 在 ` props.names`，进入 if 内部：

````python
            if type(props.names[name]) is list:
                num_ports = OPTS.num_rw_ports + OPTS.num_r_ports + OPTS.num_w_ports - 1
                cell_name = props.names[name][num_ports]
            else:
                cell_name = props.names[name]
````

`props.names["bitcell_1port"]` 是 "cell_1rw"，所以这个类型 list 判断失败，进入 else：

```python
cell_name = "cell_1rw"
```

再继续执行：

````python
hierarchy_design().__init__("bitcell_1port", "cell_1rw")
````

在其中：

```python
gds_file = OPTS.openram_tech + "gds_lib/" + cell_name + ".gds"
         = OPTS.openram_tech + "gds_lib/cell_1rw.gds"
```

存在这个路径，调用两个父类构造：

````python
        spice.__init__("bitcell_1port", "cell_1rw")
        layout.__init__("bitcell_1port", "cell_1rw")
````

在 spice 中：

````python
self.sp_file = OPTS.openram_tech + "sp_lib/" + cell_name + ".sp" 
             = OPTS.openram_tech + "sp_lib/cell_1rw.sp"
````

进入 layout：

````python
        self.name = name
        self.cell_name = cell_name
````

再调用 `gds_read`，因为存在 gds 路径

````python
            self.gds = gdsMill.VlsiLayout(units=GDS["unit"])
            reader = gdsMill.Gds2reader(self.gds)
            reader.loadFromFile(self.gds_file, special_purposes)
````





在 design 构造函数中，完成父类构造后：

````python
if prop and prop.hard_cell:
    # The pins get added from the spice file, so just check
    # that they matched here
    debug.check(prop.port_names == list(self.pins),
                "Custom cell pin names do not match spice file:\n{0} vs {1}".format(prop.port_names, list(self.pins)))
    self.add_pin_indices(prop.port_indices)
    self.add_pin_names(prop.port_map)
    self.update_pin_types(prop.port_types)


    (width, height) = utils.get_libcell_size(self.cell_name,
                                             GDS["unit"],
                                             layer[prop.boundary_layer])
    self.pin_map = utils.get_libcell_pins(list(self.pins),
                                          self.cell_name,
                                          GDS["unit"])

    # Convert names back to the original names
    # so that copying will use the new names
    for pin_name in self.pin_map:
        for index1, pin in enumerate(self.pin_map[pin_name]):
            self.pin_map[pin_name][index1].name = self.get_original_pin_name(pin.name)

    self.width = width
    self.height = height
````

可以从工艺库中获取标准单元的引脚信息、大小信息

````
self.pin_map = utils.get_libcell_pins(list(self.pins),
                                          self.cell_name,
                                          GDS["unit"])
    self.width = width
    self.height = height
````



## 读取版图引脚信息

在 design 类的构造函数中，会检查当前模块是否为 lib cell，如果是就会读取 .gds 文件。并且从中加载得到两个关键参数（创建电路结构时会使用到的！）：

- 电路尺寸（长 x 宽）
- 电路的引脚信息

````python
(width, height) = utils.get_libcell_size(self.cell_name,
                                         GDS["unit"],
                                         layer[prop.boundary_layer])

self.pin_map = utils.get_libcell_pins(list(self.pins),
                                      self.cell_name,
                                      GDS["unit"])
````

`get_libcell_pins` 函数位于 base.utils.py 中：

````python
def get_libcell_pins(pin_list, name, units):
    """
    Open a GDS file and find the pins in pin_list as text on a given layer.
    Return these as a rectangle layer pair for each pin.
    """

    cell_gds = OPTS.openram_tech + "gds_lib/" + str(name) + ".gds"
    return(get_gds_pins(pin_list, name, cell_gds, units))
````

再进入 `get_gds_pins`

````python
    cell_vlsi = _get_gds_reader(units, gds_filename)

    cell = {}
    for pin_name in pin_names:
        cell[str(pin_name)] = []
        pin_list = cell_vlsi.getPinShape(str(pin_name))
        for pin_shape in pin_list:
            if pin_shape != None:
                (lpp, boundary) = pin_shape
                rect = [vector(boundary[0], boundary[1]),
                        vector(boundary[2], boundary[3])]
                # this is a list because other cells/designs
                # may have must-connect pins
                if isinstance(lpp[1], list):
                    try:
                        from openram.tech import layer_override
                        if layer_override[pin_name]:
                            lpp = layer_override[pin_name.textString]
                    except:
                        pass
                    lpp = (lpp[0], None)
                cell[str(pin_name)].append(pin_layout(pin_name, rect, lpp))

    _GDS_PINS_CACHE[k] = cell
    return dict(cell)
````

1. 首先调用 `_get_gds_reader` 读取版图信息
2. 对调用 版图对象 的 `getPinShape` 方法获取对应的引脚位置信息，包装为特定的数据结构返回

返回的一个是一个  dict，key 为 str，表示引脚名称，而 value 是列表（因为同一个引脚名称可能存在多个摆放位置），列表中的每个元素表示一个矩形

