# Circular Motion Commands (G2, G3)

## G2 - Clockwise Circular Interpolation
**GGCODE Syntax**: `G[2] X[value] Y[value] I[value] J[value] F[feedrate]`

**Description**: Creates a clockwise circular arc from current position to specified endpoint.

**Traditional vs GGCODE**:
```
Traditional G-code: G2 X10 Y10 I5 J0 F500
GGCODE Format:      G[2] X[10] Y[10] I[5] J[0] F[500]
```

**Parameters**:
- `G[2]`: Clockwise circular interpolation command
- `X[value]`: X-axis endpoint coordinate
- `Y[value]`: Y-axis endpoint coordinate
- `I[value]`: X-axis offset from start point to arc center
- `J[value]`: Y-axis offset from start point to arc center
- `F[value]`: Feedrate

**Examples**:
```ggcode
// Quarter circle clockwise
G[2] X[10] Y[10] I[5] J[0] F[500]

// Semi-circle with variables
let radius = 8
G[2] X[radius * 2] Y[0] I[radius] J[0] F[300]

// Arc with center offset
G[2] X[15] Y[5] I[2.5] J[2.5] F[400]
```

---

## G3 - Counter-Clockwise Circular Interpolation
**GGCODE Syntax**: `G[3] X[value] Y[value] I[value] J[value] F[feedrate]`

**Description**: Creates a counter-clockwise circular arc from current position to specified endpoint.

**Traditional vs GGCODE**:
```
Traditional G-code: G3 X10 Y10 I5 J0 F500
GGCODE Format:      G[3] X[10] Y[10] I[5] J[0] F[500]
```

**Parameters**:
- `G[3]`: Counter-clockwise circular interpolation command
- `X[value]`: X-axis endpoint coordinate
- `Y[value]`: Y-axis endpoint coordinate
- `I[value]`: X-axis offset from start point to arc center
- `J[value]`: Y-axis offset from start point to arc center
- `F[value]`: Feedrate

**Examples**:
```ggcode
// Quarter circle counter-clockwise
G[3] X[10] Y[10] I[0] J[5] F[500]

// Full circle (return to start point)
G[3] X[0] Y[0] I[-5] J[0] F[300]

// Arc with mathematical center calculation
let center_x = 5
let center_y = 5
G[3] X[10] Y[0] I[center_x] J[center_y] F[400]
```

---

## Alternative Radius Format
Both G2 and G3 can use radius (R) instead of I,J:

**GGCODE Syntax**: `G[2/3] X[value] Y[value] R[radius] F[feedrate]`

**Examples**:
```ggcode
// Clockwise arc with radius
G[2] X[10] Y[10] R[7.07] F[500]

// Counter-clockwise arc with radius
G[3] X[0] Y[20] R[10] F[300]

// Variable radius
let arc_radius = 12.5
G[2] X[25] Y[0] R[arc_radius] F[400]
```

---

## Complete Circular Motion Example
```ggcode
// Create a circular pocket
let center_x = 25
let center_y = 25
let pocket_radius = 10
let feed_rate = 500

// Move to start position
G[0] X[center_x - pocket_radius] Y[center_y] Z[5]

// Plunge to cutting depth
G[1] Z[-2] F[100]

// Cut full circle counter-clockwise
G[3] X[center_x - pocket_radius] Y[center_y] I[pocket_radius] J[0] F[feed_rate]

// Retract
G[0] Z[5]
```

## Generated G-code Examples
**GGCODE Input**:
```ggcode
G[2] X[10] Y[10] I[5] J[0] F[500]
```

**Generated G-code Output**:
```gcode
G2 X10.000 Y10.000 I5.000 J0.000 F500.000
```