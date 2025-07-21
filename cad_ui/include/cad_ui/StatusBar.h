#pragma once

#include <QStatusBar>
#include <QLabel>

namespace cad_ui {

class StatusBar : public QStatusBar {
    Q_OBJECT

public:
    explicit StatusBar(QWidget* parent = nullptr);
    ~StatusBar() = default;
    
    // 更新鼠标位置显示
    void updateMousePosition(double x, double y, double z);
    void updateMousePosition2D(int screenX, int screenY);

private:
    QLabel* m_mousePositionLabel;
    
    void setupMousePositionDisplay();
};

} // namespace cad_ui