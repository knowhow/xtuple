/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "warehouse.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include "warehouseZone.h"

warehouse::warehouse(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_code,       SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_delete,	 SIGNAL(clicked()), this, SLOT(sDeleteZone()));
  connect(_edit,	 SIGNAL(clicked()), this, SLOT(sEditZone()));
  connect(_new,		 SIGNAL(clicked()), this, SLOT(sNewZone()));
  connect(_save,	 SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_standard, SIGNAL(toggled(bool)), this, SLOT(sHandleWhsType()));
  connect(_transit,  SIGNAL(toggled(bool)), this, SLOT(sHandleWhsType()));

  connect(_address, SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _contact, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

  _whsezone->addColumn(tr("Name"),        _itemColumn, Qt::AlignCenter, true,  "whsezone_name" );
  _whsezone->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "whsezone_descrip"   );
  
  //Handle single warehouse scenario
  if (!_metrics->boolean("MultiWhs"))
  {
    ParameterList params;
    q.exec("SELECT warehous_id "
              "FROM whsinfo; ");
    if (q.first())
    {
      params.append("mode", "edit");
      params.append("warehous_id", q.value("warehous_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
      params.append("mode", "new");
    set(params);

    _active->setChecked(TRUE);
    _active->hide();
  }

  _standard->setVisible(_metrics->boolean("MultiWhs"));
  _transit->setVisible(_metrics->boolean("MultiWhs"));
}

warehouse::~warehouse()
{
  // no need to delete child widgets, Qt does it all for us
}

void warehouse::languageChange()
{
  retranslateUi(this);
}

enum SetResponse warehouse::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _warehousid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _code->setFocus();

      q.exec("SELECT NEXTVAL('warehous_warehous_id_seq') AS warehous_id");
      if (q.first())
        _warehousid = q.value("warehous_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      connect(_whsezone, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_whsezone, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_whsezone, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _description->setFocus();

      connect(_whsezone, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_whsezone, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_whsezone, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _code->setEnabled(FALSE);
      _sitetype->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _contact->setEnabled(FALSE);
      _address->setEnabled(FALSE);
      _defaultFOB->setEnabled(FALSE);
      _bolPrefix->setEnabled(FALSE);
      _bolNumber->setEnabled(FALSE);
      _shipping->setEnabled(FALSE);
      _countTagPrefix->setEnabled(FALSE);
      _countTagNumber->setEnabled(FALSE);
      _sequence->setEnabled(FALSE);
      _useSlips->setEnabled(FALSE);
      _arblGroup->setEnabled(FALSE);
      _useZones->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _account->setEnabled(FALSE);
      _shipcomm->setEnabled(FALSE);
      _taxzone->setEnabled(FALSE);
      _comments->setReadOnly(TRUE);
      _transit->setEnabled(FALSE);
      _shipform->setEnabled(FALSE);
      _shipvia->setEnabled(FALSE);
      _shipcomments->setEnabled(FALSE);

      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void warehouse::sSave()
{
  //  Make sure that at least a warehouse code has been entered
  if (_code->text().trimmed().length() == 0)
  {
    QMessageBox::information( this, tr("Cannot Save Site"),
                              tr("<p>You must enter a code for this Site "
				 "before saving it.")  );
    _code->setFocus();
    return;
  }

  //  Make sure that a Site Type has been entered
  if (!_sitetype->isValid())
  {
    QMessageBox::information( this, tr("Cannot Save Site"),
                 tr("<p>You must enter a Type for this "
				 "Site before saving it.")  );
    _sitetype->setFocus();
    return;
  }

  //  Make sure that the Count Tag prefix is unique
  if (_mode != cNew)
  {
    q.prepare( "SELECT warehous_id "
               "FROM warehous "
               "WHERE ( (warehous_counttag_prefix=:prefix)"
               " AND (warehous_id<>:warehous_id) );" );

    q.bindValue(":warehous_id", _warehousid);
  }
  else
    q.prepare( "SELECT warehous_id "
               "FROM warehous "
               "WHERE (warehous_counttag_prefix=:prefix)" );

  q.bindValue(":prefix", _countTagPrefix->text());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Count Tag Prefix not Unique"),
                           tr("<p>The Count Tag prefix entered has been used "
			      "in another Site. To enable Count Tag "
			      "audits, the application requires a unique Count "
			      "Tag prefix for each Site. Please enter a "
			      "different Count Tag prefix." ) );
    _countTagPrefix->clear();
    _countTagPrefix->setFocus();
    return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare( "SELECT warehous_id "
	     "FROM warehous "
	     "WHERE ( (warehous_id<>:warehous_id)"
	     " AND (UPPER(warehous_code)=UPPER(:warehous_code)) );" );
  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":warehous_code", _code->text());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Save Site"),
			   tr( "<p>The new Site information cannot be "
			      "saved as the new Site Code that you "
			      "entered conflicts with an existing Site. "
			      "You must uniquely name this Site before "
			      "you may save it." ) );
    _code->setFocus();
    return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  //  Make sure that a default G/L Account has been entered
  if (_account->id() == -1)
  {
    QMessageBox::information( this, tr("Cannot Save Site"),
                              tr("<p>You must enter a default Account for this "
				 "Site before saving it.")  );
    _account->setFocus();
    return;
  }

  if (_transit->isChecked() && ! _costcat->isValid())
  {
    QMessageBox::information(this, tr("Cannot Save Site"),
			     tr("<p>You must select a Cost Category for this "
				"Transit Site before saving it.") );
    _costcat->setFocus();
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  if (! q.exec("BEGIN"))
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  int saveResult = _address->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("There are multiple uses of this Site "
		       "Address.\nWhat would you like to do?"),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);
    if (0 == answer)
      saveResult = _address->save(AddressCluster::CHANGEONE);
    else if (1 == answer)
      saveResult = _address->save(AddressCluster::CHANGEALL);
  }
  if (saveResult < 0)	// not else-if: this is error check for CHANGE{ONE,ALL}
  {
    rollback.exec();
    systemError(this, tr("There was an error saving this address (%1).\n"
			 "Check the database server log for errors.")
		      .arg(saveResult), __FILE__, __LINE__);
    _address->setFocus();
    return;
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO whsinfo "
               "( warehous_id, warehous_code, warehous_descrip,"
               "  warehous_cntct_id, warehous_fob, warehous_active,"
               "  warehous_bol_prefix, warehous_bol_number, warehous_shipping,"
               "  warehous_counttag_prefix, warehous_counttag_number, warehous_useslips,"
               "  warehous_aislesize, warehous_aislealpha,"
               "  warehous_racksize, warehous_rackalpha,"
               "  warehous_binsize, warehous_binalpha,"
               "  warehous_locationsize, warehous_locationalpha,"
               "  warehous_enforcearbl, warehous_usezones, "
               "  warehous_default_accnt_id, warehous_shipping_commission, "
               "  warehous_addr_id, warehous_taxzone_id, warehous_transit,"
               "  warehous_shipform_id, warehous_shipvia_id,"
               "  warehous_shipcomments, warehous_costcat_id, warehous_sitetype_id,"
               "  warehous_sequence ) "
               "VALUES "
               "( :warehous_id, :warehous_code, :warehous_descrip,"
               "  :warehous_cntct_id, :warehous_fob, :warehous_active,"
               "  :warehous_bol_prefix, :warehous_bol_number, :warehous_shipping,"
               "  :warehous_counttag_prefix, :warehous_counttag_number, :warehous_useslips,"
               "  :warehous_aislesize, :warehous_aislealpha,"
               "  :warehous_racksize, :warehous_rackalpha,"
               "  :warehous_binsize, :warehous_binalpha,"
               "  :warehous_locationsize, :warehous_locationalpha,"
               "  :warehous_enforcearbl, :warehous_usezones, "
               "  :warehous_default_accnt_id, :warehous_shipping_commission, "
               "  :warehous_addr_id, :warehous_taxzone_id, :warehous_transit,"
               "  :warehous_shipform_id, :warehous_shipvia_id,"
               "  :warehous_shipcomments, :warehous_costcat_id, :warehous_sitetype_id,"
               "  :warehous_sequence );" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE whsinfo "
               "SET warehous_code=:warehous_code,"
               "    warehous_descrip=:warehous_descrip,"
               "    warehous_cntct_id=:warehous_cntct_id, "
               "    warehous_fob=:warehous_fob,"
               "    warehous_active=:warehous_active,"
               "    warehous_bol_prefix=:warehous_bol_prefix,"
               "    warehous_bol_number=:warehous_bol_number,"
               "    warehous_shipping=:warehous_shipping,"
               "    warehous_counttag_prefix=:warehous_counttag_prefix,"
               "    warehous_counttag_number=:warehous_counttag_number,"
               "    warehous_useslips=:warehous_useslips,"
               "    warehous_aislesize=:warehous_aislesize,"
               "    warehous_aislealpha=:warehous_aislealpha,"
               "    warehous_racksize=:warehous_racksize,"
               "    warehous_rackalpha=:warehous_rackalpha,"
               "    warehous_binsize=:warehous_binsize,"
               "    warehous_binalpha=:warehous_binalpha,"
               "    warehous_locationsize=:warehous_locationsize,"
               "    warehous_locationalpha=:warehous_locationalpha,"
               "    warehous_enforcearbl=:warehous_enforcearbl,"
               "    warehous_usezones=:warehous_usezones,"
               "    warehous_default_accnt_id=:warehous_default_accnt_id, "
               "    warehous_shipping_commission=:warehous_shipping_commission,"
               "    warehous_addr_id=:warehous_addr_id,"
               "    warehous_taxzone_id=:warehous_taxzone_id,"
               "    warehous_transit=:warehous_transit,"
               "    warehous_shipform_id=:warehous_shipform_id,"
               "    warehous_shipvia_id=:warehous_shipvia_id,"
               "    warehous_shipcomments=:warehous_shipcomments,"
               "    warehous_costcat_id=:warehous_costcat_id, "
               "    warehous_sitetype_id=:warehous_sitetype_id,"
               "    warehous_sequence=:warehous_sequence "
               "WHERE (warehous_id=:warehous_id);" );
  
  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":warehous_code", _code->text().trimmed().toUpper());
  q.bindValue(":warehous_descrip", _description->text());
  if (_contact->id() > 0)
    q.bindValue(":warehous_cntct_id", _contact->id());	// else NULL
  if (_address->id() > 0)
    q.bindValue(":warehous_addr_id", _address->id());	// else NULL

  q.bindValue(":warehous_active", QVariant(_active->isChecked()));
  q.bindValue(":warehous_default_accnt_id", _account->id());
  if(_sitetype->isValid())
    q.bindValue(":warehous_sitetype_id", _sitetype->id());
  q.bindValue(":warehous_sequence",	_sequence->value());

  if (_standard->isChecked())
  {
    q.bindValue(":warehous_fob",	     _defaultFOB->text());
    q.bindValue(":warehous_bol_prefix",	     _bolPrefix->text());
    q.bindValue(":warehous_bol_number",	     _bolNumber->text().toInt());
    q.bindValue(":warehous_counttag_prefix", _countTagPrefix->text());
    q.bindValue(":warehous_counttag_number", _countTagNumber->text().toInt());
    q.bindValue(":warehous_shipping",	QVariant(_shipping->isChecked()));
    q.bindValue(":warehous_useslips",	QVariant(_useSlips->isChecked()));
    q.bindValue(":warehous_enforcearbl",QVariant(_arblGroup->isChecked()));
    q.bindValue(":warehous_aislesize",	_aisleSize->value());
    q.bindValue(":warehous_aislealpha",	QVariant(_aisleAlpha->isChecked()));
    q.bindValue(":warehous_racksize",	_rackSize->value());
    q.bindValue(":warehous_rackalpha",	QVariant(_rackAlpha->isChecked()));
    q.bindValue(":warehous_binsize",	_binSize->value());
    q.bindValue(":warehous_binalpha",	QVariant(_binAlpha->isChecked()));
    q.bindValue(":warehous_locationsize",  _locationSize->value());
    q.bindValue(":warehous_locationalpha", QVariant(_locationAlpha->isChecked()));
    q.bindValue(":warehous_usezones",	   QVariant(_useZones->isChecked()));
    q.bindValue(":warehous_shipping_commission", (_shipcomm->toDouble() / 100));
    if(_taxzone->isValid())
      q.bindValue(":warehous_taxzone_id",	_taxzone->id());
  }

  q.bindValue(":warehous_transit",	QVariant(_transit->isChecked()));
  if (_transit->isChecked())
  {
    if (_shipform->isValid())
      q.bindValue(":warehous_shipform_id",	_shipform->id());
    if (_shipvia->isValid())
      q.bindValue(":warehous_shipvia_id",	_shipvia->id());
    q.bindValue(":warehous_shipcomments",	_shipcomments->toPlainText());
    if (_costcat->isValid())
      q.bindValue(":warehous_costcat_id",	_costcat->id());
  }

  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec("COMMIT;");

  omfgThis->sWarehousesUpdated();
  done(_warehousid);
}

void warehouse::populate()
{
  q.prepare( "SELECT *, formatScrap(warehous_shipping_commission) AS f_commission "
             "FROM whsinfo "
             "WHERE (warehous_id=:warehous_id);" );
  q.bindValue(":warehous_id", _warehousid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("warehous_code"));
    _sitetype->setId(q.value("warehous_sitetype_id").toInt());
    _active->setChecked(q.value("warehous_active").toBool());
    _description->setText(q.value("warehous_descrip"));
    _contact->setId(q.value("warehous_cntct_id").toInt());
    _address->setId(q.value("warehous_addr_id").toInt());
    _defaultFOB->setText(q.value("warehous_fob"));
    _bolPrefix->setText(q.value("warehous_bol_prefix"));
    _bolNumber->setText(q.value("warehous_bol_number"));
    _shipping->setChecked(q.value("warehous_shipping").toBool());
    _countTagPrefix->setText(q.value("warehous_counttag_prefix"));
    _countTagNumber->setText(q.value("warehous_counttag_number"));
    _useSlips->setChecked(q.value("warehous_useslips").toBool());
    _arblGroup->setChecked(q.value("warehous_enforceARBL").toBool());
    _aisleSize->setValue(q.value("warehous_aislesize").toInt());
    _aisleAlpha->setChecked(q.value("warehous_aislealpha").toBool());
    _rackSize->setValue(q.value("warehous_racksize").toInt());
    _rackAlpha->setChecked(q.value("warehous_rackalpha").toBool());
    _binSize->setValue(q.value("warehous_binsize").toInt());
    _binAlpha->setChecked(q.value("warehous_binalpha").toBool());
    _locationSize->setValue(q.value("warehous_locationsize").toInt());
    _locationAlpha->setChecked(q.value("warehous_locationalpha").toBool());
    _useZones->setChecked(q.value("warehous_usezones").toBool());
    _account->setId(q.value("warehous_default_accnt_id").toInt());
    _shipcomm->setText(q.value("f_commission").toString());
    _taxzone->setId(q.value("warehous_taxzone_id").toInt());
    _transit->setChecked(q.value("warehous_transit").toBool());
    _shipform->setId(q.value("warehous_shipform_id").toInt());
    _shipvia->setId(q.value("warehous_shipvia_id").toInt());
    _shipcomments->setText(q.value("warehous_shipcomments").toString());
    _costcat->setId(q.value("warehous_costcat_id").toInt());
    _sequence->setValue(q.value("warehous_sequence").toInt());

    sFillList();
    _comments->setId(_warehousid);
  }
}

void warehouse::sCheck()
{
  _code->setText(_code->text().trimmed().toUpper());

  if (_mode == cNew)
  {
    q.prepare( "SELECT warehous_id "
               "FROM warehous "
               "WHERE (UPPER(warehous_code)=UPPER(:warehous_code));" );
    q.bindValue(":warehous_code", _code->text());
    q.exec();
    if (q.first())
    {
      _warehousid = q.value("warehous_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
    else
    {
      if (_countTagPrefix->text().trimmed().length() == 0)
        _countTagPrefix->setText(_code->text());

      if (_bolPrefix->text().trimmed().length() == 0)
        _bolPrefix->setText(_code->text());
    }
  }
}

void warehouse::sNewZone()
{
  ParameterList params;
  params.append("warehous_id", _warehousid);
  params.append("mode", "new");

  warehouseZone newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void warehouse::sEditZone()
{
  ParameterList params;
  params.append("whsezone_id", _whsezone->id());
  params.append("mode", "edit");

  warehouseZone newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void warehouse::sDeleteZone()
{
  q.prepare( "SELECT location_id "
             "FROM location "
             "WHERE (location_whsezone_id=:whsezone_id);" );
  q.bindValue(":whsezone_id", _whsezone->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::warning( this, tr("Cannot Delete Site Zone"),
                          tr( "<p>The selected Site Zone cannot be "
			     "deleted as one or more Site Locations have "
			     "been assigned to it. You must delete or reassign "
			     "these Site Location before you may delete "
			     "the selected Site Zone." ) );
    return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare( "DELETE FROM whsezone "
             "WHERE (whsezone_id=:whsezone_id);" );
  q.bindValue(":whsezone_id", _whsezone->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void warehouse::sFillList()
{
  q.prepare( "SELECT whsezone_id, whsezone_name, whsezone_descrip "
             "FROM whsezone "
             "WHERE (whsezone_warehous_id=:warehous_id) "
             "ORDER BY whsezone_name;" );
  q.bindValue(":warehous_id", _warehousid);
  q.exec();
  _whsezone->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void warehouse::sHandleWhsType()
{
  if (_standard->isChecked())
  {
    _whsTypeStack->setCurrentIndex(0);
    _locationsTab->setEnabled(true);
  }
  else if (_transit->isChecked())
  {
    _whsTypeStack->setCurrentIndex(1);
    _locationsTab->setEnabled(false);
  }
}
