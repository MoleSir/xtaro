# signal_escape_router

只在 SRAM 中使用一次：`route_escape_pins`(Add the top-level pins for a single bank SRAM with control.)

首先将所有输出引脚名称写入 `pins_to_route` 列表，再调用：

````python
rtr = signal_escape_router(
        layers=self.m3_stack,
        bbox=bbox,
        design=self)
rtr.route(pins_to_route)
````

其中 bbox 为参数，但这个函数只被调用一次，填入：`self.get_bbox()`。这是一个 `layout` 基类提供的方法，返回图形中的 boudary 覆盖的最大范围。



## `signal_escape_router`

首先调用其基类构造：

````python
router.__init__(self, layers, design, bbox)
````

在 router 构造中再基类构造：

````python
router_tech.__init__(self, layers, route_track_width=1)
````

router_tech 就是创建布线用到的 drc 信息：

- self.track_width
- self.track_space
- self.track_wire
- self.track_widths
- self.track_factor

特别是层次信息，构造指定了 layers，表示一个 layer_stack，其需要确定那个是 水平 layer，哪个是 垂直 layer

回到 router 的构造，主要是初始化一些数据结构，之后会使用到：

````python
self.design = design
ll, ur = bbox
margin = vector([11 * self.track_width] * 2)
self.bbox = [ll - margin, ur + margin]
# Dictionary for vdd and gnd pins
self.pins = {}
# Set of all the pins
self.all_pins = set()
# This is all the blockages including the pins. The graph class handles
# pins as blockages while considering their routability
self.blockages = []
# This is all the vias between routing layers
self.vias = []
# Fake pins are imaginary pins on the side supply pins to route other
# pins to them
self.fake_pins = []
````



## `rtr.route(pins_to_route)`

- `prepare_gds_reader`：把委托进来的 self.desgin（就是 SRAM 对象），输出到临时 gds 文件，然后读取到 self.layout 中

- 对输入的 `pins_to_route: list[str]` 每个名字，调用 `self.find_pins(name)`，其中，从 self.layout 中读取 name 引脚的信息，对每个引脚（一个名称可能多个引脚位置！）位置，创建一个 `graph_shape(pin_name, rect, layer)` 保存，并且将其全部放入一个集合 `pin_set`，之后：

  ````python
  self.pins[pin_name] = pin_set
  self.all_pins.update(pin_set)
  ````

  就是把这个 SRAM 中的引脚信息都封装到 graph_shape 中，使用 self.all_pins 保存

- `find_blockages`：在 sefl.layout 中找位于 self.vert_lpp, self.horiz_lpp 两个层次的所有 boundaries，封装到 `graph_shape(“blockage”, rect, lpp)` 中，全部保存到 self.blockages 列表

- `find_vias`：在 via 层中，查找到所有的 boundaries，封装到 `new_shape = graph_shape("via", rect, valid_lpp)`，全部保存到 `self.vias` 列表

- `convert_vias`：via 上的 boundaries 有些是引脚，遍历 self.vias 中的 graph_shape，如果 这个 graph_shape 与  self.all_pins  某个 pin 发生 overlaps，将 via 的 graph_shape 改名为 pin 的名字

- `convert_blockages`：遍历 self.blockages，如果里面有与 self.all_pins  某个 pin 发生 overlaps，将其改名为 pin 的名字

- 在 self.bbox 的四个边，创建一个 graph_shape，就像是加了相框

  ````python
  pin = graph_shape(name="fake",
                    rect=rect,
                    layer_name_pp=layer)
  ````

  再加入 self.fake_pins：`self.fake_pins.append(pin)`

完成这些后，开始一根一根布线：

````python
for source, target, _ in self.get_route_pairs(pin_names):
	....
````

`get_route_pairs` 接受需要连接的引脚名称列表：

```python
def get_route_pairs(self, pin_names):
    """ Return the pairs to be routed. """

    to_route = []
    for name in pin_names:
        # dict[str, list[graph_shape] ]
        # pin: graph_shape
        pin = next(iter(self.pins[name]))
        fake = self.create_fake_pin(pin)
        to_route.append((pin, fake, pin.distance(fake)))
    # 按照 pin 与 fake 之间的距离排序
    # 需要把它们连起来！
    return sorted(to_route, key=lambda x: x[2])
```

`create_fake_pin` 方法判断一个引脚矩形 bbox 边的位置，四个边哪个最近，做一个垂直线，得到一个新的引脚，使用一个元组（原引脚，新引脚，二者距离）保存到列表中，最后以新老引脚的距离将所有元组排序后返回。

那么 `source, target, _` 中，source 表示原引脚，target 表示新引脚，距离被我们舍弃了。

接下来看对每个源/目标的连接：

````python
# Change fake pin's name so the graph will treat it as routable
target.name = source.name
# Create the graph
g = graph(self)
g.create_graph(source, target)
# Find the shortest path from source to target
path = g.find_shortest_path()
# Create the path shapes on layout
new_wires, new_vias = self.add_path(path)
self.new_pins[source.name] = new_wires[-1]
# Find the recently added shapes
self.find_blockages(name, new_wires)
self.find_vias(new_vias)
````



> 找到可以走的路径点，使用 A* 算法找到最短路径，创建路径

