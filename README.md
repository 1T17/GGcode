<p align="center">
  <img src="logo.png" alt="GGcode Logo" width="320"/>
  <!-- For SVG support, you can use: <img src="logo.svg" alt="GGcode Logo" width="320"/> -->
</p>

# GGcode

**GGcode** is a custom G-code scripting language and compiler that brings programmability to CNC machining. Designed for automation, testing, and dynamic toolpath generation, it supports variables, control flow, expression evaluation, and runtime note/comment blocks.

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

## Example

```gg
let id = 100
let x = 50
let y = 25

note {
    Program ID: [id]
    Starting cut at X[x] Y[y]
}

if x > 40 {
    G1 X[x] Y[y]
}

for i = 0..3 {
    G1 X[i*10] Y[0]
}
```

---

## Getting Started

### Requirements

- GCC or Clang
- `make`

### Build

Use the following commands to build and test the project:

| Command             | Description                                      |
|---------------------|--------------------------------------------------|
| `make` or `make all`| Build the main `ggcode` compiler binary          |
| `make tests`        | Build all test binaries in the `bin/` directory  |
| `make test`         | Run all tests and show a summary                 |
| `make clean`        | Remove build artifacts and test binaries         |

- The main binary will be named **`ggcode`** and generated in the root folder.
- Test binaries are built from files matching `tests/test_*.c`.
- Running `make test` will execute all tests and print a summary.

---

## Usage

Compile and run a `.ggcode` file:

```sh
./ggcode path/to/file.ggcode
```

**Example output:**
```
(EXPECT: X123 — should trigger)
N10 G1 X123
```

---

## Supported Syntax

| Keyword   | Description                        |
|-----------|------------------------------------|
| `let`     | Define variables                   |
| `if`      | Conditional execution              |
| `else if` | Chained conditional logic          |
| `else`    | Fallback execution                 |
| `while`   | Loop while condition is true       |
| `for`     | Range-based loop                   |
| `note`    | Insert runtime comments            |
| `G1`, `M3`| G-code statements with parameters  |

---


## 📜 License

This project is licensed under the **MIT License** for personal, educational, and small business use.

See:
- [LICENSE](./LICENSE) — Free use terms  
- [LICENSE-COMMERCIAL](./LICENSE-COMMERCIAL) — Commercial license info

To inquire about commercial licensing, contact: [t@doorbase.io](mailto:t@doorbase.io)

---

## Author

T1

> GGcode is built for control, clarity, and creativity — bringing logic to motion.