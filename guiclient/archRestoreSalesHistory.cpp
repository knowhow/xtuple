/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "archRestoreSalesHistory.h"

#include <QVariant>

#define cArchive 0x01
#define cRestore  0x02

archRestoreSalesHistory::archRestoreSalesHistory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSelect()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  _customerType->setType(ParameterGroup::CustomerType);
  _productCategory->setType(ParameterGroup::ProductCategory);
}

archRestoreSalesHistory::~archRestoreSalesHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

void archRestoreSalesHistory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse archRestoreSalesHistory::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("archieve", &valid);
  if (valid)
  {
    _mode = cArchive;

    setWindowTitle(tr("Archive Sales History"));
    _miscItems->setText(tr("Archive Freight, Sales Tax and Misc. Items"));
  }

  param = pParams.value("restore", &valid);
  if (valid)
  {
    _mode = cRestore;

    setWindowTitle(tr("Restore Sales History"));
    _miscItems->setText(tr("Restore Freight, Sales Tax and Misc. Items"));
  }

  return NoError;
}

void archRestoreSalesHistory::sSelect()
{
  QString sql;

  if (_mode == cArchive)
    sql = "SELECT archiveSalesHistory(cohist_id) "
          "FROM cust, custtype,"
          "     cohist LEFT OUTER JOIN"
          "     ( itemsite JOIN"
          "       ( item JOIN prodcat"
          "         ON (item_prodcat_id=prodcat_id) )"
          "       ON (itemsite_item_id=item_id) )"
          "     ON (cohist_itemsite_id=itemsite_id) "
          "WHERE ( (cohist_cust_id=cust_id)"
          " AND (cust_custtype_id=custtype_id)"
          " AND (cohist_invcdate BETWEEN :startDate AND :endDate)";
  else if (_mode == cRestore)
    sql = "SELECT restoreSalesHistory(asohist_id) "
          "FROM cust, custtype,"
          "     asohist LEFT OUTER JOIN"
          "     ( itemsite JOIN"
          "       ( item JOIN prodcat"
          "         ON (item_prodcat_id=prodcat_id) )"
          "       ON (itemsite_item_id=item_id) )"
          "     ON (asohist_itemsite_id=itemsite_id) "
          "WHERE ( (asohist_cust_id=cust_id)"
          " AND (cust_custtype_id=custtype_id)"
          " AND (asohist_invcdate BETWEEN :startDate AND :endDate)";

  if (_miscItems->isChecked())
  {
    if (_warehouse->isSelected())
      sql += " AND ( (itemsite_id IS NULL) OR (itemsite_warehous_id=:warehous_id) )";

    if (_productCategory->isChecked())
      sql += " AND ( (itemsite_id IS NULL) OR (prodcat_id=:prodcat_id) )";
    else if (_productCategory->isPattern())
      sql += " AND ( (itemsite_id IS NULL) OR (prodcat_code ~ :prodcat_pattern) )";
  }
  else
  {
    sql += " AND (itemsite_id IS NOT NULL)";

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    if (_productCategory->isChecked())
      sql += " AND (prodcat_id=:prodcat_id)";
    else if (_productCategory->isPattern())
      sql += " AND (prodcat_code ~ :prodcat_pattern)";

  }

  if (_customerType->isSelected())
    sql += " AND (custtype_id=:custtype_id)";
  else if (_customerType->isPattern())
    sql += " AND (custtype_code ~ :custtype_pattern)";

  sql += " );";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _customerType->bindValue(q);
  _productCategory->bindValue(q);
  _dates->bindValue(q);
  q.exec();

  accept();
}

