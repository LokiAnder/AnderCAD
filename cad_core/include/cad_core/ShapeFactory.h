#pragma once

#include "Shape.h"
#include "Point.h"

namespace cad_core {

class ShapeFactory {
public:
    static ShapePtr CreateBox(const Point& corner1, const Point& corner2);
    static ShapePtr CreateBox(double width, double height, double depth);
    
    static ShapePtr CreateCylinder(const Point& center, double radius, double height);
    static ShapePtr CreateCylinder(double radius, double height);
    
    static ShapePtr CreateSphere(const Point& center, double radius);
    static ShapePtr CreateSphere(double radius);
    
private:
    ShapeFactory() = default;
};

} // namespace cad_core