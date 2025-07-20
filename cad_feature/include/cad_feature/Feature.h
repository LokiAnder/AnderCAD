/**
 * @file Feature.h
 * @brief 特征系统 - CAD建模的"变形金刚"！
 * 
 * 这个文件定义了CAD特征系统的核心结构，特征就像是对几何体的各种"魔法操作"：
 * 拉伸、旋转、扫掠、放样等等。每个特征都记录着如何从简单几何体变成复杂形状。
 * 
 * 特征系统是参数化建模的灵魂，让用户能够轻松修改设计参数，
 * 而整个模型会自动重新计算和更新。这就是现代CAD的神奇之处！🪄
 * 
 * TODO: 添加特征依赖关系管理
 * TODO: 实现特征的自动错误恢复
 * TODO: 支持特征模板和预设
 * TODO: 添加特征性能分析工具
 */

#pragma once

#include "cad_core/Shape.h"    // 几何形状基础 - 特征的"原材料"
#include "cad_core/ICommand.h" // 命令接口 - 让特征具备撤销/重做能力
#include <memory>              // 智能指针 - 现代C++的内存管家
#include <string>              // 字符串 - 特征名称和参数的载体
#include <map>                 // 映射容器 - 参数名到参数值的字典

namespace cad_feature {

/**
 * @enum FeatureType
 * @brief 特征类型枚举 - CAD的"十八般武艺"
 * 
 * 这里定义了我们系统支持的各种建模操作，
 * 每一种都有自己的"独门绝技"！
 */
enum class FeatureType {
    Extrude,      // 拉伸 - 把2D草图拉成3D实体，最常用的招式
    Revolve,      // 旋转 - 绕轴转一圈，做个花瓶啥的很方便
    Sweep,        // 扫掠 - 沿着路径拖动截面，像画笔一样
    Loft,         // 放样 - 连接多个截面，做飞机机翼的好方法
    Fillet,       // 倒圆角 - 让尖锐的边变得圆润，安全第一
    Chamfer,      // 倒角 - 切掉尖角，工程中很实用
    Draft,        // 拔模 - 让零件容易从模具中取出
    Shell,        // 抽壳 - 把实体掏空，做个容器
    Cut,          // 切除 - 用一个几何体去"咬"另一个
    Union,        // 合并 - 把多个几何体合成一个
    Intersection  // 相交 - 只保留重叠的部分
};

/**
 * @enum FeatureState
 * @brief 特征状态枚举 - 记录特征的"人生阶段"
 * 
 * 特征也有自己的生命周期，从创建到执行，
 * 有时还会"翻车"失败，很像程序员的日常 😅
 */
enum class FeatureState {
    Created,     // 已创建 - 刚出生的婴儿特征
    Previewing,  // 预览中 - 正在"试妆"，还没正式登场
    Executed,    // 已执行 - 任务完成，形状已生成
    Failed       // 执行失败 - 出了点小意外，需要调试
};

/**
 * @class Feature
 * @brief 特征基类 - 所有建模操作的"祖师爷"
 * 
 * 这个抽象基类定义了所有特征必须具备的基本能力：
 * 管理参数、生成形状、处理状态等。就像武林中的各派宗师，
 * 虽然招式不同，但都得遵循基本的江湖规矩 🥋
 * 
 * 参数化是特征系统的核心，让设计变得灵活可控！
 */
class Feature {
public:
    /**
     * 构造函数 - 创建一个新特征
     * @param type 特征类型，决定这个特征会施展什么"法术"
     * @param name 特征名称，给它起个好听的名字
     */
    Feature(FeatureType type, const std::string& name);
    
    /** 虚析构函数 - 确保派生类能优雅地"告别江湖" */
    virtual ~Feature() = default;

    /** 
     * 获取特征类型 - 看看这是个什么"门派"的特征
     * @return 特征类型枚举值
     */
    FeatureType GetType() const;
    
    /** 
     * 获取特征名称 - 看看这个特征的"艺名"
     * @return 特征名称的常量引用
     */
    const std::string& GetName() const;
    
    /** 
     * 设置特征名称 - 给特征改个名字
     * @param name 新的名称，要起得有意义哦
     */
    void SetName(const std::string& name);
    
    /** 
     * 获取特征ID - 每个特征都有自己的"身份证号"
     * @return 特征的唯一标识符
     */
    int GetId() const;
    
    /** 
     * 设置特征ID - 给特征分配"身份证号"
     * @param id 新的ID值
     */
    void SetId(int id);
    
    /** 
     * 获取特征状态 - 看看特征现在是什么"心情"
     * @return 当前的特征状态
     */
    FeatureState GetState() const;
    
    /** 
     * 设置特征状态 - 更新特征的"心情"
     * @param state 新的状态值
     */
    void SetState(FeatureState state);
    
    /** 
     * 检查特征是否激活 - 看看这个特征是"在线"还是"离线"
     * @return true表示激活，false表示休眠
     */
    bool IsActive() const;
    
    /** 
     * 设置特征激活状态 - 开启或关闭这个特征
     * @param active true激活，false休眠
     */
    void SetActive(bool active);
    
    // ========== 参数管理 - 特征的"控制面板" ==========
    
    /** 
     * 设置参数 - 调整特征的"旋钮"
     * @param name 参数名称，比如"拉伸距离"、"旋转角度"等
     * @param value 参数值，数字说话最直接
     */
    void SetParameter(const std::string& name, double value);
    
    /** 
     * 获取参数值 - 看看某个"旋钮"调到了多少
     * @param name 参数名称
     * @return 参数的当前值
     */
    double GetParameter(const std::string& name) const;
    
    /** 
     * 检查是否有某个参数 - 看看这个特征有没有这个"旋钮"
     * @param name 参数名称
     * @return true表示有这个参数，false表示没有
     */
    bool HasParameter(const std::string& name) const;
    
    // ========== 形状操作 - 特征的"表演时刻" ==========
    
    /** 
     * 创建形状 - 特征的核心能力，把参数变成实际几何体
     * 这是个纯虚函数，每个具体特征都要实现自己的"绝招"
     * @return 生成的几何形状
     */
    virtual cad_core::ShapePtr CreateShape() const = 0;
    
    /** 
     * 创建预览形状 - 让用户提前"试看"效果
     * 默认和正式创建一样，但可以做得更快一些
     * @return 预览用的几何形状
     */
    virtual cad_core::ShapePtr CreatePreviewShape() const;
    
    /** 
     * 验证参数 - 检查参数设置是否合理
     * 避免用户设置奇葩参数导致程序崩溃
     * @return true表示参数OK，false表示有问题
     */
    virtual bool ValidateParameters() const = 0;
    
    // ========== 命令接口 - 让特征支持撤销/重做 ==========
    
    /** 
     * 创建命令对象 - 把特征操作包装成可撤销的命令
     * @return 对应的命令对象
     */
    virtual std::shared_ptr<cad_core::ICommand> CreateCommand() const = 0;

protected:
    /** 特征类型 - 这个特征属于哪个"门派" */
    FeatureType m_type;
    
    /** 特征名称 - 特征的"艺名"，方便用户识别 */
    std::string m_name;
    
    /** 特征ID - 系统分配的"身份证号"，全局唯一 */
    int m_id;
    
    /** 特征状态 - 记录特征当前的"人生阶段" */
    FeatureState m_state;
    
    /** 激活状态 - 这个特征是"上班"还是"摸鱼" */
    bool m_active;
    
    /** 参数映射表 - 特征的"控制面板"，存储所有可调参数 */
    std::map<std::string, double> m_parameters;
    
    /** 静态ID计数器 - 用来分配唯一ID的"号码机" */
    static int s_nextId;
};

/** 特征智能指针类型别名 - 让特征管理更轻松 */
using FeaturePtr = std::shared_ptr<Feature>;

} // namespace cad_feature