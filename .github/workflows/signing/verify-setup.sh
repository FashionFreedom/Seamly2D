#!/bin/bash

# Verification script for code signing setup
# This script verifies that all required files are in place and have correct hashes

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Expected file hashes
EXPECTED_JSIGN_HASH="290377fc4f593256200b3ea4061b7409e8276255f449d4c6de7833faf0850cc1"
EXPECTED_PEM_HASH="7d6acf2c42b9bb10e9ea15c168b6cf5132eb5a4b1da10820e941fce1ac8caadd"

echo "=== Code Signing Setup Verification ==="
echo ""

# Check if we're in the correct directory
if [[ ! -f "sign-windows-executable.sh" ]]; then
    log_error "This script must be run from the .github/workflows/signing/ directory"
    exit 1
fi

log_success "✓ Running from correct directory"

# Check required files
log_info "Checking required files..."

# Check jsign.jar
if [[ -f "jsign.jar" ]]; then
    log_success "✓ jsign.jar exists"
    
    # Check file type
    if file jsign.jar | grep -q "Zip archive"; then
        log_success "✓ jsign.jar is a valid JAR file"
    else
        log_error "✗ jsign.jar is not a valid JAR file"
        exit 1
    fi
    
    # Check hash
    ACTUAL_JSIGN_HASH=$(sha256sum jsign.jar | cut -d' ' -f1)
    if [[ "$ACTUAL_JSIGN_HASH" == "$EXPECTED_JSIGN_HASH" ]]; then
        log_success "✓ jsign.jar hash matches expected value"
    else
        log_error "✗ jsign.jar hash mismatch!"
        log_error "Expected: $EXPECTED_JSIGN_HASH"
        log_error "Actual:   $ACTUAL_JSIGN_HASH"
        exit 1
    fi
else
    log_error "✗ jsign.jar not found"
    exit 1
fi

# Check codesign-chain.pem
if [[ -f "codesign-chain.pem" ]]; then
    log_success "✓ codesign-chain.pem exists"
    
    # Check file type
    if file codesign-chain.pem | grep -q "PEM certificate"; then
        log_success "✓ codesign-chain.pem is a valid PEM certificate"
    else
        log_error "✗ codesign-chain.pem is not a valid PEM certificate"
        exit 1
    fi
    
    # Check hash
    ACTUAL_PEM_HASH=$(sha256sum codesign-chain.pem | cut -d' ' -f1)
    if [[ "$ACTUAL_PEM_HASH" == "$EXPECTED_PEM_HASH" ]]; then
        log_success "✓ codesign-chain.pem hash matches expected value"
    else
        log_error "✗ codesign-chain.pem hash mismatch!"
        log_error "Expected: $EXPECTED_PEM_HASH"
        log_error "Actual:   $ACTUAL_PEM_HASH"
        exit 1
    fi
else
    log_error "✗ codesign-chain.pem not found"
    exit 1
fi

# Check signing script
if [[ -f "sign-windows-executable.sh" ]]; then
    log_success "✓ sign-windows-executable.sh exists"
    
    # Check if script is executable
    if [[ -x "sign-windows-executable.sh" ]]; then
        log_success "✓ sign-windows-executable.sh is executable"
    else
        log_warning "! sign-windows-executable.sh is not executable (will be fixed in workflow)"
    fi
else
    log_error "✗ sign-windows-executable.sh not found"
    exit 1
fi

# Check documentation files
log_info "Checking documentation files..."
DOC_FILES=("SECRETS_SETUP.md" "SECRETS_QUICK_REFERENCE.md")
for doc in "${DOC_FILES[@]}"; do
    if [[ -f "$doc" ]]; then
        log_success "✓ $doc exists"
    else
        log_warning "! $doc not found (optional)"
    fi
done

echo ""
echo "=== Verification Summary ==="
log_success "✓ All required files are present and valid"
log_success "✓ File hashes match expected values"
log_success "✓ File types are correct"
echo ""
log_info "Next steps:"
echo "  1. Configure GitHub secrets (see SECRETS_SETUP.md)"
echo "  2. Create SeamlySigners team"
echo "  3. Set up GitHub environment"
echo "  4. Test the workflow"
echo ""
log_success "Code signing setup is ready!" 