#pragma once

#include "ICommand.h"
#include "Shape.h"
#include "Point.h"

namespace cad_core {

class CreateBoxCommand : public ICommand {
public:
    CreateBoxCommand(const Point& corner1, const Point& corner2);
    CreateBoxCommand(double width, double height, double depth);
    virtual ~CreateBoxCommand() = default;

    bool Execute() override;
    bool Undo() override;
    bool Redo() override;
    const char* GetName() const override;

    ShapePtr GetCreatedShape() const;

private:
    Point m_corner1;
    Point m_corner2;
    double m_width, m_height, m_depth;
    bool m_useCorners;
    ShapePtr m_createdShape;
    bool m_executed;
};

} // namespace cad_core