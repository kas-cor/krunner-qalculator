# Code Review: krunner-qalculator

> **Источник:** автоматический code review через kanban (воркер `deep-reasoner`, задача `t_fd5f2868`)
> **Дата отчёта:** 2026-08-02 · **Дата исправления:** 2026-08-03
> 🔴 Critical: 2 · ⚠️ Warnings: 13 · 💡 Suggestions: 10

---

Verdict: Проект в целом добротный для своего размера, но содержит 2 критических бага (один — логическая ошибка, второй — inconsistency в защите от инъекций), 13 предупреждений и 10 предложений по улучшению. Тесты — слабое место: они тестируют не плагин, а библиотеку Qt и утилиту qalc напрямую.

Структура


krunner-qalculator/
├── src/
│   ├── qalculatorrunner.h        (50 строк)  — объявление класса KRunner-плагина
│   ├── qalculatorrunner.cpp      (151 строка) — реализация: match/run/calculate
│   ├── CMakeLists.txt            (64 строки)  — сборка плагина
│   └── manifest.json             (25 строк)   — метаданные KPlugin
├── tests/
│   ├── test_qalculatorrunner.cpp (105 строк)  — тесты (input validation + qalc)
│   ├── CMakeLists.txt            (51 строка)  — сборка тестов
│   ├── Makefile                  (398 строк)  — сгенерирован CMake (чужой машины!)
│   ├── CTestTestfile.cmake       — сгенерирован
│   └── *_autogen/                — сгенерированы MOC
├── install.sh                    (71 строка)  — скрипт установки
├── uninstall.sh                  (13 строк)   — скрипт удаления
├── postinst / postrm             (6 строк)    — post-install/remove (идентичны!)
├── CMakeLists_root.txt           (18 строк)   — альтернативная точка входа (не используется)
├── README.md / README_RU.md      — документация EN/RU
├── QWEN.md                       — AI-контекст (необычно для публичного репо)
├── LICENSE                       — GPL-2.0
├── .gitignore
└── qalculatorRunner.kdev4        — проект KDevelop 4 (устаревший формат)


Ядро: 2 source-файла (h + cpp), всё остальное — инфраструктура. Плагин вызывает qalc через QProcess, парсит stdout, возвращает результат в KRunner.



🔴 Critical (2)

1. Q_UNUSED(context) при фактическом использовании context — src/qalculatorrunner.cpp:66,75,78

Проблема: В методе run() параметр context помечен как Q_UNUSED(context) на строке 66, но затем используется на строках 75 и 78:
cpp
context.requestQueryStringUpdate(QString(), 0);       // line 75
context.requestQueryStringUpdate(result, result.length()); // line 78

Q_UNUSED раскрывается в (void)context; — это не ломает компиляцию, но является грубой логической ошибкой. Макрос существует чтобы подавлять warning «unused parameter» для параметров, которые действительно не используются. Здесь же параметр используется — макрос должен быть удалён. Если кто-то в будущем заменит Q_UNUSED на [[maybe_unused]] или удалит параметр из сигнатуры, код сломается.

Решение: Удалить строку Q_UNUSED(context).

2. Backtick разрешён во втором валидаторе, но заблокирован в первом — src/qalculatorrunner.cpp:104,108

Проблема: Два слоя защиты от command injection противоречат друг другу:
- Строка 104: QRegularExpression("\\$\\(|") — блокирует backtick и $(` 
- Строка 108: ^[0-9a-zA-Z+\-*/()=.%,\[\]{}<>'"\!?:#@_\s]+$` — разрешает backtick

Если первый чек по какой-то причине будет обойдён или удалён при рефакторинге, второй слой молча пропустит backtick. Defense-in-depth должен быть консистентным: оба слоя должны блокировать одно и то же.

Решение: Убрать backtick из разрешённых символов в регулярном выражении на строке 108.



⚠️ Warnings (13)

3. waitForFinished() без явного таймаута — src/qalculatorrunner.cpp:130

Дефолтный таймаут QProcess::waitForFinished() — 30 секунд. Если qalc зависнет, KRunner будет заблокирован на полминуты. Для плагина, вызываемого при каждом нажатии клавиши, это критично.

Решение: Установить явный таймаут: waitForFinished(5000) (5 секунд).

4. manifest.json: MinimumQuery=2 vs код term.length() < 3 — manifest.json:20, qalculatorrunner.cpp:45

Манифест декларирует минимальную длину запроса = 2, но код требует ≥ 3 символов. Выражение =1 (2 символа с триггером) будет отклонено кодом, несмотря на разрешение в манифесте.

Решение: Привести в соответствие: либо MinimumQuery: 3 в манифесте, либо term.length() < 2 в коде.

5. install.sh: git reset --hard уничтожает локальные изменения — install.sh:10

Скрипт делает git reset --hard origin/$(git branch --show-current) без предупреждения. Если пользователь вносил правки в склонированный репо, они молча теряются.

Решение: Добавить git stash перед reset, либо проверку на git diff --quiet с предупреждением.

6. install.sh не проверяет наличие qalc — install.sh

Скрипт проверяет cmake и make, но не qalc — единственную runtime-зависимость. Плагин установится, но не заработает.

Решение: Добавить проверку command -v qalc.

7. uninstall.sh не проверяет существование build/ — uninstall.sh:7

cd build без проверки, что директория существует. При отсутствии — неинформативная ошибка «No such file or directory».

Решение: Добавить [ -d build ] || { echo "Build directory not found"; exit 1; }.

8. postinst и postrm идентичны — postinst, postrm

Оба файла содержат одинаковый код (6 строк): определение версии krunner + kquitapp. postrm должен выполнять cleanup после удаления, а не только перезапускать krunner.

Решение: В postrm добавить удаление установленных файлов, если они не удалены пакетным менеджером. Как минимум — сделать разное содержимое.

9. tests/CMakeLists.txt дублирует сборку библиотеки — tests/CMakeLists.txt:34-48

qalculatorrunner_lib компилирует ../src/qalculatorrunner.cpp повторно, отдельно от основной сборки плагина. Это двойная компиляция + потенциальные ODR-нарушения.

Решение: Сделать библиотеку общей (shared/static library в src/CMakeLists.txt) и линковать тест с ней, а не перекомпилировать исходник.

10. testBasicCalculation тестирует qalc, а не плагин — test_qalculatorrunner.cpp:54-71

Тест запускает qalc напрямую через QProcess и проверяет, что 2+2 даёт 4. Это тест утилиты qalc, а не плагина. Метод QalculatorRunner::calculate() не тестируется вообще.

Решение: Создать mock для QProcess или тестировать calculate() через изолированное инстанцирование (с минимальными KDE-зависимостями).

11. testErrorHandling без assert-ов — test_qalculatorrunner.cpp:88-102

Тест запускает qalc с некорректным выражением, читает результат... и ничего не проверяет. Комментарий «Result might be empty or contain an error message» — тест-пустышка.

Решение: Добавить assert: QVERIFY(result.isEmpty() || result.contains("error")); или проверять exitCode.

12. testInputValidation тестирует QString::contains(), а не плагин — test_qalculatorrunner.cpp:38-52

Тест проверяет, что QStringLiteral("2 + 2; rm -rf /").contains(QStringLiteral(";")) возвращает true. Это тест библиотеки Qt, а не плагина. Валидация плагина (calculate()) не вызывается.

Решение: Инстанцировать QalculatorRunner (хотя бы с минимальными зависимостями) и вызывать calculate() с инъекционными строками, проверяя что возвращается пустой результат.

13. manifest.json лицензия LGPL-2.1+, исходники GPL-2.0 — manifest.json:14

Манифест заявляет "License": "LGPL-2.1+", но заголовки исходников и файл LICENSE указывают GPL-2.0. LGPL-2.1+ разрешает линковку с проприетарным кодом, GPL-2.0 — нет. Это юридически значимое расхождение.

Решение: Привести манифест в соответствие с реальной лицензией: "License": "GPL-2.0" или "License": "GPL-2.0+".

14. Сборочные артефакты в репозитории — tests/Makefile, tests/*_autogen/, tests/CTestTestfile.cmake

Сгенерированные CMake файлы (включая Makefile с хардкод-путями /home/kas-cor/projects/krunner_qalc/) закоммичены. Они не должны быть в VCS.

Решение: Добавить в .gitignore: tests/Makefile, tests/*_autogen/, tests/CTestTestfile.cmake, tests/cmake_install.cmake. Удалить уже закоммиченные.

15. install.sh: kquitapp$krunner_version с пустой версией — install.sh:68

Если krunner не установлен, krunner_version пуст, и команда становится kquitapp krunner — упадёт с ошибкой. Скрипт продолжит выполнение из-за отсутствия set -e в этой секции (контекст: set -e на строке 4, но он не покрывает все ветки).

Решение: Проверить [ -n "$krunner_version" ] перед вызовом kquitapp.



💡 Suggestions (10)

16. CMakeLists_root.txt — мёртвая точка входа

Файл не называется CMakeLists.txt, поэтому CMake его не обнаружит. install.sh использует cmake ../src/. Файл либо должен быть переименован и использован, либо удалён.

17. src/CMakeLists.txt:64 — add_subdirectory(../tests ../tests)

Использование .. в add_subdirectory — хрупкий паттерн. CMakeLists_root.txt делает это чище: add_subdirectory(tests). Стоит унифицировать.

18. Тесты линкуются с KF6::Runner без необходимости

test_qalculatorrunner.cpp не использует KRunner API (не инстанцирует QalculatorRunner). Зависимость от KF6::Runner, KF6::I18n, KF6::CoreAddons в тестах избыточна.

19. match() выставляет relevance = 1.0 безусловно — qalculatorrunner.cpp:52

Любое успешное вычисление получает максимальную релевантность. Если пользователь вводит the (3 символа), плагин попытается вычислить это через qalc и, при успехе, перекроет все остальные результаты KRunner.

Решение: Проверять, похож ли запрос на математическое выражение (содержит цифры, операторы), перед вызовом qalc. Понижать релевантность для тривиальных/неуверенных результатов.

20. match() не фильтрует нематематические запросы — qalculatorrunner.cpp:43-61

Любая строка длиной ≥ 3 символа передаётся в calculate(). Это лишние процессы qalc на каждый нефункциональный ввод.

Решение: Добавить быструю эвристику: запрос должен содержать хотя бы одну цифру или оператор.

21. setData("copy") — мёртвый код — qalculatorrunner.cpp:55

match.setData(QStringLiteral("copy")) устанавливает данные, которые нигде не читаются. run() использует match.selectedAction().id() для определения действия, а не match.data().

Решение: Удалить строку.

22. QWEN.md — AI-контекст в публичном репо

Файл содержит инструкции для AI-ассистента Qwen. Нестандартно для публичного репозитория. Если это для личного использования — ок. Если для широкой аудитории — стоит убрать.

23. qalculatorRunner.kdev4 — устаревший формат

KDevelop 4 использовал .kdev4 (XML-based). KDevelop 5+ использует .kdev5 (JSON). Файл можно удалить или обновить.

24. Copyright placeholder "Your Name" — test_qalculatorrunner.cpp:2

В тестовом файле не заполнен копирайт: Copyright (C) 2025 Your Name.

25. calculate() не читает stderr при успешном exitCode — qalculatorrunner.cpp:136-141

Если qalc завершается с кодом 0, но пишет предупреждения в stderr, они теряются. Может быть полезно для отладки.



✅ Что сделано отлично

- Двухслойная защита от инъекций — правильный подход, даже с учётом inconsistency из Critical #2
- QProcess вместо system() — бинарный вызов без shell, основная защита от инъекций
- Проверка clipboard != nullptr в copyToClipboard() — корректная обработка граничного случая
- Обработка ошибок QProcess — проверка waitForStarted(), waitForFinished(), exitCode()
- Логирование через qWarning() — информативные сообщения при сбоях
- Чистый C++17, KDE-идиоматика — правильное использование K_PLUGIN_CLASS_WITH_JSON, QStringLiteral, QLatin1String
- Интернационализация — i18n() для пользовательских строк
- Хорошая документация — README на двух языках, описание архитектуры
- Наличие тестов — даже с оговорками, сам факт их существования положителен



Итог

| Категория      | Количество |
|----------------|------------|
| 🔴 Critical    | 2          |
| ⚠️ Warnings    | 13         |
| 💡 Suggestions | 10         |
| ✅ Positive    | 9          |

Приоритет исправлений:
1. Немедленно: Critical #1 (Q_UNUSED(context)) — баг, ломающий семантику кода
2. Срочно: Critical #2 (backtick в валидаторе) — inconsistency в защите
3. До релиза: Warnings #3 (таймаут QProcess), #4 (MinimumQuery mismatch), #13 (лицензия)
4. Техдолг: Warnings #9–#12 (тесты не тестируют плагин), #14 (артефакты в репо)
5. Nice to have: Suggestions #19–#20 (фильтрация нематематических запросов)

  ┊ 💬 preparing
---

## Статус исправления (kanban)

Исправлены 2 critical в `qalculatorrunner.cpp`: удалён ложный Q_UNUSED(context); backtick заблокирован в обоих валидаторах.

> Изменения внесены воркером (`t_c9b07137`) в рабочую директорию проекта — **не закоммичены**.
