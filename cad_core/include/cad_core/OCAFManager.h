#pragma once

#include "cad_core/OCAFDocument.h"
#include "cad_core/Shape.h"
#include <memory>
#include <string>
#include <vector>

namespace cad_core {

class OCAFManager {
public:
    OCAFManager();
    ~OCAFManager();
    
    // 初始化管理器
    bool Initialize();
    
    // 文档操作
    bool NewDocument();
    bool OpenDocument(const std::string& filename);
    bool SaveDocument(const std::string& filename);
    
    // 形状操作
    bool AddShape(const ShapePtr& shape, const std::string& name = "");
    bool RemoveShape(const std::string& name);
    bool RemoveShape(const ShapePtr& shape);  // 根据形状指针删除
    ShapePtr GetShape(const std::string& name) const;
    std::vector<std::string> GetAllShapeNames() const;
    std::vector<ShapePtr> GetAllShapes() const;
    
    // 撤销/重做操作
    bool Undo();
    bool Redo();
    bool CanUndo() const;
    bool CanRedo() const;
    
    // 事务操作
    void StartTransaction(const std::string& name = "Operation");
    void CommitTransaction();
    void AbortTransaction();
    
    // 获取文档
    std::shared_ptr<OCAFDocument> GetDocument() const { return m_document; }
    
private:
    std::shared_ptr<OCAFDocument> m_document;
    bool m_isInitialized;
    
    // 辅助方法
    TDF_Label FindShapeByName(const std::string& name) const;
    std::string GenerateUniqueName(const std::string& baseName) const;
};

} // namespace cad_core