# sin/cos/tan basics
0 sin   0 1e-12 expect_approx
0 cos   1 1e-12 expect_approx
0 tan   0 1e-12 expect_approx

# sin(pi/6)=0.5, cos(pi/3)=0.5, tan(pi/4)=1
3.141592653589793 6 / sin   0.5 1e-9 expect_approx
3.141592653589793 3 / cos   0.5 1e-9 expect_approx
1 1 atan2                   0.7853981633974483 1e-9 expect_approx

# inverse domain checks (happy path)
0.5 asin                   0.5235987755982989 1e-9 expect_approx
0.5 acos                   1.0471975511965979 1e-9 expect_approx

# degree/radian helpers
180 deg2rad                3.141592653589793 1e-12 expect_approx
3.141592653589793 rad2deg  180 1e-9 expect_approx

# exponentials / logs
1 exp                      2.718281828459045 1e-12 expect_approx
2.718281828459045 ln       1 1e-9 expect_approx
1000 log10                 3 1e-12 expect_approx
2 8 logb                   3 1e-12 expect_approx  # log base 2 of 8 = 3

# hyperbolics (simple checks)
0 sinh                     0 1e-12 expect_approx
0 cosh                     1 1e-12 expect_approx
0 tanh                     0 1e-12 expect_approx
