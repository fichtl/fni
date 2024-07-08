# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 2/7/24 10:12 AM

from sklearn.datasets import load_iris
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
import joblib

# 导入数据
iris = load_iris()
x, y = iris.data, iris.target
x_train, x_test, y_train, y_test = train_test_split(x, y)
# 训练并保存模型
clr = RandomForestClassifier()
clr.fit(x_train, y_train)
joblib.dump(clr, 'model.pkl', compress=9)

# pip install skl2onnx
from skl2onnx import convert_sklearn
from skl2onnx.common.data_types import FloatTensorType
clr = joblib.load('model.pkl')
initial_type = [('float_input', FloatTensorType([None, 4]))]
onx = convert_sklearn(clr, initial_types=initial_type)
with open('model.onnx', 'wb') as f:
    f.write(onx.SerializeToString())

# pip install onnxruntime
import onnxruntime as rt
import numpy as np
data = np.array([[5.4, 6.3, 2.6, 7.4], [3.4, 6.2, 7.4, 2.3],[5.2, 6.4, 4.2,5.6]])
sess = rt.InferenceSession('model.onnx')
input_name = sess.get_inputs()[0].name
label_name = sess.get_outputs()[0].name
pred_onx = sess.run([label_name], {input_name: data.astype(np.float32)})[0]
print(pred_onx)
