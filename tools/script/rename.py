import gdspy

new_name = 'tri_gate'
old_name = 'sram_1rw0r0w_2_16_freepdk45_precharge_0'
file = f'./technology/freepdk45/gdsii_lib/{new_name}.gds'

lib = gdspy.GdsLibrary(infile=file, precision=5e-10)

for _, cell in lib.cells.items():
    if cell.name == old_name:
        print('rename')
        cell.name = new_name
        print(cell.name)

lib.write_gds(file)
