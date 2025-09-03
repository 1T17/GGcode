# Linear Motion Commands

## G0 - Rapid Positioning
**GGCODE Syntax**: `G[0] X[value] Y[value] Z[value] F[feedrate]`

**Description**: Moves the tool rapidly to the specified coordinates.

**Traditional vs GGCODE**:
```
Traditional G-code: G0 X10 Y20 Z5 F1000
GGCODE Format:      G[0] X[10] Y[20] Z[5] F[1000]
```

**Parameters**:
- `G[0]`: Rapid positioning command in square brackets
- `X[value]`: X-axis coordinate in square brackets
- `Y[value]`: Y-axis coordinate in square brackets  
- `Z[value]`: Z-axis coordinate in square brackets
- `F[value]`: Feedrate in square brackets (optional, inherited from previous)

**Examples**:
```ggcode
G[0] X[10] Y[20] Z[5] F[1000]     // Basic rapid move
G[0] X[0] Y[0]                    // Move to origin
G[0] Z[10]                        // Z-axis only move
G[0] X[25.5] Y[-10.2] Z[3.75]     // Decimal coordinates
```

**Generated G-code**:
```gcode
G0 X10.000 Y20.000 Z5.000 F1000.000
```

---

## G1 - Linear Interpolation
**GGCODE Syntax**: `G[1] X[value] Y[value] Z[value] F[feedrate]`

**Description**: Moves the tool in a straight line at the specified feedrate.

**Traditional vs GGCODE**:
```
Traditional G-code: G1 X15 Y40 Z-2 F500
GGCODE Format:      G[1] X[15] Y[40] Z[-2] F[500]
```

**Parameters**:
- `G[1]`: Linear interpolation command in square brackets
- `X[value]`: X-axis coordinate in square brackets
- `Y[value]`: Y-axis coordinate in square brackets
- `Z[value]`: Z-axis coordinate in square brackets  
- `F[value]`: Feedrate in square brackets

**Examples**:
```ggcode
G[1] X[10] Y[20] F[500]           // Basic linear move
G[1] Z[-2] F[100]                 // Z-axis plunge
G[1] X[10 + 5] Y[20 * 2] F[500]   // With expressions
G[1] X[sqrt(25)] Y[sin(30)] F[300] // With math functions
```

**Generated G-code**:
```gcode
G1 X15.000 Y40.000 Z5.000 F500.000
```

---

## Variable Integration
Motion commands support variable expressions:

```ggcode
let start_x = 10
let end_x = 50
let feed = 800

G0 X[start_x] Y0 Z5
G1 X[end_x] Y10 F[feed]
```