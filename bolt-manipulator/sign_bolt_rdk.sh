#!/bin/sh
#
# sign_bolt_rdk.sh - Sign a Bolt/RALF package using OpenSSL (RDK embedded version)
# No jq dependency - uses only basic shell tools
# Outputs .bolt file (aligned ZIP built in-shell; does not require the 'zip' tool)
#
# This shell script was created to evaluate and enable signing Bolt packages
# directly on an RDK reference device.
#
# It is intended strictly for development and testing, and must never be
# integrated into product builds.
#
# This script can be used as an alternative to ralfpack for package signing.
# Developer: vaisakh_anand@comcast.com
# Maintained in: https://github.com/rdkcentral/bolt-engineering-certificates
# Related project: https://github.com/rdkcentral/ralfpack
#                  https://github.com/rdkcentral/bolt-tools
#
# Usage: sh sign_bolt_rdk.sh [private_key.pem] [package.bolt] [--cert certificate.pem] [--chain chain.pem] [--batch]
#

set -e

# Default certificate repository (RDK engineering self-signed certs — development use only)
BOLT_CERTS_REPO_BASE_URL="https://raw.githubusercontent.com/rdkcentral/bolt-engineering-certificates/develop/certs"
BOLT_CERTS_DEFAULT_KEY="com.rdkcentral.ralf-private.key"
BOLT_CERTS_DEFAULT_CERT="com.rdkcentral.ralf-public.crt"

# BOLT_PACKAGE_URLS - semicolon-separated list of package URLs (set via environment)
# Example: export BOLT_PACKAGE_URLS="https://example.com/base.bolt;https://example.com/wpe.bolt"

# Configuration
TEMP_DIR="${TEMP_DIR:-/tmp/signing_temp_$$}"
OUTPUT_DIR="${TEMP_DIR}/signed_packages"
EXTRACT_DIR="${TEMP_DIR}/extracted"

# Print usage
usage() {
    echo "Usage: sh $0 [private_key.pem] [package.bolt] [OPTIONS]"
    echo ""
    echo "Arguments:"
    echo "  private_key.pem    Path to the RSA/ECDSA private key in PEM format (optional;"
    echo "                     if omitted the RDK engineering development key is fetched)"
    echo "  package.bolt       Path to the Bolt package (optional when using --batch)"
    echo ""
    echo "Options:"
    echo "  --cert FILE        Path to the signing certificate in PEM format"
    echo "  --chain FILE       Path to the certificate chain in PEM format"
    echo "  --passphrase PASS  Passphrase for encrypted private key"
    echo "  --identity ID      Package identity (default: extracted from package)"
    echo "  --batch            Download and sign packages from BOLT_PACKAGE_URLS"
    echo "  --install-path DIR Copy signed packages to this directory"
    echo ""
    echo "Default Certificates:"
    echo "  When no private key or --cert is provided, the RDK engineering development"
    echo "  certificates are automatically fetched from:"
    echo "    ${BOLT_CERTS_REPO_BASE_URL}"
    echo "  WARNING: These are self-signed development certificates."
    echo "           DO NOT use them in production environments."
    echo ""
    echo "Batch Mode:"
    echo "  When using --batch, packages are downloaded from URLs in the"
    echo "  BOLT_PACKAGE_URLS environment variable (semicolon-separated)."
    echo "  Example: export BOLT_PACKAGE_URLS='url1;url2;url3'"
    echo ""
    echo "Output:"
    echo "  Signed package(s): ${OUTPUT_DIR}/<name>.bolt"
    exit 1
}

log_info() {
    echo "[INFO] $1"
}

log_error() {
    echo "[ERROR] $1"
    exit 1
}

# Simple JSON value extractor using sed/grep (handles simple cases)
# Usage: json_get_value "json_string" "key"
json_get_value() {
    echo "$1" | sed 's/,/\n/g' | sed 's/[{}]//g' | grep "\"$2\"" | sed 's/.*"'$2'"[[:space:]]*:[[:space:]]*"\{0,1\}\([^",}]*\)"\{0,1\}.*/\1/' | head -n 1
}

# Extract a field from a JSON file
# Usage: json_file_get_value "file" "key"
json_file_get_value() {
    cat "$1" | tr -d '\n' | sed 's/,/\n/g' | sed 's/[{}]//g' | grep "\"$2\"" | sed 's/.*"'$2'"[[:space:]]*:[[:space:]]*"\{0,1\}\([^",}]*\)"\{0,1\}.*/\1/' | head -n 1
}

# Check dependencies
check_dependencies() {
    for cmd in openssl tar sha256sum base64 sed grep awk gzip od find dd wc cut tr tail head; do
        if ! command -v $cmd >/dev/null 2>&1; then
            log_error "Required command '$cmd' not found."
        fi
    done
    
    # Check for extraction tool (unzip or tar with gzip support)
    HAS_UNZIP=0
    if command -v unzip >/dev/null 2>&1; then
        HAS_UNZIP=1
    fi
    
    # Check for download tool (wget or curl)
    DOWNLOAD_CMD=""
    if command -v wget >/dev/null 2>&1; then
        DOWNLOAD_CMD="wget"
    elif command -v curl >/dev/null 2>&1; then
        DOWNLOAD_CMD="curl"
    fi
}

# Download a file from URL
# Usage: download_file <url> <output_path>
download_file() {
    _url="$1"
    _output="$2"
    
    log_info "Downloading: $_url"
    
    case "$DOWNLOAD_CMD" in
        wget)
            wget -q -O "$_output" "$_url" || wget -O "$_output" "$_url"
            ;;
        curl)
            curl -sL -o "$_output" "$_url" || curl -L -o "$_output" "$_url"
            ;;
        *)
            log_error "No download tool available (need wget or curl)"
            ;;
    esac
    
    if [ ! -f "$_output" ] || [ ! -s "$_output" ]; then
        log_error "Failed to download: $_url"
    fi
    
    log_info "Downloaded to: $_output"
}

# Fetch default certificates from bolt-engineering-certificates repo when not provided.
# Sets PRIVATE_KEY and/or CERTIFICATE if they are still empty.
fetch_default_certificates() {
    _fetched_any=0

    if [ -z "$PRIVATE_KEY" ]; then
        if [ -z "$DOWNLOAD_CMD" ]; then
            log_error "No download tool (wget or curl) available to fetch the default private key"
        fi
        log_info "No private key provided — fetching RDK engineering development key from repo..."
        log_info "  WARNING: This is a development-only key. DO NOT use in production."
        mkdir -p "${TEMP_DIR}"
        _key_dest="${TEMP_DIR}/${BOLT_CERTS_DEFAULT_KEY}"
        download_file "${BOLT_CERTS_REPO_BASE_URL}/${BOLT_CERTS_DEFAULT_KEY}" "$_key_dest"
        PRIVATE_KEY="$_key_dest"
        _fetched_any=1
    fi

    if [ -z "$CERTIFICATE" ]; then
        if [ -z "$DOWNLOAD_CMD" ]; then
            log_error "No download tool (wget or curl) available to fetch the default certificate"
        fi
        log_info "No certificate provided — fetching RDK engineering development certificate from repo..."
        log_info "  WARNING: This is a self-signed development certificate. DO NOT use in production."
        mkdir -p "${TEMP_DIR}"
        _cert_dest="${TEMP_DIR}/${BOLT_CERTS_DEFAULT_CERT}"
        download_file "${BOLT_CERTS_REPO_BASE_URL}/${BOLT_CERTS_DEFAULT_CERT}" "$_cert_dest"
        CERTIFICATE="$_cert_dest"
        _fetched_any=1
    fi

    if [ "$_fetched_any" = "1" ]; then
        log_info "Default certificates source: ${BOLT_CERTS_REPO_BASE_URL}"
    fi
}

# Parse arguments
PRIVATE_KEY=""
BOLT_PACKAGE=""
CERTIFICATE=""
CERT_CHAIN=""
PASSPHRASE=""
IDENTITY=""
BATCH_MODE=0
INSTALL_PATH=""

parse_args() {
    # First positional arg: detect whether it is a private key or a bolt package.
    # If it ends in a known package extension it is treated as a package and no
    # private key was supplied (defaults will be fetched later).
    if [ $# -ge 1 ] && [ "$(echo "$1" | cut -c1)" != "-" ]; then
        case "$1" in
            *.bolt|*.zip|*.tar.gz|*.tgz|*.tar)
                # First arg is the package; no explicit private key provided
                BOLT_PACKAGE="$1"
                shift 1
                ;;
            *)
                # First arg is the private key
                PRIVATE_KEY="$1"
                shift 1
                # Check if next positional arg is the package
                if [ $# -gt 0 ] && [ "$(echo "$1" | cut -c1)" != "-" ]; then
                    BOLT_PACKAGE="$1"
                    shift 1
                fi
                ;;
        esac
    fi

    while [ $# -gt 0 ]; do
        case "$1" in
            --cert)
                [ -n "$2" ] && [ "$(echo "$2" | cut -c1)" != "-" ] || log_error "--cert requires a FILE argument"
                CERTIFICATE="$2"
                shift 2
                ;;
            --chain)
                [ -n "$2" ] && [ "$(echo "$2" | cut -c1)" != "-" ] || log_error "--chain requires a FILE argument"
                CERT_CHAIN="$2"
                shift 2
                ;;
            --passphrase)
                PASSPHRASE="$2"
                shift 2
                ;;
            --identity)
                [ -n "$2" ] || log_error "--identity requires an ID argument"
                IDENTITY="$2"
                shift 2
                ;;
            -h|--help)
                usage
                ;;
            --batch)
                BATCH_MODE=1
                shift 1
                ;;
            --install-path)
                [ -n "$2" ] && [ "$(echo "$2" | cut -c1)" != "-" ] || log_error "--install-path requires a DIR argument"
                INSTALL_PATH="$2"
                shift 2
                ;;
            *)
                log_error "Unknown option: $1"
                ;;
        esac
    done

    # Validate files
    if [ -n "$PRIVATE_KEY" ] && [ ! -f "$PRIVATE_KEY" ]; then
        log_error "Private key file not found: $PRIVATE_KEY"
    fi

    # In batch mode, package is optional (will be downloaded)
    if [ "$BATCH_MODE" = "0" ] && [ -z "$BOLT_PACKAGE" ]; then
        log_error "Package path required (or use --batch mode)"
    fi
    
    if [ -n "$BOLT_PACKAGE" ] && [ ! -f "$BOLT_PACKAGE" ]; then
        log_error "Bolt package not found: $BOLT_PACKAGE"
    fi

    if [ -n "$CERTIFICATE" ] && [ ! -f "$CERTIFICATE" ]; then
        log_error "Certificate file not found: $CERTIFICATE"
    fi

    if [ -n "$CERT_CHAIN" ] && [ ! -f "$CERT_CHAIN" ]; then
        log_error "Certificate chain file not found: $CERT_CHAIN"
    fi
}

# Setup directories
setup_directories() {
    log_info "Setting up directories..."
    rm -rf "${EXTRACT_DIR}"
    mkdir -p "${TEMP_DIR}"
    mkdir -p "${OUTPUT_DIR}"
    mkdir -p "${EXTRACT_DIR}"
}

# Extract the bolt package (handles both zip and tar formats)
extract_package() {
    log_info "Extracting bolt package..."
    
    # Detect package type by extension (most reliable on embedded systems)
    PACKAGE_TYPE=""
    
    # Use extension-based detection
    if [ -z "$PACKAGE_TYPE" ]; then
        case "$BOLT_PACKAGE" in
            *.zip|*.bolt)
                PACKAGE_TYPE="zip"
                ;;
            *.tar.gz|*.tgz)
                PACKAGE_TYPE="targz"
                ;;
            *.tar)
                PACKAGE_TYPE="tar"
                ;;
            *)
                PACKAGE_TYPE="zip"
                ;;
        esac
    fi
    
    log_info "Detected package type: ${PACKAGE_TYPE}"
    
    case "$PACKAGE_TYPE" in
        zip)
            if [ "$HAS_UNZIP" = "1" ]; then
                unzip -q "$BOLT_PACKAGE" -d "${EXTRACT_DIR}" 2>/dev/null || \
                    unzip "$BOLT_PACKAGE" -d "${EXTRACT_DIR}"
            else
                log_error "Package is ZIP format but 'unzip' not available. Convert to tar.gz first: unzip pkg.bolt -d tmp && tar -czf pkg.tar.gz -C tmp ."
            fi
            ;;
        targz)
            tar -xzf "$BOLT_PACKAGE" -C "${EXTRACT_DIR}"
            ;;
        tar)
            tar -xf "$BOLT_PACKAGE" -C "${EXTRACT_DIR}"
            ;;
    esac
    
    if [ ! -f "${EXTRACT_DIR}/index.json" ]; then
        log_error "Invalid bolt package: missing index.json"
    fi
}

# Get the content manifest digest from index.json
get_content_manifest_digest() {
    log_info "Finding content manifest digest..."
    
    # Extract all manifest digests from index.json
    # Format: "digest": "sha256:xxxx"
    DIGESTS=$(grep -o '"digest"[[:space:]]*:[[:space:]]*"sha256:[a-f0-9]*"' "${EXTRACT_DIR}/index.json" | \
              sed 's/.*"sha256:\([a-f0-9]*\)".*/\1/')
    
    for digest_hash in $DIGESTS; do
        manifest_file="${EXTRACT_DIR}/blobs/sha256/${digest_hash}"
        
        if [ -f "$manifest_file" ]; then
            # Check if this manifest has the package config media type
            if grep -q "application/vnd.rdk.package.config.v1+json" "$manifest_file" 2>/dev/null; then
                CONTENT_MANIFEST_DIGEST="sha256:${digest_hash}"
                CONTENT_MANIFEST_HASH="${digest_hash}"
                log_info "Found content manifest: ${CONTENT_MANIFEST_DIGEST}"
                return 0
            fi
        fi
    done
    
    log_error "Could not find content manifest in package"
}

# Get package identity from config
get_package_identity() {
    if [ -n "$IDENTITY" ]; then
        PACKAGE_IDENTITY="$IDENTITY"
        return
    fi
    
    log_info "Extracting package identity..."
    
    manifest_file="${EXTRACT_DIR}/blobs/sha256/${CONTENT_MANIFEST_HASH}"
    
    # Get config digest from manifest
    config_digest=$(grep -o '"config"[^}]*"digest"[[:space:]]*:[[:space:]]*"sha256:[a-f0-9]*"' "$manifest_file" | \
                    grep -o 'sha256:[a-f0-9]*' | cut -d: -f2)
    
    if [ -n "$config_digest" ]; then
        config_file="${EXTRACT_DIR}/blobs/sha256/${config_digest}"
        if [ -f "$config_file" ]; then
            # Extract "id" field from config
            PACKAGE_IDENTITY=$(grep -o '"id"[[:space:]]*:[[:space:]]*"[^"]*"' "$config_file" | \
                              sed 's/.*"id"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/' | head -n 1)
        fi
    fi
    
    if [ -z "$PACKAGE_IDENTITY" ]; then
        PACKAGE_IDENTITY="unknown"
    fi
    
    log_info "Package identity: ${PACKAGE_IDENTITY}"
}

# Create the signed blob JSON
create_signed_blob() {
    log_info "Creating signed blob JSON..."
    
    timestamp=$(date +%s)
    
    cat > "${TEMP_DIR}/signed_blob.json" << EOF
{"critical":{"identity":{"docker-reference":"${PACKAGE_IDENTITY}"},"image":{"docker-manifest-digest":"${CONTENT_MANIFEST_DIGEST}"},"type":"cosign container image signature"},"optional":{"creator":"sign_bolt_rdk.sh","timestamp":${timestamp}}}
EOF
    
    SIGNED_BLOB_FILE="${TEMP_DIR}/signed_blob.json"
}

# Sign the blob with OpenSSL
sign_blob() {
    log_info "Signing blob with private key..."
    
    if [ -n "$PASSPHRASE" ]; then
        openssl dgst -sha256 -sign "${PRIVATE_KEY}" -passin "pass:${PASSPHRASE}" \
            -out "${TEMP_DIR}/signature.bin" "${SIGNED_BLOB_FILE}" 2>/dev/null || \
            log_error "Failed to sign. Check your private key and passphrase."
    else
        openssl dgst -sha256 -sign "${PRIVATE_KEY}" \
            -out "${TEMP_DIR}/signature.bin" "${SIGNED_BLOB_FILE}" 2>/dev/null || \
            log_error "Failed to sign. Check your private key."
    fi
    
    # Base64 encode - try different options for compatibility
    if base64 -w0 "${TEMP_DIR}/signature.bin" > "${TEMP_DIR}/signature.b64" 2>/dev/null; then
        SIGNATURE_B64=$(cat "${TEMP_DIR}/signature.b64")
    else
        SIGNATURE_B64=$(base64 "${TEMP_DIR}/signature.bin" | tr -d '\n')
    fi
    
    log_info "Signature created successfully"
}

# Prepare certificate strings for JSON (escape newlines)
prepare_certificates() {
    CERT_PEM=""
    CHAIN_PEM=""
    
    if [ -n "$CERTIFICATE" ]; then
        log_info "Reading signing certificate..."
        CERT_PEM=$(cat "$CERTIFICATE" | awk '{printf "%s\\n", $0}' | sed 's/\\n$//')
    fi
    
    if [ -n "$CERT_CHAIN" ]; then
        log_info "Reading certificate chain..."
        CHAIN_PEM=$(cat "$CERT_CHAIN" | awk '{printf "%s\\n", $0}' | sed 's/\\n$//')
    fi
}

# Create signature manifest and blobs
create_signature_manifest() {
    log_info "Creating signature manifest..."
    
    # Calculate signed blob digest and size
    blob_digest=$(sha256sum "${SIGNED_BLOB_FILE}" | cut -d' ' -f1)
    blob_size=$(wc -c < "${SIGNED_BLOB_FILE}" | tr -d ' ')
    
    # Copy signed blob to blobs directory
    cp "${SIGNED_BLOB_FILE}" "${EXTRACT_DIR}/blobs/sha256/${blob_digest}"
    
    # Create the signature config blob
    cat > "${TEMP_DIR}/sig_config.json" << EOF
{"architecture":"","created":"0001-01-01T00:00:00Z","history":[{"created":"0001-01-01T00:00:00Z"}],"os":"","rootfs":{"type":"layers","diff_ids":["sha256:${blob_digest}"]},"config":{}}
EOF
    
    config_digest=$(sha256sum "${TEMP_DIR}/sig_config.json" | cut -d' ' -f1)
    config_size=$(wc -c < "${TEMP_DIR}/sig_config.json" | tr -d ' ')
    
    cp "${TEMP_DIR}/sig_config.json" "${EXTRACT_DIR}/blobs/sha256/${config_digest}"
    
    # Build annotations - start with signature (always present)
    ANNOTATIONS="\"dev.cosignproject.cosign/signature\": \"${SIGNATURE_B64}\""
    
    if [ -n "$CERT_PEM" ]; then
        ANNOTATIONS="${ANNOTATIONS},
        \"dev.sigstore.cosign/certificate\": \"${CERT_PEM}\""
    fi
    
    if [ -n "$CHAIN_PEM" ]; then
        ANNOTATIONS="${ANNOTATIONS},
        \"dev.sigstore.cosign/chain\": \"${CHAIN_PEM}\""
    fi
    
    # Create signature manifest
    cat > "${TEMP_DIR}/sig_manifest.json" << EOF
{
  "schemaVersion": 2,
  "mediaType": "application/vnd.oci.image.manifest.v1+json",
  "config": {
    "mediaType": "application/vnd.oci.image.config.v1+json",
    "digest": "sha256:${config_digest}",
    "size": ${config_size}
  },
  "layers": [
    {
      "mediaType": "application/vnd.dev.cosign.simplesigning.v1+json",
      "digest": "sha256:${blob_digest}",
      "size": ${blob_size},
      "annotations": {
        ${ANNOTATIONS}
      }
    }
  ]
}
EOF
    
    SIG_MANIFEST_DIGEST=$(sha256sum "${TEMP_DIR}/sig_manifest.json" | cut -d' ' -f1)
    SIG_MANIFEST_SIZE=$(wc -c < "${TEMP_DIR}/sig_manifest.json" | tr -d ' ')
    
    cp "${TEMP_DIR}/sig_manifest.json" "${EXTRACT_DIR}/blobs/sha256/${SIG_MANIFEST_DIGEST}"
    
    log_info "Signature manifest: sha256:${SIG_MANIFEST_DIGEST}"
}

# Update index.json with signature manifest (no jq)
update_index() {
    log_info "Updating index.json..."
    
    ref_name="sha256-${CONTENT_MANIFEST_HASH}.sig"
    
    INDEX_FILE="${EXTRACT_DIR}/index.json"
    
    # Extract mediaType from original index.json (if present)
    INDEX_MEDIA_TYPE=$(grep -o '"mediaType"[[:space:]]*:[[:space:]]*"[^"]*"' "$INDEX_FILE" | head -n 1 | sed 's/.*:.*"\([^"]*\)"/\1/')
    if [ -z "$INDEX_MEDIA_TYPE" ]; then
        INDEX_MEDIA_TYPE="application/vnd.oci.image.index.v1+json"
    fi
    
    # Extract schemaVersion
    SCHEMA_VER=$(grep -o '"schemaVersion"[[:space:]]*:[[:space:]]*[0-9]*' "$INDEX_FILE" | grep -o '[0-9]*' | head -n 1)
    if [ -z "$SCHEMA_VER" ]; then
        SCHEMA_VER="2"
    fi
    
    # Extract the content manifest's ref.name annotation (not the .sig one)
    # This is the package identity annotation we need to preserve
    CONTENT_REF_NAME=$(grep '"org.opencontainers.image.ref.name"' "$INDEX_FILE" | grep -v '\.sig"' | grep -o '"org.opencontainers.image.ref.name"[[:space:]]*:[[:space:]]*"[^"]*"' | sed 's/.*:.*"\([^"]*\)"/\1/' | head -n 1)
    
    # Fallback to package identity if not found
    if [ -z "$CONTENT_REF_NAME" ]; then
        CONTENT_REF_NAME="$PACKAGE_IDENTITY"
    fi
    
    # Get correct size of content manifest
    CONTENT_MANIFEST_FILE="${EXTRACT_DIR}/blobs/sha256/${CONTENT_MANIFEST_HASH}"
    CONTENT_SIZE=$(wc -c < "$CONTENT_MANIFEST_FILE" | tr -d ' ')
    
    # Build the content manifest entry - preserve ref.name annotation if present
    CONTENT_MANIFEST_ENTRY="{
      \"mediaType\": \"application/vnd.oci.image.manifest.v1+json\",
      \"digest\": \"sha256:${CONTENT_MANIFEST_HASH}\",
      \"size\": ${CONTENT_SIZE},
      \"annotations\": {
        \"org.opencontainers.image.ref.name\": \"${CONTENT_REF_NAME}\"
      }
    }"
    
    # Build the signature manifest entry
    SIG_MANIFEST_ENTRY="{
      \"mediaType\": \"application/vnd.oci.image.manifest.v1+json\",
      \"digest\": \"sha256:${SIG_MANIFEST_DIGEST}\",
      \"size\": ${SIG_MANIFEST_SIZE},
      \"annotations\": {
        \"org.opencontainers.image.ref.name\": \"${ref_name}\"
      }
    }"
    
    # Write new index.json with proper structure
    cat > "${TEMP_DIR}/new_index.json" << EOF
{
  "schemaVersion": ${SCHEMA_VER},
  "mediaType": "${INDEX_MEDIA_TYPE}",
  "manifests": [
    ${CONTENT_MANIFEST_ENTRY},
    ${SIG_MANIFEST_ENTRY}
  ]
}
EOF
    
    # Replace original index.json
    cp "${TEMP_DIR}/new_index.json" "$INDEX_FILE"
    rm -f "${TEMP_DIR}/new_index.json"
    
    log_info "index.json updated"
}

# Repackage the bolt file
repackage() {
    log_info "Repackaging signed bolt package..."
    
    original_name=$(basename "$BOLT_PACKAGE")
    # Get base name without extension
    case "$original_name" in
        *.bolt)
            base_name=$(echo "$original_name" | sed 's/\.bolt$//')
            ;;
        *.zip)
            base_name=$(echo "$original_name" | sed 's/\.zip$//')
            ;;
        *.tar.gz)
            base_name=$(echo "$original_name" | sed 's/\.tar\.gz$//')
            ;;
        *.tgz)
            base_name=$(echo "$original_name" | sed 's/\.tgz$//')
            ;;
        *.tar)
            base_name=$(echo "$original_name" | sed 's/\.tar$//')
            ;;
        *)
            base_name="$original_name"
            ;;
    esac
    
    output_name="${base_name}.bolt"
    output_path="${OUTPUT_DIR}/${output_name}"
    rm -f "$output_path"
    
    _old_dir=$(pwd)
    cd "${EXTRACT_DIR}"
    
    # Pure shell ZIP creation with alignment support
    log_info "Creating aligned ZIP package using shell..."
    
    # Helper: write 16-bit little-endian integer to file
    write_u16() {
        _val=$1
        _fd=$2
        printf "\\$(printf '%03o' $((_val & 255)))\\$(printf '%03o' $(((_val >> 8) & 255)))" >> "$_fd"
    }
    
    # Helper: write 32-bit little-endian integer to file
    write_u32() {
        _val=$1
        _fd=$2
        printf "\\$(printf '%03o' $((_val & 255)))\\$(printf '%03o' $(((_val >> 8) & 255)))\\$(printf '%03o' $(((_val >> 16) & 255)))\\$(printf '%03o' $(((_val >> 24) & 255)))" >> "$_fd"
    }
    
    # Helper: write N null bytes
    write_nulls() {
        _count=$1
        _fd=$2
        dd if=/dev/zero bs=1 count="$_count" >> "$_fd" 2>/dev/null
    }
    
    # CRC32 calculation using gzip
    # gzip stores CRC32 in last 8 bytes (4 bytes CRC + 4 bytes size)
    calc_crc32() {
        _file=$1
        # Compress with gzip, extract CRC32 from trailer
        # BusyBox od uses -x for hex (outputs 16-bit words, byte-swapped on LE)
        # Output format: "0000000 xxyy aabb" where xxyy is bytes 1,0 and aabb is bytes 3,2
        _hex=$(gzip -c "$_file" | tail -c 8 | dd bs=1 count=4 2>/dev/null | od -x | head -n 1 | awk '{print $2 $3}')
        # _hex is now: b1b0b3b2 (byte-swapped pairs)
        # We need to rearrange to get proper little-endian CRC32
        # CRC32 in gzip trailer is already little-endian, so we need b3b2b1b0 for the decimal value
        _w0=$(echo "$_hex" | cut -c1-4)  # b1b0
        _w1=$(echo "$_hex" | cut -c5-8)  # b3b2
        # Swap bytes within each word back, then combine as big-endian for printf
        _b0=$(echo "$_w0" | cut -c3-4)
        _b1=$(echo "$_w0" | cut -c1-2)
        _b2=$(echo "$_w1" | cut -c3-4)
        _b3=$(echo "$_w1" | cut -c1-2)
        # Handle empty values
        [ -z "$_b0" ] && _b0="00"
        [ -z "$_b1" ] && _b1="00"
        [ -z "$_b2" ] && _b2="00"
        [ -z "$_b3" ] && _b3="00"
        printf "%d" "0x${_b3}${_b2}${_b1}${_b0}"
    }
    
    # Collect files and sort by size (small first, large last for alignment)
    _files_list="${TEMP_DIR}/zip_files.txt"
    find . -type f | while read _f; do
        _sz=$(wc -c < "$_f" | tr -d ' ')
        echo "${_sz} ${_f}"
    done | sort -n | cut -d' ' -f2- > "$_files_list"
    
    # Initialize output file
    rm -f "$output_path"
    touch "$output_path"
    
    # Track entries for central directory
    _entries_file="${TEMP_DIR}/zip_entries.txt"
    rm -f "$_entries_file"
    touch "$_entries_file"
    
    ALIGNMENT=4096
    
    # Write local file headers and data
    while IFS= read -r _filepath; do
        # Remove leading ./
        _arcname=$(echo "$_filepath" | sed 's|^\./||')
        _namelen=$(printf '%s' "$_arcname" | wc -c | tr -d ' ')
        _filesize=$(wc -c < "$_filepath" | tr -d ' ')
        _crc=$(calc_crc32 "$_filepath")
        
        # Get current offset
        _offset=$(wc -c < "$output_path" | tr -d ' ')
        
        # Calculate alignment padding for large files (erofs blobs)
        _padding=0
        if [ "$_filesize" -gt 100000 ]; then
            _header_base=$((30 + _namelen))
            _data_start=$((_offset + _header_base))
            _remainder=$((_data_start % ALIGNMENT))
            if [ "$_remainder" -ne 0 ]; then
                _padding=$((ALIGNMENT - _remainder))
            fi
            # Extra field needs at least 4 bytes for header
            if [ "$_padding" -gt 0 ] && [ "$_padding" -lt 4 ]; then
                _padding=$((_padding + ALIGNMENT))
            fi
        fi
        
        # Write local file header (signature 0x04034b50)
        write_u32 67324752 "$output_path"   # PK\x03\x04
        write_u16 20 "$output_path"         # version needed
        write_u16 0 "$output_path"          # flags
        write_u16 0 "$output_path"          # compression (stored)
        write_u16 0 "$output_path"          # mod time
        write_u16 0 "$output_path"          # mod date
        write_u32 "$_crc" "$output_path"    # crc32
        write_u32 "$_filesize" "$output_path"  # compressed size
        write_u32 "$_filesize" "$output_path"  # uncompressed size
        write_u16 "$_namelen" "$output_path"   # filename length
        write_u16 "$_padding" "$output_path"   # extra field length
        
        # Write filename
        printf '%s' "$_arcname" >> "$output_path"
        
        # Write extra field (padding with null header ID 0x0000)
        if [ "$_padding" -gt 0 ]; then
            write_u16 0 "$output_path"              # header ID
            write_u16 $((_padding - 4)) "$output_path"  # data size
            write_nulls $((_padding - 4)) "$output_path"
        fi
        
        # Write file data
        cat "$_filepath" >> "$output_path"
        
        # Record entry for central directory
        echo "${_offset} ${_namelen} ${_filesize} ${_crc} ${_arcname}" >> "$_entries_file"
        
    done < "$_files_list"
    
    # Get central directory start offset
    _cd_start=$(wc -c < "$output_path" | tr -d ' ')
    _entry_count=0
    
    # Write central directory entries
    while IFS= read -r _entry; do
        _e_offset=$(echo "$_entry" | cut -d' ' -f1)
        _e_namelen=$(echo "$_entry" | cut -d' ' -f2)
        _e_size=$(echo "$_entry" | cut -d' ' -f3)
        _e_crc=$(echo "$_entry" | cut -d' ' -f4)
        _e_name=$(echo "$_entry" | cut -d' ' -f5-)
        
        # Central directory file header (signature 0x02014b50)
        write_u32 33639248 "$output_path"   # PK\x01\x02
        write_u16 20 "$output_path"         # version made by
        write_u16 20 "$output_path"         # version needed
        write_u16 0 "$output_path"          # flags
        write_u16 0 "$output_path"          # compression
        write_u16 0 "$output_path"          # mod time
        write_u16 0 "$output_path"          # mod date
        write_u32 "$_e_crc" "$output_path"  # crc32
        write_u32 "$_e_size" "$output_path" # compressed size
        write_u32 "$_e_size" "$output_path" # uncompressed size
        write_u16 "$_e_namelen" "$output_path"  # filename length
        write_u16 0 "$output_path"          # extra field length
        write_u16 0 "$output_path"          # comment length
        write_u16 0 "$output_path"          # disk number start
        write_u16 0 "$output_path"          # internal attributes
        write_u32 0 "$output_path"          # external attributes
        write_u32 "$_e_offset" "$output_path"  # relative offset
        
        # Write filename
        printf '%s' "$_e_name" >> "$output_path"
        
        _entry_count=$((_entry_count + 1))
    done < "$_entries_file"
    
    # Get central directory size
    _cd_end=$(wc -c < "$output_path" | tr -d ' ')
    _cd_size=$((_cd_end - _cd_start))
    
    # Write end of central directory (signature 0x06054b50)
    write_u32 101010256 "$output_path"      # PK\x05\x06
    write_u16 0 "$output_path"              # disk number
    write_u16 0 "$output_path"              # disk with CD
    write_u16 "$_entry_count" "$output_path"    # entries on disk
    write_u16 "$_entry_count" "$output_path"    # total entries
    write_u32 "$_cd_size" "$output_path"    # central directory size
    write_u32 "$_cd_start" "$output_path"   # central directory offset
    write_u16 0 "$output_path"              # comment length
    
    # Cleanup temp files
    rm -f "$_files_list" "$_entries_file"
    
    log_info "Created $output_path with $_entry_count entries"
    
    cd "$_old_dir"
    
    # Cleanup temp files
    rm -rf "${EXTRACT_DIR}"
    rm -f "${TEMP_DIR}/signed_blob.json" "${TEMP_DIR}/signature.bin" "${TEMP_DIR}/signature.b64"
    rm -f "${TEMP_DIR}/sig_config.json" "${TEMP_DIR}/sig_manifest.json"
    
    # Copy to install path if specified
    if [ -n "$INSTALL_PATH" ]; then
        log_info "Installing to: ${INSTALL_PATH}"
        mkdir -p "$INSTALL_PATH"
        if cp "$output_path" "${INSTALL_PATH}/"; then
            log_info "Installed: ${INSTALL_PATH}/${output_name}"
        else
            log_error "Failed to copy to install path: ${INSTALL_PATH}"
        fi
    fi
    
    echo ""
    echo "========================================"
    echo "  Signing completed successfully!"
    echo "========================================"
    echo ""
    echo "Signed package: ${output_path}"
    echo ""
}

# Sign a single package (core signing workflow)
sign_package() {
    _pkg_path="$1"
    BOLT_PACKAGE="$_pkg_path"
    
    log_info "Processing package: $BOLT_PACKAGE"
    
    # Reset computed identity for each package (do not clear --identity override)
    PACKAGE_IDENTITY=""
    setup_directories
    extract_package
    get_content_manifest_digest
    get_package_identity
    create_signed_blob
    sign_blob
    prepare_certificates
    create_signature_manifest
    update_index
    repackage
}

# Process batch downloads and signing
process_batch() {
    log_info "Batch mode: downloading and signing packages from BOLT_PACKAGE_URLS"
    
    if [ -z "$DOWNLOAD_CMD" ]; then
        log_error "Batch mode requires wget or curl"
    fi
    
    if [ -z "$BOLT_PACKAGE_URLS" ]; then
        log_error "BOLT_PACKAGE_URLS environment variable not set. Example: export BOLT_PACKAGE_URLS='url1;url2;url3'"
    fi
    
    DOWNLOAD_DIR="${TEMP_DIR}/downloads"
    mkdir -p "$DOWNLOAD_DIR"
    
    SIGNED_COUNT=0
    TOTAL_COUNT=0
    
    # Parse semicolon-separated URLs
    _remaining="$BOLT_PACKAGE_URLS"
    while [ -n "$_remaining" ]; do
        # Extract first URL (before semicolon)
        _url=$(echo "$_remaining" | cut -d';' -f1)
        
        # Remove processed URL from remaining
        if echo "$_remaining" | grep -q ';'; then
            _remaining=$(echo "$_remaining" | cut -d';' -f2-)
        else
            _remaining=""
        fi
        
        # Skip empty URLs
        if [ -z "$_url" ]; then
            continue
        fi
        
        TOTAL_COUNT=$((TOTAL_COUNT + 1))
        
        echo ""
        log_info "=== Processing package $TOTAL_COUNT: $_url ==="
        
        # Extract filename from URL (remove query params)
        _filename=$(basename "$_url" | sed 's/?.*//')
        _download_path="${DOWNLOAD_DIR}/${_filename}"
        
        download_file "$_url" "$_download_path"
        sign_package "$_download_path"
        SIGNED_COUNT=$((SIGNED_COUNT + 1))
    done
    
    # Cleanup downloads
    rm -rf "$DOWNLOAD_DIR"
    
    echo ""
    echo "========================================"
    echo "  Batch signing completed!"
    echo "  Signed $SIGNED_COUNT package(s)"
    echo "========================================"
    echo ""
    echo "Output directory: ${OUTPUT_DIR}"
    ls -la "${OUTPUT_DIR}/" 2>/dev/null || true
    if [ -n "$INSTALL_PATH" ]; then
        echo ""
        echo "Install directory: ${INSTALL_PATH}"
        ls -la "${INSTALL_PATH}/" 2>/dev/null || true
    fi
    echo ""
}

# Main
main() {
    echo ""
    echo "========================================"
    echo "  Bolt Package Signing Tool (RDK)"
    echo "========================================"
    echo ""
    
    check_dependencies
    parse_args "$@"

    fetch_default_certificates

    if [ "$BATCH_MODE" = "1" ]; then
        process_batch
    else
        sign_package "$BOLT_PACKAGE"
    fi
}

main "$@"
