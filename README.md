<p align="center">
  <img src="logo.png" alt="GGcode Logo" width="320"/>
  <!-- For SVG support, you can use: <img src="logo.svg" alt="GGcode Logo" width="320"/> -->
</p>

# GGcode

**GGcode** is a custom G-code scripting language and compiler that brings programmability to CNC machining. Designed for automation, testing, and dynamic toolpath generation, it supports variables, control flow, expression evaluation, and runtime note/comment blocks.

Transform your CNC programming from static G-code to dynamic, parametric toolpaths with the power of a full programming language tailored specifically for machining.

🌐 **[Try GGcode Online →](https://gg.doorbase.io)** - No installation required!

---



## ✨ Features

**🪶 Lightweight & Embeddable**  
- Tiny, standalone compiler for `.ggcode` source files  
- Easy to embed in toolchains or CNC preprocessors

**🔤 Human-Readable Syntax**  
- JavaScript-like structure tailored for G-code logic  
- Clean, readable syntax for operators, conditionals, and loops

**📥 Variables & Assignments**  
- Use `let` to define and reassign variables  
- Supports negative numbers, runtime expressions, and math logic

**🧠 Expression Support**  
- Full arithmetic and logical expressions: `+`, `-`, `*`, `/`, `mod`, `!`, comparisons  
- Nested expressions and grouped logic fully supported

**🔁 Control Flow**  
- Conditional blocks: `if`, `else`, `else if` with any expression  
- Looping constructs: `while`, `for` with scoped variables and dynamic conditions

**🧮 Built-in Math Library**  
- Functions: `abs`, `mod`, `floor`, `ceil`, `sqrt`, `hypot`, `distance`, `sin`, `cos`, `atan2`, etc.  
- Constants: `PI`, `E`, `TAU`, `DEG_TO_RAD`, and more

**🧩 Functions**  
- Define reusable logic with the `function` keyword  
- Support for recursion, parameters, and local scopes

**🗂️ Arrays (1D & 2D)**  
- Declare and mutate arrays: `grid[1][2] = 5`  
- Dynamic indexing, assignment, and array-length flexibility

**🔄 Recursion-Safe Call Tree Output**  
- Internal tree generation with recursion tracking  
- Exported as collapsible HTML tree with auto-loop detection

**📄 Parametric G-code Generation**  
- Emit raw `G0`, `G1`, `M3`, etc. from script logic  
- Generate toolpaths dynamically using math and variables

**🗨️ `note {}` Blocks**  
- Embed runtime comments, debug logs, or human-readable G-code remarks  
- Supports variable interpolation: `note {Pass #[i], Z = [z]}`

---

## 🎯 Quick Example

Instead of writing repetitive G-code manually:
```gcode
G1 X10 Y0 F300
G1 X20 Y0 F300  
G1 X30 Y0 F300
```

Write parametric GGcode:
```ggcode
let feed = 300
let y_pos = 0

note {
    Creating linear pattern
    Feed rate: [feed] mm/min
}

for i = 1..3 {
    G1 X[i*10] Y[y_pos] F[feed]
}
```

**Output G-code:**
```gcode
; Creating linear pattern
; Feed rate: 300 mm/min
G1 X10 Y0 F300
G1 X20 Y0 F300
G1 X30 Y0 F300
```

---

## 📋 Requirements

### System Requirements
- **Linux/macOS**: GCC compiler, Make
- **Windows**: MinGW-w64 for cross-compilation
- **Node.js**: Version 14+ (for web interface)
- **Memory**: ~50MB RAM for compilation

### Dependencies
```sh
# Ubuntu/Debian
sudo apt install build-essential gcc make curl

# macOS
xcode-select --install

# Node.js dependencies (for web interface)
cd node && npm install
```

---

### 🛠️ Build System

Use the following `make` targets to build, test, and manage the project:

| Command | Description | Output |
|---------|-------------|--------|
| `make` or `make all` | Build main compiler binary | `GGCODE/ggcode` |
| `make node` | Build shared library for Node.js | `node/libggcode.so` |
| `make test` | Run all unit tests with summary | Test results |
| `make tests` | Compile test binaries only | `bin/test_*` |
| `make win` | Cross-compile for Windows | `GGCODE/ggcode.exe` |
| `make clean` | Remove all build artifacts | - |

### Build Process
1. **Core Compiler**: C source → standalone binary (`GGCODE/ggcode`)
2. **Shared Library**: C source → Node.js library (`node/libggcode.so`)  
3. **Web Server**: Node.js Express application (`node/ggcode.js`)
4. **Tests**: Comprehensive unit test suite with Unity framework

## 🚀 Usage

### Command Line Interface
```sh
# Compile a single file
./GGCODE/ggcode myfile.ggcode
# Output: myfile.g.gcode

# Compile all .ggcode files in current directory  
./GGCODE/ggcode
# Output: Creates .g.gcode files for each .ggcode file

# Cross-platform usage
./GGCODE/ggcode.exe myfile.ggcode  # Windows
```

### Web Interface
1. Start the server: `cd node && npm start`
2. Open browser: `http://localhost:6990`
3. Write GGcode in the editor
4. Click "Compile" to see G-code output
5. Use "3D View" to visualize toolpaths
6. Browse examples for inspiration

### Integration Examples
```sh
# Embed in build pipeline
./GGCODE/ggcode parts/*.ggcode && echo "All parts compiled"

# Use with CAM workflow  
./GGCODE/ggcode template.ggcode > output.gcode
```

---
## 🚦 Quick Start

### Option 1: Online Demo (Instant)
🌐 **[Try GGcode Online →](https://gg.doorbase.io)**
- No installation required
- Full web interface with examples
- 3D visualization and real-time compilation
- Perfect for testing and learning

### Option 2: Local Web Interface
```sh
# Build the compiler and Node.js backend
make all
make node

# Install Node.js dependencies
cd node
npm install

# Start the web UI
npm start
# or: node ggcode.js

# Open in your browser:
http://localhost:6990
```

### Option 3: Command Line
```sh
# Build the compiler
make

# Compile a single file
./GGCODE/ggcode myfile.ggcode

# Compile all .ggcode files in current directory
./GGCODE/ggcode
```

---

## 📁 Project Structure

```
GGcode/
├── src/                    # C source code for the compiler and runtime
│   ├── lexer/             # Tokenization and lexical analysis
│   ├── parser/            # Syntax parsing and AST generation  
│   ├── runtime/           # Expression evaluation and runtime state
│   ├── generator/         # G-code emission and output generation
│   ├── error/             # Error handling and reporting
│   ├── utils/             # Utility functions and helpers
│   ├── bindings/          # Node.js FFI bindings
│   └── main.c             # Main compiler entry point
├── node/                   # Node.js web application
│   ├── views/             # EJS templates for web pages
│   ├── public/            # Static assets (CSS, JS, images)
│   ├── data/              # Help content in 15 languages
│   ├── GGCODE/            # 34 example GGcode files
│   ├── ggcode.js          # Express.js web server
│   └── package.json       # Node.js dependencies
├── tests/                  # Unit and integration tests
├── bin/                    # Compiled test binaries
├── GGCODE/                 # Compiled binaries and examples
├── Makefile               # Build automation
└── README.md              # This documentation
```

---

## 🌐 Web Interface Features

The Node.js web application provides a complete development environment:

- **Real-time Editor** - Syntax highlighting and live compilation
- **3D Visualization** - Interactive toolpath preview using Three.js
- **Parameter Configurator** - Adjust variables with sliders and inputs
- **Example Library** - 34 ready-to-use GGcode programs including:
  - Basic shapes (circles, squares, spirals)
  - Complex patterns (Flower of Life, gear teeth, rose patterns)
  - Advanced features (arrays, functions, math demonstrations)
- **Multi-language Help** - Documentation in 15 languages
- **Export Options** - Download generated G-code files
- **Error Reporting** - Clear, helpful error messages with line numbers

---

## 🧾 Supported Syntax

### Variables & Data Types
| Syntax | Description | Example |
|--------|-------------|---------|
| `let` | Define/update variables (numbers only) | `let x = 10`, `let count = 5` |
| `[]` | Arrays (1D/2D) | `let points = [1, 2, 3]`, `grid[y][x] = 5` |
| `[expr]` | Variable interpolation | `G1 X[x+10] Y[y*2]` |

**Note**: GGcode currently supports numeric values and arrays. String literals are not supported.

### Control Flow
| Syntax | Description | Example |
|--------|-------------|---------|
| `if` / `else if` / `else` | Conditional execution | `if x > 40 { G1 X[x] }` |
| `while` | Loop with condition | `while x < 100 { x = x + 1 }` |
| `for` | Range-based loop | `for i = 0..10 { G1 X[i] }` |
| `function` | Define reusable functions | `function circle(r) { ... }` |
| `return` | Return value from function | `return x * 2 + 1` |

### G-code Integration
| Syntax | Description | Example |
|--------|-------------|---------|
| `G0`, `G1`, `G2`, `G3` | Motion commands | `G1 X[x] Y[y] F[feed]` |
| `M3`, `M5`, `M8`, `M9` | Machine commands | `M3 S[spindle_speed]` |
| `note {}` | Runtime comments | `note {Cut depth: [z_depth]mm}` |

### Operators & Expressions
| Category | Operators | Example |
|----------|-----------|---------|
| Arithmetic | `+`, `-`, `*`, `/`, `mod()` | `x = (a + b) * 2`, `r = mod(x, 10)` |
| Comparison | `==`, `!=`, `<`, `>`, `<=`, `>=` | `if x >= 10` |
| Logical | `!`, `&&`, `||` | `if x > 0 && y < 100` |

### Built-in Functions
| Function | Description | Example |
|----------|-------------|---------|
| **Math** | `abs()`, `sqrt()`, `floor()`, `ceil()` | `r = sqrt(x*x + y*y)` |
| **Trigonometry** | `sin()`, `cos()`, `tan()`, `atan2()` | `x = r * cos(angle)` |
| **Geometry** | `distance()`, `hypot()` | `d = distance(x1, y1, x2, y2)` |
| **Constants** | `PI`, `E`, `TAU`, `DEG_TO_RAD` | `angle = 45 * DEG_TO_RAD` |

---

## 🎨 Advanced Examples

### Parametric Gear Generation
```ggcode
function gear_tooth(radius, tooth_angle) {
    let x1 = radius * cos(tooth_angle - 2)
    let y1 = radius * sin(tooth_angle - 2)
    let x2 = (radius + 2) * cos(tooth_angle)
    let y2 = (radius + 2) * sin(tooth_angle)
    
    G1 X[x1] Y[y1]
    G1 X[x2] Y[y2]
    return tooth_angle + 10
}

let teeth = 12
let base_radius = 20
for i = 0..teeth {
    gear_tooth(base_radius, i * (360/teeth))
}
```

### Dynamic Array Processing
```ggcode
let points = [[0,0], [10,5], [20,0], [15,-5]]
let feed_rates = [100, 200, 300, 150]

for i = 0..3 {
    let pt = points[i]
    let feed = feed_rates[i]
    G1 X[pt[0]] Y[pt[1]] F[feed]
}
```

### Conditional Machining
```ggcode
let material_type = 1  // 1 = aluminum, 2 = steel
let depth = -1
let speed = 6000

if material_type == 1 {
    depth = -2
    speed = 8000
} else {
    depth = -1
    speed = 6000
}

note {Material type: [material_type], Depth: [depth]mm, Speed: [speed]rpm}
M3 S[speed]
G1 Z[depth] F300
```

## 🎯 Use Cases

- **Parametric Parts**: Create families of similar parts with variable dimensions
- **Pattern Generation**: Complex geometric patterns impossible with manual G-code
- **Batch Processing**: Generate multiple variations from a single template
- **Educational**: Learn CNC programming with a familiar syntax
- **Prototyping**: Rapid iteration of toolpath strategies
- **Integration**: Embed in larger CAM/manufacturing workflows

GGcode supports nesting, recursion, dynamic expressions, scoped variables, and parametric G-code generation.



## 🔧 Troubleshooting

### Common Issues

**Build Errors:**
```sh
# Missing dependencies
sudo apt install build-essential gcc make  # Ubuntu/Debian
brew install gcc make                      # macOS

# Permission issues
chmod +x GGCODE/ggcode
```

**Web Interface Issues:**
```sh
# Port already in use
lsof -ti:6990 | xargs kill -9  # Kill process on port 6990

# Node.js dependencies
cd node && npm install --force

# Missing shared library
make node  # Rebuild libggcode.so
```

**Runtime Errors:**
- Check syntax with web interface for better error messages
- Verify variable names are defined before use
- Ensure proper bracket matching `{ }` and `[ ]`
- Use `note {}` blocks for debugging variable values

### Performance Tips
- Use `make test` to verify installation
- Large arrays may require more memory
- Complex recursive functions have built-in loop detection
- Web interface provides real-time syntax checking

---

## 📜 License

This project is licensed under the **MIT License** for personal, educational, and small business use.

**License Files:**
- [LICENSE](./LICENSE) — Free use terms and conditions
- [LICENSE-COMMERCIAL](./LICENSE-COMMERCIAL) — Commercial licensing information

**Commercial Use:** For commercial licensing inquiries, contact: [t@doorbase.io](mailto:t@doorbase.io)

---

## 👨‍💻 Author

**T1** - Creator and Lead Developer

> *"GGcode is built for control, clarity, and creativity — bringing the power of programming to precision machining."*

### Contributing
- Report issues on GitHub
- Submit feature requests
- Contribute example files
- Help with documentation and translations

### Support
- 🌐 **[Online Demo](https://gg.doorbase.io)** - Try GGcode instantly in your browser
- 📧 Email: [t@doorbase.io](mailto:t@doorbase.io)
- 🌐 Web Interface: Built-in help system with 15 languages
- 📚 Examples: 34 sample programs included

---

**GGcode** - Where programming meets precision manufacturing. Transform your CNC workflow with the power of parametric toolpath generation.