#include "cad_ui/DocumentTree.h"
#include <QHeaderView>
#pragma execution_character_set("utf-8")

Q_DECLARE_METATYPE(cad_core::ShapePtr)
Q_DECLARE_METATYPE(cad_feature::FeaturePtr)

namespace cad_ui {

DocumentTree::DocumentTree(QWidget* parent) : QTreeWidget(parent) {
    SetupTree();
    CreateContextMenu();
    
    connect(this, &QTreeWidget::itemClicked, this, &DocumentTree::OnItemClicked);
    connect(this, &QTreeWidget::itemDoubleClicked, this, &DocumentTree::OnItemDoubleClicked);
}

void DocumentTree::SetupTree() {
    setHeaderLabel("Document");
    setRootIsDecorated(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Create root items
    m_shapesRoot = new QTreeWidgetItem(this);
    m_shapesRoot->setText(0, "Shapes");
    m_shapesRoot->setExpanded(true);
    
    m_featuresRoot = new QTreeWidgetItem(this);
    m_featuresRoot->setText(0, "Features");
    m_featuresRoot->setExpanded(true);
    
    addTopLevelItem(m_shapesRoot);
    addTopLevelItem(m_featuresRoot);
}

void DocumentTree::CreateContextMenu() {
    m_contextMenu = new QMenu(this);
    
    m_deleteAction = new QAction("Delete", this);
    m_renameAction = new QAction("Rename", this);
    m_toggleVisibilityAction = new QAction("Toggle Visibility", this);
    
    m_contextMenu->addAction(m_deleteAction);
    m_contextMenu->addAction(m_renameAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_toggleVisibilityAction);
    
    connect(m_deleteAction, &QAction::triggered, this, &DocumentTree::OnDeleteItem);
    connect(m_renameAction, &QAction::triggered, this, &DocumentTree::OnRenameItem);
    connect(m_toggleVisibilityAction, &QAction::triggered, this, &DocumentTree::OnToggleVisibility);
}

void DocumentTree::AddShape(const cad_core::ShapePtr& shape) {
    if (!shape) return;
    
    QTreeWidgetItem* item = new QTreeWidgetItem(m_shapesRoot);
    item->setText(0, QString("Shape %1").arg(m_shapesRoot->childCount()));
    item->setData(0, Qt::UserRole, QVariant::fromValue(shape));
    
    m_shapesRoot->addChild(item);
    m_shapesRoot->setExpanded(true);
}

void DocumentTree::RemoveShape(const cad_core::ShapePtr& shape) {
    if (!shape) return;
    
    for (int i = 0; i < m_shapesRoot->childCount(); ++i) {
        QTreeWidgetItem* item = m_shapesRoot->child(i);
        auto itemShape = item->data(0, Qt::UserRole).value<cad_core::ShapePtr>();
        if (itemShape == shape) {
            m_shapesRoot->removeChild(item);
            delete item;
            break;
        }
    }
}

void DocumentTree::AddFeature(const cad_feature::FeaturePtr& feature) {
    if (!feature) return;
    
    QTreeWidgetItem* item = new QTreeWidgetItem(m_featuresRoot);
    item->setText(0, QString::fromStdString(feature->GetName()));
    item->setData(0, Qt::UserRole, QVariant::fromValue(feature));
    
    m_featuresRoot->addChild(item);
    m_featuresRoot->setExpanded(true);
}

void DocumentTree::RemoveFeature(const cad_feature::FeaturePtr& feature) {
    if (!feature) return;
    
    for (int i = 0; i < m_featuresRoot->childCount(); ++i) {
        QTreeWidgetItem* item = m_featuresRoot->child(i);
        auto itemFeature = item->data(0, Qt::UserRole).value<cad_feature::FeaturePtr>();
        if (itemFeature == feature) {
            m_featuresRoot->removeChild(item);
            delete item;
            break;
        }
    }
}

void DocumentTree::Clear() {
    m_shapesRoot->takeChildren();
    m_featuresRoot->takeChildren();
}

void DocumentTree::contextMenuEvent(QContextMenuEvent* event) {
    QTreeWidgetItem* item = itemAt(event->pos());
    if (item && item != m_shapesRoot && item != m_featuresRoot) {
        m_contextMenu->exec(event->globalPos());
    }
}

void DocumentTree::OnItemClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    
    if (!item || item == m_shapesRoot || item == m_featuresRoot) {
        return;
    }
    
    // Check if it's a shape
    auto shape = item->data(0, Qt::UserRole).value<cad_core::ShapePtr>();
    if (shape) {
        emit ShapeSelected(shape);
        return;
    }
    
    // Check if it's a feature
    auto feature = item->data(0, Qt::UserRole).value<cad_feature::FeaturePtr>();
    if (feature) {
        emit FeatureSelected(feature);
        return;
    }
}

void DocumentTree::OnItemDoubleClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    
    if (item && item != m_shapesRoot && item != m_featuresRoot) {
        // Start editing the item name
        editItem(item, 0);
    }
}

void DocumentTree::OnDeleteItem() {
    QTreeWidgetItem* item = currentItem();
    if (!item || item == m_shapesRoot || item == m_featuresRoot) {
        return;
    }
    
    // Remove the item
    if (item->parent() == m_shapesRoot) {
        m_shapesRoot->removeChild(item);
    } else if (item->parent() == m_featuresRoot) {
        m_featuresRoot->removeChild(item);
    }
    
    delete item;
}

void DocumentTree::OnRenameItem() {
    QTreeWidgetItem* item = currentItem();
    if (item && item != m_shapesRoot && item != m_featuresRoot) {
        editItem(item, 0);
    }
}

void DocumentTree::OnToggleVisibility() {
    QTreeWidgetItem* item = currentItem();
    if (!item || item == m_shapesRoot || item == m_featuresRoot) {
        return;
    }
    
    // Toggle visibility (placeholder - would need to be implemented with actual visibility logic)
    QFont font = item->font(0);
    font.setStrikeOut(!font.strikeOut());
    item->setFont(0, font);
}

} // namespace cad_ui

#include "DocumentTree.moc"