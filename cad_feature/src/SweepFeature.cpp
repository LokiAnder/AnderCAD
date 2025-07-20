#include "cad_feature/SweepFeature.h"
#include "cad_core/CreateBoxCommand.h"
#include <BRepOffsetAPI_MakePipe.hxx>
#include <cmath>

namespace cad_feature {

SweepFeature::SweepFeature() : Feature(FeatureType::Sweep, "Sweep") {
    SetParameter("twist_angle", 0.0);
    SetParameter("scale_factor", 1.0);
    SetParameter("keep_orientation", 1.0);
}

SweepFeature::SweepFeature(const std::string& name) : Feature(FeatureType::Sweep, name) {
    SetParameter("twist_angle", 0.0);
    SetParameter("scale_factor", 1.0);
    SetParameter("keep_orientation", 1.0);
}

void SweepFeature::SetProfile(const cad_sketch::SketchPtr& profile) {
    m_profile = profile;
}

const cad_sketch::SketchPtr& SweepFeature::GetProfile() const {
    return m_profile;
}

void SweepFeature::SetPath(const cad_sketch::SketchPtr& path) {
    m_path = path;
}

const cad_sketch::SketchPtr& SweepFeature::GetPath() const {
    return m_path;
}

void SweepFeature::SetTwistAngle(double angle) {
    SetParameter("twist_angle", angle);
}

double SweepFeature::GetTwistAngle() const {
    return GetParameter("twist_angle");
}

void SweepFeature::SetScaleFactor(double factor) {
    SetParameter("scale_factor", factor);
}

double SweepFeature::GetScaleFactor() const {
    return GetParameter("scale_factor");
}

void SweepFeature::SetKeepOriginalOrientation(bool keep) {
    SetParameter("keep_orientation", keep ? 1.0 : 0.0);
}

bool SweepFeature::GetKeepOriginalOrientation() const {
    return GetParameter("keep_orientation") != 0.0;
}

cad_core::ShapePtr SweepFeature::CreateShape() const {
    if (!ValidateParameters()) {
        return nullptr;
    }
    
    return SweepProfile();
}

bool SweepFeature::ValidateParameters() const {
    if (!IsProfileValid() || !IsPathValid()) {
        return false;
    }
    
    double scaleFactor = GetScaleFactor();
    if (scaleFactor <= 0.0) {
        return false;
    }
    
    return true;
}

std::shared_ptr<cad_core::ICommand> SweepFeature::CreateCommand() const {
    // For now, return a simple box command as placeholder
    return std::make_shared<cad_core::CreateBoxCommand>(10.0, 10.0, 10.0);
}

bool SweepFeature::IsProfileValid() const {
    return m_profile && !m_profile->IsEmpty();
}

bool SweepFeature::IsPathValid() const {
    return m_path && !m_path->IsEmpty();
}

cad_core::ShapePtr SweepFeature::SweepProfile() const {
    if (!IsProfileValid() || !IsPathValid()) {
        return nullptr;
    }
    
    try {
        // In a real implementation, this would:
        // 1. Convert profile sketch to OCCT wire/face
        // 2. Convert path sketch to OCCT wire
        // 3. Use BRepOffsetAPI_MakePipe to create the swept solid
        // 4. Apply twist and scaling if specified
        
        // For now, return a simple shape as placeholder
        auto shape = std::make_shared<cad_core::Shape>();
        
        // Placeholder: create a simple box-like shape
        // In real implementation, this would properly sweep the profile along the path
        
        return shape;
    } catch (...) {
        return nullptr;
    }
}

} // namespace cad_feature