#pragma once

#include "Feature.h"
#include <QObject>
#include <QTimer>
#include <functional>

namespace cad_feature {

class LivePreview : public QObject {
    Q_OBJECT

public:
    explicit LivePreview(QObject* parent = nullptr);
    ~LivePreview() = default;

    void SetFeature(const FeaturePtr& feature);
    const FeaturePtr& GetFeature() const;
    
    void StartPreview();
    void StopPreview();
    void UpdatePreview();
    
    bool IsPreviewActive() const;
    void SetPreviewActive(bool active);
    
    void SetUpdateDelay(int milliseconds);
    int GetUpdateDelay() const;
    
    // Callbacks
    void SetPreviewUpdateCallback(std::function<void(const cad_core::ShapePtr&)> callback);
    void SetPreviewClearCallback(std::function<void()> callback);

private slots:
    void OnUpdateTimer();

private:
    FeaturePtr m_feature;
    QTimer* m_updateTimer;
    bool m_previewActive;
    int m_updateDelay;
    
    std::function<void(const cad_core::ShapePtr&)> m_previewUpdateCallback;
    std::function<void()> m_previewClearCallback;
    
    void UpdatePreviewShape();
    void ClearPreviewShape();
};

} // namespace cad_feature