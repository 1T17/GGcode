# Compound Assignment Operations

## Addition Assignment (+=)
**Syntax**: `variable += expression`

**Description**: Adds the expression value to the variable and stores the result.

**Examples**:
```ggcode
let x = 10
x += 5          // x becomes 15
x += 2 * 3      // x becomes 21

let position = 0
position += 10  // Move 10 units
G[1] X[position] Y[0] F[500]    // Note: square brackets required
```

---

## Subtraction Assignment (-=)
**Syntax**: `variable -= expression`

**Description**: Subtracts the expression value from the variable.

**Examples**:
```ggcode
let y = 50
y -= 15         // y becomes 35
y -= 5 * 2      // y becomes 25

let depth = 0
depth -= 2      // Go down 2 units
G[1] Z[depth] F[100]            // Note: square brackets required
```

---

## Multiplication Assignment (*=)
**Syntax**: `variable *= expression`

**Description**: Multiplies the variable by the expression value.

**Examples**:
```ggcode
let scale = 2
scale *= 3      // scale becomes 6
scale *= 1.5    // scale becomes 9

let radius = 5
radius *= 2     // Double the radius
G[2] X[radius * 2] Y[0] I[radius] J[0] F[500]  // Note: square brackets required
```

---

## Division Assignment (/=)
**Syntax**: `variable /= expression`

**Description**: Divides the variable by the expression value.

**Examples**:
```ggcode
let distance = 100
distance /= 4   // distance becomes 25
distance /= 2.5 // distance becomes 10

let feedrate = 1000
feedrate /= 2   // Half the feedrate
G[1] X[10] Y[10] F[feedrate]    // Note: square brackets required
```

---

## Exponentiation Assignment (^=)
**Syntax**: `variable ^= expression`

**Description**: Raises the variable to the power of the expression.

**Examples**:
```ggcode
let base = 3
base ^= 2       // base becomes 9
base ^= 0.5     // base becomes 3 (square root)

let size = 2
size ^= 3       // size becomes 8
G[1] X[size] Y[size] F[300]     // Note: square brackets required
```

---

## Practical Example
```ggcode
// Progressive scaling example
let x = 10
let y = 10
let scale = 1

// First position
G[1] X[x * scale] Y[y * scale] F[500]

// Scale up and move
scale *= 1.5
G[1] X[x * scale] Y[y * scale] F[500]

// Scale up again
scale *= 1.5
G[1] X[x * scale] Y[y * scale] F[500]
```
---


## Bitwise AND Assignment (&=)
**Syntax**: `variable &= expression`

**Description**: Performs bitwise AND operation between variable and expression.

**Examples**:
```ggcode
let flags = 12
flags &= 10     // flags becomes 8 (1100 & 1010 = 1000)

let settings = 15
settings &= 7   // settings becomes 7 (1111 & 0111 = 0111)
G[1] X[settings] Y[5] F[400]    // Uses result in G-code
```

---

## Bitwise OR Assignment (|=)
**Syntax**: `variable |= expression`

**Description**: Performs bitwise OR operation between variable and expression.

**Examples**:
```ggcode
let options = 5
options |= 8    // options becomes 13 (0101 | 1000 = 1101)

let config = 3
config |= 12    // config becomes 15 (0011 | 1100 = 1111)
M[3] S[config * 100] // Uses result for spindle speed
```

---

## Left Shift Assignment (<<=)
**Syntax**: `variable <<= expression`

**Description**: Shifts variable bits left by expression positions (equivalent to multiplying by 2^expression).

**Examples**:
```ggcode
let multiplier = 3
multiplier <<= 2    // multiplier becomes 12 (3 * 2^2 = 12)

let base_speed = 100
base_speed <<= 1    // base_speed becomes 200 (100 * 2)
M[3] S[base_speed]  // Spindle at 200 RPM
```

---

## Right Shift Assignment (>>=)
**Syntax**: `variable >>= expression`

**Description**: Shifts variable bits right by expression positions (equivalent to dividing by 2^expression).

**Examples**:
```ggcode
let divider = 32
divider >>= 2       // divider becomes 8 (32 / 2^2 = 8)

let high_feed = 800
high_feed >>= 1     // high_feed becomes 400 (800 / 2)
G[1] X[10] Y[10] F[high_feed]   // Feed at 400
```

---

## Complete Example with All Operators
```ggcode
// Demonstrate all compound assignment operators
let x = 10
let y = 20
let feed = 400
let spindle = 1000
let flags = 15

// Arithmetic assignments
x += 5          // x = 15
y -= 3          // y = 17
feed *= 1.5     // feed = 600
spindle /= 2    // spindle = 500
x ^= 2          // x = 225 (15^2)

// Bitwise assignments  
flags &= 7      // flags = 7 (1111 & 0111)
flags |= 8      // flags = 15 (0111 | 1000)
y <<= 1         // y = 34 (17 * 2)
feed >>= 1      // feed = 300 (600 / 2)

// Use results in G-code
G[0] X[0] Y[0] Z[10]
M[3] S[spindle]
G[1] X[x] Y[y] F[feed]
```