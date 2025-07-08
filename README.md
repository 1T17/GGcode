<p align="center">
  <img src="logo.png" alt="GGcode Logo" width="320"/>
  <!-- For SVG support, you can use: <img src="logo.svg" alt="GGcode Logo" width="320"/> -->
</p>

# GGcode

**GGcode** is a custom G-code scripting language and compiler that brings programmability to CNC machining. Designed for automation, testing, and dynamic toolpath generation, it supports variables, control flow, expression evaluation, and runtime note/comment blocks.

---

## Features

✨ Features

    🪶 Lightweight, embeddable compiler for .ggcode source files

    🔤 Human-readable syntax — like JavaScript but for G-code

    📥 Variables and assignments using let and dynamic evaluation

        Supports negative numbers, runtime expressions, and reassignments

    🧠 Full arithmetic + logical expression support

        +, -, *, /, mod, comparison operators, ! (not), and nested expressions

    🔁 Control flow constructs:

        if, else, else if with full expression logic

        while and for loops with scoped iteration and condition evaluation

    🧮 Built-in math function library

        abs, mod, floor, ceil, sqrt, hypot, distance, sin, cos, atan2, etc.

        Constants like PI, E, DEG_TO_RAD, TAU

    🧩 Functions

        Define reusable logic blocks with function keyword

        Supports recursive calls and nested scope resolution

    🗂️ Arrays

        1D and 2D arrays with full indexing: maze[y][x] = 1

        Array assignment, indexing, mutation, and dynamic size

    🔄 Full recursion-safe call tree

        Internal tree tracking for call structure

        Output as collapsible HTML structure with recursion detection

    📄 G-code generation support

        Emits raw G0, G1, M commands based on logic

        Math-based parametric paths and programmable toolpaths

    🗨️ note {} blocks

        Embedded runtime documentation, debug output, and comments

        Supports dynamic variable interpolation: note {Loop #[i]}


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