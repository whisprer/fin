# 💡 file-utils Usage Examples & Real-World Scenarios

## 🏠 Personal Use Cases

### 📸 Protecting Personal Photos & Videos

```bash
# Scenario: Encrypting family photos before cloud backup
cd ~/Pictures

# Method 1: Interactive encryption (most secure)
file-utils encrypt "Family Vacation 2024/"
# You'll be prompted for a secure key

# Method 2: Using quantum mode for extra future-proofing
file-utils encrypt "Wedding Photos/" -m quantum -k "$(date +%s | sha256sum | head -c 32)"

# Method 3: Encrypt and secure delete originals (be careful!)
file-utils encrypt "Private Videos/" -s
# ⚠️ Warning: This permanently deletes the originals after encryption
```

### 💰 Financial Document Security

```bash
# Secure your tax documents and financial records
cd ~/Documents/Financial

# Create encrypted backup with timestamp
backup_key="financial_backup_$(date +%Y%m%d)"
file-utils encrypt "Tax Returns/" -o ~/encrypted_backups/ -k "$backup_key"
file-utils encrypt "Bank Statements/" -o ~/encrypted_backups/ -k "$backup_key"

# Verify encryptions worked before cleaning up
file-utils decrypt ~/encrypted_backups/Tax\ Returns.enc -k "$backup_key" -o /tmp/verify_tax/
diff -r "Tax Returns/" /tmp/verify_tax/ && echo "✅ Tax backup verified"

# Store the key securely (example using password manager)
echo "$backup_key" | pass insert financial/backup_key_2024
```

### 🔐 Password Manager Backup

```bash
# Backup your password database with extra security
cd ~/.password-store

# Create quantum-encrypted backup
file-utils encrypt .password-store.gpg -m quantum -o ~/secure_backups/passwords_quantum.enc

# Traditional AES backup as well
file-utils encrypt .password-store.gpg -m aes -o ~/secure_backups/passwords_aes.enc

# Test both backups
file-utils decrypt ~/secure_backups/passwords_quantum.enc -m quantum -o /tmp/test_quantum.gpg
file-utils decrypt ~/secure_backups/passwords_aes.enc -m aes -o /tmp/test_aes.gpg

# Compare to ensure both methods work
diff .password-store.gpg /tmp/test_quantum.gpg && echo "✅ Quantum backup verified"
diff .password-store.gpg /tmp/test_aes.gpg && echo "✅ AES backup verified"
```

## 🏢 Enterprise & Business Use Cases

### 🏥 Healthcare Data Protection (HIPAA Compliance)

```bash
#!/bin/bash
# HIPAA-compliant patient data encryption script

HIPAA_KEY="$(cat /secure/hipaa_master.key)"
PATIENT_DATA_DIR="/data/patient_records"
ENCRYPTED_ARCHIVE="/secure/encrypted_patient_data"

# Function to encrypt patient files with audit trail
encrypt_patient_data() {
    local patient_file="$1"
    local timestamp=$(date -Iseconds)
    
    echo "[$timestamp] Encrypting: $patient_file" >> /var/log/hipaa_encryption.log
    
    # Use AES mode for compliance and interoperability
    if file-utils encrypt "$patient_file" -m aes -k "$HIPAA_KEY" -o "$ENCRYPTED_ARCHIVE/"; then
        echo "[$timestamp] SUCCESS: $patient_file encrypted" >> /var/log/hipaa_encryption.log
        
        # Secure delete original after verification
        if file-utils decrypt "$ENCRYPTED_ARCHIVE/$(basename "$patient_file").enc" -k "$HIPAA_KEY" -o "/tmp/verify_$(basename

Secure delete original after verification
    if file-utils decrypt "$ENCRYPTED_ARCHIVE/$(basename "$patient_file").enc" -k "$HIPAA_KEY" -o "/tmp/verify_$(basename "$patient_file")" && \
       diff "$patient_file" "/tmp/verify_$(basename "$patient_file")" > /dev/null; then
        
        file-utils -s "$patient_file"
        echo "[$timestamp] SUCCESS: $patient_file securely deleted after verification" >> /var/log/hipaa_encryption.log
        rm "/tmp/verify_$(basename "$patient_file")"
    else
        echo "[$timestamp] ERROR: Verification failed for $patient_file - original NOT deleted" >> /var/log/hipaa_encryption.log
    fi
else
    echo "[$timestamp] ERROR: Encryption failed for $patient_file" >> /var/log/hipaa_encryption.log
fi
}
Process all patient files
find "$PATIENT_DATA_DIR" -name ".pdf" -o -name ".doc*" -o -name "*.xml" | 
while read -r patient_file; do
encrypt_patient_data "$patient_file"
done
Generate compliance report
echo "HIPAA Encryption Report - $(date)" > /var/log/hipaa_daily_report.txt
echo "Files processed: $(grep -c "Encrypting:" /var/log/hipaa_encryption.log)" >> /var/log/hipaa_daily_report.txt
echo "Successful encryptions: $(grep -c "SUCCESS.*encrypted" /var/log/hipaa_encryption.log)" >> /var/log/hipaa_daily_report.txt
echo "Secure deletions: $(grep -c "securely deleted" /var/log/hipaa_encryption.log)" >> /var/log/hipaa_daily_report.txt

### 💼 Legal Document Management

```powershell
# PowerShell script for law firm document protection
param(
    [string]$CaseNumber,
    [string]$ClientName,
    [string]$DocumentPath
)

# Legal-grade encryption for sensitive case files
$LegalMasterKey = Get-Content "C:\SecureKeys\legal_master.key" -Raw
$CaseKey = "$LegalMasterKey$CaseNumber$ClientName" | ConvertTo-SecureString -AsPlainText -Force | ConvertFrom-SecureString

# Create case-specific encrypted archive
$EncryptedPath = "\\FileServer\EncryptedCases\$CaseNumber"
New-Item -Path $EncryptedPath -ItemType Directory -Force

# Encrypt all case documents
Get-ChildItem -Path $DocumentPath -Recurse -File | ForEach-Object {
    $OutputFile = Join-Path $EncryptedPath "$($_.Name).enc"
    
    Write-Host "Encrypting: $($_.FullName)" -ForegroundColor Yellow
    
    & file-utils encrypt $_.FullName -k $CaseKey -o $OutputFile -m aes
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ Encrypted: $($_.Name)" -ForegroundColor Green
        
        # Verify encryption before secure deletion
        $TempVerify = "$env:TEMP\verify_$($_.Name)"
        & file-utils decrypt $OutputFile -k $CaseKey -o $TempVerify
        
        if ((Get-FileHash $_.FullName).Hash -eq (Get-FileHash $TempVerify).Hash) {
            Write-Host "✅ Verification passed for: $($_.Name)" -ForegroundColor Green
            & file-utils -s $_.FullName
            Remove-Item $TempVerify -Force
        } else {
            Write-Host "❌ Verification failed for: $($_.Name)" -ForegroundColor Red
        }
    }
}

# Generate case encryption manifest
@"
Case Encryption Report
======================
Case Number: $CaseNumber
Client: $ClientName
Date: $(Get-Date)
Encrypted Location: $EncryptedPath
Key Identifier: SHA256(CaseKey)[0:8] = $((Get-FileHash -InputStream ([IO.MemoryStream]::new([Text.Encoding]::UTF8.GetBytes($CaseKey)))).Hash.Substring(0,8))
"@ | Out-File "$EncryptedPath\CASE_MANIFEST.txt"
🏭 Manufacturing Trade Secrets
bash#!/bin/bash
# Industrial espionage protection for manufacturing data

# Configuration
TRADE_SECRET_DIR="/data/proprietary"
SECURE_VAULT="/vault/encrypted_ip"
QUANTUM_KEY_FILE="/keys/quantum_industrial.key"
AUDIT_LOG="/var/log/trade_secret_protection.log"

# Function to log with timestamp
log_action() {
    echo "[$(date -Iseconds)] $1" | tee -a "$AUDIT_LOG"
}

# Ultra-secure encryption for trade secrets
protect_trade_secrets() {
    local secret_type="$1"
    local file_pattern="$2"
    
    log_action "Starting protection of $secret_type files"
    
    # Use quantum mode for maximum future protection
    QUANTUM_KEY=$(cat "$QUANTUM_KEY_FILE")
    
    find "$TRADE_SECRET_DIR" -name "$file_pattern" -type f | while read -r secret_file; do
        log_action "Processing: $secret_file"
        
        # Create secure filename that doesn't reveal content
        secure_filename="ts_$(date +%s)_$(openssl rand -hex 8).enc"
        output_path="$SECURE_VAULT/$secure_filename"
        
        # Quantum encryption with secure deletion
        if file-utils encrypt "$secret_file" -m quantum -k "$QUANTUM_KEY" -o "$output_path"; then
            log_action "SUCCESS: Quantum encrypted $secret_file -> $secure_filename"
            
            # Verification step
            temp_verify="/tmp/verify_$(basename "$secret_file")"
            if file-utils decrypt "$output_path" -m quantum -k "$QUANTUM_KEY" -o "$temp_verify" && \
               sha256sum "$secret_file" "$temp_verify" | awk '{print $1}' | uniq | wc -l | grep -q "1"; then
                
                # Secure delete original
                file-utils -s "$secret_file"
                log_action "SUCCESS: Verified and securely deleted $secret_file"
                
                # Create mapping file (encrypted)
                echo "$secret_file -> $secure_filename" | \
                file-utils encrypt - -k "$QUANTUM_KEY" -o "$SECURE_VAULT/mapping_$(date +%s).enc"
                
                rm -f "$temp_verify"
            else
                log_action "ERROR: Verification failed for $secret_file"
            fi
        else
            log_action "ERROR: Encryption failed for $secret_file"
        fi
    done
}

# Protect different types of trade secrets
protect_trade_secrets "CAD_Files" "*.dwg"
protect_trade_secrets "Manufacturing_Specs" "*.pdf"
protect_trade_secrets "Process_Documentation" "*.docx"
protect_trade_secrets "Chemical_Formulas" "*.xlsx"
protect_trade_secrets "Source_Code" "*.cpp"

# Generate executive summary
cat > "/tmp/trade_secret_report.txt" << EOF
CONFIDENTIAL - TRADE SECRET PROTECTION REPORT
=============================================
Date: $(date)
Protected Files: $(grep -c "SUCCESS: Quantum encrypted" "$AUDIT_LOG")
Secure Deletions: $(grep -c "securely deleted" "$AUDIT_LOG")
Failed Operations: $(grep -c "ERROR:" "$AUDIT_LOG")
Vault Location: $SECURE_VAULT
Security Level: QUANTUM-ENHANCED ENCRYPTION

Next Review Due: $(date -d "+30 days")
EOF

# Encrypt the report itself
file-utils encrypt "/tmp/trade_secret_report.txt" -m quantum -k "$QUANTUM_KEY" -o "$SECURE_VAULT/executive_report_$(date +%Y%m%d).enc"
file-utils -s "/tmp/trade_secret_report.txt"
🔬 Development & DevOps Use Cases
🔐 API Key & Secrets Management
bash#!/bin/bash
# Secure developer secrets and API keys

SECRETS_DIR="$HOME/.config/secrets"
ENCRYPTED_SECRETS="$HOME/.encrypted_secrets"
DEV_KEY_FILE="$HOME/.ssh/dev_encryption.key"

# Initialize encrypted secrets vault
init_secrets_vault() {
    mkdir -p "$ENCRYPTED_SECRETS"
    
    # Generate or load development encryption key
    if [ ! -f "$DEV_KEY_FILE" ]; then
        echo "Generating new development encryption key..."
        openssl rand -base64 32 > "$DEV_KEY_FILE"
        chmod 600 "$DEV_KEY_FILE"
        echo "🔑 New key generated at: $DEV_KEY_FILE"
    fi
}

# Encrypt and store a secret
store_secret() {
    local secret_name="$1"
    local secret_value="$2"
    local dev_key=$(cat "$DEV_KEY_FILE")
    
    echo "$secret_value" | file-utils encrypt - -k "$dev_key" -o "$ENCRYPTED_SECRETS/$secret_name.enc"
    echo "✅ Stored secret: $secret_name"
}

# Retrieve and decrypt a secret
get_secret() {
    local secret_name="$1"
    local dev_key=$(cat "$DEV_KEY_FILE")
    
    if [ -f "$ENCRYPTED_SECRETS/$secret_name.enc" ]; then
        file-utils decrypt "$ENCRYPTED_SECRETS/$secret_name.enc" -k "$dev_key" 2>/dev/null
    else
        echo "❌ Secret not found: $secret_name" >&2
        return 1
    fi
}

# Rotate all secrets with new encryption
rotate_secrets() {
    local old_key=$(cat "$DEV_KEY_FILE")
    local new_key=$(openssl rand -base64 32)
    
    echo "🔄 Rotating all secrets with new encryption..."
    
    for secret_file in "$ENCRYPTED_SECRETS"/*.enc; do
        if [ -f "$secret_file" ]; then
            secret_name=$(basename "$secret_file" .enc)
            
            # Decrypt with old key
            secret_value=$(file-utils decrypt "$secret_file" -k "$old_key")
            
            # Re-encrypt with new key
            echo "$secret_value" | file-utils encrypt - -k "$new_key" -o "$secret_file.new"
            
            # Secure delete old version
            file-utils -s "$secret_file"
            mv "$secret_file.new" "$secret_file"
            
            echo "✅ Rotated: $secret_name"
        fi
    done
    
    # Update key file
    echo "$new_key" > "$DEV_KEY_FILE"
    echo "🔑 Key rotation complete"
}

# Usage examples
init_secrets_vault

# Store common development secrets
store_secret "github_token" "ghp_xxxxxxxxxxxxxxxxxxxx"
store_secret "aws_access_key" "AKIAIOSFODNN7EXAMPLE"
store_secret "database_password" "super_secure_db_password_123"
store_secret "api_key_stripe" "sk_test_xxxxxxxxxxxxxxxxxxxx"

# Use in development scripts
export GITHUB_TOKEN=$(get_secret "github_token")
export AWS_ACCESS_KEY_ID=$(get_secret "aws_access_key")
export DB_PASSWORD=$(get_secret "database_password")

# Example: Deploy script using encrypted secrets
deploy_to_staging() {
    local stripe_key=$(get_secret "api_key_stripe")
    local db_pass=$(get_secret "database_password")
    
    # Deploy with secrets (they never touch disk unencrypted)
    docker run -e STRIPE_KEY="$stripe_key" -e DB_PASSWORD="$db_pass" myapp:latest
}
🚀 CI/CD Pipeline Security
yaml# .github/workflows/secure-deploy.yml
name: Secure Deployment Pipeline

on:
  push:
    branches: [main]

jobs:
  secure-build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Install file-utils
      run: |
        git clone https://github.com/whispr-dev/file-utils.git
        cd file-utils
        cargo build --release
        sudo cp target/release/file-utils /usr/local/bin/
    
    - name: Decrypt build secrets
      env:
        BUILD_KEY: ${{ secrets.BUILD_ENCRYPTION_KEY }}
      run: |
        # Decrypt encrypted environment files
        file-utils decrypt .env.production.enc -k "$BUILD_KEY" -o .env.production
        file-utils decrypt ssl-certs.tar.enc -k "$BUILD_KEY" -o ssl-certs.tar
        tar -xf ssl-certs.tar
    
    - name: Build application
      run: |
        # Build with decrypted secrets
        source .env.production
        npm run build:production
    
    - name: Encrypt build artifacts
      env:
        DEPLOY_KEY: ${{ secrets.DEPLOY_ENCRYPTION_KEY }}
      run: |
        # Encrypt sensitive build outputs
        tar -czf dist.tar.gz dist/
        file-utils encrypt dist.tar.gz -k "$DEPLOY_KEY" -o dist.enc
        
        # Secure delete unencrypted artifacts
        file-utils -s dist.tar.gz
        file-utils -s .env.production
        rm -rf ssl-certs/
    
    - name: Upload encrypted artifacts
      uses: actions/upload-artifact@v3
      with:
        name: encrypted-build
        path: dist.enc
    
    - name: Clean up
      run: |
        # Secure delete any remaining sensitive files
        find . -name "*.key" -o -name "*.pem" -o -name "*.env*" | \
        xargs -I {} file-utils -s {}
🏠 Home Lab & Personal Server Use Cases
🏠 Smart Home Configuration Backup
bash#!/bin/bash
# Backup and encrypt smart home configurations

SMART_HOME_CONFIGS=(
    "/etc/homeassistant/configuration.yaml"
    "/opt/zigbee2mqtt/data/configuration.yaml"
    "/etc/mosquitto/mosquitto.conf"
    "/home/pi/.homebridge/config.json"
    "/opt/nodered/data/flows.json"
)

BACKUP_DIR="/media/usb_backup/smarthome_encrypted"
BACKUP_KEY="smarthome_$(hostname)_$(date +%Y%m%d)"

mkdir -p "$BACKUP_DIR"

echo "🏠 Starting smart home configuration backup..."

# Create encrypted backup of each config
for config_file in "${SMART_HOME_CONFIGS[@]}"; do
    if [ -f "$config_file" ]; then
        config_name=$(basename "$config_file")
        service_name=$(basename "$(dirname "$config_file")")
        
        echo "📦 Backing up: $service_name/$config_name"
        
        # Encrypt with service-specific naming
        file-utils encrypt "$config_file" \
            -k "$BACKUP_KEY" \
            -o "$BACKUP_DIR/${service_name}_${config_name}.enc"
            
        if [ $? -eq 0 ]; then
            echo "✅ Encrypted: $service_name/$config_name"
        else
            echo "❌ Failed to encrypt: $config_file"
        fi
    else
        echo "⚠️  Config not found: $config_file"
    fi
done

# Create restore script
cat > "$BACKUP_DIR/restore.sh" << 'EOF'
#!/bin/bash
# Smart Home Configuration Restore Script

BACKUP_KEY="$1"
if [ -z "$BACKUP_KEY" ]; then
    echo "Usage: $0 <backup_key>"
    exit 1
fi

echo "🔄 Restoring smart home configurations..."

# Stop services before restore
sudo systemctl stop homeassistant
sudo systemctl stop zigbee2mqtt
sudo systemctl stop mosquitto
sudo systemctl stop homebridge
sudo systemctl stop nodered

# Restore each configuration
for enc_file in *.enc; do
    if [ -f "$enc_file" ]; then
        echo "📥 Restoring: $enc_file"
        file-utils decrypt "$enc_file" -k "$BACKUP_KEY" -o "/tmp/$(basename "$enc_file" .enc)"
        
        # Move to appropriate location based on filename
        case "$enc_file" in
            homeassistant_*)
                sudo cp "/tmp/$(basename "$enc_file" .enc)" /etc/homeassistant/
                ;;
            zigbee2mqtt_*)
                sudo cp "/tmp/$(basename "$enc_file" .enc)" /opt/zigbee2mqtt/data/
                ;;
            mosquitto_*)
                sudo cp "/tmp/$(basename "$enc_file" .enc)" /etc/mosquitto/
                ;;
            homebridge_*)
                cp "/tmp/$(basename "$enc_file" .enc)" /home/pi/.homebridge/
                ;;
            nodered_*)
                sudo cp "/tmp/$(basename "$enc_file" .enc)" /opt/nodered/data/
                ;;
        esac
        
        # Clean up temp file
        rm "/tmp/$(basename "$enc_file" .enc)"
    fi
done

# Restart services
sudo systemctl start homeassistant
sudo systemctl start zigbee2mqtt
sudo systemctl start mosquitto
sudo systemctl start homebridge
sudo systemctl start nodered

echo "✅ Smart home restore complete!"
EOF

chmod +x "$BACKUP_DIR/restore.sh"

# Store backup key securely
echo "$BACKUP_KEY" | file-utils encrypt - -k "$(hostname)_master" -o "$BACKUP_DIR/backup_key.enc"

echo "✅ Smart home backup complete!"
echo "📍 Backup location: $BACKUP_DIR"
echo "🔑 Backup key stored in: backup_key.enc"
🖥️ Workstation Cleanup & Privacy
bash#!/bin/bash
# Privacy-focused workstation cleanup script

echo "🧹 Starting privacy-focused system cleanup..."

# Function to secure delete with progress
secure_delete_with_progress() {
    local target="$1"
    local description="$2"
    
    if [ -e "$target" ]; then
        echo "🗑️  Securely deleting: $description"
        file-utils -s "$target" 2>/dev/null || {
            echo "⚠️  Could not delete: $target (may not exist or be locked)"
        }
    fi
}

# Browser data cleanup
echo "🌐 Cleaning browser data..."
secure_delete_with_progress "$HOME/.cache/google-chrome" "Chrome cache"
secure_delete_with_progress "$HOME/.cache/mozilla" "Firefox cache"
secure_delete_with_progress "$HOME/.cache/chromium" "Chromium cache"

# Development environment cleanup
echo "💻 Cleaning development artifacts..."
find "$HOME" -name "node_modules" -type d -exec rm -rf {} + 2>/dev/null
find "$HOME" -name ".git/objects/pack" -type d -exec rm -rf {} + 2>/dev/null
secure_delete_with_progress "$HOME/.npm/_logs" "NPM logs"
secure_delete_with_progress "$HOME/.cargo/registry/cache" "Cargo cache"

# System temporary files
echo "🗂️  Cleaning system temporary files..."
sudo find /tmp -type f -mtime +1 -exec file-utils -s {} \; 2>/dev/null
sudo find /var/tmp -type f -mtime +7 -exec file-utils -s {} \; 2>/dev/null

# Log files
echo "📝 Cleaning log files..."
secure_delete_with_progress "/var/log/*.log" "System logs"
secure_delete_with_progress "$HOME/.xsession-errors" "X session errors"

# SSH known hosts (if desired)
read -p "🔐 Clear SSH known hosts? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    secure_delete_with_progress "$HOME/.ssh/known_hosts" "SSH known hosts"
fi

# Bash history (if desired)
read -p "📚 Clear bash history? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    secure_delete_with_progress "$HOME/.bash_history" "Bash history"
    history -c
fi

echo "✅ Privacy cleanup complete!"
🎓 Educational & Research Use Cases
🔬 Research Data Protection
python#!/usr/bin/env python3
"""
Research Data Protection Script
Protects sensitive research data with quantum-enhanced encryption
"""

import os
import subprocess
import hashlib
import json
from datetime import datetime, timedelta
from pathlib import Path

class ResearchDataProtector:
    def __init__(self, research_dir, vault_dir, key_file):
        self.research_dir = Path(research_dir)
        self.vault_dir = Path(vault_dir)
        self.key_file = Path(key_file)
        self.vault_dir.mkdir(exist_ok=True)
        
    def generate_research_key(self, project_name, researcher_id):
        """Generate a unique key for research project"""
        key_material = f"{project_name}_{researcher_id}_{datetime.now().isoformat()}"
        return hashlib.sha256(key_material.encode()).hexdigest()
    
    def encrypt_research_data(self, project_name, researcher_id, sensitivity_level="high"):
        """Encrypt research data with appropriate security level"""
        
        # Use quantum mode for highly sensitive research
        crypto_mode = "quantum" if sensitivity_level == "high" else "aes"
        
        project_key = self.generate_research_key(project_name, researcher_id)
        
        # Create project vault
        project_vault = self.vault_dir / f"{project_name}_{researcher_id}"
        project_vault.mkdir(exist_ok=True)
        
        # Find all research files
        research_files = list(self.research_dir.rglob("*"))
        research_files = [f for f in research_files if f.is_file()]
        
        metadata = {
            "project": project_name,
            "researcher": researcher_id,
            "encryption_date": datetime.now().isoformat(),
            "sensitivity_level": sensitivity_level,
            "crypto_mode": crypto_mode,
            "files_encrypted": []
        }
        
        print(f"🔬 Encrypting {len(research_files)} research files...")
        
        for research_file in research_files:
            try:
                # Create secure filename that doesn't reveal research content
                secure_name = f"research_{hashlib.md5(str(research_file).encode()).hexdigest()}.enc"
                output_path = project_vault / secure_name
                
                # Encrypt using file-utils
                result = subprocess.run([
                    "file-utils", "encrypt", str(research_file),
                    "-m", crypto_mode,
                    "-k", project_key,
                    "-o", str(output_path)
                ], capture_output=True, text=True)
                
                if result.returncode == 0:
                    print(f"✅ Encrypted: {research_file.name}")
                    
                    # Add to metadata
                    metadata["files_encrypted"].append({
                        "original_path": str(research_file),
                        "encrypted_name": secure_name,
                        "file_size": research_file.stat().st_size,
                        "encryption_timestamp": datetime.now().isoformat()
                    })
                    
                    # Secure delete original after verification
                    verify_result = subprocess.run([
                        "file-utils", "decrypt", str(output_path),
                        "-m", crypto_mode,
                        "-k", project_key,
                        "-o", f"/tmp/verify_{research_file.name}"
                    ], capture_output=True, text=True)
                    
                    if verify_result.returncode == 0:
                        # Verify file integrity
                        original_hash = hashlib.sha256(research_file.read_bytes()).hexdigest()
                        verify_hash = hashlib.sha256(Path(f"/tmp/verify_{research_file.name}").read_bytes()).hexdigest()
                        
                        if original_hash == verify_hash:
                            subprocess.run(["file-utils", "-s", str(research_file)])
                            os.remove(f"/tmp/verify_{research_file.name}")
                            print(f"🗑️  Securely deleted: {research_file.name}")
                        else:
                            print(f"❌ Verification failed: {research_file.name}")
                    
                else:
                    print(f"❌ Encryption failed: {research_file.name}")
                    
            except Exception as e:
                print(f"❌ Error processing {research_file.name}: {e}")
        
        # Save encrypted metadata
        metadata_file = project_vault / "research_metadata.json"
        with open(metadata_file, 'w') as f:
            json.dump(metadata, f, indent=2)
        
        # Encrypt the metadata itself
        subprocess.run([
            "file-utils", "encrypt", str(metadata_file),
            "-m", crypto_mode,
            "-k", project_key,
            "-o", str(project_vault / "research_metadata.json.enc")
        ])
        subprocess.run(["file-utils", "-s", str(metadata_file)])
        
        # Store project key securely
        key_storage = self.key_file.parent / f"{project_name}_{researcher_id}.key"
        with open(key_storage, 'w') as f:
            f.write(project_key)
        os.chmod(key_storage, 0o600)
        
        print(f"🔐 Research data protection complete!")
        print(f"📁 Encrypted data: {project_vault}")
        print(f"🔑 Key stored: {key_storage}")
        
        return project_key

# Usage example
if __name__ == "__main__":
    protector = ResearchDataProtector(
        research_dir="/home/researcher/active_projects/covid_variant_analysis",
        vault_dir="/secure/research_vault",
        key_file="/secure/keys/research_keys.txt"
    )
    
    # Protect highly sensitive medical research data
    project_key = protector.encrypt_research_data(
        project_name="covid_variant_genomics",
        researcher_id="dr_smith_2024",
        sensitivity_level="high"  # Uses quantum encryption
    )
    
    print(f"✅ Research protection key: {project_key[:16]}...")

🎯 Summary: These real-world examples demonstrate how file-utils can be adapted for various scenarios, from personal privacy to enterprise security to research data protection. The tool's flexibility with both AES and quantum encryption modes, combined with its secure deletion capabilities, makes it suitable for a wide range of security-conscious applications.
💡 Pro Tips:

Always test encryption/decryption workflows before deploying to production
Use AES mode for compliance-critical applications
Implement proper key management practices
Regular security audits and key rotation
Document your encryption procedures for team members

