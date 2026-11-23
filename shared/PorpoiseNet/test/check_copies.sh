#!/usr/bin/env bash
# Some sketches carry their own copy of PorpoiseNet.h so nobody has to
# install a library. This checks that every copy is identical to the real
# one in shared/PorpoiseNet/src. If a copy differs, copy the real one over it:
#   cp shared/PorpoiseNet/src/PorpoiseNet.h <that sketch folder>/
# Copies under a legacy/ folder are skipped: old sketches keep the
# version they shipped with.
set -euo pipefail
cd "$(dirname "$0")/../../.."
REAL=shared/PorpoiseNet/src/PorpoiseNet.h
bad=0
n=0
while IFS= read -r copy; do
  [ "$copy" = "./$REAL" ] && continue
  case "$copy" in */legacy/*) continue ;; esac
  n=$((n + 1))
  if cmp -s "$REAL" "$copy"; then
    echo "  [PASS] $copy"
  else
    echo "  [FAIL] $copy differs from $REAL"
    bad=1
  fi
done < <(find . -name PorpoiseNet.h -not -path './.git/*')
if [ "$n" -eq 0 ]; then echo "  (no copies found)"; fi
exit $bad
