Record = []

def ReadRecord(readfilename , Writefilename):
    try:
        with open(readfilename , 'r') as file:
            for line in file :
                binary_string = ''.join(format(ord(char), '08b') for char in line)
                append_string_to_file(binary_string , Writefilename)
    except Exception as e:
        print(f"Error: {e}")

def append_string_to_file(input_string, filename):
    try:
        with open(filename, 'ab') as file :
            file.write(input_string.encode('utf-8') + b'\n')
        print(f"Input into file: {filename}'s End")
    except Exception as e:
        print(f'Error: {e}')

def main():
    n_ = 10
    for i in range(n_):
        Readfilename = f"./TextRecord/Node[{i + 1}]AppRecord.txt"
        Writefilename = f"./BinaryRecord/Node[{i + 1}]BinaryRecord.txt"
        ReadRecord(Readfilename , Writefilename)

main()
