# Control Logic

只考虑 rw 类型

## `__init__`

````python
self.num_control_signals
````



## `create_netlist`

### `setup_signal_busses`

````python
self.input_list = ["csb", "web"]
self.rbl_list = ["rbl_bl"]

self.dff_output_list = ["cs_bar", "cs", "we_bar", "we"]

self.internal_bus_list = ["rbl_bl_delay_bar", "rbl_bl_delay", "gated_clk_bar", "gated_clk_buf", "we", "we_bar", "clk_buf", "cs"]

self.internal_bus_width = 9 * self.m2_pitch

self.output_list = ["s_en", "w_en", "p_en_bar", "wl_en", "clk_buf", tech->vdd(), tech->gnd()]

self.supply_list = [tech->vdd(), tech->gnd()]
````

### `add_pins`

````python
self.add_pin_list(["csb", "web", "clk", "rbl_bl"], "INPUT")
self.add_pin_list(["cs_bar", "cs", "we_bar", "we"], "OUTPUT")
self.add_pin(tech->vdd(), "POWER")
self.add_pin(tech->gnd(), "GROUND")
````

### `add_modules`

````python
self.dff = factory.create(module_type="dff_buf")
dff_height = self.dff.height
self.ctrl_dff_array = factory.create(module_type="dff_buf_array",
                                     rows=2,
                                     columns=1)
self.and2 = factory.create(module_type="pand2",
                           size=12,
                           height=dff_height)

self.rbl_driver = factory.create(module_type="pbuf",
                                 size=self.num_cols,
                                 height=dff_height)
````

### `create_instances`

````python
````



