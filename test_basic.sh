#!/usr/bin/env bash
set -euo pipefail

# Build
make -C "$(dirname "$0")/.." -s

# Start a node in background
ROOT="$(cd "$(dirname "$0")/.."; pwd)"
BIN="$ROOT/bin"
$BIN/node --host 127.0.0.1 --port 5007 --peers 127.0.0.1:5007 >/dev/null 2>&1 &
PID=$!
sleep 0.5

# Use client to PUT/GET
RESP=$($BIN/client --host 127.0.0.1 --port 5007 <<EOF
PUT foo bar
GET foo
EOF
)

echo "$RESP"

kill $PID >/dev/null 2>&1 || true
