# REST API Documentation 🔌

The Quantum Resonant Search Engine provides a comprehensive REST API for programmatic access to all search and indexing functionality.

## 🌐 Base URL

```
http://localhost:8080/api
```

## 🔐 Authentication

Currently, no authentication is required for API access. In production deployments, consider implementing:
- API key authentication
- Rate limiting per client
- IP whitelisting

## 📝 Content Types

All API endpoints accept and return JSON unless otherwise specified.

```http
Content-Type: application/json
Accept: application/json
```

---

## 🔍 Search Endpoints

### `GET /api/search`

Perform a quantum resonant search query.

#### Parameters

| Parameter | Type | Required | Default | Description |
|-----------|------|----------|---------|-------------|
| `q` | string | ✅ | - | Search query string |
| `quantum` | boolean | ❌ | `true` | Enable quantum-inspired scoring |
| `persistence` | boolean | ❌ | `true` | Enable persistence theory scoring |
| `max_results` | integer | ❌ | `10` | Maximum number of results (1-100) |
| `file_type` | string | ❌ | - | Filter by file type (`code`, `text`, `config`, etc.) |
| `max_age_days` | integer | ❌ | - | Only include files modified within N days |
| `min_score` | float | ❌ | `0.0` | Minimum combined score threshold |

#### Example Request

```bash
curl "http://localhost:8080/api/search?q=quantum%20mechanics&quantum=true&max_results=5"
```

#### Example Response

```json
{
  "query": "quantum mechanics",
  "total_results": 42,
  "returned_results": 5,
  "search_time_ms": 23,
  "quantum_enabled": true,
  "persistence_enabled": true,
  "results": [
    {
      "title": "Introduction to Quantum Physics",
      "path": "/docs/quantum/intro.md",
      "url": "https://example.com/quantum-intro",
      "file_type": "markdown",
      "size_bytes": 15420,
      "modified_timestamp": 1703123456,
      "created_timestamp": 1702123456,
      "scores": {
        "resonance": 0.8432,
        "quantum": 0.7821,
        "persistence": 0.6543,
        "entropy_delta": 0.0234,
        "combined": 0.7456
      },
      "snippet": "Quantum mechanics is the fundamental theory in physics that describes the nature of energy and matter on the atomic and subatomic level...",
      "highlighted_terms": ["quantum", "mechanics"],
      "metadata": {
        "word_count": 3245,
        "entropy": 4.567,
        "reversibility": 0.823,
        "buffering_capacity": 0.654
      }
    }
  ],
  "facets": {
    "file_types": {
      "code": 15,
      "markdown": 12,
      "text": 8,
      "config": 7
    },
    "time_ranges": {
      "last_day": 3,
      "last_week": 12,
      "last_month": 27,
      "older": 15
    }
  },
  "suggestions": [
    "quantum entanglement",
    "quantum computing", 
    "quantum field theory"
  ]
}
```

#### Error Responses

```json
// 400 Bad Request
{
  "error": "invalid_query",
  "message": "Query parameter 'q' is required",
  "code": 400
}

// 422 Unprocessable Entity  
{
  "error": "invalid_parameter",
  "message": "max_results must be between 1 and 100",
  "code": 422
}

// 500 Internal Server Error
{
  "error": "search_failure", 
  "message": "Internal search engine error",
  "code": 500
}
```

---

## 📊 Statistics & Health

### `GET /api/health`

Get system health and status information.

#### Example Response

```json
{
  "status": "healthy",
  "uptime_seconds": 3661,
  "version": "0.2.0",
  "index": {
    "total_documents": 12486,
    "total_size_bytes": 1287356420,
    "last_updated": 1703123456,
    "index_health": "good"
  },
  "features": {
    "quantum_scoring": true,
    "persistence_scoring": true,
    "real_time_updates": true
  },
  "performance": {
    "avg_search_time_ms": 23.4,
    "memory_usage_mb": 1247.8,
    "cpu_usage_percent": 12.3
  }
}
```

### `GET /api/stats`

Get detailed search engine statistics.

#### Example Response

```json
{
  "index": {
    "total_documents": 12486,
    "total_size_bytes": 1287356420,
    "compressed_size_bytes": 312847103,
    "compression_ratio": 4.12,
    "file_types": {
      "code": 4247,
      "text": 3156, 
      "config": 1891,
      "markdown": 1204,
      "document": 856,
      "other": 1132
    },
    "size_distribution": {
      "small_0_1kb": 2341,
      "medium_1_10kb": 6784,
      "large_10_100kb": 2987,
      "huge_100kb_plus": 374
    },
    "age_distribution": {
      "last_day": 156,
      "last_week": 847,
      "last_month": 2341,
      "last_year": 6234,
      "older": 2908
    }
  },
  "search_performance": {
    "total_searches": 1247,
    "avg_search_time_ms": 23.4,
    "fastest_search_ms": 8,
    "slowest_search_ms": 156,
    "search_time_percentiles": {
      "p50": 19,
      "p90": 45, 
      "p95": 67,
      "p99": 134
    }
  },
  "quantum_metrics": {
    "avg_resonance_score": 0.423,
    "avg_quantum_score": 0.381,
    "avg_persistence_score": 0.567,
    "entropy_distribution": {
      "low_entropy_0_2": 1234,
      "medium_entropy_2_4": 7456,
      "high_entropy_4_plus": 3796
    }
  },
  "system": {
    "memory_usage_mb": 1247.8,
    "disk_usage_mb": 892.3,
    "cpu_usage_percent": 12.3,
    "uptime_seconds": 3661
  }
}
```

---

## 🗂️ Index Management

### `POST /api/index/document`

Add a single document to the index.

#### Request Body

```json
{
  "title": "My Document",
  "content": "This is the document content...",
  "url": "https://example.com/doc",
  "metadata": {
    "author": "Jane Doe",
    "tags": ["science", "physics"],
    "file_type": "text"
  }
}
```

#### Response

```json
{
  "document_id": "doc_123456",
  "status": "indexed",
  "scores": {
    "entropy": 4.231,
    "reversibility": 0.823,
    "buffering_capacity": 0.654
  },
  "vector_dimensions": 1247,
  "processing_time_ms": 45
}
```

### `POST /api/index/bulk`

Add multiple documents to the index.

#### Request Body

```json
{
  "documents": [
    {
      "title": "Document 1",
      "content": "Content of document 1...",
      "url": "https://example.com/doc1"
    },
    {
      "title": "Document 2", 
      "content": "Content of document 2...",
      "url": "https://example.com/doc2"
    }
  ]
}
```

#### Response

```json
{
  "processed": 2,
  "successful": 2,
  "failed": 0,
  "errors": [],
  "processing_time_ms": 234,
  "documents": [
    {
      "document_id": "doc_123456",
      "status": "indexed"
    },
    {
      "document_id": "doc_123457", 
      "status": "indexed"
    }
  ]
}
```

### `DELETE /api/index/document/{id}`

Remove a document from the index.

#### Response

```json
{
  "document_id": "doc_123456",
  "status": "deleted",
  "was_indexed": true
}
```

### `POST /api/index/rebuild`

Trigger a full index rebuild.

#### Request Body (Optional)

```json
{
  "paths": ["/home/user/documents", "/home/user/projects"],
  "file_types": ["txt", "md", "rs", "py"],
  "max_file_size_mb": 10,
  "exclude_patterns": [".git/", "node_modules/"]
}
```

#### Response

```json
{
  "rebuild_id": "rebuild_789012",
  "status": "started",
  "estimated_duration_seconds": 1800,
  "progress_url": "/api/index/rebuild/789012/progress"
}
```

### `GET /api/index/rebuild/{rebuild_id}/progress`

Check rebuild progress.

#### Response

```json
{
  "rebuild_id": "rebuild_789012",
  "status": "in_progress",
  "progress_percent": 67.3,
  "files_processed": 8429,
  "files_total": 12500,
  "current_file": "/home/user/projects/quantum-search/src/main.rs",
  "elapsed_seconds": 1205,
  "estimated_remaining_seconds": 591,
  "errors": [
    {
      "file": "/some/file.txt",
      "error": "Permission denied"
    }
  ]
}
```

---

## 🔧 Configuration

### `GET /api/config`

Get current search engine configuration.

#### Response

```json
{
  "quantum": {
    "enabled": true,
    "scoring_weight": 0.3,
    "complex_resonance": true,
    "biorthogonal_matching": true
  },
  "persistence": {
    "enabled": true,
    "scoring_weight": 0.3,
    "fragility": 0.2,
    "entropy_weight": 0.1,
    "trend_decay": 0.05
  },
  "indexing": {
    "max_file_size_mb": 10,
    "supported_file_types": ["txt", "md", "rs", "py", "js", "json"],
    "exclude_patterns": [".git/", "node_modules/", "target/"],
    "real_time_updates": true
  },
  "performance": {
    "max_results": 100,
    "search_timeout_ms": 5000,
    "index_cache_size_mb": 512
  }
}
```

### `PUT /api/config`

Update search engine configuration.

#### Request Body

```json
{
  "quantum": {
    "enabled": true,
    "scoring_weight": 0.4
  },
  "persistence": {
    "fragility": 0.3,
    "entropy_weight": 0.15
  }
}
```

#### Response

```json
{
  "status": "updated",
  "changes_applied": [
    "quantum.scoring_weight: 0.3 → 0.4",
    "persistence.fragility: 0.2 → 0.3",
    "persistence.entropy_weight: 0.1 → 0.15"
  ],
  "restart_required": false
}
```

---

## 🕸️ Crawler Management

### `POST /api/crawler/start`

Start a web crawling session.

#### Request Body

```json
{
  "seed_urls": [
    "https://docs.rust-lang.org",
    "https://quantum-journal.org"
  ],
  "max_pages": 5000,
  "max_depth": 3,
  "workers": 12,
  "stay_in_domain": false,
  "politeness_delay_ms": 100,
  "respect_robots_txt": true
}
```

#### Response

```json
{
  "crawl_id": "crawl_456789",
  "status": "started",
  "estimated_duration_seconds": 3600,
  "progress_url": "/api/crawler/crawl_456789/progress"
}
```

### `GET /api/crawler/{crawl_id}/progress`

Check crawling progress.

#### Response

```json
{
  "crawl_id": "crawl_456789",
  "status": "in_progress",
  "progress": {
    "pages_crawled": 1247,
    "pages_total": 5000,
    "pages_per_second": 2.3,
    "percent_complete": 24.9
  },
  "current_stats": {
    "active_workers": 12,
    "queue_size": 8934,
    "domains_discovered": 47,
    "errors": 23
  },
  "timing": {
    "elapsed_seconds": 542,
    "estimated_remaining_seconds": 1634
  }
}
```

### `POST /api/crawler/{crawl_id}/stop`

Stop a running crawl.

#### Response

```json
{
  "crawl_id": "crawl_456789",
  "status": "stopped",
  "final_stats": {
    "pages_crawled": 1247,
    "pages_indexed": 1183,
    "elapsed_seconds": 542,
    "errors": 23
  }
}
```

---

## 📡 WebSocket API

For real-time updates during indexing and crawling operations.

### Connection

```javascript
const ws = new WebSocket('ws://localhost:8080/ws');
```

### Message Types

#### Index Progress Updates

```json
{
  "type": "index_progress",
  "data": {
    "files_processed": 1247,
    "files_total": 5000,
    "current_file": "/path/to/file.txt",
    "progress_percent": 24.9
  }
}
```

#### Search Result Stream

```json
{
  "type": "search_result",
  "data": {
    "query": "quantum mechanics",
    "result": {
      "title": "Quantum Physics Intro",
      "score": 0.842,
      "snippet": "Quantum mechanics describes..."
    }
  }
}
```

#### Real-time File Updates

```json
{
  "type": "file_event",
  "data": {
    "event": "created",
    "path": "/home/user/new_file.txt",
    "file_type": "text",
    "auto_indexed": true
  }
}
```

---

## 📚 Client Libraries

### JavaScript/TypeScript

```javascript
class QuantumSearchClient {
  constructor(baseUrl = 'http://localhost:8080/api') {
    this.baseUrl = baseUrl;
  }

  async search(query, options


















