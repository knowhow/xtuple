/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspGLSeries.h"
#include "glSeries.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include "reverseGLSeries.h"
#include "dspJournals.h"

dspGLSeries::dspGLSeries(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspGLSeries", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Journal Series"));
  setListLabel(tr("Journal Transactions"));
  setReportName("GLSeries");
  setMetaSQLOptions("glseries", "detail");
  setUseAltId(true);

  list()->addColumn(tr("Date"),      _dateColumn, Qt::AlignCenter,true, "transdate");
  list()->addColumn(tr("Journal #"),_orderColumn, Qt::AlignRight, true, "journalnumber");
  list()->addColumn(tr("Source"),   _orderColumn, Qt::AlignCenter,true, "source");
  list()->addColumn(tr("Doc. Type"), _itemColumn, Qt::AlignCenter,true, "doctype");
  list()->addColumn(tr("Doc. Num."),_orderColumn, Qt::AlignCenter,true, "docnumber");
  list()->addColumn(tr("Notes/Account"),      -1, Qt::AlignLeft,  true, "accnt_id");
  list()->addColumn(tr("Debit"), _bigMoneyColumn, Qt::AlignRight, true, "debit");
  list()->addColumn(tr("Credit"),_bigMoneyColumn, Qt::AlignRight, true, "credit");
  list()->addColumn(tr("Posted"),      _ynColumn, Qt::AlignCenter,true, "posted");

  if (!_metrics->boolean("UseJournals"))
    _typeGroup->hide();

  _isJournal = false;

  _journal->setEnabled(_privileges->boolean("ViewJournals"));
}

void dspGLSeries::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspGLSeries::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("startDate", &valid);
  if(valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if(valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("journalnumber", &valid);
  if(valid)
  {
    _jrnlGroup->setChecked(true);
    _startJrnlnum->setText(param.toString());
    _endJrnlnum->setText(param.toString());
  }

  param = pParams.value("journal", &valid);
  if(valid)
  {
    _typeGroup->show();
    _journal->setChecked(true);
  }

  sFillList();

  return NoError;
}

void dspGLSeries::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  bool editable = false;
  bool deletable = false;
  bool reversible = false;
  XTreeWidgetItem * item = (XTreeWidgetItem*)list()->currentItem();
  if(0 != item)
  {
    if(item->altId() != -1)
      item = (XTreeWidgetItem*)item->QTreeWidgetItem::parent();
    if(0 != item)
    {
      if (_journal->isChecked() && !item->rawValue("posted").toBool())
      {
        menuItem = pMenu->addAction(tr("Post..."), this, SLOT(sPost()));
        menuItem->setEnabled(_privileges->check("PostJournals"));

        return;
      }
      else
      {
        if(item->rawValue("doctype").toString() == "ST" ||
           item->rawValue("doctype").toString() == "JE")
          reversible = _privileges->check("PostStandardJournals");

        // Make sure there is nothing to restricting edits
        ParameterList params;
        params.append("glSequence", list()->id());
        MetaSQLQuery mql = mqlLoad("glseries", "checkeditable");
        XSqlQuery qry = mql.toQuery(params);
        if (!qry.first())
        {
          editable = _privileges->check("EditPostedJournals") &&
                     item->rawValue("doctype").toString() == "JE";
          deletable = (_privileges->check("DeletePostedJournals") &&
                       (reversible || item->rawValue("doctype").toString() == "JP"));
        }
      }
    }
  }

  if (!_isJournal)
  {
    menuItem = pMenu->addAction(tr("Edit Journal..."), this, SLOT(sEdit()));
    menuItem->setEnabled(editable);

    menuItem = pMenu->addAction(tr("Delete Journal..."), this, SLOT(sDelete()));
    menuItem->setEnabled(deletable);

    pMenu->addSeparator();
  }

  menuItem = pMenu->addAction(tr("Reverse Journal..."), this, SLOT(sReverse()));
  menuItem->setEnabled(reversible);

  if (item->rawValue("doctype").toString() == "JP")
  {
    pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("View Journal..."), this, SLOT(sViewJournal()));
    menuItem->setEnabled(_privileges->check("ViewJournals"));
  }

}

bool dspGLSeries::setParams(ParameterList &params)
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date Range"),
                         tr("You must first specify a valid date range.") );
    _dates->setFocus();
    return false;
  }

  _dates->appendValue(params);

  if(_selectedSource->isChecked())
    params.append("source", _source->currentText());

  if(_jrnlGroup->isChecked())
  {
    params.append("startJrnlnum", _startJrnlnum->text().toInt());
    params.append("endJrnlnum", _endJrnlnum->text().toInt());
  }

  if (_journal->isChecked())
  {
    params.append("title",tr("Journal Series"));
    params.append("table", "sltrans");
    _isJournal=true;
  }
  else
  {
    params.append("title",tr("General Ledger Series"));
    params.append("gltrans", true);
    params.append("table", "gltrans");
    _isJournal=false;
  }

  return true;
}

void dspGLSeries::sEdit()
{
  ParameterList params;
  ParameterList params2;
  params.append("glSequence", list()->id());

  MetaSQLQuery mql("SELECT copyGlSeries(gltrans_sequence) AS sequence,"
                   "  gltrans_journalnumber "
                   "FROM gltrans "
                   "WHERE (gltrans_sequence=<? value(\"glSequence\") ?>) "
                   "LIMIT 1;");
  XSqlQuery qry;
  qry = mql.toQuery(params);
  if (qry.first())
  {
    params2.append("glSequence", qry.value("sequence"));
    params2.append("journalnumber", qry.value("gltrans_journalnumber"));
  }
  else if (qry.lastError().type() != QSqlError::NoError)
  {
    systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    return;

  glSeries newdlg(this);
  if(newdlg.set(params2) != NoError)
    return;
  if(newdlg.exec() == XDialog::Accepted)
    sDelete(true);
}

void dspGLSeries::sReverse()
{
  ParameterList params;
  params.append("glseries", list()->id());

  reverseGLSeries newdlg(this);
  if(newdlg.set(params) != NoError)
    return;
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void dspGLSeries::sDelete(bool edited)
{
  QString action = tr("deleted");
  if (edited)
    action = tr("edited");

  ParameterList params;
  params.append("glSequence", list()->id());
  params.append("notes", tr("Journal %1 by %2 on %3")
                .arg(action)
                .arg(omfgThis->username())
                .arg(omfgThis->dbDate().toString()));

  MetaSQLQuery mql("SELECT deleteGlSeries(<? value(\"glSequence\") ?>, "
                   " <? value(\"notes\") ?>) AS result;");
  XSqlQuery del;
  del = mql.toQuery(params);
  if (del.lastError().type() != QSqlError::NoError)
  {
    systemError(this, del.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sGlSeriesUpdated();
  sFillList();
}

void dspGLSeries::sPost()
{
  ParameterList params;
  params.append("sequence", list()->id());
  MetaSQLQuery mql = mqlLoad("glseries", "postjournal");
  XSqlQuery qry = mql.toQuery(params);
  if (qry.lastError().type() != QSqlError::NoError)
  {
    systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void dspGLSeries::sViewJournal()
{
  ParameterList params;

  params.append("startDate", omfgThis->startOfTime());
  params.append("endDate", omfgThis->endOfTime());
  if (list()->rawValue("accnt_id").toInt() == -1)
    params.append("journalnumber", list()->rawValue("journalnumber"));
  else
  {
    params.append("journalnumber", list()->rawValue("docnumber"));
    params.append("accnt_id", list()->rawValue("accnt_id").toInt());
  }
  params.append("run");

  dspJournals *newdlg = new dspJournals();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

