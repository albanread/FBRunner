//
//  EditorView.cpp
//  FasterBASIC Framework - Editor View Adapter
//
//  Implementation of editor-oriented view adapter for SourceDocument.
//
//  Copyright © 2024 FasterBASIC. All rights reserved.
//

#include "EditorView.h"

namespace FasterBASIC {

// =============================================================================
// Construction
// =============================================================================

EditorView::EditorView(std::shared_ptr<SourceDocument> document)
    : m_document(document)
    , m_showLineNumbers(false)
{
    if (!m_document) {
        m_document = std::make_shared<SourceDocument>();
    }
}

EditorView::~EditorView() = default;

// =============================================================================
// Line Operations (by index)
// =============================================================================

size_t EditorView::getLineCount() const {
    return m_document->getLineCount();
}

std::string EditorView::getLine(size_t index) const {
    if (index >= m_document->getLineCount()) {
        return "";
    }
    return m_document->getLineByIndex(index).text;
}

bool EditorView::setLine(size_t index, const std::string& text) {
    return m_document->replaceLineAtIndex(index, text);
}

size_t EditorView::getLineLength(size_t index) const {
    if (index >= m_document->getLineCount()) {
        return 0;
    }
    return m_document->getLineByIndex(index).text.length();
}

void EditorView::insertLine(size_t index, const std::string& text) {
    m_document->insertLineAtIndex(index, text, 0);
}

bool EditorView::deleteLine(size_t index) {
    return m_document->deleteLineAtIndex(index);
}

bool EditorView::splitLine(size_t index, size_t column) {
    return m_document->splitLine(index, column);
}

bool EditorView::joinLines(size_t index) {
    return m_document->joinWithNext(index);
}

// =============================================================================
// Character Operations
// =============================================================================

bool EditorView::insertChar(size_t line, size_t column, char32_t ch) {
    return m_document->insertChar(line, column, ch);
}

bool EditorView::deleteChar(size_t line, size_t column) {
    return m_document->deleteChar(line, column);
}

bool EditorView::insertText(size_t line, size_t column, const std::string& text) {
    return m_document->insertText(line, column, text);
}

char32_t EditorView::getChar(size_t line, size_t column) const {
    return m_document->getChar(line, column);
}

// =============================================================================
// Selection Operations
// =============================================================================

std::string EditorView::getSelection(size_t startLine, size_t startCol,
                                     size_t endLine, size_t endCol) const {
    return m_document->getTextRange(startLine, startCol, endLine, endCol);
}

std::string EditorView::deleteSelection(size_t startLine, size_t startCol,
                                        size_t endLine, size_t endCol) {
    return m_document->deleteRange(startLine, startCol, endLine, endCol);
}

bool EditorView::replaceSelection(size_t startLine, size_t startCol,
                                  size_t endLine, size_t endCol,
                                  const std::string& text) {
    // Delete selection first
    m_document->deleteRange(startLine, startCol, endLine, endCol);
    
    // Insert replacement text
    return m_document->insertText(startLine, startCol, text);
}

// =============================================================================
// Document Operations
// =============================================================================

void EditorView::setText(const std::string& text) {
    m_document->setText(text);
}

std::string EditorView::getText() const {
    return m_document->getText();
}

void EditorView::clear() {
    m_document->clear();
}

bool EditorView::isEmpty() const {
    return m_document->isEmpty();
}

// =============================================================================
// File Operations
// =============================================================================

bool EditorView::loadFromFile(const std::string& filename) {
    bool success = m_document->loadFromFile(filename);
    if (success) {
        m_document->setFilename(filename);
        m_document->markClean();
    }
    return success;
}

bool EditorView::saveToFile(const std::string& filename) {
    bool success = m_document->saveToFile(filename);
    if (success) {
        m_document->setFilename(filename);
        m_document->markClean();
    }
    return success;
}

std::string EditorView::getFilename() const {
    return m_document->getFilename();
}

void EditorView::setFilename(const std::string& filename) {
    m_document->setFilename(filename);
}

// =============================================================================
// Undo/Redo
// =============================================================================

void EditorView::pushUndoState() {
    m_document->pushUndoState();
}

bool EditorView::undo() {
    return m_document->undo();
}

bool EditorView::redo() {
    return m_document->redo();
}

bool EditorView::canUndo() const {
    return m_document->canUndo();
}

bool EditorView::canRedo() const {
    return m_document->canRedo();
}

// =============================================================================
// Dirty State
// =============================================================================

bool EditorView::isModified() const {
    return m_document->isDirty();
}

void EditorView::markSaved() {
    m_document->markClean();
}

void EditorView::markModified() {
    m_document->markDirty();
}

// =============================================================================
// Line Numbers (for display)
// =============================================================================

bool EditorView::hasLineNumbers() const {
    return m_document->hasLineNumbers();
}

int EditorView::getLineNumber(size_t index) const {
    if (index >= m_document->getLineCount()) {
        return 0;
    }
    return m_document->getLineByIndex(index).lineNumber;
}

void EditorView::setShowLineNumbers(bool show) {
    m_showLineNumbers = show;
}

bool EditorView::shouldShowLineNumbers() const {
    return m_showLineNumbers;
}

// =============================================================================
// Position Validation
// =============================================================================

bool EditorView::isValidPosition(size_t line, size_t column) const {
    return m_document->isValidPosition(line, column);
}

void EditorView::clampPosition(size_t& line, size_t& column) const {
    m_document->clampPosition(line, column);
}

// =============================================================================
// Search
// =============================================================================

std::vector<EditorView::SearchResult> EditorView::find(const std::string& pattern, 
                                                        bool caseSensitive) const {
    std::vector<SearchResult> results;
    
    auto docResults = m_document->find(pattern, caseSensitive);
    results.reserve(docResults.size());
    
    for (const auto& r : docResults) {
        results.emplace_back(r.lineIndex, r.column, r.length);
    }
    
    return results;
}

size_t EditorView::replaceAll(const std::string& pattern, 
                              const std::string& replacement) {
    return m_document->replaceAll(pattern, replacement);
}

} // namespace FasterBASIC