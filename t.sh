#!/bin/bash

# Wait 5 seconds so you can focus on the window
sleep 5

# Text to type
text="


// // GGcode test file
let id = 2345
// --- Setup ---
let radius     = 5
let z_safe      = 1
let z_cut       = -1
let fed       = 300
let step_deg    = 3
let deg_to_rad = DEG_TO_RAD
let round_eps  = 0.0001
let feed = 200
let ring_scale = 10


let spacing_x   = radius
let spacing_y   = radius * sqrt(3) / 2
let outer_r     = radius * 2.5
G90 G94 G17
G20
G53 G0 Z0
T1 M6
S6000 M3
G54
M8
G0 Z[z_safe]

// -- Helpers ---

function move_safe_z() {G0 Z[z_safe]}
function plunge_cut() {G0 Z[z_cut] F[feed]}
function travel_to(x, y) {G0 X[x] Y[y]}

function draw_full_circle(r) {
let i = 0
while (i <= 360) {
let angle = i * deg_to_rad
let x = r * cos(angle)
let y = r * sin(angle)
if (abs(x) < round_eps) { x = 0 }
if (abs(y) < round_eps) { y = 0 }
G1 X[x] Y[y] F[feed]
i = i + step_deg}}

function draw_cliped_circle(ox, oy) {
let drawing = 0
let was_inside = 0
let point_count = 0
let first_x = 0
let first_y = 0
let prev_x = 99999
let prev_y = 99999
let i = 0
while (i <= 360) {
let angle = i * deg_to_rad
let raw_x = ox + radius * cos(angle)
let raw_y = oy + radius * sin(angle)
let d2 = raw_x * raw_x + raw_y * raw_y
if (abs(d2 - outer_r * outer_r) < round_eps) {
let scale = outer_r / sqrt(d2)
raw_x = raw_x * scale
raw_y = raw_y * scale }
let inside = (raw_x * raw_x + raw_y * raw_y <= outer_r * outer_r + round_eps)
let  x = raw_x
let y = raw_y
if (abs(x) < round_eps) { x = 0 }
if (abs(y) < round_eps) { y = 0 }
if (inside && was_inside == 0) {
first_x = x
first_y = y 

if (distance(x, y, prev_x, prev_y) > 0.01) {
move_safe_z()
travel_to(x, y)
plunge_cut()

} else {G1 X[x] Y[y] F[feed]}

drawing = 1
point_count = 1
} else if (inside && drawing == 1) {
G1 X[x] Y[y] F[feed]
point_count = point_count + 1
} else if (!inside && was_inside == 1 && drawing == 1) {

if (point_count > 1 && distance(x, y, first_x, first_y) < 0.5) {
G1 X[first_x] Y[first_y] F[fed]}
move_safe_z()
drawing = 0
point_count = 0
}

if (inside) {prev_x = x  prev_y = y}
was_inside = inside
i = i + step_deg
}

if (drawing == 1 && distance(prev_x, prev_y, first_x, first_y) < 0.5) {
G1 X[first_x] Y[first_y] F[feed]
move_safe_z(0)
}
}

// --- Grid of clipped circles ---
for row = -5..5 {
for col = -5..5 {
let ox = col * spacing_x
let oy = row * spacing_y
let even = floor(row / 2) * 2
let is_odd = row - even
ox = ox + is_odd * (spacing_x / 2)
draw_cliped_circle(ox, oy)
}
}

// --- Outer ful rings ---
move_safe_z()

for ing_scale = 1..6 {
let r = outer_r + (ring_scale / 10) - 0.3
travel_to(r, 0)
plunge_cut()
draw_full_circle(r)
travel_to(r, 0)
move_safe_z() }



"





# Simulate typing line-by-line
while IFS= read -r line; do
    # Type the line
    xdotool type --delay 2 "$line"

    # If line ends with '{', simulate one right-arrow to skip the auto-inserted '}'
    if [[ "$line" =~ \{$ ]]; then
        xdotool key Right
    fi

    # Press Enter to go to the next line
    xdotool key Return
done <<< "$text"
