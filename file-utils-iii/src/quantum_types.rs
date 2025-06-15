// src/quantum_types.rs

use nalgebra::{DMatrix, Complex, Matrix2, Vector3};
use num_complex::ComplexFloat;
use num_complex::Complex64;

/// Type alias for complex-valued matrices used in quantum computations
pub type MatrixComplex<T> = DMatrix<Complex<T>>;

/// Type alias for complex-valued vectors used in quantum computations
pub type VectorComplex<T> = Vec<Complex<T>>;

/// Calculate the trace (sum of diagonal elements) of a matrix
pub fn trace(matrix: &MatrixComplex<f64>) -> Complex<f64> {
    let n = matrix.nrows().min(matrix.ncols());
    let mut tr = Complex::new(0.0, 0.0);
    for i in 0..n {
        tr += matrix[(i, i)];
    }
    tr
}

/// Create a quantum density matrix from a state vector
pub fn density_matrix(state: &VectorComplex<f64>) -> MatrixComplex<f64> {
    let n = state.len();
    let mut rho = MatrixComplex::zeros(n, n);
    
    for i in 0..n {
        for j in 0..n {
            rho[(i, j)] = state[i] * state[j].conj();
        }
    }
    
    rho
}

/// Calculate mutual information between two probability distributions
pub fn mutual_information(p1: &[f64], p2: &[f64]) -> f64 {
    // Ensure both distributions have the same length
    if p1.len() != p2.len() {
        return 0.0;
    }
    
    let n = p1.len();
    let mut joint_entropy = 0.0;
    let mut entropy_p1 = 0.0;
    let mut entropy_p2 = 0.0;
    
    // Calculate entropies
    for i in 0..n {
        if p1[i] > 0.0 {
            entropy_p1 -= p1[i] * p1[i].log2();
        }
        if p2[i] > 0.0 {
            entropy_p2 -= p2[i] * p2[i].log2();
        }
        
        // Simplified joint entropy calculation assuming independence
        let joint_p = p1[i] * p2[i];
        if joint_p > 0.0 {
            joint_entropy -= joint_p * joint_p.log2();
        }
    }
    
    // Mutual information is the sum of individual entropies minus joint entropy
    entropy_p1 + entropy_p2 - joint_entropy
}

/// Calculate redundancy in a vector (how many repeated elements)
pub fn calculate_redundancy(vec: &[f64]) -> f64 {
    let n = vec.len();
    if n <= 1 {
        return 0.0;
    }
    
    let mut count_map = std::collections::HashMap::new();
    for &val in vec {
        *count_map.entry(format!("{:.6}", val)).or_insert(0) += 1;
    }
    
    // Calculate redundancy as the proportion of elements that are duplicates
    let unique_elements = count_map.len();
    let redundancy = 1.0 - (unique_elements as f64 / n as f64);
    
    redundancy
}

/// Calculate symmetry in a vector (how close it is to being symmetric around its midpoint)
pub fn calculate_symmetry(vec: &[f64]) -> f64 {
    let n = vec.len();
    if n <= 1 {
        return 1.0; // Single element is perfectly symmetric
    }
    
    let mut symmetry_score = 0.0;
    let half_len = n / 2;
    
    for i in 0..half_len {
        let mirror_idx = n - 1 - i;
        let difference = (vec[i] - vec[mirror_idx]).abs();
        let max_val = vec[i].max(vec[mirror_idx]);
        
        // Normalize the difference
        if max_val > 0.0 {
            symmetry_score += 1.0 - (difference / max_val);
        } else {
            symmetry_score += 1.0; // Both values are 0, perfect symmetry
        }
    }
    
    // Normalize to 0-1 range
    symmetry_score / half_len as f64
}

/// Create a Hamiltonian for a quantum system
pub fn create_hamiltonian(energy_levels: &[f64], coupling: f64) -> MatrixComplex<f64> {
    let n = energy_levels.len();
    let mut h = MatrixComplex::zeros(n, n);
    
    // Set diagonal elements to energy levels
    for i in 0..n {
        h[(i, i)] = Complex::new(energy_levels[i], 0.0);
    }
    
    // Add coupling between adjacent levels
    for i in 0..(n-1) {
        h[(i, i+1)] = Complex::new(coupling, 0.0);
        h[(i+1, i)] = Complex::new(coupling, 0.0);
    }
    
    h
}

/// Create a dissipator operator for quantum master equations
pub fn create_dissipator(size: usize, target: usize, rate: f64) -> MatrixComplex<f64> {
    let mut l = MatrixComplex::zeros(size, size);
    
    // Simple decay operator targeting a specific state
    if target < size {
        l[(0, target)] = Complex::new(rate.sqrt(), 0.0);
    }
    
    l
}

/// Models the quantum Lindblad evolution of a density matrix using quantum master equation
pub fn lindblad_evolution(
    state: MatrixComplex<f64>, 
    coherent_h: MatrixComplex<f64>, 
    dissipators: Vec<MatrixComplex<f64>>, 
    dt: f64
) -> MatrixComplex<f64> {
    // Calculate unitary (coherent) part: -i[H, ρ]
    let unitary_part = &coherent_h * &state - &state * coherent_h.adjoint();
    
    // Apply -i factor (have to do manual element-wise multiplication for complex scaling)
    let mut i_scaled_unitary = unitary_part.clone();
    for i in 0..i_scaled_unitary.nrows() {
        for j in 0..i_scaled_unitary.ncols() {
            let value = i_scaled_unitary[(i, j)];
            i_scaled_unitary[(i, j)] = Complex::new(-value.im, value.re); // Multiply by -i
        }
    }
    
    // Initialize dissipative part
    let mut dissipative_part = MatrixComplex::zeros(state.nrows(), state.ncols());

    // Add each dissipator's contribution
    for l in dissipators {
        // L ρ L† - 1/2 (L†L ρ + ρ L†L)
        let l_adj = l.adjoint();
        let l_adj_l = &l_adj * &l;
        
        let term1 = &l * &state * &l_adj;
        let term2 = &l_adj_l * &state;
        let term3 = &state * &l_adj_l;
        
        // Manually scale the half_term
        let mut half_term = &term2 + &term3;
        for i in 0..half_term.nrows() {
            for j in 0..half_term.ncols() {
                half_term[(i, j)] = half_term[(i, j)] * Complex::new(0.5, 0.0);
            }
        }
        
        dissipative_part += term1 - half_term;
    }

    // Scale by dt and add to state
    let mut scaled_evolution = &i_scaled_unitary + &dissipative_part;
    for i in 0..scaled_evolution.nrows() {
        for j in 0..scaled_evolution.ncols() {
            scaled_evolution[(i, j)] = scaled_evolution[(i, j)] * Complex::new(dt, 0.0);
        }
    }

    // Full evolution: ρ + (unitary_part + dissipative_part) * dt
    state + scaled_evolution
}

#[derive(Debug, Clone)]
pub struct FactorMatrix {
    pub alpha: Complex64,       // Gaussian factor
    pub alpha_conj: Complex64,  // Conjugate
    pub beta: Complex64,        // Eisenstein factor
    pub beta_conj: Complex64,   // Conjugate
}

impl FactorMatrix {
    pub fn new(alpha: Complex64, beta: Complex64) -> Self {
        Self {
            alpha,
            alpha_conj: alpha.conj(),
            beta,
            beta_conj: beta.conj(),
        }
    }

    pub fn as_matrix(&self) -> Matrix2<Complex64> {
        Matrix2::new(self.alpha, self.beta, self.beta_conj, self.alpha_conj)
    }

    pub fn to_real_hamiltonian(&self) -> Matrix2<f64> {
        let a_real = self.alpha.re;
        let x = a_real;
        Matrix2::new(-1.0, x, x, 1.0)
    }

    pub fn bloch_vector(&self) -> Vector3<f64> {
        let x = self.alpha.re;
        Vector3::new(x, 0.0, 1.0).normalize()
    }

    pub fn bloch_alignment_score(&self, other: &Vector3<f64>) -> f64 {
        let self_vec = self.bloch_vector();
        self_vec.dot(other)
    }
}

pub fn eisenstein_unit() -> Complex64 {
    Complex64::new(-0.5, (3.0_f64).sqrt() / 2.0)
}

pub fn eisenstein_unit_squared() -> Complex64 {
    Complex64::new(-0.5, -(3.0_f64).sqrt() / 2.0)
}

pub fn query_bloch_vector(real_component: f64, z_component: f64) -> Vector3<f64> {
    Vector3::new(real_component, 0.0, z_component).normalize()
}
