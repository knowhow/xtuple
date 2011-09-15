/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

/*
TODO:	make deleted line items more obvious
TODO:	return to the editor if validation fails

TODO:	refactor:
	move some of the logic from ToitemTableDelegate::setModelData()
		into ToitemTableModel
	move ToitemTableDelegate into a separate file and split into
		ToitemDelegate and subclass it to ToitemTableDelegate
	write transferOrderItem to use ToitemDelegate
	move p/o and t/o commonalities to generic parent class

*/

#include "toitemTableView.h"

#include <QFont>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QString>

#include "guiclient.h"
#include "datecluster.h"
#include "itemcluster.h"
#include "toitemTableModel.h"
#include "projectcluster.h"
#include "xtreewidget.h"	// for column widths

// #define QE_PROJECT	// define this if you want project support on qe
#define QE_NONINVENTORY

ToitemTableView::ToitemTableView(QWidget* parent) :
  QTableView(parent)
{
  ToitemTableDelegate *delegate = new ToitemTableDelegate(this);
  setItemDelegate(delegate);
  setShowGrid(false);

#ifdef Q_WS_MAC
  QFont f = font();
  f.setPointSize(f.pointSize() - 2);
  setFont(f);
#endif

  connect(delegate, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SLOT(sHandleCloseEditor(QWidget*)));
}

void ToitemTableView::sHandleCloseEditor(QWidget *editor)
{
#ifdef QE_PROJECT
  if ((_metrics->boolean("UseProjects") && editor->objectName() == "prj_number") ||
      (!_metrics->boolean("UseProjects") && editor->objectName() == "toitem_duedate"))
#else
  if (editor->objectName() == "toitem_duedate")
#endif
  {
    setCurrentIndex(model()->index(model()->rowCount() - 1, ITEM_NUMBER_COL));
  }
}

void ToitemTableView::setModel(QAbstractItemModel* model)
{

  QTableView::setModel(model);

  setColumnWidth(ITEM_NUMBER_COL,		_itemColumn);
  setColumnWidth(TOITEM_QTY_ORDERED_COL,	_qtyColumn);
  setColumnWidth(TOITEM_STDCOST_COL,		_priceColumn);
  setColumnWidth(TOITEM_FREIGHT_COL,		_priceColumn);
  setColumnWidth(TOITEM_DUEDATE_COL,		_dateColumn);
#ifdef QE_PROJECT
  setColumnWidth(PRJ_NUMBER_COL,		100);
#endif

  QHeaderView *header = horizontalHeader();

  int dest = 0;
  header->moveSection(header->visualIndex(ITEM_NUMBER_COL),	 	dest++);
  header->moveSection(header->visualIndex(TOITEM_QTY_ORDERED_COL),	dest++);
  header->moveSection(header->visualIndex(TOITEM_STDCOST_COL),		dest++);
  header->moveSection(header->visualIndex(TOITEM_FREIGHT_COL),		dest++);
  header->moveSection(header->visualIndex(TOITEM_DUEDATE_COL),		dest++);
#ifdef QE_PROJECT
  header->moveSection(header->visualIndex(PRJ_NUMBER_COL),		dest++);
#endif

  // if we didn't explicitly place the logical section, hide it
  for (int i = dest; i < header->count(); i++)
    header->hideSection(header->logicalIndex(i));

#ifdef QE_PROJECT
  if (! _metrics->boolean("UseProjects"))
    header->hideSection(header->visualIndex(PRJ_NUMBER_COL));
#endif

  //header->setStretchLastSection(true);
}

void ToitemTableView::currentChanged(const QModelIndex &current, const QModelIndex &previous ) 
{
  if (current != QModelIndex() && current != previous)
  {
    setCurrentIndex(current);
    scrollTo(current);
    edit(current);
  }
}

///////////////////////////////////////////////////////////////////////////

ToitemTableDelegate::ToitemTableDelegate(QObject *parent)
  : QItemDelegate(parent)
{
}

QWidget *ToitemTableDelegate::createEditor(QWidget *parent,
					   const QStyleOptionViewItem &/*style*/,
					   const QModelIndex &index) const
{
  const ToitemTableModel *model = (ToitemTableModel*)(index.model());
  QWidget *editor = 0;

  switch (index.column())
  {
    case ITEM_NUMBER_COL:
    {
      ItemLineEdit *item = new ItemLineEdit(parent);
      item->setType(ItemLineEdit::cActive);
      item->addExtraClause(QString("(item_id IN ("
				    "  SELECT itemsite_item_id"
				    "  FROM itemsite"
				    "  WHERE itemsite_warehous_id=%1))")
				    .arg(model->srcWhsId()));

      editor = item;
      break;
    }

    case TOITEM_QTY_ORDERED_COL:
    {
      QLineEdit *qty = new QLineEdit(parent);
      qty->setValidator(omfgThis->qtyVal());
      editor = qty;
      break;
    }
    
    case TOITEM_FREIGHT_COL:
    {
      QLineEdit *price = new QLineEdit(parent);
      price->setValidator(omfgThis->priceVal());
      editor = price;
      break;
    }

    case TOITEM_DUEDATE_COL:
    {
      DLineEdit *duedate = new DLineEdit(parent);
      editor = duedate;
      editor->setObjectName("toitem_duedate");
      break;
    }

#ifdef QE_PROJECT
    case PRJ_NUMBER_COL:
    {
      ProjectLineEdit *prj = new ProjectLineEdit(parent);
      prj->setType(ProjectLineEdit::PurchaseOrder);
      editor = prj;
      editor->setObjectName("prj_number");
      break;
    }
#endif

    default:
    {
      editor = 0;
      break;
    }
  }

  if (editor)
  {
#ifdef Q_WS_MAC
    // compensate for many custom widgets making themselves smaller on OS X
    QFont f = editor->font();
    f.setPointSize(f.pointSize() + 2);
    editor->setFont(f);
#endif
    editor->installEventFilter(const_cast<ToitemTableDelegate*>(this));
  }
  return editor;
}

void ToitemTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  const QAbstractItemModel *model = index.model();
  QLineEdit *lineedit = 0;
  switch (index.column())
  {
    case ITEM_NUMBER_COL:
    {
      ItemLineEdit *item = static_cast<ItemLineEdit*>(editor);
      item->setId(model->data(model->index(index.row(),
					   TOITEM_ITEM_ID_COL)).toInt());
      break;
    }

    case TOITEM_QTY_ORDERED_COL:
    case TOITEM_FREIGHT_COL:
      lineedit = static_cast<QLineEdit*>(editor);
      lineedit->setText(model->data(index).toString());
      break;

    case TOITEM_DUEDATE_COL:
    {
      DLineEdit *duedate = static_cast<DLineEdit*>(editor);
      duedate->setDate(index.data().toDate());
      break;
    }

#ifdef QE_PROJECT
    case PRJ_NUMBER_COL:
    {
      ProjectLineEdit *prj = static_cast<ProjectLineEdit*>(editor);
      prj->setId(model->data(model->index(index.row(), TOITEM_PRJ_ID_COL)).toInt());
      break;
    }
#endif

    default:
      break;
  }
}

void ToitemTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *pModel, const QModelIndex &index) const
{
  bool hitError = false;
  QVariant oldval = pModel->data(index);
  ToitemTableModel *model = static_cast<ToitemTableModel*>(pModel);

  switch (index.column())
  {
    case ITEM_NUMBER_COL:
    {
      ItemLineEdit *item = static_cast<ItemLineEdit*>(editor);
      if (oldval.isNull() || item->itemNumber() != oldval.toString())
      {
	if (! item->itemNumber().isEmpty() && item->isValid())
	{
	  XSqlQuery itemq;
	  itemq.prepare("SELECT *, stdCost(item_id) AS stdcost, uom_name "
			"FROM item JOIN uom ON (item_inv_uom_id=uom_id) "
			"WHERE (item_id=:item_id);");
	  itemq.bindValue(":item_id", item->id());
	  itemq.exec();
	  if (itemq.first())
	  {
	    if (itemq.value("stdcost").toDouble() == 0.0)
	    {
	      QMessageBox::critical(0, tr("No Standard Cost"),
		      tr("<p>The selected item has no Standard "
			 "Costing information. Please see your "
			 "controller to correct this situation "
			 "before continuing."));
	      hitError = true;
	      break;
	    }
	    model->setData(index, item->itemNumber());
	    model->setData(model->index(index.row(), TOITEM_ITEM_ID_COL), itemq.value("item_id").toInt());
	    model->setData(model->index(index.row(), TOITEM_UOM_COL), itemq.value("uom_name"));
	    model->setData(model->index(index.row(), TOITEM_STDCOST_COL),
			   formatPurchPrice(itemq.value("stdcost").toDouble()));
	  }
	  else if (itemq.lastError().type() != QSqlError::NoError)
	  {
	    systemError(0, itemq.lastError().databaseText(), __FILE__, __LINE__);
	    hitError = true;
	    break;
	  }
	}
	if (hitError)
	{
	  model->setData(index, QVariant(QVariant::String));
	  model->setData(model->index(index.row(), TOITEM_ITEM_ID_COL), QVariant(QVariant::Int));
	  model->setData(model->index(index.row(), TOITEM_STDCOST_COL), QVariant(QVariant::Double));
	}
      }
      break;
    }

    case TOITEM_FREIGHT_COL:
    {
      QLineEdit *lineedit = static_cast<QLineEdit*>(editor);
      if (lineedit->text().toDouble() != oldval.toDouble())
	model->setData(index, formatPurchPrice(lineedit->text().toDouble()));
      break;
    }

    case TOITEM_QTY_ORDERED_COL:
    {
      QLineEdit *lineedit = static_cast<QLineEdit*>(editor);
      if (lineedit->text().isEmpty())
	model->setData(index, QVariant());
      else
	model->setData(index, lineedit->text().toDouble());
      break;
    }
    
    case TOITEM_DUEDATE_COL:
    {
      DLineEdit *duedate = static_cast<DLineEdit*>(editor);
      if (duedate->date() != oldval.toDate())
      {
	model->setData(index, duedate->date());
      }

      break;
    }

#ifdef QE_PROJECT
    case PRJ_NUMBER_COL:
    {
      ProjectLineEdit *prj = static_cast<ProjectLineEdit*>(editor);
      if (prj->id() != oldval.toInt())
      {
	model->setData(model->index(index.row(), TOITEM_PRJ_ID_COL), prj->id());
	model->setData(index, prj->text());
      }
      break;
    }
#endif

    default:
      break;
  }

  QTableView *view = qobject_cast<QTableView*>(parent());
  if (view)
  {
    if (hitError)
      view->setCurrentIndex(index);
    else if (index.row() >= (model->rowCount() - 1))
    {
      QHeaderView* header = view->horizontalHeader();
      if (header->visualIndex(index.column()) >=
	  (header->count() - header->hiddenSectionCount() - 1))
      {
	model->insertRow(model->rowCount());
      }
    }
  }

  return;
}

void ToitemTableDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
  editor->setGeometry(option.rect);
}
