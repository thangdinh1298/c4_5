import sys
import random

if __name__ == "__main__":
    file_name = sys.argv[1]
    lines = []
    header = None
    
    with open(file_name) as file:
        i = 0
        for line in file:
            if i != 0:
                lines.append(line)
            else:
                header = line
            i+=1

    random.shuffle(lines)

    with open(file_name + '_shuffled', 'w') as file:
        file.write(header)
        for line in lines:
            file.write(line)
