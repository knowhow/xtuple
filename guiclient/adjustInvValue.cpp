/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "adjustInvValue.h"

#include <metasql.h>
#include "mqlutil.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "storedProcErrorLookup.h"

adjustInvValue::adjustInvValue(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_altAccnt,                       SIGNAL(toggled(bool)), this, SLOT(sToggleAltAccnt(bool)));
  connect(_post,                           SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_site,                           SIGNAL(newID(int)), this, SLOT(sValidateSite(int)));
  connect(_item,                           SIGNAL(newId(int)), this, SLOT(sPopulate()));
  connect(_newValue,                       SIGNAL(textChanged(QString)), this, SLOT(sUpdateCost()));

  _captive = FALSE;

  _item->setType(ItemLineEdit::cActive);
  _item->addExtraClause( QString("(itemsite_active)") );  // ItemLineEdit::cActive doesn't compare against the itemsite record
  _item->addExtraClause( QString("(itemsite_costmethod='A')") );
  _site->setType(WComboBox::AllActiveInventory);
  _newValue->setValidator(omfgThis->costVal());

  if (!_metrics->boolean("MultiWhs"))
  {
    _siteLit->hide();
    _site->hide();
  }

  _itemsiteid = -1;
  _qtyonhand = -0;
  sValidateSite(_site->id());
  _item->setFocus();
}

adjustInvValue::~adjustInvValue()
{
  // no need to delete child widgets, Qt does it all for us
}

void adjustInvValue::languageChange()
{
  retranslateUi(this);
}

enum SetResponse adjustInvValue::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _site->setEnabled(FALSE);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      if (!_item->isValid())
        _item->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _site->setEnabled(FALSE);
      _post->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  return NoError;
}

void adjustInvValue::sPopulate()
{
  if (_item->id() == -1)
  {
    _itemsiteid = -1;
    _qoh->setText("0.0");
    _qtyonhand = 0.0;
    _currentValue->setText("0.0");
    _currentCost->setText("0.0");
    _newCost->setText("0.0");
    _post->setEnabled(false);
  }
  else
  {
    QString sql = "SELECT itemsite_id, itemsite_qtyonhand, "
             "       formatQty(itemsite_qtyonhand) AS f_qoh, "
             "       formatMoney(itemsite_value) AS f_value, "
             "       CASE WHEN itemsite_qtyonhand > 0 THEN "
             "         formatCost(itemsite_value / itemsite_qtyonhand) "
             "       ELSE formatCost(0) END AS f_cost "
             "FROM itemsite "
             "WHERE ((itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
             "  AND (itemsite_item_id=<? value(\"item_id\") ?>))";

    ParameterList params;
    params.append("warehous_id", _site->id());
    params.append("item_id", _item->id());
    MetaSQLQuery mql(sql);
    q = mql.toQuery(params);
    if (q.first())
    {
      _itemsiteid = q.value("itemsite_id").toInt();
      _qtyonhand = q.value("itemsite_qtyonhand").toDouble();
      _qoh->setText(q.value("f_qoh").toString());
      _currentValue->setText(q.value("f_value").toString());
      _currentCost->setText(q.value("f_cost").toString());
      _post->setEnabled(true);
    }
    else
    {
      _itemsiteid = -1;
      _qoh->setText("0.0");
      _qtyonhand = 0.0;
      _currentValue->setText("0.0");
      _currentCost->setText("0.0");
      _newCost->setText("0.0");
      _post->setEnabled(false);
    }
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void adjustInvValue::sPost()
{
  struct {
    bool        condition;
    QString     msg;
    QWidget     *widget;
  } error[] = {
    { ! _itemsiteid == -1,
      tr("You must select a valid Itemsite before posting this transaction."), _item },
    { _newValue->text().length() == 0,
      tr("<p>You must enter a valid New Value before posting this transaction."),
      _newValue },
    { _altAccnt->isChecked() && ! _accnt->isValid(),
      tr("<p>You must enter a valid Alternate G/L Account before posting this transaction."),
      _accnt },
    { true, "", NULL }
  };

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Post Transaction"),
                          error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  q.exec("BEGIN;");
  QString sql = "SELECT <? value(\"newValue\") ?> - itemsite_value AS delta "
       "FROM itemsite "
       "WHERE (itemsite_id=<? value(\"itemsite_id\") ?>) "
       "FOR UPDATE ";

  ParameterList params;
  params.append("newValue", _newValue->toDouble() - 0);  // Use toDouble() on 3.4 and above
  params.append("itemsite_id", _itemsiteid);
  if (_altAccnt->isChecked() && _accnt->id() != -1)
    params.append("accnt_id", _accnt->id());

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.first())
  {
    params.append("delta", q.value("delta").toDouble() - 0);

    sql = "SELECT insertGLTransaction('I/M', '', 'Post Value', "
         "       'Inventory Value Adjustment for ' || item_number, "
         "       COALESCE (<? value(\"accnt_id\") ?>,costcat_adjustment_accnt_id), "
         "       costcat_asset_accnt_id, -1, "
         "       <? value(\"delta\") ?>, current_date) AS glreturn "
         "FROM itemsite "
         " JOIN costcat ON (itemsite_costcat_id=costcat_id) "
         " JOIN item ON (itemsite_item_id=item_id) "
         "WHERE (itemsite_id=<? value(\"itemsite_id\") ?>); "
         "UPDATE itemsite SET "
         "  itemsite_value=<? value(\"newValue\") ?> "
         "WHERE (itemsite_id=<? value(\"itemsite_id\") ?>);";

    MetaSQLQuery mql(sql);
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
    {
      q.exec("ROLLBACK;");
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    q.exec("ROLLBACK;");
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    q.exec("ROLLBACK;");
    QMessageBox::critical( this, tr("Cannot Post"),
                           tr( "<p>Itemsite not found. "
                              "Please see your administrator." ) );
  }

  q.exec("COMMIT;");
  QMessageBox::information( this, tr("Post Successful"),
                         tr( "<p>Value successfully updated. ") );
  sPopulate();
  _newValue->clear();
}

void adjustInvValue::sUpdateCost()
{
  QString sql = "SELECT "
           "       CASE WHEN itemsite_qtyonhand > 0 THEN "
           "         formatCost(<? value(\"newvalue\") ?> / itemsite_qtyonhand) "
           "       ELSE formatCost(0) END AS f_cost "
           "FROM itemsite "
           "WHERE ((itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
           "  AND (itemsite_item_id=<? value(\"item_id\") ?>))";

  ParameterList params;
  params.append("warehous_id", _site->id());
  params.append("item_id", _item->id());
  params.append("newvalue", _newValue->toDouble());
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.first())
  {
    _newCost->setText(q.value("f_cost").toString());
  }
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void adjustInvValue::sValidateSite(int siteid)
{
  if (siteid != -1)
  {
    _item->setEnabled(true);
  }
  else
  {
    _item->setId(-1);
    _item->setEnabled(false);
  }
  sPopulate();
}

void adjustInvValue::sToggleAltAccnt(bool checked)
{
  _accnt->setReadOnly(!checked);
}
