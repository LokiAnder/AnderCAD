#pragma once

#include "SketchElement.h"
#include "SketchPoint.h"

namespace cad_sketch {

class SketchArc : public SketchElement {
public:
    SketchArc();
    SketchArc(const SketchPointPtr& center, double radius, double startAngle, double endAngle);
    SketchArc(double centerX, double centerY, double radius, double startAngle, double endAngle);
    virtual ~SketchArc() = default;

    const SketchPointPtr& GetCenter() const;
    void SetCenter(const SketchPointPtr& center);
    
    double GetRadius() const;
    void SetRadius(double radius);
    
    double GetStartAngle() const;
    void SetStartAngle(double angle);
    
    double GetEndAngle() const;
    void SetEndAngle(double angle);
    
    double GetSweepAngle() const;
    double GetLength() const;
    
    SketchPointPtr GetStartPoint() const;
    SketchPointPtr GetEndPoint() const;
    
    std::string GetDescription() const override;

private:
    SketchPointPtr m_center;
    double m_radius;
    double m_startAngle;
    double m_endAngle;
};

using SketchArcPtr = std::shared_ptr<SketchArc>;

} // namespace cad_sketch