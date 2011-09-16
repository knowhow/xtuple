/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "contacts.h"

#include <QAction>
#include <QMenu>
#include <QSqlError>
#include <QToolBar>
#include <QToolButton>
#include <QVariant>
#include <QMessageBox>

#include "contact.h"
#include "parameterwidget.h"
#include "storedProcErrorLookup.h"

contacts::contacts(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "contacts", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Contacts"));
  setReportName("ContactsMasterList");
  setMetaSQLOptions("contacts", "detail");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);

  _crmacctid = -1;
  _attachAct = 0;
  _detachAct = 0;

  parameterWidget()->append(tr("CRM Account"), "crmacct_id", ParameterWidget::Crmacct);
  parameterWidget()->append(tr("Name Pattern"), "cntct_name_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Phone Pattern"), "cntct_phone_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Email Pattern"), "cntct_email_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Street Pattern"), "addr_street_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("City Pattern"), "addr_city_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("State Pattern"), "addr_state_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Postal Code Pattern"), "addr_postalcode_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Country Pattern"), "addr_country_pattern", ParameterWidget::Text);

  parameterWidget()->applyDefaultFilterSet();

  list()->addColumn(tr("First Name"), 80, Qt::AlignLeft, true, "cntct_first_name");
  list()->addColumn(tr("Last Name"), 100, Qt::AlignLeft, true, "cntct_last_name");
  list()->addColumn(tr("Account #"), 80, Qt::AlignLeft, true, "crmacct_number");
  list()->addColumn(tr("Account Name"), -1, Qt::AlignLeft, true, "crmacct_name");
  list()->addColumn(tr("Title"), -1, Qt::AlignLeft, true, "cntct_title");
  list()->addColumn(tr("Phone"),	100, Qt::AlignLeft, true, "cntct_phone");
  list()->addColumn(tr("Alternate"), 100, Qt::AlignLeft, true, "cntct_phone2");
  list()->addColumn(tr("Fax"), 100, Qt::AlignLeft, false, "cntct_fax");
  list()->addColumn(tr("E-Mail"), 100, Qt::AlignLeft, true, "cntct_email");
  list()->addColumn(tr("Web Address"),  100, Qt::AlignLeft, false, "cntct_webaddr");
  list()->addColumn(tr("Address"), -1, Qt::AlignLeft, false, "addr_line1");
  list()->addColumn(tr("City"), 75, Qt::AlignLeft, false, "addr_city");
  list()->addColumn(tr("State"), 50, Qt::AlignLeft, false, "addr_state");
  list()->addColumn(tr("Country"), 100, Qt::AlignLeft, false, "addr_country");
  list()->addColumn(tr("Postal Code"), 75, Qt::AlignLeft, false, "addr_postalcode");

  list()->setSelectionMode(QAbstractItemView::ExtendedSelection);

  QToolButton * attachBtn = new QToolButton(this);
  attachBtn->setText(tr("Attach"));
  _attachAct = toolBar()->insertWidget(filterSeparator(), attachBtn);
  _attachAct->setEnabled(false);
  _attachAct->setVisible(false);

  QToolButton * detachBtn = new QToolButton(this);
  detachBtn->setText(tr("Detach"));
  _detachAct = toolBar()->insertWidget(filterSeparator(), detachBtn);
  _detachAct->setEnabled(false);
  _detachAct->setVisible(false);

  connect(attachBtn, SIGNAL(clicked()),      this, SLOT(sAttach()));
  connect(detachBtn, SIGNAL(clicked()),      this, SLOT(sDetach()));

  if (_privileges->check("MaintainContacts"))
  {
    _attachAct->setEnabled(true);
    connect(list(), SIGNAL(valid(bool)), _detachAct, SLOT(setEnabled(bool)));
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  }
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }
}

enum SetResponse contacts::set(const ParameterList& pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool	   valid;
  
  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "view")
    {
      _attachAct->setEnabled(false);
      disconnect(list(), SIGNAL(valid(bool)), _detachAct, SLOT(setEnabled(bool)));
    }
  }

  param = pParams.value("run", &valid);
  if (valid)
    sFillList();

  return NoError;
}

void contacts::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainContacts"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  XSqlQuery chk;
  chk.prepare("SELECT cntctused(:cntct_id) AS inUse");
  chk.bindValue(":cntct_id", list()->id());
  chk.exec();
  if (chk.lastError().type() != QSqlError::NoError) {
    systemError(this, chk.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if (chk.first() && !chk.value("inUse").toBool()) {
    menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
    menuItem->setEnabled(_privileges->check("MaintainContacts"));
  }
}

void contacts::sNew()
{
  ParameterList params;
  setParams(params);
  params.append("mode", "new");

  contact newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void contacts::sEdit()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("cntct_id", ((XTreeWidgetItem*)(selected[i]))->id());

    contact newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
  sFillList();
}

void contacts::sView()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("cntct_id", ((XTreeWidgetItem*)(selected[i]))->id());

    contact newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void contacts::sDelete()
{

  if ( QMessageBox::warning(this, tr("Delete Contact?"),
                            tr("<p>Are you sure that you want to completely "
			       "delete the selected contact?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT deleteContact(:cntct_id) AS result;");

    QList<XTreeWidgetItem*> selected = list()->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      q.bindValue(":cntct_id", ((XTreeWidgetItem*)(selected[i]))->id());
      q.exec();

      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("deleteContact", result), __FILE__, __LINE__);
          return;
        }
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }
  sFillList();
}

void contacts::sAttach()
{
  ContactCluster attached(this, "attached");
  attached.sEllipses();
  if (attached.id() > 0)
  {
    int answer = QMessageBox::Yes;

    if (attached.crmAcctId() > 0 && attached.crmAcctId() != _crmacctid)
      answer = QMessageBox::question(this, tr("Detach Contact?"),
			    tr("<p>This Contact is currently attached to a "
			       "different CRM Account. Are you sure you want "
			       "to change the CRM Account for this person?"),
			    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    if (answer == QMessageBox::Yes)
    {
      q.prepare("SELECT attachContact(:cntct_id, :crmacct_id) AS returnVal;");
      q.bindValue(":cntct_id", attached.id());
      q.bindValue(":crmacct_id", _crmacctid);
      q.exec();
      if (q.first())
      {
	int returnVal = q.value("returnVal").toInt();
	if (returnVal < 0)
	{
	  systemError(this, storedProcErrorLookup("attachContact", returnVal),
			    __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    sFillList();
  }
}

void contacts::sDetach()
{
  int answer = QMessageBox::question(this, tr("Detach Contact?"),
			tr("<p>Are you sure you want to detach this Contact "
			   "from this CRM Account?"),
			QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
  if (answer == QMessageBox::Yes)
  {
    int cntctId = list()->id();
    q.prepare("SELECT detachContact(:cntct_id, :crmacct_id) AS returnVal;");
    q.bindValue(":cntct_id", cntctId);
    q.bindValue(":crmacct_id", _crmacctid);
    q.exec();
    if (q.first())
    {
      int returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
	systemError(this, tr("Error detaching Contact from CRM Account (%1).")
			  .arg(returnVal), __FILE__, __LINE__);
	return;
      }
      emit cntctDetached(cntctId);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void contacts::setCrmacctid(int crmacctId)
{
  _crmacctid = crmacctId;
  if (_crmacctid == -1)
  {
    parameterWidget()->setDefault(tr("CRM Account"), QVariant(), true);
    _attachAct->setVisible(false);
    _detachAct->setVisible(false);
  }
  else
  {
    parameterWidget()->setDefault(tr("CRM Account"), _crmacctid, true);
    _attachAct->setVisible(true);
    _detachAct->setVisible(true);
  }
}

int contacts::crmacctId()
{
  return _crmacctid;
}

QAction* contacts::attachAction()
{
  return _attachAct;
}

QAction* contacts::detachAction()
{
  return _detachAct;
}

bool contacts::setParams(ParameterList &params)
{
  display::setParams(params);
  if (_activeOnly->isChecked())
    params.append("activeOnly",true);

  return true;
}
