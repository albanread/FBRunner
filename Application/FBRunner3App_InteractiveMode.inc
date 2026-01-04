// Interactive Mode
// =============================================================================

- (void)enterInteractiveMode {
    if (_interactiveMode) {
        LOG_INFO("Already in interactive mode");
        return;
    }

    LOG_INFO("Entering interactive mode...");

    // Initialize Lua state for interactive mode
    _interactiveLuaState = luaL_newstate();
    if (_interactiveLuaState) {
        luaL_openlibs(_interactiveLuaState);
        register_unicode_module(_interactiveLuaState);
        register_bitwise_module(_interactiveLuaState);
        register_constants_module(_interactiveLuaState);
        FBTBindings::registerBindings(_interactiveLuaState);

        // Override print to add to output lines
        lua_pushcfunction(_interactiveLuaState, [](lua_State* L) -> int {
            FBRunner3App* app = (FBRunner3App*)g_runnerInstance;
            if (app) {
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
                    } else if (lua_isnumber(L, i)) {
                        message += std::to_string(lua_tonumber(L, i));
                    }
                }
                app->_outputLines.push_back(message);
            }
            return 0;
        });
        lua_setglobal(_interactiveLuaState, "print");

        // Override os.exit to prevent it from killing the entire app
        // When a BASIC script does END or an error occurs, the generated Lua
        // code calls os.exit(). We intercept this and throw a Lua error instead.
        lua_pushcfunction(_interactiveLuaState, [](lua_State* L) -> int {
            int exitcode = luaL_optinteger(L, 1, 0);
            // Instead of exiting the app, throw a Lua error
            if (exitcode == 0) {
                luaL_error(L, "Script ended normally (os.exit called)");
            } else {
                luaL_error(L, "Script ended with error code %d (os.exit called)", exitcode);
            }
            return 0;
        });
        lua_setglobal(_interactiveLuaState, "exit");  // Set as global function 'exit'

        // Also override it in the os table
        lua_getglobal(_interactiveLuaState, "os");
        lua_pushcfunction(_interactiveLuaState, [](lua_State* L) -> int {
            int exitcode = luaL_optinteger(L, 1, 0);
            if (exitcode == 0) {
                luaL_error(L, "Script ended normally (os.exit called)");
            } else {
                luaL_error(L, "Script ended with error code %d (os.exit called)", exitcode);
            }
            return 0;
        });
        lua_setfield(_interactiveLuaState, -2, "exit");
        lua_pop(_interactiveLuaState, 1);  // Pop the os table
    }

    // Initialize modular commands registry for help system
    LOG_INFO("Initializing command registry for interactive mode...");
    initializeFBRunner3CommandRegistry();
    LOG_INFO("✓ Command registry initialized");

    // Create shell components FIRST (needed for syncing)
    _commandParser = std::make_unique<FasterBASIC::CommandParser>();
    _programManager = std::make_unique<FasterBASIC::ProgramManagerV2>();
    _interactiveProgramRunning = false;

    // Create EditorBridge to sync between TextEditor and ProgramManager
    if (self.textEditor && self.textEditor->getTextBuffer()) {
        try {
            _editorBridge = std::make_unique<FBRunner3::EditorBridge>(
                self.textEditor->getTextBuffer(),
                _programManager->getDocument()
            );
            _editorBridge->setShowLineNumbers(true);
            _editorBridge->setPreserveCursor(true);
            LOG_INFO("EditorBridge created successfully");
        } catch (const std::exception& e) {
            LOG_INFOF("Failed to create EditorBridge: %s", e.what());
            _editorBridge.reset();
        }
    }

    // Sync editor content into SourceDocument (Editor → Shell)
    // This ensures the shell starts with the current editor content
    if (self.textEditor && _commandParser) {
        LOG_INFO("Syncing TextBuffer → SourceDocument");
        [self syncEditorToProgramManager];
    }

    // Initialize display state (or restore saved state)
    if (_savedInteractiveState.outputLines.empty()) {
        // No saved state - initialize with defaults
        _outputLines.clear();
        _outputLines.push_back("FasterBASIC SuperTerminal 2025");
        _outputLines.push_back("Ready.");
        _currentInput = "";
        _cursorPos = 0;
        _scrollOffset = 0;
        _history.clear();
        _historyIndex = -1;
    } else {
        // Restore saved interactive state
        [self restoreInteractiveState];
        // Add a separator line to show we're back
        _outputLines.push_back("");
        _outputLines.push_back("-- program ended");
        _scrollOffset = 0;  // Return to live view when coming back
    }

    _interactiveMode = true;

    // Exit editor mode if active
    if (self.editorMode) {
        self.editorMode = NO;
    }

    // CRITICAL: Always restore TEXT mode when entering interactive mode
    // Scripts may have switched to graphics modes (LORES, XRES, WRES, URES)
    // Interactive mode needs text display, so disable all graphics modes
    if (self.displayManager) {
        self.displayManager->setLoResMode(false);
        self.displayManager->setXResMode(false);
        self.displayManager->setWResMode(false);
        self.displayManager->setUResMode(false);
    }

    // Update status bar
    if (self.displayManager) {
        self.displayManager->setStatusBarVisible(true);
        std::string statusText = "● BASIC interaction";
        std::string currentFilename = self.textEditor->getFilename();
        if (!currentFilename.empty() && currentFilename.find("untitled") != 0) {
            statusText += " - " + currentFilename;
            LOG_INFOF("[enterInteractiveMode] Status includes script name: %s", currentFilename.c_str());
        } else {
            LOG_INFO("[enterInteractiveMode] Filename is empty or untitled");
        }
        self.displayManager->updateStatus(statusText.c_str());
        LOG_INFOF("[enterInteractiveMode] Updated status to: %s", statusText.c_str());
    }

    // CRITICAL FIX: Make the display view first responder to receive keyboard input
    // Without this, input events may go to the wrong responder chain and appear "jammed"
    if (self.displayManager) {
        NSView* displayView = self.displayManager->getMetalView();
        if (displayView && self.window) {
            [self.window makeFirstResponder:displayView];
            LOG_INFO("Made display view first responder for interactive mode");
        }
    }

    LOG_INFO("Interactive mode active");
}

- (void)saveScript {
    // Override to handle interactive mode
    if (_interactiveMode) {
        // Save from ProgramManager instead of TextEditor
        if (!self.scriptDatabase) {
            LOG_ERROR("No script database");
            return;
        }

        std::string filename;
        std::string currentFilename = self.textEditor->getFilename();
        if (currentFilename.empty() || currentFilename.find("untitled") == 0) {
            LOG_INFO("[enterRuntimeMode] No script name set, cannot update status bar");
            return;
        }

        filename = currentFilename;

        // Get program content
        std::string content = _programManager->generateProgram();

        // Format it before saving
        FasterBASIC::FormatterOptions options;
        options.start_line = -1; // Keep original line numbers
        options.step = 10;
        options.indent_spaces = 0;
        options.update_references = false;
        options.add_indentation = false;

        FasterBASIC::FormatterResult result = FasterBASIC::formatClassic(content);
        if (result.success && !result.formatted_code.empty()) {
            content = result.formatted_code;
        }

        // Save as BASIC
        SuperTerminal::ScriptLanguage lang = SuperTerminal::ScriptLanguage::BASIC;
        if (self.scriptDatabase->saveScript(filename, lang, content, "")) {
            LOG_INFOF("✓ Script saved from interactive mode: %s", filename.c_str());
        } else {
            LOG_ERROR("Failed to save script");
            _outputLines.push_back("ERROR: Failed to save script");
        }
    } else {
        // Call parent implementation for editor mode
        [super saveScript];
    }
}

- (void)formatScript {
    // FBRunner3 is always BASIC, so just format without language check
    [self formatScriptWithStartLine:1000 step:10];
}

- (void)formatScriptWithStartLine:(int)startLine step:(int)step {
    // Ensure we're on the main thread
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self formatScriptWithStartLine:startLine step:step];
        });
        return;
    }

    if (_interactiveMode) {
        // Format from ProgramManager in interactive mode
        if (!_programManager || _programManager->isEmpty()) {
            _outputLines.push_back("No program to format");
            return;
        }

        // Get current program
        std::string content = _programManager->generateProgram();

        // Format it
        FasterBASIC::FormatterOptions options;
        options.start_line = startLine;
        options.step = step;
        options.indent_spaces = 2;
        options.update_references = true;
        options.add_indentation = true;

        FasterBASIC::FormatterResult result = FasterBASIC::formatBasicCode(content, options);

        if (result.success) {
            // Clear and reload program
            _programManager->clear();

            // Parse and load formatted code
            std::istringstream iss(result.formatted_code);
            std::string line;
            while (std::getline(iss, line)) {
                if (!line.empty()) {
                    FasterBASIC::ParsedCommand cmd = _commandParser->parse(line);
                    if (cmd.type == FasterBASIC::ShellCommandType::DIRECT_LINE) {
                        _programManager->setLine(cmd.lineNumber, cmd.code);
                    }
                }
            }

            _outputLines.push_back("Program formatted (" + std::to_string(result.lines_processed) + " lines)");
            LOG_INFOF("✓ Formatted %d lines in interactive mode", result.lines_processed);
        } else {
            _outputLines.push_back("FORMAT ERROR: " + result.error_message);
            LOG_INFOF("Format error: %s", result.error_message.c_str());
        }
    } else {
        // Format in editor mode
        if (!self.textEditor) {
            LOG_INFO("No text editor available");
            return;
        }

        std::string content = self.textEditor->getText();
        if (content.empty()) {
            LOG_INFO("No content to format");
            return;
        }

        // Check if it has valid line numbers
        if (!FasterBASIC::hasValidLineNumbers(content)) {
            NSAlert* alert = [[NSAlert alloc] init];
            [alert setMessageText:@"Format Not Available"];
            [alert setInformativeText:@"This code does not have line numbers. Formatting requires numbered BASIC code."];
            [alert addButtonWithTitle:@"OK"];
            [alert beginSheetModalForWindow:self.window completionHandler:nil];
            return;
        }

        // Format the code
        FasterBASIC::FormatterOptions options;
        options.start_line = startLine;
        options.step = step;
        options.indent_spaces = 2;
        options.update_references = true;
        options.add_indentation = true;

        FasterBASIC::FormatterResult result = FasterBASIC::formatBasicCode(content, options);

        if (result.success) {
            self.textEditor->loadText(result.formatted_code);
            self.textEditor->markDirty();
            LOG_INFOF("✓ Formatted %d lines in editor mode", result.lines_processed);
        } else {
            NSAlert* alert = [[NSAlert alloc] init];
            [alert setMessageText:@"Format Error"];
            [alert setInformativeText:[NSString stringWithFormat:@"%s", result.error_message.c_str()]];
            [alert addButtonWithTitle:@"OK"];
            [alert beginSheetModalForWindow:self.window completionHandler:nil];
            LOG_INFOF("Format error: %s", result.error_message.c_str());
        }
    }
}

- (void)renumberScript {
    // FBRunner3 is always BASIC, so just renumber without language check
    [self renumberScriptWithStartLine:1000 step:10];
}

- (void)renumberScriptWithStartLine:(int)startLine step:(int)step {
    // Ensure we're on the main thread
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self renumberScriptWithStartLine:startLine step:step];
        });
        return;
    }

    if (_interactiveMode) {
        // Renumber from ProgramManager in interactive mode
        if (!_programManager || _programManager->isEmpty()) {
            _outputLines.push_back("No program to renumber");
            return;
        }

        // Get current program
        std::string content = _programManager->generateProgram();

        // Renumber it (use renumberBasicCode for renumber-only operation)
        FasterBASIC::FormatterResult result = FasterBASIC::renumberBasicCode(content, startLine, step);

        if (result.success) {
            // Clear and reload program
            _programManager->clear();

            // Parse and load renumbered code
            std::istringstream iss(result.formatted_code);
            std::string line;
            while (std::getline(iss, line)) {
                if (!line.empty()) {
                    FasterBASIC::ParsedCommand cmd = _commandParser->parse(line);
                    if (cmd.type == FasterBASIC::ShellCommandType::DIRECT_LINE) {
                        _programManager->setLine(cmd.lineNumber, cmd.code);
                    }
                }
            }

            _outputLines.push_back("Program renumbered (" + std::to_string(result.lines_processed) + " lines)");
            LOG_INFOF("✓ Renumbered %d lines in interactive mode", result.lines_processed);
        } else {
            _outputLines.push_back("RENUMBER ERROR: " + result.error_message);
            LOG_INFOF("Renumber error: %s", result.error_message.c_str());
        }
    } else {
        // Renumber in editor mode
        if (!self.textEditor) {
            LOG_INFO("No text editor available");
            return;
        }

        std::string content = self.textEditor->getText();
        if (content.empty()) {
            LOG_INFO("No content to renumber");
            return;
        }

        // Renumber the code
        FasterBASIC::FormatterResult result = FasterBASIC::renumberBasicCode(content, startLine, step);

        if (result.success) {
            self.textEditor->loadText(result.formatted_code);
            self.textEditor->markDirty();
            LOG_INFOF("✓ Renumbered %d lines in editor mode", result.lines_processed);
        } else {
            NSAlert* alert = [[NSAlert alloc] init];
            [alert setMessageText:@"Renumber Error"];
            [alert setInformativeText:[NSString stringWithFormat:@"%s", result.error_message.c_str()]];
            [alert addButtonWithTitle:@"OK"];
            [alert beginSheetModalForWindow:self.window completionHandler:nil];
            LOG_INFOF("Renumber error: %s", result.error_message.c_str());
        }
    }
}

- (void)pasteText {
    if (_interactiveMode) {
        // Handle paste in interactive mode
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        NSString* pasteText = [pasteboard stringForType:NSPasteboardTypeString];
        if (pasteText) {
            std::string pasteStr = [pasteText UTF8String];
            // Insert pasted text at cursor position
            _currentInput.insert(_cursorPos, pasteStr);
            _cursorPos += pasteStr.length();
        }
    } else {
        // Call parent implementation for editor mode
        [super pasteText];
    }
}

- (void)exitInteractiveMode {
    if (!_interactiveMode) {
        return;
    }

    LOG_INFO("Exiting interactive mode...");

    // Stop any running program
    _interactiveProgramRunning = false;

    // Note: We don't sync here - enterEditorMode() will ALWAYS sync from
    // SourceDocument when entering editor mode. This ensures the editor
    // always displays the latest state and simplifies the logic.
    // The SourceDocument is the single source of truth.

    // Clean up Lua state
    if (_interactiveLuaState) {
        lua_close(_interactiveLuaState);
        _interactiveLuaState = nullptr;
    }

    // Clear shell components and display state
    _commandParser.reset();
    _programManager.reset();
    _outputLines.clear();
    _currentInput = "";
    _history.clear();

    _interactiveMode = false;

    // Don't clear TextGrid directly - it will be cleared by render loop
    // (calling clear() here causes thread safety crash with CVDisplayLink)

    LOG_INFO("Interactive mode exited");
}

- (void)toggleInteractiveMode {
    if (_interactiveMode) {
        [self exitInteractiveMode];
        [self enterEditorMode];
    } else {
        [self enterInteractiveMode];
    }
}

- (void)updateInteractiveMode {
    if (!_interactiveMode || !self.inputManager) {
        return;
    }

    // Handle mouse input (scrolling and selection)
    [self processInteractiveMouseInput];

    // Handle keyboard input for text entry
    while (self.inputManager->hasCharacters()) {
        uint32_t ch = self.inputManager->getNextCharacter();
        if (ch >= 32 && ch < 127) {
            _currentInput.insert(_cursorPos, 1, (char)ch);
            _cursorPos++;
        }
    }

    // Handle special keys
    if (self.inputManager->isKeyJustPressed(KeyCode::Enter)) {
        LOG_INFOF("[Interactive] Enter pressed, currentInput: %s", _currentInput.c_str());
        // In AUTO-CONTINUATION mode, check if line is empty (just the line number)
        if (_autoContinueMode) {
            std::string lineNumStr = std::to_string(_suggestedNextLine) + " ";
            if (_currentInput == lineNumStr || _currentInput.length() <= lineNumStr.length()) {
                // Empty line - exit AUTO-CONTINUATION mode
                _outputLines.push_back(_currentInput);
                _autoContinueMode = false;
                _suggestedNextLine = 0;
                _lastLineNumber = 0;
                _currentInput = "";
                _cursorPos = 0;
                _historyIndex = -1;
                [self showPrompt];
                return;
            }
        }

        // Get the actual line to execute
        std::string line = _currentInput;

        LOG_INFOF("[Interactive] About to execute line: %s", line.c_str());
        LOG_INFOF("[Interactive] Output lines before execution: %zu", _outputLines.size());

        // Add the input line to output (showing what was typed)
        _outputLines.push_back(_currentInput);

        // Execute the line
        if (!line.empty()) {
            _history.push_back(line);
            if (_history.size() > 100) {
                _history.erase(_history.begin());
            }
            bool shouldClearInput = [self executeInteractiveLine:line];
            LOG_INFOF("[Interactive] After executeInteractiveLine, output lines: %zu, shouldClearInput: %d", _outputLines.size(), shouldClearInput);
            if (shouldClearInput) {
                // Clear input for next line
                _currentInput = "";
                _cursorPos = 0;
                _historyIndex = -1;
            }
        } else {
            // Empty line, clear input
            _currentInput = "";
            _cursorPos = 0;
            _historyIndex = -1;
        }
    }

    if (self.inputManager->isKeyJustPressed(KeyCode::Escape)) {
        if (_autoContinueMode) {
            // In auto-continuation mode, save current line if it has content beyond line number
            std::string lineNumStr = std::to_string(_suggestedNextLine) + " ";
            if (_currentInput.length() > lineNumStr.length()) {
                // There's content to save
                std::string line = _currentInput;
                _outputLines.push_back(line);

                // Parse and save the line
                FasterBASIC::ParsedCommand cmd = _commandParser->parse(line);
                if (cmd.type == FasterBASIC::ShellCommandType::DIRECT_LINE) {
                    std::string formattedCode = _commandParser->formatBasicKeywords(cmd.code);
                    _programManager->setLine(cmd.lineNumber, formattedCode);
                }
            }

            // Exit auto-continuation mode
            _autoContinueMode = false;
            _suggestedNextLine = 0;
            _lastLineNumber = 0;
            _currentInput = "";
            _cursorPos = 0;
            [self showPrompt];
        }
    }

    if (self.inputManager->isKeyJustPressed(KeyCode::Backspace)) {
        if (_cursorPos > 0) {
            _currentInput.erase(_cursorPos - 1, 1);
            _cursorPos--;
        }
    }

    if (self.inputManager->isKeyJustPressed(KeyCode::Left)) {
        if (self.inputManager->isCommandPressed()) {
            // Command+Left: Move cursor to start of line (macOS standard)
            _cursorPos = 0;
        } else if (self.inputManager->isAltPressed() || self.inputManager->isControlPressed()) {
            // Option+Left or Ctrl+Left: Move to previous word boundary
            if (_cursorPos > 0) {
                // Skip any spaces/punctuation at current position
                while (_cursorPos > 0 && (_currentInput[_cursorPos - 1] == ' ' ||
                                          _currentInput[_cursorPos - 1] == ',' ||
                                          _currentInput[_cursorPos - 1] == ';' ||
                                          _currentInput[_cursorPos - 1] == ':' ||
                                          _currentInput[_cursorPos - 1] == '(' ||
                                          _currentInput[_cursorPos - 1] == ')' ||
                                          _currentInput[_cursorPos - 1] == '"' ||
                                          _currentInput[_cursorPos - 1] == '\'')) {
                    _cursorPos--;
                }
                // Move to start of word
                while (_cursorPos > 0 && _currentInput[_cursorPos - 1] != ' ' &&
                                         _currentInput[_cursorPos - 1] != ',' &&
                                         _currentInput[_cursorPos - 1] != ';' &&
                                         _currentInput[_cursorPos - 1] != ':' &&
                                         _currentInput[_cursorPos - 1] != '(' &&
                                         _currentInput[_cursorPos - 1] != ')' &&
                                         _currentInput[_cursorPos - 1] != '"' &&
                                         _currentInput[_cursorPos - 1] != '\'') {
                    _cursorPos--;
                }
            }
        } else {
            // Normal left arrow
            if (_cursorPos > 0) _cursorPos--;
        }
    }

    if (self.inputManager->isKeyJustPressed(KeyCode::Right)) {
        if (self.inputManager->isCommandPressed()) {
            // Command+Right: Move cursor to end of line (macOS standard)
            _cursorPos = _currentInput.length();
        } else if (self.inputManager->isAltPressed() || self.inputManager->isControlPressed()) {
            // Option+Right or Ctrl+Right: Move to next word boundary
            if (_cursorPos < _currentInput.length()) {
                // Skip current word
                while (_cursorPos < _currentInput.length() &&
                       _currentInput[_cursorPos] != ' ' &&
                       _currentInput[_cursorPos] != ',' &&
                       _currentInput[_cursorPos] != ';' &&
                       _currentInput[_cursorPos] != ':' &&
                       _currentInput[_cursorPos] != '(' &&
                       _currentInput[_cursorPos] != ')' &&
                       _currentInput[_cursorPos] != '"' &&
                       _currentInput[_cursorPos] != '\'') {
                    _cursorPos++;
                }
                // Skip any spaces/punctuation
                while (_cursorPos < _currentInput.length() &&
                       (_currentInput[_cursorPos] == ' ' ||
                        _currentInput[_cursorPos] == ',' ||
                        _currentInput[_cursorPos] == ';' ||
                        _currentInput[_cursorPos] == ':' ||
                        _currentInput[_cursorPos] == '(' ||
                        _currentInput[_cursorPos] == ')' ||
                        _currentInput[_cursorPos] == '"' ||
                        _currentInput[_cursorPos] == '\'')) {
                    _cursorPos++;
                }
            }
        } else {
            // Normal right arrow
            if (_cursorPos < _currentInput.length()) _cursorPos++;
        }
    }

    if (self.inputManager->isKeyJustPressed(KeyCode::Delete)) {
        // Delete character at cursor position
        if (_cursorPos < _currentInput.length()) {
            _currentInput.erase(_cursorPos, 1);
        }
    }

    if (self.inputManager->isKeyJustPressed(KeyCode::Up)) {
        if (_autoContinueMode) {
            // In auto-continuation mode: save current line and navigate to previous line
            std::string lineNumStr = std::to_string(_suggestedNextLine) + " ";
            if (_currentInput.length() > lineNumStr.length()) {
                // Save current line if it has content
                std::string line = _currentInput;
                _outputLines.push_back(line);

                FasterBASIC::ParsedCommand cmd = _commandParser->parse(line);
                if (cmd.type == FasterBASIC::ShellCommandType::DIRECT_LINE) {
                    std::string formattedCode = _commandParser->formatBasicKeywords(cmd.code);
                    _programManager->setLine(cmd.lineNumber, formattedCode);
                }
            }

            // Find previous line number
            int prevLine = [self findPreviousLineNumber:_suggestedNextLine];
            if (prevLine > 0) {
                _lastLineNumber = prevLine;
                _suggestedNextLine = prevLine;

                // Load previous line for editing
                if (_programManager->hasLine(prevLine)) {
                    std::string existingContent = _programManager->getLine(prevLine);
                    _currentInput = std::to_string(prevLine) + " " + existingContent;
                } else {
                    _currentInput = std::to_string(prevLine) + " ";
                }
                _cursorPos = _currentInput.length();
            } else {
                // No previous line exists, exit auto-continuation
                _autoContinueMode = false;
                _suggestedNextLine = 0;
                _lastLineNumber = 0;
                _currentInput = "";
                _cursorPos = 0;
                [self showPrompt];
            }
        } else if (!_history.empty()) {
            // Normal history navigation
            if (_historyIndex == -1) {
                _historyIndex = (int)_history.size() - 1;
            } else if (_historyIndex > 0) {
                _historyIndex--;
            }
            if (_historyIndex >= 0 && _historyIndex < _history.size()) {
                _currentInput = _history[_historyIndex];
                _cursorPos = _currentInput.length();
            }
        }
    }

    if (self.inputManager->isKeyJustPressed(KeyCode::Down)) {
        if (_autoContinueMode) {
            // In auto-continuation mode: save current line and navigate to next line
            std::string lineNumStr = std::to_string(_suggestedNextLine) + " ";
            if (_currentInput.length() > lineNumStr.length()) {
                // Save current line if it has content
                std::string line = _currentInput;
                _outputLines.push_back(line);

                FasterBASIC::ParsedCommand cmd = _commandParser->parse(line);
                if (cmd.type == FasterBASIC::ShellCommandType::DIRECT_LINE) {
                    std::string formattedCode = _commandParser->formatBasicKeywords(cmd.code);
                    _programManager->setLine(cmd.lineNumber, formattedCode);
                }
            }

            // Find next line number
            int nextLine = [self findNextAvailableLineNumber:_suggestedNextLine];
            if (nextLine > 0 && nextLine < 65536) {
                _lastLineNumber = _suggestedNextLine;
                _suggestedNextLine = nextLine;

                // Load next line for editing
                if (_programManager->hasLine(nextLine)) {
                    std::string existingContent = _programManager->getLine(nextLine);
                    _currentInput = std::to_string(nextLine) + " " + existingContent;
                } else {
                    _currentInput = std::to_string(nextLine) + " ";
                }
                _cursorPos = _currentInput.length();
            } else {
                // No next line exists, exit auto-continuation
                _autoContinueMode = false;
                _suggestedNextLine = 0;
                _lastLineNumber = 0;
                _currentInput = "";
                _cursorPos = 0;
                [self showPrompt];
            }
        } else if (_historyIndex != -1) {
            // Normal history navigation
            _historyIndex++;
            if (_historyIndex >= _history.size()) {
                _currentInput = "";
                _historyIndex = -1;
            } else {
                _currentInput = _history[_historyIndex];
            }
            _cursorPos = _currentInput.length();
        }
    }

    // Handle Page Up/Down for scroll-back functionality
    if (self.inputManager->isKeyJustPressed(KeyCode::PageUp)) {
        // Scroll back (up) in history
        int totalLines = (int)_outputLines.size();
        int maxScroll = std::max(0, totalLines - 1);  // Don't scroll past first line
        _scrollOffset = std::min(_scrollOffset + 10, maxScroll);  // Scroll by 10 lines
    }

    if (self.inputManager->isKeyJustPressed(KeyCode::PageDown)) {
        // Scroll forward (down) towards current
        _scrollOffset = std::max(0, _scrollOffset - 10);  // Scroll by 10 lines
    }

    // Handle End key to return to live view
    if (self.inputManager->isKeyJustPressed(KeyCode::End)) {
        _scrollOffset = 0;  // Return to live view
    }

    // Auto-return to bottom when user types
    if (self.inputManager->hasCharacters() && _scrollOffset > 0) {
        _scrollOffset = 0;  // Return to live view when typing
    }

    // Render the display
    LOG_DEBUGF("About to render, output lines: %zu, currentInput: %s, scrollOffset: %d",
               _outputLines.size(), _currentInput.c_str(), _scrollOffset);
    [self renderInteractiveMode];
}



- (void)renderInteractiveMode {
    if (!self.textGrid) {
        LOG_INFO("[Render] No textGrid available!");
        return;
    }

    int gridHeight = self.textGrid->getHeight();
    int gridWidth = self.textGrid->getWidth();

    LOG_DEBUGF("Starting render: gridHeight=%d, outputLines=%zu, scrollOffset=%d",
               gridHeight, _outputLines.size(), _scrollOffset);

    // Clear screen
    self.textGrid->clear();

    // Calculate scroll position
    int totalLines = (int)_outputLines.size();
    int startLine = 0;

    if (_scrollOffset > 0) {
        // User is scrolled back in history
        startLine = std::max(0, totalLines - gridHeight - _scrollOffset + 1);
    } else {
        // Normal auto-scroll to show current cursor position
        int displayedLines = totalLines + 1;  // +1 for input line
        if (displayedLines > gridHeight) {
            // Scroll to keep bottom visible
            startLine = std::max(0, totalLines - gridHeight + 1);
        }
    }

    // Render output lines (history) with syntax highlighting and selection
    int y = 0;
    for (int i = startLine; i < totalLines && y < gridHeight; i++, y++) {
        const std::string& line = _outputLines[i];

        // Get syntax highlighting colors using the same highlighter as the editor
        std::vector<uint32_t> colors = [self highlightLine:line lineNumber:i];

        for (int x = 0; x < std::min((int)line.length(), gridWidth); x++) {
            uint32_t fgColor = (x < colors.size()) ? colors[x] : 0xFFFFFFFF;
            uint32_t bgColor = 0xFF000000;  // Default black background

            // Check if this character is in the selection
            if (_mouseSelecting) {
                int minY = std::min(_selectionStartY, _selectionEndY);
                int maxY = std::max(_selectionStartY, _selectionEndY);

                if (y >= minY && y <= maxY) {
                    bool isSelected = false;

                    if (minY == maxY) {
                        // Single line selection
                        int minX = std::min(_selectionStartX, _selectionEndX);
                        int maxX = std::max(_selectionStartX, _selectionEndX);
                        isSelected = (x >= minX && x <= maxX);
                    } else if (y == minY) {
                        // First line of multi-line selection
                        int startX = (_selectionStartY == minY) ? _selectionStartX : _selectionEndX;
                        isSelected = (x >= startX);
                    } else if (y == maxY) {
                        // Last line of multi-line selection
                        int endX = (_selectionEndY == maxY) ? _selectionEndX : _selectionStartX;
                        isSelected = (x <= endX);
                    } else {
                        // Middle line - fully selected
                        isSelected = true;
                    }

                    if (isSelected) {
                        bgColor = 0xFF444444;  // Gray background for selection
                    }
                }
            }

            self.textGrid->putChar(x, y, line[x], fgColor, bgColor);
        }
    }

    // Show scroll indicator if scrolled back
    if (_scrollOffset > 0) {
        std::string indicator = "(Scrolled back " + std::to_string(_scrollOffset) + " lines - Press END for latest)";
        int indicatorY = gridHeight - 2;  // Show near bottom
        if (indicatorY >= 0) {
            for (int x = 0; x < std::min((int)indicator.length(), gridWidth); x++) {
                self.textGrid->putChar(x, indicatorY, indicator[x], 0xFFFFFF00, 0xFF000000);  // Yellow text
            }
        }
    }

    // Render current input line with syntax highlighting (only if not scrolled back)
    if (y < gridHeight && _scrollOffset == 0) {
        std::string inputLine = _currentInput;

        // Get syntax highlighting colors using the same highlighter as the editor
        std::vector<uint32_t> colors = [self highlightLine:inputLine lineNumber:0];

        // Render input line with colors and selection
        for (int x = 0; x < std::min((int)inputLine.length(), gridWidth); x++) {
            uint32_t fgColor = (x < colors.size()) ? colors[x] : 0xFFFFFFFF;
            uint32_t bgColor = 0xFF000000;  // Default black background

            // Check if this character is in the selection
            if (_mouseSelecting && y >= std::min(_selectionStartY, _selectionEndY) &&
                y <= std::max(_selectionStartY, _selectionEndY)) {
                int minY = std::min(_selectionStartY, _selectionEndY);
                int maxY = std::max(_selectionStartY, _selectionEndY);

                bool isSelected = false;
                if (minY == maxY) {
                    int minX = std::min(_selectionStartX, _selectionEndX);
                    int maxX = std::max(_selectionStartX, _selectionEndX);
                    isSelected = (x >= minX && x <= maxX);
                } else if (y == minY) {
                    int startX = (_selectionStartY == minY) ? _selectionStartX : _selectionEndX;
                    isSelected = (x >= startX);
                } else if (y == maxY) {
                    int endX = (_selectionEndY == maxY) ? _selectionEndX : _selectionStartX;
                    isSelected = (x <= endX);
                } else {
                    isSelected = true;
                }

                if (isSelected) {
                    bgColor = 0xFF444444;  // Gray background for selection
                }
            }

            if (x == _cursorPos && !_mouseSelecting) {
                char cursorChar = (x < inputLine.length()) ? inputLine[x] : ' ';
                // Yellow block cursor with black text (like editor)
                self.textGrid->putChar(x, y, cursorChar, 0x000000FF, 0xFFD700FF);
            } else {
                self.textGrid->putChar(x, y, inputLine[x], fgColor, bgColor);
            }
        }

        // If cursor is at end of line, show yellow block on space
        if (_cursorPos >= inputLine.length() && _cursorPos < gridWidth && !_mouseSelecting) {
            self.textGrid->putChar(_cursorPos, y, ' ', 0x000000FF, 0xFFD700FF);
        }
    }
}

- (bool)executeInteractiveLine:(const std::string&)line {
    if (!_commandParser || !_programManager) {
        _outputLines.push_back("ERROR: Shell not initialized");
        [self showPrompt];
        return true;
    }

    // Parse the command using fbsh CommandParser
    FasterBASIC::ParsedCommand cmd = _commandParser->parse(line);

    switch (cmd.type) {
        case FasterBASIC::ShellCommandType::DIRECT_LINE: {
            // Format BASIC keywords (uppercase them) before saving
            std::string formattedCode = _commandParser->formatBasicKeywords(cmd.code);

            // Add or replace program line
            _programManager->setLine(cmd.lineNumber, formattedCode);

            // Enter AUTO-CONTINUATION mode - suggest next line number
            _lastLineNumber = cmd.lineNumber;
            int nextLine = [self findNextAvailableLineNumber:cmd.lineNumber];
            if (nextLine > 0 && nextLine < 65536) {
                _autoContinueMode = true;
                _suggestedNextLine = nextLine;

                // Check if next line already exists - if so, load it for editing
                if (_programManager->hasLine(nextLine)) {
                    std::string existingContent = _programManager->getLine(nextLine);
                    _currentInput = std::to_string(_suggestedNextLine) + " " + existingContent;
                } else {
                    // Show empty line number ready for new content
                    _currentInput = std::to_string(_suggestedNextLine) + " ";
                }
                _cursorPos = _currentInput.length();
            }
            return false; // Don't clear input - we set up next line number
        }

        case FasterBASIC::ShellCommandType::DELETE_LINE:
            // Delete program line
            _programManager->deleteLine(cmd.lineNumber);
            if (_autoContinueMode) {
                // Continue suggesting line numbers in auto-continuation mode
                int nextLine = [self findNextAvailableLineNumber:_lastLineNumber];
                if (nextLine > 0 && nextLine < 65536) {
                    _suggestedNextLine = nextLine;

                    // Check if next line already exists - if so, load it for editing
                    if (_programManager->hasLine(nextLine)) {
                        std::string existingContent = _programManager->getLine(nextLine);
                        _currentInput = std::to_string(_suggestedNextLine) + " " + existingContent;
                    } else {
                        // Show empty line number ready for new content
                        _currentInput = std::to_string(_suggestedNextLine) + " ";
                    }
                    _cursorPos = _currentInput.length();
                }
            }
            return false; // Don't clear input if in auto-continuation

        case FasterBASIC::ShellCommandType::LIST:
        case FasterBASIC::ShellCommandType::LIST_RANGE:
        case FasterBASIC::ShellCommandType::LIST_LINE:
        case FasterBASIC::ShellCommandType::LIST_FROM:
        case FasterBASIC::ShellCommandType::LIST_TO:
            [self handleListCommand:cmd];
            return true;

        case FasterBASIC::ShellCommandType::RUN:
        case FasterBASIC::ShellCommandType::RUN_FROM:
            [self handleRunCommand:cmd];
            return true;

        case FasterBASIC::ShellCommandType::NEW:
            _programManager->clear();
            _outputLines.push_back("Program cleared");
            _autoContinueMode = false;
            self.textEditor->setFilename("untitled.bas");  // Clear script name for new program

            // Update status bar
            if (self.displayManager && _interactiveMode) {
                self.displayManager->updateStatus("● BASIC interaction");
            }

            // Sync the cleared program to the editor so it shows empty
            [self syncProgramManagerToEditor];
            LOG_INFO("[NEW] Synced cleared program to editor");
            return true;

        case FasterBASIC::ShellCommandType::EDIT: {
            // Load the line for editing
            if (!_programManager->hasLine(cmd.lineNumber)) {
                _outputLines.push_back("Line " + std::to_string(cmd.lineNumber) + " does not exist");
                return true;
            }

            std::string lineContent = _programManager->getLine(cmd.lineNumber);

            // Put line number and content in input buffer for editing
            _currentInput = std::to_string(cmd.lineNumber) + " " + lineContent;
            _cursorPos = _currentInput.length();

            // Set up for auto-continuation after save
            _lastLineNumber = cmd.lineNumber;
            _autoContinueMode = true;
            _suggestedNextLine = cmd.lineNumber;

            return false; // Don't clear input - we set up the line for editing
        }

        case FasterBASIC::ShellCommandType::AUTO:
        case FasterBASIC::ShellCommandType::AUTO_PARAMS:
            _outputLines.push_back("AUTO command removed - just enter numbered lines");
            return true;

        case FasterBASIC::ShellCommandType::RENUM:
        case FasterBASIC::ShellCommandType::RENUM_PARAMS:
            [self handleRenumCommand:cmd];
            return true;

        case FasterBASIC::ShellCommandType::DEL:
        case FasterBASIC::ShellCommandType::DEL_RANGE:
            [self handleDelCommand:cmd];
            return true;

        case FasterBASIC::ShellCommandType::CLS:
            // Clear output history
            _outputLines.clear();
            // Reset scrollback to home position
            _scrollOffset = 0;
            // Clear current input line
            _currentInput = "";
            // Home the cursor
            _cursorPos = 0;
            // Show fresh prompt
            [self showPrompt];
            return true;

        case FasterBASIC::ShellCommandType::DIR:
            [self handleDirCommand];
            return true;

        case FasterBASIC::ShellCommandType::LOAD:
            [self handleLoadCommand:cmd];
            return true;

        case FasterBASIC::ShellCommandType::SAVE:
            [self handleSaveCommand:cmd];
            return true;

        case FasterBASIC::ShellCommandType::FORMAT:
            [self handleFormatCommand];
            return true;

        case FasterBASIC::ShellCommandType::HELP:
            LOG_INFO("[Interactive] HELP command matched in executeInteractiveLine");
            LOG_INFOF("[Interactive] Output lines BEFORE handleHelpCommand: %zu", _outputLines.size());
            [self handleHelpCommand:line];
            LOG_INFOF("[Interactive] Output lines AFTER handleHelpCommand: %zu", _outputLines.size());
            return true;

        case FasterBASIC::ShellCommandType::IMMEDIATE:
            // Immediate mode not supported in compiled BASIC
            // User should enter numbered lines instead
            _outputLines.push_back("ERROR: Cannot execute BASIC statements directly");
            _outputLines.push_back("Use line numbers: 10 PRINT \"Hello\"");
            return true;

        case FasterBASIC::ShellCommandType::EMPTY:
            // Just show new prompt
            return true;

        case FasterBASIC::ShellCommandType::INVALID:
            // Check for cart commands (not yet in CommandParser)
            {
                std::string upperLine = line;
                std::transform(upperLine.begin(), upperLine.end(), upperLine.begin(), ::toupper);



                if (upperLine == "CART INFO") {
                    [self handleCartInfoCommand];
                    return true;
                }
                else if (upperLine == "SAVECART") {
                    [self handleSaveCartCommand];
                    return true;
                }
                else if (upperLine == "CLOSECART") {
                    [self handleCloseCartCommand];
                    return true;
                }
                else if (upperLine.length() >= 8 && upperLine.substr(0, 8) == "USECART ") {
                    [self handleUseCartCommand:line];
                    return true;
                }
                else if (upperLine.length() >= 9 && upperLine.substr(0, 9) == "LOADCART ") {
                    [self handleUseCartCommand:line];
                    return true;
                }
                else if (upperLine.length() >= 8 && upperLine.substr(0, 8) == "RUNCART ") {
                    [self handleRunCartCommand:line];
                    return true;
                }
                else if (upperLine.length() >= 11 && upperLine.substr(0, 11) == "CREATECART ") {
                    [self handleCreateCartCommand:line];
                    return true;
                }
                else if (upperLine.length() >= 9 && upperLine.substr(0, 9) == "SET INFO ") {
                    [self handleSetInfoCommand:line];
                    return true;
                }
                else if (upperLine.length() >= 9 && upperLine.substr(0, 9) == "LIST CART") {
                    [self handleListCartCommand:line];
                    return true;
                }
                else if (upperLine.length() >= 5 && upperLine.substr(0, 5) == "COPY ") {
                    LOG_INFOF("[Interactive] COPY command detected: %s", line.c_str());
                    [self handleCopyAssetCommand:line];
                    return true;
                }
                else if (upperLine.length() >= 5 && upperLine.substr(0, 5) == "PLAY ") {
                    [self handlePlayCommand:line];
                    return true;
                }
                else if (upperLine.length() >= 7 && upperLine.substr(0, 7) == "DELETE ") {
                    LOG_INFOF("[Interactive] DELETE command detected: %s", line.c_str());
                    [self handleDeleteAssetCommand:line];
                    return true;
                }
                else if (upperLine == "QUIT" || upperLine == "EXIT") {
                    LOG_INFO("[Interactive] QUIT command detected - terminating application");
                    _outputLines.push_back("Goodbye!");
                    [NSApp terminate:nil];
                    return true;
                }
                else if (upperLine == "EDITOR") {
                    LOG_INFO("[Interactive] EDITOR command detected - switching to editor mode");
                    _outputLines.push_back("Switching to editor mode...");
                    [self exitInteractiveMode];
                    [self enterEditorMode];
                    return true;
                }
            }

            // Not a cart command, show error
            LOG_INFO("[Interactive] Command not recognized as cart command");
            if (_commandParser->hasError()) {
                _outputLines.push_back("ERROR: " + _commandParser->getLastError());
            } else {
                _outputLines.push_back("Invalid command: " + line);
            }
            return true;

        default:
            _outputLines.push_back("Command not yet implemented: " + line);
            return true;
    }

    // Show prompt for next line
    [self showPrompt];
    return true;
}

- (void)showPrompt {
    // Never called in AUTO mode - line numbers go directly in input buffer
    _outputLines.push_back("Ready.");
}

- (int)findNextAvailableLineNumber:(int)lastLine {
    // Find the next available line number after lastLine
    // Typically just add 10
    int nextLine = lastLine + 10;

    // Check if that line exists, if so, find the gap
    auto lines = _programManager->getAllLines();
    bool exists = false;
    for (const auto& pair : lines) {
        if (pair.first == nextLine) {
            exists = true;
            break;
        }
    }

    if (!exists) {
        return nextLine;
    }

    // Find first gap after lastLine
    for (const auto& pair : lines) {
        if (pair.first > lastLine) {
            if (pair.first > lastLine + 1) {
                // There's a gap
                return lastLine + 10;
            }
            lastLine = pair.first;
        }
    }

    return lastLine + 10;
}

- (int)findPreviousLineNumber:(int)currentLine {
    // Find the previous line number before currentLine
    auto lines = _programManager->getAllLines();
    int prevLine = 0;

    for (const auto& pair : lines) {
        if (pair.first < currentLine) {
            prevLine = pair.first;
        } else {
            break;
        }
    }

    return prevLine; // Returns 0 if no previous line
}

- (void)handleListCommand:(const FasterBASIC::ParsedCommand&)cmd {
    if (_programManager->isEmpty()) {
        _outputLines.push_back("No program in memory");
        return;
    }

    // Determine the range based on command type
    int startLine = 0;
    int endLine = 999999;

    switch (cmd.type) {
        case FasterBASIC::ShellCommandType::LIST_LINE:
            // LIST 100 - show single line
            startLine = cmd.lineNumber;
            endLine = cmd.lineNumber;
            break;

        case FasterBASIC::ShellCommandType::LIST_RANGE:
            // LIST 100-200 - show range
            startLine = cmd.startLine;
            endLine = cmd.endLine;
            break;

        case FasterBASIC::ShellCommandType::LIST_FROM:
            // LIST 100- - show from line 100 to end
            startLine = cmd.startLine;
            endLine = 999999;
            break;

        case FasterBASIC::ShellCommandType::LIST_TO:
            // LIST -200 - show from start to line 200
            startLine = 0;
            endLine = cmd.endLine;
            break;

        case FasterBASIC::ShellCommandType::LIST:
        default:
            // LIST - show all lines
            startLine = 0;
            endLine = 999999;
            break;
    }

    // Get all lines from program manager
    auto allLines = _programManager->getAllLines();
    bool foundAny = false;

    // Display lines that match the range
    for (const auto& pair : allLines) {
        int lineNum = pair.first;
        const std::string& code = pair.second;

        if (lineNum >= startLine && lineNum <= endLine) {
            // Format: "linenum code"
            std::string displayLine = std::to_string(lineNum) + " " + code;
            _outputLines.push_back(displayLine);
            foundAny = true;
        }
    }

    // If listing a specific line and it wasn't found, show message
    if (!foundAny && cmd.type == FasterBASIC::ShellCommandType::LIST_LINE) {
        _outputLines.push_back("Line " + std::to_string(cmd.lineNumber) + " not found");
    }
}

- (void)handleRunCommand:(const FasterBASIC::ParsedCommand&)cmd {
    // Save cart if active in development mode before running
    if (self.cartManager && self.cartManager->isCartActive() &&
        self.cartManager->getCartMode() == SuperTerminal::CartMode::Development) {

        // Get current program from ProgramManager
        if (_programManager) {
            auto lines = _programManager->getAllLines();
            std::string program;
            for (const auto& pair : lines) {
                program += std::to_string(pair.first) + " " + pair.second + "\n";
            }

            if (!program.empty()) {
                auto updateResult = self.cartManager->updateProgramSource(program);
                if (updateResult.success) {
                    auto saveResult = self.cartManager->saveCart();
                    if (!saveResult.success) {
                        _outputLines.push_back("WARNING: Failed to save cart before run");
                    }
                }
            }
        }
    }

    [self runInteractiveProgram];
}
