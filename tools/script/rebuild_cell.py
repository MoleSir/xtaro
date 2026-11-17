import gdspy

# 读取输入 GDSII 文件


files = [
    "./technology/freepdk45/gdsii_lib/and_2_x1.gds",
    "./technology/freepdk45/gdsii_lib/and_3_x1.gds",
    "./technology/freepdk45/gdsii_lib/and_4_x1.gds",

    "./technology/freepdk45/gdsii_lib/nand_2_x1.gds",
    "./technology/freepdk45/gdsii_lib/nand_3_x1.gds",
    "./technology/freepdk45/gdsii_lib/nand_4_x1.gds",

    "./technology/freepdk45/gdsii_lib/nor_2_x1.gds",
]

for file in files:
    lib = gdspy.GdsLibrary(infile=file, precision=5e-10)

    for name, cell in lib.cells.items():
        for label in cell.labels:
            if label.text == 'A':
                label.text = 'A0'
            elif label.text == 'B':
                label.text = 'A1'
            elif label.text == 'C':
                label.text = 'A2'
            elif label.text == 'D':
                label.text = 'A3'

    lib.write_gds(file)
