/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "voucheringEditList.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "mqlutil.h"
#include "voucher.h"
#include "miscVoucher.h"

voucheringEditList::voucheringEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_vo, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _vo->setRootIsDecorated(TRUE);
  _vo->addColumn(tr("Vchr. #"), _orderColumn + _vo->indentation(), Qt::AlignRight, true, "vouchernumber"  );
  _vo->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight, true, "ponumber");
  _vo->addColumn(tr("Invc./Item #"), _itemColumn, Qt::AlignRight, true, "itemnumber");
  _vo->addColumn(tr("Vendor #"),     _itemColumn, Qt::AlignRight, true, "vendnumber");
  _vo->addColumn(tr("Name/Description"),      -1, Qt::AlignLeft,  true, "description");
  _vo->addColumn(tr("Vend. Type"),   _itemColumn, Qt::AlignLeft,  true, "itemtype");
  _vo->addColumn(tr("UOM"),           _uomColumn, Qt::AlignCenter,true, "iteminvuom");
  _vo->addColumn(tr("Qty. Vchrd."),   _qtyColumn, Qt::AlignRight, true, "f_qty");
  _vo->addColumn(tr("Cost"),        _moneyColumn, Qt::AlignRight, true, "cost");

  connect(omfgThis, SIGNAL(vouchersUpdated()), this, SLOT(sFillList()));

  sFillList();
}

voucheringEditList::~voucheringEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void voucheringEditList::languageChange()
{
  retranslateUi(this);
}

void voucheringEditList::sPrint()
{
  orReport report("VoucheringEditList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void voucheringEditList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  QAction *menuItem;

  if (_vo->altId() == -1)
  {
    menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainVouchers"));

    menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  }
}

void voucheringEditList::sEdit()
{
  q.prepare( "SELECT vohead_pohead_id "
             "FROM vohead "
             "WHERE (vohead_id=:vohead_id);" );
  q.bindValue(":vohead_id", _vo->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("vohead_id", _vo->id());

    if (q.value("vohead_pohead_id").toInt() == -1)
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
}

void voucheringEditList::sView()
{
  q.prepare( "SELECT vohead_pohead_id "
             "FROM vohead "
             "WHERE (vohead_id=:vohead_id);" );
  q.bindValue(":vohead_id", _vo->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("vohead_id", _vo->id());

    if (q.value("vohead_pohead_id").toInt() == -1)
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
  }
}

void voucheringEditList::sFillList()
{
  MetaSQLQuery mql = mqlLoad("voucher", "editlist");

  ParameterList params;
  params.append("credit",      tr("Credit"));
  params.append("debit",       tr("Debit"));
  params.append("notassigned", tr("Not Assigned"));
  q = mql.toQuery(params);
  _vo->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  // TODO: implement indentation
}
