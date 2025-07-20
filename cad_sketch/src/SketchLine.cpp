#include "cad_sketch/SketchLine.h"
#include <cmath>
#include <sstream>

namespace cad_sketch {

SketchLine::SketchLine() : SketchElement(SketchElementType::Line) {
    m_startPoint = std::make_shared<SketchPoint>();
    m_endPoint = std::make_shared<SketchPoint>();
}

SketchLine::SketchLine(const SketchPointPtr& startPoint, const SketchPointPtr& endPoint)
    : SketchElement(SketchElementType::Line), m_startPoint(startPoint), m_endPoint(endPoint) {
}

SketchLine::SketchLine(double x1, double y1, double x2, double y2)
    : SketchElement(SketchElementType::Line) {
    m_startPoint = std::make_shared<SketchPoint>(x1, y1);
    m_endPoint = std::make_shared<SketchPoint>(x2, y2);
}

const SketchPointPtr& SketchLine::GetStartPoint() const {
    return m_startPoint;
}

const SketchPointPtr& SketchLine::GetEndPoint() const {
    return m_endPoint;
}

void SketchLine::SetStartPoint(const SketchPointPtr& point) {
    m_startPoint = point;
}

void SketchLine::SetEndPoint(const SketchPointPtr& point) {
    m_endPoint = point;
}

double SketchLine::GetLength() const {
    if (!m_startPoint || !m_endPoint) {
        return 0.0;
    }
    return m_startPoint->GetPoint().Distance(m_endPoint->GetPoint());
}

double SketchLine::GetAngle() const {
    if (!m_startPoint || !m_endPoint) {
        return 0.0;
    }
    double dx = m_endPoint->GetX() - m_startPoint->GetX();
    double dy = m_endPoint->GetY() - m_startPoint->GetY();
    return std::atan2(dy, dx);
}

std::string SketchLine::GetDescription() const {
    std::ostringstream oss;
    oss << "Line (Length: " << GetLength() << ")";
    return oss.str();
}

} // namespace cad_sketch