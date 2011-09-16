/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspFinancialReport.h"

#include <QAction>
#include <QCloseEvent>
#include <QInputDialog>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QToolBar>
#include <QToolButton>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <orprerender.h>
#include <previewdialog.h>
#include <orprintrender.h>
#include "dspFinancialReport.h"
#include "dspGLTransactions.h"
#include "financialReportNotes.h"
#include "storedProcErrorLookup.h"

#define cFlRoot  0
#define cFlItem  1
#define cFlGroup 2
#define cFlSpec  3

#define cBegining 0
#define cEnding   1
#define cDebits   2
#define cCredits  3
#define cBudget   4
#define cDiff     5

dspFinancialReport::dspFinancialReport(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspFinancialReport", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("View Financial Report"));
  setListLabel(tr("Statement"));
  setReportName("dummy"); // This is really handled locally

  _prjid = -1;
  _col = 0;

  QToolBar *toolbar = this->toolBar();
  QToolButton *_notesBtn = new QToolButton();
  _notesBtn->setText(tr("Notes"));
  _notesAct = toolbar->insertWidget(filterSeparator(), _notesBtn);
  _notesAct->setEnabled(false);

  // signals and slots connections
  connect(list(), SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(sCollapsed(QTreeWidgetItem*)));
  connect(list(), SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(sExpanded(QTreeWidgetItem*)));
  connect(_periods, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_periods, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(sEditPeriodLabel()));
  connect(_periods, SIGNAL(valid(bool)), this, SLOT(sToggleNotes()));
  connect(_flhead, SIGNAL(newID(int)), this, SLOT(sReportChanged(int)));
  connect(_trend, SIGNAL(toggled(bool)), this, SLOT(sToggleTrend()));
  connect(_trend, SIGNAL(toggled(bool)), this, SLOT(sTogglePeriod()));
  connect(_trend, SIGNAL(toggled(bool)), this, SLOT(sFillPeriods()));
  connect(_month, SIGNAL(toggled(bool)), this, SLOT(sFillPeriods()));
  connect(_quarter, SIGNAL(toggled(bool)), this, SLOT(sFillPeriods()));
  connect(_year, SIGNAL(toggled(bool)), this, SLOT(sFillPeriods()));
  connect(_notesBtn, SIGNAL(clicked()), _notesAct, SLOT(trigger()));
  connect(_notesAct, SIGNAL(triggered()), this, SLOT(sNotes()));

  _flhead->setType(XComboBox::FinancialLayouts);

  _periods->addColumn(tr("Period"), _itemColumn, Qt::AlignLeft, true, "f_name");
  _periods->addColumn(tr("Alternate Label"), -1, Qt::AlignLeft, true, "alt_name");
  sFillPeriods();

  _tab->setTabEnabled(_tab->indexOf(_showColumnsTab),false);
  sReportChanged(_flhead->id());

  // Hide trend features until we can figure out how to make them print
  _actuals->hide();
  _budgets->hide();
}

enum SetResponse dspFinancialReport::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("flhead_id", &valid);
  if (valid)
    _flhead->setId(param.toInt());

  return NoError;
}

void dspFinancialReport::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int pColumn)
{
  if (_columnDates.contains(pColumn) && list()->id() > 0)
  {
    _col = pColumn;

    QAction* viewGlAct = pMenu->addAction(tr("View Transactions..."), this, SLOT(sViewTransactions()));
    viewGlAct->setEnabled(_privileges->check("ViewGLTransactions"));
  }
  else
    _col = 0;
}

void dspFinancialReport::sViewTransactions()
{
  ParameterList params;
  params.append("accnt_id", list()->id());
  params.append("startDate", _columnDates.value(_col).first);
  params.append("endDate", _columnDates.value(_col).second);
  params.append("run");

  dspGLTransactions *newdlg = new dspGLTransactions();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspFinancialReport::sCheck()
{
  if (!_periods->selectedItems().count())
  {
    QMessageBox::warning( this, tr("No period selected"),
                         tr("<p>You must select at least one period.") );
    return false;
  }

  //Make sure user has upgraded period settings
  q.exec("SELECT period_id FROM period WHERE period_quarter IS NULL;");
  if (q.first())
  {
    QMessageBox::warning( this, tr("Setup Incomplete"),
                         tr("<p>Please make sure all accounting periods "
                            "are associated with a quarter and fiscal year "
                            "before using this application.") );
    return false;
  }

  return true;
}

void dspFinancialReport::sFillList()
{
  if (!sCheck())
    return; 
  if (!_metrics->boolean("ManualForwardUpdate"))
  {
    if (!forwardUpdate())
      return;
  }
  if (_trend->isChecked())
    sFillListTrend();
  else
    sFillListStatement();
}

void dspFinancialReport::sFillListStatement()
{
  XSqlQuery label;
  QList<int> periodsRef;
  QString qc;
  QStringList qwList;

  q.prepare("SELECT * FROM flcol WHERE flcol_id=:flcolid");
  q.bindValue(":flcolid", _flcol->id());
  q.exec();
  if (q.first())
  {
    //Find which period selected
    QList<XTreeWidgetItem*> selected = _periods->selectedItems();
    for (int i = 0; i < selected.size(); i++)
      periodsRef.prepend(((XTreeWidgetItem*)(selected[i]))->id());
    if(periodsRef.count() < 1)
      return;

    //Get date labels for period
    label.prepare( "SELECT * FROM getflstmthead(:flcolid,:periodid)");
    label.bindValue(":flcolid", _flcol->id());
    label.bindValue(":periodid", periodsRef.at(0));
    label.exec();

    //Get column date ranges for drill down
    _columnDates.clear();
    XSqlQuery coldata;
    coldata.prepare("SELECT * FROM getflcoldata(:flcolid,:periodid)");
    coldata.bindValue(":flcolid", _flcol->id());
    coldata.bindValue(":periodid", periodsRef.at(0));
    coldata.exec();
    while(coldata.next())
    {
      QPair<QDate, QDate> range;
      range.first = coldata.value("flcoldata_start").toDate();
      range.second = coldata.value("flcoldata_end").toDate();
      _columnDates.insert(coldata.value("flcoldata_column").toInt(), range);
    };

    if (label.first())
    {
      list()->clear();
      list()->setColumnCount(0);
      list()->addColumn(tr("Group\n  Account Name"), -1, Qt::AlignLeft, true, "flstmtitem_name");

      //Build report query
      qc = ("SELECT COALESCE(flstmtitem_accnt_id,-1) AS id, flstmtitem_order AS orderby,"
            "       flstmtitem_level AS xtindentrole,"
            "       CASE WHEN flstmtitem_type = 'G' THEN 2 "
            "            WHEN flstmtitem_type = 'I' THEN 1 "
            "            ELSE -1 "
            "       END AS type,"
            "       flstmtitem_name");

      if (q.value("flcol_month").toBool())
      {
        if (q.value("flcol_showdb").toBool())
        {
          list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_month").toString()).arg(_columnLabels.value(cDebits)),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_monthdb");
          list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_month").toString()).arg(_columnLabels.value(cCredits)),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_monthcr");
          qc += ",flstmtitem_monthdb, 'curr' AS flstmtitem_monthdb_xtnumericrole"
                ",flstmtitem_monthcr, 'curr' AS flstmtitem_monthcr_xtnumericrole";
          qwList << "(flstmtitem_monthdb <> 0) OR (flstmtitem_monthcr <> 0)";
        }
        list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_month").toString()).arg(label.value("flstmthead_typedescrip2").toString()),
                                   _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_month");
        qc += ",flstmtitem_month, 'curr' AS flstmtitem_month_xtnumericrole";
        qwList << "(flstmtitem_month <> 0)";
        if (q.value("flcol_prcnt").toBool())
        {
          list()->addColumn(tr("%1\n% of Group").arg(label.value("flstmthead_month").toString()),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_monthprcnt");
          qc += ",flstmtitem_monthprcnt, 'percent' AS flstmtitem_monthprcnt_xtnumericrole";
        }
        if (q.value("flcol_budget").toBool())
        {
          list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_month").toString()).arg(_columnLabels.value(cBudget)),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_monthbudget");
          qc += ",flstmtitem_monthbudget, 'curr' AS flstmtitem_monthbudget_xtnumericrole";
          qwList << "(flstmtitem_monthbudget <> 0)";
          if (q.value("flcol_budgetprcnt").toBool())
          {
            list()->addColumn( tr("%1\n% of Group").arg(label.value("flstmthead_month").toString()),
                               _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_monthbudgetprcnt");
            qc += ",flstmtitem_monthbudgetprcnt, 'percent' AS flstmtitem_monthbudgetprcnt_xtnumericrole";
          }
          if (q.value("flcol_budgetdiff").toBool())
          {
            list()->addColumn( tr("%1\n%2 Diff.").arg(label.value("flstmthead_month").toString()).arg(_columnLabels.value(cBudget)),
                               _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_monthbudgetdiff");
            qc += ",flstmtitem_monthbudgetdiff, 'curr' AS flstmtitem_monthbudgetdiff_xtnumericrole";
          }
          if (q.value("flcol_budgetdiffprcnt").toBool())
          {
            list()->addColumn( tr("%1\n%2 % Diff.").arg(label.value("flstmthead_month").toString()).arg(_columnLabels.value(cBudget)),
                               _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_monthbudgetdiffprcnt");
            qc += ",flstmtitem_monthbudgetdiffprcnt, 'percent' AS flstmtitem_monthbudgetdiffprcent_xtnumericrole";
          }
        }
      }
      if (q.value("flcol_quarter").toBool())
      {
        if (q.value("flcol_showdb").toBool())
        {
          list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_qtr").toString()).arg(_columnLabels.value(cDebits)),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_qtrdb");
          list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_qtr").toString()).arg(_columnLabels.value(cCredits)),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_qtrcr");
          qc += ",flstmtitem_qtrdb, 'curr' AS flstmtitem_qtrdb_xtnumericrole"
                ",flstmtitem_qtrcr, 'cur' AS flstmtitem_qtrcr_xtnumericrole";
          qwList << "(flstmtitem_qtrdb <> 0) OR (flstmtitem_qtrcr <> 0)";
        }
        list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_qtr").toString()).arg(label.value("flstmthead_typedescrip2").toString()),
                           _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_qtr");
        qc += ",flstmtitem_qtr, 'curr' AS flstmtitem_qtr_xtnumericrole";
        qwList << "(flstmtitem_qtr <> 0)";
        if (q.value("flcol_prcnt").toBool())
        {
          list()->addColumn(tr("%1\n% of Group").arg(label.value("flstmthead_qtr").toString()),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstitem_qtrprcent");
          qc += ",flstmtitem_qtrprcnt, 'percent' AS flstitem_qtrprcnt_xtnumericrole";
        }
        if (q.value("flcol_budget").toBool())
        {
          list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_qtr").toString()).arg(_columnLabels.value(cBudget)),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_qtrbudget");
          qc += ",flstmtitem_qtrbudget, 'curr' AS flstmtitem_qtrbudget_xtnumericrole";
          qwList << "(flstmtitem_qtrbudget <> 0)";
          if (q.value("flcol_budgetprcnt").toBool())
          {
            list()->addColumn(tr("%1\n% of Group").arg(label.value("flstmthead_qtr").toString()),
                               _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_qtrbudgetprcnt" );
            qc += ",flstmtitem_qtrbudgetprcnt, 'percent' AS flstmtitem_qtrbudgetprcent_xtnumericrole";
          }
          if (q.value("flcol_budgetdiff").toBool())
          {
            list()->addColumn(tr("%1\n%2 Diff.").arg(label.value("flstmthead_qtr").toString()).arg(_columnLabels.value(cBudget)),
                               _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_qtrbudgetdiff");
            qc += ",flstmtitem_qtrbudgetdiff, 'curr' AS flstmtitem_qtrbudgetdiff_xtnumericrole";
          }
          if (q.value("flcol_budgetdiffprcnt").toBool())
          {
            list()->addColumn(tr("%1\n%2 % Diff.").arg(label.value("flstmthead_qtr").toString()).arg(_columnLabels.value(cBudget)),
                               _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_qtrbudgetdiffprcnt");
            qc += ",flstmtitem_qtrbudgetdiffprcnt, 'percent' AS flstmtitem_qtrbudgetdiffprcnt_xtnumericrole";
          }
        }
      }
      if (q.value("flcol_year").toBool())
      {
        if (q.value("flcol_showdb").toBool())
        {
          list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_year").toString()).arg(_columnLabels.value(cDebits)),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_yeardb");
          list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_year").toString()).arg(_columnLabels.value(cCredits)),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_yearcr");
           qc += ",flstmtitem_yeardb, 'curr' AS flstmtitem_yeardb_xtnumericrole"
                 ",flstmtitem_yearcr, 'curr' AS flstmtitem_yearcr_xtnumericrole";
           qwList << "(flstmtitem_yeardb <> 0) OR (flstmtitem_yearcr <> 0)";
        }
        list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_year").toString()).arg(label.value("flstmthead_typedescrip2").toString()),
                           _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_year");
        qc += ",flstmtitem_year, 'curr' AS flstmtitem_year_xtnumericrole";
        qwList << "(flstmtitem_year <> 0)";
        if (q.value("flcol_prcnt").toBool())
        {
          list()->addColumn(tr("%1\n% of Group").arg(label.value("flstmthead_year").toString()),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_yearprcnt");
          qc += ",flstmtitem_yearprcnt, 'percent' AS flstmtitem_yearprcnt_xtnumericrole";
        }
        if (q.value("flcol_budget").toBool())
        {
          list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_year").toString()).arg(_columnLabels.value(cBudget)),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_yearbudget");
          qc += ",flstmtitem_yearbudget, 'curr' AS flstmtitem_yearbudget_xtnumericrole";
          qwList << "(flstmtitem_yearbudget <> 0)";
          if (q.value("flcol_budgetprcnt").toBool())
          {
            list()->addColumn(tr("%1\n% of Group").arg(label.value("flstmthead_year").toString()),
                               _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_yearbudgetprcent");
            qc += ",flstmtitem_yearbudgetprcnt, 'percent' AS flstmtitem_yearbudgetprcent_xtnumericrole";
          }
          if (q.value("flcol_budgetdiff").toBool())
          {
            list()->addColumn(tr("%1\n%2 Diff.").arg(label.value("flstmthead_year").toString()).arg(_columnLabels.value(cBudget)),
                               _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_yearbudgetdiff");
             qc += ",flstmtitem_yearbudgetdiff, 'curr' AS flstmtitem_yearbudgetdiff_xtnumericrole";
          }
          if (q.value("flcol_budgetdiffprcnt").toBool())
          {
            list()->addColumn(tr("%1\n%2 % Diff.").arg(label.value("flstmthead_year").toString()).arg(_columnLabels.value(cBudget)),
                               _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_yearbudgetdiffprcnt");
             qc += ",flstmtitem_yearbudgetdiffprcnt, 'percent' AS flstmtitem_yearbudgetdiffprcnt_xtnumericrole";
          }
        }
      }
      if (q.value("flcol_priormonth").toBool())
      {
        list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_prmonth").toString()).arg(label.value("flstmthead_typedescrip2").toString()),
                           _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_prmonth");
        qc += ",flstmtitem_prmonth, 'curr' AS flstmtitem_prmonth_xtnumericrole";
        qwList << "(flstmtitem_prmonth <> 0)";
        if (q.value("flcol_priorprcnt").toBool())
        {
          list()->addColumn(tr("%1\n% of Group").arg(label.value("flstmthead_prmonth").toString()),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_prmonthprcnt");
          qc += ",flstmtitem_prmonthprcnt, 'percent' AS flstmtitem_prmonthprcnt_xtnumericrole";
        }
        if (q.value("flcol_priordiff").toBool())
        {
          list()->addColumn(tr("%1\n%2 Diff.").arg(label.value("flstmthead_prmonth").toString()).arg(label.value("flstmthead_typedescrip2").toString()),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_prmonthdiff");
          qc += ",flstmtitem_prmonthdiff, 'curr' AS flstmtitem_prmonthdiff_xtnumericrole";
        }
        if (q.value("flcol_priordiffprcnt").toBool())
        {
          list()->addColumn(tr("%1\n%2 % Diff.").arg(label.value("flstmthead_prmonth").toString()).arg(label.value("flstmthead_typedescrip2").toString()),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_prmonthdiffprcnt");
          qc += ",flstmtitem_prmonthdiffprcnt, 'percent' AS flstmtitem_prmonthdiffprcnt_xtnumericrole";
        }
      }
      if (q.value("flcol_priorquarter").toBool())
      {
        list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_prqtr").toString()).arg(label.value("flstmthead_typedescrip2").toString()),
                           _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_prqtr");
        qc += ",flstmtitem_prqtr, 'curr' AS flstmtitem_prqtr_xtnumericrole";
        qwList << "(flstmtitem_prqtr <> 0)";
        if (q.value("flcol_priorprcnt").toBool())
        {
          list()->addColumn(tr("%1\n% of Group").arg(label.value("flstmthead_prqtr").toString()),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_prqtrprcnt");
           qc += ",flstmtitem_prqtrprcnt, 'percent' AS flstmtitem_prqtrprcnt_xtnumericrole";
        }
        if (q.value("flcol_priordiff").toBool())
        {
          list()->addColumn(tr("%1\n%2 Diff.").arg(label.value("flstmthead_prqtr").toString()).arg(label.value("flstmthead_typedescrip2").toString()),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_prqtrdiff");
           qc += ",flstmtitem_prqtrdiff, 'curr' AS flstmtitem_prqtrdiff_xtnumericrole";
        }
        if (q.value("flcol_priordiffprcnt").toBool())
        {
          list()->addColumn(tr("%1\n%2 % Diff.").arg(label.value("flstmthead_prqtr").toString()).arg(label.value("flstmthead_typedescrip2").toString()),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_prqtrdiffprcnt");
           qc += ",flstmtitem_prqtrdiffprcnt, 'percent' AS flstmtitem_prqtrdiffprcnt_xtnumericrole";
        }
      }
      if (q.value("flcol_prioryear").toString() != "N")
      {
        list()->addColumn(tr("%1\n%2").arg(label.value("flstmthead_pryear").toString()).arg(label.value("flstmthead_typedescrip2").toString()),
                           _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_pryear");
        qc += ",flstmtitem_pryear, 'curr' AS flstmtitem_pryear_xtnumericrole";
        qwList << "(flstmtitem_pryear <> 0)";
        if (q.value("flcol_priorprcnt").toBool())
        {
          list()->addColumn(tr("%1\n% of Group").arg(label.value("flstmthead_pryear").toString()),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_pryearprcnt");
           qc += ",flstmtitem_pryearprcnt, 'percent' AS flstmtitem_pryearprcnt_xtnumericrole";
        }
        if (q.value("flcol_priordiff").toBool())
        {
          list()->addColumn(tr("%1\n%2 Diff.").arg(label.value("flstmthead_pryear").toString()).arg(label.value("flstmthead_typedescrip2").toString()),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_pryeardiff");
           qc += ",flstmtitem_pryeardiff, 'curr' AS flstmtitem_pryeardiff_xtnumericrole";
        }
        if (q.value("flcol_priordiffprcnt").toBool())
        {
          list()->addColumn(tr("%1\n%2 % Diff.").arg(label.value("flstmthead_pryear").toString()).arg(label.value("flstmthead_typedescrip2").toString()),
                             _bigMoneyColumn, Qt::AlignRight, true, "flstmtitem_pryeardiffprcnt");
          qc += ",flstmtitem_pryeardiffprcnt, 'percent' AS flstmtitem_pryeardiffprcnt_xtnumericrole";
        }
      }
      qc += " FROM financialreport(:flcolid,:periodid,:shownumbers,false,:prjid)";
      if (!_showzeros->isChecked())
        qc += " WHERE (" + qwList.join(" OR ") + "  OR (flstmtitem_type <> 'I'))";
      q.prepare(qc);
      q.bindValue(":flcolid", _flcol->id());
      q.bindValue(":periodid", periodsRef.at(0));
      q.bindValue(":shownumbers", _shownumbers->isChecked());
      q.bindValue(":prjid", _prjid);
      q.exec();
      list()->populate(q, true);
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
      list()->expandAll();
    }
  }
}

void dspFinancialReport::sFillListTrend()
{
  if (_typeCode=="A")
  {
    if( (!_showBegBal->isChecked()) &&
        (!_showBegBalPrcnt->isChecked()) &&
        (!_showDebits->isChecked()) &&
        (!_showDebitsPrcnt->isChecked()) &&
        (!_showCredits->isChecked()) &&
        (!_showCreditsPrcnt->isChecked()) &&
        (!_showEndBal->isChecked()) &&
        (!_showEndBalPrcnt->isChecked()) &&
        (!_showBudget->isChecked()) &&
        (!_showBudgetPrcnt->isChecked()) &&
        (!_showDiff->isChecked()) &&
        (!_showDiffPrcnt->isChecked()) &&
        (!_showCustom->isChecked()) &&
        (!_showCustomPrcnt->isChecked()) )
    {
      QMessageBox::critical(this, tr("Report Error"),
                            tr("You must select at least one Column"));
      return;
    }
  }
  else if (!_actuals->isChecked() && !_budgets->isChecked())
  {
    QMessageBox::critical(this, tr("Report Error"),
                          tr("You must select actuals or budget to proceed."));
    return;
  }

  int c = 0;
  QList<int> periodsRef;
  QStringList periods;
  QString interval;

  if (_month->isChecked())
    interval = "M";
  else if (_quarter->isChecked())
    interval = "Q";
  else
    interval = "Y";

  QString customlabel;
  q.prepare("SELECT flhead_custom_label"
            "  FROM flhead"
            " WHERE (flhead_id=:flhead_id);");
  q.bindValue(":flhead_id", _flhead->id());
  q.exec();
  if(q.first())
    customlabel = q.value("flhead_custom_label").toString();
  if(customlabel.isEmpty())
    customlabel = tr("Custom");

  QTreeWidgetItem* selected;
  QString label;
  QStringList periodList;
  for (int i = 0; i < _periods->topLevelItemCount(); i++)
  {
    if (_periods->topLevelItem(i)->isSelected())
    {
      selected =  _periods->topLevelItem(i);
      label = selected->text(1).isEmpty() ? selected->text(0) : selected->text(1);
      periodsRef.prepend(((XTreeWidgetItem*)(selected))->id());
      periods.prepend(label);
      periodList.prepend(QString::number(((XTreeWidgetItem*)(selected))->id()));
    }
  }

  if(periodsRef.count() < 1)
    return;

  //Get column date ranges for drill down
  if (_actuals->isChecked())
  {
    _columnDates.clear();
    XSqlQuery coldata;
    QString sql("SELECT * FROM getflcoldata(<? value(\"interval\") ?>,ARRAY[<? literal(\"periodids\") ?>], <? value(\"budgets\") ?>)");
    ParameterList params;
    if (_month->isChecked())
      params.append("interval", "M");
    else if (_quarter->isChecked())
      params.append("interval", "Q");
    else
      params.append("interval", "Y");
    params.append("periodids", periodList.join(","));
    params.append("budgets", QVariant(_budgets->isChecked()));
    MetaSQLQuery mql(sql);
    coldata = mql.toQuery(params);
    while(coldata.next())
    {
      QPair<QDate, QDate> range;
      range.first = coldata.value("flcoldata_start").toDate();
      range.second = coldata.value("flcoldata_end").toDate();
      _columnDates.insert(coldata.value("flcoldata_column").toInt(), range);
    };
  }

  list()->setColumnCount(0);
  list()->addColumn( tr("Group\n  Account Name"), -1, Qt::AlignLeft, true, "name");

  q.prepare("SELECT financialReport(:flhead_id, :period_id, :interval, :prjid) AS result;");

  QString q1c = QString("SELECT -1, r0.flrpt_order AS orderby, r0.flrpt_level AS xtindentrole,"
                        "       :group AS type, flgrp_id AS id,"
                        "       flgrp_name AS name");
  QString q1f = QString(" FROM flgrp");
  QString q1w = QString(" WHERE ((true)");

  QString q2c = QString("SELECT r0.flrpt_accnt_id, r0.flrpt_order AS orderby, r0.flrpt_level AS xtindentrole,"
                        "       :item AS type, flitem_id AS id,");
  if (_shownumbers->isChecked())
    q2c += " (formatGLAccount(accnt_id) || '-' || accnt_descrip) AS name ";
  else
    q2c += " accnt_descrip AS name ";

  QString q2f = QString(" FROM flitem, accnt ");
  QString q2w = QString(" WHERE ((true) AND accnt_id IN (SELECT accnt_id FROM flaccnt WHERE flitem_id=flitem_id) ");

  QString q3c = QString("SELECT -1, r0.flrpt_order AS orderby, r0.flrpt_level AS xtindentrole,"
                        "       :spec AS type, flspec_id AS id,"
                        "       flspec_name AS name");
  QString q3f = QString(" FROM flspec");
  QString q3w = QString(" WHERE ((true)");

  QString q4c = QString("SELECT -1, r0.flrpt_order AS orderby, r0.flrpt_level AS xtindentrole,"
                        "       -1 AS type, r0.flrpt_type_id AS id,"
                        "       CASE WHEN(r0.flrpt_type='T' AND r0.flrpt_level=0) THEN COALESCE(r0.flrpt_altname, 'Total')"
                        "            WHEN(r0.flrpt_type='T') THEN COALESCE(r0.flrpt_altname, 'Subtotal')"
                        "            ELSE ('Type ' || r0.flrpt_type || ' ' || text(r0.flrpt_type_id))"
                        "       END AS name");
  QString q4f = QString(" FROM");
  QString q4w = QString(" WHERE ((true)");

  QStringList qbList;
  QStringList qtList;
  QStringList qzList;

  QString colname;
  QString sharedColumns;

  for(c = 0; c < periodsRef.count(); c++)
  {
    sharedColumns = QString();
    if (_typeCode == "A")
    {
      if(_showBegBal->isChecked())
      {
        colname = "flrpt_beginning";
        list()->addColumn(tr("%1\n%2").arg(periods.at(c)).arg(_columnLabels.value(cBegining)),
                          _bigMoneyColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
        q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showstart) THEN r%1.%2 ELSE NULL END AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
        sharedColumns += QString(",r%1.%2 AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
        qzList << QString("(r%1.%2 <> 0)").arg(c).arg(colname);
      }
      if(_showBegBalPrcnt->isChecked())
      {
        colname = "flrpt_beginningprcnt";
        list()->addColumn(tr("%1\n%2 %").arg(periods.at(c)).arg(_columnLabels.value(cBegining)),
                          _ynColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
        q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showstartprcnt) THEN r%1.%2 ELSE NULL END AS r%3%4, 'percent' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
        sharedColumns += QString(",r%1.%2, 'percent' AS r%3%4_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname);
      }
      if(_showDebits->isChecked())
      {
        colname = "flrpt_debits";
        list()->addColumn(tr("%1\n%2").arg(periods.at(c)).arg(_columnLabels.value(cDebits)),
                          _bigMoneyColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
        q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showdelta) THEN r%1.%2 ELSE NULL END AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
        sharedColumns += QString(",r%1.%2 AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
        qzList << QString("(r%1.%2 <> 0)").arg(c).arg(colname);
      }
      if(_showDebitsPrcnt->isChecked())
      {
        colname = "flrpt_debitsprcnt";
        list()->addColumn( tr("%1\n%2 %").arg(periods.at(c)).arg(_columnLabels.value(cDebits)),
                           _ynColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
        q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showdeltaprcnt) THEN r%1.%2 ELSE NULL END AS r%3%4, 'percent' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
        sharedColumns += QString(",r%1.%2, 'percent' AS r%3%4_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname);
      }
      if(_showCredits->isChecked())
      {
        colname = "flrpt_credits";
        list()->addColumn(tr("%1\n%2").arg(periods.at(c)).arg(_columnLabels.value(cCredits)),
                          _bigMoneyColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
        q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showdelta) THEN r%1.%2 ELSE NULL END AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
        sharedColumns += QString(",r%1.%2 AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
        qzList << QString("(r%1.%2 <> 0)").arg(c).arg(colname);
      }
      if(_showCreditsPrcnt->isChecked())
      {
        colname = "flrpt_creditsprcnt";
        list()->addColumn(tr("%1\n%2 %").arg(periods.at(c)).arg(_columnLabels.value(cCredits)),
                          _ynColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
        q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showdeltaprcnt) THEN r%1.%2 ELSE NULL END AS r%3%4, 'percent' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
        sharedColumns += QString(",r%1.%2, 'percent' AS r%3%4_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname);
      }
    }
    if ((_showEndBal->isChecked()) ||
        (_actuals->isChecked() && _typeCode == "B"))
    {
      colname = "flrpt_ending";
      list()->addColumn( tr("%1\n%2").arg(periods.at(c)).arg(_columnLabels.value(cEnding)),
                         _bigMoneyColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showend) THEN r%1.%2 ELSE NULL END AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
      sharedColumns += QString(",r%1.%2 AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
      qzList << QString("(r%1.%2 <> 0)").arg(c).arg(colname);
    }
    if(_showEndBalPrcnt->isChecked() && _typeCode=="A")
    {
      colname = "flrpt_endingprcnt";
      list()->addColumn(tr("%1\n%2 %").arg(periods.at(c)).arg(_columnLabels.value(cEnding)),
                        _ynColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showendprcnt) THEN r%1.%2 ELSE NULL END AS r%3%4, 'percent' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
      sharedColumns += QString(",r%1.%2, 'percent' AS r%3%4_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname);
    }
    if(_showBudget->isChecked() || _budgets->isChecked())
    {
      colname = "flrpt_budget";
      list()->addColumn(tr("%1\n%2").arg(periods.at(c)).arg(_columnLabels.value(cBudget)),
                        _bigMoneyColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showbudget) THEN r%1.%2 ELSE NULL END AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
      sharedColumns += QString(",r%1.%2 AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
      qbList << QString("r%1.%2").arg(c).arg(colname);
      qzList << QString("(r%1.%2 <> 0)").arg(c).arg(colname);
    }
    if(_showBudgetPrcnt->isChecked() && _typeCode=="A")
    {
      colname = "flrpt_budgetprcnt";
      list()->addColumn( tr("%1\n%2 %").arg(periods.at(c)).arg(_columnLabels.value(cBudget)),
                         _ynColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showbudgetprcnt) THEN r%1.%2 ELSE NULL END AS r%3%4, 'percent' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
      sharedColumns += QString(",r%1.%2, 'percent' AS r%3%4_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname);
    }
    if ((_showDiff->isChecked()) ||
        (_actuals->isChecked() &&
         ((_typeCode == "I") || (_typeCode == "C"))))
    {
      colname = "flrpt_diff";
      list()->addColumn(tr("%1\n%2").arg(periods.at(c)).arg(_columnLabels.value(cDiff)),
                        _bigMoneyColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showdiff) THEN r%1.%2 ELSE NULL END AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
      sharedColumns += QString(",r%1.%2 AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);

      qtList << QString("r%1.%2").arg(c).arg(colname);

      qzList << QString("(r%1.%2 <> 0)").arg(c).arg(colname);
    }
    if (_typeCode=="A")
    {
      if(_showDiffPrcnt->isChecked())
      {
        colname = "flrpt_diffprcnt";
        list()->addColumn(tr("%1\n%2 %").arg(periods.at(c)).arg(_columnLabels.value(cDiff)),
                          _ynColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
        q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showdiffprcnt) THEN r%1.%2 ELSE NULL END AS r%3%4, 'percent' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
        sharedColumns += QString(",%1, 'percent' AS %2_xtnumericrole").arg(colname).arg(colname);
      }
      if(_showCustom->isChecked())
      {
        colname = "flrpt_custom";
        list()->addColumn(tr("%1\n%2").arg(periods.at(c)).arg(customlabel),
                          _bigMoneyColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
        q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showcustom) THEN r%1.%2 ELSE NULL END AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
        sharedColumns += QString(",r%1.%2 AS r%3%4, 'curr' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
        qzList << QString("(r%1.%2 <> 0)").arg(c).arg(colname);
      }
      if(_showCustomPrcnt->isChecked())
      {
        colname = "flrpt_customprcnt";
        list()->addColumn(tr("%1\n%2 %").arg(periods.at(c)).arg(customlabel),
                          _ynColumn, Qt::AlignRight, true, QString("r%1%2").arg(c).arg(colname));
        q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showcustomprcnt) THEN r%1.%2 ELSE NULL END AS r%3%4, 'percent' AS r%5%6_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname).arg(c).arg(colname);
        sharedColumns += QString(",r%1.%2, 'percent' AS r%3%4_xtnumericrole").arg(c).arg(colname).arg(c).arg(colname);
      }
    }
    q2c += sharedColumns;
    q3c += sharedColumns;
    q4c += sharedColumns;

    q1f += QString(", flrpt AS r%1").arg(c);

    q1w += QString(" AND (r%1.flrpt_type='G')").arg(c);
    q1w += QString(" AND (r%1.flrpt_type_id=flgrp_id)").arg(c);
    q1w += QString(" AND (r%1.flrpt_flhead_id=:flhead_id)").arg(c);
    q1w += QString(" AND (r%1.flrpt_period_id=%2)").arg(c).arg(periodsRef.at(c));
    q1w += QString(" AND (r%1.flrpt_username=getEffectiveXtUser())").arg(c);
    q1w += QString(" AND (r%1.flrpt_interval='%2')").arg(c).arg(interval);
    if(c > 0)
      q1w += QString(" AND (r0.flrpt_order=r%1.flrpt_order)").arg(c);


    q2f += QString(", flrpt AS r%1").arg(c);

    q2w += QString(" AND (r%1.flrpt_type='I')").arg(c);
    q2w += QString(" AND (r%1.flrpt_type_id=flitem_id)").arg(c);
    q2w += QString(" AND (r%1.flrpt_flhead_id=:flhead_id)").arg(c);
    q2w += QString(" AND (r%1.flrpt_period_id=%2)").arg(c).arg(periodsRef.at(c));
    q2w += QString(" AND (r%1.flrpt_username=getEffectiveXtUser())").arg(c);
    q2w += QString(" AND (r%1.flrpt_accnt_id=accnt_id)").arg(c);
    q2w += QString(" AND (r%1.flrpt_interval='%2')").arg(c).arg(interval);
    if(c > 0)
      q2w += QString(" AND (r0.flrpt_order=r%1.flrpt_order)").arg(c);


    q3f += QString(", flrpt AS r%1").arg(c);

    q3w += QString(" AND (r%1.flrpt_type='S')").arg(c);
    q3w += QString(" AND (r%1.flrpt_type_id=flspec_id)").arg(c);
    q3w += QString(" AND (r%1.flrpt_flhead_id=:flhead_id)").arg(c);
    q3w += QString(" AND (r%1.flrpt_period_id=%2)").arg(c).arg(periodsRef.at(c));
    q3w += QString(" AND (r%1.flrpt_username=getEffectiveXtUser())").arg(c);
    q3w += QString(" AND (r%1.flrpt_interval='%2')").arg(c).arg(interval);
    if(c > 0)
      q3w += QString(" AND (r0.flrpt_order=r%1.flrpt_order)").arg(c);

    if(c > 0)
      q4f += QString(", flrpt AS r%1").arg(c);
    else
      q4f += QString(" flrpt AS r%1").arg(c);

    q4w += QString(" AND (NOT (r%1.flrpt_type IN ('G','I','S')))").arg(c);
    q4w += QString(" AND (r%1.flrpt_flhead_id=:flhead_id)").arg(c);
    q4w += QString(" AND (r%1.flrpt_period_id=%2)").arg(c).arg(periodsRef.at(c));
    q4w += QString(" AND (r%1.flrpt_username=getEffectiveXtUser())").arg(c);
    q4w += QString(" AND (r%1.flrpt_interval='%2')").arg(c).arg(interval);
    if(c > 0)
      q4w += QString(" AND (r0.flrpt_order=r%1.flrpt_order)").arg(c);

    q.bindValue(":flhead_id", _flhead->id());
    q.bindValue(":period_id", periodsRef.at(c));
    q.bindValue(":interval", interval);
    q.bindValue(":prjid", _prjid);
    q.exec();
  }

  //Grand Total for Trend Reports
  if ((_trend->isChecked()) && ((_typeCode == "I") || (_typeCode == "C")))
  {
    if (_budgets->isChecked())
    {
      list()->addColumn( tr("Budget\nTotal"), _bigMoneyColumn, Qt::AlignRight, true, "budgsum");
      q1c += ",CASE WHEN(flgrp_summarize AND flgrp_showbudget) THEN (" +
             qbList.join(" + ") +
             ") ELSE NULL END AS budgsum, 'curr' AS budgsum_xtnumericrole";
      q2c += ", " + qbList.join(" + ") + " AS budgsum, 'curr' AS budgsum_xtnumericrole";
      q3c += ", " + qbList.join(" + ") + " AS budgsum, 'curr' AS budgsum_xtnumericrole";
      q4c += ", " + qbList.join(" + ") + " AS budgsum, 'curr' AS budgsum_xtnumericrole";
    }
    if (_actuals->isChecked())
    {
      list()->addColumn( tr("Grand\nTotal"), _bigMoneyColumn, Qt::AlignRight, true, "diffsum");
      q1c += ",CASE WHEN(flgrp_summarize AND flgrp_showdiff) THEN (" +
             qtList.join(" + ") +
             ") ELSE NULL END AS diffsum, 'curr' AS diffsum_xtnumericrole";
      q2c += ", " + qtList.join(" + ") + " AS diffsum, 'curr' AS diffsum_xtnumericrole";
      q3c += ", " + qtList.join(" + ") + " AS diffsum, 'curr' AS diffsum_xtnumericrole";
      q4c += ", " + qtList.join(" + ") + " AS diffsum, 'curr' AS diffsum_xtnumericrole";
    }
  }

  if (!_showzeros->isChecked())
  {
    q2w += " AND (" + qzList.join(" OR ") + ")";
    q3w += " AND (" + qzList.join(" OR ") + ")";
  }

  q1w += ")";
  q2w += ")";
  q3w += ")";
  q4w += ")";

  QString query = q1c + q1f + q1w +
                  QString(" UNION ") +
                  q2c + q2f + q2w +
                  QString(" UNION ") +
                  q3c + q3f + q3w +
                  QString(" UNION ") +
                  q4c + q4f + q4w +
                  QString(" ORDER BY orderby;");
  q.prepare(query);
  q.bindValue(":flhead_id", _flhead->id());
  q.bindValue(":item", cFlItem);
  q.bindValue(":group", cFlGroup);
  q.bindValue(":spec", cFlSpec);
  q.exec();
  list()->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  list()->expandAll();
}

void dspFinancialReport::sFillPeriods()
{
  if ((!_trend->isChecked()) || (_month->isChecked()))
  {
  _periods->populate("SELECT period_id,"
                     "(formatDate(period_start) || '-' || formatDate(period_end)) AS f_name,"
                     "period_name AS alt_name "
            "  FROM period "
            "ORDER BY period_start DESC;" );
  }
  else if (_quarter->isChecked())
  {
    _periods->populate("SELECT LAST(period_id),"
                       "('Q' || period_quarter || '-' || EXTRACT(year from yearperiod_end)) AS f_name,"
                       "'' AS alt_name "
                       "FROM "
            " (SELECT period_id, period_quarter, yearperiod_end "
            " FROM period,yearperiod "
            " WHERE (period_yearperiod_id=yearperiod_id) "
            " ORDER BY period_end) AS data "
            " GROUP by period_quarter,yearperiod_end "
            " ORDER by EXTRACT(year from yearperiod_end) DESC, period_quarter DESC");
  }
  else
  {
    _periods->populate("SELECT (SELECT LAST(period_id)"
                       "        FROM (SELECT period_id,period_start"
                       "              FROM period "
                       "              WHERE period_yearperiod_id=yearperiod_id "
                       "              ORDER BY period_start) "
                       "        AS data) AS id,"
                       "        CAST(EXTRACT(year from yearperiod_end) AS TEXT) AS f_name,"
                       "        '' AS alt_name "
                       " FROM yearperiod "
                       " ORDER BY EXTRACT(year from yearperiod_end) DESC" );
  }

}

void dspFinancialReport::sCollapsed( QTreeWidgetItem * item )
{
  for (int i = 0; i < item->childCount(); i++)
  {
    XTreeWidgetItem *child = (XTreeWidgetItem*)item->child(i);
    if(child->altId() == -1)
    {
      for (int i = 1; i < list()->columnCount(); i++)
        item->setText(i, child->text(i));
      return;
    }
  }
}

void dspFinancialReport::sExpanded( QTreeWidgetItem * item )
{
  if(item->childCount() > 0)
    for(int i = 1; i < list()->columnCount(); i++)
      item->setText(i, "");
}

void dspFinancialReport::sPreview()
{
  ParameterList params;
  if(!setParams(params))
    return;

  XSqlQuery qry;
  qry.prepare("SELECT report_source "
              " FROM report "
              " WHERE (report_name=:report)"
              " ORDER BY report_grade DESC "
              " LIMIT 1;" );
  qry.bindValue(":report",reportName());
  qry.exec();
  qry.first();

  QDomDocument _doc;
  QString errorMessage;
  int     errorLine;

  if (!_doc.setContent(qry.value("report_source").toString(), &errorMessage, &errorLine))
  {
    QMessageBox::critical(this, tr("Error Parsing Report"),
                          tr("There was an error Parsing the report definition. %1 %2").arg(errorMessage).arg(errorLine));
    return;
  }

  QPrinter printer(QPrinter::HighResolution);
  ORPreRender pre;
  pre.setDom(_doc);
  pre.setParamList(params);
  ORODocument * doc = pre.generate();

  ORPrintRender render;
  render.setupPrinter(doc, &printer);

  PreviewDialog preview (doc, &printer, this);
  preview.exec();
}

void dspFinancialReport::sPrint()
{
  ParameterList params;
  if(!setParams(params))
    return;

  orReport report(reportName(), params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspFinancialReport::sPopulateMenu( QMenu * pMenu )
{
  pMenu->addAction(tr("Edit Alternate Label..."), this, SLOT(sEditPeriodLabel()));
}

void dspFinancialReport::sEditPeriodLabel()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_periods->currentItem();
  if(!item)
    return;

  bool ok;
  QString text = QInputDialog::getText(this, tr("Alternate Label"),
        tr("Enter an alternate label for the period %1:").arg(item->text(0)),
        QLineEdit::Normal, item->text(1), &ok);
  if(ok)
    item->setText(1, text);
}

void dspFinancialReport::sReportChanged(int flheadid)
{
    //Populate column layouts
  q.prepare( "SELECT flcol_id, flcol_name "
  " FROM flcol "
  " WHERE flcol_flhead_id=:flhead_id "
  " ORDER BY flcol_name; ");
  q.bindValue(":flhead_id", flheadid);
  q.exec();
  _flcol->populate(q);

  q.prepare("SELECT flhead_usealtbegin, flhead_altbegin,"
        "       flhead_usealtend, flhead_altend,"
        "       flhead_usealtdebits, flhead_altdebits,"
        "       flhead_usealtcredits, flhead_altcredits,"
        "       flhead_usealtbudget, flhead_altbudget,"
        "       flhead_usealtdiff, flhead_altdiff, "
        "      flhead_type "
        "  FROM flhead"
        " WHERE (flhead_id=:flhead_id);");
  q.bindValue(":flhead_id", flheadid);
  q.exec();
  if(q.first())
  {
    if(q.value("flhead_usealtbegin").toBool())
      _columnLabels.insert(cBegining, q.value("flhead_altbegin").toString());
    else
      _columnLabels.insert(cBegining, tr("Beg. Bal."));

    if(q.value("flhead_usealtend").toBool())
      _columnLabels.insert(cEnding, q.value("flhead_altend").toString());
    else if (q.value("flhead_type").toString() == "B")
      _columnLabels.insert(cEnding, tr("Balance"));
    else
      _columnLabels.insert(cEnding, tr("End. Bal."));

    if(q.value("flhead_usealtdebits").toBool())
      _columnLabels.insert(cDebits, q.value("flhead_altdebits").toString());
    else
      _columnLabels.insert(cDebits, tr("Debits"));

    if(q.value("flhead_usealtcredits").toBool())
      _columnLabels.insert(cCredits, q.value("flhead_altcredits").toString());
    else
      _columnLabels.insert(cCredits, tr("Credits"));

    if(q.value("flhead_usealtbudget").toBool())
      _columnLabels.insert(cBudget, q.value("flhead_altbudget").toString());
    else
      _columnLabels.insert(cBudget, tr("Budget"));

    if(q.value("flhead_usealtdiff").toBool())
      _columnLabels.insert(cDiff, q.value("flhead_altdiff").toString());
    else if (q.value("flhead_type").toString() == "I")
      _columnLabels.insert(cDiff, tr("Income"));
    else if (q.value("flhead_type").toString() == "C")
      _columnLabels.insert(cDiff, tr("Cash"));
    else
      _columnLabels.insert(cDiff, tr("Difference"));

    _showBegBal->setText(_columnLabels.value(cBegining));
    _showBegBalPrcnt->setText(tr("%1 %").arg(_columnLabels.value(cBegining)));
    _showEndBal->setText(_columnLabels.value(cEnding));
    _showEndBalPrcnt->setText(tr("%1 %").arg(_columnLabels.value(cEnding)));
    _showDebits->setText(_columnLabels.value(cDebits));
    _showDebitsPrcnt->setText(tr("%1 %").arg(_columnLabels.value(cDebits)));
    _showCredits->setText(_columnLabels.value(cCredits));
    _showCreditsPrcnt->setText(tr("%1 %").arg(_columnLabels.value(cCredits)));
    _showBudget->setText(_columnLabels.value(cBudget));
    _showBudgetPrcnt->setText(tr("%1 %").arg(_columnLabels.value(cBudget)));
    _showDiff->setText(_columnLabels.value(cDiff));
    _showDiffPrcnt->setText(tr("%1 %").arg(_columnLabels.value(cDiff)));


    if (q.value("flhead_type").toString()== "A")
    {
      _tab->setTabEnabled(_tab->indexOf(_showColumnsTab),true);
      _trend->setChecked(true);
      _periods->setSelectionMode(QAbstractItemView::ExtendedSelection);
      _flcol->setEnabled(false);
      _type->setText(tr("Ad Hoc"));
      _typeCode = "A";
    }
    else
    {
      _typeCode = q.value("flhead_type").toString();
      if (_typeCode== "I")
        _type->setText(tr("Income Statement"));
      if (_typeCode== "B")
        _type->setText(tr("Balance Sheet"));
      if (_typeCode== "C")
        _type->setText(tr("Cash Flow Statement"));
      _tab->setTabEnabled(_tab->indexOf(_showColumnsTab),false);
      _trend->setChecked(false);
      sTogglePeriod();
    }
  }
  sToggleTrend();
}

void dspFinancialReport::sToggleTrend()
{
  if (!_trend->isChecked())
  {
    if (_typeCode == "A")
      _trend->setChecked(true);
  }
  else
  {
    _actuals->setDisabled(_typeCode == "A");
    _budgets->setDisabled(_typeCode == "A");
  }
  sToggleNotes();
}

void dspFinancialReport::sToggleNotes()
{
  _notesAct->setEnabled(_periods->selectedItems().count() &&
                        !_trend->isChecked());
}

void dspFinancialReport::sTogglePeriod()
{

  if (_trend->isChecked())
  {
    _periods->setSelectionMode(QAbstractItemView::MultiSelection);
    _flcol->setEnabled(false);
  }
  else
  {
    _periods->setSelectionMode(QAbstractItemView::SingleSelection);
    _flcol->setEnabled(true);
  }
}

bool dspFinancialReport::forwardUpdate()
{
  QString sql( "SELECT MIN(forwardUpdateAccount(accnt_id)) AS result "
               "FROM accnt;" );
  ParameterList params;
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("forwardUpdateTrialBalance", result), __FILE__, __LINE__);
      return false;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  return true;
}

QString dspFinancialReport::reportName() const
{
  if (_trend->isChecked())
  {
    if (_typeCode == "A")
      return "FinancialReport";
    else
      return "FinancialTrend";
  }
  else
  {
    XSqlQuery qry;
    qry.prepare("SELECT report_name "
                "FROM flcol "
                "  JOIN report ON (flcol_report_id=report_id)"
                "WHERE (flcol_id=:flcol_id);" );
    qry.bindValue(":flcol_id",_flcol->id());
    qry.exec();
    if (qry.first())
      return qry.value("report_name").toString();
  }
  return QString();
}

ParameterList dspFinancialReport::getParams()
{
  ParameterList params;
  bool ret = setParams(params);
  params.append("checkParamsReturn", ret);

  return params;
}

bool dspFinancialReport::setParams(ParameterList &params)
{
  QString interval;

  if (_month->isChecked())
    interval = "M";
  else if (_quarter->isChecked())
    interval = "Q";
  else
    interval = "Y";

  if(_shownumbers->isChecked())
    params.append("shownumbers");
  if(_showzeros->isChecked())
    params.append("showzeros");

  if (_trend->isChecked())
  {
    if (_actuals->isChecked())
      params.append("showActuals");
    if (_budgets->isChecked())
      params.append("showBudgets");
  }

  if (_prjid != -1)
  {
    params.append("project", tr("Project:"));
    params.append("prj_id", _prjid);
  }

  QList<QVariant> periodList;
  QTreeWidgetItem* selected;
  for (int i = 0; i < _periods->topLevelItemCount(); i++)
  {
    if (_periods->topLevelItem(i)->isSelected())
    {
      selected =  _periods->topLevelItem(i);
      periodList.prepend(((XTreeWidgetItem*)(selected))->id());
    }
  }

  if(periodList.isEmpty())
  {
    QMessageBox::warning(this, tr("No Period(s) Selected"),
      tr("You must select at least one period to report on.") );
    return false;
  }

  if (_trend->isChecked())
  {
    params.append("flhead_id", _flhead->id());
    params.append("period_id_list", periodList);
    params.append("interval", interval);
  }
  else if (!reportName().isEmpty())
  {
    params.append("flcol_id", _flcol->id());
    params.append("period_id", periodList.at(0));
  }

  return true;
}

void dspFinancialReport::sNotes()
{
  ParameterList params;
  params.append("period_id", _periods->id());
  params.append("flhead_id", _flhead->id());

  financialReportNotes newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

bool dspFinancialReport::columnHasTransactions(int col)
{
  return _columnDates.contains(col);
}

QDate dspFinancialReport::columnStart(int col)
{
  return _columnDates.value(col).first;
}

QDate dspFinancialReport::columnEnd(int col)
{
  return _columnDates.value(col).second;
}

