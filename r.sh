#!/bin/bash

echo "📊 GGcode C Code Report"
echo "==============================="

INCLUDE_TESTS=false
if [[ "$1" == "--t" ]]; then
    INCLUDE_TESTS=true
fi

INCLUDE_DIR="include"
SRC_DIR="src"
TEST_DIR="tests"
REPORT_FILE="function_report.txt"
REPORT_HTML="function_report.html"

# Collect all C/H files excluding Unity if not testing
if $INCLUDE_TESTS; then
    echo "🔎 Including test files..."
    files=$(find "$INCLUDE_DIR" "$SRC_DIR" "$TEST_DIR" \
        \( -path "$TEST_DIR/Unity" -o -path "$TEST_DIR/Unity/*" \) -prune -false -o \
        \( -name "*.c" -o -name "*.h" \))
else
    echo "🔎 Skipping test files..."
    files=$(find "$INCLUDE_DIR" "$SRC_DIR" \( -name "*.c" -o -name "*.h" \))
fi

# Check for ctags
if ! command -v ctags &> /dev/null; then
    echo "❌ Error: 'ctags' is not installed. Please install it to use accurate function counting."
    exit 1
fi

total_lines=0
total_comments=0
total_functions=0
file_count=0
total_bytes=0

echo "" > "$REPORT_FILE"
echo "" > "$REPORT_HTML"
declare -A FUNC_INFO

# Gather info from files
for file in $files; do
    file_count=$((file_count + 1))
    lines=$(wc -l < "$file")
    comments=$(grep -E '^\s*//|^\s*/\*|^\s*\*' "$file" | wc -l)
    bytes=$(wc -c < "$file")

    total_lines=$((total_lines + lines))
    total_comments=$((total_comments + comments))
    total_bytes=$((total_bytes + bytes))

    echo "📄 $file"
    echo "   🔹 Lines     : $lines"
    echo "   🔹 Comments  : $comments"
    echo "   🔹 Bytes     : $bytes"
    echo

    if [[ "$file" == *.c ]]; then
        while read -r name kind line file rest; do
            if [[ "$kind" == "function" && "$line" =~ ^[0-9]+$ ]]; then
                FUNC_INFO["$file|$line"]="$name"
                total_functions=$((total_functions + 1))
            fi
        done < <(ctags -x --c-kinds=f "$file" 2>/dev/null)
    fi
done

# Summary
human_size=$(numfmt --to=iec-i --suffix=B <<< $total_bytes)
code_lines=$((total_lines - total_comments))
avg_lines_per_file=$((total_lines / file_count))
avg_func_size=$((total_lines / (total_functions > 0 ? total_functions : 1)))

echo "==============================="
echo "📦 Total Files     : $file_count"
echo "📏 Total Lines     : $total_lines"
echo "🧩 Total Functions : $total_functions"
echo "💬 Total Comments  : $total_comments"
echo "🔢 Total Bytes     : $total_bytes ($human_size)"
echo "📊 Code Only Lines : $code_lines"
echo "📈 Avg Lines/File  : $avg_lines_per_file"
echo "📉 Avg Func Size   : $avg_func_size"
echo "==============================="

# Text Report
{
    echo "📄 Function Report - GGcode Project"
    echo "==================================="
    printf "%-40s %-10s %-10s %s\n" "Function" "Line" "Length" "File"
    echo "-----------------------------------"

    for key in $(printf "%s\n" "${!FUNC_INFO[@]}" | sort); do
        file="${key%%|*}"
        start="${key##*|}"
        name="${FUNC_INFO[$key]}"
        next_line=$(printf "%s\n" "${!FUNC_INFO[@]}" | grep "^$file|" | cut -d'|' -f2 | awk -v l="$start" '$1 > l' | sort -n | head -1)
        [[ -z "$next_line" ]] && end=$(wc -l < "$file") || end=$((next_line - 1))
        length=$((end - start + 1))
        printf "%-40s %-10s %-10s %s\n" "$name" "$start" "$length" "$file"
    done
} > "$REPORT_FILE"

echo "📝 Saved text function report to: $REPORT_FILE"

# HTML Report
{
echo "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>GGcode Function Report</title>"
echo "<style>
body { font-family: sans-serif; background: #1e1e1e; color: #eee; padding: 20px; }
h1, h2 { color: #4fc3f7; }
table { border-collapse: collapse; width: 100%; margin-bottom: 40px; }
th, td { border: 1px solid #555; padding: 6px 10px; text-align: left; }
th { background: #333; }
.low { background-color: #2e7d32; }       /* Green */
.medium { background-color: #fbc02d; }    /* Yellow */
.high { background-color: #c62828; }      /* Red */
</style></head><body>"
echo "<h1>📄 GGcode Function Report</h1>"
echo "<p><b>Total Files:</b> $file_count | <b>Lines:</b> $total_lines | <b>Functions:</b> $total_functions | <b>Size:</b> $human_size</p>"

for file in $(printf "%s\n" "${!FUNC_INFO[@]}" | cut -d'|' -f1 | sort -u); do
    echo "<h2>📁 $file</h2>"
    echo "<table><tr><th>Function</th><th>Line</th><th>Length</th></tr>"
    for key in $(printf "%s\n" "${!FUNC_INFO[@]}" | grep "^$file|" | sort -t'|' -k2 -n); do
        start="${key##*|}"
        name="${FUNC_INFO[$key]}"
        next_line=$(printf "%s\n" "${!FUNC_INFO[@]}" | grep "^$file|" | cut -d'|' -f2 | awk -v l="$start" '$1 > l' | sort -n | head -1)
        [[ -z "$next_line" ]] && end=$(wc -l < "$file") || end=$((next_line - 1))
        length=$((end - start + 1))
        if (( length <= 10 )); then
            class="low"
        elif (( length <= 40 )); then
            class="medium"
        else
            class="high"
        fi
        echo "<tr class=\"$class\"><td>$name</td><td>$start</td><td>$length</td></tr>"
    done
    echo "</table>"
done

echo "</body></html>"
} > "$REPORT_HTML"

echo "🌐 Saved HTML function report to: $REPORT_HTML"
