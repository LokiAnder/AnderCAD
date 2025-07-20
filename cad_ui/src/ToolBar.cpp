#include "cad_ui/ToolBar.h"

namespace cad_ui {

ToolBar::ToolBar(QWidget* parent) : QToolBar(parent) {
    setObjectName("ToolBar");
}

} // namespace cad_ui

#include "ToolBar.moc"