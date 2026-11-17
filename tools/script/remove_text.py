import gdspy

lib = gdspy.GdsLibrary(infile='./bitcell.gds', precision=5e-10)

for name, cell in lib.cells.items():
    gdspy.Cell.remove_labels(cell, lambda l: len(l.text) > 10)

lib.write_gds('./bitcell1.gds')