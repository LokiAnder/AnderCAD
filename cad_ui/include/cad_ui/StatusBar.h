#pragma once

#include <QStatusBar>

namespace cad_ui {

class StatusBar : public QStatusBar {
    Q_OBJECT

public:
    explicit StatusBar(QWidget* parent = nullptr);
    ~StatusBar() = default;
};

} // namespace cad_ui