"""
Solve -u'' = f using finite difference on different grids
If f(x) = sin(x) then the exact solution is u(x) = sin(x)
"""
from numpy import pi, sin, linspace, zeros, ones, abs, log
from prettytable import PrettyTable
import matplotlib.pyplot as plt
from tdma import *

# RHS function
def f(x):
    return sin(x)

# exact solution
def uexact(x):
    return sin(x)

# Solve the problem and compute error norm
def error(n):
    xmin, xmax = 0.0, 2.0*pi
    h = (xmax - xmin)/(n - 1)

    x = linspace(xmin,xmax,n)
    u = zeros(n)
    # BC for first and last points
    u[0]  = uexact(x[0])
    u[-1] = uexact(x[-1])

    b = h**2 * f(x[1:-1])
    b[0]  += u[0]
    b[-1] += u[-1]
    u[1:-1] = tdma(2*ones(n-2),-ones(n-2),-ones(n-2),b)
    return h, abs(uexact(x)-u).max()

# Compute error norm for different meshes
h = []; err = []
for n in [20,40,80,160,320,640,1280]:
    h1, err1 = error(n)
    h.append(h1); err.append(err1)

table = PrettyTable(['h','error','rate'])
table.add_row(['%.3f' % h[0], '%.4e' % err[0], '---' ])
for i in range(1,len(h)):
    p = log(err[i-1]/err[i])/log(2)
    table.add_row(['%.3f' % h[i], '%.4e' % err[i], '%.2f' % p])
print(table)

# Plot error norm vs h
plt.loglog(h, err, 'o-')
plt.xlabel('h')
plt.ylabel('Maximum Error')
plt.show()
