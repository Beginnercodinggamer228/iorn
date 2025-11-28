# Change Log

## [1.0.0] - 2025-11-18

### Added
- Initial release of IORN Language Support
- Syntax highlighting for .iorn files
- File icons for light and dark themes
- Code snippets for common IORN constructs
- Commands for compiling and running IORN files
- Support for all IORN language features:
  - Variable declarations (numeric, string, floating, boolean)
  - Conditional statements (if/else/endif)
  - String interpolation with $[variable]
  - Comments (4 different styles)
  - Import statements
  - Built-in functions (Print, input)

### Features
- Complete syntax highlighting
- IntelliSense support
- Code snippets
- File icons
- Context menu integration
- Command palette integration
- 
## [1.0.1 - 1.0.3] - 2025-11-20

### Rename
- fix md file
## [1.0.4] - 2025-11-24

### Fixed
- Fixed the issue with multithreading - earlier: `input➜if➜print`, now: `syntax by code (if the output comes first and then the input, then the code will work line by line, performing the action specified by the user)`