//
//  EditorView.h
//  FasterBASIC Framework - Editor View Adapter
//
//  Adapter class that provides editor-oriented interface to SourceDocument.
//  Designed for screen-based text editing with cursor positioning.
//
//  Copyright © 2024 FasterBASIC. All rights reserved.
//

#ifndef EDITOR_VIEW_H
#define EDITOR_VIEW_H

#include "../FasterBASICT/src/SourceDocument.h"
#include <memory>
#include <vector>
#include <string>

namespace FasterBASIC {

// =============================================================================
// EditorView - Editor-oriented view of SourceDocument
// =============================================================================

class EditorView {
public:
    // =========================================================================
    // Construction
    // =========================================================================
    
    /// Create view wrapping a SourceDocument
    explicit EditorView(std::shared_ptr<SourceDocument> document);
    
    /// Destructor
    ~EditorView();
    
    // No copy (shared pointer semantics)
    EditorView(const EditorView&) = delete;
    EditorView& operator=(const EditorView&) = delete;
    
    // Move allowed
    EditorView(EditorView&&) noexcept = default;
    EditorView& operator=(EditorView&&) noexcept = default;
    
    // =========================================================================
    // Line Operations (by index)
    // =========================================================================
    
    /// Get number of lines
    size_t getLineCount() const;
    
    /// Get line text by index
    /// @param index 0-based line index
    /// @return Line text (empty if out of bounds)
    std::string getLine(size_t index) const;
    
    /// Set line text by index
    /// @param index 0-based line index
    /// @param text New line content
    /// @return true if successful
    bool setLine(size_t index, const std::string& text);
    
    /// Get line length
    /// @param index 0-based line index
    /// @return Length in characters (0 if invalid)
    size_t getLineLength(size_t index) const;
    
    /// Insert new line at index
    /// @param index Position to insert (0-based)
    /// @param text Line content
    void insertLine(size_t index, const std::string& text);
    
    /// Delete line at index
    /// @param index 0-based line index
    /// @return true if successful
    bool deleteLine(size_t index);
    
    /// Split line at column position
    /// @param index Line index
    /// @param column Column position (0-based)
    /// @return true if successful
    bool splitLine(size_t index, size_t column);
    
    /// Join line with next line
    /// @param index Line index
    /// @return true if successful
    bool joinLines(size_t index);
    
    // =========================================================================
    // Character Operations
    // =========================================================================
    
    /// Insert character at position
    /// @param line Line index (0-based)
    /// @param column Column position (0-based)
    /// @param ch Character to insert (UTF-32)
    /// @return true if successful
    bool insertChar(size_t line, size_t column, char32_t ch);
    
    /// Delete character at position
    /// @param line Line index (0-based)
    /// @param column Column position (0-based)
    /// @return true if successful
    bool deleteChar(size_t line, size_t column);
    
    /// Insert text at position
    /// @param line Line index (0-based)
    /// @param column Column position (0-based)
    /// @param text Text to insert (may contain newlines)
    /// @return true if successful
    bool insertText(size_t line, size_t column, const std::string& text);
    
    /// Get character at position
    /// @param line Line index (0-based)
    /// @param column Column position (0-based)
    /// @return Character (0 if out of bounds)
    char32_t getChar(size_t line, size_t column) const;
    
    // =========================================================================
    // Selection Operations
    // =========================================================================
    
    /// Get text in range
    /// @param startLine Start line index
    /// @param startCol Start column
    /// @param endLine End line index
    /// @param endCol End column
    /// @return Selected text
    std::string getSelection(size_t startLine, size_t startCol,
                            size_t endLine, size_t endCol) const;
    
    /// Delete selection
    /// @param startLine Start line index
    /// @param startCol Start column
    /// @param endLine End line index
    /// @param endCol End column
    /// @return Deleted text
    std::string deleteSelection(size_t startLine, size_t startCol,
                               size_t endLine, size_t endCol);
    
    /// Replace selection with text
    /// @param startLine Start line index
    /// @param startCol Start column
    /// @param endLine End line index
    /// @param endCol End column
    /// @param text Replacement text
    /// @return true if successful
    bool replaceSelection(size_t startLine, size_t startCol,
                         size_t endLine, size_t endCol,
                         const std::string& text);
    
    // =========================================================================
    // Document Operations
    // =========================================================================
    
    /// Set entire document text
    /// @param text Full document content (with newlines)
    void setText(const std::string& text);
    
    /// Get entire document text
    /// @return Full document content
    std::string getText() const;
    
    /// Clear document
    void clear();
    
    /// Check if document is empty
    bool isEmpty() const;
    
    // =========================================================================
    // File Operations
    // =========================================================================
    
    /// Load from file
    /// @param filename File path
    /// @return true if successful
    bool loadFromFile(const std::string& filename);
    
    /// Save to file
    /// @param filename File path
    /// @return true if successful
    bool saveToFile(const std::string& filename);
    
    /// Get filename
    std::string getFilename() const;
    
    /// Set filename
    void setFilename(const std::string& filename);
    
    // =========================================================================
    // Undo/Redo
    // =========================================================================
    
    /// Push undo state
    void pushUndoState();
    
    /// Undo
    /// @return true if undo was performed
    bool undo();
    
    /// Redo
    /// @return true if redo was performed
    bool redo();
    
    /// Check if undo available
    bool canUndo() const;
    
    /// Check if redo available
    bool canRedo() const;
    
    // =========================================================================
    // Dirty State
    // =========================================================================
    
    /// Check if modified
    bool isModified() const;
    
    /// Mark as saved
    void markSaved();
    
    /// Mark as modified
    void markModified();
    
    // =========================================================================
    // Line Numbers (for display)
    // =========================================================================
    
    /// Check if document has BASIC line numbers
    bool hasLineNumbers() const;
    
    /// Get BASIC line number for editor line
    /// @param index Editor line index
    /// @return BASIC line number (0 if unnumbered)
    int getLineNumber(size_t index) const;
    
    /// Show/hide line numbers in editor
    /// @param show True to show line numbers
    void setShowLineNumbers(bool show);
    
    /// Check if line numbers should be shown
    bool shouldShowLineNumbers() const;
    
    // =========================================================================
    // Position Validation
    // =========================================================================
    
    /// Check if position is valid
    /// @param line Line index
    /// @param column Column position
    /// @return true if valid
    bool isValidPosition(size_t line, size_t column) const;
    
    /// Clamp position to valid range
    /// @param line Line index (will be clamped)
    /// @param column Column position (will be clamped)
    void clampPosition(size_t& line, size_t& column) const;
    
    // =========================================================================
    // Search
    // =========================================================================
    
    /// Search result
    struct SearchResult {
        size_t line;
        size_t column;
        size_t length;
        
        SearchResult(size_t l, size_t c, size_t len)
            : line(l), column(c), length(len) {}
    };
    
    /// Find all occurrences of pattern
    /// @param pattern Search pattern
    /// @param caseSensitive Case-sensitive search
    /// @return Vector of search results
    std::vector<SearchResult> find(const std::string& pattern, 
                                   bool caseSensitive = true) const;
    
    /// Replace all occurrences
    /// @param pattern Search pattern
    /// @param replacement Replacement text
    /// @return Number of replacements
    size_t replaceAll(const std::string& pattern, 
                     const std::string& replacement);
    
    // =========================================================================
    // Direct Document Access
    // =========================================================================
    
    /// Get underlying document (const)
    const SourceDocument& getDocument() const { return *m_document; }
    
    /// Get underlying document (mutable)
    SourceDocument& getDocument() { return *m_document; }
    
    /// Get shared pointer to document
    std::shared_ptr<SourceDocument> getDocumentPtr() { return m_document; }

private:
    // =========================================================================
    // Member Variables
    // =========================================================================
    
    std::shared_ptr<SourceDocument> m_document;     // Underlying document
    bool m_showLineNumbers;                         // Display BASIC line numbers
};

} // namespace FasterBASIC

#endif // EDITOR_VIEW_H