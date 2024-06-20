import sys

input_string = sys.stdin.read()
print(input_string)
file1 = open('myfile.txt', 'w')
file1.writelines(input_string)
file1.close()
