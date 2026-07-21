# KRunner Qalculator Plugin

*Read this in other languages: [English](README.md), [Русский](README_RU.md)*

A powerful calculator plugin for KDE Plasma's KRunner, leveraging the advanced capabilities of libqalculate.

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![Donate Crypto](https://img.shields.io/badge/Donate-Cryptocurrency-orange.svg)](https://bit.ly/3uVaKEu)

## Features

- Arbitrary precision calculations
- Support for solving algebraic equations
- Symbolic computation for exact results
- Currency conversion support
- Unit conversion capabilities
- Advanced mathematical functions
- Quick result insertion into KRunner query line
- Copy results to clipboard with a single click

## What's New

### 2.1.0

- **Rewritten tests**: now use the libqalculate C++ API directly instead of the qalc CLI — faster, more reliable, and testing the actual code path
- **Documentation overhaul**: added `AGENTS.md` with full e2e codebase analysis, libqalculate API examples, and debugging scenarios
- **License sync**: all files now consistently declare GPL-2.0 (was LGPL-2.1+ in manifest)
- **CMake fixes**: resolved duplicate test subdirectory, added `enable_testing()` at top level

### 2.0.1

- Added ability to insert calculation results directly into KRunner query line
- Added copy button with icon for quick copying results to clipboard
- Improved UI/UX with standard KDE icons and actions

## Requirements

### System Requirements
- KDE Plasma 6.x
- Qt 6.x
- CMake
- Make
- sudo privileges for installation

### Dependencies
- `libqalculate-dev` (the library, not just the qalc CLI)
- KRunner development files (`kf6/libkf6runner-dev`)

## Installation

1. Ensure all dependencies are installed
2. Clone the repository:
   ```bash
   git clone https://github.com/kas-cor/krunner-qalculator.git
   cd krunner-qalculator
   ```
3. Run the installation script:
   ```bash
   ./install.sh
   ```

    The script will:
    - Configure the build environment
    - Compile the plugin
    - Install it to the system
    - Restart KRunner automatically
4. Disable the default calculator and unit converter plugins (recommended to reduce visual clutter)

## Building from Source (with Tests)

```bash
# Install build dependencies (Debian/Ubuntu)
sudo apt install cmake make g++ libkf6runner-dev libkf6coreaddons-dev libkf6i18n-dev libqalculate-dev

# Build
mkdir build_test && cd build_test
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug \
         -DBUILD_TESTING=ON -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
make -j$(nproc)

# Run tests
./bin/test_qalculatorrunner -v2
```

## Uninstallation

To remove the plugin:
```bash
./uninstall.sh
```

## Usage

1. Press `Alt+Space` to open KRunner
2. Type your mathematical expression
3. Press Enter to see the result

### Examples:
- Basic calculations: `2 + 2 =`
- Currency conversion: `100 USD to EUR =`
- Unit conversion: `100 km/h to mph =`
- Equations: `x^2 + 2x + 1 = 0`

## Troubleshooting

1. If KRunner doesn't show the plugin:
   - Make sure KRunner is restarted after installation (`pkill -x krunner`)
   - Check that `libqalculate` is properly installed (`ldconfig -p | grep qalculate`)
   - Verify the plugin is installed (`ls /usr/lib/qt6/plugins/kf6/krunner/krunner_qalculator.so`)

2. If calculations don't work:
   - Verify libqalculate is properly installed and up to date
   - Check that the Calculator definitions loaded correctly (journalctl)
   - Try the expression in `qalc --defaults -e -t '+u8' "2+2"` to isolate issues

3. If tests fail:
   - Ensure `libqalculate-dev` is installed
   - Build with `-DCMAKE_BUILD_TYPE=Debug` for detailed error messages
   - Run `./bin/test_qalculatorrunner -v2` to see which test fails

## For Developers

For a comprehensive end-to-end codebase analysis, libqalculate API examples, debugging scenarios, and static analysis setup, see [AGENTS.md](AGENTS.md).

Key sections:
- **Architecture & API** — sections 2, 10: how the plugin works, libqalculate API reference
- **Static analysis** — section 11: `.clang-tidy` config, CI checks, local run commands
- **Quick start** — section 12: build, test, format, and analyze commands

### How It Works

The plugin uses the **libqalculate C++ API directly** (not the qalc CLI tool):

1. User types an expression in KRunner → `QalculatorRunner::match()` is called
2. `unlocalizeExpression()` converts locale-specific formatting (commas → dots, etc.)
3. `Calculator::calculate()` evaluates the expression with a 2-second timeout
4. `Calculator::print()` formats the result with midpoint interval display
5. Result is shown in KRunner with an option to copy to clipboard or insert into the query line

### Project Structure

- `src/qalculatorrunner.cpp` — core plugin logic (match, run, calculate, clipboard)
- `src/qalculatorrunner.h` — class declaration and KPlugin registration
- `src/manifest.json` — KRunner plugin metadata (triggers, icon, min query length)
- `tests/test_qalculatorrunner.cpp` — unit tests using libqalculate C++ API
- `AGENTS.md` — detailed architecture, API reference, and debugging guide

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Build and run tests locally (`mkdir build_test && cd build_test && cmake .. -DBUILD_TESTING=ON && make && ./bin/test_qalculatorrunner`)
5. Push to the branch (`git push origin feature/amazing-feature`)
6. Open a Pull Request

## License

This project is licensed under the GNU General Public License v2.0 — see the [LICENSE](LICENSE) file for details.

## Support the Project

If you find this plugin useful and want to support its development, you can contribute in several ways:

### 💖 Cryptocurrency Donations

Support this project by donating cryptocurrency:
- Visit [our donation page](https://bit.ly/3uVaKEu) for BTC and USDT wallet addresses

### ⭐ Other Ways to Support
- Star this repository
- Share the project with others
- Submit bug reports or feature requests
- Contribute code or documentation improvements

## Acknowledgments

- Thanks to the KDE community for KRunner
- Thanks to the libqalculate team for their powerful calculation engine
