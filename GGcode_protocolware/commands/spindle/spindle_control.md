# Spindle Control Commands

## M3 - Start Spindle Clockwise
**GGCODE Syntax**: `M[3] S[speed]`

**Description**: Starts the spindle rotating clockwise at specified RPM.

**Traditional vs GGCODE**:
```
Traditional G-code: M3 S1200
GGCODE Format:      M[3] S[1200]
```

**Parameters**:
- `M[3]`: Start spindle clockwise command
- `S[value]`: Spindle speed in RPM

**Examples**:
```ggcode
M[3] S[1200]                    // Start at 1200 RPM
M[3] S[2500]                    // High speed for finishing
M[3] S[800]                     // Lower speed for roughing

// With variables
let cutting_speed = 1500
M[3] S[cutting_speed]

// With expressions
let material_factor = 0.8
M[3] S[2000 * material_factor]  // S=1600
```

---

## M4 - Start Spindle Counter-Clockwise
**GGCODE Syntax**: `M[4] S[speed]`

**Description**: Starts the spindle rotating counter-clockwise at specified RPM.

**Traditional vs GGCODE**:
```
Traditional G-code: M4 S1000
GGCODE Format:      M[4] S[1000]
```

**Parameters**:
- `M[4]`: Start spindle counter-clockwise command
- `S[value]`: Spindle speed in RPM

**Examples**:
```ggcode
M[4] S[1000]                    // Counter-clockwise at 1000 RPM
M[4] S[1800]                    // For tapping operations

// Variable speed
let tap_speed = 500
M[4] S[tap_speed]
```

---

## M5 - Stop Spindle
**GGCODE Syntax**: `M[5]`

**Description**: Stops the spindle rotation.

**Traditional vs GGCODE**:
```
Traditional G-code: M5
GGCODE Format:      M[5]
```

**Examples**:
```ggcode
M[5]                            // Stop spindle
```

---

## S - Spindle Speed Setting
**GGCODE Syntax**: `S[speed]`

**Description**: Sets spindle speed without starting it (used with G-codes that auto-start spindle).

**Traditional vs GGCODE**:
```
Traditional G-code: S1500
GGCODE Format:      S[1500]
```

**Examples**:
```ggcode
S[1200]                         // Set speed to 1200 RPM
S[rpm_variable]                 // Set speed from variable
S[base_speed * 1.5]             // Set speed with calculation
```

---

## Complete Spindle Control Example
```ggcode
// Spindle control sequence
let roughing_speed = 800
let finishing_speed = 1500
let feed_rate = 300

// Start spindle for roughing
M[3] S[roughing_speed]

// Wait for spindle to reach speed (if needed)
G[4] P[2]  // Dwell 2 seconds

// Roughing pass
G[1] X[50] Y[25] Z[-5] F[feed_rate]

// Change to finishing speed
S[finishing_speed]

// Finishing pass
G[1] X[50] Y[25] Z[-5.5] F[feed_rate / 2]

// Stop spindle
M[5]
```

## Spindle Speed Calculations
```ggcode
// Calculate spindle speed based on material and tool
let tool_diameter = 6.35        // 1/4 inch end mill
let surface_speed = 100         // Surface feet per minute
let pi = 3.14159

// Calculate RPM: (Surface Speed * 12) / (π * Tool Diameter)
let calculated_rpm = (surface_speed * 12) / (pi * tool_diameter)

M[3] S[calculated_rpm]
```

## Generated G-code Examples
**GGCODE Input**:
```ggcode
M[3] S[1200]
G[1] X[10] Y[10] F[500]
M[5]
```

**Generated G-code Output**:
```gcode
M3 S1200.000
G1 X10.000 Y10.000 F500.000
M5
```