# Contributing to ToothDroid

First off, thank you for considering contributing to ToothDroid! 🎉

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Development Setup](#development-setup)
- [Branch Naming](#branch-naming)
- [Commit Messages](#commit-messages)
- [Code Style](#code-style)
- [Pull Request Process](#pull-request-process)

---

## Code of Conduct

This project follows the [Contributor Covenant](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to the maintainers.

---

## How Can I Contribute?

### 🐛 Reporting Bugs

Before creating a bug report:
1. Check if the issue already exists
2. Try the latest version
3. Collect information: OS, Qt version, BlueZ version

When reporting:
- Use a clear, descriptive title
- Describe steps to reproduce
- Include expected vs actual behavior
- Add screenshots if applicable

### 💡 Suggesting Features

Feature requests are welcome! Please:
- Explain the use case
- Describe the expected behavior
- Consider if it fits the project's scope (lightweight, native Linux tool)

### 🔧 Code Contributions

Great areas to contribute:

| Area | Difficulty | Examples |
|------|------------|----------|
| Documentation | 🟢 Easy | README improvements, tutorials |
| Tests | 🟢 Easy | Unit tests for BluetoothDevice |
| UI Polish | 🟡 Medium | Theme tweaks, animations |
| New Features | 🟠 Hard | Battery display, system tray |
| Core | 🔴 Expert | BlueZ D-Bus direct integration |

---

## Development Setup

### Prerequisites

```bash
# Arch Linux
sudo pacman -S bluez bluez-utils base-devel qt6-base clang-format

# Fedora
sudo dnf install bluez bluez-tools gcc-c++ make qt6-qtbase-devel clang-tools-extra

# Ubuntu/Debian
sudo apt install bluez build-essential qt6-base-dev clang-format
```

### Build

```bash
git clone https://github.com/suaybdemir/bluetooth-connector-linux.git
cd bluetooth-connector-linux

# Debug build
make debug

# Run tests (when available)
make test

# Check code style
make lint
```

---

## Branch Naming

Use the following prefixes:

| Prefix | Usage | Example |
|--------|-------|---------|
| `feature/` | New features | `feature/battery-display` |
| `fix/` | Bug fixes | `fix/scan-crash` |
| `docs/` | Documentation | `docs/arch-install-guide` |
| `refactor/` | Code refactoring | `refactor/bluetooth-manager` |
| `test/` | Test additions | `test/device-parsing` |

---

## Commit Messages

We follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

### Types

| Type | Description |
|------|-------------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation only |
| `style` | Code style (formatting, no logic change) |
| `refactor` | Code change that neither fixes nor adds |
| `test` | Adding or fixing tests |
| `chore` | Build, CI, dependencies |

### Examples

```
feat(gui): add battery level indicator

fix(scan): handle empty device names gracefully

docs(readme): add Fedora installation instructions

refactor(manager): extract device parsing to separate function
```

---

## Code Style

### C++ Guidelines

- **Standard**: C++17
- **Formatting**: Use `clang-format` with the project's `.clang-format`
- **Naming**:
  - Classes: `PascalCase` (e.g., `BluetoothDevice`)
  - Functions: `camelCase` (e.g., `scanDevices`)
  - Variables: `camelCase` (e.g., `deviceList`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_SCAN_TIME`)
  - Private members: `m_` prefix (e.g., `m_deviceList`)

### Before Committing

```bash
# Format code
make format

# Run linter
make lint
```

---

## Pull Request Process

1. **Fork** the repository
2. **Create** a feature branch from `main`
3. **Make** your changes
4. **Test** locally with `make debug && make run-gui`
5. **Format** code with `make format`
6. **Commit** with conventional commit messages
7. **Push** to your fork
8. **Open** a Pull Request

### PR Checklist

- [ ] Code compiles without warnings
- [ ] Follows code style guidelines
- [ ] Documentation updated if needed
- [ ] Tested on at least one Linux distribution
- [ ] Commit messages follow convention

---

## Questions?

Feel free to open an issue for any questions. We're happy to help newcomers!

---

Thank you for contributing! 🦷
