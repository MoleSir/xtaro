import gdspy

infile = './column_tri_gate.gds'
outfile = './new_column_tri_gate.gds'

lib = gdspy.GdsLibrary(infile=infile, precision=5e-10)

main_cell = lib.top_level()[0]  # 假设修改顶层 cell

# 添加一个矩形作为 VDD 引脚
vdd_width = 0.25  # 引脚的宽度
vdd_height = 0.05  # 引脚的高度
vdd_x, vdd_y = 0.26, 0.04  # 引脚左下角的坐标
vdd_rect = gdspy.Rectangle((vdd_x, vdd_y), (vdd_x + vdd_width, vdd_y + vdd_height), layer=11)
main_cell.add(vdd_rect)

label = gdspy.Label("vdd", position=(0.3, 0.06), layer=11)
main_cell.add(label)

lib.write_gds(outfile)