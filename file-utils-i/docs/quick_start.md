## 🚀 Quick Start (5 Minutes to Secure Files)

### Step 1: Get file-utils Running
```bash
# Download and build (30 seconds)
git clone https://github.com/whispr-dev/file-utils.git
cd file-utils
cargo build --release

# Your binary is now at: ./target/release/file-utils
```

### Step 2: Your First Encryption
```bash
# Create a test file
echo "This is my secret data!" > secret.txt

# Encrypt it (you'll be prompted for a key)
./target/release/file-utils encrypt secret.txt

# Result: secret.txt.enc (original file preserved)
```

### Step 3: Decrypt & Verify
```bash
# Decrypt using the same key
./target/release/file-utils decrypt secret.txt.enc

# Check the result
cat secret.txt  # Should match your original!
```

### Step 4: Secure Delete
```bash
# Nuclear option - completely obliterate a file
./target/release/file-utils -s unwanted_file.txt

# On Windows, this uses military-grade techniques:
# - Multiple overwrite passes
# - Attribute removal
# - Long path support
# - Reboot scheduling if needed
```
