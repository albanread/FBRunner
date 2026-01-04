# FBRunner3 Application Module

This directory contains the refactored components of the FBRunner3 application, extracted from the original monolithic `main.mm` file.

## Overview

The FBRunner3 application has been organized into logical, maintainable modules:

```
Application/
├── FBRunner3App.h                      # Main application class interface
├── CommandRegistryInit.h/cpp           # Command registry initialization
├── FBRunner3App_CartCommands.inc       # Cart system REPL commands
├── FBRunner3App_InteractiveMode.inc    # Interactive shell/REPL
├── FBRunner3App_EditorSupport.inc      # Editor features (highlighting, line numbering)
└── FBRunner3App_ScriptExecution.inc    # Script compilation and execution
```

## Files

### FBRunner3App.h (31 lines)
**Purpose:** Interface declaration for the main application class

**Contains:**
- Property declarations (`scriptRunning`, `preferredWindowSize`)
- Public method signatures

**Usage:**
```objc
#import "Application/FBRunner3App.h"
```

---

### CommandRegistryInit.h/cpp (59 lines)
**Purpose:** Initialize the FasterBASIC command registry

**Contains:**
- `initializeFBRunner3CommandRegistry()` function
- Registration of core BASIC commands (math, string, file I/O)
- Registration of SuperTerminal graphical commands

**Key Function:**
```cpp
void initializeFBRunner3CommandRegistry();
```

**Must be called:** Before any FasterBASIC compiler operations

---

### FBRunner3App_CartCommands.inc (806 lines)
**Purpose:** Cart system REPL command handlers

**Contains:**
```objc
- (void)handleCartInfoCommand
- (void)handleSaveCartCommand
- (void)handleCloseCartCommand
- (void)handleUseCartCommand:(const std::string&)line
- (void)handleRunCartCommand:(const std::string&)line
- (void)handleCreateCartCommand:(const std::string&)line
- (void)handleSetInfoCommand:(const std::string&)line
- (void)handleListCartCommand:(const std::string&)line
- (void)handleCopyAssetCommand:(const std::string&)line
- (void)handleRenderCommand:(const std::string&)line
- (void)handlePlayCommand:(const std::string&)line
- (void)handleDeleteAssetCommand:(const std::string&)line
```

**Included from:** `main.mm` (line ~3340)

**Use Case:** Add new cart-related commands here

---

### FBRunner3App_InteractiveMode.inc (1,041 lines)
**Purpose:** Interactive shell/REPL mode functionality

**Contains:**

**Mode Management:**
```objc
- (void)enterInteractiveMode
- (void)exitInteractiveMode
- (void)toggleInteractiveMode
```

**Input/Output:**
```objc
- (void)updateInteractiveMode        // Input handling (286 lines)
- (void)renderInteractiveMode        // Display rendering
- (bool)executeInteractiveLine:(const std::string&)line  // Command execution (219 lines)
- (void)showPrompt
```

**Features:**
- Keyboard input handling
- Mouse selection and clipboard operations
- Command history navigation
- Auto-continuation mode for line numbering
- Text rendering with scrolling
- Syntax-highlighted output

**Included from:** `main.mm` (line ~2298)

**Use Case:** Modify interactive shell behavior, add REPL features

---

### FBRunner3App_EditorSupport.inc (529 lines)
**Purpose:** Editor support functions

**Contains:**

**Automatic Line Numbering (231 lines):**
```objc
- (void)setupLineNumbering
- (BOOL)isLineBlankOrOnlyNumber:(const std::string&)line
- (int)extractLineNumber:(const std::string&)line
- (void)renumberProgramFrom:(int)startLine step:(int)step
```

**Syntax Highlighting (292 lines):**
```objc
- (std::vector<uint32_t>)highlightLine:(const std::string&)line lineNumber:(size_t)lineNumber
- (void)extractUserDefinedConstants
```

**Features:**
- Auto-incrementing line numbers for BASIC
- Smart line number suggestions
- Syntax highlighting with color tokens
- User-defined constant detection
- Keyword recognition
- String and comment highlighting

**Included from:** `main.mm` (line ~433)

**Use Case:** Enhance syntax highlighting, modify line numbering behavior

---

### FBRunner3App_ScriptExecution.inc (795 lines)
**Purpose:** Script compilation and execution

**Contains:**

**BASIC Compilation (188 lines):**
```objc
- (std::string)compileBASICToLua:(const std::string&)basicSource error:(NSString**)error
```

**Script Loading (103 lines):**
```objc
- (BOOL)loadAndExecuteScript
```

**Script Execution (504 lines):**
```objc
- (void)runScript                                         // 76 lines
- (void)startScriptExecution:(const std::string&)scriptContent  // 70 lines
- (void)executeScriptContent:(const std::string&)content        // 160 lines
- (void)resetLuaState                                     // 95 lines
```

**Features:**
- FasterBASICT compiler integration
- Lua code generation
- LuaJIT execution
- Runtime module registration
- Error handling and reporting
- Script thread management

**Included from:** `main.mm` (line ~439)

**Use Case:** Modify compilation pipeline, add runtime features

---

## Integration

All `.inc` files are included via `#include` directives in `main.mm`:

```objc
// In main.mm:
#import "Application/FBRunner3App.h"
#import "Application/CommandRegistryInit.h"

// Later in @implementation:
#include "Application/FBRunner3App_EditorSupport.inc"
#include "Application/FBRunner3App_ScriptExecution.inc"
#include "Application/FBRunner3App_InteractiveMode.inc"
#include "Application/FBRunner3App_CartCommands.inc"
```

## Adding New Features

### To add a new cart command:
1. Edit `FBRunner3App_CartCommands.inc`
2. Add method implementation
3. Wire up in `executeInteractiveLine` (in `FBRunner3App_InteractiveMode.inc`)

### To modify syntax highlighting:
1. Edit `FBRunner3App_EditorSupport.inc`
2. Update `highlightLine` method
3. Add new color tokens as needed

### To change compilation behavior:
1. Edit `FBRunner3App_ScriptExecution.inc`
2. Modify `compileBASICToLua` method
3. Update compiler pipeline as needed

### To add interactive shell features:
1. Edit `FBRunner3App_InteractiveMode.inc`
2. Add new command handlers
3. Update input/rendering logic

## Build System

Only `CommandRegistryInit.cpp` is explicitly compiled. The `.inc` files are included during compilation of `main.mm`.

In `CMakeLists.txt`:
```cmake
Application/CommandRegistryInit.cpp
```

## Benefits

- **Maintainability:** Each file has a clear, single purpose
- **Navigation:** Easy to find specific functionality
- **Collaboration:** Multiple developers can work independently
- **Testing:** Can focus on one module at a time
- **Debugging:** Clear boundaries between subsystems

## Statistics

- **Original `main.mm`:** 4,339 lines
- **Refactored `main.mm`:** 1,207 lines (72% reduction)
- **Extracted code:** 3,171 lines across 6 modules
- **Build time:** Unchanged (includes are compile-time)
- **Binary size:** Unchanged (12MB)

---

For detailed refactoring information, see:
- `../REFACTORING_COMPLETE.md` - Complete refactoring documentation
- `../REFACTORING_GUIDE.md` - Migration guide and future improvements