# Bitwise Operations

## Bitwise AND (&)
**Syntax**: `operand1 & operand2`

**Description**: Performs bitwise AND operation on integer representations.

**Examples**:
```ggcode
let result1 = 6 & 3     // 110 & 011 = 010 = 2
let result2 = 12 & 10   // 1100 & 1010 = 1000 = 8
let mask = 15 & 7       // 1111 & 0111 = 0111 = 7
```

**Use Cases**:
- Masking specific bits
- Checking if numbers share common bits
- Creating bit patterns for control

---

## Bitwise OR (|)
**Syntax**: `operand1 | operand2`

**Description**: Performs bitwise OR operation on integer representations.

**Examples**:
```ggcode
let result1 = 6 | 3     // 110 | 011 = 111 = 7
let result2 = 12 | 5    // 1100 | 0101 = 1101 = 13
let flags = 8 | 4 | 2   // Combine multiple flags
```

**Use Cases**:
- Setting specific bits
- Combining flags or options
- Creating composite values

---

## Left Shift (<<)
**Syntax**: `operand << shift_amount`

**Description**: Shifts bits to the left by specified positions.

**Examples**:
```ggcode
let result1 = 5 << 1    // 101 << 1 = 1010 = 10
let result2 = 3 << 2    // 11 << 2 = 1100 = 12
let power_of_2 = 1 << 4 // 1 << 4 = 16 (2^4)
```

**Use Cases**:
- Fast multiplication by powers of 2
- Bit positioning
- Creating masks

---

## Right Shift (>>)
**Syntax**: `operand >> shift_amount`

**Description**: Shifts bits to the right by specified positions.

**Examples**:
```ggcode
let result1 = 10 >> 1   // 1010 >> 1 = 101 = 5
let result2 = 16 >> 2   // 10000 >> 2 = 100 = 4
let half = 20 >> 1      // Fast division by 2
```

**Use Cases**:
- Fast division by powers of 2
- Extracting higher-order bits
- Scaling down values

---

## Compound Bitwise Assignments

### Bitwise AND Assignment (&=)
```ggcode
let flags = 15
flags &= 7      // flags becomes 7 (15 & 7)
```

### Bitwise OR Assignment (|=)
```ggcode
let options = 4
options |= 2    // options becomes 6 (4 | 2)
```

### Left Shift Assignment (<<=)
```ggcode
let value = 3
value <<= 2     // value becomes 12 (3 << 2)
```

### Right Shift Assignment (>>=)
```ggcode
let data = 32
data >>= 3      // data becomes 4 (32 >> 3)
```

---

## Practical Applications

### Creating Tool Numbers
```ggcode
let tool_type = 1       // End mill
let tool_size = 6       // 6mm
let tool_number = (tool_type << 4) | tool_size  // Combine into tool number
```

### Extracting Information
```ggcode
let encoded_data = 0x2A  // 42 in hex
let high_nibble = encoded_data >> 4    // Get upper 4 bits
let low_nibble = encoded_data & 15     // Get lower 4 bits (mask with 1111)
```

### Power-of-2 Operations
```ggcode
let grid_size = 1 << 3   // 8 (2^3)
let half_grid = grid_size >> 1  // 4

// Create grid pattern
for i from 0 to 4 {
    let x = i * grid_size
    let y = (i & 1) * grid_size  // Alternating pattern
    G1 X[x] Y[y]
}
```