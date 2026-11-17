# drc 

分析 drc/design_rules、drc/drc_lut、drc/drc_value



## `drc_value` 类

```python
class drc_value():
    def __init__(self, value):
        self.value = value

    def __call__(self, *args):
        return self.value
```

构造函数传入 `value` 被类对象保存。重写 `__call__` 使得在调用 `obj()` 时返回构造时候传入的值。



## `drc_lut` 类

> Implement a lookup table of rules. 
>
> Each element is a tuple with the last value being the rule.
>
> It searches through backwards until all of the key values are met and returns the rule value.
>
> For exampe, the key values can be width and length, and it would return the rule for a wire of at least a given width and length.
>
> A dimension can be ignored by passing inf.

实现规则的查找表。

每个元素都是一个元组，元组的最后一个值是规则。

它向后搜索，直到满足所有键值，并返回规则值。

例如，键值可以是 width 和 length，它将为至少给定宽度和长度的导线返回规则。

通过传递 `inf` 可以忽略维度。

### `__init__`

````python
def __init__(self, table):
    self.table = table
````

传入的是一个字典对象。例如：

````python
drc_lut({(0.00, 0.0): 0.14,
         (0.27, 0.9): 0.27,                       
         (0.50, 1.8): 0.5,
         (0.90, 2.7): 0.9,
         (1.50, 4.0): 1.5})
````

### `match`

````python
def match(self, key1, key2):
    debug.check(len(key1) == len(key2), "Comparing invalid key lengths.")
    for k1, k2 in zip(key1, key2):
        if k1 < k2:
            return False
    return True
````

对两个参数 `key1`、`key2`（都为元组），如果 `key1` 中的所有元素都大于或者等于 `key2` 中对应的元素，返回 `True` 否则为 `False`。

### `__call__`

````python
def __call__(self, *key):
    if len(key)==0:
        first_key = list(sorted(self.table.keys()))[0]
        return self.table[first_key]

    for table_key in sorted(self.table.keys(), reverse=True):
        if self.match(key, table_key):
            return self.table[table_key]
````

参数是要要需要的规则名称列表。

- 如果没有传入任何参数。将 `self.table` 键进行排序，取出第一个（最小值）赋值给 `first_key`，之后返回这个`key` 对应的值；
- 如果有参数，从表的最大键开始判断，使用 `match` 比较 `key` 列表中的所有值是不是都大于 `table_key`，如果找到就返回这个 `table_key` 对应的值。也就是找到第一个比 `key` 小的键值。



## `design_rules` 类

> This is a class that implements the design rules structures.

### `__init__`

````python
def __init__(self, name):
    self.tech_name = name
    self.rules = {}
````

传入工艺卡名称保存到 `tech_name`，创建空字典 `rules` 放置规则。

### `add`

````python
def add(self, name, value):
    self.rules[name] = value
````

给 `self.rules` 增加指定的键值对。

### `__call__`

````python
def __call__(self, name, *args):
    rule = self.rules[name]
    if callable(rule):
        return rule(*args)
    else:
        return rule
````

传入需要的规则名称 `name` 与参数列表 `args`。

1. 通过 `name` 从 `self.rules` 获取对应的 `rule`；
2. 如果 `rule` 重写了 `__call__`，调用 `rule(*args)`；
3. 否则直接返回 `rule`。

### 模仿字典

重写 `__setitem__`、`__contains__` 与 `__getitem__` 等使得类对象可以完成字典的操作。实际上都是对内部的 `self.rules` 字典操作：

````python
def __setitem__(self, b, c):
    self.rules[b] = c

def __contains__(self, b):
    return b in self.rules

def __getitem__(self, b):
    rule = self.rules[b]
    if not callable(rule):
        return rule
    else:
        debug.error("Must call complex DRC rule {} with arguments.".format(b),-1)

def keys(self):
    return self.rules.keys()
````

### `add_layer`

````python
def add_layer(self, name, width, spacing, area=0):
    # Minimum width
    self.add("minwidth_{}".format(name), width)
    # Minimum spacing (could be a table too)
    self.add("{0}_to_{0}".format(name), spacing)
    # Minimum area
    self.add("minarea_{}".format(name), area)
````

增加某 layer 的规则信息：`width`、`spacing`、`area`。

实际上就是调用三个 `add`，规则名称如上，规则值为 `width`、`spacing`、`area`。

### `add_enclosure`

````python
def add_enclosure(self, name, layer, enclosure, extension=None):
    self.add("{0}_enclose_{1}".format(name, layer), enclosure)
    # Reserved for asymmetric enclosures
    if extension:
        self.add("{0}_extend_{1}".format(name, layer), extension)
    else:
        self.add("{0}_extend_{1}".format(name, layer), enclosure)
````

增加 enclosure 规则。



## 规则含义

1. **Via**：
   - Via 是一种用于将电子信号或功率从一个层传输到另一个层的结构。它通常由金属填充或者叠层的孔洞组成，用于连接不同金属层或者不同层次的电路结构。例如，从 Metal 1 层连接到 Metal 2 层的信号线可能需要通过 Via 结构来实现。
   - 在版图中，Via 通常被放置在两个金属层之间的交叉点上，用于建立垂直连接。因为 Via 通常由金属填充，所以它们通常比较大，并且在设计中需要足够的空间来放置。
2. **Contact**：
   - 接触（Contact）用于建立金属层与其他结构（如多晶硅、掺杂区等）之间的连接。它是将金属层与其他层之间的信号或功率传输到芯片内部的关键组件。
   - 在版图中，接触通常被放置在多晶硅层或其他结构上，并与金属层相连。它们通常比 Via 小，因为它们主要用于建立水平连接，而不是垂直连接。



1. **Width（宽度）**：
   - "Width" 通常指的是单个元素或结构的宽度，比如金属线、多晶硅线、电容等的宽度。它规定了元素在设计中必须具备的最小宽度。这个参数确保了元素能够承受正确的电流或信号，并且能够满足电路的性能要求。
2. **Pitch（间距）**：
   - "Pitch" 指的是元素之间的间距，通常是指元素的中心点之间的距离。它用于规定不同元素之间的最小间距要求，如金属线之间的间距、Via 之间的间距等。通过规定合适的 pitch，可以确保避免电气短路或制造上的问题，同时在设计中留出足够的空间以满足制造的要求。
3. **Space（间隔）**：
   - "Space" 规定了元素之间的空白区域的最小尺寸。与 pitch 不同，space 指的是两个元素之间的实际距离，而不是它们的中心点之间的距离。它确保了元素之间有足够的间隔，以防止电气短路或制造上的问题。Space 通常用于规定金属线与其他元素（如多晶硅、接触等）之间的间距。
