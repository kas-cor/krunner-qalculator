#!/bin/bash
#
# package.sh — собирает prebuilt-архив krunner_qalculator для установки без toolchain.
#
# Использование:
#   ./scripts/package.sh [BUILD_DIR] [OUTPUT_DIR]
#
# По умолчанию: BUILD_DIR=build  OUTPUT_DIR=dist
#
# Что упаковывается:
#   - компилируемый плагин krunner_qalculator.so (из BUILD_DIR)
#   - манифест manifest.json (из src/)
#   - postinst / postrm (хуки KDE)
#   - install-prebuilt.sh (установщик)
#
# На выходе: dist/krunner-qalculator-<VERSION>.tar.gz

set -euo pipefail

# Корень репозитория — независимо от CWD
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

BUILD_DIR="${1:-build}"
OUTPUT_DIR="${2:-dist}"

# Версия из src/manifest.json (поле KPlugin -> Version)
VERSION="$(python3 -c "import json;print(json.load(open('src/manifest.json'))['KPlugin']['Version'])")"

PLUGIN_SO="$(find "$BUILD_DIR" -name 'krunner_qalculator.so' 2>/dev/null | head -1)"
if [ -z "$PLUGIN_SO" ]; then
    echo "ERROR: не найден krunner_qalculator.so в '$BUILD_DIR'. Сначала соберите плагин." >&2
    exit 1
fi

mkdir -p "$OUTPUT_DIR"
STAGE="$OUTPUT_DIR/stage"
rm -rf "$STAGE"
mkdir -p "$STAGE"

cp "$PLUGIN_SO" "$STAGE/krunner_qalculator.so"
cp src/manifest.json   "$STAGE/manifest.json"
cp postinst            "$STAGE/postinst" 2>/dev/null || true
cp postrm              "$STAGE/postrm"   2>/dev/null || true
cp scripts/install-prebuilt.sh "$STAGE/install-prebuilt.sh"

chmod +x "$STAGE/install-prebuilt.sh"

ARCHIVE="$OUTPUT_DIR/krunner-qalculator-$VERSION.tar.gz"
tar -czf "$ARCHIVE" -C "$OUTPUT_DIR" stage

echo "Архив собран: $ARCHIVE (внутри: krunner_qalculator.so, manifest.json, postinst, postrm, install-prebuilt.sh)"
