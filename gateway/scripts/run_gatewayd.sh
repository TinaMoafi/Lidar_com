#!/bin/bash
set -e

cd "$(dirname "$0")/../inc_someip_gateway"

echo "[gateway] Starting gatewayd..."
bazel run //src/gatewayd:gatewayd_example
