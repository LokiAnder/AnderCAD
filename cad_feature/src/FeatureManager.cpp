#include "cad_feature/FeatureManager.h"
#include <algorithm>

namespace cad_feature {

FeatureManager::FeatureManager() {
}

void FeatureManager::AddFeature(const FeaturePtr& feature) {
    m_features.push_back(feature);
    NotifyFeatureAdded(feature);
}

void FeatureManager::RemoveFeature(const FeaturePtr& feature) {
    auto it = std::find(m_features.begin(), m_features.end(), feature);
    if (it != m_features.end()) {
        m_features.erase(it);
        NotifyFeatureRemoved(feature);
    }
}

void FeatureManager::ClearFeatures() {
    m_features.clear();
}

const std::vector<FeaturePtr>& FeatureManager::GetFeatures() const {
    return m_features;
}

FeaturePtr FeatureManager::GetFeatureById(int id) const {
    for (const auto& feature : m_features) {
        if (feature->GetId() == id) {
            return feature;
        }
    }
    return nullptr;
}

FeaturePtr FeatureManager::GetFeatureByName(const std::string& name) const {
    for (const auto& feature : m_features) {
        if (feature->GetName() == name) {
            return feature;
        }
    }
    return nullptr;
}

bool FeatureManager::ExecuteFeature(const FeaturePtr& feature) {
    if (!feature || !feature->IsActive()) {
        return false;
    }
    
    if (!feature->ValidateParameters()) {
        feature->SetState(FeatureState::Failed);
        return false;
    }
    
    auto shape = feature->CreateShape();
    if (shape) {
        feature->SetState(FeatureState::Executed);
        NotifyFeatureUpdated(feature);
        return true;
    } else {
        feature->SetState(FeatureState::Failed);
        return false;
    }
}

bool FeatureManager::ExecuteAllFeatures() {
    bool allSucceeded = true;
    
    for (const auto& feature : m_features) {
        if (!ExecuteFeature(feature)) {
            allSucceeded = false;
        }
    }
    
    return allSucceeded;
}

void FeatureManager::SetFeatureActive(const FeaturePtr& feature, bool active) {
    feature->SetActive(active);
    NotifyFeatureUpdated(feature);
}

void FeatureManager::SetAllFeaturesActive(bool active) {
    for (const auto& feature : m_features) {
        feature->SetActive(active);
    }
}

void FeatureManager::MoveFeatureUp(const FeaturePtr& feature) {
    int index = FindFeatureIndex(feature);
    if (index > 0) {
        std::swap(m_features[index], m_features[index - 1]);
        NotifyFeatureUpdated(feature);
    }
}

void FeatureManager::MoveFeatureDown(const FeaturePtr& feature) {
    int index = FindFeatureIndex(feature);
    if (index >= 0 && index < static_cast<int>(m_features.size()) - 1) {
        std::swap(m_features[index], m_features[index + 1]);
        NotifyFeatureUpdated(feature);
    }
}

void FeatureManager::MoveFeatureToIndex(const FeaturePtr& feature, int newIndex) {
    int currentIndex = FindFeatureIndex(feature);
    if (currentIndex >= 0 && newIndex >= 0 && newIndex < static_cast<int>(m_features.size())) {
        m_features.erase(m_features.begin() + currentIndex);
        m_features.insert(m_features.begin() + newIndex, feature);
        NotifyFeatureUpdated(feature);
    }
}

void FeatureManager::UpdateFeature(const FeaturePtr& feature) {
    ExecuteFeature(feature);
}

void FeatureManager::RebuildAllFeatures() {
    ExecuteAllFeatures();
}

int FeatureManager::GetFeatureCount() const {
    return static_cast<int>(m_features.size());
}

bool FeatureManager::IsEmpty() const {
    return m_features.empty();
}

void FeatureManager::SetFeatureAddedCallback(std::function<void(const FeaturePtr&)> callback) {
    m_featureAddedCallback = callback;
}

void FeatureManager::SetFeatureRemovedCallback(std::function<void(const FeaturePtr&)> callback) {
    m_featureRemovedCallback = callback;
}

void FeatureManager::SetFeatureUpdatedCallback(std::function<void(const FeaturePtr&)> callback) {
    m_featureUpdatedCallback = callback;
}

int FeatureManager::FindFeatureIndex(const FeaturePtr& feature) const {
    auto it = std::find(m_features.begin(), m_features.end(), feature);
    if (it != m_features.end()) {
        return static_cast<int>(std::distance(m_features.begin(), it));
    }
    return -1;
}

void FeatureManager::NotifyFeatureAdded(const FeaturePtr& feature) {
    if (m_featureAddedCallback) {
        m_featureAddedCallback(feature);
    }
}

void FeatureManager::NotifyFeatureRemoved(const FeaturePtr& feature) {
    if (m_featureRemovedCallback) {
        m_featureRemovedCallback(feature);
    }
}

void FeatureManager::NotifyFeatureUpdated(const FeaturePtr& feature) {
    if (m_featureUpdatedCallback) {
        m_featureUpdatedCallback(feature);
    }
}

} // namespace cad_feature