import sys

def hex_to_binary(input_file, output_file):
    try:
        # 打开文本文件读取
        with open(input_file, "r") as text_file:
            # 读取十六进制数据并移除多余的空白
            hex_data = text_file.read().strip()
        
        # 将十六进制字符串拆分成字节数组
        binary_data = bytes.fromhex(hex_data)
        
        # 将字节数组写入二进制文件
        with open(output_file, "wb") as bin_file:
            bin_file.write(binary_data)
        
        print(f"解码完成，结果已保存到 {output_file}")
    
    except FileNotFoundError:
        print(f"文件 {input_file} 不存在")
    except ValueError:
        print("输入文件内容格式错误，请确保是有效的十六进制字符串")
    except Exception as e:
        print(f"发生错误：{e}")

# 使用示例
hex_to_binary(sys.argv[1], sys.argv[2])
