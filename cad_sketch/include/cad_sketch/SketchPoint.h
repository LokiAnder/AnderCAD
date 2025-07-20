#pragma once

#include "SketchElement.h"
#include "cad_core/Point.h"

namespace cad_sketch {

class SketchPoint : public SketchElement {
public:
    SketchPoint();
    SketchPoint(double x, double y);
    SketchPoint(const cad_core::Point& point);
    virtual ~SketchPoint() = default;

    const cad_core::Point& GetPoint() const;
    void SetPoint(const cad_core::Point& point);
    
    double GetX() const;
    double GetY() const;
    void SetX(double x);
    void SetY(double y);
    void SetXY(double x, double y);
    
    std::string GetDescription() const override;

private:
    cad_core::Point m_point;
};

using SketchPointPtr = std::shared_ptr<SketchPoint>;

} // namespace cad_sketch