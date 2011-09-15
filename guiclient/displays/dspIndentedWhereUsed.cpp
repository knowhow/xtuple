/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspIndentedWhereUsed.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "dspInventoryHistory.h"

dspIndentedWhereUsed::dspIndentedWhereUsed(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspIndentedWhereUsed", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Indented Where Used"));
  setListLabel(tr("Bill of Materials Items"));
  setReportName("IndentedWhereUsed");
  setMetaSQLOptions("whereUsed", "detail");
  setUseAltId(true);

  _worksetid = 0;

  if (_metrics->boolean("AllowInactiveBomItems"))
    _item->setType(ItemLineEdit::cGeneralComponents);
  else
    _item->setType(ItemLineEdit::cGeneralComponents | ItemLineEdit::cActive);

  list()->setRootIsDecorated(true);
  list()->addColumn(tr("Seq. #"),               80, Qt::AlignRight, true, "bomwork_seqnumber");
  list()->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "item_number");
  list()->addColumn(tr("Description"),          -1, Qt::AlignLeft,  true, "descrip");
  list()->addColumn(tr("UOM"),          _uomColumn, Qt::AlignCenter,true, "uom_name");
  list()->addColumn(tr("Fxd. Qty."),    _qtyColumn, Qt::AlignRight, true, "bomwork_qtyfxd");
  list()->addColumn(tr("Qty. Per"),     _qtyColumn, Qt::AlignRight, true, "bomwork_qtyper");
  list()->addColumn(tr("Scrap %"),    _prcntColumn, Qt::AlignRight, true, "bomwork_scrap");
  list()->addColumn(tr("Effective"),   _dateColumn, Qt::AlignCenter,true, "bomwork_effective");
  list()->addColumn(tr("Expires"),     _dateColumn, Qt::AlignCenter,true, "bomwork_expires");

  _item->setFocus();
}

void dspIndentedWhereUsed::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspIndentedWhereUsed::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Print;
  }

  return NoError;
}

bool dspIndentedWhereUsed::setParams(ParameterList &params)
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Item Number"),
                          tr("You must enter a valid Item Number.") );
    _item->setFocus();
    return false;
  }

  params.append("item_id", _item->id());

  if(_showExpired->isChecked())
    params.append("showExpired");

  if(_showFuture->isChecked())
    params.append("showFuture");

  params.append("byIndented");

  params.append("bomworkset_id", _worksetid);

  return true;
}

void dspIndentedWhereUsed::sViewInventoryHistory()
{
  ParameterList params;
  params.append("item_id", list()->altId());
  params.append("run");

  dspInventoryHistory *newdlg = new dspInventoryHistory();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspIndentedWhereUsed::sPopulateMenu(QMenu *menu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  menuItem = menu->addAction(tr("View Item Inventory History..."), this, SLOT(sViewInventoryHistory()));
  menuItem->setEnabled(_privileges->check("ViewInventoryHistory"));
}

void dspIndentedWhereUsed::sPreview()
{
  worksetWrapper(0);
}

void dspIndentedWhereUsed::sPrint()
{
  worksetWrapper(1);
}

void dspIndentedWhereUsed::sFillList()
{
  worksetWrapper(2);
}

void dspIndentedWhereUsed::worksetWrapper(int action)
{
  XSqlQuery qq;
  qq.prepare("SELECT indentedWhereUsed(:item_id) AS workset_id;");
  qq.bindValue(":item_id", _item->id());
  qq.exec();
  if (qq.first())
  {
    _worksetid = qq.value("workset_id").toInt();

    if(action == 0)
      display::sPreview();
    else if(action == 1)
      display::sPrint();
    else //if(action == 2)
    {
      display::sFillList();
      list()->expandAll();
    }

    qq.prepare("SELECT deleteBOMWorkset(:bomworkset_id) AS result;");
    qq.bindValue(":bomworkset_id", _worksetid);
    qq.exec();
  }
  else
    QMessageBox::critical( this, tr("Error Executing Report"),
                           tr( "Was unable to create/collect the required information to create this report." ) );
}
