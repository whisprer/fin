#!/usr/bin/env python3
"""
ProcWolf Modular Structure Overview
----------------------------------
This is a guide for splitting proc_wolf.py into smaller, more manageable modules.
"""

# Proposed Module Structure:
# --------------------------
#
# 1. proc_wolf/            # Main package directory
#    ├── __init__.py       # Package initialization, exports main functions
#    ├── core.py           # Core functionality, entry points, main monitoring loop
#    ├── config.py         # Configuration, constants, and globals
#    ├── logging_setup.py  # Logging configuration
#    ├── whitelist.py      # SecureWhitelist and whitelist management
#    ├── database.py       # Database operations
#    ├── detection.py      # Process detection and threat assessment
#    ├── actions.py        # Process actions (kill, nuke, etc.)
#    ├── utils/            # Utility functions
#    │   ├── __init__.py
#    │   ├── system.py     # System-related utilities
#    │   ├── security.py   # Security-related utilities
#    │   └── process.py    # Process-related utilities
#    └── cli/              # Command-line interface components
#        ├── __init__.py
#        ├── commands.py   # CLI command implementations
#        └── ui.py         # User interface components

# Implementation Steps:
# --------------------
# 1. Create the directory structure
# 2. Create empty files with proper imports
# 3. Move code from proc_wolf.py to the appropriate modules
# 4. Update imports and fix any references
# 5. Test each module individually
# 6. Test the whole system

# Best Practices for Modularization:
# ---------------------------------
# 1. Keep related functionality together
# 2. Minimize circular dependencies
# 3. Use proper imports (relative or absolute)
# 4. Maintain a clean interface between modules
# 5. Document module purposes and interfaces
# 6. Use type hints for better IDE support
# 7. Write unit tests for each module

Main Package Structure

proc_wolf/__init__.py - Package initialization and exports
proc_wolf/config.py - Configuration, constants, and globals
proc_wolf/logging_setup.py - Logging configuration
proc_wolf/whitelist.py - SecureWhitelist implementation
proc_wolf/database.py - SQLite database operations
proc_wolf/detection.py - Process detection and threat assessment
proc_wolf/actions.py - Process action implementations (kill, nuke)
proc_wolf/core.py - Main monitoring functionality

Utility Modules

proc_wolf/utils/system.py - System-related utilities
proc_wolf/utils/security.py - Security-related utilities
proc_wolf/utils/process.py - Process-related utilities
proc_wolf/utils/__init__.py - Utility package initialization

Documentation

README.md - Complete project documentation
requirements.txt - Required dependencies

The modular design offers several benefits:

Improved maintainability - Each module has a focused purpose
Better testability - Modules can be tested independently
Enhanced readability - Smaller files are easier to understand
Future extensibility - Adding new features is simpler


Updates to Support Files

build_exe.py:

Modified to recognize the new directory structure
Updated how it adds data files to PyInstaller
Added proper handling of module paths


proc_wolf_background.py:

Updated imports to use the modular structure
Improved logging configuration


proc_wolf_service.py:

Adapted imports for the modular packages
Updated initialization to use the core module functions


proc_wolf_full_3-0.py (CLI):

Updated imports to use the structured modules
Maintained all existing functionality


deps.bat:

Added installation for additional dependencies (pystray/pillow)
Added directory structure creation for the modular layout


install.bat/uninstall.bat:

Fixed the filename (was "installl.bat" with 3 l's)
Added a consistent uninstall script



Directory Structure
The new directory structure should look like this:
proc-wolf/
├── proc_wolf/               # Main package
│   ├── __init__.py
│   ├── core.py
│   ├── config.py
│   ├── logging_setup.py
│   ├── whitelist.py
│   ├── database.py
│   ├── detection.py
│   ├── actions.py
│   └── utils/
│       ├── __init__.py
│       ├── system.py
│       ├── security.py
│       └── process.py
├── proc_wolf_background.py  # Background service (system tray)
├── proc_wolf_service.py     # Windows service
├── proc_wolf_full_3-0.py    # CLI interface
├── build_exe.py             # Build script
├── create_icon.py           # Icon creator
├── deps.bat                 # Dependency installer
├── install.bat              # Service installer
└── uninstall.bat            # Service uninstaller