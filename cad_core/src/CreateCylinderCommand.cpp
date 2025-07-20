#include "cad_core/CreateCylinderCommand.h"
#include "cad_core/ShapeFactory.h"

namespace cad_core {

CreateCylinderCommand::CreateCylinderCommand(const Point& center, double radius, double height)
    : m_center(center), m_radius(radius), m_height(height), m_useCenter(true), m_executed(false) {
}

CreateCylinderCommand::CreateCylinderCommand(double radius, double height)
    : m_center(), m_radius(radius), m_height(height), m_useCenter(false), m_executed(false) {
}

bool CreateCylinderCommand::Execute() {
    if (m_executed) {
        return true;
    }

    if (m_useCenter) {
        m_createdShape = ShapeFactory::CreateCylinder(m_center, m_radius, m_height);
    } else {
        m_createdShape = ShapeFactory::CreateCylinder(m_radius, m_height);
    }

    m_executed = (m_createdShape != nullptr);
    return m_executed;
}

bool CreateCylinderCommand::Undo() {
    if (!m_executed) {
        return false;
    }

    m_createdShape.reset();
    m_executed = false;
    return true;
}

bool CreateCylinderCommand::Redo() {
    if (m_executed) {
        return true;
    }

    return Execute();
}

const char* CreateCylinderCommand::GetName() const {
    return "Create Cylinder";
}

ShapePtr CreateCylinderCommand::GetCreatedShape() const {
    return m_createdShape;
}

} // namespace cad_core