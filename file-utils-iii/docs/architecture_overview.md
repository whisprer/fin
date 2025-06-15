# Resonant Search Engine - Architecture Overview

This document provides a technical overview of the Resonant Search Engine architecture, explaining how the quantum-inspired search algorithms are integrated with web functionality.

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        Web Interface                        │
└───────────────────────────────┬─────────────────────────────┘
                                │
┌───────────────────────────────▼─────────────────────────────┐
│                     REST API (Axum Server)                  │
└───────────────────────────────┬─────────────────────────────┘
                                │
┌───────────────────────────────▼─────────────────────────────┐
│                         Search API                          │
└───┬─────────────────────┬─────────────────────┬─────────────┘
        │                     │                     │
    ┌───▼───────┐     ┌───────▼───────┐     ┌───────▼───────┐
    │ Database  │     │ Quantum Core  │     │   Tokenizer   │
    └───────────┘     └───────────────┘     └───────────────┘
        │                     │                     │
    ┌───▼───────┐     ┌───────▼───────┐     ┌───────▼───────┐
    │  Crawler  │     │ Prime Hilbert │     │    Entropy    │
    └───────────┘     └───────────────┘     └───────────────┘
```

## Key Components

### 1. Web Interface

The front-end interface is built with HTML, CSS, and JavaScript. It provides:
- Search input form with options for quantum and persistence scoring
- Results display with detailed scoring information
- About section explaining the quantum search technology

### 2. REST API

Built with the Axum framework, the REST API provides endpoints for:
- `/api/search` - Main search endpoint
- `/api/health` - Server health check
- Static file serving

### 3. Search API

The core search functionality that combines:
- Full-text search (FTS) for initial candidate selection
- Quantum resonance scoring
- Persistence theory scoring
- Result ranking based on combined metrics

### 4. Database Layer

SQLite database with:
- Document storage with compressed text
- Full-text search index
- Vector data storage for quantum computations
- Support for persistence metrics

### 5. Quantum Core

The heart of the search engine, implementing:
- Prime number tokenization
- Vector space representation
- Biorthogonal vector computations
- Complex resonance calculations

### 6. Crawler Components

Advanced web crawler with:
- Robots.txt compliance
- Rate limiting
- Domain filtering
- Parallel processing
- Polite crawling behavior

## Data Flow

1. **Indexing Process**:
   ```
   Web Page → Crawler → Tokenization → Vector Creation → Database Storage
   ```

2. **Search Process**:
   ```
   Query → Tokenization → Vector Creation → 
   FTS Candidates → Quantum Scoring → Persistence Scoring → 
   Result Ranking → Web Response
   ```

## Quantum-Inspired Algorithms

### Prime Hilbert Space

Documents are represented in a sparse vector space where dimensions correspond to prime numbers. This creates a unique mathematical space with interesting properties:

1. **Tokenization**: Text is tokenized and mapped to prime numbers
2. **Vector Creation**: Count frequencies and normalize to create sparse vectors
3. **Dot Product**: Calculate resonance between documents

### Biorthogonal Representation

Each document is represented by complementary "left" and "right" vectors that capture different aspects of the document's semantic structure.

### Complex Resonance

Uses complex numbers to represent both the strength of resonance (real part) and phase information (imaginary part), allowing for richer comparison between documents.

### Persistence Theory

Inspired by thermodynamic principles:

1. **Reversibility**: How much information persists over transformations
2. **Entropy Pressure**: How information resists being "washed out" over time
3. **Buffering Capacity**: Resistance to noise and change

## Performance Considerations

- Database indices optimize search performance
- Document compression reduces storage requirements
- Rate limiting ensures polite web crawling
- Semaphore-based concurrency control 
- Caching of frequently used vectors

## Security Measures

- Input sanitization for all search queries
- Rate limiting to prevent abuse
- robots.txt compliance
- User agent identification
- No execution of JavaScript from crawled sites

## Scaling Strategy

The current implementation uses SQLite for simplicity, but can be scaled by:

1. Replacing SQLite with PostgreSQL or similar for larger indices
2. Implementing a distributed crawler system
3. Setting up multiple search nodes with load balancing
4. Adding a caching layer for popular queries

## Extensions and Future Work

- Image search capability 
- Semantic clustering of results
- Enhanced visualization of quantum relationships
- API for third-party integrations
- Learning from user feedback to improve rankings