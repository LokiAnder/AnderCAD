#pragma once

#include "Feature.h"
#include "cad_sketch/Sketch.h"

namespace cad_feature {

class ExtrudeFeature : public Feature {
public:
    ExtrudeFeature();
    ExtrudeFeature(const std::string& name);
    virtual ~ExtrudeFeature() = default;

    // Sketch operations
    void SetSketch(const cad_sketch::SketchPtr& sketch);
    const cad_sketch::SketchPtr& GetSketch() const;
    
    // Extrude parameters
    void SetDistance(double distance);
    double GetDistance() const;
    
    void SetDirection(double x, double y, double z);
    void GetDirection(double& x, double& y, double& z) const;
    
    void SetTaperAngle(double angle);
    double GetTaperAngle() const;
    
    void SetMidplane(bool midplane);
    bool GetMidplane() const;
    
    // Feature interface
    cad_core::ShapePtr CreateShape() const override;
    bool ValidateParameters() const override;
    std::shared_ptr<cad_core::ICommand> CreateCommand() const override;

private:
    cad_sketch::SketchPtr m_sketch;
    
    bool IsSketchValid() const;
    cad_core::ShapePtr ExtrudeSketch() const;
};

using ExtrudeFeaturePtr = std::shared_ptr<ExtrudeFeature>;

} // namespace cad_feature