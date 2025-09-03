# GGcode Protocolware

A comprehensive breakdown of the GGcode language protocol, organized by functionality.

## Core GGCODE Syntax

### Square Bracket Notation
GGCODE uses square brackets `[]` around all numeric values, unlike traditional G-code:

```
Traditional G-code: G0 X10 Y20 Z5 F1000
GGCODE Format:      G[0] X[10] Y[20] Z[5] F[1000]
```

**Key Rule**: All G-code commands must use square brackets around values!

## Structure

```
GGcode_protocolware/
├── commands/          # G-code command implementations
│   ├── motion/        # Movement commands (G0, G1, G2, G3, etc.)
│   ├── spindle/       # Spindle control (M3, M4, M5, S)
│   ├── coolant/       # Coolant control (M7, M8, M9)
│   ├── tool/          # Tool management (T, M6)
│   └── program/       # Program control (M0, M1, M2, M30)
├── operations/        # Mathematical and logical operations
│   ├── arithmetic/    # +, -, *, /, ^, compound assignments
│   ├── bitwise/       # &, |, <<, >>, compound assignments
│   ├── comparison/    # ==, !=, <, >, <=, >=
│   └── logical/       # &&, ||, !
├── functions/         # Built-in functions
│   ├── math/          # sin, cos, tan, sqrt, abs, etc.
│   ├── geometry/      # Distance, angle calculations
│   └── utility/       # Type conversion, formatting
├── data/              # Data structures and constants
└── docs/              # Documentation and examples
```

## Core Concepts

- **Variables**: `let x = value`
- **Square Bracket Syntax**: `G[0] X[10] Y[20] F[500]`
- **Variable Integration**: `G[1] X[position] Y[height] F[speed]`
- **Expressions**: `G[1] X[10 + 5] Y[sin(45) * 10] F[feed_rate * 1.5]`
- **G-code Output**: Generates standard G-code from GGCODE syntax
- **Control Flow**: Loops, conditionals, functions
- **Built-in Functions**: Mathematical and utility functions

## Quick Examples

### Basic Commands
```ggcode
// Traditional vs GGCODE
// G0 X10 Y20 Z5 F1000    ← Traditional
G[0] X[10] Y[20] Z[5] F[1000]  // ← GGCODE

// M3 S1200              ← Traditional  
M[3] S[1200]                   // ← GGCODE
```

### With Variables
```ggcode
let x_pos = 25
let y_pos = 15
let feed_rate = 500

G[0] X[x_pos] Y[y_pos] Z[5]
G[1] X[x_pos + 10] Y[y_pos] F[feed_rate]
```

### With Expressions
```ggcode
let radius = 8
G[2] X[radius * 2] Y[0] I[radius] J[0] F[300]
```