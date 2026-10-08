#!/usr/bin/env bash
# End-to-end test: start the server, send messages with the client,
# and check that the server printed every one of them.
set -euo pipefail

PORT=${PORT:-9655}
LOG=$(mktemp)
trap 'kill "$SERVER_PID" 2>/dev/null || true; rm -f "$LOG"' EXIT

./bin/udp_server "$PORT" > "$LOG" &
SERVER_PID=$!
sleep 0.3

printf 'hello\nUDP is connectionless\nlast message\n' | ./bin/udp_client localhost "$PORT"
sleep 0.3

fail=0
for msg in "hello" "UDP is connectionless" "last message"; do
    if grep -q "\] $msg\$" "$LOG"; then
        echo "OK    received: $msg"
    else
        echo "FAIL  missing:  $msg"
        fail=1
    fi
done

# Invalid arguments must be rejected with a non-zero exit code.
if ./bin/udp_client 2>/dev/null; then echo "FAIL  client accepted no host"; fail=1;
else echo "OK    client without host rejected"; fi
if ./bin/udp_server 99999 2>/dev/null; then echo "FAIL  server accepted port 99999"; fail=1;
else echo "OK    invalid port rejected"; fi
if ./bin/udp_client no.such.host.invalid 2>/dev/null </dev/null; then echo "FAIL  unknown host accepted"; fail=1;
else echo "OK    unknown host rejected"; fi

echo "--- server output ---"
cat "$LOG"
exit $fail
