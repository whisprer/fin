# 2^10 mod 1000 = 24
2 10 1000 modexp           24 0 expect_approx
# 7^560 mod 561 (Carmichael) consistency check value range (just >0); crude truthiness:
7 560 561 modexp           1 1e-12 expect_approx
