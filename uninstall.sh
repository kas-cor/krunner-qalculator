#!/bin/bash

# Функция для перезапуска KRunner
restart_krunner() {
    if pgrep -x "krunner" > /dev/null; then
        kquitapp6 krunner &>/dev/null || true
        sleep 1
    fi
    if command -v kstart6 &> /dev/null; then
        nohup kstart6 krunner &>/dev/null & disown
    else
        nohup krunner &>/dev/null & disown
    fi
    sleep 1
}

# Проверяем наличие директории build
if [ ! -d "build" ]; then
    echo "Ошибка: директория build не найдена"
    echo "Запустите install.sh хотя бы один раз перед удалением"
    exit 1
fi

# Переходим в директорию сборки
cd build

# Удаляем плагин
echo "Удаление плагина..."
if sudo -n true 2>/dev/null; then
    sudo make uninstall
else
    # Запрашиваем пароль через systemd-ask-password если доступен
    if command -v systemd-ask-password &> /dev/null; then
        sudo -S make uninstall <<< $(systemd-ask-password "Введите пароль sudo для удаления плагина: ")
    else
        # Используем стандартный ввод пароля
        echo "Для удаления плагина требуется пароль sudo"
        sudo make uninstall
    fi
fi

if [ $? -eq 0 ]; then
    echo "Плагин успешно удален"
    restart_krunner
    echo "KRunner перезапущен"
else
    echo "Ошибка при удалении плагина"
    exit 1
fi
