//
// main.mm
// FasterBASIC - SuperTerminal FasterBASICT Runtime Application
//
// A standalone application that executes BASIC scripts using the FasterBASICT
// compiler (standalone C++ compiler that generates Lua) and LuaJIT runtime.
// Provides integrated editor with syntax highlighting, blocking/sequential
// execution style, and full SuperTerminal API access.
//
// Usage: FasterBASIC [script.bas]
//
// Script Style:
//   10 REM Initialize
//   20 LET X = 0
//   30 REM Main loop
//   40 PRINT "Hello from BASIC!"
//   50 LET X = X + 1
//   60 GOTO 40
//

#import "BaseRunner.h"
#import "Application/FBRunner3App.h"
#import "Application/CommandRegistryInit.h"
#import "HelpViewController.h"
#include "Debug/Logger.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_set>
#include "FBTBindings.h"
#include "DataManager.h"
#include "Editor/TextBuffer.h"
#include "Editor/Cursor.h"
#include "EditorBridge.h"
#include "../Framework/API/superterminal_api.h"
#include "../Framework/API/st_api_circles.h"
#include "../FasterBASICT/shell/command_parser.h"
#include "../FasterBASICT/shell/program_manager_v2.h"
#include "../FasterBASICT/shell/shell_core.h"
#include "../FasterBASICT/src/basic_formatter_lib.h"

// FasterBASICT compiler headers
#include "fasterbasic_data_preprocessor.h"
#include "fasterbasic_lexer.h"
#include "fasterbasic_parser.h"
#include "fasterbasic_semantic.h"
#include "fasterbasic_optimizer.h"
#include "fasterbasic_peephole.h"
#include "fasterbasic_cfg.h"
#include "fasterbasic_ircode.h"
#include "fasterbasic_lua_codegen.h"
#include "../FasterBASICT/src/modular_commands.h"
#include "command_registry_core.h"
#include "command_registry_superterminal.h"
#include "../Framework/Cart/CartManager.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <luajit.h>
}

// Runtime module registration functions (from FasterBASICT)
extern "C" void register_unicode_module(lua_State* L);
extern "C" void register_bitwise_module(lua_State* L);
extern "C" void register_constants_module(lua_State* L);
extern "C" void set_constants_manager(FasterBASIC::ConstantsManager* manager);

using namespace SuperTerminal;
using namespace FasterBASIC;
using namespace FasterBASIC::ModularCommands;
using namespace FBRunner3::SuperTerminalCommands;

// Forward reference to access BaseRunner from C function
static BaseRunner* g_runnerInstance = nullptr;

// =============================================================================
// FBRunner3 - FasterBASICT Runtime using BaseRunner
// =============================================================================

// FBRunner3App interface now in Application/FBRunner3App.h

@implementation FBRunner3App {
    lua_State* _luaState;
    std::thread _scriptThread;
    FasterBASIC::DataPreprocessorResult dataResult;
    std::string _currentScriptContent;
    std::mutex _luaStateMutex;
    std::mutex _scriptThreadMutex;  // Protects _scriptThread operations
    std::atomic<bool> _shouldStopScript;
    std::atomic<bool> _scriptThreadRunning;
    std::atomic<bool> _isWaitingForStop;

    // Interactive mode components
    bool _interactiveMode;
    bool _returnToInteractiveAfterRun;

    // Interactive mode shell components
    std::unique_ptr<FasterBASIC::CommandParser> _commandParser;
    std::unique_ptr<FasterBASIC::ProgramManagerV2> _programManager;

    // Help system
    __strong HelpViewController* _helpViewController;
    lua_State* _interactiveLuaState;

    // Mouse interaction state for interactive mode
    bool _mouseSelecting;
    int _selectionStartX;
    int _selectionStartY;
    int _selectionEndX;
    int _selectionEndY;
    std::atomic<bool> _interactiveProgramRunning;

    // Editor/Shell synchronization bridge
    std::unique_ptr<FBRunner3::EditorBridge> _editorBridge;

    // Interactive mode display state
    std::vector<std::string> _outputLines;  // Scrolling output buffer
    std::string _currentInput;               // Current input line
    int _cursorPos;                          // Cursor position in input
    int _scrollOffset;                       // Scroll position in output
    std::vector<std::string> _history;       // Command history
    int _historyIndex;                       // Current position in history

    // Runtime mode mouse interaction state
    bool _runtimeMouseSelecting;
    int _runtimeSelectionStartX;
    int _runtimeSelectionStartY;
    int _runtimeSelectionEndX;
    int _runtimeSelectionEndY;


    // Interactive state saving/restoration
    struct InteractiveState {
        std::vector<std::string> outputLines;
        std::string currentInput;
        int cursorPos;
        int scrollOffset;
        std::vector<std::string> history;
        int historyIndex;
        bool autoContinueMode;
        int suggestedNextLine;
        int lastLineNumber;
    };
    InteractiveState _savedInteractiveState;

    // Runtime mode text state
    struct RuntimeTextState {
        std::vector<std::string> outputLines;    // All text output from program
        int cursorX, cursorY;                   // Current cursor position (1-based)
        int scrollOffset;                       // For scroll-back (PageUp/PageDown)
        int screenWidth, screenHeight;          // Screen dimensions in characters
        bool autoScroll;                        // Auto-scroll when cursor goes off screen
        std::vector<std::string> screenBuffer;  // Current screen content (screenHeight lines)

        RuntimeTextState()
            : cursorX(1), cursorY(1), scrollOffset(0),
              screenWidth(80), screenHeight(25), autoScroll(true) {
            screenBuffer.resize(screenHeight, "");
        }
    };
    RuntimeTextState _runtimeTextState;

    // AUTO-CONTINUATION mode (automatic line suggestion after numbered line)
    bool _autoContinueMode;
    int _lastLineNumber;
    int _suggestedNextLine;

    // Syntax highlighting - user-defined constants cache
    std::unordered_set<std::string> _userDefinedConstants;
    std::vector<std::string> _predefinedConstantNames;  // Cache of predefined constant names

    // Cart system - use BaseRunner's self.cartManager (inherited)
}

// Initialize modular command registry for FBRunner3 - MUST be called before any compiler operations
// Command registry initialization moved to Application/CommandRegistryInit.cpp

// =============================================================================
// Initialization and Cleanup
// =============================================================================

- (instancetype)init {
    self = [super init];
    if (self) {
        // CRITICAL: Initialize modular command registry FIRST before any FasterBASIC operations
        // This must happen before any Lexer objects are created
        initializeFBRunner3CommandRegistry();
        LOG_INFO("✓ Modular command registry initialized with SuperTerminal commands");

        _shouldStopScript = false;
        _scriptThreadRunning = false;
        _isWaitingForStop = false;
        _interactiveMode = false;
        _returnToInteractiveAfterRun = false;
        _interactiveLuaState = nullptr;
        _helpViewController = nil;
        _interactiveProgramRunning = false;
        _cursorPos = 0;
        _scrollOffset = 0;
        _historyIndex = -1;

        // Initialize runtime mode mouse state
        _runtimeMouseSelecting = false;
        _runtimeSelectionStartX = 0;
        _runtimeSelectionStartY = 0;
        _runtimeSelectionEndX = 0;
        _runtimeSelectionEndY = 0;
        _autoContinueMode = false;
        _lastLineNumber = 0;
        _suggestedNextLine = 10;

        // Initialize runtime text state
        [self initializeRuntimeTextState];

        // Cart callbacks are set up in BaseRunner's initializeSubsystems
        // We can override or add additional callbacks here if needed
        LOG_INFO("Using BaseRunner's CartManager");


        LOG_INFO("✓ Cart manager initialized");
    }
    return self;
}

- (void)dealloc {
    // Stop any running script
    LOG_DEBUG("dealloc: Setting _shouldStopScript = true");
    _shouldStopScript = true;

    // Wait for thread to finish with mutex protection
    {
        std::lock_guard<std::mutex> lock(_scriptThreadMutex);
        if (_scriptThread.joinable()) {
            LOG_INFO("Waiting for script thread to finish in dealloc...");
            _scriptThread.join();
        }
    }

    // Close Lua state
    if (_luaState) {
        lua_close(_luaState);
        _luaState = nullptr;
    }

    // Clean up help view controller
    _helpViewController = nil;
}

// =============================================================================
// Framework Configuration
// =============================================================================

- (BOOL)initializeFramework {
    // Configure display based on preferred window size
    DisplayConfig config;

    // Parse window size and set dimensions
    std::string size = self.preferredWindowSize;
    if (size == "small") {
        config.windowWidth = 640;
        config.windowHeight = 400;
        config.cellWidth = 10;
        config.cellHeight = 18;
    } else if (size == "large") {
        config.windowWidth = 1280;
        config.windowHeight = 720;
        config.cellWidth = 12;
        config.cellHeight = 22;
    } else if (size == "fullhd") {
        config.windowWidth = 1920;
        config.windowHeight = 1080;
        config.cellWidth = 18;
        config.cellHeight = 42;
    } else {
        // Default: medium (800x600)
        config.windowWidth = 800;
        config.windowHeight = 600;
        config.cellWidth = 10;
        config.cellHeight = 18;
    }

    config.fullscreen = false;
    config.vsync = true;
    config.targetFPS = 60.0f;
    config.windowTitle = self.runnerName.c_str();

    LOG_INFOF("Configuring %s window: %dx%d (cells: %dx%d)",
          size.c_str(), config.windowWidth, config.windowHeight,
          config.cellWidth, config.cellHeight);

    BOOL result = [self initializeFrameworkWithConfig:config];

    // CRITICAL FIX: Update InputManager with cell sizes based on ACTUAL view dimensions
    // The view size may differ from requested config (e.g., 1280x675 instead of 1920x1080)
    if (result && self.inputManager && self.textGrid && self.displayManager) {
        NSView* metalView = self.displayManager->getMetalView();
        if (metalView) {
            NSRect actualBounds = metalView.bounds;
            float actualWidth = actualBounds.size.width;
            float actualHeight = actualBounds.size.height;

            int gridCols = self.textGrid->getWidth();
            int gridRows = self.textGrid->getHeight();

            // Calculate cell size from ACTUAL view size divided by ACTUAL grid size
            // This ensures mouse coordinates (in view points) map correctly to grid cells
            float actualCellW = actualWidth / (float)gridCols;
            float actualCellH = actualHeight / (float)gridRows;

            // Configure InputManager with correct cell size
            self.inputManager->setCellSize(actualCellW, actualCellH);
        }
    }

    return result;
}

// =============================================================================
// Runtime Initialization
// =============================================================================

- (BOOL)initializeRuntime {
    LOG_INFO("Initializing FasterBASICT + LuaJIT runtime...");

    // Initialize editor
    if (![self initializeEditor]) {
        LOG_WARNING("Failed to initialize editor");
    }

    // Set language for editor
    self.currentScriptLanguage = "basic";

    // Start in editor mode if no script path provided
    if (self.scriptPath.empty() || self.scriptPath == "untitled.bas") {
        LOG_INFO("Starting in editor mode");

        [self enterEditorMode];

        // Load welcome script
        std::string welcome = "1000 REM Welcome to FasterBASIC!\n"
                             "1010 REM Main loop example:\n"
                             "1020 LET X = 0\n"
                             "1030 CONSTANT SOLID_PURPLE = 0x800080FF\n"
                             "1040 WHILE TRUE\n"
                             "1050   TEXT_CLEAR()\n"
                             "1060   TEXT_PUT(0, 0, \"Hello from FasterBASICT!\", SOLID_GREEN, CLEAR_BLACK)\n"
                             "1070   TEXT_PUT(0, 2, \"Counter: \" + STR$(X), SOLID_PURPLE, CLEAR_BLACK)\n"
                             "1080   LET X = X + 1\n"
                             "1090   WAIT_FRAME()\n"
                             "1100 WEND\n";

        if (self.textEditor) {
            self.textEditor->loadText(welcome);
            self.textEditor->setFilename("untitled");

            // CRITICAL: Render the initial content to TextGrid immediately
            // Otherwise the screen stays white until first frame update
            self.textEditor->render();
            LOG_INFO("Initial editor content rendered to TextGrid");

            // Set up automatic line numbering for BASIC
            [self setupLineNumbering];
        } else {
            LOG_ERROR("textEditor is NULL!");
        }
    }

    // Store runner instance for wait_frame callback
    g_runnerInstance = self;

    // Add "Show Main Window" to Window menu
    [self addShowWindowMenuItem];

    // Create Lua state
    _luaState = luaL_newstate();
    if (!_luaState) {
        LOG_ERROR("Failed to create Lua state");
        return NO;
    }

    // Load standard libraries
    luaL_openlibs(_luaState);

    // Register FasterBASICT runtime modules
    register_unicode_module(_luaState);
    register_bitwise_module(_luaState);
    register_constants_module(_luaState);

    // Register SuperTerminal API bindings
    FBTBindings::registerBindings(_luaState);

    // Override wait_frame to use BaseRunner's frame synchronization
    // Also check if script should stop
    lua_pushcfunction(_luaState, [](lua_State* L) -> int {
        FBRunner3App* app = (FBRunner3App*)g_runnerInstance;
        if (app) {
            // Check if we should stop the script
            if (app->_shouldStopScript) {
                luaL_error(L, "Script stopped by user");
                return 0;
            }
            [app waitForNextFrame];
        }
        return 0;
    });
    lua_setglobal(_luaState, "wait_frame");

    // Add custom print function that logs to console
    lua_pushcfunction(_luaState, [](lua_State* L) -> int {
        int nargs = lua_gettop(L);
        std::string message;
        for (int i = 1; i <= nargs; i++) {
            if (i > 1) message += "\t";
            if (lua_isstring(L, i)) {
                message += lua_tostring(L, i);
            } else if (lua_isnil(L, i)) {
                message += "nil";
            } else if (lua_isboolean(L, i)) {
                message += lua_toboolean(L, i) ? "true" : "false";
            } else {
                message += luaL_typename(L, i);
            }
        }
        LOG_INFOF("[BASIC] %s", message.c_str());
        return 0;
    });
    lua_setglobal(_luaState, "print");

    // Override os.exit to prevent it from killing the entire app
    // When a BASIC script does END or an error occurs, the generated Lua
    // code calls os.exit(). We intercept this and throw a Lua error instead,
    // which will be caught by the pcall wrapper in executeScriptContent.
    LOG_INFO("🔧 INSTALLING os.exit() OVERRIDE to prevent app termination");
    lua_pushcfunction(_luaState, [](lua_State* L) -> int {
        int exitcode = luaL_optinteger(L, 1, 0);
        LOG_INFOF("🛑 os.exit(%d) INTERCEPTED - converting to Lua error", exitcode);
        // Instead of exiting the app, throw a Lua error
        if (exitcode == 0) {
            luaL_error(L, "Script ended normally (os.exit called)");
        } else {
            luaL_error(L, "Script ended with error code %d (os.exit called)", exitcode);
        }
        return 0;
    });
    lua_setglobal(_luaState, "exit");  // Set as global function 'exit'

    // Also override it in the os table
    lua_getglobal(_luaState, "os");
    lua_pushcfunction(_luaState, [](lua_State* L) -> int {
        int exitcode = luaL_optinteger(L, 1, 0);
        LOG_INFOF("🛑 os.exit(%d) INTERCEPTED - converting to Lua error", exitcode);
        if (exitcode == 0) {
            luaL_error(L, "Script ended normally (os.exit called)");
        } else {
            luaL_error(L, "Script ended with error code %d (os.exit called)", exitcode);
        }
        return 0;
    });
    lua_setfield(_luaState, -2, "exit");
    lua_pop(_luaState, 1);  // Pop the os table
    LOG_INFO("✅ os.exit() override installed successfully");

    LOG_INFO("FasterBASICT runtime initialized (LuaJIT: " LUAJIT_VERSION ")");
    return YES;
}

// =============================================================================
// Automatic Line Numbering for BASIC + Syntax Highlighting
// =============================================================================
// (Extracted to FBRunner3App_EditorSupport.inc for better organization)
#include "Application/FBRunner3App_EditorSupport.inc"

// =============================================================================
// BASIC Compilation + Script Loading + Script Execution
// =============================================================================
// (Extracted to FBRunner3App_ScriptExecution.inc for better organization)
#include "Application/FBRunner3App_ScriptExecution.inc"

// =============================================================================
// Script Loading and Execution
// =============================================================================
// (Extracted to FBRunner3App_ScriptExecution.inc - see BASIC Compilation)

// =============================================================================
// Mode Switching Override
// =============================================================================
//
// UNIFIED DOCUMENT ARCHITECTURE:
// ===============================
// We maintain a single source of truth (SourceDocument) that is always synced
// when switching modes. This ensures consistency and eliminates the need for
// complex screen state restoration logic.
//
// Editor Mode:  TextBuffer ← sync ← SourceDocument (always fresh)
// Shell Mode:   TextBuffer → sync → SourceDocument (on entry)
//
// Benefits:
// - No stale content ever displayed
// - Simple, predictable behavior
// - SourceDocument is always the canonical source
// - Screen state restoration only used for graphics/sprites, not text content
//

- (void)enterEditorMode {
    LOG_INFO("Entering editor mode (override)");

    // ALWAYS invalidate saved editor state - we rebuild from SourceDocument
    // Screen state restoration is disabled for text content to ensure we
    // always display the current state of the SourceDocument
    if (self.screenStateManager) {
        self.screenStateManager->invalidateEditorState();
        LOG_INFO("Invalidated editor state (always rebuild from source)");
    }

    // Call parent implementation to handle mode switch
    [super enterEditorMode];

    // ALWAYS sync from SourceDocument to TextBuffer to ensure consistency
    // This makes SourceDocument the single source of truth
    if (_programManager && self.textEditor) {
        LOG_INFO("Syncing SourceDocument → TextBuffer (always)");
        [self syncProgramManagerToEditor];
    }

    // Force editor to re-render from TextBuffer (which now reflects SourceDocument)
    if (self.textEditor) {
        self.textEditor->render();
        LOG_INFO("Editor rebuilt from SourceDocument");
    }
}

// =============================================================================
// Runtime Text State Management
// =============================================================================

- (void)initializeRuntimeTextState {
    _runtimeTextState.outputLines.clear();
    _runtimeTextState.cursorX = 1;
    _runtimeTextState.cursorY = 1;
    _runtimeTextState.scrollOffset = 0;
    _runtimeTextState.screenWidth = 80;
    _runtimeTextState.screenHeight = 25;
    _runtimeTextState.autoScroll = true;
    _runtimeTextState.screenBuffer.clear();
    _runtimeTextState.screenBuffer.resize(_runtimeTextState.screenHeight, "");
}

- (void)clearRuntimeTextState {
    [self initializeRuntimeTextState];
}

- (void)runtimePrintText:(const std::string&)text {
    // Split text into lines if it contains newlines
    std::string remainingText = text;
    size_t pos = 0;

    while (pos < remainingText.length()) {
        size_t newlinePos = remainingText.find('\n', pos);

        if (newlinePos == std::string::npos) {
            // No more newlines, add remaining text to current line
            std::string fragment = remainingText.substr(pos);
            [self addTextToCurrentRuntimeLine:fragment];
            break;
        } else {
            // Add text up to newline, then advance to next line
            if (newlinePos > pos) {
                std::string fragment = remainingText.substr(pos, newlinePos - pos);
                [self addTextToCurrentRuntimeLine:fragment];
            }
            [self advanceRuntimeCursor];
            pos = newlinePos + 1;
        }
    }

    [self updateRuntimeDisplay];
}

- (void)addTextToCurrentRuntimeLine:(const std::string&)text {
    // Ensure we have enough lines in outputLines
    while (_runtimeTextState.outputLines.size() < _runtimeTextState.cursorY) {
        _runtimeTextState.outputLines.push_back("");
    }

    // Get current line
    std::string& currentLine = _runtimeTextState.outputLines[_runtimeTextState.cursorY - 1];

    // Ensure line is long enough for cursor position
    while (currentLine.length() < _runtimeTextState.cursorX - 1) {
        currentLine += " ";
    }

    // Insert text at cursor position
    if (_runtimeTextState.cursorX - 1 <= currentLine.length()) {
        currentLine.insert(_runtimeTextState.cursorX - 1, text);
    } else {
        currentLine += text;
    }

    // Advance cursor X position
    _runtimeTextState.cursorX += text.length();
}

- (void)advanceRuntimeCursor {
    _runtimeTextState.cursorX = 1;
    _runtimeTextState.cursorY++;

    // Auto-scroll if we go beyond screen height
    if (_runtimeTextState.autoScroll &&
        _runtimeTextState.cursorY - _runtimeTextState.scrollOffset > _runtimeTextState.screenHeight) {
        _runtimeTextState.scrollOffset = _runtimeTextState.cursorY - _runtimeTextState.screenHeight;
    }
}

- (void)setRuntimeCursorPosition:(int)x y:(int)y {
    _runtimeTextState.cursorX = std::max(1, x);
    _runtimeTextState.cursorY = std::max(1, y);

    // Ensure we have enough output lines
    while (_runtimeTextState.outputLines.size() < _runtimeTextState.cursorY) {
        _runtimeTextState.outputLines.push_back("");
    }
}

- (void)scrollRuntimeText:(int)lines {
    _runtimeTextState.scrollOffset += lines;

    // Clamp scroll offset
    int maxScroll = std::max(0, (int)_runtimeTextState.outputLines.size() - _runtimeTextState.screenHeight);
    _runtimeTextState.scrollOffset = std::max(0, std::min(_runtimeTextState.scrollOffset, maxScroll));

    [self updateRuntimeDisplay];
}

- (void)updateRuntimeDisplay {
    if (!self.textGrid) return;

    // Clear the text grid
    self.textGrid->clear();

    // Calculate which lines to display
    int startLine = _runtimeTextState.scrollOffset;
    int endLine = std::min(startLine + _runtimeTextState.screenHeight,
                          (int)_runtimeTextState.outputLines.size());

    // Display the visible lines
    for (int i = startLine; i < endLine; i++) {
        int displayRow = i - startLine;
        const std::string& line = _runtimeTextState.outputLines[i];

        if (!line.empty() && displayRow < _runtimeTextState.screenHeight) {
            // Display line on screen (convert to 0-based for textGrid)
            // Truncate line if it's too long for screen width
            std::string displayLine = line.length() > _runtimeTextState.screenWidth ?
                                    line.substr(0, _runtimeTextState.screenWidth) : line;

            for (size_t col = 0; col < displayLine.length() && col < _runtimeTextState.screenWidth; col++) {
                // Check if this is the cursor position
                bool isCursor = false;
                if (_runtimeTextState.scrollOffset == 0 ||
                    _runtimeTextState.cursorY - _runtimeTextState.scrollOffset <= _runtimeTextState.screenHeight) {
                    int cursorScreenY = _runtimeTextState.cursorY - _runtimeTextState.scrollOffset - 1;
                    int cursorScreenX = std::min(_runtimeTextState.cursorX - 1, _runtimeTextState.screenWidth - 1);
                    if (displayRow == cursorScreenY && col == cursorScreenX) {
                        isCursor = true;
                    }
                }

                // Check if this character is in the selection
                bool isSelected = false;
                if (_runtimeMouseSelecting) {
                    int minY = std::min(_runtimeSelectionStartY, _runtimeSelectionEndY);
                    int maxY = std::max(_runtimeSelectionStartY, _runtimeSelectionEndY);

                    if (displayRow >= minY && displayRow <= maxY) {
                        if (minY == maxY) {
                            // Single line selection
                            int minX = std::min(_runtimeSelectionStartX, _runtimeSelectionEndX);
                            int maxX = std::max(_runtimeSelectionStartX, _runtimeSelectionEndX);
                            isSelected = (col >= minX && col <= maxX);
                        } else if (displayRow == minY) {
                            // First line of multi-line selection
                            int startX = (_runtimeSelectionStartY == minY) ? _runtimeSelectionStartX : _runtimeSelectionEndX;
                            isSelected = (col >= startX);
                        } else if (displayRow == maxY) {
                            // Last line of multi-line selection
                            int endX = (_runtimeSelectionEndY == maxY) ? _runtimeSelectionEndX : _runtimeSelectionStartX;
                            isSelected = (col <= endX);
                        } else {
                            // Middle line of multi-line selection
                            isSelected = true;
                        }
                    }
                }

                // Use different colors for cursor position and selection
                uint32_t fg, bg;
                if (isCursor) {
                    fg = 0x00000000;  // Black text on cursor
                    bg = 0xFFFFFFFF;  // White background on cursor
                } else if (isSelected) {
                    fg = 0xFFFFFFFF;  // White text on selection
                    bg = 0xFF444444;  // Gray background on selection
                } else {
                    fg = 0xFFFFFFFF;  // White text
                    bg = 0x00000000;  // Black background
                }

                self.textGrid->putChar(col, displayRow, displayLine[col], fg, bg);
            }

            // Handle cursor at end of line (beyond text)
            if (_runtimeTextState.scrollOffset == 0 ||
                _runtimeTextState.cursorY - _runtimeTextState.scrollOffset <= _runtimeTextState.screenHeight) {
                int cursorScreenY = _runtimeTextState.cursorY - _runtimeTextState.scrollOffset - 1;
                int cursorScreenX = std::min(_runtimeTextState.cursorX - 1, _runtimeTextState.screenWidth - 1);

                if (displayRow == cursorScreenY && cursorScreenX >= displayLine.length() &&
                    cursorScreenX < _runtimeTextState.screenWidth) {
                    // Show cursor as a space with inverted colors
                    self.textGrid->putChar(cursorScreenX, displayRow, ' ', 0x00000000, 0xFFFFFFFF);
                }
            }
        }
    }
}

- (void)handleRuntimeKeyPress:(int)keyCode {
    switch (keyCode) {
        case 126: // Up arrow / Page Up
            [self scrollRuntimeText:-1];
            break;
        case 125: // Down arrow / Page Down
            [self scrollRuntimeText:1];
            break;
        case 116: // Page Up
            [self scrollRuntimeText:-10];
            break;
        case 121: // Page Down
            [self scrollRuntimeText:10];
            break;
        case 119: // End key - return to live view
            _runtimeTextState.scrollOffset = 0;
            [self updateRuntimeDisplay];
            break;
    }
}

- (void)enterRuntimeMode {
    LOG_INFO("Entering runtime mode");

    // ALWAYS invalidate runtime state so we get a clean screen for RUN
    // When running a program, we want a fresh start, not restored state
    if (self.screenStateManager) {
        self.screenStateManager->invalidateRuntimeState();
        LOG_INFO("Invalidated runtime state (always clean screen for RUN)");
    }

    // Call parent implementation to handle mode switch
    [super enterRuntimeMode];

    // Initialize runtime text state for clean program execution
    [self clearRuntimeTextState];
    LOG_INFO("Runtime text state initialized");

    // ALWAYS clear the screen explicitly when entering runtime mode
    // This ensures the user's program starts with a clean slate
    if (self.textGrid) {
        self.textGrid->clear();
        LOG_INFO("TextGrid cleared for program execution");
    }
    if (self.graphicsLayer) {
        self.graphicsLayer->clear();
        LOG_INFO("Graphics layer cleared for program execution");
    }

    // Set up initial runtime display
    [self updateRuntimeDisplay];
}

// =============================================================================
// Script Execution from Editor
// =============================================================================
// (Extracted to FBRunner3App_ScriptExecution.inc - see BASIC Compilation)

// =============================================================================
// Syntax Highlighting
// =============================================================================
// (Extracted to FBRunner3App_EditorSupport.inc - see Line Numbering section)

// =============================================================================
// Stop Script
// =============================================================================

- (void)stopScript {
    LOG_INFO("========================================");
    LOG_INFO("🛑 stopScript CALLED - Command+. pressed");
    LOG_INFO("========================================");
    LOG_INFO("stopScript called from:");
    LOG_INFOF("%@", [NSThread callStackSymbols]);

    LOG_INFOF("Current thread: %@", [NSThread currentThread]);
    LOG_INFOF("Is main thread: %d", [NSThread isMainThread]);
    LOG_INFOF("scriptRunning: %d", self.scriptRunning);
    LOG_INFOF("_scriptThreadRunning: %d", _scriptThreadRunning.load());
    LOG_INFOF("_shouldStopScript: %d", _shouldStopScript.load());

    // Capture the return flag BEFORE we do anything else
    // This prevents race conditions where the flag might be cleared
    bool shouldReturnToInteractive = _returnToInteractiveAfterRun;
    LOG_INFOF("stopScript: Captured _returnToInteractiveAfterRun flag = %d", shouldReturnToInteractive);

    // Signal the script to stop
    LOG_INFO("stopScript: Setting _shouldStopScript = true");
    _shouldStopScript = true;
    LOG_INFO("stopScript: _shouldStopScript flag set");

    // Remove any active interrupt hooks to prevent further interruption attempts
    LOG_INFO("stopScript: Removing Lua interrupt hook...");
    [self removeLuaInterruptHook];
    LOG_INFO("stopScript: Lua interrupt hook removed");

    // Interrupt any pending frame waits to unblock the script thread
    LOG_INFO("stopScript: Interrupting frame waits...");
    STApi::Context::instance().interruptFrameWaits();
    LOG_INFO("stopScript: Frame waits interrupted");

    // Set script cancellation flag for WAIT_MS and other cancellable operations
    LOG_INFO("stopScript: Setting script should stop flag in STApi context...");
    STApi::Context::instance().setScriptShouldStop(true);
    LOG_INFO("stopScript: STApi stop flag set");

    // Wait for the script thread to finish with mutex protection
    LOG_INFO("stopScript: Attempting to join script thread...");
    {
        std::lock_guard<std::mutex> lock(_scriptThreadMutex);
        if (_scriptThread.joinable()) {
            LOG_INFO("stopScript: Script thread is joinable, calling join()...");
            _scriptThread.join();
            LOG_INFO("stopScript: Script thread join() returned successfully");
        } else {
            LOG_INFO("stopScript: Script thread is not joinable");
        }
    }
    LOG_INFO("stopScript: Script thread join complete");

    // Reset flags
    LOG_INFO("stopScript: Resetting flags...");
    self.scriptRunning = NO;
    _shouldStopScript = false;
    _scriptThreadRunning = false;
    _returnToInteractiveAfterRun = false; // Clear it now that we've captured it
    LOG_INFO("stopScript: Flags reset");

    // Clear script cancellation flag
    LOG_INFO("stopScript: Clearing STApi stop flag...");
    STApi::Context::instance().setScriptShouldStop(false);
    LOG_INFO("stopScript: STApi stop flag cleared");

    // Clear all text display items to prevent stale GPU resource access
    LOG_INFO("stopScript: Clearing text display items...");
    st_text_clear_displayed();
    LOG_INFO("stopScript: Text display items cleared");

    // Stop palette automation for all modes
    LOG_INFO("stopScript: Stopping palette automation...");
    st_xres_palette_auto_stop();
    st_wres_palette_auto_stop();
    st_pres_palette_auto_stop();
    LOG_INFO("stopScript: Palette automation stopped");

    // Reset display mode to text mode (mode 0)
    LOG_INFO("stopScript: Resetting display mode to text mode...");
    st_mode(0);
    LOG_INFO("stopScript: Display mode reset complete");

    // Return to interactive or editor mode based on captured flag
    LOG_INFO("stopScript: Dispatching mode switch to main queue...");
    dispatch_async(dispatch_get_main_queue(), ^{
        if (shouldReturnToInteractive) {
            LOG_INFO("stopScript: Returning to interactive mode...");
            [self enterInteractiveMode];
        } else {
            LOG_INFO("stopScript: Returning to editor mode...");
            [self enterEditorMode];
        }
        LOG_INFO("stopScript: Mode switch complete");
    });

    LOG_INFO("========================================");
    LOG_INFO("✅ stopScript COMPLETED successfully");
    LOG_INFO("========================================");
}

// =============================================================================
// Cleanup
// =============================================================================

- (void)cleanupRuntime {
    LOG_INFO("Cleaning up runtime...");

    self.scriptRunning = NO;

    // Stop palette automation for all modes
    st_xres_palette_auto_stop();
    st_wres_palette_auto_stop();
    st_pres_palette_auto_stop();

    // Clear DATA when cleaning up
    FBTBindings::clearDataManager();
    FBTBindings::clearFileManager();  // Close all files when cleaning up

    // Wait for script thread to finish (with timeout)
    if (_scriptThread.joinable()) {
        _scriptThread.join();
    }

    // Clean up Lua state
    if (_luaState) {
        lua_close(_luaState);
        _luaState = nullptr;
    }
}

// =============================================================================
// Frame Update
// =============================================================================

- (void)onFrameTick {
    // Update interactive mode if active
    if (_interactiveMode) {
        [self updateInteractiveMode];
    }

    // Update runtime mode if active (not editor mode and not interactive)
    if (!self.editorMode && !_interactiveMode) {
        [self updateRuntimeMode];
    }

    // Update cart auto-save
    if (self.cartManager) {
        // Get delta time from last frame (assume 60 FPS = ~0.016s)
        static double lastTime = CACurrentMediaTime();
        double currentTime = CACurrentMediaTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        self.cartManager->updateAutoSave(deltaTime);
    }

    // Call super to handle editor updates and other base functionality
    [super onFrameTick];
}

- (void)updateRuntimeMode {
    // Handle runtime mode key input for scroll-back
    if (!self.inputManager) return;

    // Check for scroll-back keys
    if (self.inputManager->isKeyJustPressed(SuperTerminal::KeyCode::PageUp)) {
        [self handleRuntimeKeyPress:116]; // Page Up
    }
    if (self.inputManager->isKeyJustPressed(SuperTerminal::KeyCode::PageDown)) {
        [self handleRuntimeKeyPress:121]; // Page Down
    }
    if (self.inputManager->isKeyJustPressed(SuperTerminal::KeyCode::Up)) {
        [self handleRuntimeKeyPress:126]; // Up arrow
    }
    if (self.inputManager->isKeyJustPressed(SuperTerminal::KeyCode::Down)) {
        [self handleRuntimeKeyPress:125]; // Down arrow
    }
    if (self.inputManager->isKeyJustPressed(SuperTerminal::KeyCode::End)) {
        [self handleRuntimeKeyPress:119]; // End key
    }
}

// =============================================================================
// C-style Runtime Text Wrapper Functions (for Lua bindings)
// =============================================================================

extern "C" {
    void fbrunner3_runtime_print_text(const char* text) {
        FBRunner3App* app = (FBRunner3App*)g_runnerInstance;
        if (app && text) {
            [app runtimePrintText:std::string(text)];
        }
    }

    void fbrunner3_runtime_set_cursor(int x, int y) {
        FBRunner3App* app = (FBRunner3App*)g_runnerInstance;
        if (app) {
            [app setRuntimeCursorPosition:x y:y];
        }
    }

    void fbrunner3_runtime_print_newline() {
        FBRunner3App* app = (FBRunner3App*)g_runnerInstance;
        if (app) {
            [app runtimePrintText:"\n"];
        }
    }

    bool fbrunner3_should_stop_script() {
        FBRunner3App* app = (FBRunner3App*)g_runnerInstance;
        if (app) {
            return app->_shouldStopScript;
        }
        return false;
    }
}

// =============================================================================
// Menu Setup
// =============================================================================

- (void)setupMenuBar {
    // Call super to set up base menus
    [super setupMenuBar];

    NSMenu* mainMenu = [NSApp mainMenu];

    // Add Interactive Mode item to Run menu
    for (NSMenuItem* item in mainMenu.itemArray) {
        if ([item.submenu.title isEqualToString:@"Run"]) {
            NSMenu* runMenu = item.submenu;

            // Add separator
            [runMenu addItem:[NSMenuItem separatorItem]];

            // Add Interactive Mode toggle
            NSMenuItem* interactiveItem = [[NSMenuItem alloc] initWithTitle:@"Toggle Interactive Mode"
                                                                     action:@selector(toggleInteractiveMode)
                                                              keyEquivalent:@"i"];
            [interactiveItem setTarget:self];
            [runMenu addItem:interactiveItem];

            break;
        }
    }

    // Add Cart menu
    NSMenu* cartMenu = [[NSMenu alloc] initWithTitle:@"Cart"];
    NSMenuItem* cartMenuItem = [[NSMenuItem alloc] initWithTitle:@"Cart" action:nil keyEquivalent:@""];
    [cartMenuItem setSubmenu:cartMenu];

    NSMenuItem* useCartItem = [[NSMenuItem alloc] initWithTitle:@"Use Cart..."
                                                         action:@selector(useCartAction:)
                                                  keyEquivalent:@"u"];
    [useCartItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand | NSEventModifierFlagShift];
    [useCartItem setTarget:self];
    [cartMenu addItem:useCartItem];

    NSMenuItem* runCartItem = [[NSMenuItem alloc] initWithTitle:@"Run Cart..."
                                                         action:@selector(runCartAction:)
                                                  keyEquivalent:@"r"];
    [runCartItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand | NSEventModifierFlagShift];
    [runCartItem setTarget:self];
    [cartMenu addItem:runCartItem];

    [cartMenu addItem:[NSMenuItem separatorItem]];

    NSMenuItem* saveCartItem = [[NSMenuItem alloc] initWithTitle:@"Save Cart"
                                                          action:@selector(saveCartAction:)
                                                   keyEquivalent:@"s"];
    [saveCartItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand];
    [saveCartItem setTarget:self];
    [cartMenu addItem:saveCartItem];

    NSMenuItem* closeCartItem = [[NSMenuItem alloc] initWithTitle:@"Close Cart"
                                                           action:@selector(closeCartAction:)
                                                    keyEquivalent:@"w"];
    [closeCartItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand];
    [closeCartItem setTarget:self];
    [cartMenu addItem:closeCartItem];

    [cartMenu addItem:[NSMenuItem separatorItem]];

    NSMenuItem* newCartItem = [[NSMenuItem alloc] initWithTitle:@"New Cart..."
                                                         action:@selector(newCartAction:)
                                                  keyEquivalent:@"n"];
    [newCartItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand | NSEventModifierFlagShift];
    [newCartItem setTarget:self];
    [cartMenu addItem:newCartItem];

    [cartMenu addItem:[NSMenuItem separatorItem]];

    NSMenuItem* cartInfoItem = [[NSMenuItem alloc] initWithTitle:@"Cart Info..."
                                                          action:@selector(cartInfoAction:)
                                                   keyEquivalent:@"i"];
    [cartInfoItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand];
    [cartInfoItem setTarget:self];
    [cartMenu addItem:cartInfoItem];

    // Insert Cart menu before Window menu
    NSInteger cartInsertIndex = [mainMenu.itemArray count] - 2;
    [mainMenu insertItem:cartMenuItem atIndex:cartInsertIndex];

    // Add Clear menu
    NSMenu* clearMenu = [[NSMenu alloc] initWithTitle:@"Clear"];
    NSMenuItem* clearMenuItem = [[NSMenuItem alloc] initWithTitle:@"Clear" action:nil keyEquivalent:@""];
    [clearMenuItem setSubmenu:clearMenu];

    // Add All submenu to Clear menu
    NSMenu* allMenu = [[NSMenu alloc] initWithTitle:@"All"];
    NSMenuItem* allMenuItem = [[NSMenuItem alloc] initWithTitle:@"All" action:nil keyEquivalent:@""];
    [allMenuItem setSubmenu:allMenu];

    // Add clear actions to All submenu
    NSMenuItem* clearAllItem = [[NSMenuItem alloc] initWithTitle:@"Clear All Layers"
                                                          action:@selector(clearAllLayers)
                                                   keyEquivalent:@""];
    [clearAllItem setTarget:self];
    [allMenu addItem:clearAllItem];

    [clearMenu addItem:allMenuItem];

    // Insert Clear menu before Window menu
    NSInteger insertIndex = [mainMenu.itemArray count] - 2; // Before Window and Help
    [mainMenu insertItem:clearMenuItem atIndex:insertIndex];

    // Add Tools menu
    NSMenu* toolsMenu = [[NSMenu alloc] initWithTitle:@"Tools"];
    NSMenuItem* toolsMenuItem = [[NSMenuItem alloc] initWithTitle:@"Tools" action:nil keyEquivalent:@""];
    [toolsMenuItem setSubmenu:toolsMenu];

    // Add SPRED menu item
    NSMenuItem* spredItem = [[NSMenuItem alloc] initWithTitle:@"Open SPRED (Sprite Editor)"
                                                       action:@selector(openSPRED:)
                                                keyEquivalent:@""];
    [spredItem setTarget:self];
    [toolsMenu addItem:spredItem];

    // Insert Tools menu before Window menu
    NSInteger toolsInsertIndex = [mainMenu.itemArray count] - 2;
    [mainMenu insertItem:toolsMenuItem atIndex:toolsInsertIndex];

    // Add Help menu items
    // Find or create the Help menu
    NSMenu* helpMenu = nil;
    for (NSMenuItem* item in mainMenu.itemArray) {
        if ([item.submenu.title isEqualToString:@"Help"]) {
            helpMenu = item.submenu;
            break;
        }
    }

    // Create Help menu if it doesn't exist
    if (!helpMenu) {
        helpMenu = [[NSMenu alloc] initWithTitle:@"Help"];
        NSMenuItem* helpMenuItem = [[NSMenuItem alloc] initWithTitle:@"Help" action:nil keyEquivalent:@""];
        [helpMenuItem setSubmenu:helpMenu];
        [mainMenu addItem:helpMenuItem];
    }

    // Add separator before our items
    [helpMenu addItem:[NSMenuItem separatorItem]];

    // Add Articles & Reference item
    NSMenuItem* commandRefItem = [[NSMenuItem alloc] initWithTitle:@"Articles & Reference"
                                                            action:@selector(showCommandReference:)
                                                     keyEquivalent:@"?"];
    [commandRefItem setTarget:self];
    [helpMenu addItem:commandRefItem];

    // Add Quick Help item
    NSMenuItem* quickHelpItem = [[NSMenuItem alloc] initWithTitle:@"Quick Help"
                                                           action:@selector(showQuickHelp:)
                                                    keyEquivalent:@""];
    [quickHelpItem setTarget:self];
    [helpMenu addItem:quickHelpItem];
}

- (void)showCommandReference:(id)sender {
    NSLog(@"[FBRunner3] Showing Command Reference...");

    @try {
        // Create help view controller if needed
        if (!_helpViewController) {
            NSLog(@"[FBRunner3] Creating help view controller...");

            // Get the command registry
            FasterBASIC::ModularCommands::CommandRegistry* registry =
                FBRunner3::SuperTerminalCommands::getSuperTerminalRegistry();

            if (!registry) {
                NSLog(@"[FBRunner3] ERROR: Registry is NULL!");
                NSAlert* alert = [[NSAlert alloc] init];
                [alert setMessageText:@"Help System Error"];
                [alert setInformativeText:@"Command registry not initialized."];
                [alert runModal];
                return;
            }

            NSLog(@"[FBRunner3] Registry obtained, initializing help controller...");
            _helpViewController = [[HelpViewController alloc] initWithCommandRegistry:registry];

            if (!_helpViewController) {
                NSLog(@"[FBRunner3] ERROR: Failed to create help view controller!");
                return;
            }
            NSLog(@"[FBRunner3] Help view controller created successfully");
        }

        NSLog(@"[FBRunner3] Showing help window...");
        [_helpViewController showHelpWindow];
        NSLog(@"[FBRunner3] Help window shown");
    }
    @catch (NSException* exception) {
        NSLog(@"[FBRunner3] EXCEPTION in showCommandReference: %@", exception);
        NSAlert* alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Help System Error"];
        [alert setInformativeText:[NSString stringWithFormat:@"Exception: %@", exception.reason]];
        [alert runModal];
    }
}

- (void)showQuickHelp:(id)sender {
    NSLog(@"[FBRunner3] Showing Quick Help...");

    NSAlert* alert = [[NSAlert alloc] init];
    [alert setMessageText:@"FasterBASIC Quick Help"];
    [alert setInformativeText:@"FasterBASIC is a BASIC programming environment.\n\n"
                               @"Quick Start:\n"
                               @"• Type BASIC code in the editor\n"
                               @"• Press Cmd+R to run your program\n"
                               @"• Press Cmd+. to stop execution\n"
                               @"• Use Cmd+? to open Articles & Reference\n\n"
                               @"View Articles & Reference for documentation and "
                               @"detailed reference of all available commands and functions."];
    [alert addButtonWithTitle:@"Open Articles & Reference"];
    [alert addButtonWithTitle:@"OK"];

    NSModalResponse response = [alert runModal];
    if (response == NSAlertFirstButtonReturn) {
        [self showCommandReference:sender];
    }
}

- (void)openSPRED:(id)sender {
    NSLog(@"[FBRunner3] Opening SPRED...");

    // Try to find SPRED.app in several locations
    NSArray* searchPaths = @[
        // 1. In Tools folder next to FasterBASIC.app (for packaged distribution)
        [[NSBundle mainBundle].bundlePath.stringByDeletingLastPathComponent stringByAppendingPathComponent:@"Tools/SPRED.app"],

        // 2. In the build directory (for development)
        @"/Users/oberon/FasterBasicGreen/spred/build/bin/SPRED.app",

        // 3. In Applications folder
        @"/Applications/SPRED.app",

        // 4. In user Applications folder
        [NSHomeDirectory() stringByAppendingPathComponent:@"Applications/SPRED.app"]
    ];

    NSString* spredPath = nil;
    for (NSString* path in searchPaths) {
        if ([[NSFileManager defaultManager] fileExistsAtPath:path]) {
            spredPath = path;
            NSLog(@"[FBRunner3] Found SPRED at: %@", spredPath);
            break;
        }
    }

    if (!spredPath) {
        NSLog(@"[FBRunner3] SPRED.app not found in any search location");

        NSAlert* alert = [[NSAlert alloc] init];
        [alert setMessageText:@"SPRED Not Found"];
        [alert setInformativeText:@"Could not find SPRED.app. Please ensure it's installed in the Tools folder, Applications folder, or build directory."];
        [alert addButtonWithTitle:@"OK"];
        [alert runModal];
        return;
    }

    // Launch SPRED
    NSError* error = nil;
    NSURL* spredURL = [NSURL fileURLWithPath:spredPath];

    [[NSWorkspace sharedWorkspace] openApplicationAtURL:spredURL
                                          configuration:[NSWorkspaceOpenConfiguration configuration]
                                      completionHandler:^(NSRunningApplication *app, NSError *error) {
        if (error) {
            NSLog(@"[FBRunner3] Failed to launch SPRED: %@", error.localizedDescription);

            dispatch_async(dispatch_get_main_queue(), ^{
                NSAlert* alert = [[NSAlert alloc] init];
                [alert setMessageText:@"Failed to Launch SPRED"];
                [alert setInformativeText:[NSString stringWithFormat:@"Error: %@", error.localizedDescription]];
                [alert addButtonWithTitle:@"OK"];
                [alert runModal];
            });
        } else {
            NSLog(@"[FBRunner3] SPRED launched successfully");
        }
    }];
}

// =============================================================================
// Cart Menu Actions
// =============================================================================

- (void)useCartAction:(id)sender {
    LOG_INFOF("useCartAction called, sender=%@", sender);

    if (!self.window) {
        LOG_ERROR("No window available for cart dialog");
        return;
    }

    NSOpenPanel* panel = [NSOpenPanel openPanel];
    [panel setTitle:@"Use Cart"];
    [panel setMessage:@"Select a cart file to open for editing"];
    [panel setAllowedFileTypes:@[@"crt"]];
    [panel setAllowsMultipleSelection:NO];

    LOG_INFO("Showing Use Cart open panel...");

    [panel beginSheetModalForWindow:self.window completionHandler:^(NSInteger result) {
        LOG_INFOF("Use Cart panel completed with result: %ld", (long)result);
        if (result == NSModalResponseOK) {
            NSURL* url = [[panel URLs] firstObject];
            if (url) {
                LOG_INFOF("Selected cart file: %@", [url path]);

                // Exit interactive mode before loading cart to prevent deadlock
                if (_interactiveMode) {
                    LOG_INFO("Exiting interactive mode before loading cart");
                    [self exitInteractiveMode];
                    [self enterEditorMode];
                }

                [self useCartAtPath:[url path]];
            } else {
                LOG_INFO("No URL returned from panel");
            }
        }
    }];
}

- (void)runCartAction:(id)sender {
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    [panel setTitle:@"Run Cart"];
    [panel setMessage:@"Select a cart file to run"];
    [panel setAllowedFileTypes:@[@"crt"]];
    [panel setAllowsMultipleSelection:NO];

    [panel beginSheetModalForWindow:self.window completionHandler:^(NSInteger result) {
        if (result == NSModalResponseOK) {
            NSURL* url = [[panel URLs] firstObject];
            if (url) {
                [self runCartAtPath:[url path]];
            }
        }
    }];
}

- (void)saveCartAction:(id)sender {
    if (!self.cartManager || !self.cartManager->isCartActive()) {
        NSAlert* alert = [[NSAlert alloc] init];
        [alert setMessageText:@"No Active Cart"];
        [alert setInformativeText:@"There is no cart currently open."];
        [alert runModal];
        return;
    }

    auto result = self.cartManager->saveCart();
    if (!result.success) {
        NSAlert* alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Save Failed"];
        [alert setInformativeText:[NSString stringWithUTF8String:result.message.c_str()]];
        [alert setAlertStyle:NSAlertStyleCritical];
        [alert runModal];
    } else {
        LOG_INFO("Cart saved successfully");
    }
}

- (void)closeCartAction:(id)sender {
    if (!self.cartManager || !self.cartManager->isCartActive()) {
        return;
    }

    auto result = self.cartManager->closeCart(true);
    if (!result.success) {
        NSAlert* alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Close Failed"];
        [alert setInformativeText:[NSString stringWithUTF8String:result.message.c_str()]];
        [alert runModal];
    } else {
        LOG_INFO("Cart closed");
    }
}

- (void)newCartAction:(id)sender {
    NSSavePanel* panel = [NSSavePanel savePanel];
    [panel setTitle:@"Create New Cart"];
    [panel setAllowedFileTypes:@[@"crt"]];
    [panel setNameFieldStringValue:@"MyGame.crt"];

    [panel beginSheetModalForWindow:self.window completionHandler:^(NSInteger result) {
        if (result == NSModalResponseOK) {
            NSURL* url = [panel URL];
            if (url) {
                [self createCartAtPath:[url path]];
            }
        }
    }];
}

- (void)cartInfoAction:(id)sender {
    if (!self.cartManager || !self.cartManager->isCartActive()) {
        NSAlert* alert = [[NSAlert alloc] init];
        [alert setMessageText:@"No Active Cart"];
        [alert setInformativeText:@"There is no cart currently open."];
        [alert runModal];
        return;
    }

    [self showCartInfo];
}

- (void)useCartAtPath:(NSString*)path {
    LOG_INFOF("useCartAtPath called with: %@", path);

    if (!self.cartManager) {
        LOG_ERROR("self.cartManager is NULL!");
        NSAlert* alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Cart Manager Not Initialized"];
        [alert setInformativeText:@"The cart manager is not available."];
        [alert setAlertStyle:NSAlertStyleCritical];
        [alert runModal];
        return;
    }

    LOG_INFOF("self.cartManager is valid: %p", self.cartManager.get());
    std::string pathStr = [path UTF8String];
    LOG_INFOF("pathStr converted: %s", pathStr.c_str());

    LOG_INFO("Calling self.cartManager->useCart() directly...");
    auto result = self.cartManager->useCart(pathStr);
    LOG_INFOF("self.cartManager->useCart() returned, success=%d", result.success);

    if (!result.success) {
        NSAlert* alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Failed to Open Cart"];
        [alert setInformativeText:[NSString stringWithUTF8String:result.message.c_str()]];
        [alert setAlertStyle:NSAlertStyleCritical];
        [alert runModal];
        return;
    }

    // Load program into editor
    LOG_INFOF("Loading program into editor, hasEditor=%d, programSize=%zu",
              self.textEditor != nullptr, result.programSource.size());
    if (self.textEditor && !result.programSource.empty()) {
        LOG_INFO("Calling textEditor->loadText()...");
        self.textEditor->loadText(result.programSource);
        LOG_INFO("Calling textEditor->setFilename()...");
        self.textEditor->setFilename([[path lastPathComponent] UTF8String]);
        LOG_INFO("Calling textEditor->render()...");
        self.textEditor->render();
        LOG_INFO("Cart program loaded into editor");
    }

    // Show warnings if any
    if (!result.warnings.empty()) {
        std::string warnMsg = "Cart loaded with warnings:\n";
        for (const auto& warning : result.warnings) {
            warnMsg += "  - " + warning + "\n";
        }
        LOG_INFOF("%s", warnMsg.c_str());
    }

    LOG_INFOF("Cart opened for editing: %@", path);
}

- (void)runCartAtPath:(NSString*)path {
    std::string pathStr = [path UTF8String];
    auto result = self.cartManager->runCart(pathStr);

    if (!result.success) {
        NSAlert* alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Failed to Open Cart"];
        [alert setInformativeText:[NSString stringWithUTF8String:result.message.c_str()]];
        [alert setAlertStyle:NSAlertStyleCritical];
        [alert runModal];
        return;
    }

    // Show cart info dialog
    [self showCartInfo];

    // Run the program
    if (!result.programSource.empty()) {
        _currentScriptContent = result.programSource;
        _returnToInteractiveAfterRun = true;
        [self exitInteractiveMode];
        [self runScript];
    }

    LOG_INFOF("Cart opened for playing: %@", path);
}

- (void)createCartAtPath:(NSString*)path {
    // Simple dialog for title
    NSAlert* alert = [[NSAlert alloc] init];
    [alert setMessageText:@"New Cart"];
    [alert setInformativeText:@"Enter cart title:"];

    NSTextField* titleField = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 300, 24)];
    [titleField setStringValue:@"My Game"];
    [alert setAccessoryView:titleField];
    [alert addButtonWithTitle:@"Create"];
    [alert addButtonWithTitle:@"Cancel"];

    NSInteger response = [alert runModal];
    if (response != NSAlertFirstButtonReturn) {
        return;
    }

    NSString* title = [titleField stringValue];
    std::string pathStr = [path UTF8String];
    std::string titleStr = [title UTF8String];

    auto result = self.cartManager->createCart(pathStr, titleStr, "", "1.0.0", "");

    if (!result.success) {
        NSAlert* errorAlert = [[NSAlert alloc] init];
        [errorAlert setMessageText:@"Failed to Create Cart"];
        [errorAlert setInformativeText:[NSString stringWithUTF8String:result.message.c_str()]];
        [errorAlert setAlertStyle:NSAlertStyleCritical];
        [errorAlert runModal];
        return;
    }

    // Load program into editor (will be empty)
    if (self.textEditor && !result.programSource.empty()) {
        self.textEditor->loadText(result.programSource);
        self.textEditor->setFilename([[path lastPathComponent] UTF8String]);
        self.textEditor->render();
    }

    LOG_INFOF("Cart created: %@", path);
}

- (void)showCartInfo {
    auto info = self.cartManager->getCartInfo();

    NSAlert* alert = [[NSAlert alloc] init];
    [alert setMessageText:[NSString stringWithUTF8String:info.metadata.title.c_str()]];

    NSString* infoText = [NSString stringWithFormat:
        @"Author: %s\nVersion: %s\n\n%s\n\nSprites: %d\nTilesets: %d\nSounds: %d\nMusic: %d\nData Files: %d\n\nSize: %.2f MB\nMode: %s",
        info.metadata.author.c_str(),
        info.metadata.version.c_str(),
        info.metadata.description.c_str(),
        info.spriteCount,
        info.tilesetCount,
        info.soundCount,
        info.musicCount,
        info.dataFileCount,
        info.totalSize / (1024.0 * 1024.0),
        info.mode.c_str()
    ];

    [alert setInformativeText:infoText];
    [alert addButtonWithTitle:@"OK"];
    [alert runModal];
}

// =============================================================================
// Clear Menu Actions
// =============================================================================

- (void)clearAllLayers {
    LOG_INFO("Clearing all layers...");

    // Clear text grid
    if (self.textGrid) {
        self.textGrid->clear();
    }

    // Clear graphics layer (with swap and clear again)
    if (self.graphicsLayer) {
        self.graphicsLayer->clear();
        self.graphicsLayer->swapBuffers();
        self.graphicsLayer->clear();
    }

    // Clear LORES graphics (via SuperTerminal API)
    st_lores_clear(0x000000FF); // Clear with black background

    // Clear displaytext items
    st_text_clear_displayed();
    LOG_INFO("DisplayText items cleared");

    // Clear all rectangles
    st_rect_delete_all();  // Clear all managed rectangles
    LOG_INFO("Rectangle items cleared");

    // Clear all circles
    st_circle_delete_all();  // Clear all managed circles
    LOG_INFO("Circle items cleared");

    // Clear all lines
    st_line_delete_all();  // Clear all managed lines
    LOG_INFO("Line items cleared");

    // Clear all sprites
    if (self.spriteManager) {
        self.spriteManager->clearAll();
        LOG_INFO("Sprite items cleared");
    }

    // Clear any other display layers
    if (self.displayManager) {
        // Force a display refresh to show cleared state
        // Display will refresh automatically on next frame
    }

    LOG_INFO("All layers cleared successfully");
}

// =============================================================================
// =============================================================================
// Interactive Mode
// =============================================================================
// (Extracted to FBRunner3App_InteractiveMode.inc for better organization)
#include "Application/FBRunner3App_InteractiveMode.inc"

// =============================================================================
// Cart REPL Command Handlers
// =============================================================================
// (Extracted to FBRunner3App_CartCommands.inc for better organization)
#include "Application/FBRunner3App_CartCommands.inc"

// =============================================================================
// Window Management
// =============================================================================

// Override to prevent app from quitting when last window closes
// This allows the app to stay running as a normal desktop app
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    return NO;  // Keep app running, user must explicitly quit
}

// Add "Show Main Window" menu item to Window menu
- (void)addShowWindowMenuItem {
    NSMenu* mainMenu = [NSApp mainMenu];

    // Find Window menu
    for (NSMenuItem* item in mainMenu.itemArray) {
        if ([item.submenu.title isEqualToString:@"Window"]) {
            NSMenu* windowMenu = item.submenu;

            // Add separator and "Show Main Window" at the top
            [windowMenu insertItem:[NSMenuItem separatorItem] atIndex:0];

            NSMenuItem* showWindowItem = [[NSMenuItem alloc] initWithTitle:@"Show Main Window"
                                                                    action:@selector(showWindow)
                                                             keyEquivalent:@"0"];
            [showWindowItem setTarget:self];
            [windowMenu insertItem:showWindowItem atIndex:0];

            break;
        }
    }
}

// Show window if it was closed
- (void)showWindow {
    if (self.window) {
        [self.window makeKeyAndOrderFront:nil];
    }
}

// Override newScript to ensure window is visible
- (void)newScript {
    [super newScript];
    [self showWindow];
}

// Override openScript to ensure window is visible
- (void)openScript {
    [super openScript];
    [self showWindow];
}

// Override applicationDidFinishLaunching to start in interactive mode by default
- (void)applicationDidFinishLaunching:(NSNotification*)notification {
    // Call parent initialization first
    [super applicationDidFinishLaunching:notification];

    // Once fully initialized, switch to interactive mode
    LOG_INFO("Starting in interactive mode by default");
    [self enterInteractiveMode];
}

// Save interactive state before running scripts
- (void)saveInteractiveState {
    if (!_interactiveMode) {
        return;
    }

    LOG_INFO("Saving interactive state...");
    _savedInteractiveState.outputLines = _outputLines;
    _savedInteractiveState.currentInput = _currentInput;
    _savedInteractiveState.cursorPos = _cursorPos;
    _savedInteractiveState.scrollOffset = _scrollOffset;
    _savedInteractiveState.history = _history;
    _savedInteractiveState.historyIndex = _historyIndex;
    _savedInteractiveState.autoContinueMode = _autoContinueMode;
    _savedInteractiveState.suggestedNextLine = _suggestedNextLine;
    _savedInteractiveState.lastLineNumber = _lastLineNumber;

    LOG_INFOF("Saved %zu output lines, %zu history entries",
              _savedInteractiveState.outputLines.size(),
              _savedInteractiveState.history.size());
}

// Restore interactive state after scripts complete
- (void)restoreInteractiveState {
    if (_savedInteractiveState.outputLines.empty()) {
        LOG_INFO("No saved interactive state to restore");
        return;
    }

    LOG_INFO("Restoring interactive state...");
    _outputLines = _savedInteractiveState.outputLines;
    _currentInput = _savedInteractiveState.currentInput;
    _cursorPos = _savedInteractiveState.cursorPos;
    _scrollOffset = _savedInteractiveState.scrollOffset;
    _history = _savedInteractiveState.history;
    _historyIndex = _savedInteractiveState.historyIndex;
    _autoContinueMode = _savedInteractiveState.autoContinueMode;
    _suggestedNextLine = _savedInteractiveState.suggestedNextLine;
    _lastLineNumber = _savedInteractiveState.lastLineNumber;

    LOG_INFOF("Restored %zu output lines, %zu history entries",
              _outputLines.size(), _history.size());

    // Clear saved state after restoring
    _savedInteractiveState = InteractiveState();
}

// =============================================================================
// Mouse Handling for Interactive Mode
// =============================================================================

- (void)handleInteractiveMouseClick:(int)gridX y:(int)gridY button:(int)button {
    if (_interactiveMode) {
        // Interactive mode text selection
        _mouseSelecting = true;
        _selectionStartX = gridX;
        _selectionStartY = gridY;
        _selectionEndX = gridX;
        _selectionEndY = gridY;
        LOG_DEBUGF("Interactive mouse click at grid position (%d, %d)", gridX, gridY);
    } else {
        // Runtime mode text selection
        _runtimeMouseSelecting = true;
        _runtimeSelectionStartX = gridX;
        _runtimeSelectionStartY = gridY;
        _runtimeSelectionEndX = gridX;
        _runtimeSelectionEndY = gridY;
        LOG_DEBUGF("Runtime mouse click at grid position (%d, %d)", gridX, gridY);
    }
}

- (void)handleInteractiveMouseRelease {
    if (_interactiveMode && _mouseSelecting) {
        // Interactive mode mouse release
        _mouseSelecting = false;

        // Check if we have a valid selection
        if (_selectionStartX != _selectionEndX || _selectionStartY != _selectionEndY) {
            // Copy selected text to clipboard
            std::string selectedText = [self getSelectedTextFromInteractive];
            if (!selectedText.empty()) {
                NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
                [pasteboard clearContents];
                [pasteboard setString:[NSString stringWithUTF8String:selectedText.c_str()]
                              forType:NSPasteboardTypeString];
                LOG_INFOF("Copied %zu characters to clipboard", selectedText.length());
            }
        }
    } else if (!_interactiveMode && _runtimeMouseSelecting) {
        // Runtime mode mouse release
        _runtimeMouseSelecting = false;

        // Check if we have a valid selection
        if (_runtimeSelectionStartX != _runtimeSelectionEndX || _runtimeSelectionStartY != _runtimeSelectionEndY) {
            // Copy selected text to clipboard
            std::string selectedText = [self getSelectedTextFromRuntime];
            if (!selectedText.empty()) {
                NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
                [pasteboard clearContents];
                [pasteboard setString:[NSString stringWithUTF8String:selectedText.c_str()]
                              forType:NSPasteboardTypeString];
                LOG_INFOF("Copied %zu characters from runtime to clipboard", selectedText.length());
            }
        }
    }
}

- (void)processInteractiveMouseInput {
    if (!self.inputManager) {
        return;
    }

    if (_interactiveMode) {
        // Handle mouse wheel scrolling in interactive mode
        float wheelDx = 0.0f, wheelDy = 0.0f;
        self.inputManager->getMouseWheel(&wheelDx, &wheelDy);

        if (wheelDy != 0.0f) {
            // Scroll by lines (negative deltaY = scroll up)
            constexpr float scrollSensitivity = 3.0f;
            int lines = static_cast<int>(-wheelDy / scrollSensitivity);

            if (lines != 0) {
                _scrollOffset += lines;

                // Clamp scroll offset
                int maxScroll = std::max(0, (int)_outputLines.size() - 1);
                _scrollOffset = std::max(0, std::min(_scrollOffset, maxScroll));

                LOG_DEBUGF("Interactive scrolled by %d lines, offset now: %d", lines, _scrollOffset);
                [self renderInteractiveMode];
            }
        }

        // Handle mouse drag for text selection in interactive mode
        if (_mouseSelecting && self.inputManager->isMouseButtonPressed(SuperTerminal::MouseButton::Left)) {
            int gridX, gridY;
            self.inputManager->getMousePosition(&gridX, &gridY);

            if (gridX != _selectionEndX || gridY != _selectionEndY) {
                _selectionEndX = gridX;
                _selectionEndY = gridY;
                [self renderInteractiveMode];  // Re-render to show selection
            }
        }
    } else {
        // Handle mouse wheel scrolling in runtime mode
        float wheelDx = 0.0f, wheelDy = 0.0f;
        self.inputManager->getMouseWheel(&wheelDx, &wheelDy);

        if (wheelDy != 0.0f) {
            // Scroll by lines (negative deltaY = scroll up)
            constexpr float scrollSensitivity = 3.0f;
            int lines = static_cast<int>(-wheelDy / scrollSensitivity);

            if (lines != 0) {
                // Use existing runtime text scrolling system
                [self scrollRuntimeText:lines];
                LOG_DEBUGF("Runtime mouse scrolled by %d lines", lines);
            }
        }

        // Handle mouse drag for text selection in runtime mode
        if (_runtimeMouseSelecting && self.inputManager->isMouseButtonPressed(SuperTerminal::MouseButton::Left)) {
            int gridX, gridY;
            self.inputManager->getMousePosition(&gridX, &gridY);

            if (gridX != _runtimeSelectionEndX || gridY != _runtimeSelectionEndY) {
                _runtimeSelectionEndX = gridX;
                _runtimeSelectionEndY = gridY;
                [self updateRuntimeDisplay];  // Re-render to show selection
            }
        }
    }
}

- (std::string)getSelectedTextFromInteractive {
    if (!self.textGrid) {
        return "";
    }

    int gridHeight = self.textGrid->getHeight();
    int gridWidth = self.textGrid->getWidth();

    // Normalize selection (start should be before end)
    int startY = std::min(_selectionStartY, _selectionEndY);
    int endY = std::max(_selectionStartY, _selectionEndY);
    int startX = (_selectionStartY < _selectionEndY) ? _selectionStartX : _selectionEndX;
    int endX = (_selectionStartY < _selectionEndY) ? _selectionEndX : _selectionStartX;

    // If same line, ensure startX < endX
    if (startY == endY && startX > endX) {
        std::swap(startX, endX);
    }

    std::string result;

    // Calculate which output lines are visible
    int totalLines = (int)_outputLines.size();
    int displayStartLine = 0;

    if (_scrollOffset > 0) {
        displayStartLine = std::max(0, totalLines - gridHeight - _scrollOffset + 1);
    } else {
        int displayedLines = totalLines + 1;  // +1 for input line
        if (displayedLines > gridHeight) {
            displayStartLine = std::max(0, totalLines - gridHeight + 1);
        }
    }

    // Extract selected text line by line
    for (int screenY = startY; screenY <= endY && screenY < gridHeight; screenY++) {
        int sourceLineIdx = displayStartLine + screenY;

        std::string lineText;
        if (sourceLineIdx >= 0 && sourceLineIdx < totalLines) {
            lineText = _outputLines[sourceLineIdx];
        } else if (sourceLineIdx == totalLines && _scrollOffset == 0) {
            lineText = _currentInput;  // Current input line
        }

        // Extract the portion of this line that's selected
        int lineStartX = (screenY == startY) ? startX : 0;
        int lineEndX = (screenY == endY) ? endX : (int)lineText.length();

        if (lineStartX < lineText.length()) {
            int len = std::min(lineEndX - lineStartX, (int)lineText.length() - lineStartX);
            if (len > 0) {
                result += lineText.substr(lineStartX, len);
            }
        }

        // Add newline between lines (but not after last line)
        if (screenY < endY) {
            result += "\n";
        }
    }

    return result;
}

- (std::string)getSelectedTextFromRuntime {
    if (!self.textGrid) {
        return "";
    }

    // Normalize selection (start should be before end)
    int startY = std::min(_runtimeSelectionStartY, _runtimeSelectionEndY);
    int endY = std::max(_runtimeSelectionStartY, _runtimeSelectionEndY);
    int startX = (_runtimeSelectionStartY < _runtimeSelectionEndY) ? _runtimeSelectionStartX : _runtimeSelectionEndX;
    int endX = (_runtimeSelectionStartY < _runtimeSelectionEndY) ? _runtimeSelectionEndX : _runtimeSelectionStartX;

    // If same line, ensure startX < endX
    if (startY == endY && startX > endX) {
        std::swap(startX, endX);
    }

    std::string result;

    // Calculate which lines are visible based on runtime scroll offset
    int startLine = _runtimeTextState.scrollOffset;
    int screenHeight = _runtimeTextState.screenHeight;

    // Extract selected text from runtime output lines
    for (int screenY = startY; screenY <= endY && screenY < screenHeight; screenY++) {
        int sourceLineIdx = startLine + screenY;

        std::string lineText;
        if (sourceLineIdx >= 0 && sourceLineIdx < _runtimeTextState.outputLines.size()) {
            lineText = _runtimeTextState.outputLines[sourceLineIdx];
        }

        // Extract the portion of this line that's selected
        int lineStartX = (screenY == startY) ? startX : 0;
        int lineEndX = (screenY == endY) ? endX : (int)lineText.length();

        if (lineStartX < lineText.length()) {
            int len = std::min(lineEndX - lineStartX, (int)lineText.length() - lineStartX);
            if (len > 0) {
                result += lineText.substr(lineStartX, len);
            }
        }

        // Add newline between lines (but not after last line)
        if (screenY < endY) {
            result += "\n";
        }
    }

    return result;
}

@end

// =============================================================================
// Entry Point
// =============================================================================

int main(int argc, const char* argv[]) {
    @autoreleasepool {
        // Parse command-line arguments
        std::string scriptPath = "untitled.bas";
        std::string windowSize = "fullhd";  // default: 1920x1080
        std::string outputPath = "";  // for -o option
        bool hasScript = false;
        bool outputLuaOnly = false;

        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--size" || arg == "-s") {
                if (i + 1 < argc) {
                    windowSize = argv[++i];
                }
            } else if (arg == "-o" || arg == "--output") {
                if (i + 1 < argc) {
                    outputPath = argv[++i];
                    outputLuaOnly = true;
                }
            } else if (arg == "--help" || arg == "-h") {
                std::cerr << "Usage: FasterBASIC [options] [script.bas]\n";
                std::cerr << "\nOptions:\n";
                std::cerr << "  -s, --size SIZE   Window size: small, medium, large, fullhd\n";
                std::cerr << "                    small:  640x400  (80x25 grid)\n";
                std::cerr << "                    medium: 800x600  (100x37 grid) [default]\n";
                std::cerr << "                    large:  1280x720 (120x36 grid)\n";
                std::cerr << "                    fullhd: 1920x1080 (120x33 grid)\n";
                std::cerr << "  -o, --output FILE Compile to Lua and save to file (no execution)\n";
                std::cerr << "  -h, --help        Show this help\n";
                std::cerr << "\nExamples:\n";
                std::cerr << "  FasterBASIC                    # Start editor in medium window\n";
                std::cerr << "  FasterBASIC --size large       # Start editor in large window\n";
                std::cerr << "  FasterBASIC --size fullhd program.bas  # Run program in Full HD\n";
                std::cerr << "  FasterBASIC -o output.lua program.bas  # Compile to Lua and save\n";
                return 0;
            } else if (!arg.empty() && arg[0] != '-') {
                scriptPath = arg;
                hasScript = true;
            }
        }

        // Handle compile-only mode
        if (outputLuaOnly) {
            if (!hasScript) {
                std::cerr << "ERROR: -o option requires an input BASIC script\n";
                return 1;
            }

            // Load the script file
            std::ifstream file(scriptPath);
            if (!file.is_open()) {
                std::cerr << "ERROR: Cannot open script file: " << scriptPath << std::endl;
                return 1;
            }

            std::string basicSource((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
            file.close();

            // Compile BASIC to Lua without starting GUI
            try {
                // Use standalone compilation function
                std::string compileError;
                std::string luaCode = compileBASICToLuaStandalone(basicSource, compileError);

                if (luaCode.empty()) {
                    std::cerr << "COMPILATION ERROR: " << compileError << std::endl;
                    return 1;
                }

                // Write Lua code to output file
                std::ofstream outFile(outputPath);
                if (!outFile.is_open()) {
                    std::cerr << "ERROR: Cannot create output file: " << outputPath << std::endl;
                    return 1;
                }

                outFile << luaCode;
                outFile.close();

                std::cerr << "SUCCESS: Compiled " << scriptPath << " to " << outputPath
                         << " (" << luaCode.length() << " bytes of Lua)" << std::endl;
                return 0;

            } catch (const std::exception& e) {
                std::cerr << "COMPILATION ERROR: " << e.what() << std::endl;
                return 1;
            }
        }

        if (!hasScript) {
            std::cerr << "\n";
            std::cerr << "=================================================\n";
            std::cerr << "  FBRunner3 - FasterBASICT Runtime\n";
            std::cerr << "  Powered by FasterBASICT + LuaJIT\n";
            std::cerr << "=================================================\n";
            std::cerr << "\n";
            std::cerr << "Starting in editor mode...\n";
            std::cerr << "\n";
            std::cerr << "Keyboard Shortcuts:\n";
            std::cerr << "  Cmd+R: Run script\n";
            std::cerr << "  Cmd+S: Save script\n";
            std::cerr << "  Cmd+O: Open script\n";
            std::cerr << "  Cmd+.: Stop script\n";
            std::cerr << "  Cmd+Q: Quit\n";
            std::cerr << "\n";
        }

        NSApplication* app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        FBRunner3App* delegate = [[FBRunner3App alloc] initWithScriptPath:scriptPath
                                                                runnerName:"FasterBASIC"];

        // Store window size preference for initialization
        delegate.preferredWindowSize = windowSize;

        [app setDelegate:delegate];
        [app run];

        return 0;
    }
}
