// =============================================================================
// Script Compilation and Execution
// =============================================================================
// This file contains:
// - BASIC Compilation
// - Script Loading and Execution
// - Script Execution from Editor

// =============================================================================
// BASIC Compilation
// =============================================================================

// Standalone compilation function for command-line use
static std::string compileBASICToLuaStandalone(const std::string& basicSource, std::string& errorOut) {
    try {
        // CRITICAL: Initialize SuperTerminal command registry for standalone compilation
        initializeFBRunner3CommandRegistry();

        // Preprocess DATA statements (extract and parse before main parsing)
        FasterBASIC::DataPreprocessor dataPreprocessor;
        FasterBASIC::DataPreprocessorResult dataResult = dataPreprocessor.process(basicSource);

        // Use cleaned source (without DATA lines) for parsing
        const std::string& sourceForParsing = dataResult.cleanedSource;

        // Lexical analysis
        Lexer lexer;
        if (!lexer.tokenize(sourceForParsing)) {
            std::stringstream ss;
            ss << "Lexer errors:\n";
            for (const auto& err : lexer.getErrors()) {
                ss << "  Line " << err.location.line << ": " << err.message << "\n";
            }
            errorOut = ss.str();
            return "";
        }

        const auto& tokens = lexer.getTokens();

        // Parse AST
        Parser parser;
        auto program = parser.parse(tokens);

        if (!program) {
            std::stringstream ss;
            ss << "=== PARSER ERROR DETECTED ===\n";
            for (const auto& err : parser.getErrors()) {
                ss << "Error at line " << err.location.line << ":" << err.location.column
                   << ": " << err.what() << "\n";
            }
            ss << "Fix the error and try again.\n";
            ss << "=============================";
            errorOut = ss.str();
            return "";
        }

        // Get compiler options from parser (collected from OPTION statements)
        const CompilerOptions& options = parser.getOptions();

        // Semantic analysis
        SemanticAnalyzer semantic;

        // Inject basic runtime constants (without GUI dependencies)
        semantic.injectRuntimeConstant("WINDOW_WIDTH", static_cast<int64_t>(1920));
        semantic.injectRuntimeConstant("WINDOW_HEIGHT", static_cast<int64_t>(1080));
        semantic.injectRuntimeConstant("TEXT_COLS", static_cast<int64_t>(120));
        semantic.injectRuntimeConstant("TEXT_ROWS", static_cast<int64_t>(33));
        semantic.injectRuntimeConstant("GRAPHICS_WIDTH", static_cast<int64_t>(1920));
        semantic.injectRuntimeConstant("GRAPHICS_HEIGHT", static_cast<int64_t>(1080));
        semantic.injectRuntimeConstant("SIXEL_WIDTH", static_cast<int64_t>(1920));
        semantic.injectRuntimeConstant("SIXEL_HEIGHT", static_cast<int64_t>(1080));

        // Register DATA labels so RESTORE can find them
        semantic.registerDataLabels(dataResult.labelDefinitions);

        semantic.analyze(*program, options);

        if (semantic.hasErrors()) {
            std::stringstream ss;
            ss << "Semantic errors:\n";
            for (const auto& err : semantic.getErrors()) {
                ss << "  " << err.toString() << "\n";
            }
            errorOut = ss.str();
            return "";
        }

        // Build CFG
        CFGBuilder cfgBuilder;
        auto cfg = cfgBuilder.build(*program, semantic.getSymbolTable());

        // Generate IR
        IRGenerator irGen;
        auto irCode = irGen.generate(*cfg, semantic.getSymbolTable());

        // Set constants manager pointer for code generation (enables constant inlining)
        irCode->constantsManager = &semantic.getConstantsManager();

        // Optimize IR (enable both AST and peephole optimizers)
        ASTOptimizer astOpt;
        astOpt.optimize(*program, semantic.getSymbolTable());

        PeepholeOptimizer peepholeOpt;
        peepholeOpt.optimize(*irCode);

        // Generate Lua code
        LuaCodeGenConfig config;
        config.emitComments = false;  // Cleaner output

        LuaCodeGenerator luaGen(config);
        std::string luaCode = luaGen.generate(*irCode);

        // Copy constants from semantic analyzer to Lua runtime
        set_constants_manager(&semantic.getConstantsManager());

        return luaCode;

    } catch (const std::exception& e) {
        errorOut = std::string("Compilation exception: ") + e.what();
        return "";
    } catch (...) {
        errorOut = "Unknown compilation error occurred";
        return "";
    }
}

- (std::string)compileBASICToLua:(const std::string&)basicSource error:(NSString**)error {
    try {
        // CRITICAL: Initialize SuperTerminal command registry for GUI compilation
        initializeFBRunner3CommandRegistry();

        // Preprocess DATA statements (extract and parse before main parsing)
        FasterBASIC::DataPreprocessor dataPreprocessor;
        dataResult = dataPreprocessor.process(basicSource);

        // Use cleaned source (without DATA lines) for parsing
        const std::string& sourceForParsing = dataResult.cleanedSource;

        // Lexical analysis
        Lexer lexer;
        if (!lexer.tokenize(sourceForParsing)) {
            if (error) {
                std::stringstream ss;
                ss << "Lexer errors:\n";
                for (const auto& err : lexer.getErrors()) {
                    ss << "  Line " << err.location.line << ": " << err.message << "\n";
                }
                std::string errorMsg = ss.str();
                *error = [NSString stringWithUTF8String:errorMsg.c_str()];
                // Also write to stderr so it's not lost if GUI disappears
                std::cerr << errorMsg << std::endl;
            }
            return "";
        }

        const auto& tokens = lexer.getTokens();

        // Parse AST
        Parser parser;
        auto program = parser.parse(tokens);

        if (!program) {
            if (error) {
                std::stringstream ss;
                ss << "=== PARSER ERROR DETECTED ===\n";
                for (const auto& err : parser.getErrors()) {
                    ss << "Error at line " << err.location.line << ":" << err.location.column
                       << ": " << err.what() << "\n";
                }
                ss << "Fix the error and try again.\n";
                ss << "=============================";
                std::string errorMsg = ss.str();
                *error = [NSString stringWithUTF8String:errorMsg.c_str()];
                // Also write to stderr so it's not lost if GUI disappears
                std::cerr << errorMsg << std::endl;
            }
            return "";
        }

        // Get compiler options from parser (collected from OPTION statements)
        const CompilerOptions& options = parser.getOptions();

        // Semantic analysis
        SemanticAnalyzer semantic;

        // Inject runtime constants from the environment
        // Window and display dimensions
        uint32_t windowWidth, windowHeight;
        self.displayManager->getWindowSize(windowWidth, windowHeight);
        semantic.injectRuntimeConstant("WINDOW_WIDTH", static_cast<int64_t>(windowWidth));
        semantic.injectRuntimeConstant("WINDOW_HEIGHT", static_cast<int64_t>(windowHeight));

        // Text grid dimensions (query actual grid size)
        int textCols = self.textGrid ? self.textGrid->getWidth() : 80;
        int textRows = self.textGrid ? self.textGrid->getHeight() : 25;
        semantic.injectRuntimeConstant("TEXT_COLS", static_cast<int64_t>(textCols));
        semantic.injectRuntimeConstant("TEXT_ROWS", static_cast<int64_t>(textRows));

        // Graphics dimensions (same as window for now)
        semantic.injectRuntimeConstant("GRAPHICS_WIDTH", static_cast<int64_t>(windowWidth));
        semantic.injectRuntimeConstant("GRAPHICS_HEIGHT", static_cast<int64_t>(windowHeight));

        // Sixel dimensions (for compatibility)
        semantic.injectRuntimeConstant("SIXEL_WIDTH", static_cast<int64_t>(windowWidth));
        semantic.injectRuntimeConstant("SIXEL_HEIGHT", static_cast<int64_t>(windowHeight));

        // Keyboard constants (common key codes)
        semantic.injectRuntimeConstant("KEY_UP", static_cast<int64_t>(126));      // Up arrow
        semantic.injectRuntimeConstant("KEY_DOWN", static_cast<int64_t>(125));    // Down arrow
        semantic.injectRuntimeConstant("KEY_LEFT", static_cast<int64_t>(123));    // Left arrow
        semantic.injectRuntimeConstant("KEY_RIGHT", static_cast<int64_t>(124));   // Right arrow
        semantic.injectRuntimeConstant("KEY_RETURN", static_cast<int64_t>(36));   // Return/Enter
        semantic.injectRuntimeConstant("KEY_ESCAPE", static_cast<int64_t>(53));   // Escape
        semantic.injectRuntimeConstant("KEY_SPACE", static_cast<int64_t>(49));    // Space
        semantic.injectRuntimeConstant("KEY_DELETE", static_cast<int64_t>(51));   // Delete
        semantic.injectRuntimeConstant("KEY_TAB", static_cast<int64_t>(48));      // Tab

        // Register DATA labels so RESTORE can find them
        semantic.registerDataLabels(dataResult.labelDefinitions);

        semantic.analyze(*program, options);

        if (semantic.hasErrors()) {
            if (error) {
                std::stringstream ss;
                ss << "Semantic errors:\n";
                for (const auto& err : semantic.getErrors()) {
                    ss << "  " << err.toString() << "\n";
                }
                std::string errorMsg = ss.str();
                *error = [NSString stringWithUTF8String:errorMsg.c_str()];
                // Also write to stderr so it's not lost if GUI disappears
                std::cerr << errorMsg << std::endl;
            }
            return "";
        }

        // Build CFG
        CFGBuilder cfgBuilder;
        auto cfg = cfgBuilder.build(*program, semantic.getSymbolTable());

        // Generate IR
        IRGenerator irGen;
        auto irCode = irGen.generate(*cfg, semantic.getSymbolTable());

        // Set constants manager pointer for code generation (enables constant inlining)
        irCode->constantsManager = &semantic.getConstantsManager();

        // Optimize IR (enable both AST and peephole optimizers)
        ASTOptimizer astOpt;
        astOpt.optimize(*program, semantic.getSymbolTable());

        PeepholeOptimizer peepholeOpt;
        peepholeOpt.optimize(*irCode);

        // Generate Lua code
        LuaCodeGenConfig config;
        config.emitComments = false;  // Cleaner output

        LuaCodeGenerator luaGen(config);
        std::string luaCode = luaGen.generate(*irCode);

        // Initialize DataManager with DATA values from preprocessor
        // This must be done BEFORE executing the Lua code
        FBTBindings::clearDataManager();  // Clear any previous DATA
        FBTBindings::clearFileManager();  // Close any open files from previous run

        // Convert typed DataValues to strings for DataManager initialization
        std::vector<std::string> dataStrings;
        for (const auto& value : dataResult.values) {
            if (std::holds_alternative<int>(value)) {
                dataStrings.push_back(std::to_string(std::get<int>(value)));
            } else if (std::holds_alternative<double>(value)) {
                dataStrings.push_back(std::to_string(std::get<double>(value)));
            } else {
                dataStrings.push_back(std::get<std::string>(value));
            }
        }

        FBTBindings::initializeDataManager(dataStrings);

        // Add line number restore points from preprocessor
        for (const auto& [lineNum, index] : dataResult.lineRestorePoints) {
            FBTBindings::addDataRestorePoint(lineNum, index);
        }

        // Add label restore points from preprocessor
        for (const auto& [labelName, index] : dataResult.labelRestorePoints) {
            FBTBindings::addDataRestorePointByLabel(labelName, index);
        }

        LOG_INFOF("DataManager initialized: %zu values, %zu line points, %zu label points",
                  dataResult.values.size(),
                  dataResult.lineRestorePoints.size(),
                  dataResult.labelRestorePoints.size());

        // Copy constants from semantic analyzer to Lua runtime
        // This ensures constants_get() calls work even if inlining didn't happen
        set_constants_manager(&semantic.getConstantsManager());

        return luaCode;

    } catch (const std::exception& e) {
        if (error) {
            *error = [NSString stringWithFormat:@"Compilation error: %s", e.what()];
        }
        return "";
    }
}


// =============================================================================
// Script Loading and Execution
// =============================================================================

- (BOOL)loadAndExecuteScript {
    // If we have editor mode and no file, don't auto-execute
    if (self.textEditor && (self.scriptPath.empty() || self.scriptPath == "untitled.bas")) {
        LOG_INFO("Editor mode - waiting for user to run script");
        return YES;
    }

    NSString* scriptPath = [NSString stringWithUTF8String:self.scriptPath.c_str()];
    LOG_INFOF("Loading BASIC script: %@", scriptPath);

    // Check if file exists
    NSFileManager* fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:scriptPath]) {
        LOG_ERRORF("Script file not found: %@", scriptPath);
        return NO;
    }

    // Read the BASIC source file
    NSError* readError = nil;
    NSString* basicSource = [NSString stringWithContentsOfFile:scriptPath
                                                       encoding:NSUTF8StringEncoding
                                                          error:&readError];
    if (!basicSource) {
        LOG_ERRORF("Failed to read script: %@", readError);
        return NO;
    }

    // Load script into editor if available
    if (self.textEditor) {
        std::string sourceContent = [basicSource UTF8String];
        self.textEditor->loadText(sourceContent);

        // Extract filename from path
        NSString* filename = [scriptPath lastPathComponent];
        self.textEditor->setFilename([filename UTF8String]);
        self.textEditor->setFilename([filename UTF8String]);
        self.textEditor->markClean();

        LOG_INFOF("Script loaded into editor: %s", [filename UTF8String]);

        // Set up automatic line numbering for BASIC
        [self setupLineNumbering];
    }

    // Compile BASIC to Lua
    NSString* compileError = nil;
    std::string luaCode = [self compileBASICToLua:std::string([basicSource UTF8String])
                                            error:&compileError];

    if (luaCode.empty()) {
        LOG_ERRORF("Compilation failed: %@", compileError);
        [self showError:[NSString stringWithFormat:@"BASIC compilation failed:\n%@", compileError]];
        return NO;
    }

    LOG_INFOF("BASIC script compiled successfully (%zu bytes of Lua)", luaCode.length());

    // Constants are already inlined in the generated Lua code - no runtime setup needed

    // Load and execute the compiled Lua code
    if (luaL_loadstring(_luaState, luaCode.c_str()) != LUA_OK) {
        const char* error = lua_tostring(_luaState, -1);
        LOG_ERRORF("Lua compile error: %s", error);
        [self showError:[NSString stringWithFormat:@"Lua compile error:\n%s", error]];
        lua_pop(_luaState, 1);
        return NO;
    }

    LOG_INFO("Starting BASIC script execution...");

    // Execute the loaded script
    if (lua_pcall(_luaState, 0, 0, 0) != LUA_OK) {
        const char* error = lua_tostring(_luaState, -1);
        LOG_ERRORF("Lua runtime error: %s", error);
        [self showError:[NSString stringWithFormat:@"Runtime error:\n%s", error]];
        lua_pop(_luaState, 1);
        return NO;
    }

    LOG_INFO("BASIC script execution completed");
    return YES;
}


// =============================================================================
// Script Execution from Editor
// =============================================================================

- (void)runScript {
    if (!self.textEditor) {
        [super runScript];
        return;
    }

    // Create program manager and command parser if not already created
    if (!_programManager) {
        _programManager = std::make_unique<FasterBASIC::ProgramManagerV2>();
    }
    if (!_commandParser) {
        _commandParser = std::make_unique<FasterBASIC::CommandParser>();
    }

    // Sync editor changes to program manager if in editor mode
    if (self.editorMode) {
        LOG_INFO("Syncing editor changes before running...");
        [self syncEditorToProgramManager];

        // Make sure we return to editor mode after run (not interactive)
        // This ensures Cmd+. returns to editor when script was started from editor
        _returnToInteractiveAfterRun = false;
        LOG_INFO("Running from editor mode, set _returnToInteractiveAfterRun = false");
    }

    // Get script content from editor
    std::string scriptContent = self.textEditor->getText();
    if (scriptContent.empty()) {
        LOG_INFO("No script to run");
        return;
    }

    // Stop any currently running script and wait for it to finish
    if (self.scriptRunning && _scriptThreadRunning) {
        // Prevent multiple concurrent stop attempts
        if (_isWaitingForStop) {
            LOG_INFO("Already waiting for previous script to stop, ignoring re-run request");
            return;
        }

        LOG_INFO("Stopping previous script...");
        LOG_INFO("runScript: Setting _shouldStopScript = true");
        _shouldStopScript = true;
        _isWaitingForStop = true;

        // Capture script content before async to avoid issues with editor
        std::string capturedContent = scriptContent;

        // Wait for the thread to finish on a background queue
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            // Lock to safely access _scriptThread
            std::unique_lock<std::mutex> lock(self->_scriptThreadMutex);

            // Wait for thread to finish
            if (self->_scriptThread.joinable()) {
                LOG_INFO("Waiting for previous script thread to finish...");
                self->_scriptThread.join();
                LOG_INFO("Previous script thread finished");
            }

            lock.unlock();

            // Reset the waiting flag
            self->_isWaitingForStop = false;

            // Now start the new script on the main queue
            dispatch_async(dispatch_get_main_queue(), ^{
                [self startScriptExecution:capturedContent];
            });
        });
        return;
    }

    [self startScriptExecution:scriptContent];
}

- (void)startScriptExecution:(const std::string&)scriptContent {
    // If a script is already running, wait for it to finish first
    if (_scriptThreadRunning) {
        LOG_INFO("Script already running, stopping previous script...");
        [self stopScript];

        // Wait for the previous script thread to finish (with timeout)
        int waitCount = 0;
        while (_scriptThreadRunning && waitCount < 50) {  // Wait up to 5 seconds
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            waitCount++;
        }

        if (_scriptThreadRunning) {
            LOG_ERROR("Previous script did not stop in time");
            return;
        }
    }

    // Join the previous thread if it exists
    {
        std::lock_guard<std::mutex> lock(_scriptThreadMutex);
        if (_scriptThread.joinable()) {
            _scriptThread.join();
        }
    }

    _currentScriptContent = scriptContent;
    self.scriptRunning = YES;
    _shouldStopScript = false;
    _scriptThreadRunning = true;

    // Save current script if it has a name (no prompts, just save)
    std::string currentFilename = self.textEditor->getFilename();
    if (!currentFilename.empty() && currentFilename.find("untitled") != 0) {
        // Save directly without any dialogs
        if (self.scriptDatabase && self.textEditor) {
            std::string content = self.textEditor->getText();
            SuperTerminal::ScriptLanguage lang = SuperTerminal::ScriptLanguage::BASIC;

            if (self.scriptDatabase->saveScript(currentFilename, lang, content, "")) {
                self.textEditor->markClean();
                LOG_INFOF("✓ Script auto-saved before run: %s", currentFilename.c_str());
            } else {
                LOG_WARNING("Failed to auto-save script before run");
            }
        }
    }

    // Switch to runtime mode (this will clear screen if needed)
    // Note: Skip if already in runtime mode (e.g., called from runInteractiveProgram)
    if (self.editorMode || _interactiveMode) {
        [self enterRuntimeMode];
    } else {
        LOG_INFO("Already in runtime mode, skipping enterRuntimeMode");
    }

    // Reset the Lua state for a fresh run
    [self resetLuaState];

    // Start script execution in background thread
    // Capture the content by value to avoid any threading issues
    std::string contentToExecute = _currentScriptContent;

    // Lock to safely create new thread
    {
        std::lock_guard<std::mutex> lock(_scriptThreadMutex);
        _scriptThread = std::thread([self, contentToExecute]() {
            [self executeScriptContent:contentToExecute];
            self->_scriptThreadRunning = false;
        });
    }

    LOG_INFO("Script execution started (no prompts)");
}

- (void)resetLuaState {
    std::lock_guard<std::mutex> lock(_luaStateMutex);

    // Close existing Lua state if it exists
    if (_luaState) {
        LOG_INFO("Closing previous Lua state");
        lua_close(_luaState);
        _luaState = nullptr;
    }

    // Create new Lua state
    _luaState = luaL_newstate();
    if (!_luaState) {
        LOG_ERROR("Failed to create Lua state");
        return;
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
    lua_pushcfunction(_luaState, [](lua_State* L) -> int {
        FBRunner3App* app = (FBRunner3App*)g_runnerInstance;
        if (app) {
            // Check if we should stop the script
            if (app->_shouldStopScript) {
                LOG_INFO("wait_frame: _shouldStopScript is TRUE, stopping script");
                luaL_error(L, "Script stopped by user");
                return 0;
            }
            [app waitForNextFrame];
        }
        return 0;
    });
    lua_setglobal(_luaState, "wait_frame");

    // Override wait_frames to check for script termination
    lua_pushcfunction(_luaState, [](lua_State* L) -> int {
        FBRunner3App* app = (FBRunner3App*)g_runnerInstance;
        if (app) {
            // Check if we should stop the script before waiting
            if (app->_shouldStopScript) {
                LOG_INFO("wait_frames: _shouldStopScript is TRUE, stopping script");
                luaL_error(L, "Script stopped by user");
                return 0;
            }
        }

        // Call the original wait_frames implementation
        int count = luaL_checkinteger(L, 1);
        st_wait_frames(count);

        // Check again after waiting in case stop was signaled during wait
        if (app && app->_shouldStopScript) {
            LOG_INFO("wait_frames: _shouldStopScript is TRUE after wait, stopping script");
            luaL_error(L, "Script stopped by user");
            return 0;
        }

        return 0;
    });
    lua_setglobal(_luaState, "wait_frames");

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
    lua_pushcfunction(_luaState, [](lua_State* L) -> int {
        int exitcode = luaL_optinteger(L, 1, 0);
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
        if (exitcode == 0) {
            luaL_error(L, "Script ended normally (os.exit called)");
        } else {
            luaL_error(L, "Script ended with error code %d (os.exit called)", exitcode);
        }
        return 0;
    });
    lua_setfield(_luaState, -2, "exit");
    lua_pop(_luaState, 1);  // Pop the os table

    LOG_INFO("Lua state reset complete");
}

- (void)executeScriptContent:(const std::string&)content {
    @autoreleasepool {
        // Compile BASIC to Lua
        NSString* compileError = nil;
        std::string luaCode = [self compileBASICToLua:content error:&compileError];

        if (luaCode.empty()) {
            LOG_ERRORF("Compilation failed: %@", compileError);
            dispatch_async(dispatch_get_main_queue(), ^{
                // Show error dialog
                NSAlert* alert = [[NSAlert alloc] init];
                [alert setMessageText:@"Parser Error"];
                [alert setInformativeText:[NSString stringWithFormat:@"%@", compileError]];
                [alert setAlertStyle:NSAlertStyleCritical];
                [alert addButtonWithTitle:@"OK"];
                [alert runModal];

                // After dialog is dismissed, return to appropriate mode
                if (_returnToInteractiveAfterRun) {
                    [self enterInteractiveMode];
                    _returnToInteractiveAfterRun = false;
                } else {
                    [self enterEditorMode];
                }
                [self.window makeKeyAndOrderFront:nil];
            });
            self.scriptRunning = NO;
            _shouldStopScript = false;
            return;
        }

        LOG_INFOF("BASIC compiled to Lua (%zu bytes)", luaCode.length());

        // Check if we should stop before executing
        if (_shouldStopScript) {
            LOG_INFO("Script stopped before execution");
            self.scriptRunning = NO;
            _shouldStopScript = false;
            return;
        }



        // Load the compiled Lua code
        if (luaL_loadstring(_luaState, luaCode.c_str()) != LUA_OK) {
            const char* error = lua_tostring(_luaState, -1);
            LOG_ERRORF("Lua compile error: %s", error);
            // Also write to stderr so it's not lost if GUI disappears
            std::cerr << "[FBRunner3] ERROR: Lua compile error: " << error << std::endl;
            dispatch_async(dispatch_get_main_queue(), ^{
                NSAlert* alert = [[NSAlert alloc] init];
                [alert setMessageText:@"Lua Compile Error"];
                [alert setInformativeText:[NSString stringWithFormat:@"%s", error]];
                [alert setAlertStyle:NSAlertStyleCritical];
                [alert addButtonWithTitle:@"OK"];
                [alert runModal];

                if (_returnToInteractiveAfterRun) {
                    [self enterInteractiveMode];
                    _returnToInteractiveAfterRun = false;
                } else {
                    [self enterEditorMode];
                }
                [self.window makeKeyAndOrderFront:nil];
            });
            lua_pop(_luaState, 1);
            self.scriptRunning = NO;
            _shouldStopScript = false;
            return;
        }

        // Install interrupt hook for immediate script stopping
        [self installLuaInterruptHook];

        // Execute the compiled script
        if (lua_pcall(_luaState, 0, 0, 0) != LUA_OK) {
            const char* error = lua_tostring(_luaState, -1);
            LOG_ERRORF("Lua runtime error: %s", error);

            // Remove interrupt hook on error
            [self removeLuaInterruptHook];

            // Check if this is a user-initiated stop or normal END (don't show error dialog)
            std::string errorStr(error);
            bool isUserStop = (errorStr.find("Script stopped by user") != std::string::npos) ||
                             (errorStr.find("Script interrupted") != std::string::npos) ||
                             (errorStr.find("Script ended normally") != std::string::npos) ||
                             (errorStr.find("os.exit called") != std::string::npos);

            if (!isUserStop) {
                // Also write to stderr so it's not lost if GUI disappears
                std::cerr << "[FBRunner3] ERROR: Lua runtime error: " << error << std::endl;
                dispatch_async(dispatch_get_main_queue(), ^{
                    NSAlert* alert = [[NSAlert alloc] init];
                    [alert setMessageText:@"Runtime Error"];
                    [alert setInformativeText:[NSString stringWithFormat:@"%s", error]];
                    [alert setAlertStyle:NSAlertStyleCritical];
                    [alert addButtonWithTitle:@"OK"];
                    [alert runModal];

                    if (_returnToInteractiveAfterRun) {
                        [self enterInteractiveMode];
                        _returnToInteractiveAfterRun = false;
                    } else {
                        [self enterEditorMode];
                    }
                    [self.window makeKeyAndOrderFront:nil];
                });
            } else {
                // User stopped the script - just return to appropriate mode
                LOG_INFO("Script stopped by user");
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (_returnToInteractiveAfterRun) {
                        [self enterInteractiveMode];
                        _returnToInteractiveAfterRun = false;
                    } else {
                        [self enterEditorMode];
                    }
                });
            }

            lua_pop(_luaState, 1);
            self.scriptRunning = NO;
            _shouldStopScript = false;
            return;
        }

        LOG_INFO("Script execution completed");

        // Remove interrupt hook on successful completion
        [self removeLuaInterruptHook];

        LOG_INFOF("_returnToInteractiveAfterRun flag is: %d", _returnToInteractiveAfterRun);

        // Clear DATA after script completes
        FBTBindings::clearDataManager();
        FBTBindings::clearFileManager();  // Close all files

        self.scriptRunning = NO;
        _shouldStopScript = false;
        _scriptThreadRunning = false;

        // Check if we should return to interactive mode after script completes
        if (_returnToInteractiveAfterRun) {
            LOG_INFO("FLAG IS TRUE - Dispatching enterInteractiveMode to main queue...");
            dispatch_async(dispatch_get_main_queue(), ^{
                LOG_INFO("Script completed, NOW CALLING enterInteractiveMode...");
                [self enterInteractiveMode];
                _returnToInteractiveAfterRun = false;
            });
        } else {
            LOG_INFO("FLAG IS FALSE - NOT returning to interactive mode");
        }
    }
}

// =============================================================================
// Lua Debug Hook for Immediate Script Interruption
// =============================================================================

// Static Lua hook function for instruction-level interruption
static void luaInterruptHook(lua_State *L, lua_Debug *ar) {
    FBRunner3App* app = (FBRunner3App*)g_runnerInstance;
    if (app && app->_shouldStopScript) {
        // Remove the hook to prevent repeated calls
        lua_sethook(L, nullptr, 0, 0);

        LOG_DEBUG("Lua interrupt hook triggered - stopping script immediately");
        luaL_error(L, "Script interrupted (Ctrl+C or Stop button)");
    }
}

- (void)installLuaInterruptHook {
    std::lock_guard<std::mutex> lock(_luaStateMutex);

    if (_luaState) {
        // Install hook to check every 100 instructions for tight loops
        // This provides ~100ms response time for infinite loops
        lua_sethook(_luaState, luaInterruptHook, LUA_MASKCOUNT, 100);
        LOG_DEBUG("Lua interrupt hook installed (main state)");
    }

    // Also install in interactive Lua state if active
    if (_interactiveLuaState) {
        lua_sethook(_interactiveLuaState, luaInterruptHook, LUA_MASKCOUNT, 100);
        LOG_DEBUG("Lua interrupt hook installed (interactive state)");
    }
}

- (void)removeLuaInterruptHook {
    std::lock_guard<std::mutex> lock(_luaStateMutex);

    if (_luaState) {
        lua_sethook(_luaState, nullptr, 0, 0);
        LOG_DEBUG("Lua interrupt hook removed (main state)");
    }

    if (_interactiveLuaState) {
        lua_sethook(_interactiveLuaState, nullptr, 0, 0);
        LOG_DEBUG("Lua interrupt hook removed (interactive state)");
    }
}
