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

# Проверяем наличие необходимых команд
if ! command -v cmake &> /dev/null; then
    echo "Ошибка: cmake не установлен"
    exit 1
fi

if ! command -v make &> /dev/null; then
    echo "Ошибка: make не установлен"
    exit 1
fi

# Очищаем и создаем директорию для сборки
rm -rf build
mkdir -p build
cd build

# Конфигурируем проект
echo "Конфигурация проекта..."
cmake ../src/ -DCMAKE_INSTALL_PREFIX=/usr

# Собираем проект
echo "Сборка проекта..."
make

# Устанавливаем
echo "Установка плагина..."
if sudo -n true 2>/dev/null; then
    sudo make install
else
    # Запрашиваем пароль через systemd-ask-password если доступен
    if command -v systemd-ask-password &> /dev/null; then
        sudo -S make install <<< $(systemd-ask-password "Введите пароль sudo для установки плагина: ")
    else
        # Используем стандартный ввод пароля
        echo "Для установки плагина требуется пароль sudo"
        sudo make install
    fi
fi

if [ $? -eq 0 ]; then
    echo "Плагин успешно установлен"
    restart_krunner
    echo "KRunner перезапущен. Попробуйте использовать плагин, нажав Alt+Space"
else
    echo "Ошибка при установке плагина"
    exit 1
fi
