// src/main.rs

// Declare the modules
mod tokenizer;
mod entropy;
mod prime_hilbert;
mod engine;

use engine::ResonantEngine;
use std::io::{self, Write}; // Import Write for flush
use std::path::Path;

fn main() {
    println!("Initializing Resonant Search Engine...");
    let mut engine = ResonantEngine::new();

    println!("Enter the directory path to load documents from (e.g., data/):");

    // Get directory path from user
    let mut dir_path = String::new();
    io::stdin().read_line(&mut dir_path)
        .expect("Failed to read line");
    let dir_path = dir_path.trim(); // Remove leading/trailing whitespace

    if dir_path.is_empty() {
        println!("No directory path entered. Exiting.");
        return;
    }

    let path = Path::new(&dir_path);
    if !path.exists() {
        eprintln!("Error: Directory '{}' not found.", dir_path);
        return;
    }
    if !path.is_dir() {
        eprintln!("Error: '{}' is not a directory.", dir_path);
        return;
    }

    println!("Loading documents from '{}'...", dir_path);
    // Load documents from the specified directory.
    if let Err(e) = engine.load_directory(path) {
        eprintln!("Error loading directory '{}': {}", dir_path, e);
        // Exit if we can't load documents, as there's nothing to search
        return;
    }
    println!("Documents loaded.");

    loop { // Allow multiple searches without restarting
        println!("\nEnter your resonant query (or type 'quit' to exit):");
        print!("> "); // Add a prompt
        io::stdout().flush().expect("Failed to flush stdout"); // Flush the prompt

        // Get query from user
        let mut query = String::new();
        io::stdin().read_line(&mut query)
            .expect("Failed to read line");
        let query = query.trim(); // Remove leading/trailing whitespace

        if query.eq_ignore_ascii_case("quit") {
            println!("Exiting.");
            break; // Exit the loop if user types 'quit'
        }

        if query.is_empty() {
            println!("Query is empty. Please enter a query.");
            continue; // Ask for query again if empty
        }

        println!("\nSearching for resonant matches...");
        // Perform search (defaulting to top 3 results)
        let results = engine.search(query, 3);

        println!("\nTop Resonant Matches:");
        if results.is_empty() {
            println!("No results found.");
        } else {
            for (idx, r) in results.iter().enumerate() {
                println!("[{}] {}", idx + 1, r.title);
                println!("    Path:           {}", r.path); // Display the path
                println!("    Resonance:      {:.4}", r.resonance);
                println!("    Δ Entropy:      {:.4}", r.delta_entropy);
                println!("    Combined Score: {:.4}", r.score);
                println!("    Preview:        {}", r.snippet);
                println!(); // Add an empty line between results for readability
            }
        }
    }
}