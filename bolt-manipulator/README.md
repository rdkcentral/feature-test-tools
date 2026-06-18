# Bolt Manipulator

Bolt Manipulator is a collection of tools designed to manipulate Bolt packages without requiring RALF-pack or bolt-tools.

## sign_bolt_rdk

Lightweight shell script to sign Bolt/RALF packages on RDK embedded devices without [ralfpack](https://github.com/rdkcentral/ralfpack)/[bolt-tools](https://github.com/rdkcentral/bolt-tools/tree/main) dependencies.

A pure POSIX shell tool for signing Bolt packages on resource-constrained RDK set-top boxes. Implements cosign-compatible OCI signatures using only BusyBox-compatible commands. Supports batch signing from URLs and creates properly aligned ZIP packages for erofs mounting.

### Features

- **No Python required** - Pure POSIX shell, works with BusyBox
- **Cosign-compatible signatures** - OCI image signature format
- **Erofs-ready output** - 4096-byte aligned ZIP for direct mounting
- **Batch mode** - Download and sign multiple packages from URLs
- **Minimal dependencies** - Uses only standard Unix tools

### Requirements

#### Required Commands
- `openssl` - Cryptographic signing
- `sha256sum` - Hash calculation
- `base64` - Encoding
- `gzip` - CRC32 calculation
- `tar` - Archive handling
- `unzip` - Package extraction (for .bolt/.zip files)
- `sed`, `grep`, `awk`, `cut` - Text processing
- `od`, `dd`, `find` - Binary/file operations

#### Optional Commands
- `wget` or `curl` - Required for batch mode and for auto-fetching default certificates

### Usage

#### Sign a Single Package

```sh
./sign_bolt_rdk.sh [private_key.pem] <package.bolt> [OPTIONS]
```

**Example:**
```sh
./sign_bolt_rdk.sh ~/private.pem /tmp/myapp.bolt --cert ~/certificate.pem
```

#### Quick Reference

```sh
# With explicit key and cert (original behaviour)
sh sign_bolt_rdk.sh private.key package.bolt --cert cert.pem

# With explicit key, auto-fetch cert
sh sign_bolt_rdk.sh private.key package.bolt

# Fully auto — fetch both key and cert from repo
sh sign_bolt_rdk.sh package.bolt
```

#### Default RDK Engineering Certificates (Auto Fetch)

If you do not provide a private key and/or `--cert`, the script automatically downloads development certificates from:

- https://github.com/rdkcentral/bolt-engineering-certificates
- Files used:
  - `certs/com.rdkcentral.ralf-private.key`
  - `certs/com.rdkcentral.ralf-public.crt`

Important:
- These are self-signed engineering certificates for development/testing only.
- Do not use them in production environments.

#### Batch Mode (Multiple Packages)

Sign multiple packages by providing URLs in an environment variable:

```sh
# Set package URLs (semicolon-separated)
export BOLT_PACKAGE_URLS="https://example.com/app1.bolt;https://example.com/app2.bolt;https://example.com/app3.bolt"

# Run batch signing
./sign_bolt_rdk.sh ~/private.pem --batch --cert ~/certificate.pem

# Or rely on auto-fetched default key/cert
./sign_bolt_rdk.sh --batch
```

#### Options

| Option | Description |
|--------|-------------|
| `--cert FILE` | Path to signing certificate (PEM format) |
| `--chain FILE` | Path to certificate chain (PEM format) |
| `--passphrase PASS` | Passphrase for encrypted private key |
| `--identity ID` | Override package identity |
| `--batch` | Enable batch mode (download from `BOLT_PACKAGE_URLS`) |
| `--install-path DIR` | Copy signed packages to specified directory |

Notes:
- `private_key.pem` positional argument is optional.
- `--cert` is optional.
- If either is omitted, the script fetches the corresponding default from the RDK engineering certificates repository.

#### Output

Signed packages are written to:
```
/tmp/signing_temp/signed_packages/<package_name>.bolt
```

### How It Works

1. **Extract** - Unpacks the bolt package (ZIP/OCI format)
2. **Find manifest** - Locates the content manifest digest
3. **Create signature blob** - JSON with manifest digest and identity
4. **Sign** - RSA/ECDSA signature using OpenSSL
5. **Create signature manifest** - OCI manifest with cosign annotations
6. **Update index** - Adds signature reference to index.json
7. **Repackage** - Creates aligned ZIP with signature included

#### Signature Format

The script creates cosign-compatible signatures:

```json
{
  "critical": {
    "identity": {"docker-reference": "package-id"},
    "image": {"docker-manifest-digest": "sha256:..."},
    "type": "cosign container image signature"
  },
  "optional": {
    "creator": "sign_bolt_rdk.sh",
    "timestamp": 1234567890
  }
}
```

#### ZIP Alignment

For erofs filesystem mounting, the script ensures large blobs are aligned to 4096-byte boundaries using ZIP extra field padding. This allows the erofs image to be mounted directly from within the ZIP without extraction.

### Examples

#### Basic Signing
```sh
./sign_bolt_rdk.sh private.pem app.bolt
```

#### Auto-Fetch Key and Certificate
```sh
./sign_bolt_rdk.sh app.bolt
```

#### Use Local Key, Auto-Fetch Certificate
```sh
./sign_bolt_rdk.sh private.pem app.bolt
```

#### With Certificate
```sh
./sign_bolt_rdk.sh private.pem app.bolt --cert signing_cert.pem
```

#### With Certificate Chain
```sh
./sign_bolt_rdk.sh private.pem app.bolt --cert signing_cert.pem --chain ca_chain.pem
```

#### Encrypted Private Key
```sh
./sign_bolt_rdk.sh encrypted_key.pem app.bolt --passphrase "mysecret"
```

#### Batch Download and Signing
```sh
#!/bin/sh
export BOLT_PACKAGE_URLS="${BASE_APP_URL};${WPE_APP_URL};${REFUI_APP_URL}"
./sign_bolt_rdk.sh "$SIGNING_KEY" --batch --cert "$SIGNING_CERT"
```

#### Batch Download and Signing with Auto-Fetched Defaults
```sh
#!/bin/sh
export BOLT_PACKAGE_URLS="${BASE_APP_URL};${WPE_APP_URL};${REFUI_APP_URL}"
./sign_bolt_rdk.sh --batch
```

### Troubleshooting

#### "Package is not mountable"
The erofs blob must be at a 4096-byte aligned offset. This script handles alignment automatically. If you see this error with packages signed by other tools, they may not have proper alignment.

#### "Required command not found"
Install missing dependencies. On RDK/BusyBox systems, most commands are built-in. Ensure `openssl` is available.

#### "No download tool available"
Batch mode requires `wget` or `curl`. Install one or use single-package mode with local files.

### Verification

Signed packages can be verified using standard cosign verification or the RDK package manager's built-in verification.