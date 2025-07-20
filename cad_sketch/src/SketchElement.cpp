#include "cad_sketch/SketchElement.h"

namespace cad_sketch {

int SketchElement::s_nextId = 1;

SketchElement::SketchElement(SketchElementType type)
    : m_type(type), m_id(s_nextId++), m_selected(false), m_visible(true) {
}

SketchElementType SketchElement::GetType() const {
    return m_type;
}

int SketchElement::GetId() const {
    return m_id;
}

void SketchElement::SetId(int id) {
    m_id = id;
}

bool SketchElement::IsSelected() const {
    return m_selected;
}

void SketchElement::SetSelected(bool selected) {
    m_selected = selected;
}

bool SketchElement::IsVisible() const {
    return m_visible;
}

void SketchElement::SetVisible(bool visible) {
    m_visible = visible;
}

} // namespace cad_sketch