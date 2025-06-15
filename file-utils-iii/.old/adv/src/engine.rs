// src/engine.rs

use crate::tokenizer::PrimeTokenizer;
use crate::prime_hilbert::{build_vector, dot_product, PrimeVector, build_biorthogonal_vector, BiorthogonalVector, to_dense_vector, resonance_complex, biorthogonal_score};
use crate::entropy::{shannon_entropy, calculate_reversibility, entropy_pressure, buffering_capacity, persistence_score};
use crate::crawler::CrawledDocument;

use std::fs;
use std::path::{Path, PathBuf};
use std::io::{self, Write, Read};
use std::time::{SystemTime, UNIX_EPOCH};
use std::collections::HashMap;
use scraper::Html;
use flate2::write::GzEncoder;
use flate2::read::GzDecoder;
use flate2::Compression;
use num_complex::Complex;

/// Represents a processed document in the engine's index.
struct IndexedDocument {
    title: String,
    text: String,
    compressed_text: Option<Vec<u8>>, // New field for compressed text
    vector: PrimeVector,
    biorthogonal: BiorthogonalVector,
    entropy: f64,
    path: PathBuf,
    timestamp: u64,
    // Persistence theory metrics
    reversibility: f64,
    buffering: f64,
    historical_vectors: Vec<Vec<f64>>,
}

// Add these methods to the IndexedDocument implementation
impl IndexedDocument {
    /// Compress the document text to save memory
    fn compress_text(&mut self) {
        if !self.text.is_empty() && self.compressed_text.is_none() {
            let mut encoder = GzEncoder::new(Vec::new(), Compression::default());
            encoder.write_all(self.text.as_bytes()).unwrap_or_default();
            self.compressed_text = encoder.finish().ok();
            
            // Only clear the text if compression was successful
            if self.compressed_text.is_some() {
                self.text.clear();
            }
        }
    }
    
    /// Decompress the text when needed
    fn decompress_text(&mut self) -> &str {
        if self.text.is_empty() && self.compressed_text.is_some() {
            let compressed = self.compressed_text.as_ref().unwrap();
            let mut decoder = GzDecoder::new(&compressed[..]);
            let mut text = String::new();
            
            if decoder.read_to_string(&mut text).is_ok() {
                self.text = text;
            }
        }
        
        &self.text
    }
    
    /// Get a snippet of the document text
    fn get_snippet(&mut self, max_len: usize) -> String {
        let text = self.decompress_text();
        let snippet_chars: String = text.chars().take(max_len).collect();
        snippet_chars.trim().replace('\n', " ") + "..."
    }
}

/// Represents a search result with scoring details and a snippet.
pub struct SearchResult {
    pub title: String,
    pub resonance: f64,
    pub delta_entropy: f64,
    pub score: f64,
    pub quantum_score: f64,
    pub persistence_score: f64,
    pub snippet: String,
    pub path: String,
}

/// The main search engine struct that manages documents and performs searches.
pub struct ResonantEngine {
    tokenizer: PrimeTokenizer,
    docs: Vec<IndexedDocument>,
    entropy_weight: f64,
    // Quantum and persistence parameters
    fragility: f64,
    trend_decay: f64,
    use_quantum_score: bool,
    use_persistence_score: bool,
}

impl ResonantEngine {
    /// Save the current index state to a file
    pub fn save_checkpoint(&self, path: &str) -> io::Result<()> {
        let mut file = fs::File::create(path)?;
        
        // Write header with metadata
        writeln!(file, "# Resonant Search Engine Checkpoint")?;
        writeln!(file, "# Total documents: {}", self.docs.len())?;
        writeln!(file, "# Timestamp: {}", 
            std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap_or_default()
                .as_secs()
        )?;
        
        // Write document entries
        for doc in &self.docs {
            writeln!(file, "{}\t{}\t{}\t{}\t{}", 
                doc.path.to_string_lossy(), 
                doc.title.replace('\t', " "),
                doc.entropy,
                doc.reversibility,
                doc.timestamp
            )?;
        }
        
        println!("Checkpoint saved to {}", path);
        Ok(())
    }
    
    /// Load a previous checkpoint
    pub fn load_checkpoint(&mut self, path: &str) -> io::Result<()> {
        let content = fs::read_to_string(path)?;
        let mut lines = content.lines();
        
        // Skip header lines
        while let Some(line) = lines.next() {
            if !line.starts_with('#') {
                // Process the first non-header line
                self.process_checkpoint_line(line)?;
                break;
            }
        }
        
        // Process remaining lines
        for line in lines {
            self.process_checkpoint_line(line)?;
        }
        
        println!("Loaded {} documents from checkpoint", self.docs.len());
        Ok(())
    }
    
    /// Process a single line from the checkpoint file
    fn process_checkpoint_line(&mut self, line: &str) -> io::Result<()> {
        let parts: Vec<&str> = line.split('\t').collect();
        if parts.len() < 5 {
            return Err(io::Error::new(
                io::ErrorKind::InvalidData, 
                format!("Invalid checkpoint line: {}", line)
            ));
        }
        
        let url = parts[0];
        let title = parts[1];
        let entropy: f64 = parts[2].parse().unwrap_or(0.0);
        let reversibility: f64 = parts[3].parse().unwrap_or(1.0);
        let timestamp: u64 = parts[4].parse().unwrap_or(0);
        
        // Create a placeholder document to be filled with real content later
        let path = PathBuf::from(url);
        let tokens = self.tokenizer.tokenize("placeholder");
        let vector = build_vector(&tokens);
        let biorthogonal = build_biorthogonal_vector(&tokens);
        let dense_vec = to_dense_vector(&vector, 1000);
        
        self.docs.push(IndexedDocument {
            title: title.to_string(),
            text: String::new(),
            compressed_text: None,
            vector,
            biorthogonal,
            entropy,
            path,
            timestamp,
            reversibility,
            buffering: 0.5, // Default value
            historical_vectors: vec![dense_vec],
        });
        
        Ok(())
    }
    
    /// Compress all documents to save memory
    pub fn compress_all_documents(&mut self) {
        for doc in &mut self.docs {
            doc.compress_text();
        }
        println!("Compressed {} documents", self.docs.len());
    }
    
    /// Export the index to a simple CSV file
    pub fn export_index(&self, path: &str) -> io::Result<()> {
        let mut file = fs::File::create(path)?;
        
        // Write CSV header
        writeln!(file, "url,title,entropy,resonance,persistence")?;
        
        // Write each document
        for doc in &self.docs {
            writeln!(file, "\"{}\",\"{}\",{},{},{}", 
                doc.path.to_string_lossy().replace('"', "\"\""), 
                doc.title.replace('"', "\"\""),
                doc.entropy,
                doc.reversibility,
                doc.buffering
            )?;
        }
        
        println!("Index exported to {}", path);
        Ok(())
    }
    
    /// Creates a new `ResonantEngine`.
    pub fn new() -> Self {
        ResonantEngine {
            tokenizer: PrimeTokenizer::new(),
            docs: Vec::new(),
            entropy_weight: 0.1,
            fragility: 0.2,
            trend_decay: 0.05,
            use_quantum_score: true,
            use_persistence_score: true,
        }
    }

    /// Returns the number of documents in the index.
    pub fn len(&self) -> usize {
        self.docs.len()
    }

    /// Enable or disable quantum scoring
    pub fn set_use_quantum_score(&mut self, enable: bool) {
        self.use_quantum_score = enable;
    }

    /// Enable or disable persistence scoring
    pub fn set_use_persistence_score(&mut self, enable: bool) {
        self.use_persistence_score = enable;
    }

    /// Adds a single local file document to the engine's index.
    #[allow(dead_code)]
    fn add_local_document(&mut self, title: String, text: String, path: PathBuf) {
        let tokens = self.tokenizer.tokenize(&text);
        let vec = build_vector(&tokens);
        let biorthogonal = build_biorthogonal_vector(&tokens);
        let entropy = shannon_entropy(&tokens);
        
        // Convert to dense vector for historical comparisons
        let dense_vec = to_dense_vector(&vec, 1000); // Arbitrary dimension
        
        // Get current timestamp
        let timestamp = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap_or_default()
            .as_secs();
        
        // Calculate persistence metrics
        let reversibility = 1.0; // New document is fully reversible with itself
        let buffering = buffering_capacity(&dense_vec);
        
        self.docs.push(IndexedDocument {
            title,
            text,
            compressed_text: None,
            vector: vec,
            biorthogonal,
            entropy,
            path,
            timestamp,
            reversibility,
            buffering,
            historical_vectors: vec![dense_vec.clone()], // Initialize with current vector
        });
    }

    /// Adds a crawled web document to the engine's index.
    pub fn add_crawled_document(&mut self, doc: CrawledDocument) {
        let tokens = self.tokenizer.tokenize(&doc.text);
        if tokens.is_empty() {
            return;
        }
        
        let vec = build_vector(&tokens);
        let biorthogonal = build_biorthogonal_vector(&tokens);
        let entropy = shannon_entropy(&tokens);
        
        // Convert to dense vector for historical comparisons
        let dense_vec = to_dense_vector(&vec, 1000); // Arbitrary dimension
        
        // Get current timestamp
        let timestamp = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap_or_default()
            .as_secs();
            
        // Calculate persistence metrics
        let reversibility = 1.0; // New document is fully reversible with itself
        let buffering = buffering_capacity(&dense_vec);

        // Store the URL string in the path field
        let doc_path = PathBuf::from(doc.url);

        self.docs.push(IndexedDocument {
            title: doc.title,
            text: doc.text,
            compressed_text: None,
            vector: vec,
            biorthogonal,
            entropy,
            path: doc_path,
            timestamp,
            reversibility,
            buffering,
            historical_vectors: vec![dense_vec.clone()], // Initialize with current vector
        });
    }

    /// Loads and indexes supported files from a directory and its subdirectories recursively.
    #[allow(dead_code)]
    pub fn load_directory<P: AsRef<Path>>(&mut self, folder: P) -> io::Result<()> {
        let path = folder.as_ref();
        if !path.is_dir() {
             return Err(io::Error::new(io::ErrorKind::NotFound, format!("'{}' is not a directory", path.display())));
        }
        self.process_directory_recursive(path)
    }

    /// Recursive helper function to process directories and files.
    #[allow(dead_code)]
    fn process_directory_recursive<P: AsRef<Path>>(&mut self, current_dir: P) -> io::Result<()> {
        for entry in fs::read_dir(current_dir)? {
            let entry = entry?;
            let file_path = entry.path();

            if file_path.is_file() {
                let extension = file_path.extension().and_then(|s| s.to_str()).unwrap_or("").to_lowercase();
                 let title = file_path.file_stem()
                                .and_then(|s| s.to_str())
                                .unwrap_or_else(|| file_path.file_name().and_then(|s| s.to_str()).unwrap_or("unknown file"))
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
                                let fragment = Html::parse_document(&html_string);
                                let text = fragment.root_element().text().collect::<String>();
                                Some(text)
                            }
                            Err(e) => {
                                eprintln!("Error reading {}: {}", file_path.display(), e);
                                None
                            }
                        }
                    }
                    _ => None,
                };

                if let Some(text) = text_content {
                    if !text.trim().is_empty() {
                         self.add_local_document(title, text, file_path);
                    } else {
                        println!("Skipping empty local document after text extraction: {}", file_path.display());
                    }
                }
            } else if file_path.is_dir() {
                if let Err(e) = self.process_directory_recursive(&file_path) {
                    eprintln!("Error traversing directory {}: {}", file_path.display(), e);
                }
            }
        }
        Ok(())
    }

    /// Update document relationships and calculate reversibility
    fn update_document_relationships(&mut self) {
        // Create a copy of all document vectors
        let all_vectors: Vec<Vec<f64>> = self.docs.iter()
            .map(|doc| {
                to_dense_vector(&doc.vector, 1000) // Convert to same-sized dense vectors
            })
            .collect();
        
        // Update reversibility for each document
        for (i, doc) in self.docs.iter_mut().enumerate() {
            // Get all vectors except this document's vector
            let others_vectors: Vec<Vec<f64>> = all_vectors.iter()
                .enumerate()
                .filter(|&(j, _)| j != i) // Skip the current document
                .map(|(_, vec)| vec.clone())
                .collect();
            
            // Update reversibility and add to historical vectors
            if !others_vectors.is_empty() {
                let current_vec = &all_vectors[i];
                doc.reversibility = calculate_reversibility(current_vec, &others_vectors);
                
                // Only keep a reasonable number of historical vectors (e.g., up to 5)
                if doc.historical_vectors.len() < 5 {
                    doc.historical_vectors.push(current_vec.clone());
                }
            }
        }
    }

    /// Calculate quantum score for a document given a query
    fn calculate_quantum_score(&self, query_vec: &PrimeVector, doc: &IndexedDocument) -> f64 {
        // Calculate basic resonance using dot product
        let _basic_resonance = dot_product(query_vec, &doc.vector);
        
        // Calculate complex resonance with decay
        // Use doc age for decay factor - newer documents have less decay
        let now = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap_or_default()
            .as_secs();
        let doc_age = ((now - doc.timestamp) as f64) / (24.0 * 3600.0); // Age in days
        let decay_factor = 0.01 * doc_age.min(100.0); // Cap at 100 days
        
        let complex_res = resonance_complex(query_vec, &doc.vector, decay_factor);
        
        // For biorthogonal scoring
        let query_bio = build_biorthogonal_vector(&self.tokenizer.tokenize_without_update(query_vec.keys().cloned().collect::<Vec<_>>().as_slice()));
        let bio_score = biorthogonal_score(&query_bio, &doc.biorthogonal);
        
        // Combine scores - weight the real part most heavily but consider phase
        let quantum_score = complex_res.re * 0.6 + complex_res.im.abs() * 0.2 + bio_score * 0.2;
        
        quantum_score
    }
    
    /// Calculate persistence score for a document
    fn calculate_persistence_score(&self, query_entropy: f64, doc: &IndexedDocument) -> f64 {
        // Calculate document age in days
        let now = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap_or_default()
            .as_secs();
        let doc_age = ((now - doc.timestamp) as f64) / (24.0 * 3600.0); // Age in days
        
        // Calculate update frequency (using a default value for now)
        let update_frequency = 0.1; // Lower means less frequent updates
        
        // Get the current vector for the document
        let _current_vec = to_dense_vector(&doc.vector, 1000);
        
        // Calculate persistence score using the thermodynamic model
        let persistence = persistence_score(
            doc.reversibility,
            entropy_pressure(doc_age, update_frequency, self.trend_decay),
            doc.buffering,
            self.fragility
        );
        
        // Adjust based on entropy delta with query
        let entropy_delta = (doc.entropy - query_entropy).abs();
        let entropy_factor = (-entropy_delta * self.entropy_weight).exp();
        
        persistence * entropy_factor
    }

    /// Performs a search query against the indexed documents.
    /// Returns a vector of `SearchResult`s, sorted by score in descending order.
    pub fn search(&mut self, query: &str, top_k: usize) -> Vec<SearchResult> {
        // First update document relationships to ensure reversibility is current
        self.update_document_relationships();
        
        let query_tokens = self.tokenizer.tokenize(query);
        if query_tokens.is_empty() {
            return Vec::new();
        }
        
        let query_vec = build_vector(&query_tokens);
        let query_entropy = shannon_entropy(&query_tokens);

        // First get all the scores without using 'self' inside the closure
        let mut results: Vec<SearchResult> = Vec::new();
        
        // Process each document individually to avoid borrowing conflicts
        for doc in &mut self.docs {
            // Standard resonance score
            let resonance = dot_product(&query_vec, &doc.vector);
            let delta_entropy = (doc.entropy - query_entropy).abs();
            let standard_score = resonance - delta_entropy * self.entropy_weight;
            
            // Quantum-inspired score
            let quantum_score = if self.use_quantum_score {
                // Calculate directly instead of calling self.method()
                // Begin quantum score calculation (copied from calculate_quantum_score)
                let now = SystemTime::now()
                    .duration_since(UNIX_EPOCH)
                    .unwrap_or_default()
                    .as_secs();
                let doc_age = ((now - doc.timestamp) as f64) / (24.0 * 3600.0); // Age in days
                let decay_factor = 0.01 * doc_age.min(100.0); // Cap at 100 days
                
                let complex_res = resonance_complex(&query_vec, &doc.vector, decay_factor);
                
                // For biorthogonal scoring
                let query_bio = build_biorthogonal_vector(&self.tokenizer.tokenize_without_update(query_vec.keys().cloned().collect::<Vec<_>>().as_slice()));
                let bio_score = biorthogonal_score(&query_bio, &doc.biorthogonal);
                
                // Combine scores - weight the real part most heavily but consider phase
                complex_res.re * 0.6 + complex_res.im.abs() * 0.2 + bio_score * 0.2
                // End quantum score calculation
            } else {
                0.0
            };
            
            // Persistence theory score
            let persistence_score = if self.use_persistence_score {
                // Calculate directly instead of calling self.method()
                // Begin persistence score calculation (copied from calculate_persistence_score)
                let now = SystemTime::now()
                    .duration_since(UNIX_EPOCH)
                    .unwrap_or_default()
                    .as_secs();
                let doc_age = ((now - doc.timestamp) as f64) / (24.0 * 3600.0); // Age in days
                
                // Calculate update frequency (using a default value for now)
                let update_frequency = 0.1; // Lower means less frequent updates
                
                // Calculate persistence score using the thermodynamic model
                let persistence = persistence_score(
                    doc.reversibility,
                    entropy_pressure(doc_age, update_frequency, self.trend_decay),
                    doc.buffering,
                    self.fragility
                );
                
                // Adjust based on entropy delta with query
                let entropy_delta = (doc.entropy - query_entropy).abs();
                let entropy_factor = (-entropy_delta * self.entropy_weight).exp();
                
                persistence * entropy_factor
                // End persistence score calculation
            } else {
                0.0
            };
            
            // Generate snippet
            let snippet = doc.get_snippet(200);

            results.push(SearchResult {
                title: doc.title.clone(),
                resonance,
                delta_entropy,
                score: standard_score,
                quantum_score,
                persistence_score,
                snippet,
                path: doc.path.to_string_lossy().into_owned(),
            });
        }

        // Now sort results based on combined score
        results.sort_by(|a, b| {
            let a_combined = if self.use_quantum_score && self.use_persistence_score {
                a.score * 0.5 + a.quantum_score * 0.25 + a.persistence_score * 0.25
            } else if self.use_quantum_score {
                a.score * 0.7 + a.quantum_score * 0.3
            } else if self.use_persistence_score {
                a.score * 0.7 + a.persistence_score * 0.3
            } else {
                a.score
            };
            
            let b_combined = if self.use_quantum_score && self.use_persistence_score {
                b.score * 0.5 + b.quantum_score * 0.25 + b.persistence_score * 0.25
            } else if self.use_quantum_score {
                b.score * 0.7 + b.quantum_score * 0.3
            } else if self.use_persistence_score {
                b.score * 0.7 + b.persistence_score * 0.3
            } else {
                b.score
            };
            
            b_combined.partial_cmp(&a_combined).unwrap_or(std::cmp::Ordering::Equal)
        });

        results.into_iter().take(top_k).collect()
    }

    // Method to set the entropy weight
    pub fn set_entropy_weight(&mut self, weight: f64) {
        self.entropy_weight = weight;
    }
    
    // Method to set the fragility parameter
    pub fn set_fragility(&mut self, fragility: f64) {
        self.fragility = fragility;
    }
    
    // Method to set the trend decay parameter  
    pub fn set_trend_decay(&mut self, decay: f64) {
        self.trend_decay = decay;
    }
    
    // Apply a quantum jump to the documents (for dynamic updates)
    pub fn apply_quantum_jump(&mut self, query: &str, importance: f64) {
        let query_tokens = self.tokenizer.tokenize(query);
        if query_tokens.is_empty() {
            return;
        }
        
        let query_vec = build_vector(&query_tokens);
        
        // Create a simple Hamiltonian for the system
        for doc in &mut self.docs {
            // Convert vectors to dense format for quantum operations
            let doc_dense = to_dense_vector(&doc.vector, 100);
            let query_dense = to_dense_vector(&query_vec, 100);
            
            // Skip if too small
            if doc_dense.is_empty() || query_dense.is_empty() {
                continue;
            }
            
            // Calculate resonance as overlap
            let resonance = dot_product(&query_vec, &doc.vector);
            
            // If the document resonates with the query, boost its relevance
            if resonance > 0.1 {
                // Add the query vector to the document's historical vectors
                let current_vec = to_dense_vector(&doc.vector, 1000);
                if doc.historical_vectors.len() < 5 {
                    doc.historical_vectors.push(current_vec);
                } else if !doc.historical_vectors.is_empty() {
                    // Replace oldest vector
                    doc.historical_vectors.remove(0);
                    doc.historical_vectors.push(current_vec);
                }
                
                // Increase reversibility based on match strength
                doc.reversibility = doc.reversibility * 0.9 + 0.1 * (resonance * importance);
                
                // Update timestamp to mark it as "fresher"
                let now = SystemTime::now()
                    .duration_since(UNIX_EPOCH)
                    .unwrap_or_default()
                    .as_secs();
                
                // Only update if significantly newer (> 1 day)
                if now - doc.timestamp > 24 * 3600 {
                    doc.timestamp = now - ((now - doc.timestamp) / 2); // Make it "halfway" newer
                }
            }
        }
    }
}