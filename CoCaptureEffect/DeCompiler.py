def binary_to_utf8(binary_str):
    if binary_str == '\n':
        return ''

    decimal_value = int(binary_str, 2)
    utf8_character = chr(decimal_value)
    return utf8_character

def ReadRecord(readfilename, Writefilename):
    try:
        with open(readfilename, 'r') as file:
            for line in file:
                utf8_result = ''.join(chr(int(line[i:i+8], 2)) for i in range(0, len(line)-1, 8))
                append_string_to_file(utf8_result , Writefilename)

    except Exception as e:
        print(f"Error: {e}")

def append_string_to_file(input_string, filename):
    try:
        with open(filename, 'a') as file:
            file.write(input_string + '\n')
        print(f"Input into file: {filename}'s End")
    except Exception as e:
        print(f'Error: {e}')

def main():
    n_ = 10
    for i in range(n_):
        Writefilename = f"./TextRecord/Node[{i + 1}]AppRecord.txt"
        Readfilename = f"../ApplicationRecord/BinaryRecord/Node[{i + 1}]BinaryRecord.txt"
        ReadRecord(Readfilename, Writefilename)

main()
