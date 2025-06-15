# Quick Start Guide 🚀

Get your Quantum Resonant Search Engine running in under 5 minutes!

## 🏁 Choose Your Adventure

### 🔍 Local Filesystem Search (Recommended)
**Perfect for**: Searching your entire computer, project directories, document collections
**Time**: ~2 minutes setup + indexing time

### 🌐 Web Search Engine  
**Perfect for**: Building a search engine for websites, creating a custom Google
**Time**: ~5 minutes setup + crawling time

---

## 🔍 Local Filesystem Search Setup

### Step 1: Install Prerequisites
```bash
# Install Rust (if not already installed)
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source ~/.cargo/env

# Verify installation
rustc --version
cargo --version
```

### Step 2: Clone and Build
```bash
# Clone the repository
git clone https://github.com/whisprdev/quantum-resonant-search.git
cd quantum-resonant-search

# Create data directories
mkdir -p data/checkpoints

# Build in release mode (optimized)
cargo build --release
```

### Step 3: Run the Local Search Engine
```bash
# Start the quantum filesystem oracle
cargo run --release
```

### Step 4: Follow the Setup Wizard
```
🧠 Quantum Resonant Local Filesystem Search Engine
    "The closest thing to mindreading for files"
=====================================================

📂 Configure search paths:
1. Scan entire drive (C:\ or /)
2. Scan home directory  
3. Custom paths
4. Network paths (SMB/NFS)
> 2

⚛️  Configure quantum features:
Enable quantum-inspired scoring? (y/n) > y
Enable persistence theory scoring? (y/n) > y

Fragility parameter (0.1-1.0, default: 0.2): > 0.2
Entropy weight (0.1-1.0, default: 0.1): > 0.1

🔍 Starting quantum filesystem scan...
📂 Indexed: 1,247 files, 156 dirs, Current: /home/user/projects/...
⚡ Quantum scan complete! 12,486 files indexed in 2m 15s
💾 Index saved to quantum_fs_index.db

🧮 Building quantum resonance vectors...
⚡ Quantum vectors built in 847ms

👁️  File watcher started for real-time updates

🌟 Quantum search ready! Enter queries or commands:
Commands: 'reindex', 'stats', 'fuzzy <pattern>', 'quantum <query>', 'quit'
```

### Step 5: Start Searching!
```bash
🔮 > rust code async networking

🌟 Quantum Resonant Matches for 'rust code async networking' (23ms):
────────────────────────────────────────────────────────────────────────────────

[1] 📄 tcp_server.rs
    📂 .../projects/network-tools/src/tcp_server.rs
    ⚛️  Resonance: 0.847 | Quantum: 0.792 | Persistence: 0.634 | Combined: 0.758
    📝 use tokio::net::TcpListener; async fn handle_connection(socket: TcpStream)...
    📊 Size: 15.2 KB | Modified: 2h ago

[2] 📄 async_client.rs  
    📂 .../rust-projects/networking/src/async_client.rs
    ⚛️  Resonance: 0.723 | Quantum: 0.681 | Persistence: 0.712 | Combined: 0.705
    📝 Asynchronous TCP client implementation using tokio runtime...
    📊 Size: 8.7 KB | Modified: 1d ago

[3] 📄 network_config.toml
    📂 .../projects/server-config/network_config.toml  
    ⚛️  Resonance: 0.456 | Quantum: 0.523 | Persistence: 0.678 | Combined: 0.552
    📝 [network] async_workers = 10 tcp_port = 8080 buffer_size = 4096...
    📊 Size: 2.1 KB | Modified: 3d ago

🔮 > fuzzy old_proj

🎯 Fuzzy Matches for 'old_proj' (12ms):
────────────────────────────────────────────────────────────────

[1] 📄 README.md (score: 8.2)
    📂 .../archived/old_project_backup/README.md
    📊 Size: 4.5 KB | Type: Markdown

[2] 📄 main.rs (score: 7.8)
    📂 .../old_projects/rust_experiments/src/main.rs  
    📊 Size: 12.3 KB | Type: Code
```

### Step 6: Use Advanced Commands
```bash
🔮 > stats
📊 Quantum Search Engine Statistics:
──────────────────────────────────────────────────
📁 Total files indexed: 12,486
🧮 Quantum vectors: 12,486
💾 Total indexed size: 1.2 GB

📋 File type distribution:
   Code: 4,247 | Text: 3,156 | Config: 1,891 | Markdown: 1,204

⚛️  Quantum features:
   Quantum scoring: enabled
   Persistence theory: enabled  
   Real-time monitoring: active

🔮 > reindex
🔄 Starting full reindex...
⚡ Reindex complete! 13,247 files indexed

🔮 > quit
🌟 Quantum search session ended. Index preserved for next time!
```

---

## 🌐 Web Search Engine Setup

### Step 1: Same Prerequisites + Build
```bash
# Follow steps 1-2 from Local Setup above
git clone https://github.com/whisprdev/quantum-resonant-search.git
cd quantum-resonant-search
cargo build --release
```

### Step 2: Create Seed URLs
```bash
# Create a file with websites to crawl
cat > data/seed_urls.txt << EOF
https://docs.rust-lang.org
https://quantum-journal.org  
https://arxiv.org/list/quant-ph/recent
https://en.wikipedia.org/wiki/Quantum_mechanics
https://stackoverflow.com/questions/tagged/rust
EOF
```

### Step 3: Configure and Start Crawling
```bash
# Start the crawler
cargo run --release
```

### Step 4: Follow Crawler Setup
```
🧠 Quantum Resonant Search Engine
=====================================================

Found existing checkpoint. Load it? (y/n) > n
Starting fresh index

⚛️  Enable quantum-inspired scoring? (y/n) > y
⚛️  Enable persistence theory scoring? (y/n) > y

Fragility parameter (0.1-1.0, default: 0.2): > 0.2
Entropy weight (0.1-1.0, default: 0.1): > 0.1

🌐 Configure search paths:
1. Use default seed URLs
2. Load URLs from a file  
3. Specify a single domain to crawl
4. Skip crawling (use existing index)
> 2

Enter the path to your URL list file:
> data/seed_urls.txt

📁 Loading URLs from: data/seed_urls.txt

How many pages would you like to crawl? (default: 1000, max: 25000)
> 5000

Maximum crawl depth? (default: 3, higher values follow more links)  
> 3

How many concurrent workers? (default: 10, max recommended: 20)
> 12

🕷️  Starting web crawling with 12 workers, targeting 5000 pages with max depth 3...

📂 Indexed: 847 files, 156 dirs, Current: https://docs.rust-lang.org/book/...
📂 Indexed: 1,456 files, 234 dirs, Current: https://quantum-journal.org/articles/...
...
⚡ Quantum scan complete! 4,892 pages indexed in 18m 32s
💾 Index saved to latest.checkpoint
📤 Index exported to data/index_export.csv

🧮 Building quantum resonance vectors...
⚡ Quantum vectors built in 3.2s
```

### Step 5: Start Web Interface
```bash
# In the search loop, type to start web server
🔮 > serve

🌐 Starting web server...
🚀 Web interface available at: http://localhost:8080
📡 API endpoints:
   GET  /api/search?q=<query>
   GET  /api/health  
   GET  /api/stats
```

### Step 6: Access Web Interface
Open your browser to `http://localhost:8080`:

```html
┌─────────────────────────────────────────────────────────┐
│                 🌌 Resonant Search                      │
│               Quantum-Inspired Web Search Engine        │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  [ quantum mechanics entropy collapse        ] 🔍      │
│                                                         │
│  ☑ Enable Quantum Scoring                              │
│  ☑ Enable Persistence Scoring                          │
│                                                         │
├─────────────────────────────────────────────────────────┤
│ About 1,247 results found in 0.023 seconds             │
│                                                         │
│ [1] Introduction to Quantum Mechanics                   │
│     https://quantum-journal.org/intro-qm                │
│     Quantum mechanics is the fundamental theory in      │
│     physics that describes nature at the smallest...    │
│     Resonance: 0.847 | Combined Score: 0.758           │
│                                                         │
│ [2] Entropy and Information Theory                       │
│     https://docs.rust-lang.org/entropy-theory           │
│     In information theory, entropy is a measure of      │
│     the average information content in a message...     │
│     Resonance: 0.723 | Combined Score: 0.705           │
└─────────────────────────────────────────────────────────┘
```

---

## 🎯 Example Queries

### Local Filesystem Queries
```bash
# Find by concept/topic
🔮 > "machine learning tensorflow python"
🔮 > "database configuration postgres docker"
🔮 > "encryption security cryptography rust"

# Find by file characteristics  
🔮 > "config files yaml kubernetes"
🔮 > "recent rust projects async"
🔮 > "large files logs errors"

# Fuzzy filename search (when you can't remember exact names)
🔮 > "fuzzy old_backup_proj"
🔮 > "fuzzy neural_net_exp"
🔮 > "fuzzy client_server"

# Complex conceptual searches
🔮 > "entropy information theory shannon quantum"
🔮 > "prime numbers mathematics riemann hypothesis" 
🔮 > "consciousness binding problem neuroscience"
```

### Web Search Queries
```bash
# Academic/research topics
🔮 > "quantum entanglement experiments 2024"
🔮 > "machine learning transformer architecture"
🔮 > "climate change carbon dioxide levels"

# Technical documentation
🔮 > "rust async programming tokio"
🔮 > "kubernetes deployment best practices"
🔮 > "react hooks state management"

# Domain-specific knowledge
🔮 > "cryptographic hash functions security"
🔮 > "neural network backpropagation algorithm"
🔮 > "distributed systems consensus protocols"
```

## 🔧 Configuration Tips

### Performance Tuning
```bash
# For large directories (100k+ files)
export RUST_LOG=warn                    # Reduce logging
ulimit -n 65536                        # Increase file descriptors

# For better search performance  
🔮 > compress                          # Compress documents in memory
🔮 > reindex                           # Rebuild index periodically
```

### Memory Management
```bash
# Monitor memory usage
🔮 > stats

📊 Quantum Search Engine Statistics:
💾 Memory usage: 1.2 GB
🗜️  Compression ratio: 4.2x
⚡ Average search time: 23ms
```

### Custom File Types
Add support for new file types by editing `src/filesystem_indexer.rs`:
```rust
fn from_extension(ext: &str) -> Self {
    match ext.to_lowercase().as_str() {
        // Add your file types here
        "log" | "out" | "err" => FileType::Log,
        "dockerfile" => FileType::Config,
        "py" | "pyw" => FileType::Code,
        // ...
    }
}
```

## 🐛 Troubleshooting

### Common Issues

**"Permission denied" errors**
```bash
# Run with appropriate permissions
sudo cargo run --release  # Linux/Mac
# Or change to user-owned directory
```

**"Too many open files"**
```bash  
# Increase file descriptor limit
ulimit -n 65536
```

**Slow indexing performance**
```bash
# Reduce worker count
🔮 > reindex
How many workers? > 4  # Instead of 10+

# Or exclude large directories
# Edit excluded_patterns in config
```

**Out of memory during large crawls**
```bash
# Enable compression during crawl
🔮 > compress

# Or reduce crawl size  
How many pages? > 1000  # Instead of 25000
```

**No search results**
```bash
# Check if index is built
🔮 > stats
📁 Total files indexed: 0  # ← Problem!

# Rebuild index
🔮 > reindex
```

### Advanced Debugging

**Enable debug logging**
```bash
RUST_LOG=debug cargo run --release
```

**Check file processing**
```bash
# Look for file processing errors in logs
tail -f search.log | grep "Error processing"
```

**Verify quantum scoring**
```bash
# Test with known good query
🔮 > "test entropy"  # Should return results if index has any content
```

## 📈 Next Steps

### Extend Functionality
1. **Add new file type parsers** - Support PDFs, Word docs, etc.
2. **Build custom extractors** - Parse code comments, metadata
3. **Create domain-specific configs** - Optimize for your use case
4. **Add visualizations** - Plot resonance fields, entropy landscapes

### Scale Up
1. **Production deployment** - Set up web server with nginx
2. **Distributed crawling** - Multiple crawler instances  
3. **Database migration** - Move from SQLite to PostgreSQL
4. **API integration** - Build tools that use the search API

### Research Extensions
1. **EEG correlation experiments** - Test consciousness-search links
2. **Quantum visualization** - 3D plots of Hilbert space vectors
3. **HQE simulations** - Validate theoretical predictions
4. **Persistence optimization** - Tune thermodynamic parameters

---

**Need help?** Check the full documentation in `docs/` or create an issue on GitHub.

**Want to contribute?** See `CONTRIBUTING.md` for guidelines.

🌟 **Happy searching, fren!** 🌟