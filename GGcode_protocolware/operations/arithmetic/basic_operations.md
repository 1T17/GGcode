# Basic Arithmetic Operations

## Addition (+)
**Syntax**: `operand1 + operand2`

**Description**: Adds two numbers together.

**Examples**:
```ggcode
let sum = 10 + 5        // Result: 15
let x = 2.5 + 3.7       // Result: 6.2
G1 X[10 + 5] Y20        // X becomes 15
```

---

## Subtraction (-)
**Syntax**: `operand1 - operand2`

**Description**: Subtracts the second operand from the first.

**Examples**:
```ggcode
let diff = 20 - 8       // Result: 12
let y = 15.5 - 2.3      // Result: 13.2
G1 X10 Y[30 - 5]        // Y becomes 25
```

---

## Multiplication (*)
**Syntax**: `operand1 * operand2`

**Description**: Multiplies two numbers.

**Examples**:
```ggcode
let product = 6 * 7     // Result: 42
let area = 3.14 * 5 * 5 // Result: 78.5
G1 X[4 * 5] Y[3 * 8]    // X=20, Y=24
```

---

## Division (/)
**Syntax**: `operand1 / operand2`

**Description**: Divides the first operand by the second.

**Examples**:
```ggcode
let quotient = 15 / 3   // Result: 5
let half = 10 / 2       // Result: 5
G1 X[100 / 4] Y[60 / 3] // X=25, Y=20
```

---

## Exponentiation (^)
**Syntax**: `base ^ exponent`

**Description**: Raises base to the power of exponent.

**Examples**:
```ggcode
let square = 5 ^ 2      // Result: 25
let cube = 3 ^ 3        // Result: 27
let root = 16 ^ 0.5     // Result: 4 (square root)
G1 X[2 ^ 4] Y[3 ^ 2]    // X=16, Y=9
```

---

## Operator Precedence
1. `^` (Exponentiation) - Right associative
2. `*`, `/` (Multiplication, Division) - Left associative
3. `+`, `-` (Addition, Subtraction) - Left associative

**Examples**:
```ggcode
let result1 = 2 + 3 * 4     // Result: 14 (not 20)
let result2 = 2 ^ 3 ^ 2     // Result: 512 (2^(3^2), not (2^3)^2)
let result3 = (2 + 3) * 4   // Result: 20 (parentheses override)
```