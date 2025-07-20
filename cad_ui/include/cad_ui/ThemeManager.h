#pragma once

#include <QObject>
#include <QApplication>
#include <QPalette>

namespace cad_ui {

class ThemeManager : public QObject {
    Q_OBJECT

public:
    explicit ThemeManager(QObject* parent = nullptr);
    ~ThemeManager() = default;

    void SetTheme(const QString& theme);
    QString GetCurrentTheme() const;

private:
    QString m_currentTheme;
    
    void ApplyDarkTheme();
    void ApplyLightTheme();
};

} // namespace cad_ui