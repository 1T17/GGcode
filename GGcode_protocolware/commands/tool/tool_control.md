# Tool Control Commands

## T - Tool Selection
**GGCODE Syntax**: `T[tool_number]`

**Description**: Selects a tool from the tool changer.

**Traditional vs GGCODE**:
```
Traditional G-code: T1
GGCODE Format:      T[1]
```

**Parameters**:
- `T[value]`: Tool number to select

**Examples**:
```ggcode
T[1]                            // Select tool 1
T[5]                            // Select tool 5
T[tool_number]                  // Select tool from variable

// With variable
let end_mill_tool = 3
T[end_mill_tool]
```

---

## M6 - Tool Change
**GGCODE Syntax**: `M[6]`

**Description**: Executes automatic tool change (usually follows T command).

**Traditional vs GGCODE**:
```
Traditional G-code: M6
GGCODE Format:      M[6]
```

**Examples**:
```ggcode
T[2]                            // Select tool 2
M[6]                            // Execute tool change
```

---

## G43 - Tool Length Compensation
**GGCODE Syntax**: `G[43] H[offset_number] Z[position]`

**Description**: Applies tool length offset compensation.

**Traditional vs GGCODE**:
```
Traditional G-code: G43 H1 Z10
GGCODE Format:      G[43] H[1] Z[10]
```

**Parameters**:
- `G[43]`: Tool length compensation on
- `H[value]`: Offset register number (usually matches tool number)
- `Z[value]`: Z-axis position to move to after applying offset

**Examples**:
```ggcode
G[43] H[1] Z[10]                // Apply offset 1, move to Z10
G[43] H[tool_num] Z[5]          // Apply offset from variable
```

---

## G49 - Cancel Tool Length Compensation
**GGCODE Syntax**: `G[49]`

**Description**: Cancels tool length offset compensation.

**Traditional vs GGCODE**:
```
Traditional G-code: G49
GGCODE Format:      G[49]
```

**Examples**:
```ggcode
G[49]                           // Cancel tool length compensation
```

---

## Complete Tool Change Sequence
```ggcode
// Complete tool change example
let roughing_tool = 1
let finishing_tool = 2
let safe_z = 25

// Move to safe position
G[0] Z[safe_z]

// Select and change to roughing tool
T[roughing_tool]
M[6]

// Apply tool length compensation
G[43] H[roughing_tool] Z[safe_z]

// Start spindle for roughing
M[3] S[800]

// Roughing operations...
G[0] X[0] Y[0]
G[1] Z[-5] F[100]
G[1] X[50] Y[25] F[300]

// Tool change to finishing tool
G[0] Z[safe_z]
M[5]                            // Stop spindle
T[finishing_tool]
M[6]

// Apply new tool length compensation
G[43] H[finishing_tool] Z[safe_z]

// Start spindle for finishing
M[3] S[1500]

// Finishing operations...
G[1] Z[-5.2] F[50]
G[1] X[50] Y[25] F[200]

// End sequence
G[0] Z[safe_z]
M[5]
G[49]                           // Cancel tool compensation
```

## Tool Database Integration
```ggcode
// Tool definitions (could be loaded from database)
let tools = {
    roughing_endmill: 1,
    finishing_endmill: 2,
    drill_6mm: 3,
    tap_m6: 4
}

// Tool parameters
let tool_speeds = {
    1: 800,   // Roughing end mill
    2: 1500,  // Finishing end mill
    3: 600,   // 6mm drill
    4: 300    // M6 tap
}

// Use tools by name
let current_tool = tools.roughing_endmill
T[current_tool]
M[6]
G[43] H[current_tool] Z[25]
M[3] S[tool_speeds[current_tool]]
```

## Generated G-code Examples
**GGCODE Input**:
```ggcode
T[1]
M[6]
G[43] H[1] Z[25]
```

**Generated G-code Output**:
```gcode
T1
M6
G43 H1 Z25.000
```