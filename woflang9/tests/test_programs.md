# WofLang Example Programs

## 1. Calculate the Area of a Circle
# Push radius, calculate π*r²
5          # radius = 5
dup        # duplicate radius (stack: 5 5)
*          # r² (stack: 25)
pi         # push π (stack: 25 π)
*          # π*r² (stack: 78.54...)
.          # show result

## 2. Fibonacci-like Sequence (first 5 terms)
# Generate: 1, 1, 2, 3, 5
1 1        # first two terms (stack: 1 1)
.          # show: 1 1
dup        # (stack: 1 1 1)
swap       # (stack: 1 1 1)
+          # (stack: 1 2)
.          # show: 1 2
dup        # (stack: 1 2 2)
swap       # (stack: 1 2 2)
+          # (stack: 1 3)
.          # show: 1 3
dup        # (stack: 1 3 3)
swap       # (stack: 1 3 3)
+          # (stack: 1 5)
.          # show: 1 5

## 3. Quantum Coin Flip
|0⟩         # create qubit in |0⟩ state
H          # apply Hadamard gate (superposition)
show       # display current state
measure    # collapse to 0 or 1 randomly
.          # show result

## 4. Prime Number Check with Powers
17         # test number
prime_check # check if prime
.          # show result (1 = prime)
drop       # remove result
2 4 pow    # calculate 2⁴ = 16
prime_check # check if 16 is prime
.          # show result (0 = not prime)

## 5. Chemistry Calculation: Molar Mass of Glucose
6          # glucose = C₆H₁₂O₆
6 molecular_weight # get molecular weight
.          # show molar mass

## 6. Fractal Exploration
-0.75 0.1 100   # real, imaginary, iterations
mandelbrot      # check if point is in Mandelbrot set
.              # show result
4              # level 4
sierpinski     # generate Sierpinski triangle

## 7. Cryptographic Hash Chain
42         # starting value
hash       # hash it
.          # show hash
hash       # hash the hash
.          # show double hash
hash       # triple hash
.          # show final result

## 8. Thermodynamics: Convert Temperature and Calculate Energy
25              # 25°C
celsius_to_kelvin # convert to Kelvin
.               # show 298.15 K
dup             # duplicate temperature
gas_constant    # R = 8.314 J/(mol·K)
*               # RT (energy per mole)
.               # show result

## 9. Logical Proof: De Morgan's Law
# Prove: NOT(A AND B) = (NOT A) OR (NOT B)
1 1 and not     # NOT(1 AND 1) = NOT(1) = 0
1 not 1 not or  # (NOT 1) OR (NOT 1) = 0 OR 0 = 0
equivalent      # check if equal
.               # show 1 (true - they're equivalent)

## 10. The Void Contemplation
42             # meaning of life
void_division  # divide by the void
.              # witness infinity
resurrect      # bring back the sacred constants
.              # show the mystical numbers
entropy        # calculate entropy of existence
.              # observe the chaos
chaos          # embrace the randomness
.              # see what emerges