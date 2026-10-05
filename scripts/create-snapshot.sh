#!/usr/bin/env bash
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

output="../EPANET-unit-independent-current.zip"

git archive --format=zip --output="$output" HEAD

echo "Created $output"
