//
// HelpViewController.mm
// FBRunner3 - BASIC Command Help Viewer
//
// Displays rich text help documentation extracted from the command registry.
// Provides categorized command reference with syntax highlighting and examples.
//

#import "HelpViewController.h"
#include "../FasterBASICT/src/modular_commands.h"
#include <sstream>
#include <algorithm>

using namespace FasterBASIC::ModularCommands;

// =============================================================================
// HelpContentGenerator Implementation
// =============================================================================

std::vector<HelpTopic> HelpContentGenerator::loadArticles() {
    std::vector<HelpTopic> articles;

    // Get the app bundle's Resources/Articles directory
    NSBundle* bundle = [NSBundle mainBundle];
    NSLog(@"[Help] Bundle path: %@", [bundle resourcePath]);
    NSString* articlesPath = [bundle pathForResource:@"Articles" ofType:nil];

    if (!articlesPath) {
        NSLog(@"[Help] Articles directory not found in bundle");
        // Try alternate path
        articlesPath = [[bundle resourcePath] stringByAppendingPathComponent:@"Articles"];
        NSLog(@"[Help] Trying alternate path: %@", articlesPath);
        if (![[NSFileManager defaultManager] fileExistsAtPath:articlesPath]) {
            NSLog(@"[Help] Alternate path also not found");
            return articles;
        }
    }

    NSLog(@"[Help] Articles path: %@", articlesPath);

    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSError* error = nil;
    NSArray* files = [fileManager contentsOfDirectoryAtPath:articlesPath error:&error];

    if (error) {
        NSLog(@"[Help] Error reading Articles directory: %@", error);
        return articles;
    }

    NSLog(@"[Help] Found %lu files in Articles directory", (unsigned long)files.count);

    for (NSString* filename in files) {
        NSLog(@"[Help] Checking file: %@", filename);
        if ([filename hasSuffix:@".md"]) {
            NSString* fullPath = [articlesPath stringByAppendingPathComponent:filename];
            NSLog(@"[Help] Loading article: %@", fullPath);
            NSString* content = [NSString stringWithContentsOfFile:fullPath encoding:NSUTF8StringEncoding error:&error];

            if (!error && content) {
                NSLog(@"[Help] Article loaded successfully, length: %lu", (unsigned long)content.length);
                // Extract first heading as article name
                NSString* firstLine = [[content componentsSeparatedByString:@"\n"] firstObject];
                NSString* articleName = [filename stringByDeletingPathExtension];

                if ([firstLine hasPrefix:@"# "]) {
                    articleName = [firstLine substringFromIndex:2];
                }

                HelpTopic article;
                article.name = [articleName UTF8String];
                article.category = "article";
                article.description = "Article";
                article.isArticle = true;
                article.articlePath = [fullPath UTF8String];

                articles.push_back(article);
                NSLog(@"[Help] Added article: %s", article.name.c_str());
            } else if (error) {
                NSLog(@"[Help] Error loading article %@: %@", filename, error);
            }
        }
    }

    NSLog(@"[Help] Total articles loaded: %lu", (unsigned long)articles.size());

    // Sort articles by name
    std::sort(articles.begin(), articles.end(), [](const HelpTopic& a, const HelpTopic& b) {
        return a.name < b.name;
    });

    return articles;
}

std::string HelpContentGenerator::renderMarkdown(const std::string& markdown) {
    // Simple markdown to plain text converter
    std::string result = markdown;

    // Replace markdown headings with emphasized text
    size_t pos = 0;
    while ((pos = result.find("\n### ", pos)) != std::string::npos) {
        result.replace(pos, 5, "\n   ");
        pos += 4;
    }
    pos = 0;
    while ((pos = result.find("\n## ", pos)) != std::string::npos) {
        result.replace(pos, 4, "\n  ");
        pos += 3;
    }
    pos = 0;
    while ((pos = result.find("# ", pos)) != std::string::npos) {
        if (pos == 0 || result[pos-1] == '\n') {
            result.replace(pos, 2, "");
        } else {
            pos += 2;
        }
    }

    // Bold markers (**text** -> text)
    pos = 0;
    while ((pos = result.find("**", pos)) != std::string::npos) {
        result.erase(pos, 2);
        size_t endPos = result.find("**", pos);
        if (endPos != std::string::npos) {
            result.erase(endPos, 2);
        }
    }

    // Code blocks (```language\n...``` -> ...)
    pos = 0;
    while ((pos = result.find("```", pos)) != std::string::npos) {
        size_t endLine = result.find("\n", pos + 3);
        if (endLine != std::string::npos) {
            result.erase(pos, endLine - pos + 1);
            size_t endBlock = result.find("```", pos);
            if (endBlock != std::string::npos) {
                result.erase(endBlock, 3);
            }
        } else {
            pos += 3;
        }
    }

    // Inline code (`code` -> code)
    pos = 0;
    while ((pos = result.find("`", pos)) != std::string::npos) {
        result.erase(pos, 1);
        size_t endPos = result.find("`", pos);
        if (endPos != std::string::npos) {
            result.erase(endPos, 1);
        }
    }

    return result;
}

std::vector<HelpTopic> HelpContentGenerator::generateTopics(CommandRegistry* registry) {
    std::vector<HelpTopic> topics;

    if (!registry) {
        return topics;
    }

    // Load articles first
    std::vector<HelpTopic> articles = loadArticles();
    if (!articles.empty()) {
        HelpTopic articlesSection;
        articlesSection.name = "📖 Articles";
        articlesSection.category = "articles_header";
        articlesSection.description = "Documentation articles";
        articlesSection.isArticle = false;
        topics.push_back(articlesSection);

        for (const auto& article : articles) {
            topics.push_back(article);
        }
    }

    // Get all categories from the registry
    std::map<std::string, std::vector<std::string>> categorizedCommands;

    // Iterate through all commands
    const auto& allCommands = registry->getAllCommands();
    for (const auto& pair : allCommands) {
        const std::string& cmdName = pair.first;
        const CommandDefinition& cmd = pair.second;
        std::string category = cmd.category.empty() ? "General" : cmd.category;

        if (cmd.isFunction) {
            categorizedCommands[category].push_back(cmdName + "()");
        } else {
            categorizedCommands[category].push_back(cmdName);
        }
    }

    // Sort commands within each category
    for (auto& pair : categorizedCommands) {
        std::sort(pair.second.begin(), pair.second.end());
    }

    // Add command reference header
    if (!categorizedCommands.empty()) {
        HelpTopic commandsHeader;
        commandsHeader.name = "📚 Command Reference";
        commandsHeader.category = "commands_header";
        commandsHeader.description = "BASIC commands and functions";
        commandsHeader.isArticle = false;
        topics.push_back(commandsHeader);
    }

    // Create topics
    for (const auto& pair : categorizedCommands) {
        HelpTopic topic;
        topic.category = pair.first;
        topic.name = pair.first + " Commands";
        topic.commands = pair.second;
        topic.isArticle = false;

        // Generate description based on category
        if (pair.first == "video") {
            topic.description = "Unified video mode commands that work across all display modes";
        } else if (pair.first == "sprite") {
            topic.description = "Sprite loading, display, and manipulation commands";
        } else if (pair.first == "audio") {
            topic.description = "Sound and music playback commands";
        } else if (pair.first == "input") {
            topic.description = "Keyboard, mouse, and input handling";
        } else if (pair.first == "graphics") {
            topic.description = "Graphics drawing and display commands";
        } else if (pair.first == "text") {
            topic.description = "Text display and formatting commands";
        } else {
            topic.description = "Commands in the " + pair.first + " category";
        }

        topics.push_back(topic);
    }

    return topics;
}

std::string HelpContentGenerator::generateCommandHelp(const std::string& commandName, CommandRegistry* registry) {
    if (!registry) {
        return "Error: No command registry available";
    }

    std::ostringstream ss;

    // Try to find as command first
    const CommandDefinition* cmd = registry->getCommand(commandName);
    bool isFunction = false;

    // If not found, try removing () if present
    if (!cmd) {
        std::string funcName = commandName;
        if (funcName.length() > 2 && funcName.substr(funcName.length() - 2) == "()") {
            funcName = funcName.substr(0, funcName.length() - 2);
        }
        cmd = registry->getCommand(funcName);
        if (cmd) {
            isFunction = cmd->isFunction;
        }
    } else {
        isFunction = cmd->isFunction;
    }

    if (!cmd) {
        ss << "Command not found: " << commandName << "\n";
        return ss.str();
    }

    // Generate help text
    ss << "═══════════════════════════════════════════════════════════\n";
    ss << " " << cmd->commandName;
    if (isFunction) ss << "()";
    ss << "\n";
    ss << "═══════════════════════════════════════════════════════════\n\n";

    // Description
    ss << "DESCRIPTION:\n";
    ss << "  " << cmd->description << "\n\n";

    // Syntax
    ss << "SYNTAX:\n";
    ss << "  ";

    if (isFunction) {
        // Function syntax
        ss << cmd->commandName << "(";
        const auto& params = cmd->parameters;
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) ss << ", ";
            if (params[i].isOptional) ss << "[";
            ss << params[i].name;
            if (params[i].isOptional) ss << "]";
        }
        ss << ")";

        // Return type
        auto retType = cmd->returnType;
        if (retType != ReturnType::VOID) {
            ss << " → ";
            switch (retType) {
                case ReturnType::INT: ss << "INTEGER"; break;
                case ReturnType::FLOAT: ss << "FLOAT"; break;
                case ReturnType::STRING: ss << "STRING"; break;
                case ReturnType::BOOL: ss << "BOOLEAN"; break;
                default: ss << "VALUE"; break;
            }
        }
        ss << "\n\n";
    } else {
        // Command syntax
        ss << cmd->commandName;
        const auto& params = cmd->parameters;
        for (const auto& param : params) {
            ss << " ";
            if (param.isOptional) ss << "[";
            ss << param.name;
            if (param.isOptional) ss << "]";
        }
        ss << "\n\n";
    }

    // Parameters
    if (!cmd->parameters.empty()) {
        ss << "PARAMETERS:\n";
        for (const auto& param : cmd->parameters) {
            ss << "  " << param.name;

            // Type
            ss << " (";
            switch (param.type) {
                case ParameterType::INT: ss << "INTEGER"; break;
                case ParameterType::FLOAT: ss << "FLOAT"; break;
                case ParameterType::STRING: ss << "STRING"; break;
                case ParameterType::BOOL: ss << "BOOLEAN"; break;
                case ParameterType::COLOR: ss << "COLOR"; break;
                case ParameterType::OPTIONAL: ss << "VALUE"; break;
                default: ss << "VALUE"; break;
            }
            ss << ")";

            if (param.isOptional) {
                ss << " [optional]";
                if (!param.defaultValue.empty()) {
                    ss << " (default: " << param.defaultValue << ")";
                }
            }

            ss << "\n";

            // Description
            if (!param.description.empty()) {
                ss << "    " << param.description << "\n";
            }
        }
        ss << "\n";
    }

    // Category
    if (!cmd->category.empty()) {
        ss << "CATEGORY: " << cmd->category << "\n\n";
    }

    // Examples (placeholder for now)
    ss << "EXAMPLE:\n";
    ss << "  REM Example usage of " << cmd->commandName << "\n";
    if (isFunction) {
        ss << "  result = " << cmd->commandName << "(";
        const auto& params = cmd->parameters;
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) ss << ", ";
            switch (params[i].type) {
                case ParameterType::INT: ss << "10"; break;
                case ParameterType::FLOAT: ss << "1.5"; break;
                case ParameterType::STRING: ss << "\"text\""; break;
                case ParameterType::BOOL: ss << "-1"; break;
                case ParameterType::COLOR: ss << "XRGB(15,0,0)"; break;
                default: ss << "value"; break;
            }
        }
        ss << ")\n";
    } else {
        ss << "  " << cmd->commandName;
        const auto& params = cmd->parameters;
        for (size_t i = 0; i < params.size() && i < 3; ++i) {
            ss << " ";
            switch (params[i].type) {
                case ParameterType::INT: ss << "100"; break;
                case ParameterType::FLOAT: ss << "1.5"; break;
                case ParameterType::STRING: ss << "\"Hello\""; break;
                case ParameterType::BOOL: ss << "-1"; break;
                case ParameterType::COLOR: ss << "XRGB(15,15,15)"; break;
                default: ss << "value"; break;
            }
        }
        ss << "\n";
    }

    ss << "\n═══════════════════════════════════════════════════════════\n";

    return ss.str();
}

std::string HelpContentGenerator::generateCategoryOverview(const std::string& category, CommandRegistry* registry) {
    if (!registry) {
        return "Error: No command registry available";
    }

    std::ostringstream ss;

    ss << "═══════════════════════════════════════════════════════════\n";
    ss << " " << category << " COMMANDS\n";
    ss << "═══════════════════════════════════════════════════════════\n\n";

    // Collect all commands in this category
    std::vector<std::string> commands;
    std::vector<std::string> functions;

    const auto& allCommands = registry->getAllCommands();
    for (const auto& pair : allCommands) {
        if (pair.second.category == category) {
            if (pair.second.isFunction) {
                functions.push_back(pair.first + "()");
            } else {
                commands.push_back(pair.first);
            }
        }
    }

    std::sort(commands.begin(), commands.end());
    std::sort(functions.begin(), functions.end());

    // Display commands
    if (!commands.empty()) {
        ss << "COMMANDS:\n";
        for (const auto& cmd : commands) {
            const CommandDefinition* def = registry->getCommand(cmd);
            if (def) {
                ss << "  " << cmd;
                // Pad to 30 characters
                int padding = 30 - cmd.length();
                if (padding > 0) {
                    ss << std::string(padding, ' ');
                }
                ss << " - " << def->description << "\n";
            }
        }
        ss << "\n";
    }

    // Display functions
    if (!functions.empty()) {
        ss << "FUNCTIONS:\n";
        for (const auto& func : functions) {
            std::string funcName = func.substr(0, func.length() - 2);
            const CommandDefinition* def = registry->getCommand(funcName);
            if (def) {
                ss << "  " << func;
                // Pad to 30 characters
                int padding = 30 - func.length();
                if (padding > 0) {
                    ss << std::string(padding, ' ');
                }
                ss << " - " << def->description << "\n";
            }
        }
        ss << "\n";
    }

    if (commands.empty() && functions.empty()) {
        ss << "No commands found in this category.\n\n";
    }

    ss << "═══════════════════════════════════════════════════════════\n";

    return ss.str();
}

NSAttributedString* HelpContentGenerator::formatHelpText(const std::string& text) {
    NSString* nsText = [NSString stringWithUTF8String:text.c_str()];
    NSMutableAttributedString* attrString = [[NSMutableAttributedString alloc] initWithString:nsText];

    // Create paragraph style
    NSMutableParagraphStyle* paragraphStyle = [[NSMutableParagraphStyle alloc] init];
    [paragraphStyle setLineSpacing:2.0];

    // Base font
    NSFont* baseFont = [NSFont fontWithName:@"Menlo" size:11.0];
    if (!baseFont) {
        baseFont = [NSFont systemFontOfSize:11.0];
    }

    // Apply base attributes
    [attrString addAttribute:NSFontAttributeName value:baseFont range:NSMakeRange(0, attrString.length)];
    [attrString addAttribute:NSParagraphStyleAttributeName value:paragraphStyle range:NSMakeRange(0, attrString.length)];
    [attrString addAttribute:NSForegroundColorAttributeName value:[NSColor textColor] range:NSMakeRange(0, attrString.length)];

    // Bold font for headers
    NSFont* boldFont = [NSFont fontWithName:@"Menlo-Bold" size:11.0];
    if (!boldFont) {
        boldFont = [NSFont boldSystemFontOfSize:11.0];
    }

    // Header font for title
    NSFont* titleFont = [NSFont fontWithName:@"Menlo-Bold" size:13.0];
    if (!titleFont) {
        titleFont = [NSFont boldSystemFontOfSize:13.0];
    }

    // Highlight keywords
    NSRegularExpression* headerRegex = [NSRegularExpression regularExpressionWithPattern:@"^[A-Z][A-Z ]+:$" options:NSRegularExpressionAnchorsMatchLines error:nil];
    [headerRegex enumerateMatchesInString:nsText options:0 range:NSMakeRange(0, nsText.length) usingBlock:^(NSTextCheckingResult *match, NSMatchingFlags flags, BOOL *stop) {
        [attrString addAttribute:NSFontAttributeName value:boldFont range:match.range];
        [attrString addAttribute:NSForegroundColorAttributeName value:[NSColor systemBlueColor] range:match.range];
    }];

    // Highlight separator lines
    NSRegularExpression* separatorRegex = [NSRegularExpression regularExpressionWithPattern:@"═+" options:0 error:nil];
    [separatorRegex enumerateMatchesInString:nsText options:0 range:NSMakeRange(0, nsText.length) usingBlock:^(NSTextCheckingResult *match, NSMatchingFlags flags, BOOL *stop) {
        [attrString addAttribute:NSForegroundColorAttributeName value:[NSColor systemGrayColor] range:match.range];
    }];

    // Highlight REM comments
    NSRegularExpression* remRegex = [NSRegularExpression regularExpressionWithPattern:@"\\bREM\\b.*$" options:NSRegularExpressionAnchorsMatchLines error:nil];
    [remRegex enumerateMatchesInString:nsText options:0 range:NSMakeRange(0, nsText.length) usingBlock:^(NSTextCheckingResult *match, NSMatchingFlags flags, BOOL *stop) {
        [attrString addAttribute:NSForegroundColorAttributeName value:[NSColor systemGreenColor] range:match.range];
    }];

    // Highlight strings
    NSRegularExpression* stringRegex = [NSRegularExpression regularExpressionWithPattern:@"\"[^\"]*\"" options:0 error:nil];
    [stringRegex enumerateMatchesInString:nsText options:0 range:NSMakeRange(0, nsText.length) usingBlock:^(NSTextCheckingResult *match, NSMatchingFlags flags, BOOL *stop) {
        [attrString addAttribute:NSForegroundColorAttributeName value:[NSColor systemRedColor] range:match.range];
    }];

    return attrString;
}

// =============================================================================
// HelpViewController Implementation
// =============================================================================

@implementation HelpViewController {
    NSWindow* _helpWindow;
    std::vector<HelpTopic> _topics;
}

- (instancetype)initWithCommandRegistry:(CommandRegistry*)registry {
    self = [super initWithNibName:nil bundle:nil];
    if (self) {
        _commandRegistry = registry;
        _topics = HelpContentGenerator::generateTopics(registry);
        NSLog(@"[Help] Initialized with %lu topics", (unsigned long)_topics.size());
        for (const auto& topic : _topics) {
            NSLog(@"[Help] Topic: %s (isArticle: %d, commands: %lu)",
                  topic.name.c_str(), topic.isArticle, (unsigned long)topic.commands.size());
        }
    }
    return self;
}

- (void)loadView {
    // Create main container view
    NSView* view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 900, 600)];
    self.view = view;

    // Create search field at top with auto-resizing
    _searchField = [[NSSearchField alloc] initWithFrame:NSMakeRect(20, 560, 860, 30)];
    _searchField.placeholderString = @"Search commands...";
    _searchField.target = self;
    _searchField.action = @selector(searchFieldChanged:);
    _searchField.autoresizingMask = NSViewWidthSizable | NSViewMinYMargin;
    [view addSubview:_searchField];

    // Create split view below search field
    _splitView = [[NSSplitView alloc] initWithFrame:NSMakeRect(0, 0, 900, 550)];
    _splitView.vertical = YES;
    _splitView.dividerStyle = NSSplitViewDividerStyleThin;
    _splitView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    [view addSubview:_splitView];

    // Create left panel (topic list)
    NSScrollView* topicScrollView = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 250, 550)];
    topicScrollView.hasVerticalScroller = YES;
    topicScrollView.autohidesScrollers = YES;
    topicScrollView.borderType = NSBezelBorder;
    topicScrollView.autoresizingMask = NSViewHeightSizable;

    _topicTableView = [[NSTableView alloc] initWithFrame:topicScrollView.bounds];
    _topicTableView.delegate = self;
    _topicTableView.dataSource = self;
    _topicTableView.headerView = nil;
    _topicTableView.rowHeight = 20;

    NSTableColumn* column = [[NSTableColumn alloc] initWithIdentifier:@"topic"];
    column.width = 230;
    [_topicTableView addTableColumn:column];

    topicScrollView.documentView = _topicTableView;
    [_splitView addSubview:topicScrollView];

    // Create right panel (help text)
    NSScrollView* textScrollView = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 640, 550)];
    textScrollView.hasVerticalScroller = YES;
    textScrollView.autohidesScrollers = YES;
    textScrollView.borderType = NSBezelBorder;
    textScrollView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

    _helpTextView = [[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, 2000, 550)];
    _helpTextView.editable = NO;
    _helpTextView.selectable = YES;
    _helpTextView.font = [NSFont fontWithName:@"Menlo" size:11.0];
    _helpTextView.textContainerInset = NSMakeSize(10, 10);

    // Enable standard text operations (Copy, Select All)
    _helpTextView.usesFindBar = YES;
    _helpTextView.enabledTextCheckingTypes = 0; // Disable spell check
    _helpTextView.automaticSpellingCorrectionEnabled = NO;
    _helpTextView.automaticQuoteSubstitutionEnabled = NO;
    _helpTextView.automaticDashSubstitutionEnabled = NO;
    _helpTextView.automaticTextReplacementEnabled = NO;

    // Disable line wrapping and make text container wide
    _helpTextView.maxSize = NSMakeSize(FLT_MAX, FLT_MAX);
    _helpTextView.horizontallyResizable = YES;
    _helpTextView.verticallyResizable = YES;
    _helpTextView.textContainer.widthTracksTextView = NO;
    _helpTextView.textContainer.heightTracksTextView = NO;
    _helpTextView.textContainer.containerSize = NSMakeSize(FLT_MAX, FLT_MAX);

    textScrollView.documentView = _helpTextView;
    textScrollView.hasHorizontalScroller = YES;
    [_splitView addSubview:textScrollView];

    // Set split view position
    [_splitView setPosition:250 ofDividerAtIndex:0];

    // Display welcome message
    [self displayWelcomeMessage];
}

- (void)showHelpWindow {
    if (!_helpWindow) {
        _helpWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 900, 600)
                                                   styleMask:(NSWindowStyleMaskTitled |
                                                            NSWindowStyleMaskClosable |
                                                            NSWindowStyleMaskResizable |
                                                            NSWindowStyleMaskMiniaturizable)
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
        _helpWindow.title = @"FasterBASIC Articles & Reference";
        _helpWindow.contentViewController = self;
        _helpWindow.minSize = NSMakeSize(600, 400);
        [_helpWindow center];

        // Force split view to resize properly after window is created
        [_splitView adjustSubviews];
    }

    [_helpWindow makeKeyAndOrderFront:nil];

    // Make the text view first responder so Copy/Select All work immediately
    [_helpWindow makeFirstResponder:_helpTextView];
}

- (void)displayWelcomeMessage {
    std::ostringstream ss;
    ss << "═══════════════════════════════════════════════════════════\n";
    ss << " FASTERBASIC ARTICLES & REFERENCE\n";
    ss << "═══════════════════════════════════════════════════════════\n\n";
    ss << "Welcome to the FasterBASIC Articles & Reference!\n\n";
    ss << "This help system provides documentation for all available\n";
    ss << "BASIC articles, commands, and functions.\n\n";
    ss << "HOW TO USE:\n";
    ss << "  • Select a category from the left panel\n";
    ss << "  • Click on a command to view detailed documentation\n";
    ss << "  • Use the search field to find specific commands\n";
    ss << "  • Select text and press Cmd+C to copy examples\n\n";
    ss << "SECTIONS:\n";
    int articleCount = 0;
    int commandCount = 0;
    for (const auto& topic : _topics) {
        if (topic.isArticle) {
            articleCount++;
        } else if (!topic.commands.empty()) {
            commandCount++;
        }
    }
    ss << "  • Articles: " << articleCount << " documentation articles\n";
    ss << "  • Command Reference: " << commandCount << " command categories\n\n";
    ss << "Select a section from the left to browse.\n\n";
    ss << "═══════════════════════════════════════════════════════════\n";

    NSAttributedString* attrString = HelpContentGenerator::formatHelpText(ss.str());
    [_helpTextView.textStorage setAttributedString:attrString];
}

- (void)displayTopic:(NSString*)topic {
    // Find the topic in our list
    std::string topicStr = [topic UTF8String];

    for (const auto& t : _topics) {
        if (t.name == topicStr) {
            if (t.isArticle) {
                // Load and display article
                NSString* articlePath = [NSString stringWithUTF8String:t.articlePath.c_str()];
                NSError* error = nil;
                NSString* content = [NSString stringWithContentsOfFile:articlePath encoding:NSUTF8StringEncoding error:&error];

                if (error || !content) {
                    NSLog(@"[Help] Error loading article: %@", error);
                    return;
                }

                std::string markdown = [content UTF8String];
                std::string rendered = HelpContentGenerator::renderMarkdown(markdown);
                NSAttributedString* attrString = HelpContentGenerator::formatHelpText(rendered);
                [_helpTextView.textStorage setAttributedString:attrString];
            } else if (t.category == "articles_header" || t.category == "commands_header") {
                // Display section header info
                std::ostringstream ss;
                ss << "═══════════════════════════════════════════════════════════\n";
                ss << " " << t.name << "\n";
                ss << "═══════════════════════════════════════════════════════════\n\n";
                ss << t.description << "\n\n";
                ss << "Select a topic from the list to view its contents.\n";
                ss << "═══════════════════════════════════════════════════════════\n";
                NSAttributedString* attrString = HelpContentGenerator::formatHelpText(ss.str());
                [_helpTextView.textStorage setAttributedString:attrString];
            } else {
                // Display command category
                std::string content = HelpContentGenerator::generateCategoryOverview(t.category, _commandRegistry);
                NSAttributedString* attrString = HelpContentGenerator::formatHelpText(content);
                [_helpTextView.textStorage setAttributedString:attrString];
            }
            return;
        }
    }
}

- (void)displayCommand:(NSString*)commandName {
    std::string cmdStr = [commandName UTF8String];
    std::string content = HelpContentGenerator::generateCommandHelp(cmdStr, _commandRegistry);
    NSAttributedString* attrString = HelpContentGenerator::formatHelpText(content);
    [_helpTextView.textStorage setAttributedString:attrString];
}

- (void)searchCommands:(NSString*)searchText {
    if (searchText.length == 0) {
        [self displayWelcomeMessage];
        return;
    }

    std::string search = [[searchText lowercaseString] UTF8String];
    std::ostringstream ss;

    ss << "═══════════════════════════════════════════════════════════\n";
    ss << " SEARCH RESULTS FOR: " << search << "\n";
    ss << "═══════════════════════════════════════════════════════════\n\n";

    int foundCount = 0;

    // Search all commands
    const auto& allCommands = _commandRegistry->getAllCommands();
    for (const auto& pair : allCommands) {
        std::string cmdLower = pair.first;
        std::transform(cmdLower.begin(), cmdLower.end(), cmdLower.begin(), ::tolower);

        if (cmdLower.find(search) != std::string::npos) {
            if (pair.second.isFunction) {
                ss << "FUNCTION: " << pair.first << "()\n";
            } else {
                ss << "COMMAND: " << pair.first << "\n";
            }
            ss << "  " << pair.second.description << "\n\n";
            foundCount++;
        }
    }

    if (foundCount == 0) {
        ss << "No commands or functions found matching your search.\n\n";
    } else {
        ss << "Found " << foundCount << " matching commands/functions.\n\n";
    }

    ss << "═══════════════════════════════════════════════════════════\n";

    NSAttributedString* attrString = HelpContentGenerator::formatHelpText(ss.str());
    [_helpTextView.textStorage setAttributedString:attrString];
}

- (void)searchFieldChanged:(id)sender {
    [self searchCommands:_searchField.stringValue];
}

// =============================================================================
// NSTableViewDataSource
// =============================================================================

- (NSInteger)numberOfRowsInTableView:(NSTableView*)tableView {
    return _topics.size();
}

- (nullable id)tableView:(NSTableView*)tableView objectValueForTableColumn:(nullable NSTableColumn*)tableColumn row:(NSInteger)row {
    if (row < 0 || row >= _topics.size()) {
        return nil;
    }

    return [NSString stringWithUTF8String:_topics[row].name.c_str()];
}

// =============================================================================
// NSTableViewDelegate
// =============================================================================

- (void)tableViewSelectionDidChange:(NSNotification*)notification {
    NSInteger row = _topicTableView.selectedRow;
    if (row >= 0 && row < _topics.size()) {
        NSString* topicName = [NSString stringWithUTF8String:_topics[row].name.c_str()];
        [self displayTopic:topicName];
    }
}

- (NSView*)tableView:(NSTableView*)tableView viewForTableColumn:(NSTableColumn*)tableColumn row:(NSInteger)row {
    NSTextField* textField = [tableView makeViewWithIdentifier:@"TopicCell" owner:self];

    if (!textField) {
        textField = [[NSTextField alloc] initWithFrame:NSZeroRect];
        textField.identifier = @"TopicCell";
        textField.bordered = NO;
        textField.backgroundColor = [NSColor clearColor];
        textField.editable = NO;
        textField.selectable = NO;
    }

    if (row >= 0 && row < _topics.size()) {
        textField.stringValue = [NSString stringWithUTF8String:_topics[row].name.c_str()];
    }

    return textField;
}

@end
