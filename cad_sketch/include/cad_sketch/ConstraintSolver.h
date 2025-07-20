#pragma once

#include "Constraint.h"
#include "SketchElement.h"
#include <vector>
#include <memory>

namespace cad_sketch {

class ConstraintSolver {
public:
    ConstraintSolver();
    ~ConstraintSolver() = default;

    void AddConstraint(const ConstraintPtr& constraint);
    void RemoveConstraint(const ConstraintPtr& constraint);
    void ClearConstraints();
    
    const std::vector<ConstraintPtr>& GetConstraints() const;
    
    bool Solve();
    bool ValidateConstraints() const;
    
    void SetTolerance(double tolerance);
    double GetTolerance() const;
    
    void SetMaxIterations(int maxIterations);
    int GetMaxIterations() const;

private:
    std::vector<ConstraintPtr> m_constraints;
    double m_tolerance;
    int m_maxIterations;
    
    double CalculateSystemError() const;
    bool IterativeSolve();
};

} // namespace cad_sketch