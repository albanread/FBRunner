// =============================================================================
// Editor Support Functions
// =============================================================================
// This file contains:
// - Automatic Line Numbering for BASIC
// - Syntax Highlighting

// =============================================================================
// Automatic Line Numbering for BASIC
// =============================================================================

// Helper function to check if a line is blank or only contains a line number (no statement)
- (BOOL)isLineBlankOrOnlyNumber:(const std::string&)line {
    size_t firstNonSpace = line.find_first_not_of(" \t");
    if (firstNonSpace == std::string::npos) {
        return YES; // Empty or whitespace only
    }

    std::string trimmed = line.substr(firstNonSpace);
    if (trimmed.empty()) {
        return YES;
    }

    // Check if it starts with a digit (line number)
    if (!std::isdigit(trimmed[0])) {
        return NO; // Doesn't start with line number
    }

    // Find where the line number ends
    size_t numEnd = 0;
    while (numEnd < trimmed.length() && std::isdigit(trimmed[numEnd])) {
        numEnd++;
    }

    // Get the rest after the line number
    std::string rest = trimmed.substr(numEnd);

    // Trim whitespace from rest
    size_t firstNonSpaceRest = rest.find_first_not_of(" \t");

    // If nothing after line number (or only whitespace), it's blank
    return (firstNonSpaceRest == std::string::npos);
}

// Helper function to extract line number from a line
- (int)extractLineNumber:(const std::string&)line {
    size_t firstNonSpace = line.find_first_not_of(" \t");
    if (firstNonSpace == std::string::npos) {
        return -1; // Empty or whitespace only
    }

    std::string trimmed = line.substr(firstNonSpace);
    if (trimmed.empty() || !std::isdigit(trimmed[0])) {
        return -1; // Doesn't start with a digit
    }

    size_t numEnd = 0;
    while (numEnd < trimmed.length() && std::isdigit(trimmed[numEnd])) {
        numEnd++;
    }

    if (numEnd == 0) {
        return -1;
    }

    return std::stoi(trimmed.substr(0, numEnd));
}

// Renumber the entire BASIC program starting from startLine with given step
- (void)renumberProgramFrom:(int)startLine step:(int)step {
    if (!self.textEditor) {
        return;
    }

    LOG_INFOF("Renumbering program from %d with step %d", startLine, step);

    SuperTerminal::TextBuffer* buffer = self.textEditor->getTextBuffer();
    size_t totalLines = buffer->getLineCount();

    // Build a list of line indices that have line numbers and their new numbers
    std::vector<std::pair<size_t, int>> renumberMap;
    int newLineNum = startLine;

    for (size_t i = 0; i < totalLines; i++) {
        std::string line = buffer->getLine(i);
        int oldLineNum = [self extractLineNumber:line];
        if (oldLineNum > 0) {
            renumberMap.push_back({i, newLineNum});
            newLineNum += step;
        }
    }

    // Now apply the renumbering
    for (const auto& [lineIdx, newNum] : renumberMap) {
        std::string line = buffer->getLine(lineIdx);

        // Find where the old line number ends
        size_t firstNonSpace = line.find_first_not_of(" \t");
        std::string trimmed = line.substr(firstNonSpace);
        size_t numEnd = 0;
        while (numEnd < trimmed.length() && std::isdigit(trimmed[numEnd])) {
            numEnd++;
        }

        // Replace old line number with new one
        std::string newLineStr = std::to_string(newNum);
        std::string restOfLine = trimmed.substr(numEnd);

        // Ensure there's a space after the line number
        if (restOfLine.empty() || restOfLine[0] != ' ') {
            newLineStr += " ";
        }
        std::string newLine = newLineStr + restOfLine;

        // Replace the entire line
        buffer->deleteLine(lineIdx);
        buffer->insertLine(lineIdx, newLine);
    }

    self.textEditor->markDirty();
    LOG_INFOF("Renumbered %zu lines", renumberMap.size());
}

- (void)setupLineNumbering {
    if (!self.textEditor) {
        return;
    }

    // Capture self for the callback
    __weak FBRunner3App* weakSelf = self;

    // Set up callback for automatic line numbering when Enter is pressed
    self.textEditor->setPostNewlineCallback([weakSelf](size_t lineNumber, SuperTerminal::TextBuffer* buffer, SuperTerminal::Cursor* cursor) {
        FBRunner3App* strongSelf = weakSelf;
        if (!strongSelf || lineNumber == 0) {
            return;
        }

        // Get the previous line
        std::string prevLine = buffer->getLine(lineNumber - 1);

        // Check if the previous line is blank or only contains a line number
        // If so, undo the newline insertion (stay on the same line)
        if ([strongSelf isLineBlankOrOnlyNumber:prevLine]) {
            // Delete the newly created line (join it back)
            if (lineNumber < buffer->getLineCount()) {
                buffer->joinLine(lineNumber - 1);
            }
            // Move cursor to end of the previous line
            size_t prevLineLen = buffer->getLine(lineNumber - 1).length();
            cursor->setPosition(lineNumber - 1, prevLineLen, *buffer);
            LOG_INFO("Blank line detected - staying on same line");
            return;
        }

        int prevLineNum = [strongSelf extractLineNumber:prevLine];

        if (prevLineNum < 0) {
            return; // Previous line doesn't have a line number
        }

        // Check if there's a next line with a line number
        int nextLineNum = -1;
        size_t totalLines = buffer->getLineCount();

        for (size_t i = lineNumber + 1; i < totalLines; i++) {
            std::string nextLine = buffer->getLine(i);
            int lineNum = [strongSelf extractLineNumber:nextLine];
            if (lineNum > 0) {
                nextLineNum = lineNum;
                break;
            }
        }

        int newLineNum;
        bool needsRenumber = false;

        if (nextLineNum > 0) {
            // We're inserting between two numbered lines
            int available = nextLineNum - prevLineNum;

            if (available > 1) {
                // We have space - insert the next available number
                newLineNum = prevLineNum + 1;
                LOG_INFOF("Inserting line %d between %d and %d", newLineNum, prevLineNum, nextLineNum);

                // Check if we're running out of space (e.g., at 29 before 30)
                if (available == 2) {
                    // This is the last line we can fit - schedule renumber for next time
                    LOG_WARNING("Running out of line numbers. Next insert will trigger renumber.");
                }
            } else {
                // No space! Need to renumber
                LOG_INFOF("No space between %d and %d - triggering renumber", prevLineNum, nextLineNum);
                needsRenumber = true;

                // For now, insert a temporary number, then renumber
                newLineNum = prevLineNum + 1;
            }
        } else {
            // No next line - just increment
            newLineNum = prevLineNum + 1;
            LOG_INFOF("Appending line %d after %d", newLineNum, prevLineNum);
        }

        // Insert the new line number with a space
        std::string newLineNumStr = std::to_string(newLineNum) + " ";
        buffer->insertText(lineNumber, 0, newLineNumStr);

        // Move cursor after the line number and space (so cursor is ready for typing)
        cursor->setPosition(lineNumber, newLineNumStr.length(), *buffer);

        LOG_INFOF("Auto-inserted line number %d, cursor at column %zu", newLineNum, newLineNumStr.length());

        // If we need to renumber, do it after a short delay
        if (needsRenumber) {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                FBRunner3App* self = weakSelf;
                if (self) {
                    [self renumberProgramFrom:1000 step:10];

                    // Show a message
                    NSAlert* alert = [[NSAlert alloc] init];
                    [alert setMessageText:@"Auto-Renumbered"];
                    [alert setInformativeText:@"Line numbers were renumbered from 1000 in steps of 10 because there was no space for new lines."];
                    [alert addButtonWithTitle:@"OK"];
                    [alert setAlertStyle:NSAlertStyleInformational];
                    [alert runModal];
                }
            });
        }
    });

    LOG_INFO("Automatic line numbering enabled");
}


// =============================================================================
// Syntax Highlighting
// =============================================================================

- (void)extractUserDefinedConstants {
    // Parse the current editor text to find all CONSTANT statements
    _userDefinedConstants.clear();

    if (!self.textEditor) {
        return;
    }

    std::string editorText = self.textEditor->getText();
    std::istringstream stream(editorText);
    std::string line;

    while (std::getline(stream, line)) {
        // Simple regex-free parsing: look for "CONSTANT name ="
        std::string upperLine = line;
        std::transform(upperLine.begin(), upperLine.end(), upperLine.begin(), ::toupper);

        size_t constPos = upperLine.find("CONSTANT");
        if (constPos != std::string::npos) {
            // Skip past "CONSTANT"
            size_t nameStart = constPos + 8;
            while (nameStart < upperLine.length() && std::isspace(upperLine[nameStart])) {
                nameStart++;
            }

            // Extract the constant name
            size_t nameEnd = nameStart;
            while (nameEnd < upperLine.length() &&
                   (std::isalnum(upperLine[nameEnd]) || upperLine[nameEnd] == '_' || upperLine[nameEnd] == '$')) {
                nameEnd++;
            }

            if (nameEnd > nameStart) {
                std::string constName = upperLine.substr(nameStart, nameEnd - nameStart);
                _userDefinedConstants.insert(constName);
            }
        }
    }
}

- (std::vector<uint32_t>)highlightLine:(const std::string&)line
                            lineNumber:(size_t)lineNumber {
    // Extract user-defined constants when editor content changes
    // Use a simple approach: extract whenever highlighting is called and cache is dirty
    static bool constantsCacheDirty = true;
    static std::string lastEditorContent;

    if (self.textEditor) {
        std::string currentContent = self.textEditor->getText();
        if (constantsCacheDirty || currentContent != lastEditorContent) {
            [self extractUserDefinedConstants];
            lastEditorContent = currentContent;
            constantsCacheDirty = false;
        }
    }

    std::vector<uint32_t> colors;
    colors.reserve(line.length());

    // BASIC syntax highlighting colors (RGBA format: 0xRRGGBBAA)
    const uint32_t COLOR_KEYWORD = 0xFFFFFFFF;    // White for keywords
    const uint32_t COLOR_COMMAND = 0x00FFFFFF;    // Cyan for registered commands
    const uint32_t COLOR_FUNCTION = 0x00CCFFFF;   // Light cyan for registered functions
    const uint32_t COLOR_CONSTANT = 0xFFAAAAFF;   // Light red/pink for constants
    const uint32_t COLOR_STRING = 0x00FF00FF;     // Green for strings
    const uint32_t COLOR_COMMENT = 0xC0C0C0FF;    // Light gray for comments
    const uint32_t COLOR_NUMBER = 0xFFFF00FF;     // Yellow for numbers
    const uint32_t COLOR_LINENUM = 0xFF0000FF;    // Red for line numbers
    const uint32_t COLOR_DEFAULT = 0xC0C0C0FF;    // Light gray default

    // Get the command registry for validating commands
    CommandRegistry& registry = getGlobalCommandRegistry();

    // BASIC control flow keywords (these are language keywords, not commands)
    static const std::vector<std::string> keywords = {
        "PRINT", "INPUT", "LET", "IF", "THEN", "ELSE", "ELSEIF", "ENDIF",
        "GOTO", "GOSUB", "RETURN", "FOR", "TO", "STEP", "NEXT",
        "WHILE", "WEND", "DO", "LOOP", "UNTIL", "REPEAT",
        "DIM", "DATA", "READ", "RESTORE", "END", "EXIT",
        "REM", "AND", "OR", "NOT", "XOR", "MOD",
        "FUNCTION", "SUB", "CALL", "LOCAL", "DEF", "FN",
        "CASE", "SELECT", "OF", "WHEN", "OTHERWISE", "ENDCASE",
        "OPTION", "BASE", "INCLUDE", "ONCE"
    };

    // BASIC built-in functions (these are built into the language)
    static const std::vector<std::string> builtins = {
        "INT", "ABS", "SIN", "COS", "TAN", "SQR", "LOG", "EXP", "RND",
        "LEFT$", "RIGHT$", "MID$", "LEN", "STR$", "VAL", "CHR$", "ASC",
        "INKEY$", "ATN", "SGN"
    };

    // Get predefined constants from ConstantsManager (cached on first call)
    if (_predefinedConstantNames.empty()) {
        FasterBASIC::ConstantsManager tempManager;
        tempManager.addPredefinedConstants();
        _predefinedConstantNames = tempManager.getAllConstantNames();

        // Also add runtime-injected constant names
        _predefinedConstantNames.push_back("WINDOW_WIDTH");
        _predefinedConstantNames.push_back("WINDOW_HEIGHT");
        _predefinedConstantNames.push_back("TEXT_COLS");
        _predefinedConstantNames.push_back("TEXT_ROWS");
        _predefinedConstantNames.push_back("GRAPHICS_WIDTH");
        _predefinedConstantNames.push_back("GRAPHICS_HEIGHT");
        _predefinedConstantNames.push_back("SIXEL_WIDTH");
        _predefinedConstantNames.push_back("SIXEL_HEIGHT");
        _predefinedConstantNames.push_back("KEY_UP");
        _predefinedConstantNames.push_back("KEY_DOWN");
        _predefinedConstantNames.push_back("KEY_LEFT");
        _predefinedConstantNames.push_back("KEY_RIGHT");
        _predefinedConstantNames.push_back("KEY_RETURN");
        _predefinedConstantNames.push_back("KEY_ESCAPE");
        _predefinedConstantNames.push_back("KEY_SPACE");
        _predefinedConstantNames.push_back("KEY_DELETE");
        _predefinedConstantNames.push_back("KEY_TAB");
    }

    size_t i = 0;

    // Check for line number at start
    if (i < line.length() && std::isdigit(line[i])) {
        while (i < line.length() && std::isdigit(line[i])) {
            colors.push_back(COLOR_LINENUM);
            i++;
        }
        // Skip whitespace after line number
        while (i < line.length() && std::isspace(line[i])) {
            colors.push_back(COLOR_DEFAULT);
            i++;
        }
    }

    while (i < line.length()) {
        char ch = line[i];

        // REM comments (rest of line)
        if (i + 3 <= line.length()) {
            std::string word = line.substr(i, 3);
            std::transform(word.begin(), word.end(), word.begin(), ::toupper);
            if (word == "REM") {
                // Rest of line is comment
                while (i < line.length()) {
                    colors.push_back(COLOR_COMMENT);
                    i++;
                }
                break;
            }
        }

        // Single quote comments (alternative to REM)
        if (ch == '\'') {
            // Rest of line is comment
            while (i < line.length()) {
                colors.push_back(COLOR_COMMENT);
                i++;
            }
            break;
        }

        // Strings with double quotes
        if (ch == '"') {
            colors.push_back(COLOR_STRING);
            i++;
            while (i < line.length() && line[i] != '"') {
                colors.push_back(COLOR_STRING);
                i++;
            }
            if (i < line.length()) {
                colors.push_back(COLOR_STRING);
                i++;
            }
            continue;
        }

        // Hexadecimal numbers (&H prefix)
        if (ch == '&' && i + 1 < line.length() && (line[i+1] == 'H' || line[i+1] == 'h')) {
            colors.push_back(COLOR_NUMBER);
            colors.push_back(COLOR_NUMBER);
            i += 2;
            while (i < line.length() && std::isxdigit(line[i])) {
                colors.push_back(COLOR_NUMBER);
                i++;
            }
            continue;
        }

        // Numbers
        if (std::isdigit(ch) || (ch == '.' && i + 1 < line.length() && std::isdigit(line[i + 1]))) {
            while (i < line.length() && (std::isdigit(line[i]) || line[i] == '.' ||
                   line[i] == 'E' || line[i] == 'e' ||
                   (i > 0 && (line[i-1] == 'E' || line[i-1] == 'e') && (line[i] == '+' || line[i] == '-')))) {
                colors.push_back(COLOR_NUMBER);
                i++;
            }
            continue;
        }

        // Identifiers and keywords
        if (std::isalpha(ch) || ch == '_') {
            size_t start = i;
            while (i < line.length() && (std::isalnum(line[i]) || line[i] == '_' || line[i] == '$')) {
                i++;
            }
            std::string word = line.substr(start, i - start);

            // Convert to uppercase for keyword matching
            std::string wordUpper = word;
            std::transform(wordUpper.begin(), wordUpper.end(), wordUpper.begin(), ::toupper);

            // Check if keyword, builtin, constant, registered command, or registered function
            bool isKeyword = false;
            bool isBuiltin = false;
            bool isConstant = false;
            bool isCommand = false;
            bool isFunction = false;

            // First check language keywords
            for (const auto& kw : keywords) {
                if (wordUpper == kw) {
                    isKeyword = true;
                    break;
                }
            }

            // Then check built-in functions
            if (!isKeyword) {
                for (const auto& fn : builtins) {
                    if (wordUpper == fn) {
                        isBuiltin = true;
                        break;
                    }
                }
            }

            // Check if it's a predefined constant
            if (!isKeyword && !isBuiltin) {
                for (const auto& cn : _predefinedConstantNames) {
                    if (wordUpper == cn) {
                        isConstant = true;
                        break;
                    }
                }
            }

            // Check if it's a user-defined constant
            if (!isKeyword && !isBuiltin && !isConstant) {
                if (_userDefinedConstants.find(wordUpper) != _userDefinedConstants.end()) {
                    isConstant = true;
                }
            }

            // Check if it's a registered command from the modular registry
            if (!isKeyword && !isBuiltin && !isConstant) {
                if (registry.hasCommand(wordUpper)) {
                    isCommand = true;
                }
            }

            // Check if it's a registered function from the modular registry
            if (!isKeyword && !isBuiltin && !isConstant && !isCommand) {
                if (registry.hasFunction(wordUpper)) {
                    isFunction = true;
                }
            }

            // Assign color based on what we found
            uint32_t color = isKeyword ? COLOR_KEYWORD :
                           isBuiltin ? COLOR_KEYWORD :
                           isConstant ? COLOR_CONSTANT :
                           isCommand ? COLOR_COMMAND :
                           isFunction ? COLOR_FUNCTION :
                           COLOR_DEFAULT;

            for (size_t k = start; k < i; k++) {
                colors.push_back(color);
            }
            continue;
        }

        // Default color
        colors.push_back(COLOR_DEFAULT);
        i++;
    }

    return colors;
}

