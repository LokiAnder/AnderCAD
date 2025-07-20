#pragma once

#include <TDocStd_Document.hxx>
#include <TDocStd_Application.hxx>
#include <TDF_Label.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDataStd_Name.hxx>
#include <TNaming_NamedShape.hxx>
#include <TDataStd_Integer.hxx>
#include <TDataStd_Real.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TCollection_AsciiString.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <memory>

#include "cad_core/Shape.h"

namespace cad_core {

class OCAFDocument {
public:
    OCAFDocument();
    ~OCAFDocument();
    
    // 初始化文档
    bool Initialize();
    
    // 文档操作
    bool NewDocument();
    bool OpenDocument(const std::string& filename);
    bool SaveDocument(const std::string& filename);
    
    // 形状操作
    TDF_Label AddShape(const ShapePtr& shape, const std::string& name = "");
    bool RemoveShape(const TDF_Label& label);
    ShapePtr GetShape(const TDF_Label& label) const;
    std::vector<TDF_Label> GetAllShapes() const;
    
    // 树操作
    TDF_Label CreateFolder(const std::string& name, const TDF_Label& parent = TDF_Label());
    bool MoveShape(const TDF_Label& shape, const TDF_Label& newParent);
    
    // 属性
    bool SetName(const TDF_Label& label, const std::string& name);
    std::string GetName(const TDF_Label& label) const;
    bool SetInteger(const TDF_Label& label, int value);
    int GetInteger(const TDF_Label& label) const;
    bool SetReal(const TDF_Label& label, double value);
    double GetReal(const TDF_Label& label) const;
    
    // 撤销/重做操作
    bool Undo();
    bool Redo();
    bool CanUndo() const;
    bool CanRedo() const;
    void StartTransaction(const std::string& name = "Operation");
    void CommitTransaction();
    void AbortTransaction();
    
    // 获取根标签
    TDF_Label GetRootLabel() const;
    
    // 获取文档
    Handle(TDocStd_Document) GetDocument() const { return m_document; }
    
private:
    Handle(TDocStd_Application) m_application;
    Handle(TDocStd_Document) m_document;
    Handle(XCAFDoc_ShapeTool) m_shapeTool;
    TDF_Label m_rootLabel;
    TDF_Label m_shapesLabel;
    
    bool m_isInitialized;
    bool m_inTransaction;
    
    // 辅助方法
    void InitializeApplication();
    void InitializeDocument();
    TDF_Label GetNextAvailableLabel(const TDF_Label& parent);
};

} // namespace cad_core