from uncertainties.umath import log
from uncertainties import ufloat, unumpy
import numpy as np
from math import pi
from uncertainties.umath import log
from matplotlib import pyplot as plt
from scipy.stats import linregress
from itertools import chain
import sys

x = []
y = []
for line in sys.stdin:
    if len(line.split()) <= 1:
        continue
    a, b = map(int, line.split())
    x.append(a)
    y.append(1e9 / b)

z = np.polyfit(x, y, 1)
p = np.poly1d(z)
plt.scatter(x, y)
plt.errorbar(x, y, fmt='o')
z = np.polyfit(x, y, 1)
p = np.poly1d(z)
plt.plot([0, 16], [0, 1.6])
slope, intercept, _, _, err = linregress(x, y)
slope_u = ufloat(slope, err)
print(f'sl={slope:.6f}, inter={intercept:.6f}, err={err:.6f}')

plt.ylabel('t⁻¹ (s⁻¹)', fontsize=16)
plt.xlabel('n', fontsize=16)

plt.show()
