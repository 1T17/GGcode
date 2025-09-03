# Ternary Operator (?:)

The ternary operator provides a concise way to write conditional expressions in a single line. It's the only operator in GGcode that takes three operands.

## Syntax

```
condition ? value_if_true : value_if_false
```

## How it works

1. The `condition` is evaluated first
2. If the condition is **true** (non-zero), the `value_if_true` is returned
3. If the condition is **false** (zero), the `value_if_false` is returned

## Examples

### Basic Usage
```gcode
let max = a > b ? a : b          // Get maximum of two values
let sign = x >= 0 ? 1 : -1       // Get sign of a number
let abs_value = x >= 0 ? x : -x  // Absolute value
```

### With Variables
```gcode
let speed = 1000
let feed_rate = speed > 500 ? speed * 0.8 : speed * 1.2
```

### Nested Ternary (Chaining)
```gcode
// Grade system: A=4, B=3, C=2, D=1, F=0
let grade = score >= 90 ? 4 : score >= 80 ? 3 : score >= 70 ? 2 : score >= 60 ? 1 : 0
```

### With Function Calls
```gcode
let result = abs(x) > threshold ? sqrt(x) : pow(x, 2)
```

### In Expressions
```gcode
let total = (count > 0 ? sum / count : 0) + bonus
```

### G-code Applications
```gcode
// Conditional feed rates
let feed = material == 1 ? 800 : material == 2 ? 1200 : 600

// Conditional coordinates
let x_pos = direction > 0 ? start_x + distance : start_x - distance
let y_pos = mirror_y ? -current_y : current_y

G1 X[x_pos] Y[y_pos] F[feed]
```

## Precedence

The ternary operator has very low precedence, lower than most other operators:

```gcode
let result = 2 + 3 > 4 ? 1 * 2 : 3 + 4
// Equivalent to: (2 + 3) > 4 ? (1 * 2) : (3 + 4)
// Result: 5 > 4 ? 2 : 7 = 2
```

## Right Associativity

Ternary operators are right-associative, meaning they group from right to left:

```gcode
a ? b : c ? d : e
// Equivalent to: a ? b : (c ? d : e)
```

## Truth Values

- **True (non-zero)**: Any number except 0
- **False (zero)**: Only 0

```gcode
let a = 5 ? 100 : 200    // Result: 100 (5 is true)
let b = 0 ? 100 : 200    // Result: 200 (0 is false)
let c = -3 ? 100 : 200   // Result: 100 (-3 is true)
```

## Best Practices

### ✅ Good Uses
- Simple conditional assignments
- Choosing between two values
- Default value assignment
- Short conditional logic

### ❌ Avoid When
- Complex conditions that are hard to read
- Multiple statements needed in branches
- Deep nesting (more than 2-3 levels)
- When debugging might be difficult

### Example: Good vs Bad

**Good:**
```gcode
let speed = is_rough_cut ? 800 : 1200
let direction = clockwise ? 1 : -1
```

**Bad (too complex):**
```gcode
let result = (a > b && c < d) ? (complex_calc(a, b) + offset) : (another_calc(c, d) * factor + adjustment)
```

## Comparison with if-else

**Ternary operator:**
```gcode
let max = a > b ? a : b
```

**Equivalent if-else:**
```gcode
let max
if a > b {
    max = a
} else {
    max = b
}
```

The ternary operator is more concise for simple conditional assignments, while if-else is better for complex logic or multiple statements.