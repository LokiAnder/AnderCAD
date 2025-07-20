#include "cad_feature/Feature.h"

namespace cad_feature {

int Feature::s_nextId = 1;

Feature::Feature(FeatureType type, const std::string& name)
    : m_type(type), m_name(name), m_id(s_nextId++), m_state(FeatureState::Created), m_active(true) {
}

FeatureType Feature::GetType() const {
    return m_type;
}

const std::string& Feature::GetName() const {
    return m_name;
}

void Feature::SetName(const std::string& name) {
    m_name = name;
}

int Feature::GetId() const {
    return m_id;
}

void Feature::SetId(int id) {
    m_id = id;
}

FeatureState Feature::GetState() const {
    return m_state;
}

void Feature::SetState(FeatureState state) {
    m_state = state;
}

bool Feature::IsActive() const {
    return m_active;
}

void Feature::SetActive(bool active) {
    m_active = active;
}

void Feature::SetParameter(const std::string& name, double value) {
    m_parameters[name] = value;
}

double Feature::GetParameter(const std::string& name) const {
    auto it = m_parameters.find(name);
    if (it != m_parameters.end()) {
        return it->second;
    }
    return 0.0;
}

bool Feature::HasParameter(const std::string& name) const {
    return m_parameters.find(name) != m_parameters.end();
}

cad_core::ShapePtr Feature::CreatePreviewShape() const {
    return CreateShape();
}

} // namespace cad_feature