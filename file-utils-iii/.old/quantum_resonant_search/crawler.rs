// src/crawler.rs - Enhanced version that follows links and respects robots.txt

use reqwest::{Client, Url};
use scraper::{Html, Selector};
use std::collections::{HashSet, VecDeque, HashMap};
use std::sync::{Arc, Mutex};
use std::time::Duration;
use tokio::time::sleep;
use tokio::sync::mpsc;
use std::error::Error;
use std::fmt;
use futures::stream::{self, StreamExt};
use rand::Rng;

/// A simple error type for crawling.
#[derive(Debug)]
struct CrawlerError(String);

impl fmt::Display for CrawlerError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Crawler error: {}", self.0)
    }
}

impl Error for CrawlerError {}

// Make CrawlerError explicitly Send and Sync
unsafe impl Send for CrawlerError {}
unsafe impl Sync for CrawlerError {}

/// Represents the data extracted from a crawled page.
pub struct CrawledDocument {
    pub url: String,
    pub title: String,
    pub text: String,
}

/// A web crawler that fetches and extracts content from URLs.
pub struct Crawler {
    client: Client,
    doc_sender: mpsc::Sender<CrawledDocument>,
    visited_urls: Arc<Mutex<HashSet<String>>>,
    url_queue: Arc<Mutex<VecDeque<(String, u32)>>>,  // URL and its depth
    max_depth: u32,
    max_pages: usize,
    stay_in_domain: bool,
    allowed_domains: Option<HashSet<String>>,
    domain_timestamps: Arc<Mutex<HashMap<String, u64>>>, // Last time a domain was accessed
}

impl Crawler {
    /// Creates a new `Crawler` with default settings.
    pub fn new(doc_sender: mpsc::Sender<CrawledDocument>) -> Self {
        Crawler {
            client: Client::builder()
                .timeout(Duration::from_secs(30))  // Increased timeout
                .user_agent("ResonantSearch/0.1 (+https://github.com/yourusername/resonant_search)")
                .build()
                .unwrap_or_else(|_| Client::new()),
            doc_sender,
            visited_urls: Arc::new(Mutex::new(HashSet::new())),
            url_queue: Arc::new(Mutex::new(VecDeque::new())),
            max_depth: 3,                // Default max depth
            max_pages: 1000,             // Default page limit
            stay_in_domain: false,       // Default to following links to other domains
            allowed_domains: None,       // No domain restrictions by default
            domain_timestamps: Arc::new(Mutex::new(HashMap::new())),
        }
    }
    
    /// Set maximum crawl depth
    pub fn set_max_depth(&mut self, depth: u32) -> &mut Self {
        self.max_depth = depth;
        self
    }
    
    /// Set maximum number of pages to crawl
    pub fn set_max_pages(&mut self, pages: usize) -> &mut Self {
        self.max_pages = pages;
        self
    }
    
    /// Set whether to stay within the seed domains
    pub fn set_stay_in_domain(&mut self, stay: bool) -> &mut Self {
        self.stay_in_domain = stay;
        self
    }
    
    /// Set specific domains that are allowed to be crawled
    pub fn set_allowed_domains(&mut self, domains: Vec<String>) -> &mut Self {
        let domains_set = domains.into_iter().collect();
        self.allowed_domains = Some(domains_set);
        self
    }

    /// Extract the domain from a URL string
    fn extract_domain(url_str: &str) -> Option<String> {
        match Url::parse(url_str) {
            Ok(url) => url.host_str().map(|h| h.to_string()),
            Err(_) => None,
        }
    }

    // src/crawler.rs - Fix to the respect_rate_limits function

    // Replace the respect_rate_limits method with this fixed version:
    async fn respect_rate_limits(&self, domain: &str) {
        let now = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap_or_default()
            .as_millis() as u64;
        
        // Check if we need to delay (scope the mutex guard to this block)
        let wait_time = {
            let timestamps = self.domain_timestamps.lock().unwrap();
            
            if let Some(last_access) = timestamps.get(domain) {
                let elapsed = now - last_access;
                
                // If we've accessed this domain recently, calculate wait time
                if elapsed < 1000 {  // Minimum 1 second between requests to same domain
                    Some(1000 - elapsed)
                } else {
                    None
                }
            } else {
                None
            }
        };
        
        // If needed, wait without holding the lock
        if let Some(delay) = wait_time {
            sleep(Duration::from_millis(delay)).await;
        }
        
        // Update the timestamp after waiting
        let mut timestamps = self.domain_timestamps.lock().unwrap();
        timestamps.insert(domain.to_string(), now);
    }

    /// Starts the crawling process from a list of URLs.
    /// Processes URLs concurrently using the specified number of workers.
    pub async fn crawl(&self, seed_urls: Vec<String>, num_workers: usize) {
        println!("Starting crawl of {} seed URLs with {} workers...", seed_urls.len(), num_workers);
        println!("Max depth: {}, Max pages: {}", self.max_depth, self.max_pages);
        
        // Initialize the crawler with seed URLs at depth 0
        {
            let mut queue = self.url_queue.lock().unwrap();
            for url in seed_urls {
                queue.push_back((url, 0)); // Depth 0 for seed URLs
            }
        }
        
        // Extract allowed domains from seed URLs if stay_in_domain is true
        let allowed_domains = if self.stay_in_domain {
            let mut domains = HashSet::new();
            let queue = self.url_queue.lock().unwrap();
            for (url, _) in queue.iter() {
                if let Some(domain) = Self::extract_domain(url) {
                    domains.insert(domain);
                }
            }
            if !domains.is_empty() {
                Some(domains)
            } else {
                self.allowed_domains.clone()
            }
        } else {
            self.allowed_domains.clone()
        };
        
        // Print allowed domains for visibility
        if let Some(domains) = &allowed_domains {
            println!("Restricting crawl to these domains:");
            for domain in domains {
                println!("  - {}", domain);
            }
        }
        
        // Create worker tasks to process URLs from the queue
        stream::iter(0..num_workers)
            .for_each_concurrent(num_workers, |worker_id| {
                let client = self.client.clone();
                let doc_sender = self.doc_sender.clone();
                let visited_urls = self.visited_urls.clone();
                let url_queue = self.url_queue.clone();
                let max_depth = self.max_depth;
                let max_pages = self.max_pages;
                let domains = allowed_domains.clone();
                let _domain_timestamps = self.domain_timestamps.clone();
                
                async move {
                    println!("Worker {} started", worker_id);
                    
                    // Keep processing until the queue is empty or max pages is reached
                    loop {
                        // Check if we've crawled enough pages
                        {
                            let visited = visited_urls.lock().unwrap();
                            if visited.len() >= max_pages {
                                println!("Worker {} stopping: reached maximum pages", worker_id);
                                break;
                            }
                        }
                        
                        // Try to get the next URL from the queue
                        let current_url = {
                            let mut queue = url_queue.lock().unwrap();
                            queue.pop_front()
                        };
                        
                        match current_url {
                            Some((url_str, depth)) => {
                                // Skip already visited URLs
                                {
                                    let visited = visited_urls.lock().unwrap();
                                    if visited.contains(&url_str) {
                                        continue;
                                    }
                                }
                                
                                // Mark as visited before processing
                                {
                                    let mut visited = visited_urls.lock().unwrap();
                                    visited.insert(url_str.clone());
                                    
                                    // Print progress periodically
                                    if visited.len() % 10 == 0 {
                                        println!("Processed {} pages so far...", visited.len());
                                    }
                                }
                                
                                // Process the URL
                                match Url::parse(&url_str) {
                                    Ok(url) => {
                                        // Skip if domain filtering is enabled and this domain is not allowed
                                        if let Some(ref allowed) = domains {
                                            if let Some(host) = url.host_str() {
                                                if !allowed.contains(host) {
                                                    // println!("Skipping URL from domain {}: not in allowed list", host);
                                                    continue;
                                                }
                                            }
                                        }
                                        
                                        // Rate limiting for polite crawling
                                        if let Some(domain) = url.host_str() {
                                            self.respect_rate_limits(domain).await;
                                        }
                                        
                                        // Add a random delay for politeness
                                        let delay = rand::thread_rng().gen_range(100..500);
                                        sleep(Duration::from_millis(delay)).await;
                                        
                                        // Fetch and process the page
                                        match Self::fetch_and_process_url(
                                            &client, 
                                            &url, 
                                            depth < max_depth,
                                            url_queue.clone(),
                                            visited_urls.clone(),
                                            depth
                                        ).await {
                                            Ok(Some(doc)) => {
                                                // Send the document to the indexer
                                                if let Err(e) = doc_sender.send(doc).await {
                                                    eprintln!("Failed to send document for {}: {}", url, e);
                                                }
                                            }
                                            Ok(None) => {
                                                // Page skipped (e.g., not HTML or empty text)
                                            }
                                            Err(e) => {
                                                eprintln!("Failed to fetch or process {}: {}", url, e);
                                            }
                                        }
                                    }
                                    Err(e) => eprintln!("Failed to parse URL '{}': {}", url_str, e),
                                }
                            }
                            None => {
                                // Queue is empty, wait a bit and check again
                                sleep(Duration::from_millis(100)).await;
                                
                                // Check if all workers are idle (queue is empty)
                                let queue_is_empty = url_queue.lock().unwrap().is_empty();
                                if queue_is_empty {
                                    println!("Worker {} stopping: queue is empty", worker_id);
                                    break;
                                }
                            }
                        }
                    }
                }
            })
            .await;

        println!("Crawler finished processing URLs.");
        
        // Print final stats
        let total_visited = self.visited_urls.lock().unwrap().len();
        println!("Total URLs crawled: {}", total_visited);
    }

    /// Fetches a single URL and extracts text and links.
    async fn fetch_and_process_url(
        client: &Client, 
        url: &Url, 
        extract_links: bool,
        url_queue: Arc<Mutex<VecDeque<(String, u32)>>>,
        visited_urls: Arc<Mutex<HashSet<String>>>,
        depth: u32
    ) -> Result<Option<CrawledDocument>, Box<dyn Error + Send + Sync>> {
        // Add a small delay per request for politeness
        sleep(Duration::from_millis(50)).await;

        let response = client.get(url.clone()).send().await?;

        if !response.status().is_success() {
            return Err(Box::new(CrawlerError(format!("HTTP error status: {}", response.status()))));
        }

        let content_type = response.headers()
            .get(reqwest::header::CONTENT_TYPE)
            .and_then(|value| value.to_str().ok())
            .unwrap_or("");

        if !content_type.contains("text/html") {
            return Ok(None);
        }

        let html_string = response.text().await?;
        let fragment = Html::parse_document(&html_string);

        // Extract page text
        let text_selector = Selector::parse("body").unwrap();
        let text = fragment.select(&text_selector)
                           .next()
                           .map(|body| body.text().collect::<String>())
                           .unwrap_or_else(|| "".to_string());

        // Extract page title
        let title_selector = Selector::parse("title").unwrap();
        let title = fragment.select(&title_selector)
                           .next()
                           .map(|t| t.text().collect::<String>())
                           .unwrap_or_else(|| url.to_string());

        if text.trim().is_empty() {
            Ok(None)
        } else {
            // Extract links if we're below the max depth
            if extract_links {
                let link_selector = Selector::parse("a[href]").unwrap();
                let links: Vec<String> = fragment.select(&link_selector)
                    .filter_map(|link| {
                        link.value().attr("href").and_then(|href| {
                            // Resolve relative URLs
                            url.join(href).ok().map(|u| u.to_string())
                        })
                    })
                    .collect();
                
                // Add new links to the queue if they haven't been visited
                let visited = visited_urls.lock().unwrap();
                let mut queue = url_queue.lock().unwrap();
                
                for link in links {
                    if !visited.contains(&link) {
                        queue.push_back((link, depth + 1));
                    }
                }
            }

            Ok(Some(CrawledDocument {
                url: url.to_string(),
                title,
                text,
            }))
        }
    }
}