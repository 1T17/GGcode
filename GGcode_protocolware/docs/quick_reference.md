# GGCODE Quick Reference Card

## Essential Syntax Rule
**All numeric values must be in square brackets: `[value]`**

```
❌ Wrong:  G0 X10 Y20 F1000
✅ Right:  G[0] X[10] Y[20] F[1000]
```

## Motion Commands

| Traditional | GGCODE | Description |
|-------------|--------|-------------|
| `G0 X10 Y20 Z5` | `G[0] X[10] Y[20] Z[5]` | Rapid positioning |
| `G1 X15 Y25 F500` | `G[1] X[15] Y[25] F[500]` | Linear interpolation |
| `G2 X10 Y10 I5 J0 F300` | `G[2] X[10] Y[10] I[5] J[0] F[300]` | Clockwise arc |
| `G3 X10 Y10 I5 J0 F300` | `G[3] X[10] Y[10] I[5] J[0] F[300]` | Counter-clockwise arc |

## Spindle & Tool Commands

| Traditional | GGCODE | Description |
|-------------|--------|-------------|
| `M3 S1200` | `M[3] S[1200]` | Start spindle clockwise |
| `M4 S1000` | `M[4] S[1000]` | Start spindle counter-clockwise |
| `M5` | `M[5]` | Stop spindle |
| `T1` | `T[1]` | Select tool 1 |
| `M6` | `M[6]` | Tool change |

## Coolant Commands

| Traditional | GGCODE | Description |
|-------------|--------|-------------|
| `M7` | `M[7]` | Mist coolant on |
| `M8` | `M[8]` | Flood coolant on |
| `M9` | `M[9]` | Coolant off |

## Program Control

| Traditional | GGCODE | Description |
|-------------|--------|-------------|
| `G4 P2.5` | `G[4] P[2.5]` | Dwell 2.5 seconds |
| `M0` | `M[0]` | Program stop |
| `M1` | `M[1]` | Optional stop |
| `M30` | `M[30]` | Program end and rewind |

## Variables & Expressions

```ggcode
// Variable declaration
let x_pos = 10
let y_pos = 20
let feed_rate = 500

// Use in commands
G[0] X[x_pos] Y[y_pos] Z[5]
G[1] X[x_pos + 10] Y[y_pos] F[feed_rate]

// Mathematical expressions
G[1] X[sin(45) * 10] Y[cos(45) * 10] F[feed_rate * 1.5]
```

## Compound Assignments

```ggcode
let position = 10
position += 5       // position = 15
position *= 2       // position = 30
position /= 3       // position = 10

G[1] X[position] Y[20] F[400]
```

## Ternary Operator (Conditional)

```ggcode
// Basic syntax: condition ? value_if_true : value_if_false
let max_speed = material == 1 ? 1200 : 800
let direction = clockwise ? 1 : -1
let safe_z = depth > 10 ? 25 : 15

// Nested ternary for multiple conditions
let feed_rate = material == 1 ? 800 :     // wood
                material == 2 ? 600 :     // aluminum  
                material == 3 ? 400 : 500 // steel or default

// Use in G-code commands
G[1] X[x_pos] Y[mirror ? -y_pos : y_pos] F[feed_rate]
M[3] S[rough_cut ? 800 : 1200]
```

## Complete Program Template

```ggcode
// Program setup
let tool_number = 1
let spindle_speed = 1200
let feed_rate = 500
let safe_z = 25

// Tool change
G[0] Z[safe_z]
T[tool_number]
M[6]
G[43] H[tool_number] Z[safe_z]

// Start machining
M[3] S[spindle_speed]
M[8]  // Coolant on

// Machining operations
G[0] X[0] Y[0]
G[1] Z[-2] F[100]
G[1] X[50] F[feed_rate]
G[1] Y[25]
G[1] X[0]
G[1] Y[0]

// End program
G[0] Z[safe_z]
M[9]  // Coolant off
M[5]  // Spindle off
G[49] // Cancel tool compensation
M[30] // End and rewind
```

## Common Mistakes

❌ **Don't do this:**
```
G0 X10 Y20          // Missing brackets
G[1] X10 Y20        // Missing brackets on coordinates
M3 S1200            // Missing brackets
```

✅ **Do this:**
```ggcode
G[0] X[10] Y[20]    // Brackets on all values
G[1] X[10] Y[20] F[500]
M[3] S[1200]
```

## Generated Output

**GGCODE Input:**
```ggcode
G[0] X[10] Y[20] Z[5] F[1000]
G[1] X[15] Y[25] F[500]
```

**Generated G-code:**
```gcode
G0 X10.000 Y20.000 Z5.000 F1000.000
G1 X15.000 Y25.000 F500.000
```