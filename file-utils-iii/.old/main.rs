// src/main.rs - Enhanced for large-scale crawling

mod tokenizer;
mod entropy;
mod prime_hilbert;
mod engine;
mod crawler;
mod quantum_types;

use engine::ResonantEngine;
use crawler::{Crawler, CrawledDocument};
use std::io::{self, Write};
use std::fs;
use std::path::Path;
use tokio::sync::mpsc;
use url::Url;
use quantum_types::{MatrixComplex, trace};
use num_complex::Complex;
use std::time::{SystemTime, UNIX_EPOCH};

/// Apply a quantum jump event to a document state matrix
pub fn quantum_jump_event(doc_state: &mut MatrixComplex<f64>, jump_operator: MatrixComplex<f64>) {
    // Apply the jump operator to both sides of the density matrix
    // Need to clone the doc_state first
    let result = &jump_operator * &(*doc_state) * &jump_operator.adjoint();
    *doc_state = result;
    
    // Normalize the density matrix by its trace
    let tr = trace(doc_state).re;
    if tr > 0.0 {
        // Apply scaling manually
        for i in 0..doc_state.nrows() {
            for j in 0..doc_state.ncols() {
                doc_state[(i, j)] = doc_state[(i, j)] * Complex::new(1.0/tr, 0.0);
            }
        }
    }
}

/// Helper function to load seed URLs from a file
fn load_urls_from_file(file_path: &str) -> io::Result<Vec<String>> {
    let content = fs::read_to_string(file_path)?;
    let urls = content.lines()
        .map(|line| line.trim().to_string())
        .filter(|line| !line.is_empty() && line.starts_with("http"))
        .collect();
    
    Ok(urls)
}

/// Helper function to ensure data directories exist
fn ensure_data_dirs() -> io::Result<()> {
    let data_dir = Path::new("data");
    if !data_dir.exists() {
        fs::create_dir_all(data_dir)?;
        println!("Created data directory");
    }
    
    let checkpoints_dir = Path::new("data/checkpoints");
    if !checkpoints_dir.exists() {
        fs::create_dir_all(checkpoints_dir)?;
        println!("Created checkpoints directory");
    }
    
    Ok(())
}

#[tokio::main]
async fn main() -> io::Result<()> {
    // Ensure data directories exist
    ensure_data_dirs()?;
    
    println!("=====================================================");
    println!("Initializing Quantum Resonant Search Engine");
    println!("=====================================================");
    
    // Check for existing checkpoint
    let checkpoint_path = "data/checkpoints/latest.checkpoint";
    let resume_from_checkpoint = Path::new(checkpoint_path).exists();
    
    let mut engine = ResonantEngine::new();
    
    if resume_from_checkpoint {
        println!("\nFound existing checkpoint. Would you like to resume? (y/n)");
        print!("> ");
        io::stdout().flush()?;
        
        let mut resume_choice = String::new();
        io::stdin().read_line(&mut resume_choice)?;
        
        if resume_choice.trim().to_lowercase().starts_with('y') {
            engine.load_checkpoint(checkpoint_path)?;
            println!("Resumed from checkpoint with {} documents", engine.len());
        } else {
            println!("Starting fresh index");
        }
    }

    // --- Feature Selection ---
    println!("\nEnable quantum-inspired scoring? (y/n)");
    print!("> ");
    io::stdout().flush()?;
    
    let mut quantum_choice = String::new();
    io::stdin().read_line(&mut quantum_choice)?;
    let use_quantum = quantum_choice.trim().to_lowercase().starts_with('y');
    
    println!("\nEnable persistence theory scoring? (y/n)");
    print!("> ");
    io::stdout().flush()?;
    
    let mut persistence_choice = String::new();
    io::stdin().read_line(&mut persistence_choice)?;
    let use_persistence = persistence_choice.trim().to_lowercase().starts_with('y');
    
    // Set engine features
    engine.set_use_quantum_score(use_quantum);
    engine.set_use_persistence_score(use_persistence);
    
    if use_quantum {
        println!("Quantum-inspired scoring enabled");
    }
    
    if use_persistence {
        println!("Persistence theory scoring enabled");
        
        // Configure persistence parameters
        println!("\nSet fragility parameter (0.1-1.0, default: 0.2):");
        print!("> ");
        io::stdout().flush()?;
        
        let mut fragility_input = String::new();
        io::stdin().read_line(&mut fragility_input)?;
        if let Ok(fragility) = fragility_input.trim().parse::<f64>() {
            if fragility > 0.0 && fragility <= 1.0 {
                engine.set_fragility(fragility);
                println!("Fragility set to {}", fragility);
            }
        }
        
        println!("\nSet entropy weight (0.1-1.0, default: 0.1):");
        print!("> ");
        io::stdout().flush()?;
        
        let mut weight_input = String::new();
        io::stdin().read_line(&mut weight_input)?;
        if let Ok(weight) = weight_input.trim().parse::<f64>() {
            if weight > 0.0 && weight <= 1.0 {
                engine.set_entropy_weight(weight);
                println!("Entropy weight set to {}", weight);
            }
        }
    }

    // --- Crawler Setup ---
    // Only do crawling if we're not starting with enough documents
    if engine.len() < 10 {
        // Channel for crawled documents
        let (doc_sender, mut doc_receiver) = mpsc::channel::<CrawledDocument>(500);

        // Create the crawler instance
        let mut crawler = Crawler::new(doc_sender.clone());

        println!("\nDo you want to:\n1. Use default seed URLs\n2. Load URLs from a file\n3. Specify a single domain to crawl\n4. Skip crawling (use existing index)");
        print!("> ");
        io::stdout().flush()?;
        
        let mut choice = String::new();
        io::stdin().read_line(&mut choice)?;
        let choice = choice.trim();
        
        let seed_urls = match choice {
            "1" => {
                println!("Using default seed URLs based on predefined topics...");
                
                // Create multiple starting points relevant to your interests
                let mut urls = Vec::new();
                
                // Add domain-specific authoritative sites
                urls.push("https://mutable-instruments.net/".to_string());
                urls.push("https://www.modulargrid.net/".to_string());
                urls.push("https://learningsynths.ableton.com/".to_string());
                urls.push("https://doepfer.de/home.htm".to_string());
                urls.push("https://doc.rust-lang.org/book/".to_string());
                urls.push("https://blog.rust-lang.org/".to_string());
                
                urls
            },
            "2" => {
                println!("Enter the path to your URL list file:");
                print!("> ");
                io::stdout().flush()?;
                
                let mut file_path = String::new();
                io::stdin().read_line(&mut file_path)?;
                let file_path = file_path.trim();
                
                println!("Loading URLs from: {}", file_path);
                load_urls_from_file(file_path)?
            },
            "3" => {
                println!("Enter the domain to crawl (e.g., example.com):");
                print!("> ");
                io::stdout().flush()?;
                
                let mut domain = String::new();
                io::stdin().read_line(&mut domain)?;
                let domain = domain.trim();
                
                println!("Stay within this domain only? (y/n)");
                print!("> ");
                io::stdout().flush()?;
                
                let mut stay_choice = String::new();
                io::stdin().read_line(&mut stay_choice)?;
                let stay_in_domain = stay_choice.trim().to_lowercase().starts_with('y');
                
                if stay_in_domain {
                    crawler.set_stay_in_domain(true);
                    println!("Staying within the specified domain");
                }
                
                // Convert to proper URL format
                let base_url = if domain.starts_with("http") {
                    domain.to_string()
                } else {
                    format!("https://{}", domain)
                };

                match Url::parse(&base_url) {
                    Ok(_) => vec![base_url],
                    Err(_) => {
                        println!("Invalid URL: {}. Using default seed URLs instead.", base_url);
                        vec!["https://mutable-instruments.net/".to_string()]
                    }
                }
            },
            "4" => {
                println!("Skipping crawling, using existing index only.");
                vec![]
            },
            _ => {
                println!("Invalid choice. Using default seed URLs.");
                vec!["https://mutable-instruments.net/".to_string()]
            }
        };
        
        if !seed_urls.is_empty() {
            println!("Starting with {} seed URLs", seed_urls.len());
            for url in &seed_urls {
                println!("  - {}", url);
            }
            
            // Configure crawling parameters
            println!("\nHow many pages would you like to crawl? (default: 1000, max: 25000)");
            print!("> ");
            io::stdout().flush()?;
            
            let mut page_limit_input = String::new();
            io::stdin().read_line(&mut page_limit_input)?;
            let page_limit: usize = page_limit_input.trim().parse().unwrap_or(1000);
            let page_limit = page_limit.min(25000);
            
            println!("Maximum crawl depth? (default: 3, higher values follow more links)");
            print!("> ");
            io::stdout().flush()?;
            
            let mut depth_input = String::new();
            io::stdin().read_line(&mut depth_input)?;
            let max_depth: u32 = depth_input.trim().parse().unwrap_or(3);
            
            // Set crawler options
            crawler.set_max_pages(page_limit);
            crawler.set_max_depth(max_depth);
            
            println!("How many concurrent workers? (default: 10, max recommended: 20)");
            print!("> ");
            io::stdout().flush()?;
            
            let mut workers_input = String::new();
            io::stdin().read_line(&mut workers_input)?;
            let num_crawler_workers: usize = workers_input.trim().parse().unwrap_or(10);
            let num_crawler_workers = num_crawler_workers.min(20).max(1); // Ensure between 1-20
            
            println!("Starting web crawling with {} workers, targeting {} pages with max depth {}...", 
                     num_crawler_workers, page_limit, max_depth);
                     
            // Spawn the crawler task
            let crawl_handle = tokio::spawn(async move {
                crawler.crawl(seed_urls, num_crawler_workers).await;
                // Drop the sender when the crawler finishes to signal the indexing loop
                drop(doc_sender);
            });

            // --- Indexing Process ---
            // Process crawled documents as they arrive from the crawler
            let mut indexed_count = 0;
            let start_time = SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .unwrap_or_default()
                .as_secs();
                
            // This loop will run until the doc_sender is dropped in the crawler task
            while let Some(doc) = doc_receiver.recv().await {
                engine.add_crawled_document(doc);
                indexed_count += 1;
                
                // Print progress periodically
                if indexed_count % 10 == 0 {
                     println!("Indexed document. Total indexed: {}", engine.len());
                }
                
                // Save checkpoints periodically
                if indexed_count % 100 == 0 {
                    if let Err(e) = engine.save_checkpoint(checkpoint_path) {
                        eprintln!("Failed to save checkpoint: {}", e);
                    }
                    
                    // Also compress documents to save memory
                    engine.compress_all_documents();
                }

                // Keep the limit if you only want a max index size and stop early
                 if engine.len() >= page_limit {
                     println!("Reached target index size of {}. Stopping crawler.", page_limit);
                     break;
                 }
            }
            println!("Indexing of crawled documents finished. Total indexed: {}", engine.len());
            
            // Calculate crawling stats
            let end_time = SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .unwrap_or_default()
                .as_secs();
            let elapsed_seconds = end_time - start_time;
            println!("Crawling took {} seconds ({:.2} minutes)", 
                elapsed_seconds, elapsed_seconds as f64 / 60.0);
            println!("Average speed: {:.2} pages per second", 
                indexed_count as f64 / elapsed_seconds.max(1) as f64);
                
            // Save final checkpoint
            println!("Saving final checkpoint...");
            if let Err(e) = engine.save_checkpoint(checkpoint_path) {
                eprintln!("Failed to save final checkpoint: {}", e);
            }
            
            // Export index to CSV for external analysis
            if let Err(e) = engine.export_index("data/index_export.csv") {
                eprintln!("Failed to export index: {}", e);
            }

            // Wait for the crawler task to complete
            let _ = crawl_handle.await;
        }
    }

    // --- Search Loop ---
    println!("\nResonant Search Engine is ready. Total documents indexed: {}", engine.len());
    // Only enter the search loop after crawling and indexing are complete
    loop {
        println!("\nEnter your resonant query (or type 'quit' to exit):");
        print!("> ");
        io::stdout().flush()?;

        let mut query = String::new();
        io::stdin().read_line(&mut query)?;
        let query = query.trim();

        if query.eq_ignore_ascii_case("quit") {
            println!("Exiting.");
            break;
        }
        
        if query.eq_ignore_ascii_case("export") {
            println!("Exporting index to CSV...");
            if let Err(e) = engine.export_index("data/index_export.csv") {
                eprintln!("Failed to export index: {}", e);
            }
            continue;
        }
        
        if query.eq_ignore_ascii_case("checkpoint") {
            println!("Saving checkpoint...");
            if let Err(e) = engine.save_checkpoint(checkpoint_path) {
                eprintln!("Failed to save checkpoint: {}", e);
            }
            continue;
        }
        
        if query.eq_ignore_ascii_case("compress") {
            println!("Compressing all documents...");
            engine.compress_all_documents();
            continue;
        }

        if query.is_empty() {
            println!("Query is empty. Please enter a query.");
            continue;
        }

        println!("\nSearching for resonant matches...");
        let results = engine.search(query, 5); // Display top 5 results

        // Apply quantum jump for feedback mechanism
        engine.apply_quantum_jump(query, 0.2);

        println!("\nTop Resonant Matches:");
        if results.is_empty() {
            println!("No results found.");
        } else {
            for (idx, r) in results.iter().enumerate() {
                println!("[{}] {}", idx + 1, r.title);
                println!("    URL:            {}", r.path); // Display URL
                
                // Show standard scores
                println!("    Resonance:      {:.4}", r.resonance);
                println!("    Δ Entropy:      {:.4}", r.delta_entropy);
                println!("    Standard Score: {:.4}", r.score);
                
                // Show extended scores if enabled
                if use_quantum {
                    println!("    Quantum Score:  {:.4}", r.quantum_score);
                }
                
                if use_persistence {
                    println!("    Persist. Score: {:.4}", r.persistence_score);
                }
                
                // Compute combined score based on what's enabled
                let combined_score = if use_quantum && use_persistence {
                    r.score * 0.5 + r.quantum_score * 0.25 + r.persistence_score * 0.25
                } else if use_quantum {
                    r.score * 0.7 + r.quantum_score * 0.3
                } else if use_persistence {
                    r.score * 0.7 + r.persistence_score * 0.3
                } else {
                    r.score
                };
                
                println!("    Combined Score: {:.4}", combined_score);
                println!("    Preview:        {}", r.snippet);
                println!();
            }
        }
        
        // Save checkpoint after each successful search to preserve learning
        if let Err(e) = engine.save_checkpoint(checkpoint_path) {
            eprintln!("Failed to save search checkpoint: {}", e);
        }
    }
    // --- End Search Loop ---
    
    Ok(())
}