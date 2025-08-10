#include "cad_ui/ToolBar.h"
#pragma execution_character_set("utf-8")

namespace cad_ui {

ToolBar::ToolBar(QWidget* parent) : QToolBar(parent) {
    setObjectName("ToolBar");
}

} // namespace cad_ui

#include "ToolBar.moc"