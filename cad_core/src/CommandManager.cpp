#include "cad_core/CommandManager.h"

namespace cad_core {

CommandManager::CommandManager() : m_currentIndex(-1) {
}

bool CommandManager::ExecuteCommand(CommandPtr command) {
    if (!command || !command->Execute()) {
        return false;
    }
    
    // Remove commands after current index (for redo functionality)
    if (m_currentIndex + 1 < static_cast<int>(m_commands.size())) {
        m_commands.erase(m_commands.begin() + m_currentIndex + 1, m_commands.end());
    }
    
    m_commands.push_back(command);
    m_currentIndex++;
    
    return true;
}

bool CommandManager::Undo() {
    if (!CanUndo()) {
        return false;
    }
    
    bool result = m_commands[m_currentIndex]->Undo();
    if (result) {
        m_currentIndex--;
    }
    
    return result;
}

bool CommandManager::Redo() {
    if (!CanRedo()) {
        return false;
    }
    
    m_currentIndex++;
    bool result = m_commands[m_currentIndex]->Redo();
    if (!result) {
        m_currentIndex--;
    }
    
    return result;
}

void CommandManager::Clear() {
    m_commands.clear();
    m_currentIndex = -1;
}

bool CommandManager::CanUndo() const {
    return m_currentIndex >= 0;
}

bool CommandManager::CanRedo() const {
    return m_currentIndex + 1 < static_cast<int>(m_commands.size());
}

const char* CommandManager::GetUndoCommandName() const {
    if (CanUndo()) {
        return m_commands[m_currentIndex]->GetName();
    }
    return nullptr;
}

const char* CommandManager::GetRedoCommandName() const {
    if (CanRedo()) {
        return m_commands[m_currentIndex + 1]->GetName();
    }
    return nullptr;
}

} // namespace cad_core