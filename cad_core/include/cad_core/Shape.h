/**
 * @file Shape.h
 * @brief 这是我们CAD系统的几何形状基础类，可以说是整个系统的"颜值担当"~
 * 
 * 这个类封装了OpenCASCADE的TopoDS_Shape，让我们能够更优雅地处理几何体
 * 不得不说OpenCASCADE的命名真的很有特色...TopoDS是什么鬼名字？😅
 * 
 * TODO: 添加更多几何属性计算（重心、包围盒等）
 * TODO: 考虑添加形状变换功能
 * TODO: 实现形状的序列化和反序列化
 */

#pragma once

#include <TopoDS_Shape.hxx>
#include <memory>

namespace cad_core {

/**
 * @class Shape
 * @brief 几何形状的包装类，让OpenCASCADE变得"人性化"一点
 * 
 * 这个类就像是给原始的OpenCASCADE形状穿了件漂亮的外衣，
 * 让它看起来更现代化，用起来更舒服~
 */
class Shape {
public:
    /** 默认构造函数 - 创建一个空的形状，就像空气一样轻盈 */
    Shape();
    
    /** 
     * 从OpenCASCADE形状构造 - 给原生形状披上我们的"马甲"
     * @param shape OpenCASCADE的原生形状
     */
    explicit Shape(const TopoDS_Shape& shape);
    
    /** 虚析构函数 - 确保继承时的安全析构，C++的基本礼貌 */
    virtual ~Shape() = default;

    /** 
     * 获取底层的OpenCASCADE形状 - 有时候还是需要"脱掉马甲"直接操作
     * @return 常量引用，只读不写，安全第一
     */
    const TopoDS_Shape& GetOCCTShape() const;
    
    /** 
     * 设置底层形状 - 给我们的"马甲"换个"内核"
     * @param shape 新的OpenCASCADE形状
     */
    void SetOCCTShape(const TopoDS_Shape& shape);
    
    /** 
     * 检查形状是否有效 - 毕竟不是所有形状都是"好形状"
     * @return true如果形状有效，false如果是个"空壳"
     */
    bool IsValid() const;
    
    /** 
     * 计算体积 - 让我们看看这个形状能装多少"水"
     * @return 体积值，单位取决于你的建模单位
     * TODO: 添加单位处理和错误检查
     */
    double Volume() const;
    
    /** 
     * 计算表面积 - 要是刷漆的话需要多少涂料？
     * @return 表面积值
     * TODO: 对于非封闭形状可能需要特殊处理
     */
    double Area() const;

private:
    /** 存储实际的OpenCASCADE形状 - 我们的"内核" */
    TopoDS_Shape m_shape;
};

/** 智能指针类型别名 - 现代C++的标配，内存管理不用愁 */
using ShapePtr = std::shared_ptr<Shape>;

} // namespace cad_core