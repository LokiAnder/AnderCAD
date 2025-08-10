#include "cad_ui/ThemeManager.h"
#include <QStyle>
#pragma execution_character_set("utf-8")

namespace cad_ui {

ThemeManager::ThemeManager(QObject* parent) : QObject(parent), m_currentTheme("light") {
}

void ThemeManager::SetTheme(const QString& theme) {
    if (m_currentTheme == theme) {
        return;
    }
    
    m_currentTheme = theme;
    
    if (theme == "dark") {
        ApplyDarkTheme();
    } else {
        ApplyLightTheme();
    }
}

QString ThemeManager::GetCurrentTheme() const {
    return m_currentTheme;
}

void ThemeManager::ApplyDarkTheme() {
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    QApplication::setPalette(darkPalette);
}

void ThemeManager::ApplyLightTheme() {
    QApplication::setPalette(QApplication::style()->standardPalette());
}

} // namespace cad_ui

#include "ThemeManager.moc"