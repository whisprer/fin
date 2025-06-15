// src/entropy.rs

use std::collections::HashMap;
use std::f64; // For log2

// Import new quantum-related types
use crate::quantum_types::{
    VectorComplex,
    mutual_information, calculate_redundancy, calculate_symmetry
};
use num_complex::Complex;

/// Calculates the Shannon entropy of a list of u64 values (prime tokens).
/// Shannon entropy measures the average uncertainty of a random variable's possible outcomes.
/// In this context, it measures the diversity/unpredictability of the prime tokens.
pub fn shannon_entropy(primes: &[u64]) -> f64 {
    if primes.is_empty() {
        return 0.0;
    }

    // Count the occurrences of each prime
    let mut counts = HashMap::new();
    for &prime in primes {
        *counts.entry(prime).or_insert(0) += 1;
    }

    let total_count = primes.len() as f64;
    let mut entropy = 0.0;

    // Calculate entropy using the formula: -sum(p * log2(p))
    for &count in counts.values() {
        let p = count as f64 / total_count;
        entropy -= p * f64::log2(p);
    }

    entropy
}

/// Apply non-Hermitian decay to a quantum state vector
pub fn apply_non_hermitian_decay(
    state_vector: &VectorComplex<f64>, 
    decay_rate: f64, 
    delta_time: f64
) -> VectorComplex<f64> {
    state_vector.iter()
        .map(|c| c * Complex::new((1.0 - decay_rate).powf(delta_time), 0.0))
        .collect()
}

/// Calculate the reversibility between a document vector and historical vectors
pub fn calculate_reversibility(doc_vector: &Vec<f64>, historical_vectors: &[Vec<f64>]) -> f64 {
    if historical_vectors.is_empty() {
        return 1.0; // By default, a vector is fully reversible with itself
    }
    
    historical_vectors.iter()
        .map(|past_vec| mutual_information(doc_vector, past_vec))
        .sum::<f64>() / historical_vectors.len() as f64
}

/// Calculate entropy pressure based on document age and frequency metrics
pub fn entropy_pressure(doc_age: f64, update_frequency: f64, trend_decay: f64) -> f64 {
    update_frequency * trend_decay * doc_age.exp()
}

/// Calculate the buffering capacity of a document vector
pub fn buffering_capacity(doc_vector: &Vec<f64>) -> f64 {
    let redundancy = calculate_redundancy(doc_vector);
    let symmetry = calculate_symmetry(doc_vector);
    redundancy + symmetry
}

/// Apply fragility to a resonance score based on entropy pressure
pub fn apply_fragility(resonance_score: f64, fragility: f64, entropy_pressure: f64) -> f64 {
    resonance_score * (-fragility * entropy_pressure).exp()
}

/// Calculate a persistence score based on thermodynamic parameters
pub fn persistence_score(
    reversibility: f64, 
    entropy_pressure: f64, 
    buffering: f64, 
    fragility: f64
) -> f64 {
    if buffering <= 0.0 {
        return 0.0; // Avoid division by zero
    }
    ((-fragility) * (1.0 - reversibility) * (entropy_pressure / buffering)).exp()
}

/// Calculate a resonant persistence score for a document
pub fn resonant_persistence_score(
    doc_vector: &Vec<f64>,
    historical_vectors: &[Vec<f64>],
    doc_age: f64,
    update_freq: f64,
    trend_decay: f64,
    fragility: f64
) -> f64 {
    let reversibility = calculate_reversibility(doc_vector, historical_vectors);
    let entropy_p = entropy_pressure(doc_age, update_freq, trend_decay);
    let buffering = buffering_capacity(doc_vector);
    
    persistence_score(reversibility, entropy_p, buffering, fragility)
}