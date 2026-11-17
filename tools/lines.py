import sys
import os

compiler_path = './source'

def not_ignore(path) -> bool:
    return path != 'template' and path != 'lines.py' and path != 'CMakeLists.txt' and path != 'si2-lef'

def dir_sub_paths(dir_path):
    sub_paths = os.listdir(dir_path)
    sub_paths = filter(not_ignore, sub_paths)
    sub_paths = map(lambda path: f'{dir_path}/{path}', sub_paths)
    return sub_paths

def count_file_lines(file_path):
    with open(file_path, 'r') as file:
        line_count = sum([1 for line in file])
    return line_count

def count_dir_lines(dir_path) -> int:
    sub_paths = dir_sub_paths(dir_path)

    def count_lines(path) -> int:
        return count_dir_lines(path) if os.path.isdir(path) else count_file_lines(path)
    
    return sum(map(count_lines, sub_paths))

print(count_dir_lines(compiler_path))