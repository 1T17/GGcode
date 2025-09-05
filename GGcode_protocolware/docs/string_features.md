# String Features in GGCODE

## Overview
GGCODE now supports string variables, comparison, and iteration, enabling more descriptive and flexible CNC programming.

## String Variable Declaration
```ggcode
let tool_name = "1/4 inch end mill"
let operation = "pocket milling"
let material = "6061 aluminum"
let programmer = "John Doe"
```

## String Usage in Notes
Strings can be interpolated in `note` statements using both `[variable]` and `{variable}` syntax:

```ggcode
let tool = "carbide end mill"
note {Using tool: [tool]}
note {Tool description: {tool}}
```

## String Comparison
Use strings in conditional statements for program logic:

```ggcode
let material = "aluminum"

if material == "aluminum" {
    let spindle_speed = 3000
    let feed_rate = 800
    note {Aluminum parameters loaded}
} else if material == "steel" {
    let spindle_speed = 1500
    let feed_rate = 400
    note {Steel parameters loaded}
}
```

## String Iteration

### Basic Character Iteration
```ggcode
let message = "HELLO"
for char in message {
    note {Character: [char]}
}
// Output: H, E, L, L, O
```

### Character Iteration with Index
```ggcode
let word = "CNC"
for (char, index) in word {
    note {Position [index]: [char]}
}
// Output: Position 0: C, Position 1: N, Position 2: C
```

## Practical Examples

### Tool Management
```ggcode
let current_tool = "1/8 inch ball end mill"
let required_tool = "1/4 inch end mill"

if current_tool != required_tool {
    note {Tool change required}
    note {Current: [current_tool]}
    note {Required: [required_tool]}
    T[2]
    M[6]
}
```

### Operation Documentation
```ggcode
let part_number = "PN-12345"
let operation_sequence = "ROUGH-SEMI-FINISH"

note {Part: [part_number]}
note {Operations to perform:}
for (op_char, step) in operation_sequence {
    if op_char != "-" {
        let step_number = step + 1
        note {Step [step_number]: [op_char]}
    }
}
```

### Material-Specific Parameters
```ggcode
function setup_material_params(material_type) {
    if material_type == "aluminum" {
        let spindle_rpm = 3000
        let feed_ipm = 800
        let coolant = "flood"
    } else if material_type == "steel" {
        let spindle_rpm = 1500
        let feed_ipm = 400
        let coolant = "mist"
    }
    
    note {Material: [material_type]}
    note {Spindle: [spindle_rpm] RPM}
    note {Feed: [feed_ipm] IPM}
    note {Coolant: [coolant]}
    
    return spindle_rpm
}

let workpiece_material = "aluminum"
let rpm = setup_material_params(workpiece_material)
M[3] S[rpm]
```

## Advanced String Operations

### Nested String Loops
```ggcode
let axes = "XYZ"
let coordinates = "123"

note {Coordinate combinations:}
for axis in axes {
    for coord in coordinates {
        note {[axis][coord]}
    }
}
// Output: X1, X2, X3, Y1, Y2, Y3, Z1, Z2, Z3
```

### Conditional String Processing
```ggcode
let program_name = "POCKET_MILL_ALUMINUM"
let vowels = "AEIOU"

note {Analyzing program name: [program_name]}
for (char, pos) in program_name {
    for vowel in vowels {
        if char == vowel {
            note {Vowel '[char]' found at position [pos]}
        }
    }
}
```

## Best Practices

1. **Use descriptive string names**: `tool_description` instead of `t`
2. **Document operations**: Use strings to explain what the program does
3. **Material-specific logic**: Use string comparison for material-dependent parameters
4. **Tool identification**: Use strings to identify and verify tools
5. **Error messages**: Use strings to provide clear error descriptions

## Integration with Existing Features

String features work seamlessly with:
- **Variables**: String and numeric variables coexist
- **Functions**: Strings can be passed as function parameters
- **Loops**: String iteration works with nested loops
- **Conditionals**: String comparison in if/else statements
- **G-code generation**: Strings document the generated G-code

## Output Example

**GGCODE Input:**
```ggcode
let tool = "end mill"
let material = "aluminum"
note {Using [tool] on [material]}
for char in tool {
    note {[char]}
}
```

**Generated Output:**
```gcode
(Using end mill on aluminum)
(e)
(n)
(d)
( )
(m)
(i)
(l)
(l)
```

String features make GGCODE programs more readable, maintainable, and self-documenting while preserving full compatibility with existing numeric operations and G-code generation.