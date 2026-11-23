#!/usr/bin/env bash
# Run the PorpoiseNet network tests on a computer. No ESP32 needed.
#   ./run_tests.sh
# Needs g++ (on Windows: winget install BrechtSanders.WinLibs.POSIX.UCRT).
# Exits 0 if every test passes, non-zero otherwise.
set -euo pipefail
cd "$(dirname "$0")"
OUT=$(mktemp -d)
trap 'rm -rf "$OUT"' EXIT

# Each node is its own .cpp file, because each one needs its own private
# copy of the network object (PorpoiseNet.h is header-only).
for N in A B C D E F; do
  sed "s/NODE_/${N}_/g" node.inc > "$OUT/node$N.cpp"
done
FLAGS="-std=gnu++17 -Wall -Wextra -Wno-unused-parameter -I../src -I. -Istubs"

echo "=== meshtest: three boards, relays and acks ==="
g++ $FLAGS meshtest.cpp "$OUT/nodeA.cpp" "$OUT/nodeB.cpp" "$OUT/nodeC.cpp" \
    stubs/fake_hardware.cpp -o "$OUT/meshtest"
"$OUT/meshtest"

echo
echo "=== stationtest: three Find the Target stations on one field ==="
g++ $FLAGS stationtest.cpp "$OUT"/node{A,B,C,D,E,F}.cpp \
    stubs/fake_hardware.cpp -o "$OUT/stationtest"
"$OUT/stationtest"

echo
echo "=== copies of PorpoiseNet.h inside sketch folders match this one ==="
./check_copies.sh
