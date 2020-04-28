import os
import numpy as np
import pickle
from table import *


if __name__ == "__main__":
	p = [2, 4, 6, 8]
	n = [1000, 2000, 4000, 8000,
		10000, 20000, 40000, 80000,
		100000, 200000, 400000, 800000,
		1000000, 2000000, 4000000, 8000000,
		10000000]

	name = ['oddeven', 'psrs']
	res = np.zeros((len(name), len(p), len(n)))
	for i in range(len(name)):
		os.system('mpicc ' + name[i] + '.c')
		for j in range(len(p)):
			for k in range(len(n)):
				tmp = []
				for _ in range(3):
					pwd = os.getcwd()
					cmd = 'mpiexec -n ' + str(p[j]) + ' ' + pwd + '/a.out ' + str(n[k])
					# print(cmd)
					r = os.popen(cmd)
					text = r.read()
					r.close()
					tmp.append(float(text))
				res[i, j, k] = np.average(tmp)
				print(p[j], n[k], res[i, j, k])

