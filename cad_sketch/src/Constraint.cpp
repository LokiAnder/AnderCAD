#include "cad_sketch/Constraint.h"

namespace cad_sketch {

int Constraint::s_nextId = 1;

Constraint::Constraint(ConstraintType type) 
    : m_type(type), m_id(s_nextId++), m_active(true) {
}

ConstraintType Constraint::GetType() const {
    return m_type;
}

int Constraint::GetId() const {
    return m_id;
}

void Constraint::SetId(int id) {
    m_id = id;
}

void Constraint::AddElement(const SketchElementPtr& element) {
    m_elements.push_back(element);
}

const std::vector<SketchElementPtr>& Constraint::GetElements() const {
    return m_elements;
}

bool Constraint::IsActive() const {
    return m_active;
}

void Constraint::SetActive(bool active) {
    m_active = active;
}

} // namespace cad_sketch