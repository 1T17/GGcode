# Complete Font Systems Guide - From DOS to Modern CNC

## Understanding Font Systems Logic

This guide explains different font rendering approaches, from classic DOS bitmap fonts to modern vector systems, with practical implementations in GGcode.

## 1. Historical Context: DOS Font Systems

### The Character Cell Concept
DOS computers used a **fixed character cell** system where each character occupied a rectangular grid:

```
Standard DOS Character: 8×16 pixels
┌────────┐
│████████│ ← Each row is 8 bits
│█      █│   stored as byte values
│█      █│
│████████│
│█      █│
│█      █│
│████████│
└────────┘
```

### Memory Layout
```c
// DOS font stored as byte arrays
unsigned char font_A[16] = {
    0x00,  // ........  (top padding)
    0x18,  // ...██...
    0x24,  // ..█..█..
    0x42,  // .█....█.
    0x42,  // .█....█.
    0x7E,  // .██████.  (crossbar)
    0x81,  // █......█
    0x81,  // █......█
    0x81,  // █......█
    0x00,  // ........  (bottom padding)
    // ... more rows
};
```

### Advantages of DOS Bitmap Fonts
- **Fast rendering**: Direct memory copy to screen
- **Pixel-perfect**: Exact control over every pixel
- **Consistent spacing**: Fixed character width
- **Low CPU usage**: No calculations needed

### Disadvantages
- **Fixed size**: Scaling creates pixelation
- **Memory intensive**: Large fonts need lots of storage
- **Limited resolution**: Bound by pixel grid

## 2. Modern Vector Font Systems

### Mathematical Representation
Vector fonts store characters as mathematical descriptions:

```
Character 'A' as vectors:
Point P1: (0, 0)     ← Bottom left
Point P2: (4, 8)     ← Top center  
Point P3: (8, 0)     ← Bottom right
Point P4: (2, 4)     ← Crossbar left
Point P5: (6, 4)     ← Crossbar right

Strokes:
1. Line P1 → P2 (left diagonal)
2. Line P2 → P3 (right diagonal)
3. Line P4 → P5 (crossbar)
```

### Scalability Mathematics
```c
// Scale character to any size
void scale_character(float scale_factor) {
    for (each_point in character) {
        point.x *= scale_factor;
        point.y *= scale_factor;
    }
}
```

## 3. Seven-Segment Display Logic

### Segment Mapping
```
Seven-segment layout:
 AAA     Segment bits:
F   B    A = bit 0
F   B    B = bit 1  
 GGG     C = bit 2
E   C    D = bit 3
E   C    E = bit 4
 DDD     F = bit 5
         G = bit 6
```

### Character Encoding Table
```c
// Seven-segment patterns (A=LSB, G=MSB)
unsigned char seven_seg_digits[10] = {
    0b00111111,  // 0: A,B,C,D,E,F (all except G)
    0b00000110,  // 1: B,C only
    0b01011011,  // 2: A,B,G,E,D
    0b01001111,  // 3: A,B,G,C,D
    0b01100110,  // 4: F,G,B,C
    0b01101101,  // 5: A,F,G,C,D
    0b01111101,  // 6: A,F,G,E,D,C
    0b00000111,  // 7: A,B,C
    0b01111111,  // 8: All segments
    0b01101111   // 9: A,B,C,D,F,G
};
```

## 4. Font System Comparison Matrix

| Feature | DOS Bitmap | Vector | Seven-Segment | Modern (TrueType) |
|---------|------------|--------|---------------|-------------------|
| **Storage** | Pixel arrays | Line segments | Bit patterns | Bezier curves |
| **Scalability** | Poor (pixelated) | Excellent | Good | Excellent |
| **Rendering Speed** | Very fast | Medium | Fast | Medium |
| **Memory Usage** | High | Low | Very low | Medium |
| **Character Quality** | Pixel-perfect | Smooth | Geometric | Professional |
| **Implementation** | Simple | Medium | Simple | Complex |
| **Best Use Case** | Retro displays | CNC/Plotting | Digital displays | Text documents |

## 5. CNC-Specific Considerations

### Tool Path Efficiency
```ggcode
// Efficient vector font approach
function draw_char_A(x, y, size) {
    // Single continuous path where possible
    G0 Z[safe_z] X[x-size/2] Y[y-size/2]  // Move to start
    G0 Z[0]                               // Lower tool
    G1 X[x] Y[y+size/2]                   // Left stroke
    G1 X[x+size/2] Y[y-size/2]            // Right stroke
    G0 Z[safe_z]                          // Lift tool
    
    // Separate crossbar (unavoidable lift)
    G0 X[x-size/4] Y[y]                   // Move to crossbar
    G0 Z[0]                               // Lower tool
    G1 X[x+size/4] Y[y]                   // Draw crossbar
    G0 Z[safe_z]                          // Lift tool
}
```

### Optimization Strategies
1. **Minimize tool lifts**: Group connected strokes
2. **Optimize travel paths**: Shortest distance between characters
3. **Single-stroke fonts**: Each line drawn only once
4. **Consistent tool depth**: Avoid unnecessary Z movements

## 6. Implementation Patterns

### DOS-Style Bitmap Renderer
```c
void render_bitmap_char(char c, int x, int y, int scale) {
    unsigned char *bitmap = get_char_bitmap(c);
    
    for (int row = 0; row < CHAR_HEIGHT; row++) {
        unsigned char line = bitmap[row];
        
        for (int col = 0; col < CHAR_WIDTH; col++) {
            if (line & (0x80 >> col)) {  // Test bit
                // Draw scaled pixel
                fill_rect(x + col*scale, y + row*scale, 
                         scale, scale);
            }
        }
    }
}
```

### Vector Font Renderer
```c
typedef struct {
    float x, y;
} Point;

typedef struct {
    Point start, end;
    int pen_down;  // 0=move, 1=draw
} Stroke;

void render_vector_char(char c, float x, float y, float scale) {
    Stroke *strokes = get_char_strokes(c);
    int stroke_count = get_stroke_count(c);
    
    for (int i = 0; i < stroke_count; i++) {
        Point start = {
            x + strokes[i].start.x * scale,
            y + strokes[i].start.y * scale
        };
        Point end = {
            x + strokes[i].end.x * scale,
            y + strokes[i].end.y * scale
        };
        
        if (strokes[i].pen_down) {
            draw_line(start.x, start.y, end.x, end.y);
        } else {
            move_to(end.x, end.y);
        }
    }
}
```

## 7. Character Set Design Principles

### Legibility Rules
1. **Consistent stroke width**: All lines same thickness
2. **Adequate spacing**: Characters don't touch
3. **Clear distinctions**: 0 vs O, 1 vs I vs l
4. **Proper proportions**: Width-to-height ratios

### Geometric Constraints
```
Standard character proportions:
- Width: 0.6 × Height (for most letters)
- Ascenders: 1.2 × x-height (b, d, f, h, k, l, t)
- Descenders: 0.3 × x-height (g, j, p, q, y)
- Cap height: 1.0 × x-height (A-Z)
- Stroke width: 0.1 × height
```

## 8. Advanced Font Features

### Kerning Tables
```c
// Adjust spacing between specific character pairs
int kerning_table[256][256] = {
    ['A']['V'] = -2,  // Move AV closer together
    ['T']['o'] = -1,  // Move To closer together
    ['W']['A'] = -1,  // Move WA closer together
    // ... more pairs
};
```

### Hinting for Small Sizes
```c
// Snap stems to pixel boundaries at small sizes
if (font_size < 12) {
    // Round stroke positions to nearest pixel
    stroke.x = round(stroke.x);
    stroke.y = round(stroke.y);
    
    // Ensure minimum stroke width
    if (stroke_width < 1.0) stroke_width = 1.0;
}
```

## 9. Performance Optimization

### Caching Strategies
```c
// Cache rendered characters for reuse
typedef struct {
    char character;
    float size;
    void *rendered_data;
} CacheEntry;

CacheEntry font_cache[256];

void *get_cached_char(char c, float size) {
    for (int i = 0; i < cache_size; i++) {
        if (font_cache[i].character == c && 
            font_cache[i].size == size) {
            return font_cache[i].rendered_data;
        }
    }
    return NULL;  // Not cached, need to render
}
```

### Memory Management
```c
// Efficient font storage
typedef struct {
    unsigned short offset;    // Offset into stroke data
    unsigned char count;      // Number of strokes
    unsigned char width;      // Character width
} CharInfo;

// Compact stroke storage
typedef struct {
    signed char dx, dy;       // Relative coordinates
    unsigned char flags;      // Pen up/down, end marker
} CompactStroke;
```

## 10. Practical Applications

### CNC Text Engraving
- Use vector fonts for smooth curves
- Optimize tool paths to minimize air time
- Consider material properties (wood vs metal)
- Single-stroke fonts for efficiency

### Embedded Displays
- Use bitmap fonts for speed
- Compress font data for memory savings
- Consider subpixel rendering for LCD
- Cache frequently used characters

### 3D Printing Text
- Vector fonts with extrusion
- Consider overhang limitations
- Optimize for layer adhesion
- Support material considerations

This comprehensive understanding enables you to choose the right font system for your specific application and implement it efficiently.