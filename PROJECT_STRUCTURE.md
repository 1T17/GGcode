# GGcode Project Structure Overview

## 🏗️ **Project Architecture**

GGcode is a **standalone C compiler** with an interactive command-line interface:

### **Core Compiler (C Language)**
- **Location:** `src/` directory
- **Purpose:** Compiles `.ggcode` files to professional G-code
- **Output:** Standalone binary (`GGCODE/ggcode`)
- **Features:** Interactive CLI, cross-platform support, comprehensive error handling

---

## 📁 **Complete Directory Structure**

```
GGcode/
├── 📁 .git/                           # Git repository data and version control history
├── 📁 .github/                        # GitHub templates and CI/CD workflows
│   └── 📁 ISSUE_TEMPLATE/
│       ├── bug_report.md              # Bug report template for GitHub issues
│       └── feature_request.md         # Feature request template for GitHub issues
├── 📁 .kiro/                          # Kiro IDE configuration and project settings
│   └── 📁 specs/                      # Kiro specification files and project specs
├── 📁 .vscode/                        # Visual Studio Code workspace settings
│   ├── c_cpp_properties.json         # C/C++ IntelliSense configuration for VS Code
│   └── settings.json                  # VS Code workspace-specific settings
│
├── 📁 src/                            # 🎯 CORE SOURCE CODE
│   ├── 📁 cli/                        # ✨ Interactive CLI (NEW)
│   │   ├── cli.c                      # Interactive file selection & argument parsing implementation
│   │   └── cli.h                      # CLI interface definitions & comprehensive documentation
│   ├── 📁 lexer/                      # Tokenization and lexical analysis
│   │   ├── lexer.c                    # Main lexical analyzer - converts source text to tokens
│   │   ├── lexer.h                    # Lexer interface and function declarations
│   │   ├── keyword_table.h            # GGcode keyword definitions (let, if, for, while, etc.)
│   │   ├── token_types.h              # Token type enumerations and definitions
│   │   ├── token_utils.c              # Token utility functions and string handling
│   │   └── token_utils.h              # Token utilities interface declarations
│   ├── 📁 parser/                     # Syntax parsing and AST generation
│   │   ├── parser.c                   # Main parser implementation - builds AST from tokens
│   │   ├── parser.h                   # Parser interface and function declarations
│   │   ├── ast_nodes.h                # AST node type definitions and data structures
│   │   └── ast_helpers.c              # AST utility functions and tree manipulation helpers
│   ├── 📁 runtime/                    # Expression evaluation and runtime state
│   │   ├── evaluator.c                # Expression evaluator engine - executes GGcode programs
│   │   ├── evaluator.h                # Evaluator interface and function declarations
│   │   └── runtime_state.h            # Runtime state definitions and variable storage
│   ├── 📁 generator/                  # G-code emission and output generation
│   │   ├── emitter.c                  # G-code generator - converts AST to professional G-code
│   │   └── emitter.h                  # Emitter interface and G-code generation functions
│   ├── 📁 config/                     # Configuration and settings management
│   │   ├── config.c                   # Configuration system - handles decimalpoint, nline variables
│   │   └── config.h                   # Configuration interface and setting definitions
│   ├── 📁 error/                      # Error handling and reporting system
│   │   ├── error.c                    # Error management - syntax errors, runtime errors, warnings
│   │   └── error.h                    # Error interface and error type definitions
│   ├── 📁 utils/                      # Utility functions and helper modules
│   │   ├── file_utils.c               # File I/O operations - read/write .ggcode and .g.gcode files
│   │   ├── file_utils.h               # File utilities interface declarations
│   │   ├── output_buffer.c            # Output buffer management - G-code accumulation and formatting
│   │   ├── output_buffer.h            # Output buffer interface and buffer operations
│   │   ├── report.c                   # Compilation reporting - statistics, timing, memory usage
│   │   ├── report.h                   # Report interface and reporting function declarations
│   │   ├── time_utils.h               # Time measurement utilities for performance profiling
│   │   ├── memory_helpers.h           # Memory management helpers and safe allocation macros
│   │   └── compat.h                   # Cross-platform compatibility definitions and macros
│   ├── 📁 bindings/                   # Node.js FFI bindings (legacy web interface)
│   │   └── nodejs.c                   # Node.js interface for web application integration
│   └── 📄 main.c                      # Main compiler entry point - CLI coordination and flow control
│
├── 📁 tests/                          # 🧪 COMPREHENSIVE TEST SUITE
│   ├── 📁 scripts/                    # Test automation shell scripts
│   │   ├── test_security.sh           # Comprehensive security test suite
│   │   ├── manual_security_test.sh    # Manual security verification tests
│   │   ├── test_performance.sh        # Performance benchmarking and profiling
│   │   ├── test_edge_cases.sh         # Edge case and unusual input testing
│   │   └── test_interactive.sh        # Interactive CLI mode testing
│   ├── 📁 Unity/                      # Unity C testing framework (third-party)
│   │   ├── 📁 src/                    # Unity framework source files and headers
│   │   ├── 📁 docs/                   # Unity documentation and usage guides
│   │   ├── 📁 examples/               # Unity usage examples and test patterns
│   │   └── 📄 README.md               # Unity framework information and setup
│   ├── 📁 temp/                       # Temporary test files and build artifacts
│   │   ├── compare_dirs.sh            # Directory comparison script for regression testing
│   │   └── diff_report.html           # Test difference reports and analysis output
│   ├── 📄 README.md                   # Test suite documentation and usage guide
│   ├── test_gcode_operations.c        # G-code generation tests - validates output correctness
│   ├── test_operation_combinations.c  # Complex operation tests - integration and edge cases
│   ├── test_lexer.c                   # Lexical analysis tests - token validation and parsing
│   ├── test_parser.c                  # Parser functionality tests - AST generation validation
│   ├── test_evaluator.c               # Expression evaluation tests - runtime behavior testing
│   ├── test_emitter.c                 # G-code emission tests - output format validation
│   ├── test_config_variables.c        # Configuration system tests - settings and variables
│   ├── test_output_buffer.c           # Output buffer tests - memory management and formatting
│   ├── test_memory_leaks.c            # Memory leak detection - resource management validation
│   ├── test_missing_operations.c      # Missing operation tests - error handling validation
│   ├── test_security_buffer_overflow.c # Buffer overflow and security vulnerability tests
│   └── test_ternary.c                 # Ternary operator tests - conditional expression validation
│
├── 📁 GGCODE/                         # 🎯 COMPILED BINARIES & EXAMPLES
│   ├── 📁 Gcode/                      # Generated G-code output directory
│   │   └── array test.g.gcode         # Example generated G-code output
│   ├── ggcode                         # Main compiler binary (Linux/macOS executable)
│   ├── array test.ggcode              # Array functionality demonstration - 1D/2D arrays
│   ├── Flower of Life basic.ggcode    # Complex geometric pattern example - advanced features
│   ├── Flower of Life basic grid.ggcode # Grid-based pattern example - array usage
│   ├── Basic Square Pocket.g.gcode    # Generated G-code example - square pocket toolpath
│   ├── Flower of Life basic.g.gcode   # Generated pattern G-code - complex geometric output
│   ├── Flower of Life basic grid.g.gcode # Generated grid pattern G-code
│   ├── Flower of Life.g.gcode         # Generated flower pattern G-code
│   ├── Rose Pattern.g.gcode           # Generated rose pattern - mathematical curve toolpath
│   └── Spiral.g.gcode                 # Generated spiral toolpath - parametric motion example
│
├── 📁 GGcode_protocolware/            # 📚 COMPREHENSIVE LANGUAGE DOCUMENTATION
│   ├── 📁 commands/                   # G-code command reference documentation
│   │   ├── 📁 motion/                 # Motion command docs (G0, G1, G2, G3) - movement control
│   │   ├── 📁 spindle/                # Spindle control docs (M3, M5) - tool rotation control
│   │   ├── 📁 coolant/                # Coolant control docs (M7, M8, M9) - fluid system control
│   │   ├── 📁 tool/                   # Tool control documentation - tool changes and management
│   │   └── 📁 program/                # Program control docs - flow control and program structure
│   ├── 📁 functions/                  # Built-in function documentation and examples
│   │   ├── 📁 math/                   # Math functions (sin, cos, sqrt, abs, etc.) - mathematical operations
│   │   ├── 📁 geometry/               # Geometry functions (distance, hypot) - spatial calculations
│   │   └── 📁 utility/                # Utility functions - general purpose helper functions
│   ├── 📁 operations/                 # Operation reference documentation and syntax
│   │   ├── 📁 arithmetic/             # Arithmetic operations (+, -, *, /, mod) - mathematical operators
│   │   ├── 📁 comparison/             # Comparison operations (==, !=, <, >, <=, >=) - relational operators
│   │   ├── 📁 logical/                # Logical operations (&&, ||, !) - boolean logic operators
│   │   ├── 📁 conditional/            # Conditional operations (if, else, else if) - control flow
│   │   └── 📁 bitwise/                # Bitwise operations - bit manipulation (future feature)
│   ├── 📁 docs/                       # Additional documentation and guides
│   │   ├── quick_reference.md         # Quick syntax reference - cheat sheet for developers
│   │   └── syntax_reference.md        # Complete syntax guide - full language specification
│   ├── 📁 data/                       # Data files, examples, and reference materials
│   └── 📄 README.md                   # Protocolware documentation overview and navigation
│
├── 📁 node/                           # 🌐 NODE.JS INTEGRATION (legacy web interface)
│   └── libggcode.so                   # Shared library for Node.js web application interface
│
├── 📁 bin/                            # 🔧 BUILD ARTIFACTS AND TEST EXECUTABLES
│   ├── test_config_variables          # Configuration system test binary
│   ├── test_emitter                   # G-code emitter test binary
│   ├── test_evaluator                 # Expression evaluator test binary
│   ├── test_gcode_operations          # G-code operations test binary
│   ├── test_lexer                     # Lexical analyzer test binary
│   ├── test_memory_leaks              # Memory leak detection test binary
│   ├── test_missing_operations        # Missing operations test binary
│   ├── test_operation_combinations    # Operation combinations test binary
│   ├── test_output_buffer             # Output buffer test binary
│   ├── test_parser                    # Parser test binary
│   ├── test_security_buffer_overflow  # Security buffer overflow test binary
│   └── test_ternary                   # Ternary operator test binary
│
├── 📄 Makefile                        # 🛠️ BUILD SYSTEM & AUTOMATION - compilation rules and targets
├── 📄 README.md                       # 📖 Main project documentation - user guide and overview
├── 📄 PROJECT_STRUCTURE.md            # 🏗️ This comprehensive structure guide and architecture overview
├── 📄 LICENSE                         # ⚖️ MIT License - open source licensing terms
├── 📄 LICENSE-COMMERCIAL              # 💼 Commercial licensing terms - business and enterprise use
├── 📄 CODE_OF_CONDUCT.md              # 🤝 Community guidelines - behavior standards and expectations
├── 📄 CONTRIBUTING.md                 # 🤝 Contribution guidelines - development process and standards
├── 📄 SECURITY.md                     # 🔒 Security policy - vulnerability reporting and handling
├── 📄 .gitignore                      # Git ignore rules - excluded files and directories from version control
├── 📄 Doxyfile                        # Doxygen documentation configuration - API documentation generation
├── 📄 logo.png                        # Project logo and branding asset - visual identity
├── 📁 Gcode/                          # Generated G-code output directory (auto-created)
│   └── Flower of Life basic grid.g.gcode # Example generated G-code output
├── 📄 SECURITY.md                     # Security measures and input validation documentation
├── 📄 SECURITY_TEST_REPORT.md         # Comprehensive security test results and analysis
├── 📄 package.json                    # Node.js package information (legacy) - web interface dependencies
├── 📄 package-lock.json               # Node.js dependency lock file (legacy) - exact version specifications
├── 📄 g.sh                            # Build script - quick compilation helper for development
└── 📄 r.sh                            # Run script - quick execution helper for testing
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

#### **cli/** - Command Line Interface
- **Purpose:** Interactive file selection and argument parsing
- **Key Files:** `cli.c`, `cli.h`
- **Function:** Interactive menu, argument validation, help system

---

### **📁 tests/ - Test Suite**

#### **Unit Tests**
- **test_gcode_operations.c** - G-code generation tests
- **test_operation_combinations.c** - Complex operation tests
- **Unity framework** - Professional C testing framework

#### **Example Files**
- **Flower of Life basic.ggcode** - Complex geometric pattern
- **Flower of Life basic grid.ggcode** - Grid-based pattern
- Various example files for testing different language features

---

## 🚀 **Build System**

### **Makefile Targets:**

| Command | Description |
|---------|-------------|
| `make` or `make all` | Build main compiler binary |
| `make test` | Run all unit tests with summary |
| `make tests` | Compile test binaries only |
| `make clean` | Remove all build artifacts |
| `make win` | Cross-compile for Windows |

### **Build Process:**
1. **C Compiler:** `src/*.c` → `GGCODE/ggcode` (standalone binary)
2. **Tests:** `tests/*.c` → `bin/test_*` (test executables)
3. **Cross-platform:** Support for Linux, macOS, and Windows

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

### **Interactive CLI Flow:**
```
User runs ./GGCODE/ggcode
        ↓
   Interactive Menu
        ↓
   File Selection
        ↓
   Compilation
        ↓
   G-code Output
        ↓
   ./Gcode/*.g.gcode
```

---

## 🖥️ **Interactive CLI Features**

### **Main Components:**
1. **Interactive Menu** - Auto-discovery of .ggcode files
2. **File Selection** - Numbered options for easy selection
3. **Compile All** - Batch compilation option
4. **Smart Output** - Organized output to ./Gcode/ directory
5. **Direct Evaluation** - Test expressions without files
6. **Help System** - Comprehensive --help documentation

### **Key Technologies:**
- **Cross-platform** - Windows, macOS, Linux support
- **ANSI Colors** - Colorized output for better UX
- **Error Handling** - Comprehensive error reporting
- **Memory Management** - Safe allocation and cleanup

---

## 📊 **Project Statistics**

- **Lines of C Code:** ~15,000+ (core compiler)
- **CLI Module:** ~500+ lines (interactive interface)
- **Example Files:** Multiple .ggcode demonstration files
- **Test Coverage:** Comprehensive unit tests with Unity framework
- **Build Targets:** Linux, macOS, Windows
- **Dependencies:** None (standalone binary)

---

## 🎯 **Key Features by Component**

### **Core Compiler (C):**
- ✅ Variables and expressions
- ✅ Control flow (if/else, loops)
- ✅ Functions and recursion
- ✅ Arrays (1D, 2D, dynamic)
- ✅ Math library (sin, cos, sqrt, etc.)
- ✅ G-code modal behavior
- ✅ Professional G-code formatting
- ✅ Configurable precision control

### **Interactive CLI:**
- ✅ Auto-discovery of .ggcode files
- ✅ Interactive file selection menu
- ✅ Batch compilation options
- ✅ Direct expression evaluation
- ✅ Smart output path management
- ✅ Cross-platform compatibility
- ✅ Comprehensive help system
- ✅ Error handling and validation

---

## 🔧 **Development Workflow**

### **Adding New Features:**
1. **Core Feature:** Modify C code in `src/`
2. **Tests:** Add tests in `tests/`
3. **Documentation:** Update help content and guides
4. **Examples:** Add examples for demonstration

### **Building and Testing:**
```bash
# Build the compiler
make

# Run comprehensive tests
make test

# Interactive compilation
./GGCODE/ggcode

# Direct evaluation
./GGCODE/ggcode -e "G1 X10 Y20 F300"
```

---

## 🎯 **Key CLI Workflows**

### **Interactive Usage:**
1. **Default Mode** - `./GGCODE/ggcode` shows file selection menu
2. **File Selection** - Choose individual files or compile all
3. **Smart Output** - Automatic organization to ./Gcode/ directory
4. **Error Handling** - Clear error messages with line numbers

### **Command Line Usage:**
1. **Direct Compilation** - `./GGCODE/ggcode file.ggcode`
2. **Batch Processing** - `./GGCODE/ggcode -a` for all files
3. **Custom Output** - `./GGCODE/ggcode -o custom.gcode file.ggcode`
4. **Testing Mode** - `./GGCODE/ggcode -e "expression"` for quick tests

### **Professional Features:**
1. **Configurable Precision** - Control decimal places in output
2. **Line Numbering** - Professional G-code formatting
3. **Modal Behavior** - Efficient G-code generation
4. **Cross-platform** - Works on Linux, macOS, Windows

---

