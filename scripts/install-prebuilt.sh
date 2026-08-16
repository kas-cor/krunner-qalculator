#!/bin/bash
#
# install-prebuilt.sh — установка Qalculator из собранного архива (без toolchain).
#
# Архив должен быть распакован так, чтобы рядом лежали:
#   krunner_qalculator.so, manifest.json, postinst, postrm
#
# Автоопределение: если sudo доступен без пароля — ставим системно,
# иначе — в локальную директорию текущего пользователя (~/.local).
# После установки перезапускаем krunner.
#
# Использование:
#   ./install-prebuilt.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLUGIN_ID="krunner_qalculator"
PLUGIN_SO="$SCRIPT_DIR/$PLUGIN_ID.so"

if [ ! -f "$PLUGIN_SO" ]; then
    echo "ERROR: не найден $PLUGIN_SO рядом со скриптом." >&2
    echo "Убедитесь, что вы распаковали архив и каталог-влево не остался пуст." >&2
    exit 1
fi

# Определение krunner-версии (5 или 6)
KRUNNER_MAJOR="$(krunner --version 2>/dev/null | grep -oP '(?<=krunner )\d+' | head -1 || true)"
KRUNNER_MAJOR="${KRUNNER_MAJOR:-6}"

USE_SYSTEM=0
if sudo -n true 2>/dev/null; then
    USE_SYSTEM=1
fi

if [ "$USE_SYSTEM" = "1" ]; then
    PLUGIN_DIR="/usr/lib/qt${KRUNNER_MAJOR}/plugins/kf${KRUNNER_MAJOR}/krunner"
    DEST="$PLUGIN_DIR/$PLUGIN_ID.so"
    MAN_DEST="/usr/share/kservices5/krunner/$PLUGIN_ID.desktop"
    echo "> Установка в системный каталог: $PLUGIN_DIR"
    sudo mkdir -p "$PLUGIN_DIR"
    sudo install -m 0644 "$PLUGIN_SO" "$DEST"
    # manifest.json → .desktop, если рядом установщик postinst
    if [ -f "$SCRIPT_DIR/postinst" ]; then
        sudo cp "$SCRIPT_DIR/postinst" /usr/bin/krunner_qalculator-postinst 2>/dev/null || true
        sudo chmod +x /usr/bin/krunner_qalculator-postinst 2>/dev/null || true
    fi
else
    PLUGIN_DIR="$HOME/.local/lib/qt${KRUNNER_MAJOR}/plugins/kf${KRUNNER_MAJOR}/krunner"
    DEST="$PLUGIN_DIR/$PLUGIN_ID.so"
    echo "> Установка в локальный каталог: $PLUGIN_DIR"
    mkdir -p "$PLUGIN_DIR"
    install -m 0644 "$PLUGIN_SO" "$DEST"
fi

echo "> Перезапуск KRunner..."
pkill -x krunner 2>/dev/null || true

echo "OK: Qalculator установлен ($DEST). Нажмите Alt+Space и введите '='."
