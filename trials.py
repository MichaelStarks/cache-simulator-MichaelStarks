import fileinput
import subprocess
import sys

import pandas as pd
from numpy.lib.shape_base import column_stack


file = str(sys.argv[1])

execution_time = 0

def is_number(string):
    return string[-1].isdecimal()

column_names = ["Lines Found","Execution Time (ps)","Execution Cycles",
            "Instructions","Memory Accesses","Overall Miss Rate",
            "Read Miss Rate","Memory CPI","Total CPI",
            "Average Memory Access Time (cycles)","Dirty Evictions","Load Misses",
            "Store Misses","Load Hits","Store Hits","Parameters"]

try:
    csv_dataframe = pd.read_csv(file,index_col=0) 
except FileNotFoundError:
    csv_dataframe = pd.DataFrame(columns=column_names)

numbers = []
parameters = []

for (index,line) in enumerate(fileinput.input()):
    # print("Python: " + line.rstrip().strip())
    try:
        number = [float(s) for s in line.split() if is_number(s) or s.isdecimal()][0]
    except IndexError:
        continue
    if index == 8:
        execution_time = number

    if index <= 4:
        parameters.append(number)
    else:
        numbers.append(number)
numbers.append(parameters)

csv_dataframe.loc[len(csv_dataframe)] = numbers

csv_dataframe.to_csv(file)

print(execution_time)
