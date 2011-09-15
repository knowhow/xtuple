/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "voucherItemDistrib.h"

#include <qvariant.h>
#include <qvalidator.h>
/*
 *  Constructs a voucherItemDistrib as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
voucherItemDistrib::voucherItemDistrib(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_costelem, SIGNAL(newID(int)), this, SLOT(sCheck()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
voucherItemDistrib::~voucherItemDistrib()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void voucherItemDistrib::languageChange()
{
    retranslateUi(this);
}


void voucherItemDistrib::init()
{
  _costelem->populate( QString( "SELECT costelem_id, costelem_type, 1 AS orderby "
                                "FROM costelem "
                                "WHERE (costelem_type='Material') "

                                "UNION SELECT 0, '%1' AS costelem_type, 2 AS orderby "

                                "UNION SELECT costelem_id, costelem_type, 3 AS orderby "
                                "FROM costelem "
                                "WHERE ( (costelem_active)"
                                " AND (NOT costelem_sys)"
                                " AND (costelem_po) ) "
                                "ORDER BY orderby, costelem_type;" )
                       .arg(tr("None")) );
}

enum SetResponse voucherItemDistrib::set(ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("poitem_id", &valid);
  if (valid)
    _poitemid = param.toInt();

  param = pParams.value("vohead_id", &valid);
  if (valid)
    _voheadid = param.toInt();

  param = pParams.value("curr_id", &valid);
  if (valid)
    _amount->setId(param.toInt());

  param = pParams.value("effective", &valid);
  if (valid)
    _amount->setEffective(param.toDate());

  param = pParams.value("vodist_id", &valid);
  if (valid)
  {
    _vodistid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      param = pParams.value("amount", &valid);
      if (valid)
        _amount->setLocalValue(param.toDouble());

      _amount->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _costelem->setEnabled(false);
      _save->setFocus();
    }
  }

  return NoError;
}

void voucherItemDistrib::populate()
{
  q.prepare( "SELECT * "
             "FROM vodist "
             "WHERE (vodist_id=:vodist_id);" ) ;
  q.bindValue(":vodist_id", _vodistid);
  q.exec();
  if (q.first())
  {
    _costelem->setId(q.value("vodist_costelem_id").toInt());
    _amount->setLocalValue(q.value("vodist_amount").toDouble());
    _discountable->setChecked(q.value("vodist_discountable").toBool());
    _notes->setText(q.value("vodist_notes").toString());
  }
}

bool voucherItemDistrib::sCheck()
{
  if (_mode == cNew)
  {
    q.prepare( "SELECT vodist_id "
               "FROM vodist "
               "WHERE ( (vodist_vohead_id=:vodist_vohead_id)"
               "  AND   (vodist_poitem_id=:vodist_poitem_id)"
               "  AND   (vodist_costelem_id=:vodist_costelem_id) );" );
    q.bindValue(":vodist_vohead_id", _voheadid);
    q.bindValue(":vodist_poitem_id", _poitemid);
    q.bindValue(":vodist_costelem_id", _costelem->id());
    q.exec();
    if (q.first())
    {
      _vodistid = q.value("vodist_id").toInt();
      _mode = cEdit;
      populate();

      _costelem->setEnabled(FALSE);
      return false;
    }
  }
  return true;
}

void voucherItemDistrib::sSave()
{
  if (_mode == cNew)
  {
    if (!sCheck())
      return;
      
    q.exec("SELECT NEXTVAL('vodist_vodist_id_seq') AS _vodistid;");
    if (q.first())
      _vodistid = q.value("_vodistid").toInt();

    q.prepare( "INSERT INTO vodist "
               "( vodist_id, vodist_vohead_id, vodist_poitem_id,"
               "  vodist_costelem_id, vodist_amount, vodist_discountable,"
			   "  vodist_notes ) "
               "VALUES "
               "( :vodist_id, :vodist_vohead_id, :vodist_poitem_id,"
               "  :vodist_costelem_id, :vodist_amount, :vodist_discountable,"
			   "  :vodist_notes );" );
  }
  if (_mode == cEdit)
    q.prepare( "UPDATE vodist "
               "SET vodist_costelem_id=:vodist_costelem_id,"
               "    vodist_amount=:vodist_amount,"
               "    vodist_discountable=:vodist_discountable,"
			   "    vodist_notes=:vodist_notes "
               "WHERE (vodist_id=:vodist_id);" );

  q.bindValue(":vodist_id", _vodistid);
  q.bindValue(":vodist_vohead_id", _voheadid);
  q.bindValue(":vodist_poitem_id", _poitemid);
  q.bindValue(":vodist_costelem_id", _costelem->id());
  q.bindValue(":vodist_amount", _amount->localValue());
  q.bindValue(":vodist_discountable", QVariant(_discountable->isChecked()));
  q.bindValue(":vodist_notes", _notes->toPlainText().trimmed());
  q.exec();

  done(_vodistid);
}
  
