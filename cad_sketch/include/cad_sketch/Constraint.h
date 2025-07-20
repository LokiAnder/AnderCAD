#pragma once

#include "SketchElement.h"
#include <memory>
#include <vector>

namespace cad_sketch {

enum class ConstraintType {
    Horizontal,
    Vertical,
    Parallel,
    Perpendicular,
    Coincident,
    Distance,
    Angle,
    Radius,
    Diameter,
    Equal
};

class Constraint {
public:
    Constraint(ConstraintType type);
    virtual ~Constraint() = default;

    ConstraintType GetType() const;
    int GetId() const;
    void SetId(int id);
    
    void AddElement(const SketchElementPtr& element);
    const std::vector<SketchElementPtr>& GetElements() const;
    
    bool IsActive() const;
    void SetActive(bool active);
    
    virtual bool IsValid() const = 0;
    virtual std::string GetDescription() const = 0;
    virtual double GetError() const = 0;

protected:
    ConstraintType m_type;
    int m_id;
    std::vector<SketchElementPtr> m_elements;
    bool m_active;
    
    static int s_nextId;
};

using ConstraintPtr = std::shared_ptr<Constraint>;

} // namespace cad_sketch