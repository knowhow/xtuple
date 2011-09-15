/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QMessageBox>
#include <QSqlError>

#include "contactemail.h"

/*
 *  This screen will receive the contact id from the calling screen and
 *  populate alternate email addresses from the contact.  Users can add
 *  or delete alternate email addresses.
 */

contactEmail::contactEmail(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : QDialog(parent, fl)
{
  setupUi(this);
  setObjectName(name ? name : "contactEmail");
  setModal(modal);

  _new = _buttonBox->addButton(tr("New"), QDialogButtonBox::ActionRole);
  _new->setObjectName("_new");
  _new->setShortcut(QKeySequence::New);
  _delete = _buttonBox->addButton(tr("Delete"), QDialogButtonBox::ActionRole);
  _delete->setObjectName("_delete");
  _delete->setEnabled(false);

  _list->addColumn("Hidden", -1, Qt::AlignLeading, true, "cntcteml_email");
  _list->setHeaderHidden(true);
  _list->setAlternatingRowColors(false);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_list, SIGNAL(valid(bool)), this, SLOT(sHandleButtons(bool)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_list, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this,
          SLOT(sCloseEdit(QTreeWidgetItem*, QTreeWidgetItem*)));
  connect(_list, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this,
          SLOT(sOpenEdit(QTreeWidgetItem*)));

  _buttonBox->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */

contactEmail::~contactEmail()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void contactEmail::languageChange()
{
  retranslateUi(this);
}

void contactEmail::accept()
{
  XTreeWidgetItem* item;

  // Update the database
  XSqlQuery ins;
  ins.prepare("INSERT INTO cntcteml ( "
              "  cntcteml_cntct_id, cntcteml_email) "
              "VALUES ("
              "  :cntct_id, :email) "
              "RETURNING cntcteml_id;");

  XSqlQuery upd;
  upd.prepare("UPDATE cntcteml SET "
              "  cntcteml_email=:email "
              "WHERE (cntcteml_id=:cntcteml_id);");

  XSqlQuery del;
  del.prepare("DELETE FROM cntcteml "
              "WHERE (cntcteml_id=:cntcteml_id);");

  for (int i = 0; i < _list->topLevelItemCount(); i++)
  {
    item = _list->topLevelItem(i);
    // Inserted
    if (!item->id())
    {
      if (item->data(0,Qt::DisplayRole).toString().isEmpty())
        continue;
      ins.bindValue(":cntct_id", _cntctid);
      ins.bindValue(":email", item->data(0, Qt::DisplayRole));
      ins.exec();
      if(ins.first())
        item->setId(ins.value("cntcteml_id").toInt());
      else if (ins.lastError().type() != QSqlError::NoError)
        QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                              .arg(__FILE__)
                              .arg(__LINE__),
                              ins.lastError().databaseText());
    }
    // Deleted
    else if (item->data(0, Xt::DeletedRole).toBool())
    {
      del.bindValue(":cntcteml_id", item->id());
      del.exec();
      if (del.lastError().type() != QSqlError::NoError)
        QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                              .arg(__FILE__)
                              .arg(__LINE__),
                              del.lastError().databaseText());
    }
    // Updated
    else
    {
      upd.bindValue(":email", item->data(0, Qt::DisplayRole));
      upd.bindValue(":cntcteml_id", item->id());
      upd.exec();
      if (upd.lastError().type() != QSqlError::NoError)
        QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                              .arg(__FILE__)
                              .arg(__LINE__),
                              upd.lastError().databaseText());
    }
  }

  if (_list->id() > 0)
    done(_list->id());
  else
    reject();
}

void contactEmail::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cntct_id", &valid);
  if (valid)
  {
    _cntctid = param.toInt();
    sFillList();
  }
}

void contactEmail::sFillList()
{
  XSqlQuery qry;
  qry.prepare("SELECT cntcteml_id, cntcteml_email "
              "FROM cntcteml "
              "WHERE (cntcteml_cntct_id=:cntct_id);");
  qry.bindValue(":cntct_id", _cntctid);
  qry.exec();
  _list->populate(qry);
}

void contactEmail::sAdd()
{
  XTreeWidgetItem* item = new XTreeWidgetItem(_list, 0, 0, "");
  sOpenEdit(item);
}

void contactEmail::sDelete()
{
  XTreeWidgetItem* item = _list->currentItem();
  item->setData(0, Xt::DeletedRole, true);
  QFont font = item->font(0);
  font.setStrikeOut(true);
  item->setFont(0, font);
  item->setTextColor(Qt::gray);
  _delete->setEnabled(false);
}

void contactEmail::sOpenEdit(QTreeWidgetItem *item)
{
  XTreeWidgetItem* xitem = (XTreeWidgetItem*)item;
  if (xitem)
  {
    _list->openPersistentEditor(xitem,0);
    _list->editItem(xitem,0);
    _delete->setEnabled(false);
  }
}

void contactEmail::sCloseEdit(QTreeWidgetItem* /*current*/, QTreeWidgetItem* previous)
{
  XTreeWidgetItem* xitem = (XTreeWidgetItem*)previous;
  if (xitem)
  {
    _list->closePersistentEditor(xitem,0);
    QString email = xitem->data(0, Qt::DisplayRole).toString().toLower();
    xitem->setData(0, Qt::EditRole, email);
  }
}

void contactEmail::sHandleButtons(bool valid)
{
  if (_list->currentItem())
  {
    bool deleted = _list->currentItem()->data(0, Xt::DeletedRole).toBool();
    _delete->setEnabled(valid && !deleted);
  }
}

