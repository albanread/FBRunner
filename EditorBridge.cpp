//
//  EditorBridge.cpp
//  FBRunner3 - Editor/Shell Synchronization Bridge
//
//  Implementation of bidirectional synchronization between TextEditor's
//  TextBuffer and Shell's SourceDocument.
//
//  Copyright © 2024 FasterBASIC. All rights reserved.
//

#include "EditorBridge.h"
#include "Editor/TextBuffer.h"
#include "EditorView.h"
#include "../FasterBASICT/src/SourceDocument.h"
#include "../FasterBASICT/shell/REPLView.h"
#include <sstream>
#include <regex>
#include <algorithm>

namespace FBRunner3 {

// =============================================================================
// Construction
// =============================================================================

EditorBridge::EditorBridge(SuperTerminal::TextBuffer* textBuffer,
                           std::shared_ptr<FasterBASIC::SourceDocument> document)
    : m_textBuffer(textBuffer)
    , m_document(document)
    , m_editorView(nullptr)
    , m_replView(nullptr)
    , m_showLineNumbers(true)
    , m_preserveCursor(true)
    , m_lastSync(SyncDirection::None)
    , m_editorVersionAtLastSync(0)
    , m_shellVersionAtLastSync(0)
{
    if (!m_textBuffer) {
        throw std::invalid_argument("EditorBridge: textBuffer cannot be null");
    }
    if (!m_document) {
        throw std::invalid_argument("EditorBridge: document cannot be null");
    }
}

EditorBridge::~EditorBridge() = default;

// =============================================================================
// Synchronization (Mode Switching)
// =============================================================================

bool EditorBridge::syncEditorToShell() {
    if (!m_textBuffer || !m_document) {
        return false;
    }
    
    // Get text from editor
    std::string editorText = m_textBuffer->getText();
    
    // Parse into document
    if (!editorTextToDocument(editorText)) {
        return false;
    }
    
    // Update sync tracking
    m_lastSync = SyncDirection::EditorToShell;
    m_shellVersionAtLastSync = m_document->getVersion();
    
    return true;
}

bool EditorBridge::syncShellToEditor() {
    if (!m_textBuffer || !m_document) {
        return false;
    }
    
    // Store cursor position if requested
    size_t cursorLine = 0;
    size_t cursorColumn = 0;
    if (m_preserveCursor) {
        getEditorCursorPosition(cursorLine, cursorColumn);
    }
    
    // Convert document to editor text
    std::string editorText = documentToEditorText(m_showLineNumbers);
    
    // Set text in editor
    m_textBuffer->setText(editorText);
    
    // Restore cursor position if requested
    if (m_preserveCursor) {
        // Map cursor position from old to new content
        // For simplicity, just clamp to valid range
        size_t lineCount = m_textBuffer->getLineCount();
        if (cursorLine >= lineCount) {
            cursorLine = lineCount > 0 ? lineCount - 1 : 0;
        }
        setEditorCursorPosition(cursorLine, cursorColumn);
    }
    
    // Update sync tracking
    m_lastSync = SyncDirection::ShellToEditor;
    m_editorVersionAtLastSync = m_document->getVersion();
    
    return true;
}

// =============================================================================
// Configuration
// =============================================================================

void EditorBridge::setShowLineNumbers(bool show) {
    m_showLineNumbers = show;
}

bool EditorBridge::getShowLineNumbers() const {
    return m_showLineNumbers;
}

void EditorBridge::setPreserveCursor(bool preserve) {
    m_preserveCursor = preserve;
}

bool EditorBridge::getPreserveCursor() const {
    return m_preserveCursor;
}

// =============================================================================
// Status Information
// =============================================================================

bool EditorBridge::hasUnsavedEditorChanges() const {
    // Check if editor is dirty (TextBuffer tracks this)
    if (m_textBuffer && m_textBuffer->isDirty()) {
        return true;
    }
    return false;
}

bool EditorBridge::hasUnsavedShellChanges() const {
    // Check if document version changed since last sync
    if (m_document && m_lastSync == SyncDirection::ShellToEditor) {
        return m_document->getVersion() != m_editorVersionAtLastSync;
    }
    return false;
}

std::string EditorBridge::getLastSyncDirection() const {
    switch (m_lastSync) {
        case SyncDirection::EditorToShell:
            return "editor→shell";
        case SyncDirection::ShellToEditor:
            return "shell→editor";
        case SyncDirection::None:
        default:
            return "none";
    }
}

size_t EditorBridge::getEditorLineCount() const {
    return m_textBuffer ? m_textBuffer->getLineCount() : 0;
}

size_t EditorBridge::getShellLineCount() const {
    return m_document ? m_document->getLineCount() : 0;
}

// =============================================================================
// Advanced Operations
// =============================================================================

void EditorBridge::markSynchronized() {
    if (m_document) {
        m_editorVersionAtLastSync = m_document->getVersion();
        m_shellVersionAtLastSync = m_document->getVersion();
    }
    m_lastSync = SyncDirection::None;
}

FasterBASIC::EditorView* EditorBridge::getEditorView() {
    if (!m_editorView && m_document) {
        m_editorView = std::make_unique<FasterBASIC::EditorView>(m_document);
    }
    return m_editorView.get();
}

FasterBASIC::REPLView* EditorBridge::getREPLView() {
    if (!m_replView && m_document) {
        m_replView = std::make_unique<FasterBASIC::REPLView>(m_document);
    }
    return m_replView.get();
}

// =============================================================================
// Internal Helpers
// =============================================================================

std::string EditorBridge::documentToEditorText(bool includeLineNumbers) const {
    if (!m_document || m_document->getLineCount() == 0) {
        return "";
    }
    
    std::ostringstream oss;
    
    for (size_t i = 0; i < m_document->getLineCount(); ++i) {
        const auto& line = m_document->getLineByIndex(i);
        
        if (includeLineNumbers && line.lineNumber > 0) {
            // Format with line number prefix: "10 PRINT"
            oss << line.lineNumber << " " << line.text;
        } else {
            // Plain text without line numbers
            oss << line.text;
        }
        
        // Add newline except for last line (TextBuffer handles this)
        if (i < m_document->getLineCount() - 1) {
            oss << "\n";
        }
    }
    
    return oss.str();
}

bool EditorBridge::editorTextToDocument(const std::string& text) {
    if (!m_document) {
        return false;
    }
    
    // Clear document
    m_document->clear();
    
    if (text.empty()) {
        return true;
    }
    
    // Split into lines
    std::vector<std::string> lines;
    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
        // Remove carriage returns if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    
    // Handle case where text doesn't end with newline
    if (!text.empty() && text.back() != '\n' && text.back() != '\r') {
        // The last line was already added by getline
    }
    
    // Parse lines and add to document
    std::regex lineNumberPattern(R"(^\s*(\d+)\s+(.*)$)");
    std::smatch match;
    
    for (const auto& lineText : lines) {
        if (std::regex_match(lineText, match, lineNumberPattern)) {
            // Line has number prefix: "10 PRINT"
            int lineNumber = std::stoi(match[1].str());
            std::string code = match[2].str();
            m_document->setLineByNumber(lineNumber, code);
        } else {
            // Line without number: treat as unnumbered or auto-number
            // For now, insert as unnumbered (lineNumber = 0)
            m_document->insertLineAtIndex(m_document->getLineCount(), lineText, 0);
        }
    }
    
    return true;
}

void EditorBridge::getEditorCursorPosition(size_t& line, size_t& column) const {
    // TextBuffer doesn't store cursor (it's in Cursor object)
    // For now, default to (0, 0)
    // This would need to be enhanced to accept a Cursor* parameter
    line = 0;
    column = 0;
}

void EditorBridge::setEditorCursorPosition(size_t line, size_t column) {
    // TextBuffer doesn't manage cursor directly
    // The TextEditor/Cursor manages this
    // This method is a placeholder for future integration
    (void)line;
    (void)column;
}

size_t EditorBridge::mapEditorLineToDocument(size_t editorLine) const {
    // Simple 1:1 mapping for now
    // Could be enhanced to handle line number display offset
    return editorLine;
}

size_t EditorBridge::mapDocumentLineToEditor(size_t docLine) const {
    // Simple 1:1 mapping for now
    return docLine;
}

bool EditorBridge::hasLineNumberPrefixes(const std::string& text) const {
    if (text.empty()) {
        return false;
    }
    
    // Check first few lines for line number pattern
    std::istringstream iss(text);
    std::string line;
    int linesChecked = 0;
    int linesWithNumbers = 0;
    const int maxLinesToCheck = 5;
    
    std::regex lineNumberPattern(R"(^\s*\d+\s+.*)");
    
    while (std::getline(iss, line) && linesChecked < maxLinesToCheck) {
        linesChecked++;
        
        // Skip empty lines
        if (line.empty() || line.find_first_not_of(" \t\r") == std::string::npos) {
            continue;
        }
        
        if (std::regex_match(line, lineNumberPattern)) {
            linesWithNumbers++;
        }
    }
    
    // If most lines have numbers, assume line-numbered format
    return linesWithNumbers > 0 && linesWithNumbers >= (linesChecked / 2);
}

std::string EditorBridge::stripLineNumberPrefix(const std::string& line) const {
    std::regex lineNumberPattern(R"(^\s*\d+\s+(.*)$)");
    std::smatch match;
    
    if (std::regex_match(line, match, lineNumberPattern)) {
        return match[1].str();
    }
    
    return line;
}

} // namespace FBRunner3