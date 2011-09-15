/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "uomConv.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a uomConv as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
uomConv::uomConv(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _uomidFrom = -1;
  _uomconvid = -1;
  _ignoreSignals = false;

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_uomFrom, SIGNAL(currentIndexChanged(int)), this, SLOT(sFromChanged()));
  connect(_uomTo, SIGNAL(currentIndexChanged(int)), this, SLOT(sToChanged()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));

  _uomFrom->setType(XComboBox::UOMs);
  _uomTo->setType(XComboBox::UOMs);
  _fromValue->setValidator(omfgThis->ratioVal());
  _toValue->setValidator(omfgThis->ratioVal());
}

/*
 *  Destroys the object and frees any allocated resources
 */
uomConv::~uomConv()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void uomConv::languageChange()
{
  retranslateUi(this);
}

enum SetResponse uomConv::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("from_uom_id", &valid);
  if (valid)
  {
    _uomidFrom = param.toInt();
    _ignoreSignals = true;
    _uomFrom->setId(_uomidFrom);
    _uomTo->setId(_uomidFrom);
    _ignoreSignals = false;
  }

  param = pParams.value("uomconv_id", &valid);
  if (valid)
  {
    _uomconvid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _uomTo->setEnabled(false);
      _uomFrom->setEnabled(false);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _fromValue->setEnabled(false);
      _toValue->setEnabled(false);
      _uomTo->setEnabled(false);
      _uomFrom->setEnabled(false);
      _fractional->setEnabled(false);
      _cancel->setText(tr("&Close"));
      _save->hide();

      _cancel->setFocus();
    }
  }

  return NoError;
}

void uomConv::sSave()
{
  bool valid;
  if (_fromValue->toDouble(&valid) == 0)
  {
    QMessageBox::information( this, tr("No Ratio Entered"),
                              tr("You must enter a valid Ratio before saving this UOM Conversion.") );
    _fromValue->setFocus();
    return;
  }

  if (_toValue->toDouble(&valid) == 0)
  {
    QMessageBox::information( this, tr("No Ratio Entered"),
                              tr("You must enter a valid Ratio before saving this UOM Conversion.") );
    _toValue->setFocus();
    return;
  }

  if (_mode == cEdit)
    q.prepare( "UPDATE uomconv "
               "   SET uomconv_from_value=:uomconv_from_value,"
               "       uomconv_to_value=:uomconv_to_value,"
               "       uomconv_fractional=:uomconv_fractional "
               " WHERE(uomconv_id=:uomconv_id);" );
  else if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('uomconv_uomconv_id_seq') AS uomconv_id");
    if (q.first())
      _uomconvid = q.value("uomconv_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
 
    q.prepare( "INSERT INTO uomconv "
               "( uomconv_id, uomconv_from_uom_id, uomconv_from_value, uomconv_to_uom_id, uomconv_to_value, uomconv_fractional ) "
               "VALUES "
               "( :uomconv_id, :uomconv_from_uom_id, :uomconv_from_value, :uomconv_to_uom_id, :uomconv_to_value, :uomconv_fractional );" );
  }

  q.bindValue(":uomconv_id", _uomconvid);
  q.bindValue(":uomconv_from_uom_id", _uomFrom->id());
  q.bindValue(":uomconv_to_uom_id", _uomTo->id());
  q.bindValue(":uomconv_from_value", _fromValue->toDouble());
  q.bindValue(":uomconv_to_value", _toValue->toDouble());
  q.bindValue(":uomconv_fractional", QVariant(_fractional->isChecked()));
  q.exec();

  accept();
}

void uomConv::sFromChanged()
{
  if(cNew != _mode || _ignoreSignals)
    return;

  if(_uomFrom->id() != _uomidFrom && _uomTo->id() != _uomidFrom)
  {
    _ignoreSignals = true;
    _uomTo->setId(_uomidFrom);
    _ignoreSignals = false;
  }
  sCheck();
}

void uomConv::sToChanged()
{
  if(cNew != _mode || _ignoreSignals)
    return;

  if(_uomFrom->id() != _uomidFrom && _uomTo->id() != _uomidFrom)
  {
    _ignoreSignals = true;
    _uomFrom->setId(_uomidFrom);
    _ignoreSignals = false;
  }
  sCheck();
}

void uomConv::sCheck()
{
  if ( (_mode == cNew) )
  {
    q.prepare( "SELECT uomconv_id"
               "  FROM uomconv"
               " WHERE((uomconv_from_uom_id=:from AND uomconv_to_uom_id=:to)"
               "    OR (uomconv_from_uom_id=:to AND uomconv_to_uom_id=:from));" );
    q.bindValue(":from", _uomFrom->id());
    q.bindValue(":to", _uomTo->id());
    q.exec();
    if (q.first())
    {
      _uomconvid = q.value("uomconv_id").toInt();
      _mode = cEdit;
      _uomTo->setEnabled(false);
      _uomFrom->setEnabled(false);
      populate();
    }
  }
}

void uomConv::populate()
{
  q.prepare( "SELECT uomconv_from_uom_id,"
             "       uomconv_from_value,"
             "       uomconv_to_uom_id,"
             "       uomconv_to_value,"
             "       uomconv_fractional "
             "  FROM uomconv"
             " WHERE(uomconv_id=:uomconv_id);" );
  q.bindValue(":uomconv_id", _uomconvid);
  q.exec();
  if (q.first())
  {
    _uomFrom->setId(q.value("uomconv_from_uom_id").toInt());
    _uomTo->setId(q.value("uomconv_to_uom_id").toInt());
    _fromValue->setText(q.value("uomconv_from_value").toDouble());
    _toValue->setText(q.value("uomconv_to_value").toDouble());
    _fractional->setChecked(q.value("uomconv_fractional").toBool());
  }
}

