# XTaro

[开源、跨工艺的 SRAM 编译器](https://gitee.com/MoleSir/XTaro.git)



## 项目构建

XTaro 基于 C++20 编写，使用 xmake 作为构建工具。并且调用外部仿真器实现 SPICE 仿真（支持 ngspice 与 spectre）。

### 安装依赖

以 Ubuntu22.04 为例： 

- C++20：最简单的是下载 gcc-13

    ````bash
    sudo apt update
    sudo apt install build-essential
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
    ````

- [xmake](https://xmake.io/#/)：官网有下载教程，部分 linux 发行版可以直接使用包管理工具下载

    ````bash
    sudo apt install xmake

- ngspice/spectre/hspice：ngspice 可以直接使用包管理工具下载。spectre/hspice 是收费软件

    ```bash
    sudo apt install ngspice
    ```

- bison：使用包管理工具下载（用于 lef 文件解析）

    ````bash
    sudo apt install bison

### 编译

下载源码

````bash
git clone https://gitee.com/MoleSir/XTaro.git && cd XTaro
````

接着使用 xmake 编译项目

````bash
xmake build xtaro_compiler
````

如果一切顺利，将在项目根目录创建 target 目录，并在其中生成 `xtaro_compiler` 即为编译成功，尝试调用：

```bash
./target/xtaro_compiler --help
```

将在屏幕上显示帮助信息。

> 如果第一次执行 `xmake build` 在链接时期显示有关 LefDefParser 的错误，再执行一次 `xmake build` 可以解决。大概是 Bison 的问题，还没找到更好的解决方案。

### 运行

运行需要输入一份配置文件，项目中提供了一个简单的[例子](./example/options.json)。在项目编译成功后运行：

````bash
./target/xtaro_compiler ./example/option.json
````

> 也可以使用 xmake 来执行，但其运行路径不在根目录，所以配置文件的路径需要修改！

即可产生对应 SRAM 输出！



## 项目结构

- [source](./source/)：编译器源码
- [document](./document/)：文档
- [example](./example/)：输入配置文件示例
- [technology](./technology/)：工艺库目录
- [third party](./third_party/): 第三方开源库代码




##  LICENSE

[木兰宽松许可证, 第2版](LICENSE)



##  致谢

开发过程中，采用了来自开源社区的代码。具体情况如下：

|   子模块   |                             来源                             |                           详细用途                           |
| :--------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
| lef parser | [Si2 LEF parser](https://github.com/The-OpenROAD-Project-Attic/lef.git) + [LEF/DEF util](https://github.com/jinwookjungs/lefdef_util.git) | 使用 Si2 LEF 库实现 lef 文件的解析，并且参考 LEF/DEF util 学习使用 Si LEF |
| argparser  |       [argparse](https://github.com/jamolnng/argparse)       |                          命令行解析                          |

感谢来自开源社区的支持。

