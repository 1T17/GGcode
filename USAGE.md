# GGcode Usage

## Interactive Mode (Default)

```bash
ggcode              # If installed globally
./GGCODE/ggcode     # Local binary
```

Shows menu with auto-discovered `.ggcode` files:
```
Found 3 .ggcode files:
  1) part1.ggcode
  2) part2.ggcode  
  3) complex_pattern.ggcode
  4) Compile all files
  h) Show help
  0) Exit

Select option (0-4, h): 
```

## Command Line

```bash
# Compile specific files
ggcode part.ggcode
ggcode file1.ggcode file2.ggcode

# Compile all .ggcode files
ggcode -a

# Custom output
ggcode -o custom.gcode part.ggcode
ggcode --output-dir ./build *.ggcode

# Test expressions directly
ggcode -e "G1 X10 Y20 F300"
ggcode -e "for i=1..5 { G1 X[i*10] Y0 }"

# Options
ggcode -q -a        # Quiet mode
ggcode --help       # Show help
ggcode --version    # Show version
```

## Global Installation

After building with `make`, you'll be prompted to install globally. This allows you to use `ggcode` from any directory instead of `./GGCODE/ggcode`.

**Manual installation:**
```bash
make install    # Interactive installation menu
make uninstall  # Remove global installation
```

## Basic Syntax

**Variables:**
```ggcode
let x = 10
let feed = 300
G1 X[x] Y20 F[feed]
```

**Arrays:**
```ggcode
let points = [10, 20, 30]
G1 X[points[0]] Y[points[1]]
```

**Loops:**
```ggcode
for i = 1..5 {
    G1 X[i*10] Y0 F300
}
```

**Functions:**
```ggcode
function circle(radius, steps) {
    for i = 0..steps {
        let angle = i * (360 / steps) * DEG_TO_RAD
        G1 X[radius * cos(angle)] Y[radius * sin(angle)]
    }
}
circle(10, 36)
```

**Conditionals:**
```ggcode
let material = 1  // 1=aluminum, 2=steel
let speed = material == 1 ? 8000 : 6000
M3 S[speed]
```

**Math & Constants:**
```ggcode
let angle = 45 * DEG_TO_RAD   // PI, E, TAU, DEG_TO_RAD
let dist = sqrt(x*x + y*y)    // abs, sin, cos, tan, sqrt
```

**Output Control:**
```ggcode
let decimalpoint = 2  // Control decimal places (0-6)
let nline = 0         // Disable line numbering
```

## Output

- Input: `part.ggcode`
- Output: `part.g.gcode` (same directory)
- Format: Professional G-code with line numbers and modal behavior

## Examples

Check `GGCODE/` directory for example files:
- `Flower of Life basic.ggcode` - Complex patterns
- `array test.ggcode` - Array usage
- Generated `.g.gcode` files show expected output