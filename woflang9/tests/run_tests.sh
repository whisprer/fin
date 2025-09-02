#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="${ROOT}/build/bin/woflang.exe"
if [[ ! -x "$BIN" ]]; then
  echo "missing $BIN — build first"; exit 1
fi
pass=0; fail=0
echo "== Woflang test runner =="
for t in "${ROOT}/tests/"*.wofl; do
  [[ -e "$t" ]] || continue
  echo "--> $(basename "$t")"
  if cat "$t" | "$BIN" > /dev/null 2>&1; then
    echo "    PASS"
    pass=$((pass+1))
  else
    echo "    FAIL"
    fail=$((fail+1))
  fi
done
echo "== Summary =="; echo "PASS: $pass"; echo "FAIL: $fail"
exit $fail
