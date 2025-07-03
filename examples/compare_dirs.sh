#!/bin/bash

DIR_A="$1"
DIR_B="$2"
OUTPUT_FILE="diff_report.html"

if [[ -z "$DIR_A" || -z "$DIR_B" ]]; then
  echo "Usage: $0 <dirA> <dirB>"
  exit 1
fi

echo "Comparing files in '$DIR_A' and '$DIR_B'..."
echo "Generating HTML report at $OUTPUT_FILE"

# Begin HTML
echo "<!DOCTYPE html>
<html>
<head>
  <meta charset='UTF-8'>
  <title>Diff Report (Only Mismatched Lines)</title>
  <style>
    body { font-family: monospace; background: #111; color: #eee; padding: 20px; }
    h2 { color: #6cf; }
    .diff { background: #222; border: 1px solid #444; padding: 10px; white-space: pre-wrap; margin-bottom: 20px; }
    .add { color: #6f6; }
    .del { color: #f66; }
  </style>
</head>
<body>
<h1>G-Code Differences (Only Mismatched Lines)</h1>
<p>Compared: <b>$DIR_A</b> ↔ <b>$DIR_B</b></p>
<hr>" > "$OUTPUT_FILE"

# Compare files
for fileA in "$DIR_A"/*; do
  filename=$(basename "$fileA")
  fileB="$DIR_B/$filename"

  if [[ -f "$fileB" ]]; then
    # Get raw diff


    DIFF=$(diff -u "$fileA" "$fileB" | grep -E '^\+|^\-' | grep -vE '^\+\+\+|^\-\-\-')DIFF=$(diff -u <(sed 's/[[:space:]]*$//' "$fileA" | dos2unix 2>/dev/null) \
               <(sed 's/[[:space:]]*$//' "$fileB" | dos2unix 2>/dev/null) \
       | grep -E '^\+|^\-' | grep -vE '^\+\+\+|^\-\-\-')



    if [[ -n "$DIFF" ]]; then
      echo "<h2>🔍 $filename</h2>" >> "$OUTPUT_FILE"
      echo "<div class='diff'>" >> "$OUTPUT_FILE"
      echo "$DIFF" | sed \
        -e 's/&/\&amp;/g' \
        -e 's/</\&lt;/g' \
        -e 's/>/\&gt;/g' \
        -e 's/^+/<span class="add">+/g' \
        -e 's/^-/<span class="del">-/g' \
        -e 's/$/<\/span>/' >> "$OUTPUT_FILE"
      echo "</div>" >> "$OUTPUT_FILE"
    fi
  else
    echo "<h2>⚠️ File missing in $DIR_B: $filename</h2>" >> "$OUTPUT_FILE"
  fi
done

echo "</body></html>" >> "$OUTPUT_FILE"
echo "✅ Done. Open $OUTPUT_FILE to see the minimal diff."
