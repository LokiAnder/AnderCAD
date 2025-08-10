#include "cad_ui/StatusBar.h"
#include <QString>
#pragma execution_character_set("utf-8")

namespace cad_ui {

StatusBar::StatusBar(QWidget* parent) : QStatusBar(parent), m_mousePositionLabel(nullptr) {
    setObjectName("StatusBar");
    setupMousePositionDisplay();
}

void StatusBar::setupMousePositionDisplay() {
    // 创建鼠标位置显示标签
    m_mousePositionLabel = new QLabel("鼠标位置: (0, 0)");
    m_mousePositionLabel->setObjectName("MousePositionLabel");
    m_mousePositionLabel->setMinimumWidth(200);
    m_mousePositionLabel->setStyleSheet("QLabel { padding: 2px 8px; border: 1px solid #ccc; border-radius: 3px; background: #f8f8f8; }");
    
    // 将标签添加到状态栏右侧（永久显示）
    addPermanentWidget(m_mousePositionLabel);
    
    // 初始显示
    updateMousePosition2D(0, 0);
}

void StatusBar::updateMousePosition(double x, double y, double z) {
    if (m_mousePositionLabel) {
        QString posText = QString("3D位置: (%1, %2, %3)")
            .arg(x, 0, 'f', 2)
            .arg(y, 0, 'f', 2)
            .arg(z, 0, 'f', 2);
        m_mousePositionLabel->setText(posText);
    }
}

void StatusBar::updateMousePosition2D(int screenX, int screenY) {
    if (m_mousePositionLabel) {
        QString posText = QString("鼠标位置: (%1, %2)")
            .arg(screenX)
            .arg(screenY);
        m_mousePositionLabel->setText(posText);
    }
}

} // namespace cad_ui

#include "StatusBar.moc"