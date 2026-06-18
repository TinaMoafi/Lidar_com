#!/bin/bash
set -e

cd "$(dirname "$0")/../inc_someip_gateway"

echo "[gateway] Building Eclipse S-CORE SOME/IP gateway..."
bazel build //src/gatewayd:gatewayd_example
bazel build //src/someipd:someipd_example

echo "[gateway] Build finished."