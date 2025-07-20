#include "cad_sketch/SketchArc.h"
#include <cmath>
#include <sstream>

namespace cad_sketch {

SketchArc::SketchArc() 
    : SketchElement(SketchElementType::Arc), m_radius(1.0), m_startAngle(0.0), m_endAngle(M_PI) {
    m_center = std::make_shared<SketchPoint>();
}

SketchArc::SketchArc(const SketchPointPtr& center, double radius, double startAngle, double endAngle)
    : SketchElement(SketchElementType::Arc), m_center(center), m_radius(radius), 
      m_startAngle(startAngle), m_endAngle(endAngle) {
}

SketchArc::SketchArc(double centerX, double centerY, double radius, double startAngle, double endAngle)
    : SketchElement(SketchElementType::Arc), m_radius(radius), 
      m_startAngle(startAngle), m_endAngle(endAngle) {
    m_center = std::make_shared<SketchPoint>(centerX, centerY);
}

const SketchPointPtr& SketchArc::GetCenter() const {
    return m_center;
}

void SketchArc::SetCenter(const SketchPointPtr& center) {
    m_center = center;
}

double SketchArc::GetRadius() const {
    return m_radius;
}

void SketchArc::SetRadius(double radius) {
    m_radius = radius;
}

double SketchArc::GetStartAngle() const {
    return m_startAngle;
}

void SketchArc::SetStartAngle(double angle) {
    m_startAngle = angle;
}

double SketchArc::GetEndAngle() const {
    return m_endAngle;
}

void SketchArc::SetEndAngle(double angle) {
    m_endAngle = angle;
}

double SketchArc::GetSweepAngle() const {
    double sweep = m_endAngle - m_startAngle;
    while (sweep < 0) sweep += 2 * M_PI;
    while (sweep > 2 * M_PI) sweep -= 2 * M_PI;
    return sweep;
}

double SketchArc::GetLength() const {
    return m_radius * GetSweepAngle();
}

SketchPointPtr SketchArc::GetStartPoint() const {
    if (!m_center) return nullptr;
    
    double x = m_center->GetX() + m_radius * std::cos(m_startAngle);
    double y = m_center->GetY() + m_radius * std::sin(m_startAngle);
    return std::make_shared<SketchPoint>(x, y);
}

SketchPointPtr SketchArc::GetEndPoint() const {
    if (!m_center) return nullptr;
    
    double x = m_center->GetX() + m_radius * std::cos(m_endAngle);
    double y = m_center->GetY() + m_radius * std::sin(m_endAngle);
    return std::make_shared<SketchPoint>(x, y);
}

std::string SketchArc::GetDescription() const {
    std::ostringstream oss;
    oss << "Arc (Radius: " << m_radius << ", Sweep: " << GetSweepAngle() * 180.0 / M_PI << "°)";
    return oss.str();
}

} // namespace cad_sketch