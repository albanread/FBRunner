//
//  EditorBridge.h
//  FBRunner3 - Editor/Shell Synchronization Bridge
//
//  Provides bidirectional synchronization between the TextEditor's TextBuffer
//  (used for screen-based editing) and the Shell's SourceDocument (used for
//  interactive BASIC programming with line numbers).
//
//  Copyright © 2024 FasterBASIC. All rights reserved.
//

#ifndef EDITOR_BRIDGE_H
#define EDITOR_BRIDGE_H

#include <memory>
#include <string>

// Forward declarations
namespace SuperTerminal {
    class TextBuffer;
}

namespace FasterBASIC {
    class SourceDocument;
    class EditorView;
    class REPLView;
}

namespace FBRunner3 {

// =============================================================================
// EditorBridge - Synchronization between TextBuffer and SourceDocument
// =============================================================================
//
// Purpose:
//   Provides two-way synchronization between the framework's TextBuffer
//   (used by TextEditor for screen-based editing) and FasterBASIC's
//   SourceDocument (used by the REPL/Shell for line-numbered BASIC programs).
//
// Usage Pattern (Mode Switching):
//   1. Editor Mode → Shell Mode:
//      - Call syncEditorToShell() to copy TextBuffer → SourceDocument
//      - Shell can now manipulate program with line numbers
//
//   2. Shell Mode → Editor Mode:
//      - Call syncShellToEditor() to copy SourceDocument → TextBuffer
//      - Editor now displays the program for screen-based editing
//
// Design Notes:
//   - This is the MVP (Minimum Viable Product) approach: sync on mode switch
//   - Future enhancement: real-time two-way binding
//   - The bridge handles line number display/hiding automatically
//   - Cursor position is preserved where possible during sync
//
// Thread Safety:
//   - Not thread-safe; must be called from main thread only
//   - Synchronization should happen during mode transitions (single-threaded)
//

class EditorBridge {
public:
    // =========================================================================
    // Construction
    // =========================================================================
    
    /// Create bridge connecting TextBuffer and SourceDocument
    /// @param textBuffer Editor's text buffer (screen-based editing)
    /// @param document Shell's source document (line-numbered BASIC)
    EditorBridge(SuperTerminal::TextBuffer* textBuffer,
                 std::shared_ptr<FasterBASIC::SourceDocument> document);
    
    /// Destructor
    ~EditorBridge();
    
    // No copy/move (holds pointers to external resources)
    EditorBridge(const EditorBridge&) = delete;
    EditorBridge& operator=(const EditorBridge&) = delete;
    EditorBridge(EditorBridge&&) = delete;
    EditorBridge& operator=(EditorBridge&&) = delete;
    
    // =========================================================================
    // Synchronization (Mode Switching)
    // =========================================================================
    
    /// Sync Editor → Shell (copy TextBuffer to SourceDocument)
    /// Call when switching from Editor Mode to Shell Mode
    /// @return true if successful
    bool syncEditorToShell();
    
    /// Sync Shell → Editor (copy SourceDocument to TextBuffer)
    /// Call when switching from Shell Mode to Editor Mode
    /// @return true if successful
    bool syncShellToEditor();
    
    // =========================================================================
    // Configuration
    // =========================================================================
    
    /// Set whether to show line numbers in editor
    /// @param show If true, editor displays BASIC line numbers as prefixes
    void setShowLineNumbers(bool show);
    
    /// Get whether line numbers are shown
    bool getShowLineNumbers() const;
    
    /// Set whether to preserve cursor position during sync
    /// @param preserve If true, attempt to keep cursor at same logical location
    void setPreserveCursor(bool preserve);
    
    /// Get whether cursor position is preserved
    bool getPreserveCursor() const;
    
    // =========================================================================
    // Status Information
    // =========================================================================
    
    /// Check if editor has unsaved changes relative to shell
    /// @return true if editor content differs from shell document
    bool hasUnsavedEditorChanges() const;
    
    /// Check if shell has unsaved changes relative to editor
    /// @return true if shell document differs from editor content
    bool hasUnsavedShellChanges() const;
    
    /// Get last sync direction
    /// @return "editor→shell", "shell→editor", or "none"
    std::string getLastSyncDirection() const;
    
    /// Get line count in editor
    size_t getEditorLineCount() const;
    
    /// Get line count in shell document
    size_t getShellLineCount() const;
    
    // =========================================================================
    // Advanced Operations
    // =========================================================================
    
    /// Force both sides to be marked as synchronized
    /// Use after external operations that modify both sides
    void markSynchronized();
    
    /// Get EditorView wrapper for direct access
    /// @return EditorView adapter (creates on first access)
    FasterBASIC::EditorView* getEditorView();
    
    /// Get REPLView wrapper for direct access
    /// @return REPLView adapter (creates on first access)
    FasterBASIC::REPLView* getREPLView();

private:
    // =========================================================================
    // Member Variables
    // =========================================================================
    
    // External resources (not owned)
    SuperTerminal::TextBuffer* m_textBuffer;
    std::shared_ptr<FasterBASIC::SourceDocument> m_document;
    
    // View adapters (created on demand)
    std::unique_ptr<FasterBASIC::EditorView> m_editorView;
    std::unique_ptr<FasterBASIC::REPLView> m_replView;
    
    // Configuration
    bool m_showLineNumbers;
    bool m_preserveCursor;
    
    // Sync state tracking
    enum class SyncDirection {
        None,
        EditorToShell,
        ShellToEditor
    };
    SyncDirection m_lastSync;
    size_t m_editorVersionAtLastSync;
    size_t m_shellVersionAtLastSync;
    
    // =========================================================================
    // Internal Helpers
    // =========================================================================
    
    /// Convert SourceDocument to plain text (with or without line numbers)
    /// @param includeLineNumbers If true, format as "10 PRINT" etc.
    /// @return Text suitable for TextBuffer
    std::string documentToEditorText(bool includeLineNumbers) const;
    
    /// Parse editor text into SourceDocument
    /// @param text Editor content (may have line numbers as text)
    /// @return true if successful
    bool editorTextToDocument(const std::string& text);
    
    /// Extract cursor position from TextBuffer
    /// @param line Output: cursor line
    /// @param column Output: cursor column
    void getEditorCursorPosition(size_t& line, size_t& column) const;
    
    /// Set cursor position in TextBuffer
    /// @param line Cursor line
    /// @param column Cursor column
    void setEditorCursorPosition(size_t line, size_t column);
    
    /// Map editor line to document line (handling line number prefixes)
    /// @param editorLine Line number in editor
    /// @return Corresponding document line (or editorLine if no mapping)
    size_t mapEditorLineToDocument(size_t editorLine) const;
    
    /// Map document line to editor line (handling line number prefixes)
    /// @param docLine Line number in document
    /// @return Corresponding editor line (or docLine if no mapping)
    size_t mapDocumentLineToEditor(size_t docLine) const;
    
    /// Check if editor text looks like it has line numbers
    /// @param text Editor content
    /// @return true if text appears to have BASIC line numbers
    bool hasLineNumberPrefixes(const std::string& text) const;
    
    /// Strip line number prefix from a line
    /// @param line Line with potential "10 " prefix
    /// @return Line without prefix, or original if no prefix found
    std::string stripLineNumberPrefix(const std::string& line) const;
};

} // namespace FBRunner3

#endif // EDITOR_BRIDGE_H