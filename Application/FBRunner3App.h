//
// FBRunner3App.h
// FasterBASIC - Main Application Class
//
// The main application class for FBRunner3.
// Manages the BASIC runtime, editor, and interactive shell.
//

#ifndef FBRUNNER3APP_H
#define FBRUNNER3APP_H

#import "../BaseRunner.h"
#include "Debug/Logger.h"
#include <string>

/**
 * @brief Main application class for FBRunner3
 * 
 * FBRunner3App extends BaseRunner to provide:
 * - BASIC script execution via FasterBASICT compiler
 * - Integrated text editor with syntax highlighting
 * - Interactive shell/REPL mode
 * - Cart system integration
 */
@interface FBRunner3App : BaseRunner

// Properties
@property (assign) BOOL scriptRunning;
@property (nonatomic, assign) std::string preferredWindowSize;

// Runtime text management methods
- (void)initializeRuntimeTextState;
- (void)clearRuntimeTextState;
- (void)runtimePrintText:(const std::string&)text;
- (void)addTextToCurrentRuntimeLine:(const std::string&)text;
- (void)advanceRuntimeCursor;
- (void)setRuntimeCursorPosition:(int)x y:(int)y;
- (void)scrollRuntimeText:(int)lines;
- (void)updateRuntimeDisplay;
- (void)handleRuntimeKeyPress:(int)keyCode;
- (void)updateRuntimeMode;

// Lua debug hook methods for script interruption
- (void)installLuaInterruptHook;
- (void)removeLuaInterruptHook;

@end

#endif // FBRUNNER3APP_H