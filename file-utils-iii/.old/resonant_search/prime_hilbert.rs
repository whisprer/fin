// src/prime_hilbert.rs

use std::collections::{HashMap, HashSet};
use std::f64; // For sqrt

/// A sparse vector representation where keys are prime numbers (u64)
/// and values are normalized frequencies (f64).
pub type PrimeVector = HashMap<u64, f64>;

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

// Note: In a real project, you would add tests here.