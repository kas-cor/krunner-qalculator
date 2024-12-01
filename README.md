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

## What's New in 2.0.1

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
- `libqalculate` (qalc command must be available in $PATH)
- KRunner development files

## Installation

1. Ensure all dependencies are installed
2. Clone the repository:
   ```bash
   git clone https://github.com/kas-cor/krunner-qalculator
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
- Equations: `solve x^2 + 2x + 1 = 0`

## Troubleshooting

1. If KRunner doesn't show the plugin:
   - Make sure KRunner is restarted after installation
   - Check if qalc is installed and available in PATH

2. If calculations don't work:
   - Verify libqalculate is properly installed
   - Check the syntax of your expression

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the GNU General Public License v2.0 - see the [LICENSE](LICENSE) file for details.

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
