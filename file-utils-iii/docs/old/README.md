# Quantum Resonant Search Engine
A novel search engine combining quantum mechanics concepts and persistence theory to find resonant patterns in content.

## Overview
Resonant Search is not an ordinary search engine - it's a quantum-inspired system that evaluates content based on both traditional similarity metrics and advanced concepts from quantum physics and information theory. Unlike traditional search engines that focus only on keyword matching, Resonant Search evaluates documents based on:

- Resonance - How strongly document vectors align with query vectors in a prime number Hilbert space
- Entropy - The information density and diversity within documents
- Quantum Effects - Non-Hermitian dynamics with complex-valued scores capturing both amplitude and phase
- Persistence Theory - How well documents maintain their structure under entropy pressure

## Mathematical Foundations
### Prime Number Tokenization
The engine tokenizes text into unique prime numbers, creating a mathematical representation of linguistic patterns. This approach leverages the fundamental theorem of arithmetic (unique prime factorization) to encode semantic relationships.
Text → Tokens → Prime Numbers → Vector Space

### Quantum-Inspired Scoring
The engine implements advanced quantum-inspired scoring mechanisms:

### Biorthogonal Vectors: Each document is represented by both "left" and "right" eigenvectors, similar to non-Hermitian quantum systems:

ruststruct BiorthogonalVector {
    left: PrimeVector,
    right: PrimeVector,
}

### Complex Resonance: Document-query relationships are measured using complex numbers with both magnitude and phase:

rustlet complex_res = resonance_complex(&query_vec, &doc.vector, decay_factor);

### Quantum Jumps: As users search, documents undergo "quantum jumps" - sudden changes in their state based on measurement interactions:

rustdoc.reversibility = doc.reversibility * 0.9 + 0.1 * (resonance * importance);

## Persistence Theory
Inspired by thermodynamic stability models, documents are evaluated for their "persistence" under entropy:

- Reversibility (η): How much mutual information is retained across transformations
- Entropy Pressure (Q): The tendency toward disorder, affected by document age
- Buffering Capacity (T): The document's resistance to collapse through redundancy or symmetry
- Fragility (α): How sensitive the document is to entropy-induced collapse

The fundamental equation guiding this approach is:
S(η) = exp[-α(1-η)(Q/T)]
Where S(η) represents the probability of a structure persisting under entropy pressure.
Features

## Web Crawling: Crawl up to 25,000 pages with link following up to specified depth
- Quantum Scoring: Calculate document relevance using non-Hermitian quantum dynamics
- Persistence Scoring: Evaluate documents based on their thermodynamic stability
- Adaptive Learning: Documents evolve through quantum jumps as users interact with them
- Memory Efficiency: Automatic document compression for large-scale crawling
- Checkpointing: Save and resume crawling progress
- CSV Export: Export your index for analysis in other tools

## Getting Started
Prerequisites

Rust (latest stable version)
Cargo package manager

Installation

Clone the repository:

bashgit clone https://github.com/yourusername/resonant_search.git
cd resonant_search

Build the project:

bash
cargo build --release
Usage

Run the search engine:

bash
cargo run --release

When prompted:

Choose whether to enable quantum-inspired scoring
Choose whether to enable persistence theory scoring
Configure parameters like fragility (α) and entropy weight
Select seed URLs or load from file


Crawling and indexing:

Set the number of pages to crawl (up to 25,000)
Set the maximum crawl depth
Set the number of crawler workers


Searching:

Enter queries to find resonant documents
Observe how documents respond to quantum jumps over time
Use special commands:

export - Save your index to CSV
checkpoint - Save a checkpoint
compress - Compress all documents to save memory





Example
Enter your resonant query (or type 'quit' to exit):
> quantum harmonic oscillators

Searching for resonant matches...

Top Resonant Matches:
[1] Introduction to Quantum Physics
    URL:            https://example.com/quantum-physics
    Resonance:      0.7842
    Δ Entropy:      0.2154
    Standard Score: 0.7627
    Quantum Score:  0.8125
    Persist. Score: 0.6543
    Combined Score: 0.7481
    Preview:        In quantum mechanics, the harmonic oscillator is one of the most important model systems...
How It Works
Crawling Process

The crawler starts with seed URLs and follows links up to a specified depth
For each page:

Extracts text and links
Tokenizes content into prime numbers
Builds vector representations
Calculates entropy and persistence metrics



## Search Process

The query is tokenized into prime numbers
For each document:

Standard score = resonance - (entropy_delta * weight)
Quantum score = complex resonance with phase information
Persistence score = thermodynamic stability under entropy
Combined score = weighted sum of all three


Results are sorted by combined score
After each search, a quantum jump is applied to update document states based on the interaction

Quantum Jump Mechanism
The quantum jump represents a measurement-induced state change:

Documents that resonate with queries are "boosted"
Their reversibility is increased
They appear "fresher" in future searches
This creates a feedback loop where frequently matched documents evolve over time

## Advanced Usage
Custom Seed URLs
Create a text file with URLs (one per line):
https://example.com/page1
https://example.com/page2
Then choose "Load URLs from a file" when prompted.
Adjusting Parameters

Fragility (α): Higher values make documents more sensitive to entropy
Entropy Weight: Controls how strongly entropy differences affect scores
Trend Decay: Controls how quickly document relevance decays over time

Domain-Specific Crawling
Choose "Specify a single domain to crawl" to focus on a specific website.
Troubleshooting

Memory Issues: Type compress during search to compress all documents
Slow Crawling: Reduce the number of worker threads
Blocked By Websites: The crawler implements politeness features but some sites may still block it

## Technical Details
Resonant Search uses several innovative techniques:

Prime Number Vector Space: Documents and queries are represented as sparse vectors in a Hilbert space where dimensions correspond to prime numbers
Non-Hermitian Dynamics: While traditional quantum mechanics uses Hermitian operators for observables, this engine draws inspiration from non-Hermitian quantum mechanics to model information decay and amplification
Lindblad Evolution: For quantum master equations describing how document relevance evolves over time
Biorthogonal Matching: Using both left and right eigenvectors for richer query-document relationships
Thermodynamic Stability: Evaluating documents based on their stability under entropy pressure using persistence theory

## References

"Open Quantum Systems and Quantum Information Dynamics"
"The Dual Kernel Principle: A Thermodynamic Filter for Navigating the Multiverse"
"Persistence Theory: A Toolkit for the Millennium Prize Problems"
"Non-Hermitian Quantum Mechanics and the Biorthogonal Basis"

Acknowledgments

Inspired by concepts from quantum mechanics and thermodynamics
Special thanks to the Rust community for excellent libraries
& ofc. Claude n G-Petey. :)