#!/bin/bash

# Sign Windows Executable Script
# Usage: ./sign-windows-executable.sh <executable_path>
# Example: ./sign-windows-executable.sh path/to/seamly2d.exe

set -e  # Exit on any error

# Colors for logging
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $(date '+%Y-%m-%d %H:%M:%S') - $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $(date '+%Y-%m-%d %H:%M:%S') - $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $(date '+%Y-%m-%d %H:%M:%S') - $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $(date '+%Y-%m-%d %H:%M:%S') - $1"
}

# Function to display usage
usage() {
    echo "Usage: $0 <executable_path>"
    echo "Example: $0 path/to/seamly2d.exe"
    echo ""
    echo "Required environment variables:"
    echo "  SEAMLY_SIGNING_PROJECT_ID - Google Cloud project ID"
    echo "  SEAMLY_SIGNING_LOCATION - KMS location (e.g., us-central1)"
    echo "  SEAMLY_SIGNING_KEYRING_NAME - KMS keyring name"
    echo "  SEAMLY_SIGNING_KEY_NAME - KMS key name"
    exit 1
}

# Function to validate environment variables
validate_environment() {
    log_info "Validating environment variables..."
    
    local required_vars=(
        "SEAMLY_SIGNING_PROJECT_ID"
        "SEAMLY_SIGNING_LOCATION"
        "SEAMLY_SIGNING_KEYRING_NAME"
        "SEAMLY_SIGNING_KEY_NAME"
    )
    
    for var in "${required_vars[@]}"; do
        if [[ -z "${!var}" ]]; then
            log_error "Required environment variable $var is not set"
            exit 1
        fi
        log_info "✓ $var is set"
    done
}

# Function to validate files
validate_files() {
    log_info "Validating required files..."
    
    # Check if executable exists
    if [[ ! -f "$1" ]]; then
        log_error "Executable file not found: $1"
        exit 1
    fi
    log_info "✓ Executable file exists: $1"
    
    # Check if jsign.jar exists
    if [[ ! -f "jsign.jar" ]]; then
        log_error "jsign.jar not found in current directory"
        exit 1
    fi
    log_info "✓ jsign.jar exists"
    
    # Check if codesign-chain.pem exists
    if [[ ! -f "codesign-chain.pem" ]]; then
        log_error "codesign-chain.pem not found in current directory"
        exit 1
    fi
    log_info "✓ codesign-chain.pem exists"
}

# Function to validate jar file hash
validate_jar_hash() {
    log_info "Validating jsign.jar hash..."
    
    local expected_hash="290377fc4f593256200b3ea4061b7409e8276255f449d4c6de7833faf0850cc1"
    local actual_hash=$(sha256sum jsign.jar | cut -d' ' -f1)
    
    if [[ "$expected_hash" != "$actual_hash" ]]; then
        log_error "jsign.jar hash validation failed!"
        log_error "Expected: $expected_hash"
        log_error "Actual:   $actual_hash"
        exit 1
    fi
    log_success "✓ jsign.jar hash validation passed"
}

# Function to validate pem file
validate_pem_file() {
    log_info "Validating codesign-chain.pem file..."
    
    # Check if it's a valid PEM file
    if ! openssl x509 -in codesign-chain.pem -text -noout >/dev/null 2>&1; then
        log_error "codesign-chain.pem is not a valid PEM certificate file"
        exit 1
    fi
    log_success "✓ codesign-chain.pem is a valid PEM certificate"
    
    # Log PEM file checksum
    local pem_hash=$(sha256sum codesign-chain.pem | cut -d' ' -f1)
    log_info "PEM file checksum: $pem_hash"
}

# Function to check Google Cloud authentication
check_gcloud_auth() {
    log_info "Checking Google Cloud authentication..."
    
    if ! gcloud auth print-access-token >/dev/null 2>&1; then
        log_error "Google Cloud authentication failed. Please run 'gcloud auth activate-service-account' first"
        exit 1
    fi
    log_success "✓ Google Cloud authentication successful"
}

# Function to sign the executable
sign_executable() {
    local executable_path="$1"
    local executable_name=$(basename "$executable_path")
    
    log_info "Starting signing process for: $executable_name"
    log_info "Full path: $executable_path"
    
    # Log file checksum before signing
    local before_hash=$(sha256sum "$executable_path" | cut -d' ' -f1)
    log_info "File checksum before signing: $before_hash"
    
    # Sign the executable
    log_info "Executing signing command..."
    
    if java -jar jsign.jar \
        --storetype GOOGLECLOUD \
        --storepass "$(gcloud auth print-access-token)" \
        --keystore "projects/$SEAMLY_SIGNING_PROJECT_ID/locations/$SEAMLY_SIGNING_LOCATION/keyRings/$SEAMLY_SIGNING_KEYRING_NAME" \
        --alias "$SEAMLY_SIGNING_KEY_NAME" \
        --certfile "codesign-chain.pem" \
        --tsmode RFC3161 \
        --tsaurl http://timestamp.globalsign.com/tsa/r6advanced1 \
        "$executable_path"; then
        
        log_success "✓ Successfully signed: $executable_name"
        
        # Log file checksum after signing
        local after_hash=$(sha256sum "$executable_path" | cut -d' ' -f1)
        log_info "File checksum after signing: $after_hash"
        
        # Verify the file was actually modified (signed)
        if [[ "$before_hash" == "$after_hash" ]]; then
            log_warning "File checksum unchanged after signing - this may indicate an issue"
        else
            log_success "✓ File checksum changed, confirming successful signing"
        fi
        
    else
        log_error "Failed to sign: $executable_name"
        exit 1
    fi
}

# Main execution
main() {
    log_info "=== Windows Executable Signing Script ==="
    log_info "Script version: 2.0"
    log_info "Started at: $(date)"
    
    # Check if executable path is provided
    if [[ $# -eq 0 ]]; then
        log_error "No executable path provided"
        usage
    fi
    
    local executable_path="$1"
    
    # Validate environment variables
    validate_environment
    
    # Validate files
    validate_files "$executable_path"
    
    # Validate jar file hash
    validate_jar_hash
    
    # Validate PEM file
    validate_pem_file
    
    # Check Google Cloud authentication
    check_gcloud_auth
    
    # Log configuration
    log_info "Signing configuration:"
    log_info "  Project ID: $SEAMLY_SIGNING_PROJECT_ID"
    log_info "  Location: $SEAMLY_SIGNING_LOCATION"
    log_info "  Keyring: $SEAMLY_SIGNING_KEYRING_NAME"
    log_info "  Key: $SEAMLY_SIGNING_KEY_NAME"
    log_info "  JAR file: jsign.jar"
    log_info "  PEM file: codesign-chain.pem"
    
    # Sign the executable
    sign_executable "$executable_path"
    
    log_success "=== Signing process completed successfully ==="
    log_info "Finished at: $(date)"
}

# Run main function with all arguments
main "$@"
