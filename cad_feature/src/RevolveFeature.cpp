#include "cad_feature/RevolveFeature.h"
#include "cad_core/CreateCylinderCommand.h"
#include <BRepPrimAPI_MakeRevol.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <cmath>

namespace cad_feature {

RevolveFeature::RevolveFeature() : Feature(FeatureType::Revolve, "Revolve") {
    SetParameter("angle", 2.0 * M_PI);
    SetParameter("axis_x", 0.0);
    SetParameter("axis_y", 0.0);
    SetParameter("axis_z", 1.0);
    SetParameter("axis_origin_x", 0.0);
    SetParameter("axis_origin_y", 0.0);
    SetParameter("axis_origin_z", 0.0);
    SetParameter("midplane", 0.0);
}

RevolveFeature::RevolveFeature(const std::string& name) : Feature(FeatureType::Revolve, name) {
    SetParameter("angle", 2.0 * M_PI);
    SetParameter("axis_x", 0.0);
    SetParameter("axis_y", 0.0);
    SetParameter("axis_z", 1.0);
    SetParameter("axis_origin_x", 0.0);
    SetParameter("axis_origin_y", 0.0);
    SetParameter("axis_origin_z", 0.0);
    SetParameter("midplane", 0.0);
}

void RevolveFeature::SetSketch(const cad_sketch::SketchPtr& sketch) {
    m_sketch = sketch;
}

const cad_sketch::SketchPtr& RevolveFeature::GetSketch() const {
    return m_sketch;
}

void RevolveFeature::SetAngle(double angle) {
    SetParameter("angle", angle);
}

double RevolveFeature::GetAngle() const {
    return GetParameter("angle");
}

void RevolveFeature::SetAxis(double x, double y, double z) {
    SetParameter("axis_x", x);
    SetParameter("axis_y", y);
    SetParameter("axis_z", z);
}

void RevolveFeature::GetAxis(double& x, double& y, double& z) const {
    x = GetParameter("axis_x");
    y = GetParameter("axis_y");
    z = GetParameter("axis_z");
}

void RevolveFeature::SetAxisOrigin(double x, double y, double z) {
    SetParameter("axis_origin_x", x);
    SetParameter("axis_origin_y", y);
    SetParameter("axis_origin_z", z);
}

void RevolveFeature::GetAxisOrigin(double& x, double& y, double& z) const {
    x = GetParameter("axis_origin_x");
    y = GetParameter("axis_origin_y");
    z = GetParameter("axis_origin_z");
}

void RevolveFeature::SetMidplane(bool midplane) {
    SetParameter("midplane", midplane ? 1.0 : 0.0);
}

bool RevolveFeature::GetMidplane() const {
    return GetParameter("midplane") != 0.0;
}

cad_core::ShapePtr RevolveFeature::CreateShape() const {
    if (!ValidateParameters()) {
        return nullptr;
    }
    
    return RevolveSketch();
}

bool RevolveFeature::ValidateParameters() const {
    if (!IsSketchValid()) {
        return false;
    }
    
    double angle = GetAngle();
    if (angle <= 0.0 || angle > 2.0 * M_PI) {
        return false;
    }
    
    double ax, ay, az;
    GetAxis(ax, ay, az);
    double length = std::sqrt(ax*ax + ay*ay + az*az);
    if (length < 1e-10) {
        return false;
    }
    
    return true;
}

std::shared_ptr<cad_core::ICommand> RevolveFeature::CreateCommand() const {
    // For now, return a simple cylinder command as placeholder
    return std::make_shared<cad_core::CreateCylinderCommand>(5.0, 10.0);
}

bool RevolveFeature::IsSketchValid() const {
    return m_sketch && !m_sketch->IsEmpty();
}

cad_core::ShapePtr RevolveFeature::RevolveSketch() const {
    if (!IsSketchValid()) {
        return nullptr;
    }
    
    try {
        // In a real implementation, this would:
        // 1. Convert sketch elements to OCCT wire/face
        // 2. Create revolution axis from origin and direction
        // 3. Use BRepPrimAPI_MakeRevol to create the solid
        
        // For now, return a simple shape as placeholder
        auto shape = std::make_shared<cad_core::Shape>();
        
        // Placeholder: create a simple cylinder-like shape
        // In real implementation, this would properly revolve the sketch
        
        return shape;
    } catch (...) {
        return nullptr;
    }
}

} // namespace cad_feature