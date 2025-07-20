#include "cad_core/CreateBoxCommand.h"
#include "cad_core/ShapeFactory.h"

namespace cad_core {

CreateBoxCommand::CreateBoxCommand(const Point& corner1, const Point& corner2)
    : m_corner1(corner1), m_corner2(corner2), m_width(0), m_height(0), m_depth(0), 
      m_useCorners(true), m_executed(false) {
}

CreateBoxCommand::CreateBoxCommand(double width, double height, double depth)
    : m_corner1(), m_corner2(), m_width(width), m_height(height), m_depth(depth), 
      m_useCorners(false), m_executed(false) {
}

bool CreateBoxCommand::Execute() {
    if (m_executed) {
        return true;
    }

    if (m_useCorners) {
        m_createdShape = ShapeFactory::CreateBox(m_corner1, m_corner2);
    } else {
        m_createdShape = ShapeFactory::CreateBox(m_width, m_height, m_depth);
    }

    m_executed = (m_createdShape != nullptr);
    return m_executed;
}

bool CreateBoxCommand::Undo() {
    if (!m_executed) {
        return false;
    }

    m_createdShape.reset();
    m_executed = false;
    return true;
}

bool CreateBoxCommand::Redo() {
    if (m_executed) {
        return true;
    }

    return Execute();
}

const char* CreateBoxCommand::GetName() const {
    return "Create Box";
}

ShapePtr CreateBoxCommand::GetCreatedShape() const {
    return m_createdShape;
}

} // namespace cad_core