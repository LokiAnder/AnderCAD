#pragma once

#include "Feature.h"
#include "cad_sketch/Sketch.h"

namespace cad_feature {

class RevolveFeature : public Feature {
public:
    RevolveFeature();
    RevolveFeature(const std::string& name);
    virtual ~RevolveFeature() = default;

    // Sketch operations
    void SetSketch(const cad_sketch::SketchPtr& sketch);
    const cad_sketch::SketchPtr& GetSketch() const;
    
    // Revolve parameters
    void SetAngle(double angle);
    double GetAngle() const;
    
    void SetAxis(double x, double y, double z);
    void GetAxis(double& x, double& y, double& z) const;
    
    void SetAxisOrigin(double x, double y, double z);
    void GetAxisOrigin(double& x, double& y, double& z) const;
    
    void SetMidplane(bool midplane);
    bool GetMidplane() const;
    
    // Feature interface
    cad_core::ShapePtr CreateShape() const override;
    bool ValidateParameters() const override;
    std::shared_ptr<cad_core::ICommand> CreateCommand() const override;

private:
    cad_sketch::SketchPtr m_sketch;
    
    bool IsSketchValid() const;
    cad_core::ShapePtr RevolveSketch() const;
};

using RevolveFeaturePtr = std::shared_ptr<RevolveFeature>;

} // namespace cad_feature