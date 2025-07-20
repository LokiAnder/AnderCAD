#include "cad_sketch/SketchPoint.h"
#include <sstream>
#include <gp_XYZ.hxx>

namespace cad_sketch {

SketchPoint::SketchPoint() : SketchElement(SketchElementType::Point) {
}

SketchPoint::SketchPoint(double x, double y) 
    : SketchElement(SketchElementType::Point), m_point(x, y, 0) {
}

SketchPoint::SketchPoint(const cad_core::Point& point)
    : SketchElement(SketchElementType::Point), m_point(point) {
}

const cad_core::Point& SketchPoint::GetPoint() const {
    return m_point;
}

void SketchPoint::SetPoint(const cad_core::Point& point) {
    m_point = point;
}

double SketchPoint::GetX() const {
    return m_point.X();
}

double SketchPoint::GetY() const {
    return m_point.Y();
}

void SketchPoint::SetX(double x) {
    m_point.SetX(x);
}

void SketchPoint::SetY(double y) {
    m_point.SetY(y);
}

void SketchPoint::SetXY(double x, double y) {
    m_point.SetXYZ(x, y, 0);
}

std::string SketchPoint::GetDescription() const {
    std::ostringstream oss;
    oss << "Point (" << GetX() << ", " << GetY() << ")";
    return oss.str();
}

} // namespace cad_sketch