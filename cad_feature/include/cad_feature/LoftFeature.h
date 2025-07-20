#pragma once

#include "Feature.h"
#include "cad_sketch/Sketch.h"
#include <vector>

namespace cad_feature {

class LoftFeature : public Feature {
public:
    LoftFeature();
    LoftFeature(const std::string& name);
    virtual ~LoftFeature() = default;

    // Section operations
    void AddSection(const cad_sketch::SketchPtr& section);
    void RemoveSection(const cad_sketch::SketchPtr& section);
    void ClearSections();
    
    const std::vector<cad_sketch::SketchPtr>& GetSections() const;
    int GetSectionCount() const;
    
    // Guide curve operations
    void AddGuideCurve(const cad_sketch::SketchPtr& guide);
    void RemoveGuideCurve(const cad_sketch::SketchPtr& guide);
    void ClearGuideCurves();
    
    const std::vector<cad_sketch::SketchPtr>& GetGuideCurves() const;
    int GetGuideCurveCount() const;
    
    // Loft parameters
    void SetSolid(bool solid);
    bool GetSolid() const;
    
    void SetRuled(bool ruled);
    bool GetRuled() const;
    
    void SetClosed(bool closed);
    bool GetClosed() const;
    
    // Feature interface
    cad_core::ShapePtr CreateShape() const override;
    bool ValidateParameters() const override;
    std::shared_ptr<cad_core::ICommand> CreateCommand() const override;

private:
    std::vector<cad_sketch::SketchPtr> m_sections;
    std::vector<cad_sketch::SketchPtr> m_guideCurves;
    
    bool AreSectionsValid() const;
    bool AreGuideCurvesValid() const;
    cad_core::ShapePtr LoftSections() const;
};

using LoftFeaturePtr = std::shared_ptr<LoftFeature>;

} // namespace cad_feature