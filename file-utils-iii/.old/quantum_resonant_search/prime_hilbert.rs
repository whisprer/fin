// src/prime_hilbert.rs

use std::collections::{HashMap, HashSet};
use std::f64; // For sqrt
use num_complex::Complex;
use crate::quantum_types::{VectorComplex};

/// A sparse vector representation where keys are prime numbers (u64)
/// and values are normalized frequencies (f64).
pub type PrimeVector = HashMap<u64, f64>;

/// A biorthogonal representation with left and right prime vectors
pub struct BiorthogonalVector {
    pub left: PrimeVector,
    pub right: PrimeVector,
}

/// Takes a slice of prime tokens and builds a normalized frequency vector.
///
/// This function counts the occurrences of each prime, calculates the L2 norm
/// of the frequency counts, and then normalizes the counts by dividing by the norm.
pub fn build_vector(primes: &[u64]) -> PrimeVector {
    if primes.is_empty() {
        return HashMap::new();
    }

    // Count the occurrences of each prime
    let mut counts = HashMap::new();
    for &prime in primes {
        *counts.entry(prime).or_insert(0) += 1;
    }

    // Calculate the L2 norm (magnitude) of the frequency counts
    let norm: f64 = f64::sqrt(counts.values().map(|&c| (c * c) as f64).sum());

    // Normalize the frequency counts
    let mut vector = HashMap::new();
    if norm > 0.0 { // Avoid division by zero if the vector is empty or all counts are zero
        for (&prime, &count) in &counts {
            vector.insert(prime, count as f64 / norm);
        }
    }

    vector
}

/// Converts a PrimeVector to a dense vector representation
pub fn to_dense_vector(vector: &PrimeVector, dimension: usize) -> Vec<f64> {
    let mut dense = vec![0.0; dimension];
    
    for (&prime, &value) in vector {
        if prime < dimension as u64 {
            dense[prime as usize] = value;
        }
    }
    
    dense
}

/// Build a complex-valued vector with phase information
pub fn build_complex_vector(primes: &[u64], phases: &[f64]) -> VectorComplex<f64> {
    if primes.is_empty() {
        return Vec::new();
    }

    // Count the occurrences of each prime
    let mut counts = HashMap::new();
    for &prime in primes {
        *counts.entry(prime).or_insert(0) += 1;
    }

    // Calculate the L2 norm
    let norm: f64 = f64::sqrt(counts.values().map(|&c| (c * c) as f64).sum());
    
    // Create a complex vector with phase information
    let mut result = Vec::new();
    
    for (i, &prime) in primes.iter().enumerate() {
        let magnitude = *counts.get(&prime).unwrap_or(&0) as f64 / norm;
        let phase = if i < phases.len() { phases[i] } else { 0.0 };
        let complex_val = Complex::from_polar(magnitude, phase);
        result.push(complex_val);
    }

    result
}

/// Build a biorthogonal representation of a document
pub fn build_biorthogonal_vector(primes: &[u64]) -> BiorthogonalVector {
    let base_vector = build_vector(primes);
    
    // For demonstration, we'll create a right vector with slight variations
    // In a real application, these could represent different aspects of the document
    let mut right_vector = PrimeVector::new();
    
    for (&prime, &value) in &base_vector {
        // Modify the weights slightly for the right vector
        right_vector.insert(prime, value * (1.0 + 0.1 * (prime % 2) as f64));
    }
    
    // Normalize the right vector
    let norm: f64 = f64::sqrt(right_vector.values().map(|&v| v * v).sum());
    if norm > 0.0 {
        for val in right_vector.values_mut() {
            *val /= norm;
        }
    }
    
    BiorthogonalVector {
        left: base_vector,
        right: right_vector,
    }
}

/// Calculates the sparse dot product of two prime-based vectors.
///
/// This function iterates over the union of keys from both vectors and
/// sums the product of corresponding values. It efficiently handles
/// sparse vectors where many prime frequencies are zero.
pub fn dot_product(vec1: &PrimeVector, vec2: &PrimeVector) -> f64 {
    // Get the union of keys from both vectors
    let keys1: HashSet<_> = vec1.keys().collect();
    let keys2: HashSet<_> = vec2.keys().collect();
    let all_keys = keys1.union(&keys2);

    // Calculate the dot product
    let mut dot_prod = 0.0;
    for &key in all_keys {
        let val1 = vec1.get(key).unwrap_or(&0.0);
        let val2 = vec2.get(key).unwrap_or(&0.0);
        dot_prod += val1 * val2;
    }

    dot_prod
}

/// Calculates the biorthogonal score between two biorthogonal vectors
pub fn biorthogonal_score(query: &BiorthogonalVector, doc: &BiorthogonalVector) -> f64 {
    dot_product(&query.left, &doc.right) + dot_product(&query.right, &doc.left)
}

/// Calculates a complex resonance score with both magnitude and phase
pub fn resonance_complex(vec1: &PrimeVector, vec2: &PrimeVector, decay_factor: f64) -> Complex<f64> {
    let dot_real = dot_product(vec1, vec2);
    
    // Use the decay factor as a basis for imaginary component
    Complex::new(dot_real, decay_factor)
}