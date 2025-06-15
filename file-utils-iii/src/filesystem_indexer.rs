// src/filesystem_indexer.rs - Blazing fast filesystem indexing with metadata extraction

use std::collections::{HashMap, BTreeMap};
use std::path::{Path, PathBuf};
use std::fs::{self, Metadata};
use std::io::{self, Read};
use std::time::{SystemTime, UNIX_EPOCH};
use tokio::sync::mpsc;
use walkdir::{WalkDir, DirEntry};
use regex::Regex;
use flate2::write::GzEncoder;
use flate2::read::GzDecoder;
use flate2::Compression;
use serde::{Serialize, Deserialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum FileType {
    Text,
    Code,
    Document,
    Image,
    Audio,
    Video,
    Archive,
    Binary,
    Config,
    Data,
    Log,
    Markdown,
    Unknown,
}

impl FileType {
    fn from_extension(ext: &str) -> Self {
        match ext.to_lowercase().as_str() {
            // Text files
            "txt" | "rtf" => FileType::Text,
            
            // Code files
            "rs" | "py" | "js" | "ts" | "cpp" | "c" | "h" | "hpp" | "java" | "cs" | 
            "go" | "rb" | "php" | "swift" | "kt" | "scala" | "clj" | "hs" | "ml" |
            "elm" | "ex" | "exs" | "erl" | "pl" | "r" | "m" | "lua" | "dart" | "nim" => FileType::Code,
            
            // Documents  
            "pdf" | "doc" | "docx" | "odt" | "rtf" | "tex" | "epub" => FileType::Document,
            
            // Images
            "jpg" | "jpeg" | "png" | "gif" | "bmp" | "svg" | "webp" | "tiff" | "ico" => FileType::Image,
            
            // Audio
            "mp3" | "wav" | "flac" | "ogg" | "aac" | "m4a" | "wma" => FileType::Audio,
            
            // Video
            "mp4" | "avi" | "mkv" | "mov" | "wmv" | "flv" | "webm" | "m4v" => FileType::Video,
            
            // Archives
            "zip" | "rar" | "7z" | "tar" | "gz" | "bz2" | "xz" | "dmg" => FileType::Archive,
            
            // Config files
            "json" | "yaml" | "yml" | "toml" | "ini" | "conf" | "cfg" | "xml" => FileType::Config,
            
            // Data files
            "csv" | "tsv" | "xlsx" | "xls" | "ods" | "db" | "sqlite" | "sql" => FileType::Data,
            
            // Logs
            "log" | "out" | "err" => FileType::Log,
            
            // Markdown
            "md" | "markdown" | "mdown" | "mkd" => FileType::Markdown,
            
            _ => FileType::Unknown,
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IndexedFile {
    pub path: PathBuf,
    pub display_name: String,
    pub file_type: FileType,
    pub size: u64,
    pub modified: u64,
    pub created: u64,
    pub content_hash: Option<u64>,
    pub text_content: Option<String>,
    pub compressed_content: Option<Vec<u8>>,
    pub metadata_tags: Vec<String>,
    pub embedding_ready: bool,
}

impl IndexedFile {
    fn new(path: PathBuf, metadata: &Metadata) -> Self {
        let display_name = path.file_name()
            .and_then(|n| n.to_str())
            .unwrap_or("unknown")
            .to_string();
            
        let extension = path.extension()
            .and_then(|e| e.to_str())
            .unwrap_or("");
            
        let file_type = FileType::from_extension(extension);
        
        let modified = metadata.modified()
            .and_then(|t| t.duration_since(UNIX_EPOCH).ok())
            .map(|d| d.as_secs())
            .unwrap_or(0);
            
        let created = metadata.created()
            .and_then(|t| t.duration_since(UNIX_EPOCH).ok())
            .map(|d| d.as_secs())
            .unwrap_or(modified);
        
        Self {
            path,
            display_name,
            file_type,
            size: metadata.len(),
            modified,
            created,
            content_hash: None,
            text_content: None,
            compressed_content: None,
            metadata_tags: Vec::new(),
            embedding_ready: false,
        }
    }
    
    /// Extract text content from the file based on its type
    pub fn extract_text_content(&mut self) -> io::Result<()> {
        match self.file_type {
            FileType::Text | FileType::Code | FileType::Markdown | FileType::Config => {
                self.extract_plain_text()?;
            },
            FileType::Document => {
                self.extract_document_text()?;
            },
            FileType::Log => {
                self.extract_log_content()?;
            },
            _ => {
                // For other file types, try to extract filename and path keywords
                self.extract_metadata_content();
            }
        }
        
        // Compress content if it's large
        if let Some(ref content) = self.text_content {
            if content.len() > 1024 { // 1KB threshold
                self.compress_content();
            }
        }
        
        Ok(())
    }
    
    fn extract_plain_text(&mut self) -> io::Result<()> {
        // Limit file size to avoid memory issues
        if self.size > 10_000_000 { // 10MB limit
            self.text_content = Some(format!("Large file: {} ({} bytes)", 
                                            self.display_name, self.size));
            return Ok(());
        }
        
        let content = fs::read_to_string(&self.path)?;
        
        // Clean and normalize the content
        let cleaned = self.clean_text_content(&content);
        self.text_content = Some(cleaned);
        
        // Generate content hash for change detection
        self.content_hash = Some(self.calculate_content_hash(&content));
        
        Ok(())
    }
    
    fn extract_document_text(&mut self) -> io::Result<()> {
        // For now, just use filename and metadata
        // TODO: Integrate with document parsing libraries
        self.extract_metadata_content();
        Ok(())
    }
    
    fn extract_log_content(&mut self) -> io::Result<()> {
        // For log files, extract last N lines and key patterns
        if self.size > 1_000_000 { // 1MB limit for logs
            self.text_content = Some(format!("Large log file: {} ({} bytes)", 
                                            self.display_name, self.size));
            return Ok(());
        }
        
        let content = fs::read_to_string(&self.path)?;
        
        // Extract error patterns, timestamps, and key information
        let log_summary = self.extract_log_patterns(&content);
        self.text_content = Some(log_summary);
        
        Ok(())
    }
    
    fn extract_metadata_content(&mut self) {
        let mut content = Vec::new();
        
        // Add filename without extension
        if let Some(stem) = self.path.file_stem().and_then(|s| s.to_str()) {
            content.push(stem.to_string());
        }
        
        // Add directory name
        if let Some(parent) = self.path.parent().and_then(|p| p.file_name()).and_then(|n| n.to_str()) {
            content.push(parent.to_string());
        }
        
        // Add file type
        content.push(format!("{:?}", self.file_type));
        
        // Split camelCase and snake_case filenames
        let filename_words = self.extract_filename_words(&self.display_name);
        content.extend(filename_words);
        
        self.text_content = Some(content.join(" "));
    }
    
    fn clean_text_content(&self, content: &str) -> String {
        // Remove excessive whitespace and normalize
        let re = Regex::new(r"\s+").unwrap();
        let cleaned = re.replace_all(content, " ");
        
        // Limit content length
        let max_len = 50_000; // 50KB of text content
        if cleaned.len() > max_len {
            format!("{}...[truncated]", &cleaned[..max_len])
        } else {
            cleaned.to_string()
        }
    }
    
    fn extract_log_patterns(&self, content: &str) -> String {
        let mut patterns = Vec::new();
        
        // Extract error patterns
        let error_re = Regex::new(r"(?i)(error|exception|failed|fatal|panic).*").unwrap();
        for cap in error_re.find_iter(content).take(10) {
            patterns.push(cap.as_str().to_string());
        }
        
        // Extract timestamp patterns
        let time_re = Regex::new(r"\d{4}-\d{2}-\d{2}|\d{2}:\d{2}:\d{2}").unwrap();
        for cap in time_re.find_iter(content).take(5) {
            patterns.push(cap.as_str().to_string());
        }
        
        // Add filename for context
        patterns.push(self.display_name.clone());
        
        patterns.join(" ")
    }
    
    fn extract_filename_words(&self, filename: &str) -> Vec<String> {
        let mut words = Vec::new();
        
        // Split on common separators
        let separators = Regex::new(r"[_\-\.\s]+").unwrap();
        words.extend(separators.split(filename).map(|s| s.to_string()));
        
        // Split camelCase
        let camel_re = Regex::new(r"([a-z])([A-Z])").unwrap();
        let camel_split = camel_re.replace_all(filename, "$1 $2");
        words.extend(camel_split.split_whitespace().map(|s| s.to_string()));
        
        // Filter out empty and very short words
        words.into_iter()
            .filter(|w| w.len() > 1)
            .collect()
    }
    
    fn compress_content(&mut self) {
        if let Some(ref content) = self.text_content {
            let mut encoder = GzEncoder::new(Vec::new(), Compression::default());
            if encoder.write_all(content.as_bytes()).is_ok() {
                if let Ok(compressed) = encoder.finish() {
                    self.compressed_content = Some(compressed);
                    self.text_content = None; // Clear uncompressed content
                }
            }
        }
    }
    
    pub fn get_text_content(&mut self) -> String {
        if let Some(ref content) = self.text_content {
            content.clone()
        } else if let Some(ref compressed) = self.compressed_content {
            // Decompress on demand
            let mut decoder = GzDecoder::new(&compressed[..]);
            let mut content = String::new();
            if decoder.read_to_string(&mut content).is_ok() {
                content
            } else {
                self.display_name.clone()
            }
        } else {
            self.display_name.clone()
        }
    }
    
    fn calculate_content_hash(&self, content: &str) -> u64 {
        use std::collections::hash_map::DefaultHasher;
        use std::hash::{Hash, Hasher};
        
        let mut hasher = DefaultHasher::new();
        content.hash(&mut hasher);
        hasher.finish()
    }
}

#[derive(Debug)]
pub struct IndexProgress {
    pub files_indexed: usize,
    pub dirs_scanned: usize,
    pub current_path: String,
}

pub struct FilesystemIndexer {
    files: HashMap<PathBuf, IndexedFile>,
    file_type_stats: HashMap<FileType, usize>,
    total_size: u64,
    excluded_patterns: Vec<Regex>,
    max_file_size: u64,
}

impl FilesystemIndexer {
    pub fn new() -> Self {
        let excluded_patterns = Self::default_excluded_patterns();
        
        Self {
            files: HashMap::new(),
            file_type_stats: HashMap::new(),
            total_size: 0,
            excluded_patterns,
            max_file_size: 100_000_000, // 100MB default limit
        }
    }
    
    fn default_excluded_patterns() -> Vec<Regex> {
        let patterns = vec![
            // System directories
            r"\.git/",
            r"\.svn/",
            r"\.hg/",
            r"node_modules/",
            r"target/",
            r"build/",
            r"dist/",
            r"\.cargo/",
            
            // OS specific
            r"System Volume Information/",
            r"\$Recycle\.Bin/",
            r"\.Trash/",
            r"\.DS_Store",
            r"Thumbs\.db",
            
            // Temporary files
            r"\.tmp$",
            r"\.temp$",
            r"\.cache/",
            r"\.local/share/Trash/",
            
            // Large binary patterns
            r"\.iso$",
            r"\.dmg$",
            r"\.img$",
            
            // Lock files
            r"\.lock$",
            r"package-lock\.json$",
            r"Cargo\.lock$",
        ];
        
        patterns.into_iter()
            .filter_map(|p| Regex::new(p).ok())
            .collect()
    }
    
    pub async fn index_path(&mut self, root_path: &Path, progress_tx: Option<mpsc::Sender<IndexProgress>>) -> io::Result<()> {
        let mut files_indexed = 0;
        let mut dirs_scanned = 0;
        
        let walker = WalkDir::new(root_path)
            .follow_links(false)
            .max_depth(20) // Reasonable depth limit
            .into_iter();
        
        for entry in walker {
            match entry {
                Ok(entry) => {
                    if entry.file_type().is_dir() {
                        dirs_scanned += 1;
                        
                        // Send progress update for directories
                        if let Some(ref tx) = progress_tx {
                            if dirs_scanned % 100 == 0 {
                                let _ = tx.send(IndexProgress {
                                    files_indexed,
                                    dirs_scanned,
                                    current_path: entry.path().to_string_lossy().to_string(),
                                }).await;
                            }
                        }
                        continue;
                    }
                    
                    if self.should_index_file(&entry) {
                        match self.index_single_file(entry.path()).await {
                            Ok(true) => {
                                files_indexed += 1;
                                
                                // Send progress update for files
                                if let Some(ref tx) = progress_tx {
                                    if files_indexed % 50 == 0 {
                                        let _ = tx.send(IndexProgress {
                                            files_indexed,
                                            dirs_scanned,
                                            current_path: entry.path().to_string_lossy().to_string(),
                                        }).await;
                                    }
                                }
                            },
                            Ok(false) => {
                                // File was skipped, no action needed
                            },
                            Err(e) => {
                                eprintln!("Error indexing {}: {}", entry.path().display(), e);
                            }
                        }
                    }
                },
                Err(e) => {
                    eprintln!("Error walking directory: {}", e);
                }
            }
        }
        
        // Send final progress update
        if let Some(ref tx) = progress_tx {
            let _ = tx.send(IndexProgress {
                files_indexed,
                dirs_scanned,
                current_path: "Indexing complete".to_string(),
            }).await;
        }
        
        Ok(())
    }
    
    async fn index_single_file(&mut self, path: &Path) -> io::Result<bool> {
        let metadata = fs::metadata(path)?;
        
        // Skip files that are too large
        if metadata.len() > self.max_file_size {
            return Ok(false);
        }
        
        let mut indexed_file = IndexedFile::new(path.to_path_buf(), &metadata);
        
        // Extract text content based on file type
        if let Err(e) = indexed_file.extract_text_content() {
            eprintln!("Warning: Could not extract content from {}: {}", path.display(), e);
            // Continue indexing with just metadata
        }
        
        // Update statistics
        *self.file_type_stats.entry(indexed_file.file_type.clone()).or_insert(0) += 1;
        self.total_size += indexed_file.size;
        
        // Store the indexed file
        self.files.insert(path.to_path_buf(), indexed_file);
        
        Ok(true)
    }
    
    fn should_index_file(&self, entry: &DirEntry) -> bool {
        let path_str = entry.path().to_string_lossy();
        
        // Check against excluded patterns
        for pattern in &self.excluded_patterns {
            if pattern.is_match(&path_str) {
                return false;
            }
        }
        
        // Skip hidden files on Unix systems
        #[cfg(unix)]
        {
            if let Some(filename) = entry.file_name().to_str() {
                if filename.starts_with('.') && filename.len() > 1 {
                    return false;
                }
            }
        }
        
        // Skip system files on Windows
        #[cfg(windows)]
        {
            if let Ok(metadata) = entry.metadata() {
                use std::os::windows::fs::MetadataExt;
                const FILE_ATTRIBUTE_HIDDEN: u32 = 0x2;
                const FILE_ATTRIBUTE_SYSTEM: u32 = 0x4;
                
                let attrs = metadata.file_attributes();
                if (attrs & FILE_ATTRIBUTE_HIDDEN) != 0 || (attrs & FILE_ATTRIBUTE_SYSTEM) != 0 {
                    return false;
                }
            }
        }
        
        true
    }
    
    pub fn save_index(&self, path: &str) -> io::Result<()> {
        let serialized = bincode::serialize(&self.files)
            .map_err(|e| io::Error::new(io::ErrorKind::Other, e))?;
        
        // Compress the index
        let mut encoder = GzEncoder::new(Vec::new(), Compression::default());
        encoder.write_all(&serialized)?;
        let compressed = encoder.finish()?;
        
        fs::write(path, compressed)?;
        Ok(())
    }
    
    pub fn load_index(&mut self, path: &str) -> io::Result<()> {
        let compressed = fs::read(path)?;
        
        // Decompress the index
        let mut decoder = GzDecoder::new(&compressed[..]);
        let mut serialized = Vec::new();
        decoder.read_to_end(&mut serialized)?;
        
        // Deserialize
        self.files = bincode::deserialize(&serialized)
            .map_err(|e| io::Error::new(io::ErrorKind::Other, e))?;
        
        // Rebuild statistics
        self.rebuild_stats();
        
        Ok(())
    }
    
    fn rebuild_stats(&mut self) {
        self.file_type_stats.clear();
        self.total_size = 0;
        
        for file in self.files.values() {
            *self.file_type_stats.entry(file.file_type.clone()).or_insert(0) += 1;
            self.total_size += file.size;
        }
    }
    
    pub fn search_by_name(&self, pattern: &str) -> Vec<&IndexedFile> {
        let pattern_lower = pattern.to_lowercase();
        
        self.files.values()
            .filter(|file| {
                file.display_name.to_lowercase().contains(&pattern_lower) ||
                file.path.to_string_lossy().to_lowercase().contains(&pattern_lower)
            })
            .collect()
    }
    
    pub fn search_by_content(&mut self, query: &str) -> Vec<&IndexedFile> {
        let query_lower = query.to_lowercase();
        
        self.files.values()
            .filter(|file| {
                // Create a mutable copy to get content
                let mut file_copy = (*file).clone();
                let content = file_copy.get_text_content();
                content.to_lowercase().contains(&query_lower)
            })
            .collect()
    }
    
    pub fn get_files_by_type(&self, file_type: &FileType) -> Vec<&IndexedFile> {
        self.files.values()
            .filter(|file| &file.file_type == file_type)
            .collect()
    }
    
    pub fn get_recently_modified(&self, days: u64) -> Vec<&IndexedFile> {
        let cutoff = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap_or_default()
            .as_secs() - (days * 24 * 3600);
        
        self.files.values()
            .filter(|file| file.modified > cutoff)
            .collect()
    }
    
    pub fn get_large_files(&self, min_size_mb: u64) -> Vec<&IndexedFile> {
        let min_size = min_size_mb * 1024 * 1024;
        
        self.files.values()
            .filter(|file| file.size > min_size)
            .collect()
    }
    
    pub fn update_file(&mut self, path: &Path) -> io::Result<bool> {
        if let Ok(metadata) = fs::metadata(path) {
            let modified = metadata.modified()
                .and_then(|t| t.duration_since(UNIX_EPOCH).ok())
                .map(|d| d.as_secs())
                .unwrap_or(0);
            
            // Check if file needs updating
            if let Some(existing) = self.files.get(path) {
                if existing.modified >= modified {
                    return Ok(false); // No update needed
                }
            }
            
            // Remove old entry if it exists
            if let Some(old_file) = self.files.remove(path) {
                // Update statistics
                if let Some(count) = self.file_type_stats.get_mut(&old_file.file_type) {
                    *count = count.saturating_sub(1);
                }
                self.total_size = self.total_size.saturating_sub(old_file.size);
            }
            
            // Add new entry
            self.index_single_file(path).await
        } else {
            // File was deleted
            self.remove_file(path);
            Ok(true)
        }
    }
    
    pub fn remove_file(&mut self, path: &Path) -> bool {
        if let Some(file) = self.files.remove(path) {
            // Update statistics
            if let Some(count) = self.file_type_stats.get_mut(&file.file_type) {
                *count = count.saturating_sub(1);
            }
            self.total_size = self.total_size.saturating_sub(file.size);
            true
        } else {
            false
        }
    }
    
    pub fn clear(&mut self) {
        self.files.clear();
        self.file_type_stats.clear();
        self.total_size = 0;
    }
    
    // Public getters
    pub fn file_count(&self) -> usize {
        self.files.len()
    }
    
    pub fn get_total_size(&self) -> u64 {
        self.total_size
    }
    
    pub fn get_file_type_stats(&self) -> &HashMap<FileType, usize> {
        &self.file_type_stats
    }
    
    pub fn get_all_files(&self) -> impl Iterator<Item = &IndexedFile> {
        self.files.values()
    }
    
    pub fn get_file_by_path(&self, path: &Path) -> Option<&IndexedFile> {
        self.files.get(path)
    }
    
    pub fn get_files_sorted_by_relevance(&self, query: &str) -> Vec<(&IndexedFile, f64)> {
        let query_lower = query.to_lowercase();
        let query_words: Vec<&str> = query_lower.split_whitespace().collect();
        
        let mut scored_files: Vec<(&IndexedFile, f64)> = self.files.values()
            .map(|file| {
                let score = self.calculate_relevance_score(file, &query_words);
                (file, score)
            })
            .filter(|(_, score)| *score > 0.0)
            .collect();
        
        // Sort by score descending
        scored_files.sort_by(|a, b| b.1.partial_cmp(&a.1).unwrap_or(std::cmp::Ordering::Equal));
        
        scored_files
    }
    
    fn calculate_relevance_score(&self, file: &IndexedFile, query_words: &[&str]) -> f64 {
        let mut score = 0.0;
        
        let file_name_lower = file.display_name.to_lowercase();
        let path_lower = file.path.to_string_lossy().to_lowercase();
        
        for word in query_words {
            // Exact filename match gets highest score
            if file_name_lower.contains(word) {
                score += 10.0;
            }
            
            // Path match gets medium score
            if path_lower.contains(word) {
                score += 5.0;
            }
            
            // File type match
            let file_type_str = format!("{:?}", file.file_type).to_lowercase();
            if file_type_str.contains(word) {
                score += 3.0;
            }
        }
        
        // Boost score for recently modified files
        let now = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap_or_default()
            .as_secs();
        
        let age_days = (now - file.modified) / (24 * 3600);
        if age_days < 7 {
            score *= 1.5; // Recent files get 50% boost
        } else if age_days < 30 {
            score *= 1.2; // Files from last month get 20% boost
        }
        
        score
    }
    
    pub fn get_similar_files(&self, target_file: &IndexedFile) -> Vec<&IndexedFile> {
        self.files.values()
            .filter(|file| {
                file.path != target_file.path &&
                file.file_type == target_file.file_type &&
                (file.size as i64 - target_file.size as i64).abs() < 1024 * 1024 // Within 1MB size
            })
            .collect()
    }
}












