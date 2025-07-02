<p align="center">
  <img src="logo.png" alt="GGcode Logo" width="320"/>
  <!-- For SVG support, you can use: <img src="logo.svg" alt="GGcode Logo" width="320"/> -->
</p>

# GGcode

**GGcode** is a custom G-code scripting language and compiler that brings programmability to CNC machining. Designed for automation, testing, and dynamic toolpath generation, it supports variables, control flow, expression evaluation, and runtime note/comment blocks.

---

## Features

- Lightweight compiler for `.gg` source files
- Full support for variables with `let`
- Arithmetic and logic expressions
- Conditional execution: `if`, `else`, `else if`
- Loops: `while`, `for`
- Dynamic G-code generation (`G1`, `M3`, etc.)
- Runtime `note {}` blocks with variable interpolation
- Human-readable debug output
- Clean, raw G-code output

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

Compile and run a `.gg` file:

```sh
./ggcode path/to/file.gg
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