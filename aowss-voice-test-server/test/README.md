# AOWS Server Test Guide

This directory contains automated tests for `aows_test_server.js`.

## Test Files

- `aows_test_server.positive.test.js`: happy-path behavior and expected successful flows.
- `aows_test_server.negative.test.js`: rejection, error, and edge-case behavior.
- `test_utils.js`: shared helpers for server lifecycle, health polling, and websocket setup.

## Run Tests

Run all tests:

```bash
npm test
```

Run only positive tests:

```bash
node --test test/aows_test_server.positive.test.js
```

Run only negative tests:

```bash
node --test test/aows_test_server.negative.test.js
```

## Coverage Snapshot

Positive suite currently validates:

- `GET /health` returns `200` and expected JSON.
- Non-upgrade HTTP requests return `426 Upgrade Required`.
- Binary websocket frames are captured to raw `.pcm` files.
- Diagnostics mode returns `session_open`, `pong`, and `stats`.
- ADPCM decode mode writes decoded PCM output for aligned ADPCM frames.

Negative suite currently validates:

- Websocket upgrade is rejected when request path does not match `AOWS_PATH`.
- Unsupported diagnostics events return an error payload.
- Misaligned ADPCM payload does not write decoded PCM samples.

## Notes

- Tests launch the server as a child process on a random local port.
- Each test uses a temporary output directory and removes it at teardown.
- Node.js 18+ is required.
