# Trigonometric Functions

## sin(x) - Sine Function
**Syntax**: `sin(angle_in_radians)`

**Description**: Returns the sine of the angle in radians.

**Examples**:
```ggcode
let result1 = sin(0)        // 0
let result2 = sin(PI / 2)   // 1
let result3 = sin(PI)       // 0 (approximately)
let result4 = sin(30 * DEG_TO_RAD)  // 0.5 (30 degrees)
```

---

## cos(x) - Cosine Function
**Syntax**: `cos(angle_in_radians)`

**Description**: Returns the cosine of the angle in radians.

**Examples**:
```ggcode
let result1 = cos(0)        // 1
let result2 = cos(PI / 2)   // 0 (approximately)
let result3 = cos(PI)       // -1
let result4 = cos(60 * DEG_TO_RAD)  // 0.5 (60 degrees)
```

---

## tan(x) - Tangent Function
**Syntax**: `tan(angle_in_radians)`

**Description**: Returns the tangent of the angle in radians.

**Examples**:
```ggcode
let result1 = tan(0)        // 0
let result2 = tan(PI / 4)   // 1 (45 degrees)
let result3 = tan(45 * DEG_TO_RAD)  // 1
```

---

## asin(x) - Arcsine Function
**Syntax**: `asin(value)`

**Description**: Returns the arcsine (inverse sine) in radians. Input must be between -1 and 1.

**Examples**:
```ggcode
let angle1 = asin(0)        // 0
let angle2 = asin(1)        // PI/2
let angle3 = asin(0.5)      // PI/6 (30 degrees)
let degrees = asin(0.5) * RAD_TO_DEG  // 30
```

---

## acos(x) - Arccosine Function
**Syntax**: `acos(value)`

**Description**: Returns the arccosine (inverse cosine) in radians. Input must be between -1 and 1.

**Examples**:
```ggcode
let angle1 = acos(1)        // 0
let angle2 = acos(0)        // PI/2
let angle3 = acos(0.5)      // PI/3 (60 degrees)
```

---

## atan(x) - Arctangent Function
**Syntax**: `atan(value)`

**Description**: Returns the arctangent (inverse tangent) in radians.

**Examples**:
```ggcode
let angle1 = atan(0)        // 0
let angle2 = atan(1)        // PI/4 (45 degrees)
let angle3 = atan(sqrt(3))  // PI/3 (60 degrees)
```

---

## atan2(y, x) - Two-Argument Arctangent
**Syntax**: `atan2(y, x)`

**Description**: Returns the angle from the positive x-axis to the point (x, y).

**Examples**:
```ggcode
let angle1 = atan2(1, 1)    // PI/4 (45 degrees)
let angle2 = atan2(1, 0)    // PI/2 (90 degrees)
let angle3 = atan2(0, -1)   // PI (180 degrees)
```

---

## Practical Applications

### Circular Motion
```ggcode
let center_x = 50
let center_y = 50
let radius = 20
let num_points = 8

for i from 0 to num_points {
    let angle = i * TAU / num_points  // TAU = 2*PI
    let x = center_x + radius * cos(angle)
    let y = center_y + radius * sin(angle)
    G1 X[x] Y[y] F300
}
```

### Spiral Pattern
```ggcode
let center_x = 25
let center_y = 25
let max_radius = 15
let turns = 3

for i from 0 to 100 {
    let t = i / 100.0
    let angle = t * turns * TAU
    let radius = t * max_radius
    let x = center_x + radius * cos(angle)
    let y = center_y + radius * sin(angle)
    G1 X[x] Y[y] F200
}
```

### Angle Calculations
```ggcode
// Calculate angle between two points
let dx = 30 - 10  // Point 2 - Point 1
let dy = 40 - 20
let angle_rad = atan2(dy, dx)
let angle_deg = angle_rad * RAD_TO_DEG

// Use angle for tool orientation
G1 X30 Y40 A[angle_deg]
```