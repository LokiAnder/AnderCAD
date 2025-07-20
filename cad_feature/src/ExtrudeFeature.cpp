#include "cad_feature/ExtrudeFeature.h"
#include "cad_core/CreateBoxCommand.h"
#include <BRepPrimAPI_MakePrism.hxx>
#include <gp_Vec.hxx>

namespace cad_feature {

ExtrudeFeature::ExtrudeFeature() : Feature(FeatureType::Extrude, "Extrude") {
    SetParameter("distance", 10.0);
    SetParameter("direction_x", 0.0);
    SetParameter("direction_y", 0.0);
    SetParameter("direction_z", 1.0);
    SetParameter("taper_angle", 0.0);
    SetParameter("midplane", 0.0);
}

ExtrudeFeature::ExtrudeFeature(const std::string& name) : Feature(FeatureType::Extrude, name) {
    SetParameter("distance", 10.0);
    SetParameter("direction_x", 0.0);
    SetParameter("direction_y", 0.0);
    SetParameter("direction_z", 1.0);
    SetParameter("taper_angle", 0.0);
    SetParameter("midplane", 0.0);
}

void ExtrudeFeature::SetSketch(const cad_sketch::SketchPtr& sketch) {
    m_sketch = sketch;
}

const cad_sketch::SketchPtr& ExtrudeFeature::GetSketch() const {
    return m_sketch;
}

void ExtrudeFeature::SetDistance(double distance) {
    SetParameter("distance", distance);
}

double ExtrudeFeature::GetDistance() const {
    return GetParameter("distance");
}

void ExtrudeFeature::SetDirection(double x, double y, double z) {
    SetParameter("direction_x", x);
    SetParameter("direction_y", y);
    SetParameter("direction_z", z);
}

void ExtrudeFeature::GetDirection(double& x, double& y, double& z) const {
    x = GetParameter("direction_x");
    y = GetParameter("direction_y");
    z = GetParameter("direction_z");
}

void ExtrudeFeature::SetTaperAngle(double angle) {
    SetParameter("taper_angle", angle);
}

double ExtrudeFeature::GetTaperAngle() const {
    return GetParameter("taper_angle");
}

void ExtrudeFeature::SetMidplane(bool midplane) {
    SetParameter("midplane", midplane ? 1.0 : 0.0);
}

bool ExtrudeFeature::GetMidplane() const {
    return GetParameter("midplane") != 0.0;
}

cad_core::ShapePtr ExtrudeFeature::CreateShape() const {
    if (!ValidateParameters()) {
        return nullptr;
    }
    
    return ExtrudeSketch();
}

bool ExtrudeFeature::ValidateParameters() const {
    if (!IsSketchValid()) {
        return false;
    }
    
    double distance = GetDistance();
    if (distance <= 0.0) {
        return false;
    }
    
    double dx, dy, dz;
    GetDirection(dx, dy, dz);
    double length = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (length < 1e-10) {
        return false;
    }
    
    return true;
}

std::shared_ptr<cad_core::ICommand> ExtrudeFeature::CreateCommand() const {
    // For now, return a simple box command as placeholder
    return std::make_shared<cad_core::CreateBoxCommand>(GetDistance(), GetDistance(), GetDistance());
}

bool ExtrudeFeature::IsSketchValid() const {
    return m_sketch && !m_sketch->IsEmpty();
}

cad_core::ShapePtr ExtrudeFeature::ExtrudeSketch() const {
    if (!IsSketchValid()) {
        return nullptr;
    }
    
    try {
        // In a real implementation, this would:
        // 1. Convert sketch elements to OCCT wire/face
        // 2. Create extrusion vector from direction and distance
        // 3. Use BRepPrimAPI_MakePrism to create the solid
        
        // For now, return a simple box as placeholder
        double distance = GetDistance();
        auto shape = std::make_shared<cad_core::Shape>();
        
        // Placeholder: create a simple box
        // In real implementation, this would properly extrude the sketch
        
        return shape;
    } catch (...) {
        return nullptr;
    }
}

} // namespace cad_feature