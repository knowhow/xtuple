/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemAlias.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a itemAlias as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemAlias::itemAlias(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _itemaliasid = -1;
  _itemid = -1;
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemAlias::~itemAlias()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemAlias::languageChange()
{
    retranslateUi(this);
}

enum SetResponse itemAlias::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    q.prepare("SELECT item_number FROM item WHERE (item_id=:item_id);");
    q.bindValue(":item_id", _itemid);
    q.exec();
    if(q.first())
      _item->setText(q.value("item_number").toString());
  }

  param = pParams.value("itemalias_id", &valid);
  if (valid)
  {
    _itemaliasid = param.toInt();
    populate();
  }

  param = pParams.value("item_number", &valid);
  if (valid)
    _item->setText(param.toString());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _number->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _number->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _useDescription->setEnabled(FALSE);
      _descrip1->setEnabled(FALSE);
      _descrip2->setEnabled(FALSE);
      _comments->setEnabled(FALSE);

      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void itemAlias::sSave()
{
  if (_mode == cNew)
  {
    q.prepare( "SELECT itemalias_id "
               "FROM itemalias "
               "WHERE ( (itemalias_item_id=:item_id)"
               " AND (itemalias_number=:itemalias_number) );" );
    q.bindValue(":item_id", _itemid);
    q.bindValue(":itemalias_number", _number->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Item Alias"),
                             tr( "An Item Alias for the selected Item Number has already been defined with the selected Alias Item Number.\n"
                                 "You may not create duplicate Item Aliases." ) );
      _number->setFocus();
      return;
    }

    q.exec("SELECT NEXTVAL('itemalias_itemalias_id_seq') AS _itemalias_id;");
    if (q.first())
      _itemaliasid = q.value("_itemalias_id").toInt();

    q.prepare( "INSERT INTO itemalias "
               "( itemalias_id, itemalias_item_id, itemalias_number,"
               "  itemalias_usedescrip, itemalias_descrip1, itemalias_descrip2,"
               "  itemalias_comments ) "
               "VALUES "
               "( :itemalias_id, :itemalias_item_id, :itemalias_number,"
               "  :itemalias_usedescrip, :itemalias_descrip1, :itemalias_descrip2,"
               "  :itemalias_comments );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE itemalias "
               "SET itemalias_number=:itemalias_number, itemalias_comments=:itemalias_comments,"
               "    itemalias_usedescrip=:itemalias_usedescrip,"
               "    itemalias_descrip1=:itemalias_descrip1, itemalias_descrip2=:itemalias_descrip2 "
               "WHERE (itemalias_id=:itemalias_id);" );

  q.bindValue(":itemalias_id", _itemaliasid);
  q.bindValue(":itemalias_item_id", _itemid);
  q.bindValue(":itemalias_number", _number->text().trimmed());
  q.bindValue(":itemalias_descrip1", _descrip1->text().trimmed());
  q.bindValue(":itemalias_descrip2", _descrip2->text().trimmed());
  q.bindValue(":itemalias_comments", _comments->toPlainText());
  q.bindValue(":itemalias_usedescrip", QVariant(_useDescription->isChecked()));
  q.exec();

  done(_itemaliasid);
}

void itemAlias::populate()
{
  q.prepare( "SELECT itemalias_item_id, itemalias_number,"
             "       itemalias_usedescrip, itemalias_descrip1, itemalias_descrip2,"
             "       itemalias_comments, item_number "
             "FROM itemalias LEFT OUTER JOIN item ON (itemalias_item_id=item_id) "
             "WHERE (itemalias_id=:itemalias_id);" );
  q.bindValue(":itemalias_id", _itemaliasid);
  q.exec();
  if (q.first())
  {
    _itemid = q.value("itemalias_item_id").toInt();
    _item->setText(q.value("item_number").toString());
    _number->setText(q.value("itemalias_number").toString());

    if (q.value("itemalias_usedescrip").toBool())
    {
      _useDescription->setChecked(TRUE);
      _descriptionGroup->setEnabled(TRUE);
      _descrip1->setText(q.value("itemalias_descrip1").toString());
      _descrip2->setText(q.value("itemalias_descrip2").toString());
    }
    else
    {
      _useDescription->setChecked(FALSE);
      _descriptionGroup->setEnabled(FALSE);
    }

    _comments->setText(q.value("itemalias_comments").toString());
  }
}

