// src/engine.rs

use crate::tokenizer::PrimeTokenizer;
use crate::prime_hilbert::{build_vector, dot_product, PrimeVector};
use crate::entropy::shannon_entropy;

use std::fs;
use std::path::{Path, PathBuf}; // Import PathBuf
use std::io;
use scraper::{Html, Selector}; // Import necessary items from scraper

/// Represents a processed document in the engine's index.
struct IndexedDocument {
    title: String,
    text: String,
    vector: PrimeVector,
    entropy: f64,
    path: PathBuf, // Store the file path
}

/// Represents a search result with scoring details and a snippet.
pub struct SearchResult {
    pub title: String,
    pub resonance: f64,
    pub delta_entropy: f64,
    pub score: f64,
    pub snippet: String,
    pub path: String, // Include the file path for display
}

/// The main search engine struct that manages documents and performs searches.
pub struct ResonantEngine {
    tokenizer: PrimeTokenizer,
    docs: Vec<IndexedDocument>,
    entropy_weight: f64, // Weight for the entropy difference in the score
}

impl ResonantEngine {
    /// Creates a new `ResonantEngine`.
    pub fn new() -> Self {
        ResonantEngine {
            tokenizer: PrimeTokenizer::new(),
            docs: Vec::new(),
            entropy_weight: 0.1, // Default weight based on the Python code
        }
    }

    /// Adds a single document to the engine's index.
    fn add_document(&mut self, title: String, text: String, path: PathBuf) { // Add path parameter
        let tokens = self.tokenizer.tokenize(&text);
        let vec = build_vector(&tokens);
        let entropy = shannon_entropy(&tokens);

        self.docs.push(IndexedDocument {
            title,
            text,
            vector: vec,
            entropy,
            path, // Store the path
        });
    }

    /// Loads and indexes supported files from a directory and its subdirectories recursively.
    /// Currently supports .txt and .html files.
    /// Returns a Result indicating success or an I/O error.
    pub fn load_directory<P: AsRef<Path>>(&mut self, folder: P) -> io::Result<()> {
        // Ensure the path is a directory
        let path = folder.as_ref();
        if !path.is_dir() {
             return Err(io::Error::new(io::ErrorKind::NotFound, format!("'{}' is not a directory", path.display())));
        }

        self.process_directory_recursive(path)
    }

    /// Recursive helper function to process directories and files.
    fn process_directory_recursive<P: AsRef<Path>>(&mut self, current_dir: P) -> io::Result<()> {
        for entry in fs::read_dir(current_dir)? {
            let entry = entry?;
            let file_path = entry.path();

            if file_path.is_file() {
                let extension = file_path.extension().and_then(|s| s.to_str()).unwrap_or("").to_lowercase();
                 let title = file_path.file_stem()
                                .and_then(|s| s.to_str())
                                .unwrap_or_else(|| file_path.file_name().and_then(|s| s.to_str()).unwrap_or("unknown file")) // Use file_name if stem fails
                                .to_string();

                let text_content = match extension.as_str() {
                    "txt" => {
                        match fs::read_to_string(&file_path) {
                            Ok(text) => Some(text),
                            Err(e) => {
                                eprintln!("Error reading {}: {}", file_path.display(), e);
                                None
                            }
                        }
                    }
                    "html" => {
                        match fs::read_to_string(&file_path) {
                            Ok(html_string) => {
                                // Use scraper to extract text from HTML
                                let fragment = Html::parse_document(&html_string);
                                // Select all text nodes and concatenate them
                                let text = fragment.root_element().text().collect::<String>();
                                Some(text)
                            }
                            Err(e) => {
                                eprintln!("Error reading {}: {}", file_path.display(), e);
                                None
                            }
                        }
                    }
                    // Ignore other file types for now
                    _ => {
                        // Optionally, print a message about skipping
                        // println!("Skipping unsupported file type: {}", file_path.display());
                        None
                    }
                };

                if let Some(text) = text_content {
                    // Basic check to avoid indexing empty files after text extraction
                    if !text.trim().is_empty() {
                         self.add_document(title, text, file_path); // Pass file_path
                    } else {
                        println!("Skipping empty document after text extraction: {}", file_path.display());
                    }
                }
            } else if file_path.is_dir() {
                // Recursively call for subdirectories
                if let Err(e) = self.process_directory_recursive(&file_path) {
                    eprintln!("Error traversing directory {}: {}", file_path.display(), e);
                    // Decide how to handle errors in subdirectories - continue or return?
                    // Returning the error would stop the entire process on the first subdirectory error.
                    // Continuing allows processing other directories. Let's continue and just log the error.
                }
            }
        }
        Ok(())
    }


    /// Performs a search query against the indexed documents.
    /// Returns a vector of `SearchResult`s, sorted by score in descending order.
    pub fn search(&mut self, query: &str, top_k: usize) -> Vec<SearchResult> {
        let query_tokens = self.tokenizer.tokenize(query);
        // If query tokens are empty, return no results
         if query_tokens.is_empty() {
             return Vec::new();
         }
        let query_vec = build_vector(&query_tokens);
        let query_entropy = shannon_entropy(&query_tokens);

        let mut results: Vec<SearchResult> = self.docs.iter().map(|doc| {
            let resonance = dot_product(&query_vec, &doc.vector);
            let delta_entropy = (doc.entropy - query_entropy).abs();
            let score = resonance - delta_entropy * self.entropy_weight; // Score calculation

            // Generate snippet
            // Take up to 200 characters to avoid indexing issues with chars() and then trim
            let snippet_chars: String = doc.text.chars().take(200).collect();
            let snippet = snippet_chars.trim().replace('\n', " ") + "..."; // Corrected replace

            SearchResult {
                title: doc.title.clone(),
                resonance,
                delta_entropy,
                score,
                snippet,
                path: doc.path.to_string_lossy().into_owned(), // Include the path
            }
        }).collect();

        // Sort results by score in descending order
        results.sort_by(|a, b| b.score.partial_cmp(&a.score).unwrap_or(std::cmp::Ordering::Equal));

        // Return top_k results
        results.into_iter().take(top_k).collect()
    }

    // Method to set the entropy weight, if needed
    // #[allow(dead_code)] // Add this attribute to silence the dead_code warning
    pub fn set_entropy_weight(&mut self, weight: f64) {
        self.entropy_weight = weight;
    }

     // Add allow(dead_code) to the unused methods in tokenizer
     // #[allow(dead_code)]
     // pub fn print_vocab(&self) { ... }
     // #[allow(dead_code)]
     // pub fn get_token(&self, prime: u64) -> Option<&String> { ... }
     // #[allow(dead_code)]
     // pub fn get_prime(&self, token: &str) -> Option<&u64> { ... }
}

// Note: In a real project, you would add tests here.