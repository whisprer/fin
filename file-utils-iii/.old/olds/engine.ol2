// src/tokenizer.rs

// Add allow(dead_code) to the unused methods in tokenizer
// #[allow(dead_code)]
// pub fn print_vocab(&self) { ... }
// #[allow(dead_code)]
// pub fn get_token(&self, prime: u64) -> Option<&String> { ... }
// #[allow(dead_code)]
// pub fn get_prime(&self, token: &str) -> Option<&u64> { ... }

use regex::Regex;
use std::collections::HashMap;
// use num_primes::Generator; // Remove this line
// use primal::arith::prime_after; // Remove this line
use primal::Primes; // Import the Primes struct

/// A tokenizer that maps words to unique prime numbers.
pub struct PrimeTokenizer {
    token_to_prime: HashMap<String, u64>,
    prime_to_token: HashMap<u64, String>,
    current_prime: u64,
    word_regex: Regex,
    primal_generator: Primes, // Keep the Primes struct instance
}

impl PrimeTokenizer {
    /// Creates a new `PrimeTokenizer`.
    pub fn new() -> Self {
        let word_regex = Regex::new(r"\b\w+\b").expect("Failed to create word regex");
        PrimeTokenizer {
            token_to_prime: HashMap::new(),
            prime_to_token: HashMap::new(),
            current_prime: 2, // Start with the first prime
            word_regex,
            primal_generator: Primes::all(), // Create a prime number iterator
        }
    }

    /// Tokenizes the input text into a vector of prime numbers.
    pub fn tokenize(&mut self, text: &str) -> Vec<u64> {
        let lower_text = text.to_lowercase();
        let mut primes_list = Vec::new(); // Renamed from 'primes' to avoid shadowing

        for mat in self.word_regex.find_iter(&lower_text) {
            let token = mat.as_str().to_string();
            if !self.token_to_prime.contains_key(&token) {
                // Find the next prime greater than the current_prime using the iterator
                // We skip primes until we find one greater than the current_prime
                let next_p_usize = self.primal_generator
                    .by_ref() // Use by_ref to borrow the iterator mutably
                    // Cast p (usize) to u64 for comparison with self.current_prime (u64)
                    .find(|&p| p as u64 > self.current_prime)
                    .expect("Should always be able to find a next prime"); // Assuming primes are infinite

                // Cast the usize result to u64 before storing
                let next_p = next_p_usize as u64;

                self.token_to_prime.insert(token.clone(), next_p);
                self.prime_to_token.insert(next_p, token.clone());
                self.current_prime = next_p; // Update current_prime to the newly assigned prime
            }
            primes_list.push(*self.token_to_prime.get(&token).unwrap());
        }

        primes_list
    }

    #[allow(dead_code)]
    /// Prints the current vocabulary (token to prime mapping).
    pub fn print_vocab(&self) {
        for (token, prime) in &self.token_to_prime {
            println!("{}: {}", token, prime);
        }
    }

    #[allow(dead_code)]
    /// Returns the token associated with a prime number, if it exists.
    pub fn get_token(&self, prime: u64) -> Option<&String> {
        self.prime_to_token.get(&prime)
    }

    #[allow(dead_code)]
    /// Returns the prime number associated with a token, if it exists.
    pub fn get_prime(&self, token: &str) -> Option<&u64> {
        self.token_to_prime.get(token)
    }
}

// Note: In a real project, you would add tests here.