/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "transferOrders.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "mqlutil.h"
#include "copyTransferOrder.h"
#include "issueToShipping.h"
#include "storedProcErrorLookup.h"
#include "transferOrder.h"
#include "printPackingList.h"

transferOrders::transferOrders(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_release, SIGNAL(clicked()), this, SLOT(sRelease()));
  connect(_copy,	  SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_delete,	  SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_destWarehouse, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_edit,	  SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,		  SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,	  SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_issue,	  SIGNAL(clicked()), this, SLOT(sIssue()));
  connect(_srcWarehouse,  SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_allStatuses,  SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_selectedStatus,  SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_statuses,  SIGNAL(activated(int)), this, SLOT(sFillList()));
  connect(_to, SIGNAL(itemSelectionChanged()), this, SLOT(sHandleButtons()));
  connect(_to, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*, int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_view,	  SIGNAL(clicked()), this, SLOT(sView()));
  connect(omfgThis, SIGNAL(transferOrdersUpdated(int)), this, SLOT(sFillList()));

  _to->addColumn(tr("Order #"),                -1,  Qt::AlignLeft,   true,  "tohead_number"   );
  _to->addColumn(tr("Status"),       _statusColumn*2, Qt::AlignCenter, true,  "f_status" );
  _to->addColumn(tr("Source Site"),  _whsColumn*3,  Qt::AlignLeft,   true,  "srcWhs"   );
  _to->addColumn(tr("Dest. Site"),	 _whsColumn*3,  Qt::AlignLeft,   true,  "destWhs"   );
  _to->addColumn(tr("Ordered"),       _dateColumn,  Qt::AlignCenter, true,  "tohead_orderdate" );
  _to->addColumn(tr("Scheduled"),     _dateColumn,  Qt::AlignCenter, true,  "scheduled" );
  
  if (_privileges->check("MaintainTransferOrders"))
  {
    connect(_to,       SIGNAL(valid(bool)), _edit,    SLOT(setEnabled(bool)));
    connect(_to,       SIGNAL(valid(bool)), _copy,    SLOT(setEnabled(bool)));
    connect(_to,       SIGNAL(valid(bool)), _delete,  SLOT(setEnabled(bool)));
    connect(_to,       SIGNAL(valid(bool)), _issue,   SLOT(setEnabled(bool)));
    connect(_to, SIGNAL(itemSelected(int)), _edit,    SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_to, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _numSelected = 0;
  _destWarehouse->setAll();

  sFillList();
}

transferOrders::~transferOrders()
{
    // no need to delete child widgets, Qt does it all for us
}

void transferOrders::languageChange()
{
    retranslateUi(this);
}

void transferOrders::setParams(ParameterList& params)
{
  if (_srcWarehouse->isSelected())
    params.append("src_warehous_id", _srcWarehouse->id());
  if (_destWarehouse->isSelected())
    params.append("dest_warehous_id", _destWarehouse->id());
  if (_selectedStatus->isChecked())
  {
    const char *statusTypes[] = { "U", "O", "C" };
    QString status = QString(statusTypes[_statuses->currentIndex()]);
    params.append("tohead_status", status);
  }
  else
    params.append("excludeClosed");
  params.append("unreleased", tr("Unreleased"));
  params.append("open", tr("Open"));
  params.append("closed", tr("Closed"));
}

void transferOrders::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("ListTransferOrders", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void transferOrders::sNew()
{
  transferOrder::newTransferOrder(
		  (_srcWarehouse->isSelected()  ? _srcWarehouse->id()  : -1),
		  (_destWarehouse->isSelected() ? _destWarehouse->id() : -1));
}

void transferOrders::sEdit()
{
  transferOrder::editTransferOrder(_to->id(), false);
}

void transferOrders::sView()
{
  transferOrder::viewTransferOrder(_to->id());
}

void transferOrders::sCopy()
{
  ParameterList params;
  params.append("tohead_id", _to->id());
      
  copyTransferOrder newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void transferOrders::sRelease()
{
	q.prepare( "SELECT releaseTransferOrder(:tohead_id) AS result;");
	q.bindValue(":tohead_id", _to->id());
	q.exec();
	if (q.first())
	{
	  int result = q.value("result").toInt();
	  if (result < 0)
	  {
	    systemError(this, storedProcErrorLookup("releaseTransferOrder", result), __FILE__, __LINE__);
	    return;
	  }
	}
	else if (q.lastError().type() != QSqlError::NoError)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
  
	sFillList();
}

void transferOrders::sIssue()
{
  ParameterList params;
  params.append("tohead_id", _to->id());

  issueToShipping *newdlg = new issueToShipping();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  sFillList();
}

void transferOrders::sDelete()
{
  if ( QMessageBox::question(this, tr("Delete Transfer Order?"),
                             tr("<p>Are you sure that you want to completely "
				"delete the selected Transfer Order?"),
			     QMessageBox::Yes,
			     QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT deleteTo(:tohead_id) AS result;");
    q.bindValue(":tohead_id", _to->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result == -1)
      {
	if (QMessageBox::question(this, tr("Cannot Delete Transfer Order"),
				  tr("<p>The selected Transfer Order cannot be "
				     "deleted as there have been shipments "
				     "posted against it. Would you like to "
				     "Close it instead?"),
				  QMessageBox::Yes,
				  QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
	  return;

	q.prepare( "SELECT closeTransferOrder(:tohead_id) AS result;");
	q.bindValue(":tohead_id", _to->id());
	q.exec();
	if (q.first())
	{
	  int result = q.value("result").toInt();
	  if (result < 0)
	  {
	    systemError(this, storedProcErrorLookup("closeTransferOrder", result), __FILE__, __LINE__);
	    return;
	  }
	}
	else if (q.lastError().type() != QSqlError::NoError)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
  
	sFillList();
      }
      else if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteTo", result),
		    __FILE__, __LINE__);
	return;
      }
      omfgThis->sTransferOrdersUpdated(-1);
      omfgThis->sProjectsUpdated(-1);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void transferOrders::sPrintPackingList()
{
  ParameterList params;
  params.append("tohead_id", _to->id());
  params.append("print");

  printPackingList newdlg(this, "", TRUE);
  if (newdlg.set(params) != NoError_Print)
    newdlg.exec();
}

void transferOrders::sAddToPackingListBatch()
{
  q.prepare("SELECT addToPackingListBatch('TO', :tohead_id) AS result;");
  q.bindValue(":tohead_id", _to->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("addToPackingListBatch", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void transferOrders::sHandleButtons()
{
  XTreeWidgetItem *selected = 0;
  _numSelected = 0;

  QList<XTreeWidgetItem*> selectedlist = _to->selectedItems();
  _numSelected = selectedlist.size();
  if (_numSelected > 0)
    selected = (XTreeWidgetItem*)(selectedlist[0]);

  if (selected)
  {
    if (selected->altId() == 1 || selected->altId() == 2)
    {
      if (!_privileges->check("MaintainTransferOrders"))
      {
        _edit->setEnabled(FALSE);
        _delete->setEnabled(FALSE);
      }
      else
      {
        _edit->setEnabled(TRUE);
        _delete->setEnabled(TRUE);
      }
    }
    else
    {
      _edit->setEnabled(FALSE);
      _delete->setEnabled(FALSE);
    }

    if (selected->altId() == 1)
    {  
      if (!_privileges->check("ReleaseTransferOrders"))
        _release->setEnabled(FALSE);
      else
        _release->setEnabled(TRUE);
    }
    else
      _release->setEnabled(FALSE);

    if (!_privileges->check("MaintainTransferOrders"))
      _copy->setEnabled(FALSE);
    else
      _copy->setEnabled(TRUE);

    if (selected->altId() == 2)
    {
      if (!_privileges->check("IssueStockToShipping"))
        _issue->setEnabled(FALSE);
      else
        _issue->setEnabled(TRUE);
    }
    else
      _issue->setEnabled(FALSE);
  }
}

void transferOrders::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  QAction *menuItem;
  XTreeWidgetItem * item = (XTreeWidgetItem*)pSelected;

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  if (item->altId() == 1 || item->altId() == 2)
  {
    menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("MaintainTransferOrders"));

    menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
    menuItem->setEnabled(_privileges->check("MaintainTransferOrders"));
  }

  if (item->altId() == 1)
  {  
    menuItem = pMenu->addAction(tr("Release..."), this, SLOT(sRelease()));
    menuItem->setEnabled(_privileges->check("ReleaseTransferOrders"));
  }

  menuItem = pMenu->addAction(tr("Copy..."), this, SLOT(sCopy()));
  menuItem->setEnabled(_privileges->check("MaintainTransferOrders"));

  if (item->altId() == 2)
  {
    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Issue To Shipping..."), this, SLOT(sIssue()));
    menuItem->setEnabled(_privileges->check("IssueStockToShipping"));

    menuItem = pMenu->addAction(tr("Print Packing List..."), this, SLOT(sPrintPackingList()));
    menuItem->setEnabled(_privileges->check("PrintPackingLists"));

    menuItem = pMenu->addAction(tr("Add to Packing List Batch..."), this, SLOT(sAddToPackingListBatch()));
    menuItem->setEnabled(_privileges->check("MaintainPackingListBatch"));
  }
}

void transferOrders::sFillList()
{
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql = mqlLoad("transferOrders", "detail");
  XSqlQuery r = mql.toQuery(params);
  _to->populate(r, true);
  if (r.lastError().type() != QSqlError::NoError)
  {
    systemError(this, r.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _to->setDragString("toheadid=");
  sHandleButtons();
}
