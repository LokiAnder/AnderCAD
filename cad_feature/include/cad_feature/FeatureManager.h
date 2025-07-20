#pragma once

#include "Feature.h"
#include <vector>
#include <memory>
#include <string>

namespace cad_feature {

class FeatureManager {
public:
    FeatureManager();
    ~FeatureManager() = default;

    // 特征管理
    void AddFeature(const FeaturePtr& feature);
    void RemoveFeature(const FeaturePtr& feature);
    void ClearFeatures();
    
    const std::vector<FeaturePtr>& GetFeatures() const;
    FeaturePtr GetFeatureById(int id) const;
    FeaturePtr GetFeatureByName(const std::string& name) const;
    
    // 特征操作
    bool ExecuteFeature(const FeaturePtr& feature);
    bool ExecuteAllFeatures();
    
    void SetFeatureActive(const FeaturePtr& feature, bool active);
    void SetAllFeaturesActive(bool active);
    
    // 特征排序
    void MoveFeatureUp(const FeaturePtr& feature);
    void MoveFeatureDown(const FeaturePtr& feature);
    void MoveFeatureToIndex(const FeaturePtr& feature, int index);
    
    // 更新和重建
    void UpdateFeature(const FeaturePtr& feature);
    void RebuildAllFeatures();
    
    // 实用方法
    int GetFeatureCount() const;
    bool IsEmpty() const;
    
    // 事件（用于界面通知）
    void SetFeatureAddedCallback(std::function<void(const FeaturePtr&)> callback);
    void SetFeatureRemovedCallback(std::function<void(const FeaturePtr&)> callback);
    void SetFeatureUpdatedCallback(std::function<void(const FeaturePtr&)> callback);

private:
    std::vector<FeaturePtr> m_features;
    
    // 回调函数
    std::function<void(const FeaturePtr&)> m_featureAddedCallback;
    std::function<void(const FeaturePtr&)> m_featureRemovedCallback;
    std::function<void(const FeaturePtr&)> m_featureUpdatedCallback;
    
    int FindFeatureIndex(const FeaturePtr& feature) const;
    void NotifyFeatureAdded(const FeaturePtr& feature);
    void NotifyFeatureRemoved(const FeaturePtr& feature);
    void NotifyFeatureUpdated(const FeaturePtr& feature);
};

} // namespace cad_feature