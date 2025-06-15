# Resonant Search Engine - Web Setup Guide

This guide will help you set up the Resonant Search Engine as a web-facing search engine using your existing quantum-based algorithm.

## System Requirements

- Rust (latest stable version)
- At least 4GB RAM
- 10GB+ free disk space (more for larger indices)
- Linux, macOS, or Windows operating system

## Directory Structure

After building, your project will have the following structure:

```
resonant_search/
├── Cargo.toml                 # Dependency configuration
├── src/                       # Source code
│   ├── main.rs                # Main application entry point
│   ├── crawler.rs             # Original crawler code 
│   ├── advanced_crawler.rs    # Enhanced web crawler
│   ├── database.rs            # Database integration
│   ├── engine.rs              # Original search engine
│   ├── search_api.rs          # Web API for search
│   ├── web_server.rs          # Axum web server
│   └── ...                    # Other existing files
├── static_templates/          # Template files
│   ├── index.html             # Main HTML template
│   ├── styles.css             # CSS styles
│   ├── app.js                 # Client-side JavaScript
│   ├── logo.svg               # Logo SVG
│   └── favicon.ico            # Favicon
├── static/                    # Generated static files (created at runtime)
└── data/                      # Data storage directory
    └── search_db.sqlite       # SQLite database
```

## Installation Steps

1. Clone the repository (if not already done):
   ```bash
   git clone https://github.com/whisprdev/resonant_search.git
   cd resonant_search
   ```

2. Create the necessary directories:
   ```bash
   mkdir -p static_templates data
   ```

3. Copy the template files into place:
   - Put the HTML, CSS, JavaScript, and SVG files into the `static_templates` directory

4. Build the project:
   ```bash
   cargo build --release
   ```

## Running the Web Search Engine

### Option 1: Start with an Empty Index

1. Start the web server:
   ```bash
   ./target/release/resonant_search serve --port 8080
   ```

2. Access the search interface at `http://localhost:8080`

### Option 2: Crawl Websites First

1. Create a file with seed URLs:
   ```bash
   echo "https://example.com" > seed_urls.txt
   echo "https://another-domain.com" >> seed_urls.txt
   ```

2. Start the crawler:
   ```bash
   ./target/release/resonant_search crawl --urls seed_urls.txt --limit 5000 --max-depth 3
   ```

3. After crawling completes, start the web server:
   ```bash
   ./target/release/resonant_search serve --port 8080
   ```

## Command Line Options

### Crawler Mode
```
resonant_search crawl [OPTIONS]
```

Options:
- `--urls <FILE>`: File containing seed URLs (one per line)
- `--domain <DOMAIN>`: Single domain to crawl
- `--limit <NUM>`: Maximum number of pages to crawl (default: 1000)
- `--max-depth <NUM>`: Maximum crawl depth (default: 3)
- `--workers <NUM>`: Number of concurrent crawlers (default: 10)
- `--stay-in-domain`: Stay within the initial domain(s)
- `--db-path <PATH>`: Path to the database file (default: data/search_db.sqlite)

### Interactive Search Mode
```
resonant_search search [OPTIONS]
```

Options:
- `--db-path <PATH>`: Path to the database file (default: data/search_db.sqlite)
- `--disable-quantum`: Disable quantum-inspired scoring
- `--disable-persistence`: Disable persistence-based scoring

### Web Server Mode
```
resonant_search serve [OPTIONS]
```

Options:
- `--port <PORT>`: Port to listen on (default: 8080)
- `--db-path <PATH>`: Path to the database file (default: data/search_db.sqlite)
- `--disable-quantum`: Disable quantum-inspired scoring
- `--disable-persistence`: Disable persistence-based scoring

## Using Your Existing Index

If you already have a large index created with your original crawler, you'll need to import this data into the new database format. We've provided a utility for this:

```bash
./target/release/resonant_search import --source your_old_data_file --db-path data/search_db.sqlite
```

This will convert your existing index to the new database format, preserving all quantum vector information.

## Customizing the Search Interface

You can customize the appearance and behavior of the search interface by modifying the files in the `static_templates` directory:

- `index.html`: Main HTML structure
- `styles.css`: Visual styling
- `app.js`: Client-side behavior
- `logo.svg`: Search engine logo

After making changes, restart the server for them to take effect.

## Production Deployment

For production deployment, consider the following:

1. Set up a reverse proxy (Nginx, Apache) in front of the application
2. Configure SSL/TLS certificates for HTTPS
3. Set up proper logging and monitoring
4. Use a process manager like systemd or supervisor to ensure the service stays running

Example Nginx configuration:

```nginx
server {
    listen 80;
    server_name search.whispr.dev;
    
    location / {
        proxy_pass http://localhost:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

## Performance Tuning

For better performance with large indices:

1. Consider using a dedicated database server instead of SQLite for very large indices
2. Increase the number of workers for the server (configured in web_server.rs)
3. Use a machine with more RAM for better caching
4. Set up proper database indices for faster queries

## Troubleshooting

- **Database errors**: Ensure the data directory is writable
- **Crawler issues**: Check network connectivity and robots.txt compliance
- **Performance problems**: 
  - Try reducing the number of concurrent workers
  - Check database indices
  - Monitor memory usage
  - Consider more powerful hardware for larger indices

For more help, report issues on the GitHub re