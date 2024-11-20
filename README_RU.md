# KRunner Qalculator Plugin

*Read this in other languages: [English](README.md), [Русский](README_RU.md)*

Мощный калькулятор-плагин для KRunner в KDE Plasma, использующий расширенные возможности libqalculate.

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

## Возможности

- Вычисления с произвольной точностью
- Поддержка решения алгебраических уравнений
- Символьные вычисления для точных результатов
- Поддержка конвертации валют
- Возможности конвертации единиц измерения
- Расширенные математические функции

## Требования

### Системные требования
- KDE Plasma 6.x
- Qt 6.x
- CMake
- Make
- Права sudo для установки

### Зависимости
- `libqalculate` (команда qalc должна быть доступна в $PATH)
- Файлы разработки KRunner

## Установка

1. Убедитесь, что все зависимости установлены
2. Клонируйте репозиторий:
   ```bash
   git clone https://github.com/your-username/krunner_qalc.git
   cd krunner_qalc
   ```
3. Запустите скрипт установки:
   ```bash
   ./install.sh
   ```

Скрипт выполнит:
- Настройку среды сборки
- Компиляцию плагина
- Установку в систему
- Автоматический перезапуск KRunner

## Удаление

Для удаления плагина выполните:
```bash
./uninstall.sh
```

## Использование

1. Нажмите `Alt+Space` для открытия KRunner
2. Введите математическое выражение
3. Нажмите Enter для получения результата

### Примеры:
- Базовые вычисления: `2 + 2 =`
- Конвертация валют: `100 USD to EUR =`
- Конвертация единиц измерения: `100 km/h to mph =`
- Уравнения: `solve x^2 + 2x + 1 = 0`

## Устранение неполадок

1. Если KRunner не показывает плагин:
   - Убедитесь, что KRunner был перезапущен после установки
   - Проверьте, что qalc установлен и доступен в PATH

2. Если вычисления не работают:
   - Проверьте, что libqalculate правильно установлен
   - Проверьте синтаксис вашего выражения

## Участие в разработке

1. Создайте форк репозитория
2. Создайте ветку для новой функции (`git checkout -b feature/amazing-feature`)
3. Зафиксируйте изменения (`git commit -m 'Добавлена новая функция'`)
4. Отправьте изменения в репозиторий (`git push origin feature/amazing-feature`)
5. Откройте Pull Request

## Лицензия

Этот проект лицензирован под GNU General Public License v2.0 - подробности смотрите в файле [LICENSE](LICENSE).

## Благодарности

- Спасибо сообществу KDE за KRunner
- Спасибо команде libqalculate за их мощный движок вычислений