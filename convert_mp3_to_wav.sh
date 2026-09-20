#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Define input and output directories relative to the script location
INPUT_DIR="$SCRIPT_DIR/mp3"
OUTPUT_DIR="$SCRIPT_DIR/converted"

# Ensure output directory exists
mkdir -p "$OUTPUT_DIR"

# Initialize counter
i=0

# Loop through all .mp3 files in INPUT_DIR
for file in "$INPUT_DIR"/*.mp3; do
    # Skip if no .mp3 files
    [ -e "$file" ] || continue

    # Define output file name
    OUTPUT_FILE="$OUTPUT_DIR/sample_$i.wav"

    # Run VLC conversion
    cvlc "$file" --sout="#transcode{acodec=s16l,channels=1,samplerate=48000}:std{access=file,mux=wav,dst='${OUTPUT_FILE}'}" vlc://quit

    # Increment counter
    i=$((i + 1))
done
