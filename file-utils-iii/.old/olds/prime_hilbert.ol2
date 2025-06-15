// src/entropy.rs

use std::collections::HashMap;
use std::f64; // For log2

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

// Note: In a real project, you would add tests here.