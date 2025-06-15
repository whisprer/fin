## 📚 Client Libraries

### JavaScript/TypeScript

```javascript
class QuantumSearchClient {
  constructor(baseUrl = 'http://localhost:8080/api') {
    this.baseUrl = baseUrl;
  }

  async search(query, options = {}) {
    const params = new URLSearchParams({
      q: query,
      quantum: options.quantum ?? true,
      persistence: options.persistence ?? true,
      max_results: options.maxResults ?? 10,
      ...options
    });

    const response = await fetch(`${this.baseUrl}/search?${params}`);
    return await response.json();
  }

  async getHealth() {
    const response = await fetch(`${this.baseUrl}/health`);
    return await response.json();
  }

  async addDocument(doc) {
    const response = await fetch(`${this.baseUrl}/index/document`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(doc)
    });
    return await response.json();
  }

  // WebSocket connection for real-time updates
  connectWebSocket(callbacks = {}) {
    const ws = new WebSocket(`ws://${this.baseUrl.replace('http://', '').replace('/api', '')}/ws`);
    
    ws.onmessage = (event) => {
      const message = JSON.parse(event.data);
      const callback = callbacks[message.type];
      if (callback) callback(message.data);
    };

    return ws;
  }
}

// Usage example
const client = new QuantumSearchClient();

// Basic search
const results = await client.search('quantum mechanics', {
  maxResults: 5,
  fileType: 'code'
});

// Real-time updates
const ws = client.connectWebSocket({
  index_progress: (data) => console.log(`Progress: ${data.progress_percent}%`),
  search_result: (data) => console.log('New result:', data.result),
  file_event: (data) => console.log('File updated:', data.path)
});
```

### Python

```python
import requests
import websocket
import json
from typing import Dict, List, Optional

class QuantumSearchClient:
    def __init__(self, base_url: str = "http://localhost:8080/api"):
        self.base_url = base_url
        self.session = requests.Session()

    def search(self, query: str, **options) -> Dict:
        """Perform a quantum resonant search."""
        params = {
            'q': query,
            'quantum': options.get('quantum', True),
            'persistence': options.get('persistence', True),
            'max_results': options.get('max_results', 10),
            **{k: v for k, v in options.items() 
               if k not in ['quantum', 'persistence', 'max_results']}
        }
        
        response = self.session.get(f"{self.base_url}/search", params=params)
        response.raise_for_status()
        return response.json()

    def get_health(self) -> Dict:
        """Get system health status."""
        response = self.session.get(f"{self.base_url}/health")
        response.raise_for_status()
        return response.json()

    def get_stats(self) -> Dict:
        """Get detailed system statistics."""
        response = self.session.get(f"{self.base_url}/stats")
        response.raise_for_status()
        return response.json()

    def add_document(self, title: str, content: str, url: str = None, **metadata) -> Dict:
        """Add a document to the index."""
        doc_data = {
            'title': title,
            'content': content,
            'url': url or f"doc_{hash(content)}",
            'metadata': metadata
        }
        
        response = self.session.post(
            f"{self.base_url}/index/document",
            json=doc_data
        )
        response.raise_for_status()
        return response.json()

    def add_documents_bulk(self, documents: List[Dict]) -> Dict:
        """Add multiple documents to the index."""
        response = self.session.post(
            f"{self.base_url}/index/bulk",
            json={'documents': documents}
        )
        response.raise_for_status()
        return response.json()

    def start_crawl(self, seed_urls: List[str], **options) -> Dict:
        """Start a web crawling session."""
        crawl_config = {
            'seed_urls': seed_urls,
            'max_pages': options.get('max_pages', 1000),
            'max_depth': options.get('max_depth', 3),
            'workers': options.get('workers', 10),
            **options
        }
        
        response = self.session.post(
            f"{self.base_url}/crawler/start",
            json=crawl_config
        )
        response.raise_for_status()
        return response.json()

    def get_crawl_progress(self, crawl_id: str) -> Dict:
        """Check crawling progress."""
        response = self.session.get(f"{self.base_url}/crawler/{crawl_id}/progress")
        response.raise_for_status()
        return response.json()

# Usage example
client = QuantumSearchClient()

# Search with quantum scoring
results = client.search(
    "entropy collapse resonance",
    quantum=True,
    persistence=True,
    max_results=10,
    file_type="text"
)

for result in results['results']:
    print(f"📄 {result['title']} (Score: {result['scores']['combined']:.3f})")
    print(f"    {result['snippet'][:100]}...")
    print()

# Add documents
response = client.add_document(
    title="Quantum Theory Basics",
    content="Quantum mechanics describes the behavior of matter and energy...",
    url="https://example.com/quantum-basics",
    author="Jane Physicist",
    tags=["quantum", "physics", "theory"]
)

# Start a crawl
crawl = client.start_crawl([
    "https://docs.rust-lang.org",
    "https://quantum-journal.org"
], max_pages=5000, max_depth=3)

print(f"Started crawl {crawl['crawl_id']}")
```

### Rust

```rust
use reqwest::{Client, Response};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use tokio_tungstenite::{connect_async, WebSocketStream};

#[derive(Debug, Serialize)]
pub struct SearchOptions {
    pub quantum: Option<bool>,
    pub persistence: Option<bool>,
    pub max_results: Option<usize>,
    pub file_type: Option<String>,
    pub max_age_days: Option<u32>,
}

#[derive(Debug, Deserialize)]
pub struct SearchResult {
    pub title: String,
    pub path: String,
    pub url: Option<String>,
    pub scores: ScoreBreakdown,
    pub snippet: String,
    pub metadata: HashMap<String, serde_json::Value>,
}

#[derive(Debug, Deserialize)]
pub struct ScoreBreakdown {
    pub resonance: f64,
    pub quantum: f64,
    pub persistence: f64,
    pub combined: f64,
}

#[derive(Debug, Deserialize)]
pub struct SearchResponse {
    pub query: String,
    pub total_results: usize,
    pub returned_results: usize,
    pub search_time_ms: u64,
    pub results: Vec<SearchResult>,
}

pub struct QuantumSearchClient {
    client: Client,
    base_url: String,
}

impl QuantumSearchClient {
    pub fn new(base_url: Option<String>) -> Self {
        Self {
            client: Client::new(),
            base_url: base_url.unwrap_or_else(|| "http://localhost:8080/api".to_string()),
        }
    }

    pub async fn search(&self, query: &str, options: SearchOptions) -> Result<SearchResponse, reqwest::Error> {
        let mut params = vec![("q", query.to_string())];
        
        if let Some(quantum) = options.quantum {
            params.push(("quantum", quantum.to_string()));
        }
        if let Some(persistence) = options.persistence {
            params.push(("persistence", persistence.to_string()));
        }
        if let Some(max_results) = options.max_results {
            params.push(("max_results", max_results.to_string()));
        }
        if let Some(file_type) = options.file_type {
            params.push(("file_type", file_type));
        }

        let response = self.client
            .get(&format!("{}/search", self.base_url))
            .query(&params)
            .send()
            .await?;

        response.json().await
    }

    pub async fn get_health(&self) -> Result<serde_json::Value, reqwest::Error> {
        let response = self.client
            .get(&format!("{}/health", self.base_url))
            .send()
            .await?;

        response.json().await
    }

    pub async fn add_document(&self, doc: &DocumentInput) -> Result<serde_json::Value, reqwest::Error> {
        let response = self.client
            .post(&format!("{}/index/document", self.base_url))
            .json(doc)
            .send()
            .await?;

        response.json().await
    }
}

#[derive(Debug, Serialize)]
pub struct DocumentInput {
    pub title: String,
    pub content: String,
    pub url: Option<String>,
    pub metadata: Option<HashMap<String, serde_json::Value>>,
}

// Usage example
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let client = QuantumSearchClient::new(None);

    // Perform search
    let results = client.search("quantum mechanics", SearchOptions {
        quantum: Some(true),
        persistence: Some(true),
        max_results: Some(5),
        file_type: None,
        max_age_days: None,
    }).await?;

    println!("Found {} results in {}ms", 
             results.total_results, 
             results.search_time_ms);

    for result in results.results {
        println!("📄 {} (Score: {:.3})", 
                 result.title, 
                 result.scores.combined);
        println!("   {}", result.snippet);
    }

    // Add a document
    let doc = DocumentInput {
        title: "Rust Async Programming".to_string(),
        content: "Async programming in Rust using tokio...".to_string(),
        url: Some("https://example.com/rust-async".to_string()),
        metadata: None,
    };

    let response = client.add_document(&doc).await?;
    println!("Document added: {:?}", response);

    Ok(())
}
```

---

## 🔌 Webhook Support

### Register Webhook

Register a webhook URL to receive notifications about index updates and search events.

#### `POST /api/webhooks`

```json
{
  "url": "https://your-app.com/webhook",
  "events": ["document_indexed", "search_performed", "crawl_completed"],
  "secret": "your_webhook_secret"
}
```

#### Response

```json
{
  "webhook_id": "webhook_123",
  "status": "registered",
  "events": ["document_indexed", "search_performed", "crawl_completed"]
}
```

### Webhook Events

#### Document Indexed

```json
{
  "event": "document_indexed",
  "timestamp": "2024-01-15T10:30:00Z",
  "data": {
    "document_id": "doc_456",
    "title": "New Document",
    "url": "https://example.com/new-doc",
    "scores": {
      "entropy": 4.231,
      "reversibility": 0.823
    }
  }
}
```

#### Search Performed

```json
{
  "event": "search_performed", 
  "timestamp": "2024-01-15T10:31:00Z",
  "data": {
    "query": "quantum mechanics",
    "results_count": 42,
    "search_time_ms": 23,
    "user_ip": "192.168.1.100"
  }
}
```

#### Crawl Completed

```json
{
  "event": "crawl_completed",
  "timestamp": "2024-01-15T11:00:00Z", 
  "data": {
    "crawl_id": "crawl_789",
    "pages_crawled": 5000,
    "pages_indexed": 4847,
    "duration_seconds": 1800,
    "errors": 23
  }
}
```

---

## 🚀 Rate Limiting

The API implements rate limiting to ensure fair usage and system stability.

### Default Limits

| Endpoint Category | Requests per Minute | Burst Limit |
|-------------------|-------------------|-------------|
| Search | 60 | 120 |
| Index Management | 30 | 60 |
| Statistics | 20 | 40 |
| Configuration | 10 | 20 |

### Rate Limit Headers

```http
X-RateLimit-Limit: 60
X-RateLimit-Remaining: 42
X-RateLimit-Reset: 1640995200
X-RateLimit-Retry-After: 30
```

### Rate Limit Exceeded

```json
{
  "error": "rate_limit_exceeded",
  "message": "Too many requests. Try again in 30 seconds.",
  "code": 429,
  "retry_after": 30
}
```

---

## 🔍 Query Language

The search API supports an advanced query language for complex searches.

### Basic Syntax

```bash
# Simple terms
quantum mechanics

# Phrase search  
"quantum entanglement"

# Boolean operators
quantum AND mechanics
quantum OR physics
quantum NOT classical

# Wildcards
quant*    # Matches quantum, quantity, etc.
?uantum   # Matches quantum

# Field-specific search
title:quantum
content:mechanics
type:code
author:"Einstein"

# Range queries
modified:[2024-01-01 TO 2024-12-31]
size:[1KB TO 1MB]
score:[0.8 TO 1.0]

# Proximity search
"quantum mechanics"~10  # Within 10 words of each other

# Boost terms
quantum^2 mechanics     # Give "quantum" 2x weight
```

### Advanced Examples

```bash
# Find code files about async programming
type:code AND (async OR tokio OR "async/await")

# Recent documents about machine learning
modified:[2024-01-01 TO NOW] AND ("machine learning" OR "neural network")

# High-relevance quantum physics papers
(quantum OR physics) AND score:[0.8 TO 1.0] AND type:document

# Large files with specific content
size:[1MB TO 10MB] AND (database OR "data structure")

# Complex boolean query
(rust AND (async OR tokio)) OR (python AND asyncio) NOT deprecated
```

---

## 🔧 Error Handling

### Error Response Format

All API errors return a consistent JSON structure:

```json
{
  "error": "error_code",
  "message": "Human-readable error description",
  "code": 400,
  "details": {
    "field": "Additional context",
    "suggestion": "Try rephrasing your query"
  },
  "request_id": "req_123456789"
}
```

### Common Error Codes

| Code | Error Type | Description |
|------|------------|-------------|
| 400 | `invalid_request` | Malformed request syntax |
| 401 | `unauthorized` | Authentication required |
| 403 | `forbidden` | Insufficient permissions |
| 404 | `not_found` | Resource doesn't exist |
| 422 | `invalid_parameter` | Parameter validation failed |
| 429 | `rate_limit_exceeded` | Too many requests |
| 500 | `internal_error` | Server-side error |
| 503 | `service_unavailable` | System temporarily unavailable |

### Error Recovery

```javascript
// JavaScript error handling example
async function searchWithRetry(query, maxRetries = 3) {
  for (let attempt = 1; attempt <= maxRetries; attempt++) {
    try {
      return await client.search(query);
    } catch (error) {
      if (error.code === 429) {
        // Rate limited - wait and retry
        const retryAfter = error.retry_after || (attempt * 1000);
        await new Promise(resolve => setTimeout(resolve, retryAfter));
        continue;
      } else if (error.code >= 500 && attempt < maxRetries) {
        // Server error - exponential backoff
        await new Promise(resolve => setTimeout(resolve, Math.pow(2, attempt) * 1000));
        continue;
      } else {
        // Non-recoverable error
        throw error;
      }
    }
  }
  throw new Error(`Failed after ${maxRetries} attempts`);
}
```

---

## 📊 Monitoring & Analytics

### Custom Headers

Include these headers in requests for better monitoring:

```http
X-Client-Name: MyApp
X-Client-Version: 1.2.3
X-Request-ID: req_abc123
X-User-ID: user_456
```

### Request Tracing

Enable detailed request tracing:

```bash
curl -H "X-Trace: true" "http://localhost:8080/api/search?q=quantum"
```

Response includes timing breakdown:

```json
{
  "results": [...],
  "trace": {
    "total_time_ms": 23,
    "tokenization_ms": 2,
    "vector_building_ms": 3,
    "search_ms": 15,
    "ranking_ms": 2,
    "serialization_ms": 1
  }
}
```

---

## 🧪 Development & Testing

### Mock Server

For testing, you can run a mock API server:

```bash
# Start mock server with sample data
cargo run --features mock-server -- mock --port 8081

# Returns pre-defined responses for testing
curl "http://localhost:8081/api/search?q=test"
```

### API Validation

Validate your API requests:

```bash
# Check if request is valid before sending
curl -X POST "http://localhost:8080/api/validate" \
  -H "Content-Type: application/json" \
  -d '{"endpoint": "/search", "params": {"q": "test"}}'
```

### Load Testing

Example load test using Apache Bench:

```bash
# Test search endpoint
ab -n 1000 -c 10 "http://localhost:8080/api/search?q=quantum"

# Test with POST requests
ab -n 100 -c 5 -T "application/json" -p search_payload.json \
   "http://localhost:8080/api/search"
```

---

## 📋 API Changelog

### v0.2.0 (Current)
- ✅ Added quantum scoring configuration
- ✅ Added persistence theory scoring  
- ✅ Added WebSocket support for real-time updates
- ✅ Added bulk document indexing
- ✅ Added webhook notifications
- ✅ Added advanced query language
- ✅ Improved error handling and rate limiting

### v0.1.0
- ✅ Basic search functionality
- ✅ Document indexing
- ✅ Health checks and statistics
- ✅ RESTful API design

### Coming in v0.3.0
- 🔄 GraphQL API support
- 🔄 Advanced analytics dashboard
- 🔄 Machine learning relevance tuning
- 🔄 Multi-language support
- 🔄 Federated search across instances

---

**Need help with the API?** 
- 📖 Check the [Architecture Guide](ARCHITECTURE.md) for system details
- 🚀 See [Quick Start](QUICKSTART.md) for setup instructions  
- 💬 Join our Discord community for real-time support
- 🐛 Report API bugs on GitHub Issues

*Built with ❤️ by the whispr.dev team*