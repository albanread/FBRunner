//
// HelpViewController.h
// FBRunner3 - BASIC Command Help Viewer
//
// Displays rich text help documentation extracted from the command registry.
// Provides categorized command reference with syntax highlighting and examples.
//

#import <Cocoa/Cocoa.h>
#include <string>
#include <vector>
#include <map>

namespace FasterBASIC {
namespace ModularCommands {
    class CommandRegistry;
}
}

@interface HelpViewController : NSViewController <NSTableViewDelegate, NSTableViewDataSource>

// UI Components
@property (nonatomic, strong) NSTableView* topicTableView;
@property (nonatomic, strong) NSTextView* helpTextView;
@property (nonatomic, strong) NSSearchField* searchField;
@property (nonatomic, strong) NSSplitView* splitView;

// Help content
@property (nonatomic, assign) FasterBASIC::ModularCommands::CommandRegistry* commandRegistry;

// Initialize with command registry
- (instancetype)initWithCommandRegistry:(FasterBASIC::ModularCommands::CommandRegistry*)registry;

// Show help window
- (void)showHelpWindow;

// Search functionality
- (void)searchCommands:(NSString*)searchText;

// Display specific topic
- (void)displayTopic:(NSString*)topic;
- (void)displayCommand:(NSString*)commandName;

@end

// Help topic structure
struct HelpTopic {
    std::string name;
    std::string category;
    std::vector<std::string> commands;
    std::string description;
    bool isArticle;
    std::string articlePath;
    
    HelpTopic() : isArticle(false) {}
};

// C++ Helper class for extracting documentation
class HelpContentGenerator {
public:
    static std::vector<HelpTopic> generateTopics(FasterBASIC::ModularCommands::CommandRegistry* registry);
    static std::string generateCommandHelp(const std::string& commandName,
                                          FasterBASIC::ModularCommands::CommandRegistry* registry);
    static std::string generateCategoryOverview(const std::string& category,
                                               FasterBASIC::ModularCommands::CommandRegistry* registry);
    static std::vector<HelpTopic> loadArticles();
    static std::string renderMarkdown(const std::string& markdown);
    static NSAttributedString* formatHelpText(const std::string& text);
};