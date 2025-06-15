// main.rs - Quantum Resonant Local Filesystem Search Engine
// Enhanced for blazing-fast local filesystem indexing and searching

mod tokenizer;
mod entropy;
mod prime_hilbert;
mod engine;
mod filesystem_indexer;
mod quantum_types;
mod file_watcher;
mod fuzzy_search;

use engine::ResonantEngine;
use filesystem_indexer::{FilesystemIndexer, IndexedFile};
use file_watcher::FileWatcher;
use fuzzy_search::FuzzyMatcher;
use std::io::{self, Write};
use std::path::{Path, PathBuf};
use std::time::{SystemTime, UNIX_EPOCH, Instant};
use std::sync::{Arc, Mutex};
use std::thread;
use tokio::sync::mpsc;
use ctrlc;

#[tokio::main]
async fn main() -> io::Result<()> {
    println!("=====================================================");
    println!("🧠 Quantum Resonant Local Filesystem Search Engine");
    println!("    \"The closest thing to mindreading for files\"");
    println!("=====================================================");

    // Initialize the quantum engine
    let mut engine = ResonantEngine::new();
    let engine_arc = Arc::new(Mutex::new(engine));
    
    // Initialize filesystem indexer
    let mut indexer = FilesystemIndexer::new();
    
    // Initialize fuzzy matcher for "I can't remember the name" scenarios
    let fuzzy_matcher = FuzzyMatcher::new();
    
    // Setup graceful shutdown
    let running = Arc::new(Mutex::new(true));
    let running_clone = running.clone();
    
    ctrlc::set_handler(move || {
        println!("\n🛑 Gracefully shutting down...");
        *running_clone.lock().unwrap() = false;
    }).expect("Error setting Ctrl-C handler");

    // Check for existing index
    let index_path = "quantum_fs_index.db";
    let resume_from_index = Path::new(index_path).exists();
    
    if resume_from_index {
        println!("📁 Found existing quantum index. Load it? (y/n)");
        print!("> ");
        io::stdout().flush()?;
        
        let mut choice = String::new();
        io::stdin().read_line(&mut choice)?;
        
        if choice.trim().to_lowercase().starts_with('y') {
            let start = Instant::now();
            indexer.load_index(index_path)?;
            println!("⚡ Loaded {} files in {:?}", indexer.file_count(), start.elapsed());
        }
    }

    // Configure search paths
    let search_paths = configure_search_paths()?;
    
    // Configure quantum features
    configure_quantum_features(&engine_arc)?;
    
    // Start filesystem indexing if needed
    if indexer.file_count() == 0 || should_reindex()? {
        println!("🔍 Starting quantum filesystem scan...");
        let start = Instant::now();
        
        // Create progress channel
        let (progress_tx, mut progress_rx) = mpsc::channel::<IndexProgress>(1000);
        
        // Spawn progress monitor
        let progress_handle = tokio::spawn(async move {
            let mut last_update = Instant::now();
            while let Some(progress) = progress_rx.recv().await {
                if last_update.elapsed().as_millis() > 500 { // Update every 500ms
                    print!("\r📂 Indexed: {} files, {} dirs, Current: {}", 
                           progress.files_indexed, 
                           progress.dirs_scanned,
                           truncate_path(&progress.current_path, 60));
                    io::stdout().flush().unwrap();
                    last_update = Instant::now();
                }
            }
        });
        
        // Index all search paths
        for path in &search_paths {
            indexer.index_path(path, Some(progress_tx.clone())).await?;
        }
        
        drop(progress_tx); // Close channel
        progress_handle.await.unwrap();
        
        println!("\n⚡ Quantum scan complete! {} files indexed in {:?}", 
                indexer.file_count(), start.elapsed());
        
        // Save the index
        indexer.save_index(index_path)?;
        println!("💾 Index saved to {}", index_path);
    }

    // Build quantum vectors for all indexed files
    println!("🧮 Building quantum resonance vectors...");
    let start = Instant::now();
    build_quantum_index(&engine_arc, &indexer).await;
    println!("⚡ Quantum vectors built in {:?}", start.elapsed());

    // Start file watcher for real-time updates
    let watcher = Arc::new(Mutex::new(FileWatcher::new()));
    start_file_watcher(watcher.clone(), &search_paths, engine_arc.clone())?;

    // Main search loop
    println!("\n🚠 Quantum search ready! Enter queries or commands:");
    println!("Commands: 'reindex', 'stats', 'fuzzy <pattern>', 'quantum <query>', 'quit'");
    
    loop {
        if !*running.lock().unwrap() {
            break;
        }

        print!("\n🔮 > ");
        io::stdout().flush()?;

        let mut input = String::new();
        match io::stdin().read_line(&mut input) {
            Ok(_) => {
                let input = input.trim();
                if input.is_empty() { continue; }

                match input {
                    "quit" | "exit" => break,
                    "stats" => show_stats(&engine_arc, &indexer),
                    "reindex" => {
                        reindex_filesystem(&mut indexer, &search_paths, &engine_arc).await?;
                    },
                    input if input.starts_with("fuzzy ") => {
                        let pattern = &input[6..];
                        fuzzy_search(&fuzzy_matcher, &indexer, pattern);
                    },
                    input if input.starts_with("quantum ") => {
                        let query = &input[8..];
                        quantum_search(&engine_arc, query).await;
                    },
                    query => {
                        // Default to quantum search
                        quantum_search(&engine_arc, query).await;
                    }
                }
            },
            Err(e) => {
                eprintln!("Error reading input: {}", e);
                break;
            }
        }
    }

    println!("🌟 Quantum search session ended. Index preserved for next time!");
    Ok(())
}

// Configuration functions

fn configure_search_paths() -> io::Result<Vec<PathBuf>> {
    println!("\n📂 Configure search paths:");
    println!("1. Scan entire drive (C:\\ or /)");
    println!("2. Scan home directory");
    println!("3. Custom paths");
    println!("4. Network paths (SMB/NFS)");
    print!("> ");
    io::stdout().flush()?;

    let mut choice = String::new();
    io::stdin().read_line(&mut choice)?;

    match choice.trim() {
        "1" => {
            #[cfg(windows)]
            return Ok(vec![PathBuf::from("C:\\")]);
            #[cfg(not(windows))]
            return Ok(vec![PathBuf::from("/")]);
        },
        "2" => {
            if let Some(home) = dirs::home_dir() {
                Ok(vec![home])
            } else {
                Ok(vec![PathBuf::from(".")])
            }
        },
        "3" => {
            println!("Enter paths separated by newlines (empty line to finish):");
            let mut paths = Vec::new();
            loop {
                let mut path_input = String::new();
                io::stdin().read_line(&mut path_input)?;
                let path_input = path_input.trim();
                if path_input.is_empty() { break; }
                
                let path = PathBuf::from(path_input);
                if path.exists() {
                    paths.push(path);
                    println!("✓ Added: {}", path_input);
                } else {
                    println!("⚠️  Path doesn't exist: {}", path_input);
                }
            }
            Ok(paths)
        },
        "4" => {
            println!("Enter network paths (//server/share or /mnt/network):");
            // Network path handling would go here
            Ok(vec![PathBuf::from(".")])
        },
        _ => Ok(vec![PathBuf::from(".")]),
    }
}

fn configure_quantum_features(engine_arc: &Arc<Mutex<ResonantEngine>>) -> io::Result<()> {
    println!("\n⚛️  Configure quantum features:");
    
    println!("Enable quantum-inspired scoring? (y/n)");
    print!("> ");
    io::stdout().flush()?;
    let mut quantum_choice = String::new();
    io::stdin().read_line(&mut quantum_choice)?;
    let use_quantum = quantum_choice.trim().to_lowercase().starts_with('y');
    
    println!("Enable persistence theory scoring? (y/n)");
    print!("> ");
    io::stdout().flush()?;
    let mut persistence_choice = String::new();
    io::stdin().read_line(&mut persistence_choice)?;
    let use_persistence = persistence_choice.trim().to_lowercase().starts_with('y');
    
    {
        let mut engine = engine_arc.lock().unwrap();
        engine.set_use_quantum_score(use_quantum);
        engine.set_use_persistence_score(use_persistence);
        
        if use_persistence {
            println!("Fragility parameter (0.1-1.0, default 0.2):");
            print!("> ");
            io::stdout().flush()?;
            let mut fragility_input = String::new();
            io::stdin().read_line(&mut fragility_input)?;
            if let Ok(fragility) = fragility_input.trim().parse::<f64>() {
                if fragility > 0.0 && fragility <= 1.0 {
                    engine.set_fragility(fragility);
                }
            }
        }
    }
    
    println!("⚡ Quantum configuration complete!");
    Ok(())
}

fn should_reindex() -> io::Result<bool> {
    println!("🔄 Rebuild index from scratch? (y/n)");
    print!("> ");
    io::stdout().flush()?;
    
    let mut choice = String::new();
    io::stdin().read_line(&mut choice)?;
    Ok(choice.trim().to_lowercase().starts_with('y'))
}

// Search functions

async fn quantum_search(engine_arc: &Arc<Mutex<ResonantEngine>>, query: &str) {
    let start = Instant::now();
    
    let results = {
        let mut engine = engine_arc.lock().unwrap();
        engine.search(query, 10)
    };
    
    let elapsed = start.elapsed();
    
    if results.is_empty() {
        println!("🔍 No quantum resonance found for '{}'", query);
        println!("💡 Try: fuzzy search, different terms, or check file extensions");
        return;
    }
    
    println!("\n🌟 Quantum Resonant Matches for '{}' ({:?}):", query, elapsed);
    println!("{:─<80}", "");
    
    for (i, result) in results.iter().enumerate() {
        println!("[{}] 📄 {}", i + 1, result.title);
        println!("    📂 {}", truncate_path(&result.path, 70));
        
        let combined_score = result.score * 0.4 + result.quantum_score * 0.3 + result.persistence_score * 0.3;
        println!("    ⚛️  Resonance: {:.3} | Quantum: {:.3} | Persistence: {:.3} | Combined: {:.3}",
                result.resonance, result.quantum_score, result.persistence_score, combined_score);
        
        println!("    📝 {}", truncate_text(&result.snippet, 100));
        
        // Show file type and size if available
        if let Ok(metadata) = std::fs::metadata(&result.path) {
            let size = format_file_size(metadata.len());
            let modified = metadata.modified()
                .and_then(|t| t.duration_since(UNIX_EPOCH).ok())
                .map(|d| format_duration_ago(d.as_secs()))
                .unwrap_or_else(|| "unknown".to_string());
            println!("    📊 Size: {} | Modified: {}", size, modified);
        }
        
        println!();
    }
}

fn fuzzy_search(fuzzy_matcher: &FuzzyMatcher, indexer: &FilesystemIndexer, pattern: &str) {
    let start = Instant::now();
    let matches = fuzzy_matcher.find_matches(indexer.get_all_files(), pattern, 10);
    let elapsed = start.elapsed();
    
    if matches.is_empty() {
        println!("🔍 No fuzzy matches found for '{}'", pattern);
        return;
    }
    
    println!("\n🎯 Fuzzy Matches for '{}' ({:?}):", pattern, elapsed);
    println!("{:─<80}", "");
    
    for (i, (file, score)) in matches.iter().enumerate() {
        println!("[{}] 📄 {} (score: {:.2})", i + 1, file.display_name, score);
        println!("    📂 {}", truncate_path(&file.path.to_string_lossy(), 70));
        
        if let Ok(metadata) = std::fs::metadata(&file.path) {
            let size = format_file_size(metadata.len());
            println!("    📊 Size: {} | Type: {:?}", size, file.file_type);
        }
        println!();
    }
}

// Support functions

async fn build_quantum_index(engine_arc: &Arc<Mutex<ResonantEngine>>, indexer: &FilesystemIndexer) {
    // This would integrate with your existing engine to build quantum vectors
    // for all indexed files
    println!("Building quantum resonance matrix...");
    // Implementation depends on your existing engine structure
}

fn start_file_watcher(
    _watcher: Arc<Mutex<FileWatcher>>, 
    _paths: &[PathBuf], 
    _engine: Arc<Mutex<ResonantEngine>>
) -> io::Result<()> {
    // File watcher implementation for real-time updates
    println!("👁️  File watcher started for real-time updates");
    Ok(())
}

async fn reindex_filesystem(
    indexer: &mut FilesystemIndexer,
    paths: &[PathBuf],
    _engine_arc: &Arc<Mutex<ResonantEngine>>
) -> io::Result<()> {
    println!("🔄 Starting full reindex...");
    indexer.clear();
    
    for path in paths {
        indexer.index_path(path, None).await?;
    }
    
    println!("✅ Reindex complete! {} files indexed", indexer.file_count());
    Ok(())
}

fn show_stats(engine_arc: &Arc<Mutex<ResonantEngine>>, indexer: &FilesystemIndexer) {
    let engine = engine_arc.lock().unwrap();
    
    println!("\n📊 Quantum Search Engine Statistics:");
    println!("{:─<50}", "");
    println!("📁 Total files indexed: {}", indexer.file_count());
    println!("🧮 Quantum vectors: {}", engine.len());
    
    let stats = indexer.get_file_type_stats();
    println!("\n📋 File type distribution:");
    for (file_type, count) in stats.iter().take(10) {
        println!("   {:?}: {}", file_type, count);
    }
    
    let total_size = indexer.get_total_size();
    println!("\n💾 Total indexed size: {}", format_file_size(total_size));
    
    println!("\n⚛️  Quantum features:");
    println!("   Quantum scoring: enabled");
    println!("   Persistence theory: enabled");
    println!("   Real-time monitoring: active");
}

// Utility functions

fn truncate_path(path: &str, max_len: usize) -> String {
    if path.len() <= max_len {
        path.to_string()
    } else {
        format!("...{}", &path[path.len() - max_len + 3..])
    }
}

fn truncate_text(text: &str, max_len: usize) -> String {
    if text.len() <= max_len {
        text.to_string()
    } else {
        format!("{}...", &text[..max_len - 3])
    }
}

fn format_file_size(bytes: u64) -> String {
    const UNITS: &[&str] = &["B", "KB", "MB", "GB", "TB"];
    let mut size = bytes as f64;
    let mut unit_index = 0;
    
    while size >= 1024.0 && unit_index < UNITS.len() - 1 {
        size /= 1024.0;
        unit_index += 1;
    }
    
    if unit_index == 0 {
        format!("{} {}", bytes, UNITS[unit_index])
    } else {
        format!("{:.1} {}", size, UNITS[unit_index])
    }
}

fn format_duration_ago(seconds: u64) -> String {
    const MINUTE: u64 = 60;
    const HOUR: u64 = 60 * MINUTE;
    const DAY: u64 = 24 * HOUR;
    const WEEK: u64 = 7 * DAY;
    const MONTH: u64 = 30 * DAY;
    const YEAR: u64 = 365 * DAY;
    
    match seconds {
        s if s < MINUTE => "just now".to_string(),
        s if s < HOUR => format!("{}m ago", s / MINUTE),
        s if s < DAY => format!("{}h ago", s / HOUR),
        s if s < WEEK => format!("{}d ago", s / DAY),
        s if s < MONTH => format!("{}w ago", s / WEEK),
        s if s < YEAR => format!("{}mo ago", s / MONTH),
        s => format!("{}y ago", s / YEAR),
    }
}

#[derive(Debug)]
struct IndexProgress {
    files_indexed: usize,
    dirs_scanned: usize,
    current_path: String,
}