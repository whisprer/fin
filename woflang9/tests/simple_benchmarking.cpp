# tests/simple_benchmark.wof
# Simple prime benchmarking in pure WofLang

# Clear the stack
stack_slayer

# Test a few small primes
97 prime_check
997 prime_check
9973 prime_check

# All should be 1 (true), so sum should be 3
+ + 
3 =

# If they're equal, push "PASS", otherwise "FAIL"
"PASS" "FAIL" if