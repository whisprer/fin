# powers: x y pow  (our op expects x y on stack, then pow)
2 10 pow                   1024 1e-12 expect_approx
9 0.5 pow                  3 1e-12 expect_approx

# roots: x n root  (nth root of x)
27 3 root                  3 1e-12 expect_approx
81 4 root                  3 1e-12 expect_approx
9 sqrt                     3 1e-12 expect_approx

# factorial (0..170 range)
0 factorial                1 1e-12 expect_approx
5 factorial                120 1e-12 expect_approx
10 factorial               3628800 1e-12 expect_approx

# gamma
1 gamma                    1 1e-12 expect_approx
0.5 gamma                  1.772453850905516 1e-9 expect_approx   # sqrt(pi)
