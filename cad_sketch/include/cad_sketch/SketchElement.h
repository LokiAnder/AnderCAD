#pragma once

#include <memory>
#include <vector>
#include <string>

namespace cad_sketch {

enum class SketchElementType {
    Point,
    Line,
    Circle,
    Arc
};

class SketchElement {
public:
    SketchElement(SketchElementType type);
    virtual ~SketchElement() = default;

    SketchElementType GetType() const;
    int GetId() const;
    void SetId(int id);
    
    bool IsSelected() const;
    void SetSelected(bool selected);
    
    bool IsVisible() const;
    void SetVisible(bool visible);
    
    virtual std::string GetDescription() const = 0;

protected:
    SketchElementType m_type;
    int m_id;
    bool m_selected;
    bool m_visible;
    
    static int s_nextId;
};

using SketchElementPtr = std::shared_ptr<SketchElement>;

} // namespace cad_sketch