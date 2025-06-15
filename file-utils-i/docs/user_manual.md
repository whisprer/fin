# 📖 file-utils User Manual & Quick Start Guide

## 📋 Complete Command Reference

### 🔐 Encryption Commands

```bash
# Basic encryption (interactive key prompt)
file-utils encrypt myfile.pdf

# Encryption with specified key
file-utils encrypt myfile.pdf -k "my_secure_password"

# Choose encryption mode
file-utils encrypt myfile.pdf -m aes      # Traditional AES-256-CTR
file-utils encrypt myfile.pdf -m quantum  # Experimental quantum-resistant

# Encrypt with custom output location
file-utils encrypt myfile.pdf -o /secure/encrypted_file.enc

# Encrypt and immediately secure-delete original
file-utils encrypt myfile.pdf -s
```

### 🔓 Decryption Commands

```bash
# Basic decryption
file-utils decrypt myfile.pdf.enc

# Decrypt with specified key
file-utils decrypt myfile.pdf.enc -k "my_secure_password"

# Decrypt to specific location
file-utils decrypt myfile.pdf.enc -o /restored/myfile.pdf

# Auto-detect encryption mode and decrypt
file-utils decrypt quantum_encrypted.enc -m quantum
```

### 💀 Secure Deletion Commands

```bash
# Secure delete a single file
file-utils -s sensitive_document.docx

# The tool will:
# 1. Overwrite with zeros (Pass 1)
# 2. Overwrite with ones (Pass 2)  
# 3. Overwrite with random data (Pass 3)
# 4. Remove the file entry
# 5. (Windows) Handle stubborn files with special techniques
```

## 🎛️ Advanced Usage Patterns

### 🏢 Batch Operations

**Encrypt Multiple Files**:
```bash
# Encrypt all PDFs in a directory
for file in *.pdf; do
    file-utils encrypt "$file" -k "$MASTER_KEY" -s
done

# Windows PowerShell version
Get-ChildItem *.pdf | ForEach-Object { 
    file-utils encrypt $_.Name -k $env:MASTER_KEY -s 
}
```

**Directory Processing**:
```bash
# Process all files in a directory structure
find /sensitive -type f -name "*.doc*" -exec file-utils encrypt {} -s \;
```

### 🔧 Environment Configuration

**Set Default Encryption Mode**:
```bash
# Linux/macOS
export CRYPTO_MODE=quantum
file-utils encrypt data.bin  # Will use quantum mode

# Windows
set CRYPTO_MODE=aes
file-utils encrypt data.bin  # Will use AES mode
```

**Scripting with Environment Variables**:
```bash
#!/bin/bash
# Secure backup script
export CRYPTO_MODE=aes
BACKUP_KEY="$(cat /secure/backup.key)"

for important_file in ~/documents/*.important; do
    file-utils encrypt "$important_file" -k "$BACKUP_KEY" -o ~/encrypted_backup/
    file-utils -s "$important_file"  # Secure delete original
done
```

### 🎯 Windows-Specific Features

**Long Path Support**:
```bash
# Handle Windows paths >260 characters
file-utils -s "\\?\C:\Very\Long\Path\That\Exceeds\The\Traditional\Windows\260\Character\Limit\For\File\Paths\sensitive.docx"
```

**Stubborn File Handling**:
```bash
# Files locked by other processes
file-utils -s "C:\Windows\Temp\locked_by_service.tmp"

# The tool will:
# 1. Try normal deletion
# 2. Remove file attributes (READ_ONLY, HIDDEN, SYSTEM)
# 3. Attempt to identify locking processes
# 4. Schedule deletion on next reboot if needed
```

## 🔬 Understanding Encryption Modes

### 🏛️ AES Mode (Recommended for Production)

**When to use**:
- Production systems requiring compliance
- Large files (better performance)
- Interoperability with other tools
- Maximum compatibility

**Technical details**:
- Algorithm: AES-256 in CTR mode
- Key derivation: PBKDF2-style for short keys
- IV: 16 random bytes per file
- Format: `[IV][encrypted_data]`

```bash
# Force AES mode
file-utils encrypt confidential.xlsx -m aes -k "corporate_master_key"
```

### 🧪 Quantum Mode (Experimental)

**When to use**:
- Experimental/research purposes
- Future-proofing against quantum attacks
- Small to medium files
- Learning about post-quantum cryptography

**Technical details**:
- Custom RKState algorithm
- Multi-round entropy cascading
- 64-byte entropy pools
- Golden ratio decay factors

```bash
# Enable quantum mode
file-utils encrypt research_data.json -m quantum -k "quantum_key_2024"
```

## 🛡️ Security Best Practices

### ✅ Do's

1. **Test Before Trusting**:
   ```bash
   # Always verify encryption/decryption works
   file-utils encrypt test.txt -k "mykey"
   file-utils decrypt test.txt.enc -k "mykey"
   diff test.txt test.txt  # Should be identical
   ```

2. **Use Strong Keys**:
   ```bash
   # Good: Long, random keys
   file-utils encrypt data.bin -k "$(openssl rand -base64 32)"
   
   # Better: Interactive secure prompts
   file-utils encrypt data.bin  # Prompts securely for key
   ```

3. **Verify Before Secure Delete**:
   ```bash
   # Encrypt first, verify, then secure delete
   file-utils encrypt original.doc -o backup.enc -k "$KEY"
   file-utils decrypt backup.enc -o test_restore.doc -k "$KEY"
   diff original.doc test_restore.doc && file-utils -s original.doc
   ```

### ❌ Don'ts

1. **Don't Use Weak Keys**:
   ```bash
   # Bad examples:
   file-utils encrypt data.bin -k "password123"
   file-utils encrypt data.bin -k "abc"
   ```

2. **Don't Skip Verification**:
   ```bash
   # Dangerous:
   file-utils encrypt important.doc -s  # Deletes original without verification
   ```

3. **Don't Lose Your Keys**:
   ```bash
   # Store keys securely - consider:
   # - Hardware security modules
   # - Password managers  
   # - Encrypted key files
   # - Environment variables (with care)
   ```

## 🚨 Emergency Procedures

### 🆘 Lost Encryption Key

**Unfortunately, there's no recovery mechanism by design**. If you lose your encryption key:

1. **Check common locations**:
   - Password managers
   - Environment variables
   - Backup scripts
   - Documentation

2. **Try variations**:
   - Different capitalizations
   - With/without special characters
   - Shorter/longer versions you might remember

3. **Learn for next time**:
   - Use a password manager
   - Store keys separately from encrypted files
   - Consider key escrow for critical data

### 🔧 File Won't Delete (Windows)

```bash
# Try escalating privileges
# 1. Run Command Prompt as Administrator
runas /user:Administrator cmd

# 2. Use the long path format
file-utils -s "\\?\C:\path\to\stubborn\file.exe"

# 3. Check if file is in use
handle.exe C:\path\to\stubborn\file.exe  # SysInternals tool

# 4. Reboot and try again (file may be scheduled for deletion)
```

### 🔍 Verify Secure Deletion Worked

```bash
# On Linux/macOS, check if file data is recoverable
grep -a "known_file_content" /dev/sda1  # Requires root, BE CAREFUL

# Windows: Use forensic tools like:
# - DBAN (Darik's Boot and Nuke)
# - SDelete (SysInternals)
# - PhotoRec/TestDisk

# Best practice: Test on a VM first
```

## 📊 Performance Tuning

### ⚡ Optimization Tips

1. **Use AES for large files**:
   ```bash
   # AES is faster for files >100MB
   file-utils encrypt huge_database.sql -m aes
   ```

2. **Process in parallel**:
   ```bash
   # Linux/macOS parallel processing
   find . -name "*.sensitive" | xargs -P 4 -I {} file-utils encrypt {} -s
   ```

3. **SSD considerations**:
   ```bash
   # On SSDs, secure deletion may not be 100% effective
   # Consider full disk encryption (BitLocker/LUKS) as primary security
   ```

### 📈 Benchmarking

```bash
# Time encryption operations
time file-utils encrypt large_file.iso -m aes
time file-utils encrypt large_file.iso -m quantum

# Monitor system resources
htop  # Linux/macOS
tasklist  # Windows

# Profile memory usage
/usr/bin/time -v file-utils encrypt big_file.bin  # Linux
```

## 🔗 Integration Examples

### 🐍 Python Integration

```python
import subprocess
import os

def encrypt_file(filepath, key, mode='aes'):
    """Encrypt a file using file-utils"""
    cmd = ['file-utils', 'encrypt', filepath, '-k', key, '-m', mode]
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.returncode == 0

def secure_delete(filepath):
    """Securely delete a file"""
    cmd = ['file-utils', '-s', filepath]
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result.returncode == 0

# Usage example
if encrypt_file('sensitive.txt', os.environ['SECRET_KEY']):
    print("Encryption successful")
    if secure_delete('sensitive.txt'):
        print("Original file securely deleted")
```

### 🟦 PowerShell Integration

```powershell
function Invoke-FileUtilsEncrypt {
    param(
        [string]$Path,
        [string]$Key,
        [string]$Mode = 'aes',
        [switch]$SecureDelete
    )
    
    $args = @('encrypt', $Path, '-k', $Key, '-m', $Mode)
    if ($SecureDelete) { $args += '-s' }
    
    & file-utils @args
    return $LASTEXITCODE -eq 0
}

# Usage
if (Invoke-FileUtilsEncrypt -Path "C:\sensitive.docx" -Key $env:MASTER_KEY -SecureDelete) {
    Write-Host "File encrypted and original securely deleted"
}
```

### 🐧 Bash Integration

```bash
#!/bin/bash
# Secure backup function
secure_backup() {
    local source_file="$1"
    local backup_dir="$2"
    local key="$3"
    
    # Encrypt to backup location
    file-utils encrypt "$source_file" -o "$backup_dir/$(basename "$source_file").enc" -k "$key"
    
    # Verify encryption worked
    if [ $? -eq 0 ]; then
        # Secure delete original
        file-utils -s "$source_file"
        echo "✅ $source_file backed up and securely deleted"
    else
        echo "❌ Encryption failed for $source_file"
        return 1
    fi
}

# Usage
secure_backup "/home/user/confidential.pdf" "/backup/encrypted/" "$BACKUP_KEY"
```

## 🧪 Testing & Validation

### 🔬 Unit Testing Your Setup

```bash
# Create test environment
mkdir file-utils-test
cd file-utils-test

# Test 1: Basic encryption/decryption
echo "Test data 123" > test1.txt
file-utils encrypt test1.txt -k "testkey123"
file-utils decrypt test1.txt.enc -k "testkey123"
diff test1.txt test1.txt && echo "✅ Basic test passed"

# Test 2: Quantum mode
echo "Quantum test data" > test2.txt
file-utils encrypt test2.txt -k "quantumkey" -m quantum
file-utils decrypt test2.txt.enc -k "quantumkey" -m quantum
diff test2.txt test2.txt && echo "✅ Quantum test passed"

# Test 3: Secure deletion
echo "Delete me" > test3.txt
file-utils -s test3.txt
[ ! -f test3.txt ] && echo "✅ Secure deletion test passed"

# Cleanup
cd ..
rm -rf file-utils-test
```

### 🎯 Production Readiness Checklist

- [ ] **Functionality tested** with representative data
- [ ] **Key management** strategy implemented
- [ ] **Backup procedures** established  
- [ ] **Recovery processes** documented
- [ ] **Performance** acceptable for your use case
- [ ] **Security review** completed
- [ ] **User training** provided
- [ ] **Monitoring** and logging configured

---

**🎓 Congratulations!** You're now a file-utils expert. Remember: with great power comes great responsibility - use these tools wisely to protect what matters most.

**💡 Pro Tip**: Start with small, non-critical files to build confidence before protecting your most sensitive data.

*Happy encrypting, fren!* 🔐✨