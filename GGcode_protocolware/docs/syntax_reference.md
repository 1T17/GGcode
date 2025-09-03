# GGCODE Syntax Reference

## Core Syntax Rules

### Square Bracket Notation
GGCODE uses square brackets `[]` around all numeric values and expressions, unlike traditional G-code.

**Traditional G-code vs GGCODE:**
```
Traditional: G0 X10 Y20 Z5 F1000
GGCODE:      G[0] X[10] Y[20] Z[5] F[1000]

Traditional: G1 X15.5 Y-10 F500
GGCODE:      G[1] X[15.5] Y[-10] F[500]

Traditional: M3 S1200
GGCODE:      M[3] S[1200]
```

### Variable Integration
Variables can be used inside square brackets:

```ggcode
let x_pos = 10
let y_pos = 20
let speed = 1000

G[0] X[x_pos] Y[y_pos] F[speed]
```

### Expression Support
Mathematical expressions are evaluated inside brackets:

```ggcode
G[1] X[10 + 5] Y[20 * 2] F[500]        // X=15, Y=40
G[0] X[sin(45) * 10] Y[cos(45) * 10]   // Trigonometric positioning
G[1] Z[sqrt(25)] F[100]                // Z=5
```

### Command Structure
Every GGCODE command follows this pattern:

```
COMMAND[value] PARAMETER[value] PARAMETER[value] ...
```

Examples:
- `G[0]` - G-command with value 0
- `X[10]` - X parameter with value 10
- `F[speed]` - F parameter with variable 'speed'
- `S[rpm * 2]` - S parameter with expression

## Data Types

### Numbers
```ggcode
G[0] X[10]      // Integer
G[1] Y[15.5]    // Decimal
G[2] Z[-5]      // Negative
```

### Variables
```ggcode
let position = 25
G[0] X[position]
```

### Expressions
```ggcode
G[1] X[10 + 5]           // Arithmetic
G[0] Y[radius * 2]       // With variables
G[1] Z[sin(angle) * 10]  // With functions
```

## Comments
```ggcode
// Single line comment
G[0] X[10] Y[20]  // End of line comment

/* Multi-line
   comment block */
```

## Complete Example
```ggcode
// GGCODE example showing proper syntax
let start_x = 0
let start_y = 0
let end_x = 50
let end_y = 25
let feed_rate = 800
let spindle_speed = 1200

// Rapid move to start position
G[0] X[start_x] Y[start_y] Z[5] F[1000]

// Start spindle
M[3] S[spindle_speed]

// Linear cut to end position
G[1] X[end_x] Y[end_y] Z[-2] F[feed_rate]

// Rapid retract
G[0] Z[10]

// Stop spindle
M[5]
```

## Error Examples
❌ **Wrong - Traditional G-code syntax:**
```
G0 X10 Y20 Z5 F1000
G1 X15 Y25 F500
M3 S1200
```

✅ **Correct - GGCODE syntax:**
```ggcode
G[0] X[10] Y[20] Z[5] F[1000]
G[1] X[15] Y[25] F[500]
M[3] S[1200]
```