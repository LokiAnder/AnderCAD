#pragma once

#include "ICommand.h"
#include <vector>
#include <memory>

namespace cad_core {

class CommandManager {
public:
    CommandManager();
    ~CommandManager() = default;

    bool ExecuteCommand(CommandPtr command);
    bool Undo();
    bool Redo();
    
    void Clear();
    bool CanUndo() const;
    bool CanRedo() const;
    
    const char* GetUndoCommandName() const;
    const char* GetRedoCommandName() const;

private:
    std::vector<CommandPtr> m_commands;
    int m_currentIndex;
};

} // namespace cad_core