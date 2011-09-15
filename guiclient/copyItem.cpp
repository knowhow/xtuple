/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "copyItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "itemSite.h"
#include "storedProcErrorLookup.h"

copyItem::copyItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_source, SIGNAL(typeChanged(const QString&)), this, SLOT(sHandleItemType(const QString&)));

  _captive = FALSE;
}

copyItem::~copyItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void copyItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copyItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _source->setId(param.toInt());
    _source->setEnabled(FALSE);
    _targetItemNumber->setFocus();
  }

  return NoError;
}

void copyItem::sHandleItemType(const QString &pItemType)
{
  if  (pItemType == "M" || pItemType == "B" ||
       pItemType == "F" || pItemType == "K" ||
       pItemType == "P" || pItemType == "O" ||
       pItemType == "L" || pItemType == "J")
  {
    _copyBOM->setChecked(TRUE);
    _copyBOM->setEnabled(TRUE);
  }
  else
  {
    _copyBOM->setChecked(FALSE);
    _copyBOM->setEnabled(FALSE);
  }
}

bool copyItem::okToSave()
{
  _targetItemNumber->setText(_targetItemNumber->text().trimmed().toUpper());

  if (_targetItemNumber->text().length() == 0)
  {
    QMessageBox::warning(this, tr("Enter Item Number"),
                         tr("<p>Please enter a Target Item Number."));
    _targetItemNumber->setFocus();
    return false;
  }

  q.prepare( "SELECT item_number "
             "FROM item "
             "WHERE item_number=:item_number;" );
  q.bindValue(":item_number", _targetItemNumber->text());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical(this, tr("Item Number Exists"),
                          tr("<p>An Item with the item number '%1' already "
                             "exists. You may not copy over an existing item.")
                            .arg(_targetItemNumber->text()));

    _targetItemNumber->clear();
    _targetItemNumber->setFocus();
    return false;
  }

  return true;
}

void copyItem::createItemSites(int pItemid)
{
  if (QMessageBox::question(this,tr("Create New Item Sites"),
                            tr("<p>Would you like to create new Item Sites "
                               "for the newly created Item?"),
                            QMessageBox::Yes | QMessageBox::No,  QMessageBox::Yes) == QMessageBox::Yes)
  {
    ParameterList params;
    params.append("mode", "new");
    params.append("item_id", pItemid);
    
    itemSite newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void copyItem::sCopy()
{
  if (! okToSave())
    return;

  int itemid = -1;

  q.prepare("SELECT copyItem(:source_item_id, :newItemNumber, :copyBOM, :copyItemCosts) AS itemid;");
  q.bindValue(":source_item_id", _source->id());
  q.bindValue(":newItemNumber", _targetItemNumber->text());
  q.bindValue(":copyBOM",       QVariant(_copyBOM->isChecked()));
  q.bindValue(":copyItemCosts", QVariant(_copyCosts->isChecked()));
  q.exec();
  if (q.first())
  {
    itemid = q.value("itemid").toInt();
    if (itemid < 0)
    {
      systemError(this, storedProcErrorLookup("copyItem", itemid),
                  __FILE__, __LINE__);
      return;
    }

    omfgThis->sItemsUpdated(itemid, TRUE);

    if (_copyBOM->isChecked())
      omfgThis->sBOMsUpdated(itemid, TRUE);

    createItemSites(itemid);

  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
    done(itemid);
  else
    clear();
}

void copyItem::clear()
{
  _source->setId(-1);
  _targetItemNumber->clear();
  _source->setFocus();
  _copyBOM->setEnabled(TRUE);
  _close->setText(tr("&Close"));
}
