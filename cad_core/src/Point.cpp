#include "cad_core/Point.h"
#include <gp_XYZ.hxx>

namespace cad_core {

Point::Point() : m_point(0.0, 0.0, 0.0) {
}

Point::Point(double x, double y, double z) : m_point(x, y, z) {
}

Point::Point(const gp_Pnt& pnt) : m_point(pnt) {
}

double Point::X() const {
    return m_point.X();
}

double Point::Y() const {
    return m_point.Y();
}

double Point::Z() const {
    return m_point.Z();
}

void Point::SetX(double x) {
    m_point.SetX(x);
}

void Point::SetY(double y) {
    m_point.SetY(y);
}

void Point::SetZ(double z) {
    m_point.SetZ(z);
}

void Point::SetXYZ(double x, double y, double z) {
    m_point.SetX(x);
    m_point.SetY(y);
    m_point.SetZ(z);
}

const gp_Pnt& Point::GetOCCTPoint() const {
    return m_point;
}

void Point::SetOCCTPoint(const gp_Pnt& pnt) {
    m_point = pnt;
}

double Point::Distance(const Point& other) const {
    return m_point.Distance(other.m_point);
}

} // namespace cad_core