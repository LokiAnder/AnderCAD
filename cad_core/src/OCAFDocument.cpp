#include "cad_core/OCAFDocument.h"
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_Tool.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_Integer.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_NamedShape.hxx>
#include <BinDrivers.hxx>
#include <BinXCAFDrivers.hxx>
#include <XmlDrivers.hxx>
#include <XmlXCAFDrivers.hxx>
#include <Standard_GUID.hxx>
#include <TCollection_ExtendedString.hxx>
#include <iostream>

namespace cad_core {

OCAFDocument::OCAFDocument() 
    : m_isInitialized(false), m_inTransaction(false) {
}

OCAFDocument::~OCAFDocument() {
    if (m_inTransaction) {
        AbortTransaction();
    }
}

bool OCAFDocument::Initialize() {
    if (m_isInitialized) {
        return true;
    }
    
    try {
        InitializeApplication();
        NewDocument();
        m_isInitialized = true;
        return true;
    } catch (const Standard_Failure& e) {
        m_isInitialized = false;
        return false;
    }
}

void OCAFDocument::InitializeApplication() {
    // Create application
    m_application = new TDocStd_Application();
    
    // Initialize drivers
    BinDrivers::DefineFormat(m_application);
    XmlDrivers::DefineFormat(m_application);
    BinXCAFDrivers::DefineFormat(m_application);
    XmlXCAFDrivers::DefineFormat(m_application);
}

bool OCAFDocument::NewDocument() {
    try {
        // Create new document
        m_application->NewDocument(TCollection_ExtendedString("BinOcaf"), m_document);
        
        if (m_document.IsNull()) {
            return false;
        }
        
        InitializeDocument();
        return true;
    } catch (const Standard_Failure& e) {
        return false;
    }
}

void OCAFDocument::InitializeDocument() {
    // Get root label
    m_rootLabel = m_document->GetData()->Root();
    
    // Enable undo/redo for this document - this is crucial!
    m_document->SetUndoLimit(50); // Allow up to 50 undo operations
    
    // Create shapes folder
    m_shapesLabel = m_rootLabel.FindChild(1);
    TDataStd_Name::Set(m_shapesLabel, TCollection_ExtendedString("Shapes"));
    
    std::cout << "[OCAF] Document initialized with undo limit: " << m_document->GetUndoLimit() << std::endl;
    
    // Initialize XCAFDoc tools
    m_shapeTool = XCAFDoc_DocumentTool::ShapeTool(m_document->Main());
}

bool OCAFDocument::OpenDocument(const std::string& filename) {
    try {
        TCollection_ExtendedString path(filename.c_str());
        
        // Use the correct method for opening documents
        m_application->Open(path, m_document);
        if (!m_document.IsNull()) {
            InitializeDocument();
            return true;
        }
        return false;
    } catch (const Standard_Failure& e) {
        return false;
    }
}

bool OCAFDocument::SaveDocument(const std::string& filename) {
    try {
        if (m_document.IsNull()) {
            return false;
        }
        
        TCollection_ExtendedString path(filename.c_str());
        // Use the correct method for saving documents
        m_application->SaveAs(m_document, path);
        return true;
    } catch (const Standard_Failure& e) {
        return false;
    }
}

TDF_Label OCAFDocument::AddShape(const ShapePtr& shape, const std::string& name) {
    if (!shape || shape->GetOCCTShape().IsNull()) {
        return TDF_Label();
    }
    
    try {
        // Create label for the shape
        TDF_Label shapeLabel = GetNextAvailableLabel(m_shapesLabel);
        
        // Set the shape - this will be tracked by OCAF for undo/redo
        TNaming_Builder builder(shapeLabel);
        builder.Generated(shape->GetOCCTShape());
        
        // Also create a backup using TDataStd to ensure the transaction is recognized
        TDataStd_Integer::Set(shapeLabel, 1); // Mark as active shape
        
        // Set name if provided
        if (!name.empty()) {
            SetName(shapeLabel, name);
        } else {
            SetName(shapeLabel, "Shape");
        }
        
        return shapeLabel;
    } catch (const Standard_Failure& e) {
        return TDF_Label();
    }
}

bool OCAFDocument::RemoveShape(const TDF_Label& label) {
    if (label.IsNull()) {
        return false;
    }
    
    try {
        // Use TNaming_Builder to properly record the deletion for undo/redo
        TNaming_Builder builder(label);
        Handle(TNaming_NamedShape) namedShape;
        if (label.FindAttribute(TNaming_NamedShape::GetID(), namedShape)) {
            TopoDS_Shape shape = namedShape->Get();
            if (!shape.IsNull()) {
                builder.Delete(shape);
            }
        }
        
        // Mark as deleted but keep TNaming for undo/redo
        TDataStd_Integer::Set(label, 0); // Mark as deleted
        
        return true;
    } catch (const Standard_Failure& e) {
        return false;
    }
}

ShapePtr OCAFDocument::GetShape(const TDF_Label& label) const {
    if (label.IsNull()) {
        return nullptr;
    }
    
    try {
        Handle(TNaming_NamedShape) namedShape;
        if (label.FindAttribute(TNaming_NamedShape::GetID(), namedShape)) {
            TopoDS_Shape shape = namedShape->Get();
            if (!shape.IsNull()) {
                return std::make_shared<Shape>(shape);
            }
        }
        return nullptr;
    } catch (const Standard_Failure& e) {
        return nullptr;
    }
}

std::vector<TDF_Label> OCAFDocument::GetAllShapes() const {
    std::vector<TDF_Label> shapes;
    
    try {
        TDF_ChildIterator it(m_shapesLabel);
        for (; it.More(); it.Next()) {
            TDF_Label child = it.Value();
            Handle(TNaming_NamedShape) namedShape;
            if (child.FindAttribute(TNaming_NamedShape::GetID(), namedShape)) {
                shapes.push_back(child);
            }
        }
    } catch (const Standard_Failure& e) {
        // Return empty vector on error
    }
    
    return shapes;
}

TDF_Label OCAFDocument::CreateFolder(const std::string& name, const TDF_Label& parent) {
    try {
        TDF_Label parentLabel = parent.IsNull() ? m_rootLabel : parent;
        TDF_Label folderLabel = GetNextAvailableLabel(parentLabel);
        
        // Start transaction if not already started
        bool needCommit = false;
        if (!m_inTransaction) {
            StartTransaction();
            needCommit = true;
        }
        
        SetName(folderLabel, name);
        
        if (needCommit) {
            CommitTransaction();
        }
        
        return folderLabel;
    } catch (const Standard_Failure& e) {
        if (m_inTransaction) {
            AbortTransaction();
        }
        return TDF_Label();
    }
}

bool OCAFDocument::MoveShape(const TDF_Label& shape, const TDF_Label& newParent) {
    // For simplicity, we'll implement this as copy + delete
    // In a full implementation, you'd want to use TDF_CopyLabel
    return false; // Not implemented yet
}

bool OCAFDocument::SetName(const TDF_Label& label, const std::string& name) {
    if (label.IsNull()) {
        return false;
    }
    
    try {
        TDataStd_Name::Set(label, TCollection_ExtendedString(name.c_str()));
        return true;
    } catch (const Standard_Failure& e) {
        return false;
    }
}

std::string OCAFDocument::GetName(const TDF_Label& label) const {
    if (label.IsNull()) {
        return "";
    }
    
    try {
        Handle(TDataStd_Name) nameAttr;
        if (label.FindAttribute(TDataStd_Name::GetID(), nameAttr)) {
            TCollection_AsciiString asciiName(nameAttr->Get());
            return std::string(asciiName.ToCString());
        }
        return "";
    } catch (const Standard_Failure& e) {
        return "";
    }
}

bool OCAFDocument::SetInteger(const TDF_Label& label, int value) {
    if (label.IsNull()) {
        return false;
    }
    
    try {
        TDataStd_Integer::Set(label, value);
        return true;
    } catch (const Standard_Failure& e) {
        return false;
    }
}

int OCAFDocument::GetInteger(const TDF_Label& label) const {
    if (label.IsNull()) {
        return 0;
    }
    
    try {
        Handle(TDataStd_Integer) intAttr;
        if (label.FindAttribute(TDataStd_Integer::GetID(), intAttr)) {
            return intAttr->Get();
        }
        return 0;
    } catch (const Standard_Failure& e) {
        return 0;
    }
}

bool OCAFDocument::SetReal(const TDF_Label& label, double value) {
    if (label.IsNull()) {
        return false;
    }
    
    try {
        TDataStd_Real::Set(label, value);
        return true;
    } catch (const Standard_Failure& e) {
        return false;
    }
}

double OCAFDocument::GetReal(const TDF_Label& label) const {
    if (label.IsNull()) {
        return 0.0;
    }
    
    try {
        Handle(TDataStd_Real) realAttr;
        if (label.FindAttribute(TDataStd_Real::GetID(), realAttr)) {
            return realAttr->Get();
        }
        return 0.0;
    } catch (const Standard_Failure& e) {
        return 0.0;
    }
}

bool OCAFDocument::Undo() {
    if (!CanUndo()) {
        return false;
    }
    
    try {
        m_document->Undo();
        return true;
    } catch (const Standard_Failure& e) {
        return false;
    }
}

bool OCAFDocument::Redo() {
    if (!CanRedo()) {
        return false;
    }
    
    try {
        m_document->Redo();
        return true;
    } catch (const Standard_Failure& e) {
        return false;
    }
}

bool OCAFDocument::CanUndo() const {
    if (m_document.IsNull()) {
        return false;
    }
    
    return m_document->GetAvailableUndos() > 0;
}

bool OCAFDocument::CanRedo() const {
    if (m_document.IsNull()) {
        return false;
    }
    
    return m_document->GetAvailableRedos() > 0;
}

void OCAFDocument::StartTransaction(const std::string& name) {
    if (m_document.IsNull() || m_inTransaction) {
        return;
    }
    
    try {
        m_document->NewCommand();
        m_inTransaction = true;
        std::cout << "[OCAF] Transaction started: " << name << std::endl;
    } catch (const Standard_Failure& e) {
        m_inTransaction = false;
        std::cout << "[OCAF] Failed to start transaction: " << name << std::endl;
    }
}

void OCAFDocument::CommitTransaction() {
    if (m_document.IsNull() || !m_inTransaction) {
        std::cout << "[OCAF] Cannot commit: document null or no transaction" << std::endl;
        return;
    }
    
    try {
        m_document->CommitCommand();
        m_inTransaction = false;
        std::cout << "[OCAF] Transaction committed. Available undos: " << m_document->GetAvailableUndos() << std::endl;
    } catch (const Standard_Failure& e) {
        m_inTransaction = false;
        std::cout << "[OCAF] Failed to commit transaction" << std::endl;
    }
}

void OCAFDocument::AbortTransaction() {
    if (m_document.IsNull() || !m_inTransaction) {
        return;
    }
    
    try {
        m_document->AbortCommand();
        m_inTransaction = false;
    } catch (const Standard_Failure& e) {
        m_inTransaction = false;
    }
}

TDF_Label OCAFDocument::GetRootLabel() const {
    return m_rootLabel;
}

TDF_Label OCAFDocument::GetNextAvailableLabel(const TDF_Label& parent) {
    int tag = 1;
    TDF_Label child = parent.FindChild(tag, Standard_False);
    
    while (!child.IsNull()) {
        tag++;
        child = parent.FindChild(tag, Standard_False);
    }
    
    return parent.FindChild(tag, Standard_True);
}

} // namespace cad_core