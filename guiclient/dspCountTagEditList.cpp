/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCountTagEditList.h"

#include <math.h>

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <parameter.h>
#include <openreports.h>

#include <metasql.h>
#include "mqlutil.h"

#include "countSlip.h"
#include "countTag.h"
#include "dspInventoryHistory.h"
#include "dspCountSlipEditList.h"
#include "storedProcErrorLookup.h"

dspCountTagEditList::dspCountTagEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _highlightGroupInt = new QButtonGroup(this);
  _highlightGroupInt->addButton(_noHighlight);
  _highlightGroupInt->addButton(_highlightValue);
  _highlightGroupInt->addButton(_highlightPercent);

  _codeGroup = new QButtonGroup(this);
  _codeGroup->addButton(_plancode);
  _codeGroup->addButton(_classcode);

  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_showSlips, SIGNAL(clicked()), this, SLOT(sToggleList()));
  connect(_enterSlip, SIGNAL(clicked()), this, SLOT(sEnterCountSlip()));
  connect(_cnttag, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_codeGroup, SIGNAL(buttonClicked(int)), this, SLOT(sParameterTypeChanged()));

  _parameter->setType(ParameterGroup::ClassCode);
  _variancePercent->setValidator(omfgThis->percentVal());

  _cnttag->setRootIsDecorated(TRUE);
  _cnttag->addColumn(tr("Pri."), (_whsColumn + 10), Qt::AlignCenter,true, "invcnt_priority");
  _cnttag->addColumn(tr("Tag/Slip #"),_orderColumn, Qt::AlignRight, true, "tagnumber");
  _cnttag->addColumn(tr("Tag Date"),   _dateColumn, Qt::AlignCenter,true, "tagdate");
  _cnttag->addColumn(tr("Item Number"),         -1, Qt::AlignLeft,  true, "item_number");
  _cnttag->addColumn(tr("Site"),        _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _cnttag->addColumn(tr("Location"),     _ynColumn, Qt::AlignCenter,true, "loc_specific");
  _cnttag->addColumn(tr("QOH"),         _qtyColumn, Qt::AlignRight, true, "qoh");
  _cnttag->addColumn(tr("NN QOH"),      _qtyColumn, Qt::AlignRight, true, "nnqoh");
  _cnttag->addColumn(tr("Count Qty."),  _qtyColumn, Qt::AlignRight, true, "qohafter");
  _cnttag->addColumn(tr("Variance"),    _qtyColumn, Qt::AlignRight, true, "variance");
  _cnttag->addColumn(tr("%"),         _prcntColumn, Qt::AlignRight, true, "varianceprcnt");
  _cnttag->addColumn(tr("Amount"),     _costColumn, Qt::AlignRight, true, "variancecost");
  _cnttag->setIndentation(10);
  
  if (_privileges->check("EnterCountTags"))
  {
    connect(_cnttag, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_cnttag, SIGNAL(valid(bool)), this, SLOT(sHandleButtons(bool)));
  }

  if (_privileges->check("DeleteCountTags"))
    connect(_cnttag, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

  if (_privileges->check("PostCountTags"))
    connect(_cnttag, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));

  if (_privileges->check("EnterCountSlips"))
    connect(_cnttag, SIGNAL(valid(bool)), _enterSlip, SLOT(setEnabled(bool)));

  _searchFor->setFocus();
}

dspCountTagEditList::~dspCountTagEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCountTagEditList::languageChange()
{
  retranslateUi(this);
}

void dspCountTagEditList::sToggleList()
{
  if (_showSlips->isChecked())
    _cnttag->setSelectionMode(QAbstractItemView::SingleSelection);
  else
    _cnttag->setSelectionMode(QAbstractItemView::ExtendedSelection);
  sFillList();
}

void dspCountTagEditList::setParams(ParameterList &params)
{
  if (_parameter->type() == ParameterGroup::ClassCode)
    params.append("ParameterType", "ClassCode");
  else
    params.append("ParameterType", "PlannerCode");
  _warehouse->appendValue(params);
  _parameter->appendValue(params);

  params.append("all",	    tr("All"));
  params.append("posted",   tr("Posted"));
  params.append("unposted", tr("Unposted"));
  if (_showSlips->isChecked())
    params.append("showSlips");
  if (_highlightValue->isChecked())
    params.append("varianceValue",   _varianceValue->localValue());
  else if (_highlightPercent->isChecked())
    params.append("variancePercent", _variancePercent->toDouble());
}

void dspCountTagEditList::sPrint()
{
  ParameterList params;
  _parameter->appendValue(params);
  _warehouse->appendValue(params);

  params.append("maxTags", 10000);

  orReport report("CountTagEditList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCountTagEditList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  QAction *menuItem;

  if (((XTreeWidgetItem *)pSelected)->altId() == -1)
  {
    menuItem = pMenu->addAction("Enter Count Slip...", this, SLOT(sEnterCountSlip()));
    if (!_privileges->check("EnterCountSlips"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction("Count Slip Edit List...", this, SLOT(sCountSlipEditList()));

    pMenu->addSeparator();

    menuItem = pMenu->addAction("View Pending Inventory History...", this, SLOT(sViewInventoryHistory()));
    if (!_privileges->check("ViewInventoryHistory"))
      menuItem->setEnabled(false);

    pMenu->addSeparator();

    menuItem = pMenu->addAction("Edit Count Tag...", this, SLOT(sEdit()));
    if (!_privileges->check("EnterCountTags"))
      menuItem->setEnabled(false);

    if (pSelected->text(5) != "")
    {
      menuItem = pMenu->addAction("Post Count Tag...", this, SLOT(sPost()));
      if (!_privileges->check("PostCountTags"))
        menuItem->setEnabled(false);
    }

    menuItem = pMenu->addAction("Delete Count Tag", this, SLOT(sDelete()));
    if (!_privileges->check("DeleteCountTags"))
      menuItem->setEnabled(false);
  }
  else
  {
    if (pSelected->text(3) == tr("Unposted"))
    {
      menuItem = pMenu->addAction("Edit Count Slip...", this, SLOT(sEdit()));
      if (!_privileges->check("EnterCountSlips"))
        menuItem->setEnabled(false);
    }
  }
}

void dspCountTagEditList::sEnterCountSlip()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cnttag_id", _cnttag->id());
  
  countSlip newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillList();

    _searchFor->setFocus();
    _searchFor->setSelection(0, _searchFor->text().length());
  }
}

void dspCountTagEditList::sCountSlipEditList()
{
  ParameterList params;
  params.append("cnttag_id", _cnttag->id());
  params.append("run");

  dspCountSlipEditList *newdlg = new dspCountSlipEditList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCountTagEditList::sViewInventoryHistory()
{
  q.prepare( "SELECT invcnt_itemsite_id, invcnt_tagdate "
                 "FROM invcnt "
                 "WHERE (invcnt_id=:invcnt_id);" );
  q.bindValue(":invcnt_id", _cnttag->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", q.value("invcnt_itemsite_id").toInt());
    params.append("startDate", q.value("invcnt_tagdate").toDate());
    params.append("endDate", omfgThis->dbDate());
    params.append("run");

    dspInventoryHistory *newdlg = new dspInventoryHistory();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspCountTagEditList::sEdit()
{
  bool update  = FALSE;
  if ( (_showSlips->isChecked()) &&
      (((XTreeWidgetItem *)_cnttag->currentItem())->altId() == -1) )
    sEditTag();
  else if (_showSlips->isChecked())
    sEditSlip();
  else
  {
    QList<XTreeWidgetItem*> selected = _cnttag->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      ParameterList params;
      params.append("mode", "edit");
      params.append("cnttag_id", ((XTreeWidgetItem*)(selected[i]))->id());

      countTag newdlg(this, "", TRUE);
      newdlg.set(params);

      if (newdlg.exec() != XDialog::Rejected)
	update = TRUE;
    }
    if (update)
      sFillList();
  }
}

void dspCountTagEditList::sEditTag()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cnttag_id", _cnttag->id());

  countTag newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCountTagEditList::sEditSlip()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cntslip_id", _cnttag->altId());
  
  countSlip newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCountTagEditList::sDelete()
{
  if ( (_showSlips->isChecked()) &&
      (((XTreeWidgetItem *)_cnttag->currentItem())->altId() == -1) )
    sDeleteTag();
  else if (_showSlips->isChecked())
    sDeleteSlip();
  else
  {
    QList<XTreeWidgetItem*> selected = _cnttag->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      int itemsiteid = -1;

      q.prepare( "SELECT cntslip_id FROM cntslip "
		 "WHERE (cntslip_cnttag_id=:cnttag_id);" );
      q.bindValue(":cnttag_id", ((XTreeWidgetItem*)(selected[i]))->id());
      q.exec();
      if (q.first())
      {
	QMessageBox::critical( this, tr("Cannot Delete Count tag"),
			      tr("<p>There are Count Slips entered for this "
				 "Count Tag. You must delete Count Slips for "
				 "the Count Tag before you may delete this Tag.") );
	continue;
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
      q.prepare("SELECT itemsite_id"
	        "  FROM invcnt JOIN itemsite ON (invcnt_itemsite_id=itemsite_id AND itemsite_freeze) "
		" WHERE (invcnt_id=:cnttag_id); ");
      q.bindValue(":cnttag_id", ((XTreeWidgetItem*)(selected[i]))->id());
      if(q.exec() && q.first())
	itemsiteid = q.value("itemsite_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }

      q.prepare( "DELETE FROM cntslip "
	     "WHERE (cntslip_cnttag_id=:cnttag_id); "
	     "DELETE FROM invcnt "
	     "WHERE (invcnt_id=:cnttag_id); " );
      q.bindValue(":cnttag_id", ((XTreeWidgetItem*)(selected[i]))->id());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }

      if (itemsiteid != -1)
      {
	if (QMessageBox::question(this, tr("Unfreeze Itemsite?"),
				  tr("<p>The Itemsite for the Count Tag you "
				     "deleted is frozen. Would you like to "
				     "unfreeze the Itemsite at this time?"),
				  QMessageBox::Yes | QMessageBox::Default,
				  QMessageBox::No | QMessageBox::Escape ) == QMessageBox::Yes )
	{
	  q.prepare("SELECT thawItemsite(:itemsite_id) AS result;");
	  q.bindValue(":itemsite_id", itemsiteid);
	  q.exec();
	  if (q.first())
	  {
	    int result = q.value("result").toInt();
	    if (result < 0)
	    {
	      systemError(this, storedProcErrorLookup("thawItemsite", result),
			  __FILE__, __LINE__);
	      continue;
	    }
	    else if (q.lastError().type() != QSqlError::NoError)
	    {
	      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	      return;
	    }
	  }
	}
      }
    }
    sFillList();
  }
}

void dspCountTagEditList::sDeleteTag()
{
  int itemsiteid = -1;
  q.prepare( "SELECT cntslip_id FROM cntslip "
                 "WHERE (cntslip_cnttag_id=:cnttag_id);" );
  q.bindValue(":cnttag_id", _cnttag->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Count tag"),
                           tr("<p>There are Count Slips entered for this count "
			      "tag. You must delete Count Slips for the Count "
			      "Tag before you may delete this Tag.") );
    return;
  }
  q.prepare( "SELECT itemsite_id"
             "  FROM invcnt JOIN itemsite ON (invcnt_itemsite_id=itemsite_id AND itemsite_freeze) "
             " WHERE (invcnt_id=:cnttag_id); ");
  q.bindValue(":cnttag_id", _cnttag->id());
  if (q.exec() && q.first())
    itemsiteid = q.value("itemsite_id").toInt();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  q.prepare( "DELETE FROM invcnt "
	     "WHERE (invcnt_id=:cnttag_id);" );
  q.bindValue(":cnttag_id", _cnttag->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(itemsiteid != -1)
  {
    if (QMessageBox::question( this, tr("Unfreeze Itemsite?"),
			      tr("<p>The Itemsite for the Count Tag you deleted"
				 " is frozen. Would you like to unfreeze the "
				 "Itemsite at this time?"),
			      QMessageBox::Yes | QMessageBox::Default,
			      QMessageBox::No | QMessageBox::Escape ) == QMessageBox::Yes )
    {
      q.prepare("SELECT thawItemsite(:itemsite_id) AS result;");
      q.bindValue(":itemsite_id", itemsiteid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("thawItemsite", result),
		      __FILE__, __LINE__);
	  return;
	}
	else if (q.lastError().type() != QSqlError::NoError)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
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

void dspCountTagEditList::sDeleteSlip()
{
  q.prepare( "DELETE FROM cntslip "
                 "WHERE (cntslip_id=:cntslip_id);" );
  q.bindValue(":cntslip_id", _cnttag->altId());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void dspCountTagEditList::sPost()
{
  bool update  = FALSE;
  if ( (_showSlips->isChecked()) &&
      (((XTreeWidgetItem *)_cnttag->currentItem())->altId() == -1) )
    sPostTag();
  else if  (_showSlips->isChecked())
    sPostSlip();
  else
  {
    QList<XTreeWidgetItem*> selected = _cnttag->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      if (((XTreeWidgetItem*)(selected[i]))->altId() == -1)
      {
	ParameterList params;
	params.append("mode", "post");
	params.append("cnttag_id", ((XTreeWidgetItem*)(selected[i]))->id());

	countTag newdlg(this, "", TRUE);
	newdlg.set(params);

	if (newdlg.exec() != XDialog::Rejected)
	  update = TRUE;
      }
    }
    if (update)
      sFillList();
  }
}

void dspCountTagEditList::sPostTag()
{
  ParameterList params;
  params.append("mode", "post");
  params.append("cnttag_id", _cnttag->id());

  countTag newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCountTagEditList::sPostSlip()
{
  ParameterList params;
  params.append("mode", "post");
  params.append("cntslip_id", _cnttag->altId());
  
  countSlip newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCountTagEditList::sSearch(const QString &pTarget)
{
  QList<XTreeWidgetItem*> search = _cnttag->findItems(pTarget,
						      Qt::MatchStartsWith, 3);

  if (search.size() > 0)
  {
    _cnttag->setCurrentItem(search[0], TRUE);
    _cnttag->scrollToItem(search[0]);
  }
}

void dspCountTagEditList::sFillList()
{
  MetaSQLQuery mql = mqlLoad("countTag", "detail");
  ParameterList params;
  setParams(params);

  q = mql.toQuery(params);
  _cnttag->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  _cnttag->expandAll();

  if (_searchFor->text().trimmed().length())
    sSearch(_searchFor->text());
}

void dspCountTagEditList::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

void dspCountTagEditList::sHandleButtons(bool valid)
{
  if (valid)
  {
    // Handle Edit Button
    if (_cnttag->currentItem()->altId() == -1) // count tag
      _edit->setEnabled(true);
    else if (_cnttag->currentItem()->rawValue("item_number") == tr("Unposted")) // unposted count slip
      _edit->setEnabled(true);
    else
      _edit->setEnabled(false);
  }
  else
    _edit->setEnabled(false);
}

void dspCountTagEditList::sParameterTypeChanged()
{
  if(_plancode->isChecked())
    _parameter->setType(ParameterGroup::PlannerCode);
  else
    _parameter->setType(ParameterGroup::ClassCode);

}
