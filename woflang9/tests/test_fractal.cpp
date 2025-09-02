# Mandelbrot: c=0 stays bounded up to max_iter; returns max_iter
0 0 10 mandelbrot          10 0 expect_approx
0 0 100 mandelbrot         100 0 expect_approx

# Julia: pick c=0 (same as z^2), z0=0 stays bounded
0 0 0 0 50 julia           50 0 expect_approx
