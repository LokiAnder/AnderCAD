/**
 * @file ICommand.h
 * @brief 命令接口 - CAD系统的"时光机器"核心技术！
 * 
 * 这个接口实现了经典的命令模式(Command Pattern)，让我们的CAD系统
 * 具备了强大的撤销/重做功能。想象一下，用户做错了什么操作，
 * 只需要Ctrl+Z就能回到过去，简直就是"后悔药"！
 * 
 * 设计模式真是个好东西，让代码变得优雅而强大 ✨
 * 
 * TODO: 考虑添加命令分组功能，支持复合操作的撤销
 * TODO: 添加命令执行状态查询
 * TODO: 实现命令的序列化，支持操作历史的保存和恢复
 */

#pragma once

#include <memory>

namespace cad_core {

/**
 * @class ICommand
 * @brief 命令接口 - 所有操作命令的"祖师爷"
 * 
 * 这个接口定义了所有命令必须实现的基本功能：
 * - 执行 (Execute): 做事情
 * - 撤销 (Undo): 后悔了，想回到过去
 * - 重做 (Redo): 后悔自己后悔了，再来一遍
 * - 获取名称 (GetName): 告诉别人自己是谁
 * 
 * 就像人生一样，有做有撤有重来，只是我们的系统比现实更仁慈 😄
 */
class ICommand {
public:
    /** 虚析构函数 - 多态的基本礼貌，确保派生类能正确析构 */
    virtual ~ICommand() = default;
    
    /** 
     * 执行命令 - "开始你的表演！"
     * @return true表示执行成功，false表示失败了
     */
    virtual bool Execute() = 0;
    
    /** 
     * 撤销命令 - "我后悔了，让我回到过去吧"
     * @return true表示撤销成功，false表示时光机坏了
     */
    virtual bool Undo() = 0;
    
    /** 
     * 重做命令 - "算了，还是按原计划来吧"
     * @return true表示重做成功，false表示又失败了
     */
    virtual bool Redo() = 0;
    
    /** 
     * 获取命令名称 - 自报家门，告诉大家自己是做什么的
     * @return 命令的名称，用于显示给用户看
     */
    virtual const char* GetName() const = 0;
};

/** 
 * 命令智能指针类型别名 - 现代C++的标配
 * 用智能指针管理命令对象，再也不用担心内存泄漏了
 */
using CommandPtr = std::shared_ptr<ICommand>;

} // namespace cad_core