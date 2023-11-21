import math

def append_string_to_file(input_string, filename):
    try:
        with open(filename, 'w') as file:
            file.write(input_string + '\n')
        print(f"Input into file : {filename} 's End")
    except Exception as e:
        print(f'Error : {e}')

def main() : 
    n_ = 10
    radius1 = 0.5
    radius2 = 5.0
    Standard = "11b"

    NodesLocation = [[0.0 , 0.0 , 0.0] for _ in range(n_)]

    for i in range (n_) : 
        rho = 0.5
        if not (i+1 <= 10/2) : 
            rho = 5.0
        theta = (2 * math.pi * i) / n_
        x = rho * math.cos(theta)
        y = rho * math.sin(theta)
        z = 0.0
        NodesLocation[i] = [x , y , z]

    for i in range(n_) : 
        rate = 5.0
        if not (i+1 <= 10/2) : 
            rate = 11.0
        input_string = "Search()\n"
        input_string += f"BuildCahnnel({Standard} , {rate} , {NodesLocation[i][0]} , {NodesLocation[i][1]} , {NodesLocation[i][2]})\n"
        input_string += f"Schedule(1.0 , 20.0)"
        filename = f"./TextRecord/Node[{i+1}]AppRecord.txt"
        append_string_to_file(input_string , filename)

main()