#include "cad_feature/LivePreview.h"

namespace cad_feature {

LivePreview::LivePreview(QObject* parent) : QObject(parent), m_previewActive(false), m_updateDelay(500) {
    m_updateTimer = new QTimer(this);
    m_updateTimer->setSingleShot(true);
    
    connect(m_updateTimer, &QTimer::timeout, this, &LivePreview::OnUpdateTimer);
}

void LivePreview::SetFeature(const FeaturePtr& feature) {
    m_feature = feature;
    
    if (m_previewActive) {
        UpdatePreview();
    }
}

const FeaturePtr& LivePreview::GetFeature() const {
    return m_feature;
}

void LivePreview::StartPreview() {
    m_previewActive = true;
    UpdatePreview();
}

void LivePreview::StopPreview() {
    m_previewActive = false;
    m_updateTimer->stop();
    ClearPreviewShape();
}

void LivePreview::UpdatePreview() {
    if (!m_previewActive || !m_feature) {
        return;
    }
    
    // Restart the timer to delay the update
    m_updateTimer->start(m_updateDelay);
}

bool LivePreview::IsPreviewActive() const {
    return m_previewActive;
}

void LivePreview::SetPreviewActive(bool active) {
    if (active) {
        StartPreview();
    } else {
        StopPreview();
    }
}

void LivePreview::SetUpdateDelay(int milliseconds) {
    m_updateDelay = milliseconds;
}

int LivePreview::GetUpdateDelay() const {
    return m_updateDelay;
}

void LivePreview::SetPreviewUpdateCallback(std::function<void(const cad_core::ShapePtr&)> callback) {
    m_previewUpdateCallback = callback;
}

void LivePreview::SetPreviewClearCallback(std::function<void()> callback) {
    m_previewClearCallback = callback;
}

void LivePreview::OnUpdateTimer() {
    if (m_previewActive && m_feature) {
        UpdatePreviewShape();
    }
}

void LivePreview::UpdatePreviewShape() {
    if (!m_feature) {
        return;
    }
    
    // Set feature state to previewing
    m_feature->SetState(FeatureState::Previewing);
    
    // Create preview shape
    auto previewShape = m_feature->CreatePreviewShape();
    
    // Notify callback
    if (m_previewUpdateCallback) {
        m_previewUpdateCallback(previewShape);
    }
}

void LivePreview::ClearPreviewShape() {
    if (m_previewClearCallback) {
        m_previewClearCallback();
    }
}

} // namespace cad_feature

#include "LivePreview.moc"