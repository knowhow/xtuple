/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "unpostedGlSeries.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <parameter.h>
#include <openreports.h>

#include "failedPostList.h"
#include "getGLDistDate.h"
#include "glSeries.h"
#include "storedProcErrorLookup.h"

unpostedGlSeries::unpostedGlSeries(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_delete,   SIGNAL(clicked()),		this, SLOT(sDelete()));
  connect(_edit,     SIGNAL(clicked()),		this, SLOT(sEdit()));
  connect(_glseries, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_new,	     SIGNAL(clicked()),		this, SLOT(sNew()));
  connect(_post,     SIGNAL(clicked()),		this, SLOT(sPost()));
  connect(_print,    SIGNAL(clicked()),		this, SLOT(sPrint()));
  connect(_view,     SIGNAL(clicked()),		this, SLOT(sView()));
  connect(omfgThis,  SIGNAL(glSeriesUpdated()),	this, SLOT(sFillList()));

  _glseries->addColumn(tr("Date"),          _dateColumn,     Qt::AlignCenter, true,  "glseries_distdate" );
  _glseries->addColumn(tr("Source"),        _orderColumn,    Qt::AlignCenter, true,  "glseries_source" );
  _glseries->addColumn(tr("Doc. Type"),     _docTypeColumn,  Qt::AlignCenter, true,  "glseries_doctype" );
  _glseries->addColumn(tr("Doc. #"),        _orderColumn,    Qt::AlignCenter, true,  "glseries_docnumber" );
  _glseries->addColumn(tr("Reference"),     -1,              Qt::AlignLeft,   true,  "glseries_notes"   );
  _glseries->addColumn(tr("Account"),       -1,              Qt::AlignLeft,   true,  "account"   );
  _glseries->addColumn(tr("Debit"),         _moneyColumn,    Qt::AlignRight,  true,  "debit"  );
  _glseries->addColumn(tr("Credit"),        _moneyColumn,    Qt::AlignRight,  true,  "credit"  );

  sFillList();
}

unpostedGlSeries::~unpostedGlSeries()
{
  // no need to delete child widgets, Qt does it all for us
}

void unpostedGlSeries::languageChange()
{
  retranslateUi(this);
}

void unpostedGlSeries::sPrint()
{
  orReport report("UnpostedGlSeries");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void unpostedGlSeries::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  glSeries *newdlg = new glSeries();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedGlSeries::sEdit()
{
  QList<XTreeWidgetItem*>selected = _glseries->selectedItems();
  removeDupAltIds(selected);
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("mode",	"edit");
    params.append("glSequence",	((XTreeWidgetItem*)(selected[i]))->altId() );

    glSeries *newdlg = new glSeries();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void unpostedGlSeries::sDelete()
{
  if (QMessageBox::question(this, tr("Cancel G/L Transactions?"),
			    tr("<p>Are you sure you want to delete these "
			       "unposted G/L Transactions?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare( "DELETE FROM glseries "
	       "WHERE (glseries_sequence=:id);");
    QList<XTreeWidgetItem*>selected = _glseries->selectedItems();
    removeDupAltIds(selected);
    for (int i = 0; i < selected.size(); i++)
    {
      q.bindValue(":id", ((XTreeWidgetItem*)(selected[i]))->altId() );
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    omfgThis->sGlSeriesUpdated();
  }
}

void unpostedGlSeries::sView()
{
  QList<XTreeWidgetItem*>selected = _glseries->selectedItems();
  removeDupAltIds(selected);
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("mode",	"view");
    params.append("glSequence",	((XTreeWidgetItem*)(selected[i]))->altId() );

    glSeries *newdlg = new glSeries();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void unpostedGlSeries::sPost()
{
  QDate newDate = QDate::currentDate();

  QList<XTreeWidgetItem*>selected = _glseries->selectedItems();
  removeDupAltIds(selected);
  QList<XTreeWidgetItem*>triedToClosed;

  XSqlQuery post;
  post.prepare("SELECT postGLSeriesNoSumm(:sequence) AS result;");

  bool tryagain = false;
  do {
    for (int i = 0; i < selected.size(); i++)
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->altId();

      post.bindValue(":sequence", id);
      post.exec();
      if (post.first())
      {
	int result = post.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("postGLSeriesNoSumm", result),
		      __FILE__, __LINE__);
	  continue;
	}
      }
      // contains() string is hard-coded in stored procedure
      else if (post.lastError().databaseText().contains("post to closed period"))
      {
	triedToClosed.append(selected[i]);
      }
      else if (post.lastError().type() != QSqlError::NoError)
      {
	systemError(this, post.lastError().databaseText(), __FILE__, __LINE__);
      }
    } // for each selected line

    if (triedToClosed.size() > 0)
    {
      failedPostList newdlg(this, "", true);
      newdlg.sSetList(triedToClosed, _glseries->headerItem(), _glseries->header());
      tryagain = (newdlg.exec() == XDialog::Accepted);
      selected = triedToClosed;
      triedToClosed.clear();
    }
  } while (tryagain);

  omfgThis->sGlSeriesUpdated();
}

void unpostedGlSeries::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit G/L Series..."),  this, SLOT(sEdit()));
  menuItem = pMenu->addAction(tr("View G/L Series..."),  this, SLOT(sView()));
  menuItem = pMenu->addAction(tr("Delete G/L Series..."),this, SLOT(sDelete()));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Post G/L Series..."),  this, SLOT(sPost()));

  pMenu->addSeparator();
}

void unpostedGlSeries::sFillList()
{
  XSqlQuery fillq;
  fillq.prepare("SELECT *, "
                "       (formatGLAccount(glseries_accnt_id) || ' - ' || accnt_descrip) AS account,"
                "       CASE WHEN (glseries_amount < 0) THEN (glseries_amount * -1)"
                "            ELSE 0 END AS debit,"
                "       CASE WHEN (glseries_amount >= 0) THEN (glseries_amount)"
                "            ELSE 0 END AS credit,"
                "       'curr' AS debit_xtnumericrole,"
                "       'curr' AS credit_xtnumericrole,"
                "       CASE WHEN (glseries_amount < 0) THEN '' END AS credit_qtdisplayrole,"
                "       CASE WHEN (glseries_amount >= 0) THEN '' END AS debit_qtdisplayrole "
                "FROM glseries, accnt "
                "WHERE (glseries_accnt_id=accnt_id) "
                "ORDER BY glseries_distdate, glseries_sequence, glseries_amount;");
  fillq.exec();
  _glseries->populate(fillq, true);
  if (fillq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, fillq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void unpostedGlSeries::removeDupAltIds(QList<XTreeWidgetItem*> & list)
{
  for (int i = 0; i < list.size(); i++)
  {
    for (int j = i + 1; j < list.size(); j++)
    {
      if (((XTreeWidgetItem*)(list[i]))->altId() ==
	  ((XTreeWidgetItem*)(list[j]))->altId())
	list.removeAt(j);
    }
  }
}
