import sys

def binary_to_hex(input_file, output_file):
    try:
        # 打开二进制文件读取
        with open(input_file, "rb") as bin_file:
            # 读取整个文件
            binary_data = bin_file.read()
        
        # 将每个字节转换为两位16进制字符串，并用空格分隔
        hex_data = " ".join(f"{byte:02X}" for byte in binary_data)
        
        # 写入到文本文件
        with open(output_file, "w") as text_file:
            text_file.write(hex_data)
        
        print(f"转换完成，结果已保存到 {output_file}")
    
    except FileNotFoundError:
        print(f"文件 {input_file} 不存在")
    except Exception as e:
        print(f"发生错误：{e}")

# 使用示例
binary_to_hex(sys.argv[1], sys.argv[2])
