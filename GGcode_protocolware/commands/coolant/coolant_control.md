# Coolant Control Commands

## M7 - Mist Coolant On
**GGCODE Syntax**: `M[7]`

**Description**: Turns on mist coolant system.

**Traditional vs GGCODE**:
```
Traditional G-code: M7
GGCODE Format:      M[7]
```

**Examples**:
```ggcode
M[7]                            // Turn on mist coolant
```

---

## M8 - Flood Coolant On
**GGCODE Syntax**: `M[8]`

**Description**: Turns on flood coolant system.

**Traditional vs GGCODE**:
```
Traditional G-code: M8
GGCODE Format:      M[8]
```

**Examples**:
```ggcode
M[8]                            // Turn on flood coolant
```

---

## M9 - Coolant Off
**GGCODE Syntax**: `M[9]`

**Description**: Turns off all coolant systems.

**Traditional vs GGCODE**:
```
Traditional G-code: M9
GGCODE Format:      M[9]
```

**Examples**:
```ggcode
M[9]                            // Turn off all coolant
```

---

## Complete Coolant Control Example
```ggcode
// Machining sequence with coolant control
let cutting_speed = 1200
let feed_rate = 500

// Setup - turn on flood coolant
M[8]

// Start spindle
M[3] S[cutting_speed]

// Cutting operations
G[0] X[0] Y[0] Z[5]
G[1] Z[-2] F[100]
G[1] X[50] Y[25] F[feed_rate]

// Finish - turn off coolant and spindle
M[9]
M[5]
```

## Conditional Coolant Control
```ggcode
// Use coolant based on material type
let material_type = 1  // 1=aluminum, 2=steel, 3=plastic

if material_type == 1 {
    M[8]  // Flood coolant for aluminum
} else if material_type == 2 {
    M[7]  // Mist coolant for steel
} else {
    // No coolant for plastic
}

// Machining operations here...

// Always turn off coolant at end
M[9]
```

## Generated G-code Examples
**GGCODE Input**:
```ggcode
M[8]
G[1] X[10] Y[10] F[500]
M[9]
```

**Generated G-code Output**:
```gcode
M8
G1 X10.000 Y10.000 F500.000
M9
```