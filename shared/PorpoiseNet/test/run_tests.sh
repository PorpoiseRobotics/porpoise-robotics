#!/usr/bin/env bash
# Run the PorpoiseNet network tests on a computer. No ESP32 needed.
#   ./run_tests.sh
# Exits 0 if every test passes, non-zero otherwise.
set -euo pipefail
cd "$(dirname "$0")"
OUT=$(mktemp -d)
trap 'rm -rf "$OUT"' EXIT

# Three nodes = three separate .cpp files, because each one needs its own
# private copy of the network object (PorpoiseNet.h is header-only).
for N in A B C; do
  sed "s/NODE_/${N}_/g" node.inc > "$OUT/node$N.cpp"
done

g++ -std=gnu++17 -Wall -Wextra -Wno-unused-parameter \
    -I../src -I. -Istubs \
    meshtest.cpp "$OUT/nodeA.cpp" "$OUT/nodeB.cpp" "$OUT/nodeC.cpp" \
    stubs/fake_hardware.cpp -o "$OUT/meshtest"
"$OUT/meshtest"
