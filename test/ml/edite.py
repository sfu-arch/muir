#!/usr/bin/env python3

import os

search = "cluster_0__XlaCompiledKernel_true__XlaNumConstantArgs_0__XlaNumResourceArgs_0_.v15"

func = list()
for file in os.listdir("."):
    if(file.endswith(".ll")):
        if search in open(file).read():
            func.append(('test_' + file.split('.', 1)[0], file))

print(func)

for replace in func:
    with open(replace[1], 'r') as file:
        data = file.read().replace(search, replace[0])
    with open(replace[1], 'w') as file:
        file.write(data)
