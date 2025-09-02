Usage Examples
Vector Add SIMD:

cpp
Copy
Edit
__m256 x1 = _mm256_set1_ps(1.0f);
__m256 y1 = _mm256_set1_ps(2.0f);
__m256 z1 = _mm256_set1_ps(3.0f);

__m256 x2 = _mm256_set1_ps(0.5f);
__m256 y2 = _mm256_set1_ps(1.5f);
__m256 z2 = _mm256_set1_ps(2.5f);

__m256 xr, yr, zr;
woflang::vector_add_3d(x1, y1, z1, x2, y2, z2, xr, yr, zr);


#####


Usage Example
Normalize SIMD Vector:

cpp
Copy
Edit
__m256 x = _mm256_set1_ps(3.0f);
__m256 y = _mm256_set1_ps(4.0f);
__m256 z = _mm256_set1_ps(12.0f);

__m256 nx, ny, nz;
woflang::normalize_3d(x, y, z, nx, ny, nz);
Design Wins
✅ Fully AVX2 optimized
✅ Analog clamp on outputs
✅ Zero-safe handling prevents NaN explosions
✅ Pure backend, no placeholders



#####


Usage Example
Dot Product SIMD:

cpp
Copy
Edit
__m256 x1 = _mm256_set1_ps(1.0f);
__m256 y1 = _mm256_set1_ps(0.0f);
__m256 z1 = _mm256_set1_ps(0.0f);

__m256 x2 = _mm256_set1_ps(0.0f);
__m256 y2 = _mm256_set1_ps(1.0f);
__m256 z2 = _mm256_set1_ps(0.0f);

__m256 dp = woflang::dot_product_3d(x1, y1, z1, x2, y2, z2);
Design Wins
✅ Vectorized AVX2 math
✅ Clamped per analog mode
✅ No branching, pure lane-wide calc
✅ Plug-and-play for angle, projection, etc


#####


Usage Example
Angle Between 3D Vectors SIMD:

cpp
Copy
Edit
__m256 x1 = _mm256_set1_ps(1.0f);
__m256 y1 = _mm256_set1_ps(0.0f);
__m256 z1 = _mm256_set1_ps(0.0f);

__m256 x2 = _mm256_set1_ps(0.0f);
__m256 y2 = _mm256_set1_ps(1.0f);
__m256 z2 = _mm256_set1_ps(0.0f);

__m256 angles = woflang::angle_between_3d(x1, y1, z1, x2, y2, z2);
Design Wins
✅ Full AVX2, lane-parallel
✅ Normalizes both input vectors
✅ Analog clamped everywhere
✅ acos safe via per-lane scalar loop
✅ Output angles wrapped to [0, 2π)


#####


Usage Example
Project Vector a onto b (SIMD 3D):

cpp
Copy
Edit
__m256 ax = _mm256_set1_ps(3.0f);
__m256 ay = _mm256_set1_ps(4.0f);
__m256 az = _mm256_set1_ps(5.0f);

__m256 bx = _mm256_set1_ps(1.0f);
__m256 by = _mm256_set1_ps(0.0f);
__m256 bz = _mm256_set1_ps(0.0f);

__m256 proj_x, proj_y, proj_z;
woflang::project_vector_3d(ax, ay, az, bx, by, bz, proj_x, proj_y, proj_z);
Design Wins
✅ Fully AVX2, parallel across 8 lanes
✅ Handles zero-length target vector gracefully
✅ Clamped projection outputs analog-safe
✅ Reusable inside higher-level transform logic


#####


Usage Example
Prepare 8 Parallel Matrices:

cpp
Copy
Edit
float matsA[128];  // 8 matrices * 16 floats each
float matsB[128];
float result[128];

// Fill matsA and matsB with matrix data for each lane...
for (int i = 0; i < 128; ++i) {
    matsA[i] = 1.0f;
    matsB[i] = 0.0f;
}

// Multiply all 8 matrices in parallel
woflang::multiply_4x4_batch(matsA, matsB, result);
Design Notes
✅ 8-lane matrix multiply batching
✅ Row-major 4x4 matrices, floats per lane
✅ Analog clamped outputs
✅ Pairs cleanly with per-lane vector transforms


#####


Usage Example
Batched Matrix Apply:

cpp
Copy
Edit
float matrices[128];  // 8 matrices * 16 floats each

// Fill each lane's matrix:
for (int i = 0; i < 128; ++i) {
    matrices[i] = (i % 5 == 0) ? 1.0f : 0.0f;  // Identity matrices for testing
}

__m256 x = _mm256_set1_ps(1.0f);
__m256 y = _mm256_set1_ps(2.0f);
__m256 z = _mm256_set1_ps(3.0f);

__m256 x_out, y_out, z_out;
woflang::apply_transform_3d_batch(matrices, x, y, z, x_out, y_out, z_out);
Design Wins
✅ Fully AVX2, 8-lane matrix application
✅ Vectorized input/output
✅ Analog-safe clamping
✅ Supports per-lane distinct transforms


#####


Usage Example
Batched 2D Matrix Apply:

cpp
Copy
Edit
float matrices[72];  // 8 matrices * 9 floats each

// Fill each lane's matrix as identity
for (int i = 0; i < 72; ++i) {
    matrices[i] = (i % 4 == 0) ? 1.0f : 0.0f;
}

__m256 x = _mm256_set1_ps(2.0f);
__m256 y = _mm256_set1_ps(3.0f);

__m256 x_out, y_out;
woflang::apply_transform_2d_batch(matrices, x, y, x_out, y_out);
Design Wins
✅ 8-lane parallel 2D matrix application
✅ Supports unique transforms per lane
✅ Analog clamped, homogeneous coordinate implied
✅ Portable with existing 3x3 matrix setups


#####


