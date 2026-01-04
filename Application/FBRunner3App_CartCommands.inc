// =============================================================================
// Cart REPL Command Handlers
// =============================================================================

- (void)handleCartInfoCommand {
    if (!self.cartManager || !self.cartManager->isCartActive()) {
        _outputLines.push_back("No cart is currently active");
        return;
    }

    auto info = self.cartManager->getCartInfo();

    _outputLines.push_back("=== Cart Information ===");
    _outputLines.push_back("Title: " + info.metadata.title);
    _outputLines.push_back("Author: " + info.metadata.author);
    _outputLines.push_back("Version: " + info.metadata.version);
    if (!info.metadata.description.empty()) {
        _outputLines.push_back("Description: " + info.metadata.description);
    }
    _outputLines.push_back("");
    _outputLines.push_back("Sprites: " + std::to_string(info.spriteCount));
    _outputLines.push_back("Tilesets: " + std::to_string(info.tilesetCount));
    _outputLines.push_back("Sounds: " + std::to_string(info.soundCount));
    _outputLines.push_back("Music: " + std::to_string(info.musicCount));
    _outputLines.push_back("Data Files: " + std::to_string(info.dataFileCount));
    _outputLines.push_back("");
    _outputLines.push_back("Mode: " + info.mode);

    char sizeStr[64];
    snprintf(sizeStr, sizeof(sizeStr), "Size: %.2f MB", info.totalSize / (1024.0 * 1024.0));
    _outputLines.push_back(std::string(sizeStr));
}

- (void)handleSaveCartCommand {
    if (!self.cartManager || !self.cartManager->isCartActive()) {
        _outputLines.push_back("No cart is currently active");
        return;
    }

    // Update program source from ProgramManager first
    if (_programManager) {
        auto lines = _programManager->getAllLines();
        std::string program;
        for (const auto& pair : lines) {
            program += std::to_string(pair.first) + " " + pair.second + "\n";
        }

        if (!program.empty()) {
            auto updateResult = self.cartManager->updateProgramSource(program);
            if (!updateResult.success) {
                _outputLines.push_back("ERROR: " + updateResult.message);
                return;
            }
        }
    }

    auto result = self.cartManager->saveCart();
    if (result.success) {
        _outputLines.push_back("Cart saved successfully");
    } else {
        _outputLines.push_back("ERROR: " + result.message);
    }
}

- (void)handleCloseCartCommand {
    if (!self.cartManager || !self.cartManager->isCartActive()) {
        _outputLines.push_back("No cart is currently active");
        return;
    }

    // Update program before closing
    if (_programManager) {
        auto lines = _programManager->getAllLines();
        std::string program;
        for (const auto& pair : lines) {
            program += std::to_string(pair.first) + " " + pair.second + "\n";
        }

        if (!program.empty()) {
            self.cartManager->updateProgramSource(program);
        }
    }

    auto result = self.cartManager->closeCart(true);
    if (result.success) {
        _outputLines.push_back("Cart closed");
    } else {
        _outputLines.push_back("ERROR: " + result.message);
    }
}

- (void)handleUseCartCommand:(const std::string&)line {
    // Parse: USECART "path.crt" or LOADCART "path.crt"
    size_t quoteStart = line.find('"');
    if (quoteStart == std::string::npos) {
        _outputLines.push_back("Usage: USECART \"path.crt\"");
        return;
    }

    size_t quoteEnd = line.find('"', quoteStart + 1);
    if (quoteEnd == std::string::npos) {
        _outputLines.push_back("Usage: USECART \"path.crt\"");
        return;
    }

    std::string path = line.substr(quoteStart + 1, quoteEnd - quoteStart - 1);

    auto result = self.cartManager->useCart(path);
    if (!result.success) {
        _outputLines.push_back("ERROR: " + result.message);
        return;
    }

    _outputLines.push_back("Cart opened for editing: " + path);

    // Load program into ProgramManager
    if (_programManager && !result.programSource.empty()) {
        _programManager->clear();

        std::istringstream iss(result.programSource);
        std::string line;
        while (std::getline(iss, line)) {
            // Parse line number and content
            size_t spacePos = line.find(' ');
            if (spacePos != std::string::npos && std::isdigit(line[0])) {
                int lineNum = std::stoi(line.substr(0, spacePos));
                std::string content = line.substr(spacePos + 1);
                _programManager->setLine(lineNum, content);
            }
        }

        _outputLines.push_back("Program loaded into memory");

        // Sync the loaded content to the editor so it's visible
        [self syncProgramManagerToEditor];
        LOG_INFO("[USECART] Synced program manager to editor");
    }
}

- (void)handleRunCartCommand:(const std::string&)line {
    // Parse: RUNCART "path.crt"
    size_t quoteStart = line.find('"');
    if (quoteStart == std::string::npos) {
        _outputLines.push_back("Usage: RUNCART \"path.crt\"");
        return;
    }

    size_t quoteEnd = line.find('"', quoteStart + 1);
    if (quoteEnd == std::string::npos) {
        _outputLines.push_back("Usage: RUNCART \"path.crt\"");
        return;
    }

    std::string path = line.substr(quoteStart + 1, quoteEnd - quoteStart - 1);

    auto result = self.cartManager->runCart(path);
    if (!result.success) {
        _outputLines.push_back("ERROR: " + result.message);
        return;
    }

    _outputLines.push_back("Cart opened for playing: " + path);
    _outputLines.push_back("Running program...");

    // Exit interactive mode and run the program
    _returnToInteractiveAfterRun = true;
    _currentScriptContent = result.programSource;

    // Save interactive state before exiting
    [self saveInteractiveState];

    [self exitInteractiveMode];
    // Execute the cart program
    [self runScript];
}

- (void)handleCreateCartCommand:(const std::string&)line {
    // Parse: CREATECART "path.crt" [, "title", "author", "version"]
    std::vector<std::string> args;
    size_t pos = 0;
    while (true) {
        size_t quoteStart = line.find('"', pos);
        if (quoteStart == std::string::npos) break;

        size_t quoteEnd = line.find('"', quoteStart + 1);
        if (quoteEnd == std::string::npos) break;

        args.push_back(line.substr(quoteStart + 1, quoteEnd - quoteStart - 1));
        pos = quoteEnd + 1;
    }

    if (args.empty()) {
        _outputLines.push_back("Usage: CREATECART \"path.crt\" [, \"title\", \"author\", \"version\"]");
        return;
    }

    std::string path = args[0];
    std::string title = args.size() > 1 ? args[1] : "Untitled";
    std::string author = args.size() > 2 ? args[2] : "";
    std::string version = args.size() > 3 ? args[3] : "1.0.0";

    auto result = self.cartManager->createCart(path, title, author, version, "");
    if (result.success) {
        _outputLines.push_back("Cart created: " + path);
        _outputLines.push_back("Cart opened for editing");
    } else {
        _outputLines.push_back("ERROR: " + result.message);
    }
}

- (void)handleSetInfoCommand:(const std::string&)line {
    if (!self.cartManager || !self.cartManager->isCartActive()) {
        _outputLines.push_back("No cart is currently active");
        return;
    }

    // Parse: SET INFO NAME "value"
    size_t infoPos = line.find("INFO");
    if (infoPos == std::string::npos) {
        _outputLines.push_back("Usage: SET INFO <key> \"value\"");
        return;
    }

    std::string rest = line.substr(infoPos + 4);
    size_t quoteStart = rest.find('"');
    if (quoteStart == std::string::npos) {
        _outputLines.push_back("Usage: SET INFO <key> \"value\"");
        return;
    }

    std::string keyPart = rest.substr(0, quoteStart);
    // Trim whitespace
    keyPart.erase(0, keyPart.find_first_not_of(" \t"));
    keyPart.erase(keyPart.find_last_not_of(" \t") + 1);

    size_t quoteEnd = rest.find('"', quoteStart + 1);
    if (quoteEnd == std::string::npos) {
        _outputLines.push_back("Usage: SET INFO <key> \"value\"");
        return;
    }

    std::string value = rest.substr(quoteStart + 1, quoteEnd - quoteStart - 1);

    auto result = self.cartManager->setMetadata(keyPart, value);
    if (result.success) {
        _outputLines.push_back("Metadata updated: " + keyPart);
    } else {
        _outputLines.push_back("ERROR: " + result.message);
    }
}

- (void)handleListCartCommand:(const std::string&)line {
    if (!self.cartManager || !self.cartManager->isCartActive()) {
        _outputLines.push_back("No cart is currently active");
        return;
    }

    // Parse optional filter: LIST CART [filter]
    std::string filter;
    size_t cartPos = line.find("CART");
    if (cartPos != std::string::npos) {
        std::string rest = line.substr(cartPos + 4);
        rest.erase(0, rest.find_first_not_of(" \t"));
        if (!rest.empty()) {
            filter = rest;
            std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);
        }
    }

    auto assets = self.cartManager->listAssets(filter);

    if (assets.empty()) {
        _outputLines.push_back("No assets found");
    } else {
        _outputLines.push_back("Cart Contents:");
        for (const auto& asset : assets) {
            _outputLines.push_back("  " + asset);
        }
    }
}

- (void)handleRenumCommand:(const FasterBASIC::ParsedCommand&)cmd {
    // Use the parsed values which have defaults of 10 already set by the parser
    int start = cmd.startLine;  // Parser sets default to 10
    int step = cmd.step;        // Parser sets default to 10

    _programManager->renumber(start, step);

    std::string msg = "Program renumbered (start: " + std::to_string(start) +
                      ", step: " + std::to_string(step) + ")";
    _outputLines.push_back(msg);
}

- (void)handleDelCommand:(const FasterBASIC::ParsedCommand&)cmd {
    if (cmd.type == FasterBASIC::ShellCommandType::DEL_RANGE) {
        // Delete range of lines
        int startLine = cmd.startLine;
        int endLine = cmd.endLine;
        int deletedCount = 0;

        // Get all line numbers in range
        auto allLines = _programManager->getAllLines();
        for (const auto& pair : allLines) {
            int lineNum = pair.first;
            if (lineNum >= startLine && lineNum <= endLine) {
                _programManager->deleteLine(lineNum);
                deletedCount++;
            }
        }

        if (deletedCount > 0) {
            std::string msg = "Deleted " + std::to_string(deletedCount) + " line";
            if (deletedCount > 1) msg += "s";
            msg += " (" + std::to_string(startLine) + "-" + std::to_string(endLine) + ")";
            _outputLines.push_back(msg);
        } else {
            _outputLines.push_back("No lines found in range " + std::to_string(startLine) +
                                  "-" + std::to_string(endLine));
        }
    } else {
        // Delete single line
        int lineNum = cmd.lineNumber;
        if (_programManager->hasLine(lineNum)) {
            _programManager->deleteLine(lineNum);
            _outputLines.push_back("Deleted line " + std::to_string(lineNum));
        } else {
            _outputLines.push_back("Line " + std::to_string(lineNum) + " not found");
        }
    }

    // Exit auto-continuation mode if active
    _autoContinueMode = false;
    _suggestedNextLine = 0;
    _lastLineNumber = 0;
}

- (void)handleFormatCommand {
    if (_programManager->isEmpty()) {
        _outputLines.push_back("No program to format");
        return;
    }

    // Get current program
    std::string programText = _programManager->generateProgram();

    LOG_INFOF("[FORMAT] Original program:\n%s", programText.c_str());

    // Use the formatter to format and renumber the program
    FasterBASIC::FormatterOptions options;
    options.start_line = 10;
    options.step = 10;
    options.indent_spaces = 2;
    options.update_references = true;
    options.add_indentation = true;

    FasterBASIC::FormatterResult result = FasterBASIC::formatBasicCode(programText, options);

    if (!result.success || result.formatted_code.empty()) {
        _outputLines.push_back("Failed to format program: " + result.error_message);
        return;
    }

    LOG_INFOF("[FORMAT] Formatted program:\n%s", result.formatted_code.c_str());
    LOG_INFO("[FORMAT] Line mapping:");
    for (const auto& pair : result.line_number_map) {
        LOG_INFOF("  %d -> %d", pair.first, pair.second);
    }

    // Clear current program and reload the formatted version
    _programManager->clear();

    // Parse the formatted program back into the program manager
    std::istringstream iss(result.formatted_code);
    std::string line;
    int lineCount = 0;

    while (std::getline(iss, line)) {
        if (line.empty()) continue;

        // Parse line number and code
        std::istringstream lineStream(line);
        int lineNum;
        if (lineStream >> lineNum) {
            std::string code;
            std::getline(lineStream, code);
            // Remove leading space
            if (!code.empty() && code[0] == ' ') {
                code = code.substr(1);
            }
            _programManager->setLine(lineNum, code);
            lineCount++;
        }
    }

    _outputLines.push_back("Program formatted and renumbered (" + std::to_string(lineCount) + " lines)");
}

- (void)handleDirCommand {
    // Get list of BASIC scripts from database
    auto scripts = self.scriptDatabase->listScripts(SuperTerminal::ScriptLanguage::BASIC, false);

    if (scripts.empty()) {
        _outputLines.push_back("No BASIC scripts in database");
        return;
    }

    _outputLines.push_back("BASIC Scripts:");
    _outputLines.push_back("");

    // List each script with name and size
    for (const auto& script : scripts) {
        std::string line = "  " + script.name;

        // Add size info
        if (script.contentLength > 0) {
            line += " (" + std::to_string(script.contentLength) + " bytes)";
        }

        _outputLines.push_back(line);
    }

    _outputLines.push_back("");
    _outputLines.push_back(std::to_string(scripts.size()) + " file(s)");
}

- (void)handleLoadCommand:(const FasterBASIC::ParsedCommand&)cmd {
    if (!cmd.hasFilename) {
        _outputLines.push_back("Usage: LOAD \"filename.bas\"");
        return;
    }

    std::string filename = cmd.filename;

    // Try to load from script database
    std::string content;
    SuperTerminal::ScriptLanguage lang = SuperTerminal::ScriptLanguage::BASIC;

    if (!self.scriptDatabase->loadScript(filename, lang, content)) {
        _outputLines.push_back("ERROR: Could not load \"" + filename + "\"");
        _outputLines.push_back("File not found in database");
        return;
    }

    // Clear current program
    _programManager->clear();

    // Parse the loaded content into program manager
    std::istringstream stream(content);
    std::string line;
    int linesLoaded = 0;

    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        // Parse each line
        FasterBASIC::ParsedCommand parsedLine = _commandParser->parse(line);
        if (parsedLine.type == FasterBASIC::ShellCommandType::DIRECT_LINE) {
            // This is a numbered line - add it to program manager
            std::string formattedCode = _commandParser->formatBasicKeywords(parsedLine.code);
            _programManager->setLine(parsedLine.lineNumber, formattedCode);
            linesLoaded++;
        }
    }

    _outputLines.push_back("Loaded \"" + filename + "\" (" + std::to_string(linesLoaded) + " lines)");

    // Remember the script name for future saves
    self.textEditor->setFilename(filename);
    LOG_INFOF("[LOAD] Set editor filename to: %s", filename.c_str());

    // Update status bar
    if (self.displayManager && _interactiveMode) {
        std::string statusText = "● BASIC interaction - " + filename;
        LOG_INFOF("[LOAD] Updating status bar to: %s", statusText.c_str());
        self.displayManager->updateStatus(statusText.c_str());
        LOG_INFO("[LOAD] Status bar updated successfully");
    } else {
        LOG_INFOF("[LOAD] NOT updating status: displayManager=%p, _interactiveMode=%d", self.displayManager.get(), _interactiveMode);
    }

    // Sync the loaded content to the editor so it's visible
    [self syncProgramManagerToEditor];
    LOG_INFO("[LOAD] Synced program manager to editor");

    // Exit auto-continuation mode if active
    _autoContinueMode = false;
    _suggestedNextLine = 0;
    _lastLineNumber = 0;
}

- (void)handleSaveCommand:(const FasterBASIC::ParsedCommand&)cmd {
    std::string filename;

    if (!cmd.hasFilename) {
        // No filename provided - use current script name if available
        std::string currentFilename = self.textEditor->getFilename();
        if (currentFilename.empty() || currentFilename.find("untitled") == 0) {
            _outputLines.push_back("Usage: SAVE \"filename.bas\"");
            _outputLines.push_back("(No current filename - please specify one)");
            return;
        }
        filename = currentFilename;
        _outputLines.push_back("Saving to: " + filename);
    } else {
        filename = cmd.filename;
    }

    // Check if program is empty
    if (_programManager->isEmpty()) {
        _outputLines.push_back("ERROR: No program to save");
        return;
    }

    // Generate program text from program manager
    std::string programText = _programManager->generateProgram();

    // Format the program before saving
    FasterBASIC::FormatterResult result = FasterBASIC::formatClassic(programText);
    std::string contentToSave;

    if (result.success) {
        contentToSave = result.formatted_code;
    } else {
        // If formatting fails, save unformatted
        contentToSave = programText;
    }

    // Save to script database
    SuperTerminal::ScriptLanguage lang = SuperTerminal::ScriptLanguage::BASIC;

    if (self.scriptDatabase->saveScript(filename, lang, contentToSave, "")) {
        _outputLines.push_back("Saved \"" + filename + "\" (" + std::to_string(_programManager->getLineCount()) + " lines)");
        // Update current script name
        self.textEditor->setFilename(filename);

        // Update status bar
        if (self.displayManager && _interactiveMode) {
            std::string statusText = "● BASIC interaction - " + filename;
            self.displayManager->updateStatus(statusText.c_str());
        }
    } else {
        _outputLines.push_back("ERROR: Could not save \"" + filename + "\"");
        _outputLines.push_back("Database save failed");
    }
}

- (void)handleHelpCommand:(const std::string&)line {
    using namespace FasterBASIC::ModularCommands;

    // Get the command registry
    auto& registry = getGlobalCommandRegistry();

    // DEBUG: Check registry state
    auto categories = registry.getCategories();
    auto allNames = registry.getAllNames();
    LOG_INFOF("[HELP DEBUG] Registry has %zu categories, %zu total commands", categories.size(), allNames.size());
    if (!categories.empty()) {
        LOG_INFO("[HELP DEBUG] Categories: ");
        for (size_t i = 0; i < std::min(categories.size(), size_t(10)); i++) {
            LOG_INFOF("[HELP DEBUG]   - %s", categories[i].c_str());
        }
    }
    if (!allNames.empty()) {
        LOG_INFO("[HELP DEBUG] First 10 commands: ");
        for (size_t i = 0; i < std::min(allNames.size(), size_t(10)); i++) {
            LOG_INFOF("[HELP DEBUG]   - %s", allNames[i].c_str());
        }
    }

    // Parse the line to extract topic/command name after HELP
    std::string topic;
    size_t helpPos = line.find_first_not_of(" \t");
    if (helpPos != std::string::npos) {
        size_t afterHelp = line.find_first_of(" \t", helpPos);
        if (afterHelp != std::string::npos) {
            size_t topicStart = line.find_first_not_of(" \t", afterHelp);
            if (topicStart != std::string::npos) {
                topic = line.substr(topicStart);
                // Trim trailing whitespace
                size_t topicEnd = topic.find_last_not_of(" \t");
                if (topicEnd != std::string::npos) {
                    topic = topic.substr(0, topicEnd + 1);
                }
            }
        }
    }

    // If topic specified, search for it
    if (!topic.empty()) {
        LOG_INFOF("[HELP DEBUG] Searching for topic: '%s'", topic.c_str());

        std::string topicUpper = topic;
        std::transform(topicUpper.begin(), topicUpper.end(), topicUpper.begin(), ::toupper);

        std::string topicLower = topic;
        std::transform(topicLower.begin(), topicLower.end(), topicLower.begin(), ::tolower);

        LOG_INFOF("[HELP DEBUG] Topic upper: '%s', lower: '%s'", topicUpper.c_str(), topicLower.c_str());

        // Check if it's a category
        bool isCategory = std::find(categories.begin(), categories.end(), topicLower) != categories.end();

        LOG_INFOF("[HELP DEBUG] Is category? %d", isCategory);

        if (isCategory) {
            // Show commands in this category
            [self showHelpForCategory:topicLower registry:registry];
            return;
        }

        // Check if it's a command
        const auto* cmd = registry.getCommandOrFunction(topicUpper);
        LOG_INFOF("[HELP DEBUG] Found command? %d", cmd != nullptr);
        if (cmd) {
            // Show detailed command help
            [self showHelpForCommand:cmd];
            return;
        }

        // Not found - show error
        _outputLines.push_back("");
        _outputLines.push_back("Unknown command or category: " + topic);
        _outputLines.push_back("Type HELP to see all categories");
        _outputLines.push_back("");
        return;
    }

    // No topic - show general help
    _outputLines.push_back("");
    _outputLines.push_back("FasterBASIC Shell Commands:");
    _outputLines.push_back("===========================");
    _outputLines.push_back("");
    _outputLines.push_back("Program Entry:");
    _outputLines.push_back("  10 PRINT \"Hello\"   Add or replace line 10");
    _outputLines.push_back("  10               Delete line 10");
    _outputLines.push_back("");
    _outputLines.push_back("Listing:");
    _outputLines.push_back("  LIST             List entire program");
    _outputLines.push_back("  LIST 10          List line 10");
    _outputLines.push_back("  LIST 10-50       List lines 10 through 50");
    _outputLines.push_back("");
    _outputLines.push_back("Execution:");
    _outputLines.push_back("  RUN              Run program from beginning");
    _outputLines.push_back("");
    _outputLines.push_back("File Operations:");
    _outputLines.push_back("  NEW              Clear program from memory");
    _outputLines.push_back("  LOAD \"file\"       Load program from file");
    _outputLines.push_back("  SAVE \"file\"       Save program to file");
    _outputLines.push_back("  DIR              List .bas files");
    _outputLines.push_back("");
    _outputLines.push_back("Other:");
    _outputLines.push_back("  CLS              Clear screen");
    _outputLines.push_back("  RENUM            Renumber program");
    _outputLines.push_back("  FORMAT           Format and renumber program");
    _outputLines.push_back("  EDIT 100         Edit line 100");
    _outputLines.push_back("  QUIT             Exit shell");
    _outputLines.push_back("  EDITOR           Switch to editor mode");
    _outputLines.push_back("");
    _outputLines.push_back("Cart Commands:");
    _outputLines.push_back("  CREATECART \"file.crt\" - Create new cart");
    _outputLines.push_back("  USECART \"file.crt\"    - Open cart for editing");
    _outputLines.push_back("  RUNCART \"file.crt\"    - Run cart");
    _outputLines.push_back("");

    // Show BASIC command categories
    if (!categories.empty()) {
        _outputLines.push_back("BASIC Command Categories:");
        _outputLines.push_back("========================");
        _outputLines.push_back("Type HELP <category> to see commands in that category:");
        _outputLines.push_back("");

        std::sort(categories.begin(), categories.end());

        std::unordered_map<std::string, std::string> categoryDesc = {
            {"audio", "Music and sound playback"},
            {"cart", "Cart/cartridge system"},
            {"circle", "Circle ID system"},
            {"control", "Control flow commands"},
            {"data", "Data storage commands"},
            {"file", "File I/O operations"},
            {"graphics", "Graphics primitives"},
            {"input", "Keyboard and mouse input"},
            {"line", "Line ID system"},
            {"math", "Mathematical functions"},
            {"particle", "Particle effects"},
            {"rectangle", "Rectangle ID system"},
            {"sprite", "Sprite management"},
            {"string", "String manipulation"},
            {"system", "System utilities"},
            {"text", "Text display and manipulation"},
            {"tilemap", "Tilemap operations"},
            {"voice", "Voice synthesis"}
        };

        for (const auto& cat : categories) {
            std::string catUpper = cat;
            std::transform(catUpper.begin(), catUpper.end(), catUpper.begin(), ::toupper);

            std::string desc = "Commands";
            auto it = categoryDesc.find(cat);
            if (it != categoryDesc.end()) {
                desc = it->second;
            }

            char line[128];
            snprintf(line, sizeof(line), "  %-12s - %s", catUpper.c_str(), desc.c_str());
            _outputLines.push_back(line);
        }
        _outputLines.push_back("");
    }

    _outputLines.push_back("Type HELP <command> for detailed help on a specific command");
    _outputLines.push_back("");
    _outputLines.push_back("Ready.");
}

- (void)showHelpForCategory:(const std::string&)category registry:(FasterBASIC::ModularCommands::CommandRegistry&)registry {
    auto commands = registry.getCommandsByCategory(category);
    auto functions = registry.getFunctionsByCategory(category);

    if (commands.empty() && functions.empty()) {
        _outputLines.push_back("");
        _outputLines.push_back("No commands found in category: " + category);
        _outputLines.push_back("");
        return;
    }

    std::string catUpper = category;
    std::transform(catUpper.begin(), catUpper.end(), catUpper.begin(), ::toupper);

    _outputLines.push_back("");
    _outputLines.push_back(catUpper + " Commands");
    _outputLines.push_back(std::string(catUpper.length() + 9, '='));
    _outputLines.push_back("");

    if (!commands.empty()) {
        _outputLines.push_back("Commands:");
        for (const auto& cmdName : commands) {
            const auto* cmd = registry.getCommand(cmdName);
            if (cmd) {
                std::string sig = "  " + cmd->commandName;
                if (!cmd->parameters.empty()) {
                    sig += " ";
                    for (size_t i = 0; i < cmd->parameters.size(); i++) {
                        if (i > 0) sig += ", ";
                        sig += cmd->parameters[i].name;
                        if (cmd->parameters[i].isOptional) {
                            sig += "?";
                        }
                    }
                }
                _outputLines.push_back(sig);
                _outputLines.push_back("      " + cmd->description);
            }
        }
        _outputLines.push_back("");
    }

    if (!functions.empty()) {
        _outputLines.push_back("Functions:");
        for (const auto& funcName : functions) {
            const auto* func = registry.getFunction(funcName);
            if (func) {
                std::string sig = "  " + func->commandName + "()";
                _outputLines.push_back(sig);
                _outputLines.push_back("      " + func->description);
            }
        }
        _outputLines.push_back("");
    }

    _outputLines.push_back("Type HELP <command> for detailed help");
    _outputLines.push_back("");
    _outputLines.push_back("Ready.");
}

- (void)showHelpForCommand:(const FasterBASIC::ModularCommands::CommandDefinition*)cmd {
    _outputLines.push_back("");
    _outputLines.push_back(cmd->commandName + " - " + cmd->description);
    _outputLines.push_back(std::string(cmd->commandName.length() + cmd->description.length() + 3, '='));
    _outputLines.push_back("");

    std::string catUpper = cmd->category;
    std::transform(catUpper.begin(), catUpper.end(), catUpper.begin(), ::toupper);
    _outputLines.push_back("Category: " + catUpper);
    _outputLines.push_back("");

    // Show syntax
    std::string syntax = cmd->commandName;
    if (!cmd->parameters.empty()) {
        syntax += " ";
        for (size_t i = 0; i < cmd->parameters.size(); i++) {
            if (i > 0) syntax += ", ";
            if (cmd->parameters[i].isOptional) syntax += "[";
            syntax += cmd->parameters[i].name;
            if (cmd->parameters[i].isOptional) syntax += "]";
        }
    }
    _outputLines.push_back("Syntax: " + syntax);
    _outputLines.push_back("");

    // Show parameters
    if (!cmd->parameters.empty()) {
        _outputLines.push_back("Parameters:");
        for (const auto& param : cmd->parameters) {
            std::string paramLine = "  " + param.name;
            if (param.isOptional) {
                paramLine += " (optional)";
            }
            _outputLines.push_back(paramLine);
            _outputLines.push_back("      " + param.description);
            if (param.isOptional && !param.defaultValue.empty()) {
                _outputLines.push_back("      Default: " + param.defaultValue);
            }
        }
        _outputLines.push_back("");
    }

    _outputLines.push_back("Ready.");
}

- (void)handleHelpCartCommand {
    _outputLines.push_back("=== Cart Commands ===");
    _outputLines.push_back("");
    _outputLines.push_back("  CREATECART \"game.crt\" TITLE \"My Game\" AUTHOR \"Your Name\"");
    _outputLines.push_back("  USECART \"game.crt\"");
    _outputLines.push_back("  RUNCART \"game.crt\"");
    _outputLines.push_back("  SAVECART");
    _outputLines.push_back("  CLOSECART");
    _outputLines.push_back("  CART INFO");
    _outputLines.push_back("");
    _outputLines.push_back("  SET INFO TITLE \"New Title\"");
    _outputLines.push_back("  SET INFO AUTHOR \"Author Name\"");
    _outputLines.push_back("  SET INFO VERSION \"1.0.0\"");
    _outputLines.push_back("  SET INFO DESCRIPTION \"Game description\"");
    _outputLines.push_back("");
    _outputLines.push_back("Ready. (Press Enter to continue)");

    LOG_INFOF("[HELP CART] Cart help displayed, output lines now: %zu", _outputLines.size());
}

- (void)executeImmediateMode:(const std::string&)code {
    if (!_interactiveLuaState) {
        _outputLines.push_back("ERROR: Lua state not initialized");
        return;
    }

    // Compile the code with a line number wrapper
    std::string wrappedCode = "10 " + code;
    std::string luaCode = [self compileBasicCode:wrappedCode];

    if (luaCode.empty()) {
        // Error was already added to output by compileBasicCode
        return;
    }

    // Execute the Lua code
    int result = luaL_dostring(_interactiveLuaState, luaCode.c_str());
    if (result != LUA_OK) {
        const char* error = lua_tostring(_interactiveLuaState, -1);
        _outputLines.push_back(std::string("RUNTIME ERROR: ") + error);
        lua_pop(_interactiveLuaState, 1);
    }
}

// Synchronization methods between editor and program manager
- (void)syncEditorToProgramManager {
    if (!self.textEditor || !_programManager) {
        return;
    }

    // Use EditorBridge if available, otherwise fall back to manual sync
    if (_editorBridge) {
        if (_editorBridge->syncEditorToShell()) {
            LOG_INFOF("Synced %zu lines from editor to shell using EditorBridge",
                      _editorBridge->getShellLineCount());
        } else {
            LOG_INFO("EditorBridge sync failed, falling back to manual sync");
        }
        return;
    }

    // Legacy manual sync (fallback)
    _programManager->clear();

    std::string editorText = self.textEditor->getText();
    std::istringstream stream(editorText);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;

        FasterBASIC::ParsedCommand cmd = _commandParser->parse(line);
        if (cmd.type == FasterBASIC::ShellCommandType::DIRECT_LINE) {
            _programManager->setLine(cmd.lineNumber, cmd.code);
        }
    }

    LOG_INFOF("Synced %zu lines from editor to program manager (legacy)", _programManager->getLineCount());
}

- (void)syncProgramManagerToEditor {
    if (!self.textEditor || !_programManager) {
        return;
    }

    // Use EditorBridge if available
    if (_editorBridge) {
        if (_editorBridge->syncShellToEditor()) {
            LOG_INFOF("Synced %zu lines from shell to editor using EditorBridge",
                      _editorBridge->getEditorLineCount());

            // Reset cursor and re-render
            self.textEditor->setCursorPosition(0, 0);
            self.textEditor->update(0.0);
            self.textEditor->render();
        } else {
            LOG_INFO("EditorBridge sync failed");
        }
        return;
    }

    // Legacy manual sync (fallback)
    std::string programText = _programManager->generateProgram();
    FasterBASIC::FormatterResult result = FasterBASIC::formatClassic(programText);

    std::string formattedText;
    if (result.success) {
        formattedText = result.formatted_code;
        LOG_INFO("Formatted program before syncing to editor");
    } else {
        formattedText = programText;
        LOG_INFOF("Warning: Formatting failed, using unformatted text: %s", result.error_message.c_str());
    }

    self.textEditor->loadText(formattedText);
    self.textEditor->setCursorPosition(0, 0);
    self.textEditor->markDirty();
    self.textEditor->update(0.0);
    self.textEditor->render();

    LOG_INFOF("Synced %zu lines from program manager to editor (legacy)", self.textEditor->getLineCount());
}

- (void)runInteractiveProgram {
    if (_programManager->isEmpty()) {
        _outputLines.push_back("No program to run");
        return;
    }

    LOG_INFO("Running program from interactive mode...");

    // Sync program manager to editor before running
    if (self.textEditor && _programManager) {
        [self syncProgramManagerToEditor];
    }

    // Set flag to return to interactive mode after script completes
    LOG_INFO("runInteractiveProgram: Setting _returnToInteractiveAfterRun = true");
    _returnToInteractiveAfterRun = true;

    // Save interactive state before exiting
    [self saveInteractiveState];

    // Exit interactive mode (cleans up shell components and stops interactive rendering)
    [self exitInteractiveMode];

    // CRITICAL: Switch to runtime mode BEFORE running the script
    // This ensures the screen is cleared and we're in the correct display mode
    [self enterRuntimeMode];
    LOG_INFO("Switched to runtime mode, ready to execute program");

    // Now run the script using the standard script runner
    // This will use the program now in the editor
    // Note: runScript will call startScriptExecution which normally calls enterRuntimeMode,
    // but we've already done it here to ensure clean transition from interactive mode
    [self runScript];
}

- (std::string)compileBasicCode:(const std::string&)source {
    // Preprocess DATA statements
    DataPreprocessor preprocessor;
    DataPreprocessorResult dataResult = preprocessor.process(source);
// Tokenize
Lexer lexer;
if (!lexer.tokenize(dataResult.cleanedSource)) {
    if (!lexer.getErrors().empty()) {
        _outputLines.push_back("SYNTAX ERROR: " + lexer.getErrors()[0].toString());
    }
    return "";
}

// Parse
Parser parser;
auto program = parser.parse(lexer.getTokens(), "interactive");
if (!program) {
    if (!parser.getErrors().empty()) {
        _outputLines.push_back("PARSE ERROR: " + parser.getErrors()[0].toString());
    }
    return "";
}

// Semantic analysis
SemanticAnalyzer semantic;
CompilerOptions opts;
if (!semantic.analyze(*program, opts)) {
    if (!semantic.getErrors().empty()) {
        _outputLines.push_back("SEMANTIC ERROR: " + semantic.getErrors()[0].toString());
    }
    return "";
}

    // Build CFG
    CFGBuilder cfgBuilder;
    auto cfg = cfgBuilder.build(*program, semantic.getSymbolTable());

    // Generate IR
    IRGenerator irGen;
    auto irCode = irGen.generate(*cfg, semantic.getSymbolTable());

    // Set constants manager
    irCode->constantsManager = &semantic.getConstantsManager();

    // Generate Lua code
    LuaCodeGenerator luaGen;
    std::string luaCode = luaGen.generate(*irCode);

    // Copy constants from semantic analyzer to Lua runtime
    set_constants_manager(&semantic.getConstantsManager());

    return luaCode;
}

// =============================================================================
// Asset Management Command Handlers
// =============================================================================

- (void)handleCopyAssetCommand:(const std::string&)line {
    // Parse: COPY "source_path" TO "asset_name" [AS SPRITE|TILESET|SOUND|MUSIC|DATA]

    if (!self.cartManager || !self.cartManager->isCartActive()) {
        _outputLines.push_back("ERROR: No active cart. Use CREATECART or USECART first.");
        return;
    }

    // Find the source path (first quoted string)
    size_t firstQuote = line.find('"');
    if (firstQuote == std::string::npos) {
        _outputLines.push_back("Usage: COPY \"source_path\" TO \"asset_name\" [AS SPRITE|TILESET|SOUND|MUSIC|DATA]");
        return;
    }

    size_t firstQuoteEnd = line.find('"', firstQuote + 1);
    if (firstQuoteEnd == std::string::npos) {
        _outputLines.push_back("ERROR: Missing closing quote for source path");
        return;
    }

    std::string sourcePath = line.substr(firstQuote + 1, firstQuoteEnd - firstQuote - 1);

    // Find " TO "
    size_t toPos = line.find(" TO ", firstQuoteEnd);
    if (toPos == std::string::npos) {
        _outputLines.push_back("Usage: COPY \"source_path\" TO \"asset_name\" [AS SPRITE|TILESET|SOUND|MUSIC|DATA]");
        return;
    }

    // Find the asset name (second quoted string)
    size_t secondQuote = line.find('"', toPos + 4);
    if (secondQuote == std::string::npos) {
        _outputLines.push_back("ERROR: Missing asset name");
        return;
    }

    size_t secondQuoteEnd = line.find('"', secondQuote + 1);
    if (secondQuoteEnd == std::string::npos) {
        _outputLines.push_back("ERROR: Missing closing quote for asset name");
        return;
    }

    std::string assetName = line.substr(secondQuote + 1, secondQuoteEnd - secondQuote - 1);

    // Find asset type (optional " AS TYPE")
    std::string assetType = "AUTO";
    size_t asPos = line.find(" AS ", secondQuoteEnd);
    if (asPos != std::string::npos) {
        std::string typeStr = line.substr(asPos + 4);
        // Trim whitespace
        size_t start = typeStr.find_first_not_of(" \t\r\n");
        if (start != std::string::npos) {
            typeStr = typeStr.substr(start);
            size_t end = typeStr.find_first_of(" \t\r\n");
            if (end != std::string::npos) {
                typeStr = typeStr.substr(0, end);
            }
        }
        assetType = typeStr;
    }

    // Convert to uppercase
    for (char& c : assetType) {
        c = std::toupper(c);
    }

    // Determine file extension to detect type
    std::string ext;
    size_t dotPos = sourcePath.rfind('.');
    if (dotPos != std::string::npos) {
        ext = sourcePath.substr(dotPos);
        for (char& c : ext) {
            c = std::tolower(c);
        }
    }

    // Call appropriate CartManager method based on type
    SuperTerminal::CartOperationResult result;

    if (assetType == "SPRITE" || ext == ".png" || ext == ".bmp") {
        result = self.cartManager->addSpriteFromFile(sourcePath, assetName);
    }
    else if (assetType == "TILESET") {
        // Default tile size 16x16
        result = self.cartManager->addTilesetFromFile(sourcePath, assetName, 16, 16);
    }
    else if (assetType == "SOUND" || ext == ".wav" || ext == ".vscript") {
        result = self.cartManager->addSoundFromFile(sourcePath, assetName);
    }
    else if (assetType == "MUSIC" || ext == ".sid" || ext == ".abc" || ext == ".mid" || ext == ".midi") {
        if (ext == ".sid") {
            result = self.cartManager->addSidFromFile(sourcePath, assetName);
        } else if (ext == ".abc") {
            result = self.cartManager->addAbcFromFile(sourcePath, assetName);
        } else if (ext == ".mid" || ext == ".midi") {
            result = self.cartManager->addMidiFromFile(sourcePath, assetName);
        } else {
            result = self.cartManager->addMusicFromFile(sourcePath, assetName);
        }
    }
    else if (assetType == "DATA") {
        result = self.cartManager->addDataFile(sourcePath, assetName);
    }
    else {
        _outputLines.push_back("ERROR: Unknown asset type. Use AS SPRITE|TILESET|SOUND|MUSIC|DATA");
        return;
    }

    if (result.success) {
        _outputLines.push_back("✓ Asset copied: " + assetName);
    } else {
        _outputLines.push_back("ERROR: " + result.message);
    }
}

- (void)handlePlayCommand:(const std::string&)line {
    // Parse: PLAY "file" [AS "type"] [INTO_WAV "wavname"]
    // Checks cart first, then filesystem
    // type can be: ABC, SID, VSCRIPT (VoiceScript)

    // Find the filename (first quoted string)
    size_t firstQuote = line.find('"');
    if (firstQuote == std::string::npos) {
        _outputLines.push_back("Usage: PLAY \"file\" [AS \"type\"] [INTO_WAV \"wavname\"]");
        return;
    }

    size_t firstQuoteEnd = line.find('"', firstQuote + 1);
    if (firstQuoteEnd == std::string::npos) {
        _outputLines.push_back("ERROR: Missing closing quote for filename");
        return;
    }

    std::string filename = line.substr(firstQuote + 1, firstQuoteEnd - firstQuote - 1);

    // Check for optional AS "type" (case-insensitive)
    std::string asType;
    std::string upperLine = line;
    for (char& c : upperLine) {
        c = std::toupper(c);
    }
    size_t asPos = upperLine.find(" AS ", firstQuoteEnd);
    if (asPos != std::string::npos) {
        size_t typeQuote = line.find('"', asPos + 4);
        if (typeQuote != std::string::npos) {
            size_t typeQuoteEnd = line.find('"', typeQuote + 1);
            if (typeQuoteEnd != std::string::npos) {
                asType = line.substr(typeQuote + 1, typeQuoteEnd - typeQuote - 1);
                // Convert to lowercase for st_music_play_file_with_format
                for (char& c : asType) {
                    c = std::tolower(c);
                }
                // Normalize VSCRIPT to voicescript
                if (asType == "vscript") {
                    asType = "voicescript";
                }
            }
        }
    }

    // Check for optional INTO_WAV "wavname" (case-insensitive)
    std::string wavOutput;
    size_t intoWavPos = upperLine.find("INTO_WAV", firstQuoteEnd);
    if (intoWavPos != std::string::npos) {
        size_t wavQuote = line.find('"', intoWavPos + 8);
        if (wavQuote != std::string::npos) {
            size_t wavQuoteEnd = line.find('"', wavQuote + 1);
            if (wavQuoteEnd != std::string::npos) {
                wavOutput = line.substr(wavQuote + 1, wavQuoteEnd - wavQuote - 1);
            }
        }
    }

    // If INTO_WAV specified, render to WAV instead of playing
    if (!wavOutput.empty()) {
        _outputLines.push_back("Rendering " + filename + " to WAV: " + wavOutput);

        // Determine format from AS clause or file extension
        // Convert asType to uppercase for rendering logic
        std::string format = asType;
        if (!format.empty()) {
            for (char& c : format) {
                c = std::toupper(c);
            }
            // Normalize voicescript to VSCRIPT
            if (format == "VOICESCRIPT") {
                format = "VSCRIPT";
            }
        }
        if (format.empty()) {
            // Auto-detect from extension
            std::string lower = filename;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower.find(".abc") != std::string::npos) {
                format = "ABC";
            } else if (lower.find(".sid") != std::string::npos) {
                format = "SID";
            } else if (lower.find(".vscript") != std::string::npos) {
                format = "VSCRIPT";
            }
        }

        // Load file data from cart or filesystem
        std::vector<uint8_t> fileData;
        bool foundData = false;

        // Try cart first if active
        if (self.cartManager && self.cartManager->isCartActive()) {
            auto loader = self.cartManager->getLoader();
            if (loader) {
                SuperTerminal::CartDataFile dataFile;
                if (loader->loadDataFile(filename, dataFile)) {
                    fileData = dataFile.data;
                    foundData = true;
                }

                // Also try music table
                if (!foundData) {
                    SuperTerminal::CartMusic cartMusic;
                    if (loader->loadMusic(filename.c_str(), cartMusic)) {
                        fileData = cartMusic.data;
                        foundData = true;
                    }
                }
            }
        }

        // Try filesystem if not in cart
        if (!foundData) {
            std::ifstream file(filename, std::ios::binary | std::ios::ate);
            if (file.is_open()) {
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                fileData.resize(size);
                if (file.read(reinterpret_cast<char*>(fileData.data()), size)) {
                    foundData = true;
                }
            }
        }

        if (!foundData) {
            _outputLines.push_back("ERROR: Failed to load file: " + filename);
            return;
        }

        // Render based on format (uppercase)
        if (format == "VSCRIPT" || format == "VOICESCRIPT") {
            // Use a consistent temp script name
            std::string tempScriptName = "temp_render_script";
            std::string vscriptContent(fileData.begin(), fileData.end());

            // Define the script directly in memory
            std::string error;
            if (!self.audioManager->voiceScriptDefine(tempScriptName, vscriptContent, error)) {
                _outputLines.push_back("ERROR: Failed to compile VoiceScript: " + error);
                return;
            }

            // Render to WAV
            if (self.audioManager->voiceScriptRenderToWAV(tempScriptName, wavOutput, 10.0f)) {
                _outputLines.push_back("✓ Rendered to: " + wavOutput);

                // If cart is active, offer to add the WAV to cart
                if (self.cartManager && self.cartManager->isCartActive()) {
                    auto addResult = self.cartManager->addSoundFromFile(wavOutput, wavOutput);
                    if (addResult.success) {
                        _outputLines.push_back("✓ Added WAV to cart: " + wavOutput);
                    }
                }
            } else {
                _outputLines.push_back("ERROR: Failed to render VoiceScript");
                _outputLines.push_back("       Check that the VoiceScript is valid");
            }

            // Clean up the temp script
            self.audioManager->voiceScriptRemove(tempScriptName);
        }
        else if (format == "ABC") {
            // Render ABC notation to WAV using offline rendering
            std::string abcString(fileData.begin(), fileData.end());

            // Use abcRenderToWAV (0.0f = auto-detect duration from ABC)
            if (self.audioManager->abcRenderToWAV(abcString, wavOutput, 0.0f)) {
                _outputLines.push_back("✓ Rendered to: " + wavOutput);
                _outputLines.push_back("  Note: ABC rendering is partial - needs CoreAudioEngine capture");
            } else {
                _outputLines.push_back("ERROR: Failed to render ABC to WAV");
            }
        }
        else if (format == "SID") {
            // Render SID to WAV using offline rendering
            uint32_t sidId = self.audioManager->sidLoadMemory(fileData.data(), fileData.size());
            if (sidId == 0) {
                _outputLines.push_back("ERROR: Failed to load SID file");
                return;
            }

            // Render offline - default 3 minutes for SID tunes
            if (self.audioManager->sidRenderToWAV(sidId, wavOutput, 180.0f, 0)) {
                _outputLines.push_back("✓ Rendered to: " + wavOutput);
            } else {
                _outputLines.push_back("ERROR: Failed to render SID to WAV");
            }

            // Clean up
            self.audioManager->sidFree(sidId);
        }
        else {
            _outputLines.push_back("ERROR: Unknown format: " + format);
            _outputLines.push_back("       Use AS \"ABC\", \"SID\", or \"VSCRIPT\"");
        }
        return;
    }

    // Play the file (uses st_music_play_file which handles cart/filesystem)
    _outputLines.push_back("Playing: " + filename);

    if (!asType.empty()) {
        // Use st_music_play_file_with_format
        st_music_play_file_with_format(filename.c_str(), asType.c_str());
    } else {
        // Use st_music_play_file (auto-detects format)
        st_music_play_file(filename.c_str());
    }

    _outputLines.push_back("✓ Playback started");
}

- (void)handleDeleteAssetCommand:(const std::string&)line {
    // Parse: DELETE "asset_name"

    if (!self.cartManager || !self.cartManager->isCartActive()) {
        _outputLines.push_back("ERROR: No active cart. Use CREATECART or USECART first.");
        return;
    }

    // Find the asset name (quoted string)
    size_t firstQuote = line.find('"');
    if (firstQuote == std::string::npos) {
        _outputLines.push_back("Usage: DELETE \"asset_name\"");
        return;
    }

    size_t firstQuoteEnd = line.find('"', firstQuote + 1);
    if (firstQuoteEnd == std::string::npos) {
        _outputLines.push_back("ERROR: Missing closing quote for asset name");
        return;
    }

    std::string assetName = line.substr(firstQuote + 1, firstQuoteEnd - firstQuote - 1);

    // Use deleteAssetByName which searches all asset types
    std::string deletedFrom;
    auto result = self.cartManager->deleteAssetByName(assetName, deletedFrom);

    if (result.success) {
        _outputLines.push_back("✓ Deleted: " + assetName + " (" + deletedFrom + ")");
    } else {
        _outputLines.push_back("ERROR: " + result.message);
    }
}
