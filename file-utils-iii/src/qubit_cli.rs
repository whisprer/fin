use num_complex::Complex64;
use std::env;
use crate::plugins::qubit::*;
use nalgebra::Vector3;

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() != 3 {
        eprintln!("Usage: qubit_cli <prime_real_gauss> <prime_real_z>");
        std::process::exit(1);
    }

    let gauss_real: f64 = args[1].parse().unwrap();
    let z_component: f64 = args[2].parse().unwrap();

    let omega = eisenstein_unit();
    let gauss = Complex64::new(gauss_real, 1.0);        // e.g. 6+i
    let eisenstein = Complex64::new(7.0, 3.0) * omega;   // e.g. 7+3ω

    let fmat = FactorMatrix::new(gauss, eisenstein);

    println!("=== Factor Matrix ===");
    println!("{:?}", fmat.as_matrix());

    println!("=== Real Hamiltonian ===");
    println!("{:?}", fmat.to_real_hamiltonian());

    let bloch_vec = fmat.bloch_vector();
    println!("=== Document Bloch Vector ===");
    println!("{:.4?}", bloch_vec);

    let query_vec = query_bloch_vector(gauss_real, z_component);
    println!("=== Query Bloch Vector ===");
    println!("{:.4?}", query_vec);

    let score = fmat.bloch_alignment_score(&query_vec);
    println!("=== Bloch Alignment Score ===");
    println!("{:.4}", score);
}
