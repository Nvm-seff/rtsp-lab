#!/bin/bash

# Change to project root for consistent paths
cd "$(dirname "$0")/.." || exit 1

# 1. Test against local fixture
echo "Running fixture test..."
OUTPUT=$(./build/rtsp-lab probe ./tests/fixtures/sample.mp4 --json)
exit_code=$?

if [ $exit_code -eq 0 ] && [[ $OUTPUT == *"h264"* ]] && [[ $OUTPUT == *"fps_measured"* ]]; then
    echo "✓ Fixture Test Passed"
else
    echo "✗ Fixture Test Failed"
    echo "Output: $OUTPUT"
    echo "Exit code: $exit_code"
    exit 1
fi

# 2. Test timeout and exit code
echo "Running timeout test..."
start_time=$(date +%s)
OUTPUT=$(./build/rtsp-lab probe rtsp://does.not.exist:554/none --timeout 2 --json 2>&1)
exit_code=$?
end_time=$(date +%s)
elapsed=$((end_time - start_time))

if [ $exit_code -ne 0 ] && [ $elapsed -le 3 ] && [[ $OUTPUT == *"error"* ]]; then
    echo "✓ Timeout Test Passed"
else
    echo "✗ Timeout Test Failed (Time: $elapsed, Exit: $exit_code)"
    echo "Output: $OUTPUT"
    exit 1
fi

echo ""
echo "All integration tests passed!"