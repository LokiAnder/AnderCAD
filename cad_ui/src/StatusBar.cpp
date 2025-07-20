#include "cad_ui/StatusBar.h"

namespace cad_ui {

StatusBar::StatusBar(QWidget* parent) : QStatusBar(parent) {
    setObjectName("StatusBar");
}

} // namespace cad_ui

#include "StatusBar.moc"