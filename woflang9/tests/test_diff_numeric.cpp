# central diff on x^2 at x=3 ≈ 6
"dup *" 3 1e-5 diff        6 1e-4 expect_approx

# sdiff on exp at x=2 ≈ e^2
"exp" 2 sdiff              7.38905609893065 1e-6 expect_approx

# second derivative: d^2/dx^2 (x^2) = 2
"dup *" 4 1e-4 diff2       2 1e-3 expect_approx

# forward/backward sanity on sin at 0 => cos(0)=1
"sin" 0 1e-6 diff_fwd      1 1e-4 expect_approx
"sin" 0 1e-6 diff_bwd      1 1e-4 expect_approx
