# KRunner Qalculator Plugin — Agent Context

> This file is maintained for AI coding agents and human contributors.  
> It is the canonical reference for understanding the codebase end-to-end.

---

## 1. Project Overview

**krunner-qalculator** — плагин-калькулятор для KRunner (KDE Plasma), использующий `libqalculate` напрямую (C++ API, не через `qalc` CLI).

| Атрибут | Значение |
|---|---|
| Язык | C++17 |
| Фреймворк | Qt 6 + KF6 (KDE Frameworks 6) |
| Сборка | CMake + ECM |
| Плагин раннера | `KRunner::AbstractRunner` |
| Движок вычислений | `libqalculate` (линковка через `PkgConfig`) |
| Версия | 2.1.0 |
| Лицензия | GPL-2.0 (LICENSE, manifest.json, заголовки файлов) |

### Структура репозитория

```
krunner_qalc/
├── AGENTS.md                    # ← Этот файл
├── CMakeLists_root.txt          # Корневой CMake-файл (ручной запуск)
├── CMakeLists.txt               # Копия CMakeLists_root.txt для сборки
├── qalculatorRunner.kdev4       # KDevelop project file
├── README.md                    # Английская документация
├── README_RU.md                 # Русская документация
├── LICENSE                      # GPL-2.0
├── install.sh                   # Скрипт установки
├── uninstall.sh                 # Скрипт удаления
├── postinst                     # Post-install hook
├── postrm                       # Post-remove hook
├── src/
│   ├── CMakeLists.txt           # CMake-сборка плагина
│   ├── manifest.json            # KPlugin-метаданные
│   ├── qalculatorrunner.h       # Заголовок класса
│   └── qalculatorrunner.cpp     # Реализация
├── tests/
│   ├── CMakeLists.txt           # Сборка тестов
│   └── test_qalculatorrunner.cpp # Тесты (libqalculate C++ API)
└── build/                       # Сгенерировано сборкой
```

---

## 2. End-to-End Architecture

### 2.1. Жизненный цикл плагина

```
Пользователь печатает в KRunner (Alt+Space)
    │
    ▼
KRunner запускает все плагины → QalculatorRunner::match()
    │
    ▼
Проверка: term.isEmpty() → return
    │
    ▼
QalculatorRunner::calculate(term)
    │
    ├── CALCULATOR->unlocalizeExpression()
    ├── CALCULATOR->calculate()
    ├── CALCULATOR->print()
    │
    ▼
Если результат не пустой и отличается от ввода →
    │
    ▼
KRunner::QueryMatch с relevance=1.0
    │   + иконка accessories-calculator
    │   + Action "copy" (edit-copy)
    │
    ▼
Пользователь выбирает действие:
    ├── Copy to clipboard → run() вызывает copyToClipboard()
    │                        и закрывает KRunner
    └── Insert result     → run() вызывает requestQueryStringUpdate()
                             без закрытия KRunner
```

### 2.2. Инициализация

В конструкторе `QalculatorRunner`:

```cpp
if (!CALCULATOR) {
    CALCULATOR = new Calculator();
    CALCULATOR->loadGlobalDefinitions();
    CALCULATOR->loadLocalDefinitions();
    CALCULATOR->loadExchangeRates();
}
```

`CALCULATOR` — глобальный синглтон `libqalculate::Calculator`.  
Инициализируется один раз при первой загрузке плагина.  
Загрузка курсов валют происходит здесь же (может быть медленной).

### 2.3. Механизм вычислений

Функция `calculate()` принимает строку запроса:

```cpp
QString QalculatorRunner::calculate(const QString &term)
```

**Шаги:**
1. `CALCULATOR->unlocalizeExpression()` — конвертирует локализованное выражение в стандартное (запятые → точки и т.д.).
2. `CALCULATOR->calculate(&mstruct, expr, timeout, eo)` — вычисляет с таймаутом 2000 мс.
3. `CALCULATOR->print(mstruct, timeout, po)` — форматирует результат.
4. Если результат совпадает с исходным выражением → возвращает `QString()` (ничего не показываем).
5. Возвращает отформатированную строку.

**Настройки печати (`PrintOptions`):**
- `interval_display = INTERVAL_DISPLAY_MIDPOINT` — аппроксимация интервалов.
- Остальные — по умолчанию.

**TODO из кода:** Настройки `eo` и `po` должны быть конфигурируемыми.

### 2.4. Матчинг (match → KRunner)

```cpp
void QalculatorRunner::match(KRunner::RunnerContext &context)
```

- Релевантность всегда `1.0` — максимальная.
- Одно действие по умолчанию — "вставить результат".
- Добавляется action "Copy to clipboard" с иконкой `edit-copy`.
- Используется `KLocalizedString::i18n()` для перевода.

### 2.5. Обработка действий (run)

```cpp
void QalculatorRunner::run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match)
```

| Action | Поведение |
|---|---|
| `"copy"` | Копирует текст в буфер обмена → `context.requestQueryStringUpdate(QString(), 0)` (закрывает KRunner) |
| default | Вставляет результат в строку запроса → `context.requestQueryStringUpdate(result, result.length())` (KRunner остаётся открыт) |

### 2.6. Буфер обмена

```cpp
bool QalculatorRunner::copyToClipboard(const QString &text)
```

Использует `QGuiApplication::clipboard()->setText()`.  
Возвращает `false`, если клипборд недоступен.

---

## 3. Конфигурация (manifest.json)

```json
{
  "KPlugin": {
    "Id": "krunner_qalculator",
    "Name": "Qalculator",
    "Category": "Finance",
    "Description": "Calculator using qalc",
    "Icon": "accessories-calculator",
    "Version": "2.0.1",
    "License": "GPL-2.0",
    "Website": "https://github.com/kas-cor/krunner-qalculator"
  },
  "KRunner": {
    "MinimumQuery": 2,
    "Triggers": ["="]
  }
}
```

**Важно:** У плагина есть триггер `"="`, но код в `match()` проверяет только `term.isEmpty()` — триггер не обрабатывается явно. KRunner сам активирует плагин по триггеру. `MinimumQuery: 2` означает, что нужно минимум 2 символа.

---

## 4. Сборка

### Основной CMakeLists (src/CMakeLists.txt)

```
- C++17
- AUTOMOC + AUTORCC
- Зависимости: Qt6::Core, Qt6::Widgets, KF6::Runner, KF6::I18n, KF6::CoreAddons, KF6::ConfigCore
- libqalculate через pkg_check_modules
- kcoreaddons_add_plugin() — установка в kf6/krunner
- BUILD_TESTING включает тесты
```

### Корневой CMakeLists_root.txt

Отдельный файл, не включённый в основной проект. Собирает `src/` и `tests/`. Используется для ручной сборки с тестами.

### install.sh

1. Клонирует/обновляет репозиторий
2. Определяет версию Plasma (5 или 6) через `krunner --version`
3. `cmake ../src/` с `-DBUILD_TESTING=OFF`
4. `make -j$(nproc)`
5. `sudo make install`
6. `pkill -x krunner` — перезапуск KRunner

### uninstall.sh

- `sudo make uninstall` из папки `build/`
- `kquitapp${krunner_version} krunner`

---

## 5. Тесты

Тесты в `tests/test_qalculatorrunner.cpp` используют QTest + `libqalculate` C++ API напрямую (без KDE сессии).

| Тест | Что проверяет |
|---|---|
| `testBasicCalculation` | Арифметика: 2+2, 10*5, 100/4, 7-3, 2^10 |
| `testComplexExpression` | Функции: sqrt, pi, sin, факториал, log10, вложенные скобки |
| `testUnitConversion` | Конвертация единиц (locale-aware: cm/см, mph) |
| `testInputEqualsResult` | Подавление результата, совпадающего с вводом |
| `testErrorHandling` | Краш-тесты: пустой ввод, мусор, 1/0 |
| `testExpressionUnlocalization` | `unlocalizeExpression()` |

**Важное:** Тесты не тестируют `QalculatorRunner::calculate()` напрямую (private метод), а реплицируют его логику с тем же `CALCULATOR` синглтоном. `loadExchangeRates()` пропущен — он требует сеть и замедляет тесты.

### Сборка тестов

```bash
mkdir build_test && cd build_test
cmake .. -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
make && ctest --output-on-failure -V
```

---

## 6. Безопасность

- **Исходный код использует `libqalculate` C++ API, не вызывает shell.**  
  Shell-инъекции невозможны, т.к. `QProcess` и shell не используются.
- Старые тесты проверяли command injection prevention (для исторического QProcess-кода).
- Текущие тесты проверяют корректность вычислений libqalculate.

### Что всё ещё может пойти не так

- `libqalculate` может быть не установлен или устаревшей версии.
- Долгая загрузка exchange rates (блокирует UI).
- Нет обработки переполнения памяти при сложных выражениях.

---

## 7. Зависимости

### Сборка
- `cmake` >= 3.16 / `make`
- `g++` с поддержкой C++17
- `ECM` (Extra CMake Modules) >= 5.240.0
- `Qt6` (Core, Widgets, Test)
- `KF6` (Runner, I18n, CoreAddons, Config)
- `libqalculate-dev` (через PkgConfig)

### Runtime
- `libqalculate` (разделяемая библиотека)
- KDE Plasma 6 / KRunner

---

## 8. Известные проблемы и TODO

1. **Настройки eo/po** — захардкожены, должны быть конфигурируемыми (возможно, через конфиг KRunner'а).
2. **Загрузка exchange rates** — в конструкторе может тормозить первый запуск. Можно сделать ленивой или фоновой.
3. **Триггер `"="`** — не проверяется явно, KRunner сам решает. Стоит добавить явную проверку в `match()`.
4. **Локализация** — `KLocalizedString` подключена, но переводов может не быть.
5. **Копия репозитория** — `install.sh` клонирует репозиторий отдельно от исходников.

---

## 9. Ключевые файлы для агента

| Файл | Зачем |
|---|---|
| `src/qalculatorrunner.h` | API класса, макрос `K_PLUGIN_CLASS_WITH_JSON` |
| `src/qalculatorrunner.cpp` | Вся логика: match, run, calculate, copyToClipboard |
| `src/manifest.json` | Конфигурация KRunner (триггеры, иконка, мин. длина) |
| `src/CMakeLists.txt` | Зависимости, линковка, установка |
| `tests/test_qalculatorrunner.cpp` | Тесты libqalculate C++ API |
| `tests/CMakeLists.txt` | Сборка тестов (Qt6::Test + libqalculate) |
| `install.sh` | Полный lifecycle установки |

---

## 10. libqalculate C++ API — примеры и отладка

### 10.1. Инициализация Calculator

```cpp
#include <libqalculate/Calculator.h>
#include <libqalculate/includes.h>

// Глобальный синглтон (определён в includes.h)
if (!CALCULATOR) {
    CALCULATOR = new Calculator();
    CALCULATOR->loadGlobalDefinitions();   // функции, единицы, константы
    CALCULATOR->loadLocalDefinitions();    // пользовательские определения
    CALCULATOR->loadExchangeRates();       // курсы валют (медленно, требует сеть)
}
```

**Совет:** Для тестов `loadExchangeRates()` можно пропустить — это ускоряет запуск в ~10 раз.

### 10.2. Вычисление выражения

Полный цикл, как в `QalculatorRunner::calculate()`:

```cpp
#include <libqalculate/Calculator.h>
#include <libqalculate/includes.h>

QString calculate(const QString &term)
{
    const int TIMEOUT_MS = 2000;

    // 1. Убрать локализацию (запятые → точки и т.п.)
    std::string expr = CALCULATOR->unlocalizeExpression(term.toStdString());

    // 2. Вычислить
    MathStructure mstruct;
    EvaluationOptions eo;                     // можно настроить точность, режимы
    bool success = CALCULATOR->calculate(&mstruct, expr, TIMEOUT_MS, eo);
    if (!success) return QString();

    // 3. Отформатировать вывод
    PrintOptions po;
    po.interval_display = INTERVAL_DISPLAY_MIDPOINT;  // интервалы → приближение
    std::string res_str = CALCULATOR->print(mstruct, TIMEOUT_MS, po);

    // 4. Не показывать выражение, если результат совпадает с вводом
    if (res_str == expr) return QString();

    return QString::fromStdString(res_str);
}
```

### 10.3. EvaluationOptions — настройка вычислений

```cpp
EvaluationOptions eo;
eo.approximation      = APPROXIMATION_APPROXIMATE;   // или EXACT
eo.allow_complex      = true;                        // разрешить комплексные числа
eo.parse_unknowns     = true;                        // неизвестные как переменные
eo.expand             = false;                       // не раскрывать скобки
eo.structuring        = STRUCTURING_NONE;            // без упрощения структуры
```

**Типичные проблемы:**
- `appoximation = APPROXIMATION_EXACT` может вернуть корень из 2 как `sqrt(2)` вместо `1.414...`
- `parse_unknowns = false` вызовет ошибку для неизвестных переменных
- `allow_complex = false` вернёт ошибку для `sqrt(-1)`

### 10.4. PrintOptions — настройка форматирования

```cpp
PrintOptions po;
po.number_base            = BASE_DECIMAL;            // система счисления
po.min_exp                = -6;                      // порог для экспон. записи
po.use_ending_zeroes      = false;                   // убрать лишние нули
po.interval_display       = INTERVAL_DISPLAY_MIDPOINT; // midpoint для интервалов
po.indicate_infinite_series = false;                 // не показывать "..." для рядов
```

### 10.5. Парсинг результата с учётом локали

```cpp
// libqalculate уважает системную локаль — может вернуть "3,14" вместо "3.14"
// и "100 см" вместо "100 cm"
double parseNumeric(const QString &str, bool *ok = nullptr)
{
    QString normalized = str;
    if (normalized.contains(',') && !normalized.contains('.')) {
        normalized.replace(',', '.');
    }
    return normalized.toDouble(ok);
}
```

### 10.6. Типовые сценарии отладки

#### Сценарий А: Плагин не появляется в KRunner

```bash
# Проверить, что qalc доступен
which qalc && qalc --version

# Проверить, что библиотека установлена
ldconfig -p | grep qalculate

# Перезапустить KRunner
pkill -x krunner

# Проверить логи KRunner
journalctl -f | grep -iE 'krunner|qalc|calculator'
```

#### Сценарий Б: Вычисление возвращает неожиданный результат

```cpp
// Изолированная проверка через libqalculate C++ API:
std::string expr = CALCULATOR->unlocalizeExpression("2,5+2,5");
// → "2.5+2.5"

MathStructure mstruct;
EvaluationOptions eo;
bool success = CALCULATOR->calculate(&mstruct, expr, 5000, eo);
// success == true → mstruct содержит 5

PrintOptions po;
std::string res = CALCULATOR->print(mstruct, 5000, po);
// → "5" (или "5,0" в зависимости от локали)
```

#### Сценарий В: Ошибка "expression returns itself"

Если выражение ввода и результат совпадают, `calculate()` возвращает пустую строку:

```cpp
// Например: ввод "5" → unlocalize → "5" → calculate → 5 → print → "5"
// res_str ("5") == expr ("5") → return QString()
// ИЛИ: "pi to pi" → конвертация пи в пи → результат не меняется
```

Это корректное поведение: не показываем "5" в ответ на "5".

#### Сценарий Г: Краш при запуске тестов (SIGSEGV)

```bash
# 1. Проверить версию libqalculate
qalc --version

# 2. Убедиться, что Calculator проинициализирован
# CALCULATOR == nullptr → crash при первом calculate()

# 3. Запустить тест под valgrind
valgrind --leak-check=full ./build_test/bin/test_qalculatorrunner

# 4. Собрать с -g и запустить под gdb
cd build_test && cmake .. -DCMAKE_BUILD_TYPE=Debug && make
gdb -ex run -ex bt ./bin/test_qalculatorrunner
```

#### Сценарий Д: Проблемы с locale (запятая вместо точки)

```cpp
// Проверка локали
QLocale locale;
qDebug() << "Decimal point:" << locale.decimalPoint();      // '.' или ','
qDebug() << "Group separator:" << locale.groupSeparator(); // ' ' или ','

// libqalculate уважает LC_NUMERIC
// Для тестов в нейтральной локали:
setlocale(LC_NUMERIC, "C");
```

#### Сценарий Е: Тесты не проходят после изменений

```bash
# 1. Полная пересборка
cd build_test && cmake .. -DBUILD_TESTING=ON && make -j$(nproc)

# 2. Запуск с детальным выводом
./bin/test_qalculatorrunner -v2

# 3. Если зависает — увеличить TIMEOUT_MS или проверить loadExchangeRates
# 4. Если падает — сверить ожидания с фактическим выводом libqalculate:
qalc --defaults -e -t '+u8' "2+2"
```

### 10.7. Проверка libqalculate через CLI (быстрая отладка)

```bash
# Простое выражение
qalc --defaults -e -t '+u8' "2+2"

# Функции
qalc --defaults -e -t '+u8' "sqrt(16)"

# Конвертация единиц
qalc --defaults -e -t '+u8' "100 km/h to mph"

# Переменные и константы
qalc --defaults -e -t '+u8' "pi"
qalc --defaults -e -t '+u8' "sin(pi/2)"

# Смена локали для теста
LC_NUMERIC=ru_RU.UTF-8 qalc --defaults -e -t '+u8' "2.5+2.5"
LC_NUMERIC=C qalc --defaults -e -t '+u8' "2,5+2,5"
```

### 10.8. Архитектура libqalculate (ключевые типы)

| Тип | Назначение |
|---|---|
| `Calculator` | Главный класс, синглтон. Методы: `calculate()`, `print()`, `unlocalizeExpression()` |
| `MathStructure` | Представление математического выражения (дерево). Результат вычисления. |
| `EvaluationOptions` | Настройки вычисления: точность, комплексные числа, упрощение |
| `PrintOptions` | Настройки форматирования вывода: система счисления, интервалы |
| `Number` | Произвольная точность числа. Используется внутри `MathStructure`. |
| `ExpressionItem` | Базовый класс для функций, переменных, единиц |
| `Function` / `Variable` / `Unit` | Конкретные типы выражений |
| `BuiltinFunctions` | Коллекция встроенных функций |
| `DataSet` | Датасеты (валюты, физические константы) |

---

## 11. Статический анализ

### 11.1. Настройка (`.clang-tidy`)

Файл `.clang-tidy` в корне проекта определяет проверки для clang-tidy.

```yaml
Checks: >
  -*,
  bugprone-*,
  clang-analyzer-*,
  modernize-*,
  performance-*,
  readability-*,
  -readability-identifier-naming,          # слишком строго для Qt-кода
  -readability-magic-numbers,              # шум для числовых констант
  -readability-function-cognitive-complexity,
  -readability-else-after-return,
  -modernize-concat-nested-namespaces,      # C++17 не требует
  -modernize-use-trailing-return-type,      # стиль KDE, не auto
  -bugprone-easily-swappable-parameters,    # шум для маленького проекта
  -readability-redundant-access-specifiers, # Qt MOC: public + public Q_SLOTS
  -readability-identifier-length,           # eo, po, ok — конвенция libqalculate
  -readability-convert-member-functions-to-static, # Qt slots-паттерны

WarningsAsErrors: >
  clang-analyzer.core.*,          # core analyzer → ошибка сборки
  clang-analyzer.cplusplus.*,

FormatStyle: none

CheckOptions:
  readability-implicit-bool-conversion.AllowPointerConditions: true
```

**Подавленные проверки (и почему):**

| Проверка | Причина подавления |
|---|---|
| `readability-redundant-access-specifiers` | Qt MOC требует `public Q_SLOTS:` после `public:` — с точки зрения C++ это избыточно, но MOC без этого не работает |
| `readability-identifier-length` | `eo`/`po` — стандартные имена в libqalculate API; `ok` — общепринятый паттерн `bool *ok` в Qt |
| `readability-convert-member-functions-to-static` | Методы с `Q_OBJECT`/слоты должны быть нестатическими для корректной работы MOC-генерации |
| `readability-magic-numbers` | Слишком шумно для числовых констант в конвертации единиц, таймаутов и т.п. |

### 11.2. Запуск в CI

В `.github/workflows/ci.yml` добавлены два шага статического анализа:

```yaml
- name: Static analysis — clang-tidy
  run: |
    find src/ tests/ -name '*.cpp' -o -name '*.h' \
      | xargs clang-tidy -p build --quiet 2>&1 || true
  continue-on-error: true

- name: Static analysis — cppcheck
  run: |
    cppcheck --enable=all \
             --std=c++17 \
             --suppress=syntaxError \
             --suppress=unknownMacro \
             --suppress=unmatchedSuppression \
             --inline-suppr \
             --quiet \
             --language=c++ \
             -I src/ -I tests/ \
             src/ tests/ 2>&1 || true
  continue-on-error: true
```

Оба шага используют `continue-on-error: true` — предупреждения не ломают CI, но видны в логах.

**Ограничения:**
- В режиме `BUILD_ONLY_TESTS=ON` `compile_commands.json` содержит только записи для тестовых файлов. Clang-tidy для `src/` файлов выдаст ошибки (`unable to handle compilation`), но CI не упадёт.
- Cppcheck работает без `compile_commands.json`, поэтому анализирует все файлы независимо от режима.

### 11.3. Локальный запуск

```bash
# Test-only сборка (без KF6) с compile_commands.json для clang-tidy
mkdir -p build_ci && cd build_ci
cmake .. -DBUILD_ONLY_TESTS=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build . -j$(nproc)

# clang-tidy: только наши файлы (без build_ci/)
find src/ tests/ -name '*.cpp' -o -name '*.h' \
  | xargs clang-tidy -p . --quiet 2>&1

# cppcheck (не нужен compile_commands.json)
cppcheck --enable=all --std=c++17 \
  --suppress=syntaxError \
  --suppress=unknownMacro \
  --suppress=unmatchedSuppression \
  --inline-suppr --quiet \
  --language=c++ \
  -I src/ -I tests/ \
  src/ tests/ 2>&1

# Запуск CI локально через act (требуется Docker)
# Установка: curl -sSf https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash
act push --container-architecture linux/amd64 -j build
```

### 11.4. Типовые проблемы, которые находит анализ

| Проблема | Пример | Фикс |
|---|---|
| Деструктор без `= default` | `~Foo() {}` | `~Foo() = default;` |
| Пропущенные скобки у `if` | `if (x) return;` | `if (x) { return; }` |
| `return QString()` вместо `{}` | `return QString();` | `return {};` |
| Мёртвые `#include` | `#include <QProcess>` (не используется) | Удалить |
| Сужение типа (narrowing) | `qsizetype` → `int` | Явный `static_cast` |

### 11.5. Интеграция с `.clang-format`

- `.clang-format` — автоматическое форматирование (стиль Allman, 4 пробела, указатели справа)
- `.clang-tidy` — статический анализ (баги, современный C++, производительность)
- Оба файла в корне проекта, применяются автоматически при вызове `clang-format`/`clang-tidy` из корня

---

## 12. Типовые команды для быстрого старта

```bash
# Сборка с тестами (полная, с KF6)
mkdir -p build_test && cd build_test
cmake .. -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
make -j$(nproc)

# Сборка только тестов (без KF6, для CI)
mkdir -p build_ci && cd build_ci
cmake .. -DBUILD_ONLY_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)

# Запуск тестов
./bin/test_qalculatorrunner -v2

# Форматирование кода
find src/ tests/ -name '*.cpp' -o -name '*.h' | xargs clang-format -i -style=file

# Статический анализ (см. раздел 11)
find src/ tests/ -name '*.cpp' -o -name '*.h' | xargs clang-tidy -p build_ci --quiet 2>&1

# Установка плагина
./install.sh

# Удаление
./uninstall.sh
```
