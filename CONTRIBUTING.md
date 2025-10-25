# Contributing to Automated Watering System

Thank you for your interest in contributing to the Automated Watering System project! This document provides guidelines and instructions for contributing.

## Table of Contents
- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Development Setup](#development-setup)
- [Coding Guidelines](#coding-guidelines)
- [Submitting Changes](#submitting-changes)
- [Reporting Bugs](#reporting-bugs)
- [Suggesting Features](#suggesting-features)

## Code of Conduct

### Our Pledge
We are committed to providing a welcoming and inspiring community for all. Please be respectful and constructive in all interactions.

### Expected Behavior
- Use welcoming and inclusive language
- Be respectful of differing viewpoints and experiences
- Gracefully accept constructive criticism
- Focus on what is best for the community
- Show empathy towards other community members

## How Can I Contribute?

### Types of Contributions

1. **Bug Reports** - Help us identify and fix issues
2. **Feature Requests** - Suggest new features or improvements
3. **Code Contributions** - Submit bug fixes or new features
4. **Documentation** - Improve or translate documentation
5. **Testing** - Test new features and report results
6. **Hardware Designs** - Share alternative hardware configurations

## Development Setup

### Prerequisites
- PlatformIO installed
- Git installed
- ESP32-S3 board (for hardware testing)
- Basic understanding of C++ and Arduino framework

### Setup Steps

1. **Fork the Repository**
   ```bash
   # Fork on GitHub, then clone your fork
   git clone https://github.com/YOUR_USERNAME/automated-watering-system-esp32.git
   cd automated-watering-system-esp32
   ```

2. **Add Upstream Remote**
   ```bash
   git remote add upstream https://github.com/Jockey12/automated-watering-system-esp32.git
   ```

3. **Create Development Branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

4. **Install Dependencies**
   ```bash
   platformio pkg install
   ```

5. **Configure for Testing**
   - Copy `src/config.h.example` to `src/config.h`
   - Update WiFi credentials
   - Adjust pin assignments if needed

## Coding Guidelines

### C++ Style Guide

#### Naming Conventions
```cpp
// Classes: PascalCase
class WateringZone { };

// Functions: camelCase
void startWatering() { }

// Variables: camelCase
int zoneCount = 0;

// Constants: UPPER_SNAKE_CASE
#define MAX_ZONES 4

// Private members: camelCase with descriptive names
private:
    uint8_t pinNumber;
    bool isActive;
```

#### Code Formatting
- Indentation: 2 spaces (no tabs)
- Line length: Maximum 100 characters
- Braces: Same line for functions and control structures
```cpp
void example() {
  if (condition) {
    // code
  } else {
    // code
  }
}
```

#### Comments
```cpp
// Single-line comments for brief explanations
int count = 0;  // Inline comment when needed

/**
 * Multi-line comments for functions and complex logic
 * Explain the "why" not the "what"
 */
void complexFunction() {
  // Implementation
}
```

### File Organization

```
src/
├── main.cpp              // Main application logic
├── config.h              // Configuration constants
├── WateringZone.h        // Class declarations
├── WateringZone.cpp      // Class implementations
└── [NewFeature].h/cpp    // New feature files
```

### Best Practices

1. **Keep Functions Small** - Each function should do one thing
2. **Error Handling** - Check for errors and handle gracefully
3. **Memory Management** - Be careful with dynamic memory
4. **Const Correctness** - Use `const` where appropriate
5. **Documentation** - Document public APIs and complex logic

### Arduino/ESP32 Specific

```cpp
// Prefer millis() over delay()
unsigned long lastUpdate = 0;
if (millis() - lastUpdate > 1000) {
  lastUpdate = millis();
  // Do periodic task
}

// Use proper pin modes
pinMode(pin, OUTPUT);
pinMode(pin, INPUT_PULLUP);

// Avoid String class for frequent operations
// Use char arrays or const char* instead
```

## Submitting Changes

### Branch Naming
- `feature/description` - New features
- `bugfix/description` - Bug fixes
- `docs/description` - Documentation changes
- `refactor/description` - Code refactoring

### Commit Messages

Follow the [Conventional Commits](https://www.conventionalcommits.org/) format:

```
type(scope): subject

body (optional)

footer (optional)
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Maintenance tasks

**Examples:**
```
feat(zones): add support for 8 zones

Extends the system to support up to 8 watering zones by
making the zone configuration more flexible.

Closes #42

fix(web): correct remaining time display

The remaining time was showing incorrect values due to
integer overflow. Fixed by using proper time calculation.

docs(readme): update wiring diagram

Added clearer labels and color coding to the wiring diagram
for easier understanding.
```

### Pull Request Process

1. **Update Your Branch**
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Test Your Changes**
   - Build successfully: `platformio run`
   - Test on hardware if possible
   - Verify no new warnings or errors
   - Check documentation is updated

3. **Create Pull Request**
   - Use a clear, descriptive title
   - Reference any related issues
   - Provide detailed description of changes
   - Include testing steps
   - Add screenshots for UI changes

4. **PR Template**
   ```markdown
   ## Description
   Brief description of changes
   
   ## Type of Change
   - [ ] Bug fix
   - [ ] New feature
   - [ ] Documentation update
   - [ ] Code refactoring
   
   ## Testing
   - [ ] Builds successfully
   - [ ] Tested on hardware
   - [ ] Documentation updated
   - [ ] No new warnings
   
   ## Related Issues
   Closes #123
   
   ## Screenshots (if applicable)
   ```

5. **Code Review**
   - Address reviewer feedback
   - Make requested changes
   - Update PR as needed
   - Be responsive and courteous

## Reporting Bugs

### Before Submitting
- Check existing issues to avoid duplicates
- Test with latest version
- Gather relevant information

### Bug Report Template

```markdown
**Description**
Clear description of the bug

**To Reproduce**
1. Go to '...'
2. Click on '...'
3. Scroll down to '...'
4. See error

**Expected Behavior**
What you expected to happen

**Actual Behavior**
What actually happened

**Environment**
- Hardware: ESP32-S3 N16R8
- Firmware Version: v1.0.0
- Browser: Chrome 120
- OS: Windows 11

**Serial Output**
```
Paste relevant serial output here
```

**Screenshots**
If applicable, add screenshots

**Additional Context**
Any other relevant information
```

## Suggesting Features

### Feature Request Template

```markdown
**Is your feature request related to a problem?**
Clear description of the problem

**Describe the solution you'd like**
Clear description of desired solution

**Describe alternatives you've considered**
Alternative solutions or features

**Additional context**
Mockups, examples, or related projects

**Implementation Considerations**
- Hardware requirements
- Software dependencies
- Backward compatibility
- Performance impact
```

## Testing Guidelines

### Manual Testing
1. Test on actual hardware when possible
2. Verify all existing functionality still works
3. Test edge cases and error conditions
4. Document testing steps in PR

### Code Review Checklist
- [ ] Code follows style guidelines
- [ ] Comments are clear and helpful
- [ ] No unnecessary code duplication
- [ ] Error handling is appropriate
- [ ] Memory usage is reasonable
- [ ] Documentation is updated
- [ ] No hardcoded values (use config.h)
- [ ] Backward compatible (or breaking changes noted)

## Documentation

### When to Update Documentation
- Adding new features
- Changing existing behavior
- Adding new configuration options
- Fixing documentation errors

### Documentation Files
- `README.md` - Main documentation
- `HARDWARE_SETUP.md` - Hardware assembly
- `QUICKSTART.md` - Quick setup guide
- `TESTING.md` - Testing procedures
- `CHANGELOG.md` - Version history
- Code comments - Inline documentation

## Community

### Communication Channels
- GitHub Issues - Bug reports and feature requests
- GitHub Discussions - Questions and general discussion
- Pull Requests - Code contributions

### Getting Help
- Check existing documentation first
- Search closed issues for similar problems
- Ask in GitHub Discussions
- Be patient and respectful

## Recognition

Contributors will be recognized in:
- CONTRIBUTORS.md file
- Release notes
- Project README

Thank you for contributing to make this project better! 🌱
