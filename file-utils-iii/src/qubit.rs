use num_complex::Complex64;
use nalgebra::{Matrix2, Vector3};

#[derive(Debug, Clone)]
pub struct FactorMatrix {
    pub alpha: Complex64,       // Gaussian factor (e.g., 6 + i)
    pub alpha_conj: Complex64,  // Conjugate of Gaussian (e.g., 6 - i)
    pub beta: Complex64,        // Eisenstein factor (e.g., 7 + 3ω)
    pub beta_conj: Complex64,   // Conjugate (e.g., 7 + 3ω²)
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
        let x = a_real; // shortcut using Gaussian real part
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
