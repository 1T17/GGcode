# GGcode Setup

## Prerequisites

**Linux/Ubuntu:**
```bash
sudo apt install build-essential gcc make
```

**macOS:**
```bash
xcode-select --install
```

**Windows:**
- Install MinGW-w64 or use WSL with Linux instructions

## Build & Install

```bash
# Clone and build
git clone <repository>
cd GGcode
make
```

**Automatic Installation Prompt:**
After building, you'll be prompted to install globally:
```
🌍 Install ggcode globally? (y/N): y

Choose installation method:
  1) Symlink to /usr/local/bin (recommended)
  2) Copy to /usr/local/bin  
  3) Add to PATH in ~/.bashrc
Enter choice (1-3): 1
```

## Build Options

| Command | Purpose |
|---------|---------|
| `make` | Build + prompt for global install |
| `make install` | Install globally (manual) |
| `make uninstall` | Remove global installation |
| `make test` | Run all tests |
| `make win` | Cross-compile for Windows |
| `make clean` | Clean build files |

## Troubleshooting

**Permission issues:**
```bash
chmod +x GGCODE/ggcode
```

**Missing dependencies:**
```bash
# Ubuntu/Debian
sudo apt update && sudo apt install build-essential

# macOS
brew install gcc make
```

**Test Unity framework:**
```bash
make unity  # Downloads test framework automatically
make test   # Runs comprehensive test suite
```

## Manual Global Installation

If you skipped the automatic prompt:

```bash
# Install globally with interactive menu
make install

# Or manually:
sudo ln -sf $(pwd)/GGCODE/ggcode /usr/local/bin/ggcode  # Symlink (recommended)
sudo cp GGCODE/ggcode /usr/local/bin/ggcode             # Copy
echo 'export PATH="$PATH:$(pwd)/GGCODE"' >> ~/.bashrc   # Add to PATH
```

**Remove global installation:**
```bash
make uninstall
```

## Verification

```bash
# Test basic functionality
ggcode -e "G1 X10 Y20 F300"

# Interactive mode from any directory
ggcode

# Check version
ggcode --version
```

Done. Ready to compile GGcode files from anywhere.