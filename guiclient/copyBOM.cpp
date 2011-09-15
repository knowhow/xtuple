/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "copyBOM.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a copyBOM as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
copyBOM::copyBOM(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_source, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_target, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _captive = FALSE;

  _source->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased |
                   ItemLineEdit::cPhantom | ItemLineEdit::cKit |
                   ItemLineEdit::cPlanning);
  _target->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased |
                   ItemLineEdit::cPhantom | ItemLineEdit::cKit |
                   ItemLineEdit::cPlanning);
}

/*
 *  Destroys the object and frees any allocated resources
 */
copyBOM::~copyBOM()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void copyBOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copyBOM::set(const ParameterList &pParams)
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
    _target->setFocus();
  }

  return NoError;
}

void copyBOM::sCopy()
{
  q.prepare( "SELECT bomitem_id "
             "FROM bomitem "
             "WHERE (bomitem_parent_item_id=:item_id) "
             "LIMIT 1;" );
  q.bindValue(":item_id", _source->id());
  q.exec();
  if (!q.first())
    QMessageBox::information( this, tr("Non-Existent Bill of Materials"),
                              tr("The selected target Item does not have any Bill of Material Component Items associated with it.") );

  else
  {
    q.prepare( "SELECT bomitem_id "
               "FROM bomitem "
               "WHERE ( (bomitem_expires > CURRENT_DATE)"
               " AND (bomitem_parent_item_id=:item_id) ) "
               "LIMIT 1;" );
    q.bindValue(":item_id", _target->id());
    q.exec();
    if (q.first())
      QMessageBox::information( this, tr("Existing Bill of Materials"),
                                tr( "The selected target Item already has a Bill of Materials associated with it.\n"
                                    "You must first delete the Bill of Materials for the selected target item before\n"
                                    "attempting to copy an existing Bill of Materials." ) );
    else
    {
	
      q.prepare("SELECT bomitem_id "
		        "FROM bomitem(:item_id) "
                "WHERE ( (bomitem_booitem_seq_id != -1) "
                " AND (bomitem_booitem_seq_id IS NOT NULL) ) "
                "LIMIT 1;" );
      q.bindValue(":item_id", _source->id());
      q.exec();
      if (q.first())
        QMessageBox::information( this, tr("Dependent BOO Data"),
          tr("One or more of the components for this Bill of Materials make reference to a\n"
             "Bill of Operations. These references cannot be copied and must be added manually.") );
      
      q.prepare("SELECT copyBOM(:sourceid, :targetid) AS result;");
      q.bindValue(":sourceid", _source->id());
      q.bindValue(":targetid", _target->id());
      q.exec();
      if(q.first() && q.value("result").toInt() < 0)
      {
        QMessageBox::information( this, tr("Recursive BOM"),
          tr("The Item you are trying to copy this Bill of Material to is a\n"
             "component item which would cause a recursive Bill of Material.\n"
             "Cannot copy Bill of Material.") );
        return;
      }
      omfgThis->sBOMsUpdated(_target->id(), TRUE);
    }
  }

  if (_captive)
    close();
  else
  {
    _source->setId(-1);
    _target->setId(-1);
    _source->setFocus();
  }
}

void copyBOM::sHandleButtons()
{
  _copy->setEnabled(_source->isValid() && _target->isValid());
}
