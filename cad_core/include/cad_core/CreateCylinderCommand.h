#pragma once

#include "ICommand.h"
#include "Shape.h"
#include "Point.h"

namespace cad_core {

class CreateCylinderCommand : public ICommand {
public:
    CreateCylinderCommand(const Point& center, double radius, double height);
    CreateCylinderCommand(double radius, double height);
    virtual ~CreateCylinderCommand() = default;

    bool Execute() override;
    bool Undo() override;
    bool Redo() override;
    const char* GetName() const override;

    ShapePtr GetCreatedShape() const;

private:
    Point m_center;
    double m_radius;
    double m_height;
    bool m_useCenter;
    ShapePtr m_createdShape;
    bool m_executed;
};

} // namespace cad_core