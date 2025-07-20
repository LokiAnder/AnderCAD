#pragma once

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include "cad_core/Shape.h"
#include "cad_feature/Feature.h"

namespace cad_ui {

class DocumentTree : public QTreeWidget {
    Q_OBJECT

public:
    explicit DocumentTree(QWidget* parent = nullptr);
    ~DocumentTree() = default;

    void AddShape(const cad_core::ShapePtr& shape);
    void RemoveShape(const cad_core::ShapePtr& shape);
    void AddFeature(const cad_feature::FeaturePtr& feature);
    void RemoveFeature(const cad_feature::FeaturePtr& feature);
    void Clear();

signals:
    void ShapeSelected(const cad_core::ShapePtr& shape);
    void FeatureSelected(const cad_feature::FeaturePtr& feature);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void OnItemClicked(QTreeWidgetItem* item, int column);
    void OnItemDoubleClicked(QTreeWidgetItem* item, int column);
    void OnDeleteItem();
    void OnRenameItem();
    void OnToggleVisibility();

private:
    QTreeWidgetItem* m_shapesRoot;
    QTreeWidgetItem* m_featuresRoot;
    QMenu* m_contextMenu;
    QAction* m_deleteAction;
    QAction* m_renameAction;
    QAction* m_toggleVisibilityAction;
    
    void CreateContextMenu();
    void SetupTree();
};

} // namespace cad_ui