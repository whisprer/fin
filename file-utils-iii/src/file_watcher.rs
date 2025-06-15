// src/file_watcher.rs - Real-time filesystem monitoring

use std::path::{Path, PathBuf};
use std::sync::{Arc, Mutex};
use std::time::Duration;
use tokio::time::sleep;
use notify::{Watcher, RecursiveMode, Result as NotifyResult, Event, EventKind};
use tokio::sync::mpsc;

pub struct FileWatcher {
    watcher: Option<notify::RecommendedWatcher>,
    events_tx: Option<mpsc::Sender<FileEvent>>,
}

#[derive(Debug, Clone)]
pub enum FileEvent {
    Created(PathBuf),
    Modified(PathBuf),
    Deleted(PathBuf),
    Renamed { from: PathBuf, to: PathBuf },
}

impl FileWatcher {
    pub fn new() -> Self {
        Self {
            watcher: None,
            events_tx: None,
        }
    }
    
    pub async fn start_watching(
        &mut self, 
        paths: &[PathBuf],
        callback: impl Fn(FileEvent) + Send + 'static
    ) -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
        let (tx, mut rx) = mpsc::channel::<FileEvent>(1000);
        self.events_tx = Some(tx.clone());
        
        // Spawn event processor
        tokio::spawn(async move {
            while let Some(event) = rx.recv().await {
                callback(event);
            }
        });
        
        // Create the watcher
        let tx_clone = tx.clone();
        let mut watcher = notify::recommended_watcher(move |res: NotifyResult<Event>| {
            if let Ok(event) = res {
                if let Some(file_event) = Self::convert_event(event) {
                    let _ = tx_clone.try_send(file_event);
                }
            }
        })?;
        
        // Watch all specified paths
        for path in paths {
            if path.exists() {
                watcher.watch(path, RecursiveMode::Recursive)?;
                println!("👁️  Watching: {}", path.display());
            }
        }
        
        self.watcher = Some(watcher);
        Ok(())
    }
    
    fn convert_event(event: Event) -> Option<FileEvent> {
        match event.kind {
            EventKind::Create(_) => {
                if let Some(path) = event.paths.first() {
                    Some(FileEvent::Created(path.clone()))
                } else {
                    None
                }
            },
            EventKind::Modify(_) => {
                if let Some(path) = event.paths.first() {
                    Some(FileEvent::Modified(path.clone()))
                } else {
                    None
                }
            },
            EventKind::Remove(_) => {
                if let Some(path) = event.paths.first() {
                    Some(FileEvent::Deleted(path.clone()))
                } else {
                    None
                }
            },
            EventKind::Access(_) => None, // Ignore access events to reduce noise
            _ => None,
        }
    }
    
    pub fn stop_watching(&mut self) {
        self.watcher = None;
        self.events_tx = None;
    }
}

// src/fuzzy_search.rs - Advanced fuzzy matching for "I can't remember the name" scenarios

use crate::filesystem_indexer::{IndexedFile, FileType};
use std::collections::HashMap;

pub struct FuzzyMatcher {
    // Weights for different match types
    exact_weight: f64,
    prefix_weight: f64,
    substring_weight: f64,
    soundex_weight: f64,
    levenshtein_weight: f64,
}

impl FuzzyMatcher {
    pub fn new() -> Self {
        Self {
            exact_weight: 10.0,
            prefix_weight: 8.0,
            substring_weight: 5.0,
            soundex_weight: 3.0,
            levenshtein_weight: 2.0,
        }
    }
    
    pub fn find_matches<'a>(
        &self,
        files: impl Iterator<Item = &'a IndexedFile>,
        query: &str,
        max_results: usize
    ) -> Vec<(&'a IndexedFile, f64)> {
        let query_lower = query.to_lowercase();
        let query_words: Vec<&str> = query_lower.split_whitespace().collect();
        
        let mut matches: Vec<(&IndexedFile, f64)> = files
            .map(|file| {
                let score = self.calculate_fuzzy_score(file, &query_lower, &query_words);
                (file, score)
            })
            .filter(|(_, score)| *score > 0.0)
            .collect();
        
        // Sort by score descending
        matches.sort_by(|a, b| b.1.partial_cmp(&a.1).unwrap_or(std::cmp::Ordering::Equal));
        
        matches.into_iter().take(max_results).collect()
    }
    
    fn calculate_fuzzy_score(&self, file: &IndexedFile, query: &str, query_words: &[&str]) -> f64 {
        let mut total_score = 0.0;
        
        // Get searchable text from file
        let searchable_text = self.get_searchable_text(file);
        let filename_lower = file.display_name.to_lowercase();
        let path_lower = file.path.to_string_lossy().to_lowercase();
        
        // Score against filename
        total_score += self.score_text_match(&filename_lower, query, query_words) * 2.0; // Filename gets double weight
        
        // Score against full path
        total_score += self.score_text_match(&path_lower, query, query_words);
        
        // Score against extracted content
        total_score += self.score_text_match(&searchable_text, query, query_words) * 0.5;
        
        // Bonus for file type relevance
        total_score += self.score_file_type_relevance(file, query_words);
        
        // Recency boost
        total_score *= self.calculate_recency_multiplier(file);
        
        total_score
    }
    
    fn get_searchable_text(&self, file: &IndexedFile) -> String {
        let mut text = Vec::new();
        
        // Add filename words
        text.extend(self.extract_words(&file.display_name));
        
        // Add directory names
        for component in file.path.components() {
            if let Some(name) = component.as_os_str().to_str() {
                text.extend(self.extract_words(name));
            }
        }
        
        // Add file type
        text.push(format!("{:?}", file.file_type).to_lowercase());
        
        text.join(" ")
    }
    
    fn extract_words(&self, text: &str) -> Vec<String> {
        let mut words = Vec::new();
        
        // Split on common separators
        let separators = regex::Regex::new(r"[_\-\.\s/\\]+").unwrap();
        words.extend(separators.split(text).map(|s| s.to_lowercase()));
        
        // Split camelCase
        let camel_re = regex::Regex::new(r"([a-z])([A-Z])").unwrap();
        let camel_split = camel_re.replace_all(text, "$1 $2");
        words.extend(camel_split.split_whitespace().map(|s| s.to_lowercase()));
        
        // Filter meaningful words
        words.into_iter()
            .filter(|w| w.len() > 1 && !w.chars().all(|c| c.is_numeric()))
            .collect()
    }
    
    fn score_text_match(&self, text: &str, query: &str, query_words: &[&str]) -> f64 {
        let mut score = 0.0;
        
        // Exact match
        if text == query {
            score += self.exact_weight;
        }
        
        // Prefix match
        if text.starts_with(query) {
            score += self.prefix_weight;
        }
        
        // Substring match
        if text.contains(query) {
            score += self.substring_weight;
        }
        
        // Word-by-word matching
        for word in query_words {
            if text.contains(word) {
                score += self.substring_weight * 0.8;
            }
            
            // Fuzzy word matching
            score += self.score_fuzzy_word_match(text, word);
        }
        
        score
    }
    
    fn score_fuzzy_word_match(&self, text: &str, word: &str) -> f64 {
        let mut best_score = 0.0;
        
        // Split text into words and check each
        for text_word in text.split_whitespace() {
            let mut word_score = 0.0;
            
            // Levenshtein distance
            let distance = self.levenshtein_distance(word, text_word);
            let max_len = word.len().max(text_word.len());
            if max_len > 0 {
                let similarity = 1.0 - (distance as f64 / max_len as f64);
                if similarity > 0.7 { // Only consider good matches
                    word_score += self.levenshtein_weight * similarity;
                }
            }
            
            // Soundex matching for phonetic similarity
            if self.soundex_match(word, text_word) {
                word_score += self.soundex_weight;
            }
            
            best_score = best_score.max(word_score);
        }
        
        best_score
    }
    
    fn score_file_type_relevance(&self, file: &IndexedFile, query_words: &[&str]) -> f64 {
        let file_type_keywords = match file.file_type {
            FileType::Code => vec!["code", "source", "script", "program"],
            FileType::Document => vec!["doc", "document", "text", "paper"],
            FileType::Image => vec!["image", "picture", "photo", "graphic"],
            FileType::Audio => vec!["audio", "sound", "music", "song"],
            FileType::Video => vec!["video", "movie", "clip", "film"],
            FileType::Archive => vec!["archive", "zip", "compressed"],
            FileType::Config => vec!["config", "configuration", "settings"],
            FileType::Data => vec!["data", "database", "csv", "excel"],
            FileType::Log => vec!["log", "logs", "debug", "error"],
            FileType::Markdown => vec!["markdown", "readme", "documentation"],
            _ => vec![],
        };
        
        let mut relevance_score = 0.0;
        for keyword in file_type_keywords {
            for query_word in query_words {
                if keyword.contains(query_word) || query_word.contains(keyword) {
                    relevance_score += 2.0;
                }
            }
        }
        
        relevance_score
    }
    
    fn calculate_recency_multiplier(&self, file: &IndexedFile) -> f64 {
        let now = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap_or_default()
            .as_secs();
        
        let age_days = (now - file.modified) / (24 * 3600);
        
        match age_days {
            0..=1 => 1.5,      // Last day: 50% boost
            2..=7 => 1.3,      // Last week: 30% boost
            8..=30 => 1.1,     // Last month: 10% boost
            31..=90 => 1.0,    // Last 3 months: no change
            _ => 0.9,          // Older: 10% penalty
        }
    }
    
    fn levenshtein_distance(&self, s1: &str, s2: &str) -> usize {
        let len1 = s1.chars().count();
        let len2 = s2.chars().count();
        
        if len1 == 0 { return len2; }
        if len2 == 0 { return len1; }
        
        let mut matrix = vec![vec![0; len2 + 1]; len1 + 1];
        
        // Initialize first row and column
        for i in 0..=len1 {
            matrix[i][0] = i;
        }
        for j in 0..=len2 {
            matrix[0][j] = j;
        }
        
        let s1_chars: Vec<char> = s1.chars().collect();
        let s2_chars: Vec<char> = s2.chars().collect();
        
        for i in 1..=len1 {
            for j in 1..=len2 {
                let cost = if s1_chars[i-1] == s2_chars[j-1] { 0 } else { 1 };
                matrix[i][j] = std::cmp::min(
                    std::cmp::min(
                        matrix[i-1][j] + 1,     // deletion
                        matrix[i][j-1] + 1      // insertion
                    ),
                    matrix[i-1][j-1] + cost     // substitution
                );
            }
        }
        
        matrix[len1][len2]
    }
    
    fn soundex_match(&self, word1: &str, word2: &str) -> bool {
        if word1.len() < 3 || word2.len() < 3 {
            return false;
        }
        
        self.soundex(word1) == self.soundex(word2)
    }
    
    fn soundex(&self, word: &str) -> String {
        if word.is_empty() {
            return "0000".to_string();
        }
        
        let word = word.to_uppercase();
        let chars: Vec<char> = word.chars().collect();
        let mut result = String::new();
        
        // First character is always kept
        result.push(chars[0]);
        
        let mut prev_code = self.soundex_code(chars[0]);
        
        for &ch in chars.iter().skip(1) {
            let code = self.soundex_code(ch);
            if code != '0' && code != prev_code {
                result.push(code);
                if result.len() == 4 {
                    break;
                }
            }
            prev_code = code;
        }
        
        // Pad with zeros
        while result.len() < 4 {
            result.push('0');
        }
        
        result
    }
    
    fn soundex_code(&self, ch: char) -> char {
        match ch {
            'B' | 'F' | 'P' | 'V' => '1',
            'C' | 'G' | 'J' | 'K' | 'Q' | 'S' | 'X' | 'Z' => '2',
            'D' | 'T' => '3',
            'L' => '4',
            'M' | 'N' => '5',
            'R' => '6',
            _ => '0',
        }
    }
}

// Enhanced engine integration for local filesystem search
// src/enhanced_engine.rs

use crate::engine::ResonantEngine;
use crate::filesystem_indexer::{FilesystemIndexer, IndexedFile};
use crate::crawler::CrawledDocument;
use std::collections::HashMap;
use std::path::PathBuf;

impl ResonantEngine {
    /// Add a filesystem document to the quantum index
    pub fn add_filesystem_document(&mut self, file: &IndexedFile) {
        let mut content = file.display_name.clone();
        
        // Add path components as searchable content
        for component in file.path.components() {
            if let Some(name) = component.as_os_str().to_str() {
                content.push_str(" ");
                content.push_str(name);
            }
        }
        
        // Add file type information
        content.push_str(&format!(" {:?}", file.file_type));
        
        // Create a CrawledDocument-like structure for compatibility
        let doc = CrawledDocument {
            url: file.path.to_string_lossy().to_string(),
            title: file.display_name.clone(),
            text: content,
        };
        
        self.add_crawled_document(doc);
    }
    
    /// Bulk add filesystem documents with progress reporting
    pub fn add_filesystem_documents(&mut self, files: impl Iterator<Item = &IndexedFile>, progress_callback: Option<impl Fn(usize)>) {
        let mut count = 0;
        
        for file in files {
            self.add_filesystem_document(file);
            count += 1;
            
            if let Some(ref callback) = progress_callback {
                if count % 100 == 0 {
                    callback(count);
                }
            }
        }
        
        if let Some(ref callback) = progress_callback {
            callback(count);
        }
    }
    
    /// Search with filesystem-specific optimizations
    pub fn search_filesystem(&mut self, query: &str, file_type_filter: Option<&str>, max_age_days: Option<u64>) -> Vec<crate::engine::SearchResult> {
        let mut results = self.search(query, 50); // Get more results to filter
        
        // Apply filesystem-specific filters
        if let Some(file_type) = file_type_filter {
            results.retain(|result| {
                result.path.to_lowercase().contains(&file_type.to_lowercase()) ||
                result.title.to_lowercase().contains(&file_type.to_lowercase())
            });
        }
        
        if let Some(max_age) = max_age_days {
            let cutoff = std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap_or_default()
                .as_secs() - (max_age * 24 * 3600);
            
            results.retain(|result| {
                // Try to get file modification time
                if let Ok(metadata) = std::fs::metadata(&result.path) {
                    if let Ok(modified) = metadata.modified() {
                        if let Ok(duration) = modified.duration_since(std::time::UNIX_EPOCH) {
                            return duration.as_secs() > cutoff;
                        }
                    }
                }
                true // Keep if we can't determine age
            });
        }
        
        results.truncate(10); // Return top 10 after filtering
        results
    }
}

// Main CLI enhancements for better UX
// src/cli_enhancements.rs

use std::io::{self, Write};
use termion::{color, style};

pub struct CLIFormatter;

impl CLIFormatter {
    pub fn print_header(title: &str) {
        println!("{}{}{}", 
                 color::Fg(color::Cyan), 
                 title, 
                 color::Fg(color::Reset));
        println!("{}{}{}", 
                 color::Fg(color::Blue), 
                 "─".repeat(title.len()), 
                 color::Fg(color::Reset));
    }
    
    pub fn print_success(message: &str) {
        println!("{}✓{} {}", 
                 color::Fg(color::Green), 
                 color::Fg(color::Reset), 
                 message);
    }
    
    pub fn print_warning(message: &str) {
        println!("{}⚠{} {}", 
                 color::Fg(color::Yellow), 
                 color::Fg(color::Reset), 
                 message);
    }
    
    pub fn print_error(message: &str) {
        eprintln!("{}✗{} {}", 
                  color::Fg(color::Red), 
                  color::Fg(color::Reset), 
                  message);
    }
    
    pub fn print_info(message: &str) {
        println!("{}ℹ{} {}", 
                 color::Fg(color::Blue), 
                 color::Fg(color::Reset), 
                 message);
    }
    
    pub fn print_search_result(index: usize, title: &str, path: &str, score: f64, snippet: &str) {
        println!("{}[{}]{} {}{}{}", 
                 color::Fg(color::Yellow),
                 index,
                 color::Fg(color::Reset),
                 color::Fg(color::White),
                 title,
                 color::Fg(color::Reset));
        
        println!("    {}📂{} {}", 
                 color::Fg(color::Blue),
                 color::Fg(color::Reset),
                 Self::truncate_path(path, 70));
        
        println!("    {}⚛️{} Score: {:.3}", 
                 color::Fg(color::Magenta),
                 color::Fg(color::Reset),
                 score);
        
        println!("    {}📝{} {}", 
                 color::Fg(color::Green),
                 color::Fg(color::Reset),
                 Self::truncate_text(snippet, 100));
        
        println!();
    }
    
    pub fn print_progress_bar(current: usize, total: usize, label: &str) {
        let percentage = if total > 0 { (current * 100) / total } else { 0 };
        let bar_width = 40;
        let filled = (percentage * bar_width) / 100;
        
        print!("\r{} [", label);
        for i in 0..bar_width {
            if i < filled {
                print!("█");
            } else {
                print!("░");
            }
        }
        print!("] {}% ({}/{})", percentage, current, total);
        io::stdout().flush().unwrap();
        
        if current >= total {
            println!(); // New line when complete
        }
    }
    
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
}

// Performance monitoring and statistics
// src/performance_monitor.rs

use std::time::{Instant, Duration};
use std::collections::VecDeque;

pub struct PerformanceMonitor {
    search_times: VecDeque<Duration>,
    index_times: VecDeque<Duration>,
    max_samples: usize,
}

impl PerformanceMonitor {
    pub fn new() -> Self {
        Self {
            search_times: VecDeque::new(),
            index_times: VecDeque::new(),
            max_samples: 100,
        }
    }
    
    pub fn record_search_time(&mut self, duration: Duration) {
        self.search_times.push_back(duration);
        if self.search_times.len() > self.max_samples {
            self.search_times.pop_front();
        }
    }
    
    pub fn record_index_time(&mut self, duration: Duration) {
        self.index_times.push_back(duration);
        if self.index_times.len() > self.max_samples {
            self.index_times.pop_front();
        }
    }
    
    pub fn get_average_search_time(&self) -> Duration {
        if self.search_times.is_empty() {
            return Duration::from_millis(0);
        }
        
        let total: Duration = self.search_times.iter().sum();
        total / self.search_times.len() as u32
    }
    
    pub fn get_average_index_time(&self) -> Duration {
        if self.index_times.is_empty() {
            return Duration::from_millis(0);
        }
        
        let total: Duration = self.index_times.iter().sum();
        total / self.index_times.len() as u32
    }
    
    pub fn print_statistics(&self) {
        println!("\n📊 Performance Statistics:");
        println!("   Average search time: {:?}", self.get_average_search_time());
        println!("   Average index time: {:?}", self.get_average_index_time());
        println!("   Search samples: {}", self.search_times.len());
        println!("   Index samples: {}", self.index_times.len());
        
        if !self.search_times.is_empty() {
            let fastest = self.search_times.iter().min().unwrap();
            let slowest = self.search_times.iter().max().unwrap();
            println!("   Fastest search: {:?}", fastest);
            println!("   Slowest search: {:?}", slowest);
        }
    }
}

// Advanced query processing for natural language queries
// src/query_processor.rs

use regex::Regex;
use std::collections::HashSet;

pub struct QueryProcessor {
    stop_words: HashSet<String>,
    file_type_keywords: HashMap<String, Vec<String>>,
}

impl QueryProcessor {
    pub fn new() -> Self {
        let stop_words = [
            "the", "a", "an", "and", "or", "but", "in", "on", "at", "to", "for", "of", "with", "by",
            "from", "up", "about", "into", "through", "during", "before", "after", "above", "below",
            "between", "among", "within", "without", "under", "over", "inside", "outside", "beside",
            "near", "far", "around", "across", "behind", "beyond", "beneath", "below", "above"
        ].iter().map(|&s| s.to_string()).collect();
        
        let mut file_type_keywords = HashMap::new();
        file_type_keywords.insert("code".to_string(), vec!["rust".to_string(), "python".to_string(), "javascript".to_string(), "cpp".to_string()]);
        file_type_keywords.insert("document".to_string(), vec!["pdf".to_string(), "word".to_string(), "text".to_string()]);
        file_type_keywords.insert("image".to_string(), vec!["jpg".to_string(), "png".to_string(), "gif".to_string()]);
        
        Self {
            stop_words,
            file_type_keywords,
        }
    }
    
    pub fn process_query(&self, query: &str) -> ProcessedQuery {
        let cleaned = self.clean_query(query);
        let tokens = self.tokenize(&cleaned);
        let filtered = self.remove_stop_words(tokens);
        let (keywords, file_type_hints, time_hints) = self.extract_hints(filtered);
        
        ProcessedQuery {
            original: query.to_string(),
            keywords,
            file_type_hints,
            time_hints,
        }
    }
    
    fn clean_query(&self, query: &str) -> String {
        // Remove special characters but keep meaningful ones
        let re = Regex::new(r"[^\w\s\-_\.]").unwrap();
        re.replace_all(query, " ").to_string()
    }
    
    fn tokenize(&self, query: &str) -> Vec<String> {
        query.split_whitespace()
            .map(|s| s.to_lowercase())
            .filter(|s| !s.is_empty())
            .collect()
    }
    
    fn remove_stop_words(&self, tokens: Vec<String>) -> Vec<String> {
        tokens.into_iter()
            .filter(|token| !self.stop_words.contains(token))
            .collect()
    }
    
    fn extract_hints(&self, tokens: Vec<String>) -> (Vec<String>, Vec<String>, Vec<String>) {
        let mut keywords = Vec::new();
        let mut file_type_hints = Vec::new();
        let mut time_hints = Vec::new();
        
        for token in tokens {
            // Check for file type hints
            if let Some(types) = self.file_type_keywords.get(&token) {
                file_type_hints.extend(types.clone());
                continue;
            }
            
            // Check for time hints
            if self.is_time_hint(&token) {
                time_hints.push(token.clone());
                continue;
            }
            
            // Regular keyword
            keywords.push(token);
        }
        
        (keywords, file_type_hints, time_hints)
    }
    
    fn is_time_hint(&self, token: &str) -> bool {
        matches!(token, 
            "today" | "yesterday" | "recent" | "new" | "old" | "latest" | 
            "last" | "week" | "month" | "year" | "daily" | "weekly" | "monthly"
        )
    }
}

#[derive(Debug)]
pub struct ProcessedQuery {
    pub original: String,
    pub keywords: Vec<String>,
    pub file_type_hints: Vec<String>,
    pub time_hints: Vec<String>,
}

impl ProcessedQuery {
    pub fn to_search_string(&self) -> String {
        self.keywords.join(" ")
    }
    
    pub fn has_file_type_filter(&self) -> bool {
        !self.file_type_hints.is_empty()
    }
    
    pub fn has_time_filter(&self) -> bool {
        !self.time_hints.is_empty()
    }
    
    pub fn get_age_filter_days(&self) -> Option<u64> {
        for hint in &self.time_hints {
            match hint.as_str() {
                "today" => return Some(1),
                "yesterday" => return Some(2),
                "recent" | "new" => return Some(7),
                "week" => return Some(7),
                "month" => return Some(30),
                _ => continue,
            }
        }
        None
    }
}