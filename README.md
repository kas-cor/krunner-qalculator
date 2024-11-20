# KRunner Qalculator Plugin

*Read this in other languages: [English](README.md), [Русский](README_RU.md)*

A powerful calculator plugin for KDE Plasma's KRunner, leveraging the advanced capabilities of libqalculate.

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

## Features

- Arbitrary precision calculations
- Support for solving algebraic equations
- Symbolic computation for exact results
- Currency conversion support
- Unit conversion capabilities
- Advanced mathematical functions

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
   git clone https://github.com/your-username/krunner_qalc.git
   cd krunner_qalc
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

## Acknowledgments

- Thanks to the KDE community for KRunner
- Thanks to the libqalculate team for their powerful calculation engine
