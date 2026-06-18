#!/bin/bash
set -e

cd "$(dirname "$0")/../inc_someip_gateway"

echo "[gateway] Starting someipd..."
bazel run //src/someipd:someipd_example
