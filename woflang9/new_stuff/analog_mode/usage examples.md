How to Use (Hardcoded Ready, No Placeholders)
Example Setting Modes:

cpp
Copy
Edit
using namespace woflang;

set_analog_mode(AnalogMode::INT_201);           // Classic -100 to +100
set_analog_mode(AnalogMode::INT_2001);          // Finer -1000 to +1000
set_analog_mode(AnalogMode::FLOAT_UNIT);        // Analog-like float -1.0 to +1.0
set_analog_mode(AnalogMode::FLOAT_CUSTOM, -5.5f, 12.25f);  // Your own range
Example SIMD Clamp Application:

cpp
Copy
Edit
__m256 data = _mm256_set_ps(-200.0f, -150.0f, 0.0f, 25.0f, 99.0f, 150.0f, 500.0f, -500.0f);

set_analog_mode(AnalogMode::INT_201);
data = clamp_analog(data);
// Now all values clamped to [-100, 100]
Scalar Clamp Example:

cpp
Copy
Edit
float v = 250.0f;
set_analog_mode(AnalogMode::FLOAT_UNIT);
v = clamp_analog(v);  // Now v is clamped to [-1.0, +1.0]
Future-Proof Extending
Add AnalogMode::INT_10001 etc for even finer control

Vector width adapts—just switch _mm256_* to _mm512_* for AVX512 lanes

Extend to fixed-point support with minimal tweaks

Fren, you happy if I roll next step as the core math operators, e.g.:

Analog add, sub, mul, div

Auto-clamping versions of each

SIMD and scalar supported, directly working off this mode logic

OR you wanna inject this in your codebase now and we move module-by-module together?


#####


Usage Examples
Scalar:

cpp
Copy
Edit
using namespace woflang;

set_analog_mode(AnalogMode::FLOAT_UNIT);

float x = 0.9f;
float y = 0.5f;
float z = analog_add(x, y);  // Will clamp to +1.0f
SIMD Vector:

cpp
Copy
Edit
__m256 vec1 = _mm256_set1_ps(0.8f);
__m256 vec2 = _mm256_set1_ps(0.6f);

set_analog_mode(AnalogMode::FLOAT_UNIT);
__m256 result = analog_add(vec1, vec2);  // All lanes clamp to +1.0f
Division Safety:

cpp
Copy
Edit
float safe = analog_div(50.0f, 0.0f);  // No crash, clamps to 0.0 within current mode
High-Precision Mode Example:

cpp
Copy
Edit
set_analog_mode(AnalogMode::INT_2001);  // -1000 to +1000

float a = 900.0f;
float b = 200.0f;
float c = analog_add(a, b);  // Clamped to +1000
Design Notes
All math ops auto-clamp to the defined analog range, ensures analog safety

SIMD and scalar both behave identically from logic standpoint

Div by zero gracefully handled, fallback to 0 (can be modified if you prefer NaN handling)



#####



Usage Examples
Negation:

cpp
Copy
Edit
float x = 75.0f;
float y = analog_negate(x);  // Clamped to analog min/max

__m256 vec = _mm256_set1_ps(50.0f);
vec = analog_negate(vec);
Absolute Value:

cpp
Copy
Edit
float a = -123.4f;
float b = analog_abs(a);  // Positive and clamped
Sqrt with Negative Safety:

cpp
Copy
Edit
float z = analog_sqrt(-50.0f);  // Outputs 0.0f safely
Power Example:

cpp
Copy
Edit
float base = 2.0f, exp = 3.0f;
float res = analog_pow(base, exp);  // 8.0f, clamped if needed
Sin/Cos Examples:

cpp
Copy
Edit
float theta = 0.5f;
float s = analog_sin(theta);
float c = analog_cos(theta);


#####


Usage Examples
Basic Trig:

cpp
Copy
Edit
using namespace woflang;

set_analog_mode(AnalogMode::FLOAT_UNIT);

float angle = 0.75f;
float t = analog_tan(angle);
float s = analog_sin(angle);
float c = analog_cos(angle);
Inverse Trig with Safety Clamps:

cpp
Copy
Edit
float val = 1.2f;
float asin_res = analog_asin(val);  // Input clamped to valid [-1,1], output clamped to analog mode
Vector Example (AVX2):

cpp
Copy
Edit
__m256 angles = _mm256_set_ps(0.1f, 0.2f, -0.3f, 0.5f, -0.5f, 0.75f, 1.0f, -1.0f);
__m256 sines = analog_sin(angles);
__m256 tangents = analog_tan(angles);
Atan2 Vector Example:

cpp
Copy
Edit
__m256 y_vals = _mm256_set1_ps(1.0f);
__m256 x_vals = _mm256_set1_ps(0.0f);
__m256 angles = analog_atan2(y_vals, x_vals);  // Handles 90-degree logic safely
What’s Covered
✅ Full scalar and SIMD for:

sin, cos, tan

asin, acos, atan, atan2

✅ Input domain clamping for inverse functions
✅ Output clamped to analog mode range
✅ Safe for INT and FLOAT modes



#####



Usage Examples
2D Scalar Distance:

cpp
Copy
Edit
float dist = woflang::analog_distance_2d(10.0f, 20.0f, 13.0f, 24.0f);  // Euclidean dist, clamped to analog range
3D Scalar Distance:

cpp
Copy
Edit
float d3 = woflang::analog_distance_3d(0.0f, 0.0f, 0.0f, 3.0f, 4.0f, 0.0f);  // Should output 5.0, clamped
SIMD Hypotenuse:

cpp
Copy
Edit
__m256 a = _mm256_set1_ps(3.0f);
__m256 b = _mm256_set1_ps(4.0f);
__m256 hyp = woflang::analog_hypot(a, b);  // All lanes = 5.0
SIMD 2D Distance:

cpp
Copy
Edit
__m256 x1 = _mm256_set1_ps(1.0f);
__m256 y1 = _mm256_set1_ps(2.0f);
__m256 x2 = _mm256_set1_ps(4.0f);
__m256 y2 = _mm256_set1_ps(6.0f);
__m256 d2 = woflang::analog_distance_2d(x1, y1, x2, y2);  // All lanes get 5.0
SIMD 3D Distance Example:

cpp
Copy
Edit
__m256 x1 = _mm256_set1_ps(0.0f);
__m256 y1 = _mm256_set1_ps(0.0f);
__m256 z1 = _mm256_set1_ps(0.0f);
__m256 x2 = _mm256_set1_ps(1.0f);
__m256 y2 = _mm256_set1_ps(2.0f);
__m256 z2 = _mm256_set1_ps(2.0f);
__m256 d3 = woflang::analog_distance_3d(x1, y1, z1, x2, y2, z2);  // sqrt(1+4+4)=3, clamped to analog
Design Strengths
✅ Fully clamped to analog mode range
✅ Works for all analog modes (int, float, custom)
✅ SIMD parallel-friendly, AVX2-backed
✅ Scalar versions portable & safe


#####


Usage Examples
Scalar Conversion:

cpp
Copy
Edit
using namespace woflang;

float deg = 90.0f;
float rad = degrees_to_radians(deg);  // ~1.5708

float back_to_deg = radians_to_degrees(rad);  // Should get ~90.0f
SIMD Vector Conversion:

cpp
Copy
Edit
__m256 angles_deg = _mm256_set_ps(0.0f, 30.0f, 45.0f, 60.0f, 90.0f, 180.0f, 270.0f, 360.0f);
__m256 angles_rad = degrees_to_radians(angles_deg);

__m256 back_to_deg = radians_to_degrees(angles_rad);
Notes
✅ Clamped to current analog mode range
✅ SIMD-accelerated for batch processing
✅ Accurate, portable, clean


#####


Usage Examples
Scalar Wrapping:

cpp
Copy
Edit
float rad = 7.0f;  // Slightly over 2π
float wrapped_rad = woflang::wrap_radians(rad);  // Should wrap to [0, 2π)

float deg = 450.0f;
float wrapped_deg = woflang::wrap_degrees(deg);  // Should wrap to 90°
SIMD Wrapping:

cpp
Copy
Edit
__m256 angles_rad = _mm256_set_ps(-1.0f, 0.0f, 1.0f, 3.0f, 6.5f, 10.0f, -5.0f, 20.0f);
__m256 wrapped_rad = woflang::wrap_radians(angles_rad);

__m256 angles_deg = _mm256_set_ps(-90.0f, 0.0f, 45.0f, 90.0f, 360.0f, 720.0f, -450.0f, 1080.0f);
__m256 wrapped_deg = woflang::wrap_degrees(angles_deg);
Why Important
✅ Keeps angles clean within [0, 2π) or [0°, 360°)
✅ Handles negatives properly
✅ SIMD safe, clean for big batches
✅ Fully analog-mode safe with clamping


#####


Usage Examples
Scalar Symmetric Wrap:

cpp
Copy
Edit
float rad = 4.0f;  // > π
float wrapped = woflang::wrap_radians_symmetric(rad);  // Should land between [-π, π)

float deg = 270.0f;
float wrapped_deg = woflang::wrap_degrees_symmetric(deg);  // Should wrap to -90°
SIMD Symmetric Wrap:

cpp
Copy
Edit
__m256 angles_rad = _mm256_set_ps(-7.0f, 0.0f, 1.0f, 3.0f, 6.5f, 10.0f, -5.0f, 20.0f);
__m256 wrapped_rad = woflang::wrap_radians_symmetric(angles_rad);

__m256 angles_deg = _mm256_set_ps(-450.0f, 0.0f, 45.0f, 90.0f, 360.0f, 720.0f, -450.0f, 1080.0f);
__m256 wrapped_deg = woflang::wrap_degrees_symmetric(angles_deg);
Design Wins
✅ Clean symmetric wraps for direction-sensitive math
✅ Supports negatives and overshoots
✅ Fully SIMD safe
✅ Honors analog mode clamping


#####


Usage Examples
Scalar Polar to Cartesian:

cpp
Copy
Edit
float r = 5.0f;
float angle = woflang::degrees_to_radians(45.0f);
float x, y;

woflang::polar_to_cartesian(r, angle, x, y);  // x,y filled with clamped Cartesian coords
Scalar Cartesian to Polar:

cpp
Copy
Edit
float radius, angle_rad;
woflang::cartesian_to_polar(3.0f, 4.0f, radius, angle_rad);  // Outputs radius=5, angle ≈ 0.927 rad
SIMD Polar to Cartesian Example:

cpp
Copy
Edit
__m256 radii = _mm256_set1_ps(5.0f);
__m256 angles = _mm256_set1_ps(woflang::degrees_to_radians(30.0f));
__m256 xs, ys;

woflang::polar_to_cartesian(radii, angles, xs, ys);
SIMD Cartesian to Polar Example:

cpp
Copy
Edit
__m256 x = _mm256_set1_ps(3.0f);
__m256 y = _mm256_set1_ps(4.0f);
__m256 radius, angle;

woflang::cartesian_to_polar(x, y, radius, angle);  // radius lanes = 5.0, angle ≈ 0.927 rad
Tight Features
✅ Full clamped outputs to analog range
✅ Angles wrapped properly for clean cyclical math
✅ SIMD-friendly for vector ops
✅ Portable scalar fallback


###


Usage Examples
2D Dot Product Scalar:

cpp
Copy
Edit
float dp = woflang::dot_product_2d(1.0f, 2.0f, 3.0f, 4.0f);  // (1*3)+(2*4)=11, clamped
3D Dot Product Scalar:

cpp
Copy
Edit
float dp3 = woflang::dot_product_3d(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);  // 1*4 + 2*5 + 3*6 = 32
Vector Length Scalar:

cpp
Copy
Edit
float mag = woflang::magnitude_2d(3.0f, 4.0f);  // Outputs 5.0, clamped
SIMD Dot Product 2D:

cpp
Copy
Edit
__m256 x1 = _mm256_set1_ps(1.0f);
__m256 y1 = _mm256_set1_ps(2.0f);
__m256 x2 = _mm256_set1_ps(3.0f);
__m256 y2 = _mm256_set1_ps(4.0f);

__m256 dp = woflang::dot_product_2d(x1, y1, x2, y2);  // All lanes = 11.0
SIMD Magnitude 3D:

cpp
Copy
Edit
__m256 x = _mm256_set1_ps(3.0f);
__m256 y = _mm256_set1_ps(4.0f);
__m256 z = _mm256_set1_ps(12.0f);

__m256 mag = woflang::magnitude_3d(x, y, z);  // Outputs 13.0 across lanes
Next Expansion Options
✅ These prep for:

Vector normalization

Full 2D/3D rotation matrix operations

Projection math (dot projections, angle between vectors)

Matrix multiply chains


#####


Usage Examples
Normalize 2D Scalar:

cpp
Copy
Edit
float x = 3.0f, y = 4.0f, nx, ny;
woflang::normalize_2d(x, y, nx, ny);  // nx = 0.6, ny = 0.8, clamped
Normalize 3D Scalar:

cpp
Copy
Edit
float nx, ny, nz;
woflang::normalize_3d(3.0f, 4.0f, 12.0f, nx, ny, nz);  // Outputs unit vector
Angle Between 2D Vectors Scalar:

cpp
Copy
Edit
float angle = woflang::angle_between_2d(1.0f, 0.0f, 0.0f, 1.0f);  // Should be ~π/2
SIMD Normalization Example:

cpp
Copy
Edit
__m256 x = _mm256_set1_ps(3.0f);
__m256 y = _mm256_set1_ps(4.0f);
__m256 nx, ny;

woflang::normalize_2d(x, y, nx, ny);
SIMD Angle Between Vectors:

cpp
Copy
Edit
__m256 x1 = _mm256_set1_ps(1.0f);
__m256 y1 = _mm256_set1_ps(0.0f);
__m256 x2 = _mm256_set1_ps(0.0f);
__m256 y2 = _mm256_set1_ps(1.0f);

__m256 angle = woflang::angle_between_2d(x1, y1, x2, y2);


#####


Usage Examples
Scalar 2D Projection:

cpp
Copy
Edit
float ax = 3.0f, ay = 4.0f;
float bx = 1.0f, by = 0.0f;
float proj_x, proj_y;

woflang::project_vector_2d(ax, ay, bx, by, proj_x, proj_y);  // Projects (3,4) onto (1,0)
Scalar 3D Projection:

cpp
Copy
Edit
float px, py, pz;
woflang::project_vector_3d(3.0f, 4.0f, 5.0f, 0.0f, 1.0f, 0.0f, px, py, pz);
SIMD 2D Projection:

cpp
Copy
Edit
__m256 ax = _mm256_set1_ps(3.0f);
__m256 ay = _mm256_set1_ps(4.0f);
__m256 bx = _mm256_set1_ps(1.0f);
__m256 by = _mm256_set1_ps(0.0f);
__m256 proj_x, proj_y;

woflang::project_vector_2d(ax, ay, bx, by, proj_x, proj_y);
SIMD 3D Projection Example:

cpp
Copy
Edit
__m256 ax = _mm256_set1_ps(1.0f);
__m256 ay = _mm256_set1_ps(2.0f);
__m256 az = _mm256_set1_ps(3.0f);
__m256 bx = _mm256_set1_ps(0.0f);
__m256 by = _mm256_set1_ps(1.0f);
__m256 bz = _mm256_set1_ps(0.0f);
__m256 proj_x, proj_y, proj_z;

woflang::project_vector_3d(ax, ay, az, bx, by, bz, proj_x, proj_y, proj_z);
Design Notes
✅ Fully clamped projection results
✅ Graceful handling of degenerate (zero-length) target vectors
✅ Scalar and SIMD implementations aligned
✅ Full analog range safety


#####


Usage Examples
Create and Multiply 2x2 Matrices:

cpp
Copy
Edit
woflang::Matrix2x2 matA, matB;
matA.set(1.0f, 2.0f, 3.0f, 4.0f);
matB.set(0.5f, 0.0f, 0.0f, 0.5f);

woflang::Matrix2x2 result = woflang::multiply(matA, matB);
Create and Multiply 3x3 Matrices:

cpp
Copy
Edit
woflang::Matrix3x3 matC, matD;
matC.set(
    1.0f, 2.0f, 3.0f,
    0.0f, 1.0f, 4.0f,
    5.0f, 6.0f, 0.0f
);

matD.set(
    -2.0f, 1.0f, 2.0f,
    3.0f, 0.0f, -1.0f,
    4.0f, 2.0f, 0.0f
);

woflang::Matrix3x3 product = woflang::multiply(matC, matD);
Design Notes
✅ Clean 2x2 and 3x3 matrix structs
✅ Clamp every element post-mult, analog-safe
✅ Identity matrices by default for easy ops
✅ Fast multiply, ready for future SIMD versions



#####


Usage Examples
Build Rotation Matrix & Rotate Vector:

cpp
Copy
Edit
float angle = woflang::degrees_to_radians(45.0f);
woflang::Matrix2x2 rot = woflang::rotation_matrix_2d(angle);

float x = 1.0f, y = 0.0f, xr, yr;
woflang::rotate_vector_2d(rot, x, y, xr, yr);  // Rotates (1,0) by 45°
Quick Inline Rotate:

cpp
Copy
Edit
float x = 0.0f, y = 1.0f, xr, yr;
woflang::rotate_vector_2d(x, y, woflang::degrees_to_radians(90.0f), xr, yr);
Next Steps Prep
✅ Analog clamped
✅ 2D rotation matrix builder
✅ Rotate vectors inline or via reusable matrix


####


Usage Examples
Build Transform Matrix (Rotate + Translate):

cpp
Copy
Edit
float angle = woflang::degrees_to_radians(30.0f);
float tx = 5.0f, ty = 2.0f;

woflang::Matrix3x3 tmat = woflang::transform_matrix_2d(angle, tx, ty);
Apply to Vector (Transforms):

cpp
Copy
Edit
float x = 1.0f, y = 0.0f, xr, yr;

woflang::apply_transform_2d(tmat, x, y, xr, yr);  // Rotates by 30°, translates by (5,2)
Apply Pure Rotation Only:

cpp
Copy
Edit
float rot_x, rot_y;
woflang::apply_rotation_2d(tmat, x, y, rot_x, rot_y);  // Rotation without translation
Design Wins
✅ Full 3x3 matrix with translation support
✅ Clamped outputs analog-mode safe
✅ Homogeneous coordinate system compliant
✅ Clean matrix reuse for chained transforms


#####


Usage Examples
Chain Two Matrices:

cpp
Copy
Edit
woflang::Matrix3x3 m1, m2;
m1.set(
    1.0f, 0.0f, 5.0f,
    0.0f, 1.0f, 2.0f,
    0.0f, 0.0f, 1.0f
);

float angle = woflang::degrees_to_radians(45.0f);
m2 = woflang::transform_matrix_2d(angle, 0.0f, 0.0f);

woflang::Matrix3x3 combined = woflang::multiply_chain(m1, m2);
Chain Three or More Matrices:

cpp
Copy
Edit
woflang::Matrix3x3 m3 = woflang::transform_matrix_2d(woflang::degrees_to_radians(30.0f), 1.0f, 1.0f);

woflang::Matrix3x3 final = woflang::multiply_chain(m1, m2, m3);
Design Notes
✅ Full analog-mode clamped results
✅ Clean recursive template allows arbitrary chains
✅ Preps easy sequencing of rotations, translations, scalings


#####


Usage Examples
Basic Translation Matrix 3D:

cpp
Copy
Edit
woflang::Matrix4x4 trans = woflang::translation_matrix_3d(3.0f, 2.0f, 1.0f);
Rotation Around Z Axis:

cpp
Copy
Edit
float angle = woflang::degrees_to_radians(90.0f);
woflang::Matrix4x4 rotZ = woflang::rotation_matrix_z_3d(angle);
Chained Transform Example:

cpp
Copy
Edit
woflang::Matrix4x4 move = woflang::translation_matrix_3d(5.0f, 0.0f, 0.0f);
woflang::Matrix4x4 spin = woflang::rotation_matrix_y_3d(woflang::degrees_to_radians(45.0f));
woflang::Matrix4x4 combined = woflang::multiply(move, spin);
Apply to Vector:

cpp
Copy
Edit
float x = 1.0f, y = 0.0f, z = 0.0f, x_out, y_out, z_out;
woflang::apply_transform_3d(combined, x, y, z, x_out, y_out, z_out);
What’s Covered
✅ Full 4x4 matrix struct
✅ Translation and axis rotations in 3D
✅ Clamped results per analog mode
✅ Clean apply-to-vector logic


#####



Usage Example
Switch Modes:

cpp
Copy
Edit
woflang::analog_state.set_mode(woflang::AnalogMode::FLOAT_UNIT);
woflang::analog_state.set_mode(woflang::AnalogMode::FLOAT_CUSTOM, -5.0f, 5.0f);
Clamp Value:

cpp
Copy
Edit
float clamped = woflang::analog_state.clamp(250.0f);
Check Integer Mode:

cpp
Copy
Edit
if (woflang::analog_state.is_integer_mode()) {
    // Integer math rules apply
}
Design Wins
✅ Singleton analog settings
✅ Live min/max accessible everywhere
✅ Safe clamps for math ops
✅ Supports dynamic mode swaps


#####


Interpreter Hook Examples
Setting Mode from Script:

cpp
Copy
Edit
woflang::set_analog_mode_from_string("int2001");
woflang::set_analog_mode_from_string("float_custom", -10.0f, 10.0f);
Getting Current Status:

cpp
Copy
Edit
std::string status = woflang::get_analog_status();
std::cout << status << std::endl;
Error Handling:

cpp
Copy
Edit
try {
    woflang::set_analog_mode_from_string("unknown_mode");
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
Interpreter Integration Suggestion
If your scripting layer uses string commands, you can:

cpp
Copy
Edit
// Inside your command dispatch system:
if (cmd == "analog_status") {
    std::cout << woflang::get_analog_status() << std::endl;
}
if (cmd == "analog_mode") {
    std::string mode;
    float min = -1.0f, max = 1.0f;
    iss >> mode >> min >> max;
    woflang::set_analog_mode_from_string(mode, min, max);
}


#####


Usage Example
In REPL:

yaml
Copy
Edit
woflang> analog_status
Analog Mode: int201
Min: -100
Max: 100

woflang> analog_mode float_custom -5 5
Analog mode updated.

woflang> analog_status
Analog Mode: float_custom (-5 to 5)
Min: -5
Max: 5
Design Wins
✅ Analog mode fully script-controllable
✅ Live REPL interaction
✅ Float custom range supported
✅ Safe error handling for invalid modes


#####


New Commands Available
Clamp a Value to Current Analog Range:

shell
Copy
Edit
woflang> analog_clamp 250
Clamped Value: 100   (in int201 mode)
Get Analog Min/Max Directly:

makefile
Copy
Edit
woflang> analog_range
Min: -100
Max: 100
Remap a Value Between Ranges with Analog Clamp:

shell
Copy
Edit
woflang> analog_remap 5 0 10 -100 100
Remapped Value: 0
Design Wins
✅ More math tools at REPL
✅ Range remapping analog-clamped
✅ Quick clamping debug
✅ Error handling for invalid remaps


#####


New Commands Explained
Apply Deadzone to Value:

shell
Copy
Edit
woflang> analog_deadzone 0.05 0.1
Deadzone Applied: 0
Query Current Analog Scale Range:

mathematica
Copy
Edit
woflang> analog_scalequery
Analog Range Span: 200
Check if Integer Analog Mode:

yaml
Copy
Edit
woflang> analog_isint
Integer Mode: Yes
Possible Next Quick Adds
Normalize raw input to [0,1]


#####



