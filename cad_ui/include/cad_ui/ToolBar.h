#pragma once

#include <QToolBar>

namespace cad_ui {

class ToolBar : public QToolBar {
    Q_OBJECT

public:
    explicit ToolBar(QWidget* parent = nullptr);
    ~ToolBar() = default;
};

} // namespace cad_ui