# GGcode Project Structure Overview

## 🏗️ **Project Architecture**

GGcode is a **hybrid C/Node.js application** with two main components:

### **1. Core Compiler (C Language)**
- **Location:** `src/` directory
- **Purpose:** The heart of GGcode - compiles `.ggcode` files to G-code
- **Output:** Standalone binary (`GGCODE/ggcode`) and shared library (`node/libggcode.so`)

### **2. Web Interface (Node.js)**
- **Location:** `node/` directory  
- **Purpose:** Provides web-based IDE, examples, help system, and 3D visualization
- **Output:** Web server running on port 6990

---

## 📁 **Directory Structure**

```
GGcode_DEV/
├── 📁 src/                    # Core C compiler source code
│   ├── 📁 lexer/             # Tokenization and lexical analysis
│   ├── 📁 parser/            # Syntax parsing and AST generation
│   ├── 📁 runtime/           # Expression evaluation and runtime state
│   ├── 📁 generator/         # G-code emission and output generation
│   ├── 📁 error/             # Error handling and reporting
│   ├── 📁 config/            # Configuration and settings
│   ├── 📁 utils/             # Utility functions and helpers
│   ├── 📁 bindings/          # Node.js FFI bindings
│   └── 📄 main.c             # Main compiler entry point
│
├── 📁 node/                   # Web application and interface
│   ├── 📁 views/             # EJS templates for web pages
│   ├── 📁 public/            # Static assets (CSS, JS, images)
│   ├── 📁 data/              # Help content and documentation
│   ├── 📁 GGCODE/            # Example GGcode files
│   ├── 📄 ggcode.js          # Express.js web server
│   ├── 📄 libggcode.so       # Shared library (built from C code)
│   └── 📄 package.json       # Node.js dependencies
│
├── 📁 tests/                  # Unit tests and test framework
├── 📁 bin/                    # Compiled test binaries
├── 📄 Makefile               # Build system and targets
├── 📄 README.md              # Project documentation
└── 📄 video.md               # Video script for project promotion
```

---

## 🔧 **Core Components Breakdown**

### **📁 src/ - C Compiler Core**

#### **lexer/** - Tokenization
- **Purpose:** Converts GGcode source text into tokens
- **Key Files:** `lexer.c`, `token_types.h`, `keyword_table.h`
- **Function:** Identifies keywords, operators, variables, literals

#### **parser/** - Syntax Analysis  
- **Purpose:** Builds Abstract Syntax Tree (AST) from tokens
- **Key Files:** `parser.c`, `ast_nodes.h`, `ast_helpers.c`
- **Function:** Validates syntax, creates program structure

#### **runtime/** - Execution Engine
- **Purpose:** Evaluates expressions and manages program state
- **Key Files:** `evaluator.c`, `runtime_state.h`
- **Function:** Variable storage, math operations, control flow

#### **generator/** - G-code Output
- **Purpose:** Converts AST into G-code commands
- **Key Files:** `emitter.c`, `emitter.h`
- **Function:** G-code modal behavior, coordinate calculations

#### **error/** - Error Handling
- **Purpose:** Comprehensive error reporting and recovery
- **Key Files:** `error.c`, `error.h`
- **Function:** Syntax errors, runtime errors, warnings

#### **utils/** - Helper Functions
- **Purpose:** File I/O, memory management, reporting
- **Key Files:** `file_utils.c`, `output_buffer.c`, `report.c`
- **Function:** File operations, memory allocation, output formatting

#### **bindings/** - Node.js Integration
- **Purpose:** FFI (Foreign Function Interface) for Node.js
- **Key Files:** `nodejs.c`
- **Function:** Exposes C functions to JavaScript

---

### **📁 node/ - Web Application**

#### **views/** - Web Templates
- **index.ejs** - Main editor interface
- **app.ejs** - JavaScript for editor functionality
- **helpExamples.ejs** - Help system interface
- **view.ejs** - 3D visualization viewer
- **help-template.ejs** - Help content template

#### **public/** - Static Assets
- **style.css** - Main application styling
- **flags.css** - Language flag icons (SVG)
- **OrbitControls.js** - 3D camera controls

#### **data/help-content/** - Multi-language Documentation
- **metadata.json** - Language configuration
- **en.json, es.json, fr.json, etc.** - Help content in 15 languages
- **MULTILANGUAGE_GUIDE.md** - Translation guidelines

#### **GGCODE/** - Example Files
- **Basic examples:** `basic_circle.ggcode`, `square.ggcode`
- **Advanced patterns:** `Flower Pattern with Arcs.ggcode`
- **Complex geometries:** `Gear Teeth.ggcode`, `True Spiral with Arcs.ggcode`
- **Educational:** `configurator_demo.ggcode`, `math_functions.ggcode`

---

## 🚀 **Build System**

### **Makefile Targets:**

| Command | Description |
|---------|-------------|
| `make` | Build main compiler binary |
| `make node` | Build shared library for Node.js |
| `make test` | Run all unit tests |
| `make clean` | Remove build artifacts |
| `make win` | Cross-compile for Windows |

### **Build Process:**
1. **C Compiler:** `src/*.c` → `GGCODE/ggcode` (standalone binary)
2. **Shared Library:** `src/*.c` → `node/libggcode.so` (for Node.js)
3. **Web Server:** `node/ggcode.js` (Express.js application)

---

## 🔄 **Data Flow**

### **GGcode Compilation Pipeline:**
```
GGcode Source (.ggcode)
        ↓
    Lexer (Tokens)
        ↓
    Parser (AST)
        ↓
   Runtime (Evaluation)
        ↓
   Generator (G-code)
        ↓
   Output (.g.gcode)
```

### **Web Application Flow:**
```
User Input (GGcode)
        ↓
   Node.js Server
        ↓
   FFI → C Library
        ↓
   Compilation
        ↓
   G-code Output
        ↓
   3D Visualization
```

---

## 🌐 **Web Interface Features**

### **Main Components:**
1. **Code Editor** - Syntax highlighting, line numbers
2. **Configurator Modal** - Interactive parameter adjustment
3. **3D Visualizer** - Real-time tool path preview
4. **Help System** - Multi-language documentation
5. **Examples Library** - 30+ example files
6. **Language Selector** - 15 supported languages

### **Key Technologies:**
- **Backend:** Node.js + Express.js
- **Frontend:** EJS templates + vanilla JavaScript
- **3D Graphics:** Three.js for visualization
- **FFI:** ffi-napi for C/JavaScript bridge
- **Styling:** Custom CSS with RTL support

---

## 📊 **Project Statistics**

- **Lines of C Code:** ~15,000+ (core compiler)
- **Lines of JavaScript:** ~2,000+ (web interface)
- **Example Files:** 30+ GGcode programs
- **Supported Languages:** 15 (help documentation)
- **Test Coverage:** Comprehensive unit tests
- **Build Targets:** Linux, Windows, Node.js

---

## 🎯 **Key Features by Component**

### **Core Compiler (C):**
- ✅ Variables and expressions
- ✅ Control flow (if/else, loops)
- ✅ Functions and recursion
- ✅ Arrays (1D, 2D, dynamic)
- ✅ Math library (sin, cos, sqrt, etc.)
- ✅ G-code modal behavior
- ✅ Error handling and reporting

### **Web Interface (Node.js):**
- ✅ Real-time compilation
- ✅ Interactive parameter adjustment
- ✅ 3D tool path visualization
- ✅ Multi-language help system
- ✅ Example library browser
- ✅ Syntax highlighting
- ✅ Export functionality

---

## 🔧 **Development Workflow**

### **Adding New Features:**
1. **Core Feature:** Modify C code in `src/`
2. **Tests:** Add tests in `tests/`
3. **Web Integration:** Update `node/` if needed
4. **Documentation:** Update help content in `node/data/`
5. **Examples:** Add examples in `node/GGCODE/`

### **Building and Testing:**
```bash
# Build everything
make && make node

# Run tests
make test

# Start web server
cd node && npm start
```

---

## 🎬 **Video Production Assets**

### **Files for Video:**
- **video.md** - Complete script and production plan
- **node/GGCODE/** - Example files to demonstrate
- **Web Interface** - Real-time demonstrations
- **Help System** - Feature explanations

### **Key Demonstrations:**
1. **Basic Variables** - `basic_circle.ggcode`
2. **Loops and Patterns** - `Flower Pattern with Arcs.ggcode`
3. **Complex Geometry** - `Gear Teeth.ggcode`
4. **Configurator** - `configurator_demo.ggcode`
5. **3D Visualization** - Real-time preview

---

This structure makes GGcode a powerful, professional tool that combines the performance of C with the accessibility of a modern web interface! 🚀 