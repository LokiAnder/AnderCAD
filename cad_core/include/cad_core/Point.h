#pragma once

#include <gp_Pnt.hxx>

namespace cad_core {

class Point {
public:
    Point();
    Point(double x, double y, double z);
    explicit Point(const gp_Pnt& pnt);
    virtual ~Point() = default;

    double X() const;
    double Y() const;
    double Z() const;

    void SetX(double x);
    void SetY(double y);
    void SetZ(double z);
    void SetXYZ(double x, double y, double z);

    const gp_Pnt& GetOCCTPoint() const;
    void SetOCCTPoint(const gp_Pnt& pnt);

    double Distance(const Point& other) const;

private:
    gp_Pnt m_point;
};

} // namespace cad_core