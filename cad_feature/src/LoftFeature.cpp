#include "cad_feature/LoftFeature.h"
#include "cad_core/CreateSphereCommand.h"
#include <BRepOffsetAPI_ThruSections.hxx>
#include <algorithm>

namespace cad_feature {

LoftFeature::LoftFeature() : Feature(FeatureType::Loft, "Loft") {
    SetParameter("solid", 1.0);
    SetParameter("ruled", 0.0);
    SetParameter("closed", 0.0);
}

LoftFeature::LoftFeature(const std::string& name) : Feature(FeatureType::Loft, name) {
    SetParameter("solid", 1.0);
    SetParameter("ruled", 0.0);
    SetParameter("closed", 0.0);
}

void LoftFeature::AddSection(const cad_sketch::SketchPtr& section) {
    m_sections.push_back(section);
}

void LoftFeature::RemoveSection(const cad_sketch::SketchPtr& section) {
    auto it = std::find(m_sections.begin(), m_sections.end(), section);
    if (it != m_sections.end()) {
        m_sections.erase(it);
    }
}

void LoftFeature::ClearSections() {
    m_sections.clear();
}

const std::vector<cad_sketch::SketchPtr>& LoftFeature::GetSections() const {
    return m_sections;
}

int LoftFeature::GetSectionCount() const {
    return static_cast<int>(m_sections.size());
}

void LoftFeature::AddGuideCurve(const cad_sketch::SketchPtr& guide) {
    m_guideCurves.push_back(guide);
}

void LoftFeature::RemoveGuideCurve(const cad_sketch::SketchPtr& guide) {
    auto it = std::find(m_guideCurves.begin(), m_guideCurves.end(), guide);
    if (it != m_guideCurves.end()) {
        m_guideCurves.erase(it);
    }
}

void LoftFeature::ClearGuideCurves() {
    m_guideCurves.clear();
}

const std::vector<cad_sketch::SketchPtr>& LoftFeature::GetGuideCurves() const {
    return m_guideCurves;
}

int LoftFeature::GetGuideCurveCount() const {
    return static_cast<int>(m_guideCurves.size());
}

void LoftFeature::SetSolid(bool solid) {
    SetParameter("solid", solid ? 1.0 : 0.0);
}

bool LoftFeature::GetSolid() const {
    return GetParameter("solid") != 0.0;
}

void LoftFeature::SetRuled(bool ruled) {
    SetParameter("ruled", ruled ? 1.0 : 0.0);
}

bool LoftFeature::GetRuled() const {
    return GetParameter("ruled") != 0.0;
}

void LoftFeature::SetClosed(bool closed) {
    SetParameter("closed", closed ? 1.0 : 0.0);
}

bool LoftFeature::GetClosed() const {
    return GetParameter("closed") != 0.0;
}

cad_core::ShapePtr LoftFeature::CreateShape() const {
    if (!ValidateParameters()) {
        return nullptr;
    }
    
    return LoftSections();
}

bool LoftFeature::ValidateParameters() const {
    if (!AreSectionsValid()) {
        return false;
    }
    
    if (GetSectionCount() < 2) {
        return false;
    }
    
    return true;
}

std::shared_ptr<cad_core::ICommand> LoftFeature::CreateCommand() const {
    // For now, return a simple sphere command as placeholder
    return std::make_shared<cad_core::CreateSphereCommand>(5.0);
}

bool LoftFeature::AreSectionsValid() const {
    for (const auto& section : m_sections) {
        if (!section || section->IsEmpty()) {
            return false;
        }
    }
    return true;
}

bool LoftFeature::AreGuideCurvesValid() const {
    for (const auto& guide : m_guideCurves) {
        if (!guide || guide->IsEmpty()) {
            return false;
        }
    }
    return true;
}

cad_core::ShapePtr LoftFeature::LoftSections() const {
    if (!AreSectionsValid() || GetSectionCount() < 2) {
        return nullptr;
    }
    
    try {
        // In a real implementation, this would:
        // 1. Convert section sketches to OCCT wires
        // 2. Optionally convert guide curves to OCCT wires
        // 3. Use BRepOffsetAPI_ThruSections to create the lofted solid
        // 4. Apply solid/ruled/closed parameters
        
        // For now, return a simple shape as placeholder
        auto shape = std::make_shared<cad_core::Shape>();
        
        // Placeholder: create a simple sphere-like shape
        // In real implementation, this would properly loft between sections
        
        return shape;
    } catch (...) {
        return nullptr;
    }
}

} // namespace cad_feature