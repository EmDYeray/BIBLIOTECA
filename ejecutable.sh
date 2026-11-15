#!/usr/bin/env bash
set -euo pipefail

carpeta=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
exec "$carpeta/biblioteca" "$@"
