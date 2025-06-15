// src/main.rs

mod tokenizer;
mod entropy;
mod prime_hilbert;
mod engine;
mod crawler;

use engine::ResonantEngine;
use crawler::{Crawler, CrawledDocument};
use std::io::{self, Write};
use std::fs;
use tokio::sync::mpsc;
use url::Url;

#[tokio::main]
async fn main() -> io::Result<()> {
    println!("Initializing Resonant Search Engine...");
    let mut engine = ResonantEngine::new();

    // --- Crawler Setup ---
    // Channel for crawled documents
    // The buffer size should be large enough to not block the crawler significantly.
    // Adjust based on expected throughput and memory.
    let (doc_sender, mut doc_receiver) = mpsc::channel::<CrawledDocument>(500);

    // Create the crawler instance
    let crawler = Crawler::new(doc_sender.clone());

    // Define topics for your search engine
    // You can replace these with topics you're actually interested in
    let topics = [
        "modular synthesizer", "analog electronics", 
        "sound design", "eurorack", "diy synthesizer",
        "rust programming", "signal processing",
        "electronic music", "audio engineering", 
        "quantum physics",
    ];

    println!("Do you want to:\n1. Use default seed URLs\n2. Load URLs from a file\n3. Specify a single domain to crawl");
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
            urls.push("https://www.muffwiggler.com/forum/".to_string());
            urls.push("https://learningsynths.ableton.com/".to_string());
            urls.push("https://www.soundonsound.com/techniques/synth-secrets".to_string());
            urls.push("https://doepfer.de/home.htm".to_string());
            urls.push("https://academic.oup.com/journals/search-results?page=1&q=quantum%20physics".to_string());
            urls.push("https://doc.rust-lang.org/book/".to_string());
            urls.push("https://blog.rust-lang.org/".to_string());
            urls.push("https://crates.io/categories/audio".to_string());
            
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
            let content = fs::read_to_string(file_path)?;
            content.lines()
                .map(|line| line.trim().to_string())
                .filter(|line| !line.is_empty() && line.starts_with("http"))
                .collect()
        },
        "3" => {
            println!("Enter the domain to crawl (e.g., example.com):");
            print!("> ");
            io::stdout().flush()?;
            
            let mut domain = String::new();
            io::stdin().read_line(&mut domain)?;
            let domain = domain.trim();
            
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
        _ => {
            println!("Invalid choice. Using default seed URLs.");
            vec!["https://mutable-instruments.net/".to_string()]
        }
    };
    
    println!("Starting with {} seed URLs", seed_urls.len());
    for url in &seed_urls {
        println!("  - {}", url);
    }
    
    // Configure crawling parameters
    println!("\nHow many pages would you like to crawl? (default: 1000)");
    print!("> ");
    io::stdout().flush()?;
    
    let mut page_limit_input = String::new();
    io::stdin().read_line(&mut page_limit_input)?;
    let page_limit: usize = page_limit_input.trim().parse().unwrap_or(1000);
    
    println!("How many concurrent workers? (default: 10, max recommended: 20)");
    print!("> ");
    io::stdout().flush()?;
    
    let mut workers_input = String::new();
    io::stdin().read_line(&mut workers_input)?;
    let num_crawler_workers: usize = workers_input.trim().parse().unwrap_or(10);
    let num_crawler_workers = num_crawler_workers.min(20).max(1); // Ensure between 1-20
    
    println!("Starting web crawling with {} workers, targeting {} pages...", 
             num_crawler_workers, page_limit);
             
    // Spawn the crawler task
    let crawl_handle = tokio::spawn(async move {
        crawler.crawl(seed_urls, num_crawler_workers).await;
        // Drop the sender when the crawler finishes to signal the indexing loop
        drop(doc_sender);
    });
    // --- End Crawler Setup ---


    // --- Indexing Process ---
    // Process crawled documents as they arrive from the crawler
    let mut indexed_count = 0;
    // This loop will run until the doc_sender is dropped in the crawler task
    while let Some(doc) = doc_receiver.recv().await {
        engine.add_crawled_document(doc);
        indexed_count += 1;
        // Print progress periodically, not for every document, to avoid excessive output
        if indexed_count % 100 == 0 {
             println!("Indexed document. Total indexed: {}", engine.len()); // Use public len()
        }

        // Keep the limit if you only want a max index size and stop early
         if engine.len() >= page_limit {
             println!("Reached target index size of {}. Attempting to stop crawler and finish indexing.", page_limit);
             // In a more complex app, you'd signal the crawler task to stop here.
             // For now, we'll break the indexing loop and wait for the crawler to finish its current tasks.
             break;
         }
    }
    println!("Indexing of crawled documents finished. Total indexed: {}", engine.len());
    // --- End Indexing Process ---

    // Wait for the crawler task to complete. This ensures all documents
    // that were sent before the indexing loop potentially broke are processed.
    let _ = crawl_handle.await;

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

        if query.is_empty() {
            println!("Query is empty. Please enter a query.");
            continue;
        }

        println!("\nSearching for resonant matches...");
        let results = engine.search(query, 5); // Display top 5 results

        println!("\nTop Resonant Matches:");
        if results.is_empty() {
            println!("No results found.");
        } else {
            for (idx, r) in results.iter().enumerate() {
                println!("[{}] {}", idx + 1, r.title);
                println!("    URL:            {}", r.path); // Display URL
                println!("    Resonance:      {:.4}", r.resonance);
                println!("    Δ Entropy:      {:.4}", r.delta_entropy);
                println!("    Combined Score: {:.4}", r.score);
                println!("    Preview:        {}", r.snippet);
                println!();
            }
        }
    }
    // --- End Search Loop ---
    
    Ok(())
}