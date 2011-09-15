/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUninvoicedReceivings.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>

#include "enterPoitemReceipt.h"
#include "poLiabilityDistrib.h"
#include "postPoReturnCreditMemo.h"

dspUninvoicedReceivings::dspUninvoicedReceivings(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspUninvoicedReceivings", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Uninvoiced P/O Receipts and Returns"));
  setListLabel(tr("Purchase Order &Receipts"));
  setReportName("UninvoicedReceipts");
  setMetaSQLOptions("uninvoicedReceivings", "detail");
  setUseAltId(true);

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  list()->addColumn(tr("Date"),        _dateColumn,  Qt::AlignCenter, true, "thedate");
  list()->addColumn(tr("By"),          _orderColumn, Qt::AlignCenter, true, "f_user" );
  list()->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignLeft,   true, "ponumber");
  list()->addColumn(tr("#"),           _whsColumn,   Qt::AlignCenter, true, "poitem_linenumber");
  list()->addColumn(tr("Vendor"),      -1,           Qt::AlignLeft,   true, "vend_name");
  list()->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true, "itemnumber");
  list()->addColumn(tr("Uninvoiced"),  _qtyColumn,   Qt::AlignRight,  true, "qty");
  list()->addColumn(tr("Type"),        _itemColumn,  Qt::AlignLeft,   true, "type"); 
  list()->addColumn(tr("Value"),       _moneyColumn, Qt::AlignRight,  true, "value");

  sFillList();
}

void dspUninvoicedReceivings::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

void dspUninvoicedReceivings::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  if(_privileges->check("MaintainUninvoicedReceipts"))
  {
    if(list()->altId() < 3)
    {
      menuItem = pMenu->addAction(tr("Mark as Invoiced..."), this, SLOT(sMarkAsInvoiced()));
      menuItem = pMenu->addAction(tr("Correct Receiving..."), this, SLOT(sCorrectReceiving()));
      if(list()->altId() == 2)
        menuItem->setEnabled(false);
    }
    if (list()->altId() == 3)
      menuItem = pMenu->addAction(tr("Create Credit Memo..."), this, SLOT(sCreateCreditMemo()));
  }
}

void dspUninvoicedReceivings::sMarkAsInvoiced()
{
  bool update = true;
  
  q.prepare("SELECT * FROM porecv "
            "WHERE ((porecv_value <> 0) "
            "AND (porecv_id=:porecv_id));");
  q.bindValue(":porecv_id",list()->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("porecv_id", list()->id());  
    poLiabilityDistrib newdlg(this, "", true);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Rejected)
      update = false;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if (update)
  {
    q.prepare("UPDATE recv "
	      "SET recv_invoiced=true "
	      "WHERE (recv_id=:porecv_id); ");
    q.bindValue(":porecv_id",list()->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
}

void dspUninvoicedReceivings::sCorrectReceiving()
{
  if (enterPoitemReceipt::correctReceipt(list()->id(), this) != XDialog::Rejected)
    sFillList();
}

void dspUninvoicedReceivings::sCreateCreditMemo()
{
  ParameterList params;
  params.append("poreject_id", list()->id());

  postPoReturnCreditMemo newdlg(this, "", true);
  newdlg.set(params);

  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

bool dspUninvoicedReceivings::setParams(ParameterList &pParams)
{
  _warehouse->appendValue(pParams);
  if(_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());
  return true;
}

