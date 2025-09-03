# Program Control Commands

## G4 - Dwell/Pause
**GGCODE Syntax**: `G[4] P[seconds]` or `G[4] X[seconds]`

**Description**: Pauses program execution for specified time.

**Traditional vs GGCODE**:
```
Traditional G-code: G4 P2.5
GGCODE Format:      G[4] P[2.5]
```

**Parameters**:
- `G[4]`: Dwell command
- `P[value]`: Pause time in seconds
- `X[value]`: Alternative pause time parameter

**Examples**:
```ggcode
G[4] P[2]                       // Pause for 2 seconds
G[4] P[0.5]                     // Pause for half second
G[4] X[dwell_time]              // Pause using variable

// Wait for spindle to reach speed
M[3] S[1200]
G[4] P[3]                       // Wait 3 seconds
```

---

## M0 - Program Stop
**GGCODE Syntax**: `M[0]`

**Description**: Stops program execution and waits for operator to restart.

**Traditional vs GGCODE**:
```
Traditional G-code: M0
GGCODE Format:      M[0]
```

**Examples**:
```ggcode
M[0]                            // Stop and wait for operator
```

---

## M1 - Optional Stop
**GGCODE Syntax**: `M[1]`

**Description**: Conditional stop (only stops if optional stop is enabled on machine).

**Traditional vs GGCODE**:
```
Traditional G-code: M1
GGCODE Format:      M[1]
```

**Examples**:
```ggcode
M[1]                            // Optional stop
```

---

## M2 - Program End
**GGCODE Syntax**: `M[2]`

**Description**: Ends the program execution.

**Traditional vs GGCODE**:
```
Traditional G-code: M2
GGCODE Format:      M[2]
```

**Examples**:
```ggcode
M[2]                            // End program
```

---

## M30 - Program End and Rewind
**GGCODE Syntax**: `M[30]`

**Description**: Ends program and rewinds to beginning.

**Traditional vs GGCODE**:
```
Traditional G-code: M30
GGCODE Format:      M[30]
```

**Examples**:
```ggcode
M[30]                           // End program and rewind
```

---

## Complete Program Structure
```ggcode
// Program header
// Program: Sample Part v1.0
// Material: Aluminum 6061
// Tools: T1=6mm End Mill, T2=3mm End Mill

// Program initialization
G[90]                           // Absolute positioning
G[94]                           // Feed rate per minute
G[17]                           // XY plane selection

// Safety move
G[0] Z[25]

// Tool 1 - Roughing
T[1]
M[6]
G[43] H[1] Z[25]
M[3] S[800]
M[8]                            // Coolant on

// Optional stop for inspection
M[1]

// Roughing operations
G[0] X[5] Y[5]
G[1] Z[-5] F[100]
G[1] X[45] F[300]
G[1] Y[20]
G[1] X[5]
G[1] Y[5]
G[0] Z[25]

// Tool change to finishing tool
M[9]                            // Coolant off
M[5]                            // Spindle off
T[2]
M[6]
G[43] H[2] Z[25]
M[3] S[1500]
M[8]                            // Coolant on

// Finishing operations
G[0] X[5] Y[5]
G[1] Z[-5.2] F[50]
G[1] X[45] F[200]
G[1] Y[20]
G[1] X[5]
G[1] Y[5]

// Program end sequence
G[0] Z[25]
M[9]                            // Coolant off
M[5]                            // Spindle off
G[49]                           // Cancel tool compensation
G[0] X[0] Y[0]                  // Return to origin

M[30]                           // End program and rewind
```

## Conditional Program Control
```ggcode
// Conditional stops based on operation count
let operation_count = 0

while operation_count < 5 {
    // Machining operation
    G[1] X[operation_count * 10] Y[10] F[500]
    
    operation_count += 1
    
    // Optional stop every 3 operations
    if operation_count % 3 == 0 {
        M[1]
    }
}

// Final stop
M[0]
```

## Generated G-code Examples
**GGCODE Input**:
```ggcode
G[4] P[2.5]
M[1]
M[30]
```

**Generated G-code Output**:
```gcode
G4 P2.500
M1
M30
```