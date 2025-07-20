#pragma once

#include "ICommand.h"
#include "Shape.h"
#include "Point.h"
#include <vector>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax1.hxx>

namespace cad_core {

/**
 * @enum TransformationType
 * @brief 变换操作类型
 */
enum class TransformationType {
    Translate,  // 平移
    Rotate,     // 旋转
    Scale       // 缩放
};

/**
 * @class TransformCommand
 * @brief 变换命令基类，实现对几何体的变换操作
 */
class TransformCommand : public ICommand {
public:
    TransformCommand(const std::vector<ShapePtr>& shapes, TransformationType type);
    virtual ~TransformCommand() = default;

    bool Execute() override;
    bool Undo() override;
    bool Redo() override;
    const char* GetName() const override;

    // 获取变换后的形状（用于预览）
    std::vector<ShapePtr> GetTransformedShapes() const;
    
    // 设置变换参数（由派生类具体实现）
    virtual void SetTransformParameters() = 0;
    
protected:
    virtual gp_Trsf CreateTransformation() const = 0;
    virtual const char* GetTypeName() const = 0;

    std::vector<ShapePtr> m_originalShapes;
    std::vector<ShapePtr> m_transformedShapes;
    TransformationType m_type;
    bool m_executed;
};

/**
 * @class TranslateCommand
 * @brief 平移命令
 */
class TranslateCommand : public TransformCommand {
public:
    TranslateCommand(const std::vector<ShapePtr>& shapes, 
                    const Point& translation);
    TranslateCommand(const std::vector<ShapePtr>& shapes, 
                    double dx, double dy, double dz);

    void SetTransformParameters() override;
    void SetTranslation(const Point& translation);
    void SetTranslation(double dx, double dy, double dz);

protected:
    gp_Trsf CreateTransformation() const override;
    const char* GetTypeName() const override;

private:
    Point m_translation;
};

/**
 * @class RotateCommand  
 * @brief 旋转命令
 */
class RotateCommand : public TransformCommand {
public:
    RotateCommand(const std::vector<ShapePtr>& shapes,
                 const Point& axisPoint, const Point& axisDirection, 
                 double angleRadians);

    void SetTransformParameters() override;
    void SetRotationAxis(const Point& axisPoint, const Point& axisDirection);
    void SetRotationAngle(double angleRadians);
    void SetRotationAngleDegrees(double angleDegrees);

protected:
    gp_Trsf CreateTransformation() const override;
    const char* GetTypeName() const override;

private:
    Point m_axisPoint;
    Point m_axisDirection;
    double m_angleRadians;
};

/**
 * @class ScaleCommand
 * @brief 缩放命令
 */
class ScaleCommand : public TransformCommand {
public:
    ScaleCommand(const std::vector<ShapePtr>& shapes,
                const Point& centerPoint, double scaleFactor);
    ScaleCommand(const std::vector<ShapePtr>& shapes,
                const Point& centerPoint, double scaleX, double scaleY, double scaleZ);

    void SetTransformParameters() override;
    void SetScaleCenter(const Point& centerPoint);
    void SetUniformScale(double scaleFactor);
    void SetNonUniformScale(double scaleX, double scaleY, double scaleZ);

protected:
    gp_Trsf CreateTransformation() const override;
    const char* GetTypeName() const override;

private:
    Point m_centerPoint;
    double m_scaleX, m_scaleY, m_scaleZ;
    bool m_isUniform;
};

} // namespace cad_core