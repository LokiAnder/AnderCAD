#include "cad_sketch/SnappingManager.h"
#include "cad_sketch/SketchLine.h"
#include "cad_sketch/SketchCircle.h"
#include "cad_sketch/SketchArc.h"
#include <cmath>
#include <algorithm>

namespace cad_sketch {

SnappingManager::SnappingManager() : m_snapTolerance(5.0), m_gridSize(10.0) {
    // 默认启用常见的捕捉类型
    m_enabledSnapTypes.push_back(SnapType::Endpoint);
    m_enabledSnapTypes.push_back(SnapType::Midpoint);
    m_enabledSnapTypes.push_back(SnapType::Center);
    m_enabledSnapTypes.push_back(SnapType::Grid);
}

void SnappingManager::SetSnapTolerance(double tolerance) {
    m_snapTolerance = tolerance;
}

double SnappingManager::GetSnapTolerance() const {
    return m_snapTolerance;
}

void SnappingManager::EnableSnapType(SnapType type) {
    if (!IsSnapTypeEnabled(type)) {
        m_enabledSnapTypes.push_back(type);
    }
}

void SnappingManager::DisableSnapType(SnapType type) {
    auto it = std::find(m_enabledSnapTypes.begin(), m_enabledSnapTypes.end(), type);
    if (it != m_enabledSnapTypes.end()) {
        m_enabledSnapTypes.erase(it);
    }
}

bool SnappingManager::IsSnapTypeEnabled(SnapType type) const {
    return std::find(m_enabledSnapTypes.begin(), m_enabledSnapTypes.end(), type) != m_enabledSnapTypes.end();
}

void SnappingManager::SetGridSize(double gridSize) {
    m_gridSize = gridSize;
}

double SnappingManager::GetGridSize() const {
    return m_gridSize;
}

SnapResult SnappingManager::FindSnapPoint(const cad_core::Point& inputPoint, 
                                        const std::vector<SketchElementPtr>& elements) const {
    SnapResult bestResult;
    double bestDistance = m_snapTolerance;
    
    // 首先尝试网格捕捉
    if (IsSnapTypeEnabled(SnapType::Grid)) {
        SnapResult gridResult = SnapToGrid(inputPoint);
        if (gridResult.found) {
            double distance = inputPoint.Distance(gridResult.snapPoint);
            if (distance < bestDistance) {
                bestResult = gridResult;
                bestDistance = distance;
            }
        }
    }
    
    // 尝试端点捕捉
    if (IsSnapTypeEnabled(SnapType::Endpoint)) {
        SnapResult endpointResult = SnapToEndpoints(inputPoint, elements);
        if (endpointResult.found) {
            double distance = inputPoint.Distance(endpointResult.snapPoint);
            if (distance < bestDistance) {
                bestResult = endpointResult;
                bestDistance = distance;
            }
        }
    }
    
    // 尝试中点捕捉
    if (IsSnapTypeEnabled(SnapType::Midpoint)) {
        SnapResult midpointResult = SnapToMidpoints(inputPoint, elements);
        if (midpointResult.found) {
            double distance = inputPoint.Distance(midpointResult.snapPoint);
            if (distance < bestDistance) {
                bestResult = midpointResult;
                bestDistance = distance;
            }
        }
    }
    
    // 尝试中心点捕捉
    if (IsSnapTypeEnabled(SnapType::Center)) {
        SnapResult centerResult = SnapToCenters(inputPoint, elements);
        if (centerResult.found) {
            double distance = inputPoint.Distance(centerResult.snapPoint);
            if (distance < bestDistance) {
                bestResult = centerResult;
                bestDistance = distance;
            }
        }
    }
    
    return bestResult;
}

SnapResult SnappingManager::SnapToGrid(const cad_core::Point& inputPoint) const {
    SnapResult result;
    
    double x = std::round(inputPoint.X() / m_gridSize) * m_gridSize;
    double y = std::round(inputPoint.Y() / m_gridSize) * m_gridSize;
    
    cad_core::Point snapPoint(x, y, 0);
    
    if (IsWithinTolerance(inputPoint, snapPoint)) {
        result.found = true;
        result.type = SnapType::Grid;
        result.snapPoint = snapPoint;
    }
    
    return result;
}

SnapResult SnappingManager::SnapToEndpoints(const cad_core::Point& inputPoint, 
                                          const std::vector<SketchElementPtr>& elements) const {
    SnapResult result;
    
    for (const auto& element : elements) {
        if (element->GetType() == SketchElementType::Line) {
            auto line = std::dynamic_pointer_cast<SketchLine>(element);
            if (line && line->GetStartPoint() && line->GetEndPoint()) {
                if (IsWithinTolerance(inputPoint, line->GetStartPoint()->GetPoint())) {
                    result.found = true;
                    result.type = SnapType::Endpoint;
                    result.snapPoint = line->GetStartPoint()->GetPoint();
                    result.element = element;
                    return result;
                }
                if (IsWithinTolerance(inputPoint, line->GetEndPoint()->GetPoint())) {
                    result.found = true;
                    result.type = SnapType::Endpoint;
                    result.snapPoint = line->GetEndPoint()->GetPoint();
                    result.element = element;
                    return result;
                }
            }
        }
    }
    
    return result;
}

SnapResult SnappingManager::SnapToMidpoints(const cad_core::Point& inputPoint, 
                                          const std::vector<SketchElementPtr>& elements) const {
    SnapResult result;
    
    for (const auto& element : elements) {
        if (element->GetType() == SketchElementType::Line) {
            auto line = std::dynamic_pointer_cast<SketchLine>(element);
            if (line && line->GetStartPoint() && line->GetEndPoint()) {
                double midX = (line->GetStartPoint()->GetX() + line->GetEndPoint()->GetX()) / 2.0;
                double midY = (line->GetStartPoint()->GetY() + line->GetEndPoint()->GetY()) / 2.0;
                cad_core::Point midPoint(midX, midY, 0);
                
                if (IsWithinTolerance(inputPoint, midPoint)) {
                    result.found = true;
                    result.type = SnapType::Midpoint;
                    result.snapPoint = midPoint;
                    result.element = element;
                    return result;
                }
            }
        }
    }
    
    return result;
}

SnapResult SnappingManager::SnapToCenters(const cad_core::Point& inputPoint, 
                                        const std::vector<SketchElementPtr>& elements) const {
    SnapResult result;
    
    for (const auto& element : elements) {
        if (element->GetType() == SketchElementType::Circle) {
            auto circle = std::dynamic_pointer_cast<SketchCircle>(element);
            if (circle && circle->GetCenter()) {
                if (IsWithinTolerance(inputPoint, circle->GetCenter()->GetPoint())) {
                    result.found = true;
                    result.type = SnapType::Center;
                    result.snapPoint = circle->GetCenter()->GetPoint();
                    result.element = element;
                    return result;
                }
            }
        } else if (element->GetType() == SketchElementType::Arc) {
            auto arc = std::dynamic_pointer_cast<SketchArc>(element);
            if (arc && arc->GetCenter()) {
                if (IsWithinTolerance(inputPoint, arc->GetCenter()->GetPoint())) {
                    result.found = true;
                    result.type = SnapType::Center;
                    result.snapPoint = arc->GetCenter()->GetPoint();
                    result.element = element;
                    return result;
                }
            }
        }
    }
    
    return result;
}

bool SnappingManager::IsWithinTolerance(const cad_core::Point& p1, const cad_core::Point& p2) const {
    return p1.Distance(p2) <= m_snapTolerance;
}

} // namespace cad_sketch