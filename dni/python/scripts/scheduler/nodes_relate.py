# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2024-03-20 08:52:09

import dask
from dask import delayed
import time


def func1(x):
    print("1")
    res1 = x +1
    return res1

def func2(res1, y):
    print("2")
    res2 = res1 + y
    time.sleep(5)
    return res2

def func3(res1, z):
    print("3")
    res3 = res1 * z
    time.sleep(5)
    return res3

def func4(res2, res3):
    print("4")
    res4 = res2 + res3
    res4a = res2 * res3
    return res4, res4a

def func5(res4):
    print("5")
    res5 = res4 + 1
    time.sleep(5)
    return res5

def func6(res4a):
    print("6")
    res6 = res4a + 2
    res6a = res4a * 2
    time.sleep(5)
    return res6, res6a

def func7(res5):
    print("7")
    res7 = res5 + 1
    time.sleep(5)
    return res7

def func8(res6):
    print("8")
    res8 = res6 + 1
    time.sleep(5)
    return res8

def func9(res6a):
    print("9")
    res9 = res6a + 2
    time.sleep(5)
    return res9

def func10(res7, res8, res9):
    print("10")
    res10 = res7 + res8 + res9
    return res10

start_time = time.time()

# x = 1
# y = 2
# z = 3
# delayed_1 = delayed(func1)(x)
# delayed_2 = delayed(func2)(delayed_1, y)
# delayed_3 = delayed(func3)(delayed_1, z)
# delayed_4 = delayed(func4)(delayed_2, delayed_3)
# delayed_5 = delayed(func5)(delayed_4[0])
# delayed_6 = delayed(func6)(delayed_4[1])
# delayed_7 = delayed(func7)(delayed_5)
# delayed_8 = delayed(func8)(delayed_6[0])
# delayed_9 = delayed(func9)(delayed_6[1])
# delayed_10 = delayed(func10)(delayed_7, delayed_8, delayed_9)
# res = delayed_10.compute()

node1 = {"node_name": "node1", "executor": "func1", "input": "x", "output": "res1"}
node2 = {"node_name": "node2", "executor": "func2", "input": "res1,y", "output": "res2"}
node3 = {"node_name": "node3", "executor": "func3", "input": "res1,z", "output": "res3"}
node4 = {"node_name": "node4", "executor": "func4", "input": "res2,res3", "output": "res4,res4a"}
node5 = {"node_name": "node5", "executor": "func5", "input": "res4", "output": "res5"}
node6 = {"node_name": "node6", "executor": "func6", "input": "res4a", "output": "res6,res6a"}
node7 = {"node_name": "node7", "executor": "func7", "input": "res5", "output": "res7"}
node8 = {"node_name": "node8", "executor": "func8", "input": "res6", "output": "res8"}
node9 = {"node_name": "node9", "executor": "func9", "input": "res6a", "output": "res9"}
node10 = {"node_name": "node10", "executor": "func10", "input": "res7,res8,res9", "output": "res10"}
node_list = [node1, node2, node3, node4, node5,
             node6, node7, node8, node9, node10]

import pandas as pd
nodeDF = pd.DataFrame(node_list)

inputs = {"x": 1, "y": 2, "z": 3}
outputs = {}
for idx in range(len(nodeDF)):
    # idx = 0
    node = nodeDF.iloc[idx]
    input_name_list = node["input"].split(",")
    input_values = [outputs[inp] if inp in outputs else inputs[inp] for inp in input_name_list]
    output_func = globals()[node["executor"]]
    delayed_output = delayed(output_func)(*input_values)
    output_num = len(node["output"].split(","))
    if output_num >= 2:
        for sid, output in enumerate(node["output"].split(",")):
            outputs[output] = delayed_output[sid]
    else:
        output = node["output"]
        outputs[output] = delayed_output
final_output_list = list(nodeDF["output"])[len(nodeDF)-1].split(",")
if len(final_output_list) == 1:
    final_output = final_output_list[0]
    final_res = outputs[final_output].compute()
else:
    final_output = [outputs[temp] for temp in final_output_list]
    final_res = dask.compute(*final_output)

print(final_res)
end_time = time.time()
runtime = end_time - start_time
print(runtime)

