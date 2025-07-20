#pragma once

#include "SketchElement.h"
#include "SketchPoint.h"

namespace cad_sketch {

class SketchCircle : public SketchElement {
public:
    SketchCircle();
    SketchCircle(const SketchPointPtr& center, double radius);
    SketchCircle(double centerX, double centerY, double radius);
    virtual ~SketchCircle() = default;

    const SketchPointPtr& GetCenter() const;
    void SetCenter(const SketchPointPtr& center);
    
    double GetRadius() const;
    void SetRadius(double radius);
    
    double GetDiameter() const;
    double GetCircumference() const;
    double GetArea() const;
    
    std::string GetDescription() const override;

private:
    SketchPointPtr m_center;
    double m_radius;
};

using SketchCirclePtr = std::shared_ptr<SketchCircle>;

} // namespace cad_sketch