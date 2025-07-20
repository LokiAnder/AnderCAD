#pragma once

#include "SketchElement.h"
#include "SketchPoint.h"

namespace cad_sketch {

class SketchLine : public SketchElement {
public:
    SketchLine();
    SketchLine(const SketchPointPtr& startPoint, const SketchPointPtr& endPoint);
    SketchLine(double x1, double y1, double x2, double y2);
    virtual ~SketchLine() = default;

    const SketchPointPtr& GetStartPoint() const;
    const SketchPointPtr& GetEndPoint() const;
    void SetStartPoint(const SketchPointPtr& point);
    void SetEndPoint(const SketchPointPtr& point);
    
    double GetLength() const;
    double GetAngle() const;
    
    std::string GetDescription() const override;

private:
    SketchPointPtr m_startPoint;
    SketchPointPtr m_endPoint;
};

using SketchLinePtr = std::shared_ptr<SketchLine>;

} // namespace cad_sketch