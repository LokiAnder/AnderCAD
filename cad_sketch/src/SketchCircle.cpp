#include "cad_sketch/SketchCircle.h"
#include <cmath>
#include <sstream>

namespace cad_sketch {

SketchCircle::SketchCircle() : SketchElement(SketchElementType::Circle), m_radius(1.0) {
    m_center = std::make_shared<SketchPoint>();
}

SketchCircle::SketchCircle(const SketchPointPtr& center, double radius)
    : SketchElement(SketchElementType::Circle), m_center(center), m_radius(radius) {
}

SketchCircle::SketchCircle(double centerX, double centerY, double radius)
    : SketchElement(SketchElementType::Circle), m_radius(radius) {
    m_center = std::make_shared<SketchPoint>(centerX, centerY);
}

const SketchPointPtr& SketchCircle::GetCenter() const {
    return m_center;
}

void SketchCircle::SetCenter(const SketchPointPtr& center) {
    m_center = center;
}

double SketchCircle::GetRadius() const {
    return m_radius;
}

void SketchCircle::SetRadius(double radius) {
    m_radius = radius;
}

double SketchCircle::GetDiameter() const {
    return 2.0 * m_radius;
}

double SketchCircle::GetCircumference() const {
    return 2.0 * M_PI * m_radius;
}

double SketchCircle::GetArea() const {
    return M_PI * m_radius * m_radius;
}

std::string SketchCircle::GetDescription() const {
    std::ostringstream oss;
    oss << "Circle (Radius: " << m_radius << ")";
    return oss.str();
}

} // namespace cad_sketch