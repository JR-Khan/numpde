set term pdf
set out 'sol.pdf'

set xlabel 'x'
set ylabel 'Density'
p 'sol.txt' u 1:2 t 'Numerical' w lp, 'toro5.dat' u 1:2 t 'Exact' w l

set xlabel 'x'
set ylabel 'Velocity'
p 'sol.txt' u 1:3 t 'Numerical' w lp, 'toro5.dat' u 1:4 t 'Exact' w l

set xlabel 'x'
set ylabel 'Pressure'
p 'sol.txt' u 1:4 t 'Numerical' w lp, 'toro5.dat' u 1:3 t 'Exact' w l
