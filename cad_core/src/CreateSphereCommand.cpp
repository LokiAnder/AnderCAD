#include "cad_core/CreateSphereCommand.h"
#include "cad_core/ShapeFactory.h"

namespace cad_core {

CreateSphereCommand::CreateSphereCommand(const Point& center, double radius)
    : m_center(center), m_radius(radius), m_useCenter(true), m_executed(false) {
}

CreateSphereCommand::CreateSphereCommand(double radius)
    : m_center(), m_radius(radius), m_useCenter(false), m_executed(false) {
}

bool CreateSphereCommand::Execute() {
    if (m_executed) {
        return true;
    }

    if (m_useCenter) {
        m_createdShape = ShapeFactory::CreateSphere(m_center, m_radius);
    } else {
        m_createdShape = ShapeFactory::CreateSphere(m_radius);
    }

    m_executed = (m_createdShape != nullptr);
    return m_executed;
}

bool CreateSphereCommand::Undo() {
    if (!m_executed) {
        return false;
    }

    m_createdShape.reset();
    m_executed = false;
    return true;
}

bool CreateSphereCommand::Redo() {
    if (m_executed) {
        return true;
    }

    return Execute();
}

const char* CreateSphereCommand::GetName() const {
    return "Create Sphere";
}

ShapePtr CreateSphereCommand::GetCreatedShape() const {
    return m_createdShape;
}

} // namespace cad_core