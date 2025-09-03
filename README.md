<p align="center">
  <img src="logo.png" alt="GGcode Logo" width="320"/>
</p>

# GGcode

**GGcode** is a parametric G-code compiler that transforms static CNC programs into dynamic, programmable toolpaths. Write JavaScript-like code that generates professional G-code with variables, loops, functions, and math.

🌐 **[Try GGcode Online →](https://gg.doorbase.io)** - No installation required!

## ✨ Key Features

- **Interactive CLI** - Smart file selection menu for easy compilation
- **Professional G-code Output** - Industry-standard formatting with configurable precision
- **Full Programming Language** - Variables, functions, loops, conditionals, and arrays
- **Built-in Math Library** - Trigonometry, geometry functions, and constants
- **Cross-platform** - Works on Linux, macOS, and Windows
- **Lightweight** - Single binary, no dependencies

## 🎯 Quick Example

Instead of repetitive G-code:
```gcode
G1 X10 Y0 F300
G1 X20 Y0 F300  
G1 X30 Y0 F300
```

Write parametric GGcode:
```ggcode
let feed = 300
for i = 1..3 {
    G1 X[i*10] Y0 F[feed]
}
```

**Output:**
```gcode
N10 G1 X10.000 Y0.000 F300.000
N15  X20.000 Y0.000
N20  X30.000 Y0.000
```

## 🚀 Quick Start

### Build & Install
```sh
# Install dependencies
sudo apt install build-essential gcc make  # Ubuntu/Debian
xcode-select --install                     # macOS

# Build with automatic global installation prompt
make
```

**After building, you'll be prompted:**
```
🌍 Install ggcode globally? (y/N): y
Choose installation method:
  1) Symlink to /usr/local/bin (recommended)
  2) Copy to /usr/local/bin  
  3) Add to PATH in ~/.bashrc
```

### Build Options
| Command | Description |
|---------|-------------|
| `make` | Build + prompt for global install |
| `make install` | Install globally (manual) |
| `make uninstall` | Remove global installation |
| `make test` | Run unit tests |
| `make win` | Cross-compile for Windows |
| `make clean` | Clean build files |

## �️U Interactive CLI

**Default behavior:** Run `ggcode` (if installed globally) or `./GGCODE/ggcode` to get an interactive menu:

```
┏┓┏┓┏┓┏┓┳┓┏┓  ┏┓       •┓      ┏┓┓ ┳
┃┓┃┓┃ ┃┃┃┃┣   ┃ ┏┓┏┳┓┏┓┓┃┏┓┏┓  ┃ ┃ ┃
┗┛┗┛┗┛┗┛┻┛┗┛  ┗┛┗┛┛┗┗┣┛┗┗┗ ┛   ┗┛┗┛┻   v1.1.2

Found 3 .ggcode files:

  1) part1.ggcode
  2) part2.ggcode  
  3) complex_pattern.ggcode
  4) Compile all files
  h) Show help
  0) Exit

Select option (0-4, h): 
```

### Command Line Options

```sh
# Interactive menu (default) - use globally or locally
ggcode                       # If installed globally
./GGCODE/ggcode             # Local binary

# Compile specific files
ggcode part.ggcode
ggcode part1.ggcode part2.ggcode

# Compile all files
ggcode -a

# Custom output
ggcode -o custom.gcode part.ggcode
ggcode --output-dir ./build *.ggcode

# Direct evaluation (testing)
ggcode -e "G1 X10 Y20 F300"
ggcode -e "for i=1..5 { G1 X[i*10] Y0 }"

# Options
ggcode -q -a        # Quiet mode
ggcode --help       # Show help
ggcode --version    # Show version
```

## 📁 Project Structure

```
GGcode/
├── src/                    # C source code
│   ├── lexer/             # Tokenization
│   ├── parser/            # Syntax parsing and AST
│   ├── runtime/           # Expression evaluation
│   ├── generator/         # G-code generation
│   ├── cli/               # Command line interface
│   └── main.c             # Main entry point
├── tests/                  # Unit tests
├── GGCODE/                 # Compiled binaries
├── Makefile               # Build system
└── README.md              # Documentation
```

## 📝 Language Reference

### Variables & Arrays
```ggcode
let x = 10                    // Variables
let points = [1, 2, 3]        // 1D arrays
let grid = [[1,2], [3,4]]     // 2D arrays
G1 X[x] Y[points[0]]          // Variable interpolation
```

### Control Flow
```ggcode
if x > 10 { G1 X[x] }         // Conditionals
for i = 1..5 { G1 X[i] }      // Loops
while x < 100 { x = x + 1 }   // While loops

function circle(radius) {      // Functions
    return PI * radius * radius
}
```

### G-code Commands
```ggcode
G0 X[x] Y[y]                  // Rapid move
G1 X[x] Y[y] F[feed]          // Linear move
M3 S[speed]                   // Spindle on
note {Cut depth: [z]mm}       // Comments
```

### Math & Constants
```ggcode
let angle = 45 * DEG_TO_RAD   // Constants: PI, E, TAU, DEG_TO_RAD
let dist = sqrt(x*x + y*y)    // Math: abs, sqrt, sin, cos, tan, etc.
let result = distance(x1, y1, x2, y2)  // Geometry functions
```

### Formatting Control
```ggcode
let decimalpoint = 2          // Control decimal places (0-6)
let nline = 0                 // Disable line numbering
```

## 🎨 Examples

### Parametric Circle
```ggcode
function circle(radius, steps) {
    for i = 0..steps {
        let angle = i * (360 / steps) * DEG_TO_RAD
        let x = radius * cos(angle)
        let y = radius * sin(angle)
        G1 X[x] Y[y] F300
    }
}

circle(10, 36)  // 10mm radius, 36 steps
```

### Conditional Machining
```ggcode
let material = 1  // 1=aluminum, 2=steel
let speed = material == 1 ? 8000 : 6000
let depth = material == 1 ? -2 : -1

M3 S[speed]
G1 Z[depth] F300
```

### Array Processing
```ggcode
let holes = [[10,10], [20,10], [30,10]]
for i = 0..2 {
    G0 X[holes[i][0]] Y[holes[i][1]]
    G1 Z-5 F100
    G0 Z5
}
```



## 🔧 Troubleshooting

**Build Issues:**
```sh
# Install dependencies
sudo apt install build-essential gcc make  # Ubuntu/Debian
brew install gcc make                      # macOS

# Fix permissions
chmod +x GGCODE/ggcode
```

**Runtime Issues:**
- Verify variable names are defined before use
- Check bracket matching `{ }` and `[ ]`
- Use `note {}` blocks for debugging
- Run `make test` to verify installation

---



## 👨‍💻 Author

**T1** - Creator and Lead Developer


### Contributing
- Report issues on GitHub
- Submit feature requests
- Contribute example files
- Help with documentation and translations

### Support
- 🌐 **[Online Demo](https://gg.doorbase.io)** - Try GGcode instantly
- 📧 Email: [t@doorbase.io](mailto:t@doorbase.io)

---


## 📜 License

This project is licensed under the **MIT License** for personal, educational, and small business use.

**License Files:**
- [LICENSE](./LICENSE) — Free use terms and conditions
- [LICENSE-COMMERCIAL](./LICENSE-COMMERCIAL) — Commercial licensing information

**Commercial Use:** For commercial licensing inquiries, contact: [t@doorbase.io](mailto:t@doorbase.io)

---