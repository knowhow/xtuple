/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "location.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include "itemcluster.h"

location::location(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_location, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sHandleWarehouse(int)));

  _locitem->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft, true, "item_number" );
  _locitem->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "item_descrip" );

  _warehouse->setAllowNull(_metrics->boolean("MultiWhs"));
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
    sHandleWarehouse(_warehouse->id());
  }
  else
    _warehouse->setNull();

}

location::~location()
{
  // no need to delete child widgets, Qt does it all for us
}

void location::languageChange()
{
  retranslateUi(this);
}

enum SetResponse location::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("location_id", &valid);
  if (valid)
  {
    _locationid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('location_location_id_seq') AS location_id;");
      if (q.first())
        _locationid = q.value("location_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );

        return UndefinedError;
      }

      _warehouse->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _warehouse->setEnabled(FALSE);

      _location->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _warehouse->setEnabled(FALSE);
      _whsezone->setEnabled(FALSE);
      _aisle->setEnabled(FALSE);
      _rack->setEnabled(FALSE);
      _bin->setEnabled(FALSE);
      _location->setEnabled(FALSE);
      _netable->setEnabled(FALSE);
      _restricted->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _locitem->setEnabled(FALSE);
      _save->hide();
      _new->hide();
      _delete->hide();
      _close->setText(tr("&Close"));

      _close->setFocus();
    }
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _warehouse->setId(param.toInt());
    _warehouse->setEnabled(FALSE);
  }

  return NoError;
}

void location::sCheck()
{
  _location->setText(_location->text().trimmed());
  if ( (_mode == cNew) &&
       (_warehouse->isEnabled()) &&
       (_location->text().length()) )
  {
    q.prepare( "SELECT location_id "
               "FROM location "
               "WHERE ( (location_warehous_id=:location_warehous_id)"
               " AND (UPPER(location_name)=UPPER(:location_name)) );" );
    q.bindValue(":warehous_id", _warehouse->id());
    q.bindValue(":location_name", _location->text());
    q.exec();
    if (q.first())
    {
      _locationid = q.value("location_id").toInt();
      _mode = cEdit;
      _warehouse->setEnabled(FALSE);
      _location->setEnabled(FALSE);

      populate();
    }
  }
}

void location::sSave()
{
  if (_warehouse->id() == -1)
  {
    QMessageBox::critical( this, tr("Select a Site"),
                           tr( "<p>You must select a Site for this "
			      "Location before creating it." ) );
    _warehouse->setFocus();
    return;
  }

  if ( (_location->text().trimmed().length() == 0) &&
       (_aisle->text().trimmed().length() == 0) &&
       (_rack->text().trimmed().length() == 0) &&
       (_bin->text().trimmed().length() == 0) )
  {
    QMessageBox::critical( this, tr("Enter Location Name"),
                           tr("<p>You must enter a unique name to identify "
			      "this Location.") );
    _location->setFocus();
    return;
  }

  q.prepare("SELECT location_id"
            "  FROM location"
            " WHERE((location_id != :location_id)"
            "   AND (location_warehous_id=:location_warehous_id)"
            "   AND (location_aisle=:location_aisle)"
            "   AND (location_rack=:location_rack)"
            "   AND (location_bin=:location_bin)"
            "   AND (location_name=:location_name))");
  q.bindValue(":location_id", _locationid);
  q.bindValue(":location_warehous_id", _warehouse->id());
  q.bindValue(":location_aisle", _aisle->text());
  q.bindValue(":location_rack", _rack->text());
  q.bindValue(":location_bin", _bin->text());
  q.bindValue(":location_name", _location->text());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical( this, tr("Duplicate Location Name"),
                           tr("<p>You must enter a unique name to identify "
			      "this Location in the specified Site.") );
    _location->setFocus();
    return;
  }
  

  if (_mode == cNew)
    q.prepare( "INSERT INTO location "
               "( location_id, location_warehous_id,"
               "  location_aisle, location_rack, location_bin,  location_name,"
               "  location_whsezone_id, location_descrip,"
               "  location_netable, location_restrict ) "
               "VALUES "
               "( :location_id, :location_warehous_id,"
               "  :location_aisle, :location_rack, :location_bin, :location_name,"
               "  :location_whsezone_id, :location_descrip,"
               "  :location_netable, :location_restrict );" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE location "
               "SET location_warehous_id=:location_warehous_id,"
               "    location_aisle=:location_aisle, location_rack=:location_rack,"
               "    location_bin=:location_bin, location_name=:location_name,"
               "    location_whsezone_id=:location_whsezone_id, location_descrip=:location_descrip,"
               "    location_netable=:location_netable, location_restrict=:location_restrict "
               "WHERE (location_id=:location_id);" );

  q.bindValue(":location_id", _locationid);
  q.bindValue(":location_warehous_id", _warehouse->id());
  q.bindValue(":location_aisle", _aisle->text());
  q.bindValue(":location_rack", _rack->text());
  q.bindValue(":location_bin", _bin->text());
  q.bindValue(":location_name", _location->text());
  q.bindValue(":location_whsezone_id", _whsezone->id());
  q.bindValue(":location_descrip", _description->toPlainText().trimmed());
  q.bindValue(":location_netable", QVariant(_netable->isChecked()));
  q.bindValue(":location_restrict", QVariant(_restricted->isChecked()));
  q.exec();

  done(_locationid);
}

void location::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM locitem "
                "WHERE (locitem_location_id=:location_id);" );
    q.bindValue(":location_id", _locationid);
    q.exec();
  }

  reject();
}

void location::sHandleWarehouse(int pWarehousid)
{
  q.prepare( "SELECT warehous_enforcearbl, warehous_usezones,"
             "       warehous_aislesize, warehous_aislealpha,"
             "       warehous_racksize, warehous_rackalpha,"
             "       warehous_binsize, warehous_binalpha,"
             "       warehous_locationsize, warehous_locationalpha "
             "FROM warehous "
             "WHERE (warehous_id=:warehous_id);" );
  q.bindValue(":warehous_id", pWarehousid);
  q.exec();
  if (q.first())
  {
    if (q.value("warehous_enforcearbl").toBool())
    {
      QString regex;

      if (q.value("warehous_aislesize").toInt() > 0)
      {
        _aisleLit->show();
        _aisle->show();

        if (q.value("warehous_aislealpha").toBool())
          regex = QString("\\w{1%1}").arg(q.value("warehous_aislesize").toInt());
        else
          regex = QString("\\d{1%1}").arg(q.value("warehous_aislesize").toInt());

        _aisle->setValidator(new QRegExpValidator(QRegExp(regex), this));
        _aisle->setMaxLength(q.value("warehous_aislesize").toInt());
      }
      else
      {
        _aisleLit->hide();
        _aisle->hide();
      }

      if (q.value("warehous_racksize").toInt() > 0)
      {
        _rackLit->show();
        _rack->show();

        if (q.value("warehous_rackalpha").toBool())
          regex = QString("\\w{1%1}").arg(q.value("warehous_racksize").toInt());
        else
          regex = QString("\\d{1%1}").arg(q.value("warehous_racksize").toInt());

        _rack->setValidator(new QRegExpValidator(QRegExp(regex), this));
        _rack->setMaxLength(q.value("warehous_racksize").toInt());
      }
      else
      {
        _rackLit->hide();
        _rack->hide();
      }

      if (q.value("warehous_binsize").toInt() > 0)
      {
        _binLit->show();
        _bin->show();

        if (q.value("warehous_binalpha").toBool())
          regex = QString("\\w{1%1}").arg(q.value("warehous_binsize").toInt());
        else
          regex = QString("\\d{1%1}").arg(q.value("warehous_binsize").toInt());

        _bin->setValidator(new QRegExpValidator(QRegExp(regex), this));
        _bin->setMaxLength(q.value("warehous_binsize").toInt());
      }
      else
      {
        _binLit->hide();
        _bin->hide();
      }

      if (q.value("warehous_locationsize").toInt() > 0)
      {
        _locationLit->show();
        _location->show();

        if (q.value("warehous_locationalpha").toBool())
          regex = QString("\\w{1%1}").arg(q.value("warehous_locationsize").toInt());
        else
          regex = QString("\\d{1%1}").arg(q.value("warehous_locationsize").toInt());

        _location->setValidator(new QRegExpValidator(QRegExp(regex), this));
        _location->setMaxLength(q.value("warehous_locationsize").toInt());
      }
      else
      {
        _locationLit->hide();
        _location->hide();
      }
    }
    else
    {
      _aisleLit->hide();
      _aisle->hide();
      _rackLit->hide();
      _rack->hide();
      _binLit->hide();
      _bin->hide();
      _locationLit->show();
      _location->show();
    }

    _whsezone->setAllowNull(!q.value("warehous_usezones").toBool());
    q.prepare( "SELECT whsezone_id, whsezone_name "
               "FROM whsezone "
               "WHERE (whsezone_warehous_id=:warehous_id) "
               "ORDER BY whsezone_name;" );
    q.bindValue(":warehous_id", pWarehousid);
    q.exec();
    _whsezone->populate(q);
    if (!_whsezone->count())
      _whsezone->setEnabled(FALSE);
  }
}

void location::sNew()
{
  ParameterList params;
  itemList* newdlg = new itemList(this);
  newdlg->set(params);

  int itemid = newdlg->exec();
  if (itemid != -1)
  {
//  Make sure that a locitem does not already exist for this
//  location and the selected item
    q.prepare( "SELECT locitem_id "
               "FROM locitem "
               "WHERE ( (locitem_location_id=:location_id)"
               " AND (locitem_item_id=:item_id) );" );
    q.bindValue(":location_id", _locationid);
    q.bindValue(":item_id", itemid);
    q.exec();
    if (q.first())
//  Tell the user that a locitem already exists for this location/item
      QMessageBox::information( this, tr("Location/Item Exists"),
                                tr( "<p>An Item record already exists in the "
				    "Location. You may not add another record "
				    "for the same Item") );
    else
    {
      q.prepare( "INSERT INTO locitem "
                 "(locitem_location_id, locitem_item_id) "
                 "VALUES "
                 "(:location_id, :item_id);" );
      q.bindValue(":location_id", _locationid);
      q.bindValue(":item_id", itemid);
      q.exec();

      sFillList();
    }
  }
}

void location::sDelete()
{
//  ToDo - add a check to make sure that am itemloc does not exist for the selected locitem pair?
  q.prepare( "DELETE FROM locitem "
             "WHERE (locitem_id=:locitem_id);" );
  q.bindValue(":locitem_id", _locitem->id());
  q.exec();

  sFillList();
}

void location::sFillList()
{
  q.prepare( "SELECT locitem_id, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2) AS item_descrip  "
             "FROM locitem, item "
             "WHERE ( (locitem_item_id=item_id)"
             " AND (locitem_location_id=:location_id) );" );
  q.bindValue(":location_id", _locationid);
  q.exec();
  _locitem->populate(q);
}

void location::populate()
{
  q.prepare( "SELECT * "
             "FROM location "
             "WHERE (location_id=:location_id);" );
  q.bindValue(":location_id", _locationid);
  q.exec();
  if (q.first())
  {
    _aisle->setText(q.value("location_aisle").toString());
    _rack->setText(q.value("location_rack").toString());
    _bin->setText(q.value("location_bin").toString());
    _location->setText(q.value("location_name").toString());
    _description->setText(q.value("location_descrip").toString());
    _netable->setChecked(q.value("location_netable").toBool());
    _restricted->setChecked(q.value("location_restrict").toBool());

    int whsezoneid = q.value("location_whsezone_id").toInt();
    _warehouse->setId(q.value("location_warehous_id").toInt());
    sHandleWarehouse(_warehouse->id());

    _whsezone->setId(whsezoneid);

    sFillList();
  }
}
