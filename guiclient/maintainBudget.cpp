/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "maintainBudget.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include <xlistbox.h>
#include <glcluster.h>
#include <openreports.h>

maintainBudget::maintainBudget(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_accountsAdd,    SIGNAL(clicked()), this, SLOT(sAccountsAdd()));
  connect(_accountsRemove, SIGNAL(clicked()), this, SLOT(sAccountsRemove()));
  connect(_close,          SIGNAL(clicked()), this, SLOT(close()));
  connect(_generate,       SIGNAL(clicked()), this, SLOT(sGenerateTable()));
  connect(_periodsAll,     SIGNAL(clicked()), this, SLOT(sPeriodsAll()));
  connect(_periodsInvert,  SIGNAL(clicked()), this, SLOT(sPeriodsInvert()));
  connect(_print,          SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_save,           SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_table, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(sValueChanged(QTableWidgetItem*)));

  _accounts->addColumn(QString(), -1, Qt::AlignLeft, true,"result");

  _dirty = false;
  _budgheadid = -1;
  _mode = cNew;
  
  q.exec("SELECT period_id, (formatDate(period_start) || '-' || formatDate(period_end)) AS f_name "
         "  FROM period "
         "ORDER BY period_start DESC;" );
  while(q.next())
  {
    XListBoxText *item = new XListBoxText(q.value("f_name").toString(), q.value("period_id").toInt());
    _periods->addItem(dynamic_cast<QListWidgetItem*>(item));
  }
}

maintainBudget::~maintainBudget()
{
  // no need to delete child widgets, Qt does it all for us
}

void maintainBudget::languageChange()
{
  retranslateUi(this);
}

enum SetResponse maintainBudget::set(const ParameterList & pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("budghead_id", &valid);
  if(valid)
  {
    _budgheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if(valid)
  {
    if(param.toString() == "new")
    {
      _mode = cNew;
      _print->setEnabled(false);
    }
    else if(param.toString() == "edit")
    {
      _mode = cEdit;
      _name->setFocus();
    }
    else if(param.toString() == "view")
    {
      _name->setReadOnly(true);
      _descrip->setReadOnly(true);
      _save->setEnabled(false);
      _save->hide();
      _table->setSelectionMode(QAbstractItemView::NoSelection);
      _accountsAdd->setEnabled(false);
      _accountsRemove->setEnabled(false);
      _periodsAll->setEnabled(false);
      _periodsInvert->setEnabled(false);
      _periodsNone->setEnabled(false);
      _generate->setEnabled(false);
      _close->setFocus();
    }
  }

  return NoError;
}

void maintainBudget::sSave()
{
  if(_name->text().trimmed().isEmpty())
  {
    QMessageBox::warning(this, tr("Cannot Save Budget"),
        tr("You must specify a name for this budget before saving."));
    _name->setFocus();
    return;
  }

  XSqlQuery qry;
  qry.prepare("SELECT budghead_id "
      "FROM budghead "
      "WHERE ((budghead_id != :budghead_id) "
      " AND (budghead_name = :budghead_name));");
  qry.bindValue(":budghead_id", _budgheadid);
  qry.bindValue(":budghead_name", _name->text());
  qry.exec();
  if (qry.first())
  {
    QMessageBox::warning(this, tr("Cannot Save Budget"),
        tr("The name is already in use by another budget."));
    _name->setFocus();
    return;
  }

  _save->setFocus();

  if(cEdit == _mode)
    q.prepare("UPDATE budghead "
              "   SET budghead_name=:name,"
              "       budghead_descrip=:descrip "
              " WHERE(budghead_id=:budghead_id);");
  else if(cNew == _mode)
  {
    q.prepare("SELECT nextval('budghead_budghead_id_seq') AS result;");
    q.exec();
    if(q.first())
      _budgheadid = q.value("result").toInt();
    else
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare("INSERT INTO budghead"
              "      (budghead_id, budghead_name, budghead_descrip) "
              "VALUES(:budghead_id, :name, :descrip);");
  }

  q.bindValue(":budghead_id", _budgheadid);
  q.bindValue(":name", _name->text());
  q.bindValue(":descrip", _descrip->text());
  if(!q.exec())
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // Delete the Budget Items and regenerate from table
  q.prepare("SELECT deleteBudgetItems(:budghead_id) AS result;");
  q.bindValue(":budghead_id", _budgheadid);
  if(!q.exec())
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT setBudget(:budghead_id, :period_id, :accnt_id, :amount) AS result;");

  for(int r = 0; r < _table->rowCount(); r++)
  {
    int accountid = _accountsRef.at(r);
    for(int c = 1; c < _table->columnCount(); c++)
    {
      if (_table->item(r, c))
      {
        QString amount = _table->item(r, c)->text();
        if(amount.isEmpty())
          amount = "0.0";
        q.bindValue(":budghead_id", _budgheadid);
        q.bindValue(":period_id", _periodsRef.at(c));
        q.bindValue(":accnt_id", accountid);
        q.bindValue(":amount", amount.toDouble());
        q.exec();
      }
      else
      {
        q.bindValue(":budghead_id", _budgheadid);
        q.bindValue(":period_id", _periodsRef.at(c));
        q.bindValue(":accnt_id", accountid);
        q.bindValue(":amount", 0.0);
        q.exec();
      }
    }
  }
 
  _dirty = false;

  omfgThis->sBudgetsUpdated(_budgheadid, true);

  if(cNew == _mode)
  {
    _name->clear();
    _descrip->clear();
    _accounts->clear();
    _periods->clearSelection();
    _accountsRef.clear();
    _periodsRef.clear();
    _table->setRowCount(0);
    _table->setColumnCount(0);
  }
  else
    close();
}

void maintainBudget::closeEvent( QCloseEvent * e )
{
  _close->setFocus();
  if(!_dirty)
  {
    e->accept();
    return;
  }

  switch(QMessageBox::question(this, tr("Unsaved Changes"),
                               tr("<p>The document has been changed "
                                  "since the last save.<br>Do you want "
                                  "to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                               QMessageBox::Save)) {
    case QMessageBox::Save:
      sSave();
      e->accept();
      break;
    case QMessageBox::Discard:
      e->accept();
      break;
    case QMessageBox::Cancel:
    default: // just for sanity
      e->ignore();
      break;
  }

}

void maintainBudget::sAccountsAdd()
{
  ParameterList params;
  params.append("type", (GLCluster::cAsset | GLCluster::cLiability | GLCluster::cExpense | GLCluster::cRevenue | GLCluster::cEquity));

  accountList newdlg(this);
  newdlg.setAttribute(Qt::WA_DeleteOnClose, false);
  newdlg.xtreewidget()->setSelectionMode(QAbstractItemView::ExtendedSelection);
  newdlg.set(params);
  int accnt_id=newdlg.exec();

  if (accnt_id == -1)
    return;

  QList<XTreeWidgetItem*> selected = newdlg.xtreewidget()->selectedItems();


  for (int i = 0; i < selected.size(); i++)
  {
    int accntid = selected[i]->id();

    if (_prjid != -1)
    {
      q.prepare("SELECT getPrjAccntId(:prj_id,:accnt_id) AS accnt_id");
      q.bindValue(":prj_id", _prjid);
      q.bindValue(":accnt_id", accntid);
      q.exec();
      if (q.first())
        accntid = q.value("accnt_id").toInt();
    }

    if (_accountsRef.contains(accntid))
      continue;

    q.prepare("SELECT formatGLAccountLong(:accnt_id) AS result;");
    q.bindValue(":accnt_id", accntid);
    q.exec();
    if(q.first())
    {
      new XTreeWidgetItem(_accounts,
                          accntid,
                          q.value("result"));
      _accountsRef.append(accntid);
    }
  }
}

void maintainBudget::sAccountsRemove()
{
  QList<XTreeWidgetItem*> selected = _accounts->selectedItems();
  for (int i = 0; i < selected.count(); i++)
    _accounts->takeTopLevelItem(_accounts->indexOfTopLevelItem(selected[i]));

  sGenerateTable();
}

void maintainBudget::sPeriodsAll()
{
  _periods->selectAll();
}

void maintainBudget::sPeriodsInvert()
{
  for (int row = 0; row < _periods->count(); row++)
    _periods->setCurrentItem(_periods->item(row),
                             QItemSelectionModel::Toggle);
}

void maintainBudget::sValueChanged(QTableWidgetItem * /* item */)
{
  _dirty = true;
}

void maintainBudget::sGenerateTable()
{
  _generate->setFocus();
  if(_dirty)
  {
    switch(QMessageBox::question(this, tr("Unsaved Changes"),
                                 tr("<p>The document has been changed "
                                    "since the last save.<br>Do you want "
                                    "to save your changes?"),
                                 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                 QMessageBox::Save)) {
      case QMessageBox::Save:
        sSave();
        break;
      case QMessageBox::Discard:
        break;
      case QMessageBox::Cancel:
      default: // just for sanity
        return;
        break;
    }
  }

  _dirty = false;

  _accountsRef.clear();
  _periodsRef.clear();
  _table->setRowCount(0);
  _table->setColumnCount(0);

  QStringList accounts;
  QStringList periods;

  for (int row = 0; row < _accounts->topLevelItemCount(); row++)
  {
    XTreeWidgetItem* item = _accounts->topLevelItem(row);
    _accountsRef.append(item->id());
    accounts.append(item->text(0));
  }

  QList<QListWidgetItem *>periodlist = _periods->selectedItems();
  for (int i = 0; i < periodlist.size(); i++)
  {
    XListBoxText *item = dynamic_cast<XListBoxText*>(periodlist.at(i));
    if (item)
    {
      _periodsRef.prepend(item->id());
      periods.prepend(item->text());
    }
  }
  if (periodlist.isEmpty())
  {
    QMessageBox::critical(this, tr("Incomplete criteria"),
                          tr("<p>Please select at least one Period "
                             "before generating the table." ) );
    return;
  }
  
  _periodsRef.prepend(-1);
  periods.prepend(tr("Account"));
  _table->setColumnCount(periods.count());
  _table->setHorizontalHeaderLabels(periods);

  periods.clear();
  for(int i = 0; i < _periodsRef.count(); i++)
  {
    if (_periodsRef.at(i) != -1)
      periods.append(QString::number(_periodsRef.at(i)));
  }

  QString sql = "SELECT budgitem_period_id, budgitem_amount"
                "  FROM budgitem"
                " WHERE ((budgitem_accnt_id=:accnt_id)"
                "   AND  (budgitem_budghead_id=:budghead_id)"
                "   AND  (budgitem_period_id IN (" + periods.join(",") + ")) ); ";
  q.prepare(sql);

  _table->setRowCount(_accountsRef.count());
  _table->setColumnCount(periods.size() + 1);   // + 1 to hold account number
  
  for(int i = 0; i < _accountsRef.count(); i++)
  {
    QTableWidgetItem *item = new QTableWidgetItem();
    _table->setItem(i, 0, item);
    item->setText(accounts.at(i));

    q.bindValue(":accnt_id", _accountsRef.at(i));
    q.bindValue(":budghead_id", _budgheadid);
    q.exec();
    while(q.next())
    {
      item = new QTableWidgetItem(q.value("budgitem_amount").toString());
      _table->setItem(i, _periodsRef.indexOf(q.value("budgitem_period_id").toInt()), item);
    }
    _table->item(i, 0)->setFlags(_table->item(i, 0)->flags() & (~Qt::ItemIsEditable));
  }

  _dirty = false;
}

void maintainBudget::populate()
{
  q.prepare("SELECT budghead_name, budghead_descrip"
            "  FROM budghead"
            " WHERE(budghead_id=:budghead_id);");
  q.bindValue(":budghead_id", _budgheadid);
  q.exec();
  if(q.first())
  {
    _name->setText(q.value("budghead_name").toString());
    _descrip->setText(q.value("budghead_descrip").toString());

    q.prepare("SELECT DISTINCT budgitem_accnt_id, formatGLAccountLong(budgitem_accnt_id) AS result"
              "  FROM budgitem JOIN accnt ON (accnt_id=budgitem_accnt_id)"
              " WHERE(budgitem_budghead_id=:budghead_id)"
              " ORDER BY result;");
    q.bindValue(":budghead_id", _budgheadid);
    q.exec();
    while(q.next())
      /*XTreeWidgetItem *item =*/ new XTreeWidgetItem(_accounts,
                                                  q.value("budgitem_accnt_id").toInt(),
                                                  q.value("result"));


    q.prepare("SELECT DISTINCT budgitem_period_id"
              "  FROM budgitem"
              " WHERE(budgitem_budghead_id=:budghead_id)"
              " ORDER BY budgitem_period_id;");
    q.bindValue(":budghead_id", _budgheadid);
    q.exec();
    while(q.next())
    {
      for (int row = 0; _periods->item(row); row++)
      {
        XListBoxText* item = (XListBoxText*)_periods->item(row);
        if(item->id() == q.value("budgitem_period_id").toInt())
        {
          _periods->setCurrentItem(item, QItemSelectionModel::Select);
          break;
        }
      }
    }
    sGenerateTable();
  }
}

void maintainBudget::sPrint()
{
  ParameterList params;
  params.append("budghead_id", _budgheadid);

  orReport report("Budget", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}
