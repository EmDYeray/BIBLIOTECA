#!/usr/bin/env bash
set -euo pipefail

carpeta=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
if command -v python3 >/dev/null 2>&1; then
    python3 "$carpeta/tools/update-content.py" "$carpeta/assets" >&2 || true
fi
exec "$carpeta/biblioteca" "$@"
