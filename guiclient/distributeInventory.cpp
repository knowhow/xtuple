/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "distributeInventory.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include "assignLotSerial.h"
#include "distributeToLocation.h"
#include "inputManager.h"

#define cIncludeLotSerial   0x01
#define cNoIncludeLotSerial 0x02

distributeInventory::distributeInventory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_bcDistribute,    SIGNAL(clicked()), this, SLOT(sBcDistribute()));
  connect(_default,         SIGNAL(clicked()), this, SLOT(sDefault()));
  connect(_defaultAndPost,  SIGNAL(clicked()), this, SLOT(sDefaultAndPost()));
  connect(_distribute,      SIGNAL(clicked()), this, SLOT(sSelectLocation()));
  connect(_itemloc, SIGNAL(itemSelected(int)), this, SLOT(sSelectLocation()));
  connect(_post,            SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_taggedOnly,  SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_bc,   SIGNAL(textChanged(QString)), this, SLOT(sBcChanged(QString)));
  connect(_qtyOnly,     SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  omfgThis->inputManager()->notify(cBCLotSerialNumber, this, this, SLOT(sCatchLotSerialNumber(QString)));

  _item->setReadOnly(TRUE);
  _qtyToDistribute->setPrecision(omfgThis->qtyVal());
  _qtyTagged->setPrecision(omfgThis->qtyVal());
  _qtyRemaining->setPrecision(omfgThis->qtyVal());
  
  _itemloc->addColumn(tr("Location"),     _itemColumn, Qt::AlignLeft,  true, "locationname");
  _itemloc->addColumn(tr("Default"),      _ynColumn,   Qt::AlignLeft,  true, "defaultlocation");
  _itemloc->addColumn(tr("Netable"),      _ynColumn,   Qt::AlignCenter,true, "location_netable");
  _itemloc->addColumn(tr("Lot/Serial #"), -1,          Qt::AlignLeft,  true, "lotserial");
  _itemloc->addColumn(tr("Expiration"),   _dateColumn, Qt::AlignCenter,true, "f_expiration");
  _itemloc->addColumn(tr("Qty. Before"),  _qtyColumn,  Qt::AlignRight, true, "qty");
  _itemloc->addColumn(tr("Tagged Qty."),  _qtyColumn,  Qt::AlignRight, true, "qtytagged");
  _itemloc->addColumn(tr("Qty. After"),   _qtyColumn,  Qt::AlignRight, true, "balance");
  
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
  
  //If not lot serial control, hide info
  if (!_metrics->boolean("LotSerialControl"))
  {
    _lotSerial->hide();
    _lotSerialLit->hide();
    _bcLit->hide();
    _bc->hide();
    _bcQtyLit->hide();
    _bcQty->hide();
    _bcDistribute->hide();
  }

  _itemlocdistid = -1;
  
  _locationDefaultLit->hide();
  _locations->hide();
}

distributeInventory::~distributeInventory()
{
  // no need to delete child widgets, Qt does it all for us
}

void distributeInventory::languageChange()
{
  retranslateUi(this);
}
int distributeInventory::SeriesAdjust(int pItemlocSeries, QWidget *pParent, const QString & pPresetLotnum, const QDate & pPresetLotexp, const QDate & pPresetLotwarr)
{
  int result;
  QList<int>  ildsList; // Item Loc Dist Series 
  QList<int>  ildList; // Item Loc Dist List

  if (pItemlocSeries != 0)
  {
    XSqlQuery itemloc;
    itemloc.prepare( "SELECT itemlocdist_id, itemlocdist_reqlotserial," 
                     "       itemlocdist_distlotserial, itemlocdist_qty,"
                     "       itemsite_loccntrl, itemsite_controlmethod,"
                     "       itemsite_perishable, itemsite_warrpurc, "
                     "       COALESCE(itemsite_lsseq_id,-1) AS itemsite_lsseq_id, "
                     "       COALESCE(itemlocdist_source_id,-1) AS itemlocdist_source_id "
                     "FROM itemlocdist, itemsite "
                     "WHERE ( (itemlocdist_itemsite_id=itemsite_id)"
                     " AND (itemlocdist_series=:itemlocdist_series) ) "
                     "ORDER BY itemlocdist_id;" );
    itemloc.bindValue(":itemlocdist_series", pItemlocSeries);
    itemloc.exec();
    while (itemloc.next())
    {
      if (itemloc.value("itemlocdist_reqlotserial").toBool())
      {
        int itemlocSeries = -1;
        XSqlQuery query;
        // Check to see if this is a lot controlled item and if we have
        // a predefined lot#/expdate to use. If so assign that information
        // with itemlocdist_qty and move on. otherwise do the normal dialog
        // to ask the user for that information.
        if(itemloc.value("itemsite_controlmethod").toString() == "L" && !pPresetLotnum.isEmpty())
        {
          query.exec("SELECT nextval('itemloc_series_seq') AS _itemloc_series;");
          if(query.first())
          {
            itemlocSeries = query.value("_itemloc_series").toInt();
            query.prepare( "SELECT createlotserial(itemlocdist_itemsite_id,:lotserial,:itemlocdist_series,'I',NULL,itemlocdist_id,:qty,:expiration,:warranty) "
                           "FROM itemlocdist "
                           "WHERE (itemlocdist_id=:itemlocdist_id);"
                           
                           "UPDATE itemlocdist "
                           "SET itemlocdist_source_type='O' "
                           "WHERE (itemlocdist_series=:itemlocdist_series);"

                           "DELETE FROM itemlocdist "
                           "WHERE (itemlocdist_id=:itemlocdist_id);" );
            query.bindValue(":lotserial", pPresetLotnum);
            query.bindValue(":qty", itemloc.value("itemlocdist_qty"));
            query.bindValue(":itemlocdist_series", itemlocSeries);
            query.bindValue(":itemlocdist_id", itemloc.value("itemlocdist_id"));
            if(itemloc.value("itemsite_perishable").toBool())
              query.bindValue(":expiration", pPresetLotexp);
            else
              query.bindValue(":expiration", omfgThis->endOfTime());
            if(itemloc.value("itemsite_warrpurc").toBool())
              query.bindValue(":warranty", pPresetLotwarr);
            query.exec();
          }
        }

        if(itemlocSeries == -1)
        {
          // Check to see if Lot/Serial distributions should be created using
          // "from" side of transaction.  Transactions are related by itemlocdist_source_id.
          // InterWarehouseTransfer uses this technique.
          if (itemloc.value("itemlocdist_source_id").toInt() > -1)
          {
            XSqlQuery fromlots;
            fromlots.exec("SELECT nextval('itemloc_series_seq') AS _itemloc_series;");
            if(fromlots.first())
            {
              itemlocSeries = fromlots.value("_itemloc_series").toInt();
              fromlots.prepare("SELECT  createlotserial(s.itemlocdist_itemsite_id, ls_number, "
                               "        :itemlocdist_series, 'I', NULL, :itemlocdist_id, (d.itemlocdist_qty * -1.0), "
                               "        itemloc_expiration, itemloc_warrpurc) "
                               "FROM itemlocdist s JOIN itemlocdist o ON (o.itemlocdist_id=s.itemlocdist_source_id) "
                               "                   JOIN itemlocdist d ON (d.itemlocdist_itemlocdist_id=o.itemlocdist_id) "
                               "                   JOIN itemloc ON (itemloc_id=d.itemlocdist_source_id) "
                               "                   JOIN ls ON (ls_id=itemloc_ls_id) "
                               "WHERE (s.itemlocdist_id=:itemlocdist_id);"

                               "UPDATE itemlocdist "
                               "SET itemlocdist_source_type='O' "
                               "WHERE (itemlocdist_series=:itemlocdist_series);"

                               "DELETE FROM itemlocdist "
                               "WHERE (itemlocdist_id=:itemlocdist_id);" );
              fromlots.bindValue(":itemlocdist_series", itemlocSeries);
              fromlots.bindValue(":itemlocdist_id", itemloc.value("itemlocdist_id"));
              fromlots.exec();
              if (fromlots.lastError().type() != QSqlError::NoError)
              {
                systemError(0, fromlots.lastError().databaseText(), __FILE__, __LINE__);
                return XDialog::Rejected;
              }
            }
          }
        }

        if(itemlocSeries == -1)
        { 
          ParameterList params;
          params.append("itemlocdist_id", itemloc.value("itemlocdist_id").toInt());

          // Auto assign lot/serial if applicable
          if (itemloc.value("itemsite_lsseq_id").toInt() != -1 &&
              !itemloc.value("itemsite_perishable").toBool() &&
              !itemloc.value("itemsite_warrpurc").toBool()) {
            XSqlQuery autocreatels;
            autocreatels.prepare("SELECT autocreatels(:itemlocdist_id) AS itemlocseries;");
            autocreatels.bindValue(":itemlocdist_id", itemloc.value("itemlocdist_id").toInt());
            autocreatels.exec();
            if (autocreatels.first())
              params.append("itemlocseries", autocreatels.value("itemlocseries").toInt());
            else if (autocreatels.lastError().type() != QSqlError::NoError)
            {
              systemError(0, autocreatels.lastError().databaseText(), __FILE__, __LINE__);
              return XDialog::Rejected;
            }
          }

          assignLotSerial newdlg(pParent, "", TRUE);
          newdlg.set(params);
          itemlocSeries = newdlg.exec();
          if (itemlocSeries == XDialog::Rejected)
            return XDialog::Rejected;
        }
        
        if (itemloc.value("itemsite_loccntrl").toBool())
        {
          query.prepare( "SELECT itemlocdist_id " 
                         "FROM itemlocdist "
                         "WHERE (itemlocdist_series=:itemlocdist_series) "
                         "ORDER BY itemlocdist_id;" );
          query.bindValue(":itemlocdist_series", itemlocSeries);
          query.exec();
          while (query.next())
          {
            ParameterList params;
            params.append("itemlocdist_id", query.value("itemlocdist_id").toInt());
            distributeInventory newdlg(pParent, "", TRUE);
            newdlg.set(params);
            result = newdlg.exec();
            if (result == XDialog::Rejected)
              return XDialog::Rejected;
            else
              ildList.append(result);
          }
        }
        else
        {
          query.prepare( "UPDATE itemlocdist "
                         "SET itemlocdist_source_type='L', itemlocdist_source_id=-1 "
                         "WHERE (itemlocdist_series=:itemlocdist_series); ");
          query.bindValue(":itemlocdist_series", itemlocSeries);
          query.exec();

          // Append id to list and process at the end
          ildsList.append(itemlocSeries);
        }
      }
      else
      {
        ParameterList params;
        params.append("itemlocdist_id", itemloc.value("itemlocdist_id").toInt());

        if (itemloc.value("itemlocdist_distlotserial").toBool())
          params.append("includeLotSerialDetail");

        distributeInventory newdlg(pParent, "", TRUE);
        newdlg.set(params);
        result = newdlg.exec();
        if (result == XDialog::Rejected)
          return XDialog::Rejected;
        else
          ildList.append(result);
      }
    }

    XSqlQuery post;
    
    // Process Lot/Serial distributions
    for (int i = 0; i < ildsList.size(); ++i) {
      post.prepare( "SELECT distributeItemlocSeries(:itemlocdist_series) AS result;");
      post.bindValue(":itemlocdist_series", ildsList.at(i));
      post.exec();
      if (post.lastError().type() != QSqlError::NoError)
      {
        systemError(0, post.lastError().databaseText(), __FILE__, __LINE__);
        return XDialog::Rejected;
      }
    }
    
    // Process location distributions
    for (int i = 0; i < ildList.size(); ++i) {
      post.prepare("SELECT distributeToLocations(:itemlocdist_id) AS result;");
      post.bindValue(":itemlocdist_id", ildList.at(i));
      post.exec();
      if (post.lastError().type() != QSqlError::NoError)
      {
        systemError(0, post.lastError().databaseText(), __FILE__, __LINE__);
        return XDialog::Rejected;
      }
    }
    
    //Post inventory history for any remaining non-distributed transactions and trial balance
    post.prepare("SELECT postItemlocseries(:itemlocseries) AS result;");
    post.bindValue(":itemlocseries",  pItemlocSeries);
    post.exec();
    if (post.first())
    {
      if (!post.value("result").toBool())
            QMessageBox::warning( 0, tr("Inventory Distribution"), 
        tr("There was an error posting the transaction.  Contact your administrator") );
    }
    else if (post.lastError().type() != QSqlError::NoError)
    {
      systemError(0, post.lastError().databaseText(), __FILE__, __LINE__);
      return XDialog::Rejected;
    }
  }
  
  return XDialog::Accepted;
}

enum SetResponse distributeInventory::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("includeLotSerialDetail", &valid);
  if (valid && _metrics->boolean("LotSerialControl"))
  {
    _mode = cIncludeLotSerial;
  }
  else
    _mode = cNoIncludeLotSerial;

  param = pParams.value("itemlocdist_id", &valid);
  if (valid)
  {
    _itemlocdistid = param.toInt();
    populate();
  }

  _bc->setFocus();

  return NoError;
}

void distributeInventory::closeEvent(QCloseEvent *pEvent)
{
  pEvent->accept();
}

void distributeInventory::populate()
{
  q.prepare("SELECT itemsite_controlmethod "
	    "FROM itemsite, itemlocdist "
	    "WHERE ((itemlocdist_itemsite_id=itemsite_id)"
	    "  AND  (itemlocdist_id=:itemlocdist_id));");

  q.bindValue(":itemlocdist_id", _itemlocdistid);
  q.exec();
  if (q.first())
  {
    _controlMethod = q.value("itemsite_controlmethod").toString();
    _bc->setEnabled(_controlMethod == "L" || _controlMethod == "S");
    _bcQty->setEnabled(_controlMethod == "L");
    _bcDistribute->setEnabled(_controlMethod == "L" || _controlMethod == "S");
    if (_controlMethod == "S")
      _bcQty->setText("1");
    else
      _bcQty->clear();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

//  Auto distribute location reservations
  if ( (_metrics->boolean("EnableSOReservationsByLocation")) && (_mode == cIncludeLotSerial) )
  {
    q.prepare("SELECT distributeToReservedItemLoc(:itemlocdist_id) AS result;");
    q.bindValue(":itemlocdist_id", _itemlocdistid);
    q.exec();
  }

  sFillList();
  if (_metrics->boolean("SetDefaultLocations"))
    sPopulateDefaultSelector();
  
}

void distributeInventory::sSelectLocation()
{
  ParameterList params;
  params.append("source_itemlocdist_id", _itemlocdistid);

  if (_itemloc->altId() == cLocation)
    params.append("location_id", _itemloc->id());
  else if (_itemloc->altId() == cItemloc)
    params.append("itemlocdist_id", _itemloc->id());

  distributeToLocation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void distributeInventory::sPost()
{
  if (_qtyRemaining->toDouble() != 0.0)
  {
    QMessageBox::critical( this, tr("Cannot Perform Partial Distribution"),
                           tr( "<p>You must completely distribute the quantity "
			       "before selecting the 'Post' button." ) );
    return;
  }

  // Append id to the list and process at the end   
  done(_itemlocdistid);
}

bool distributeInventory::sDefault()
{
   bool distribOk = true;
   double qty = 0.0;
   double availToDistribute = 0.0;

   q.prepare("SELECT   itemlocdist_qty AS qty, "
             "         qtyLocation(location_id, NULL, NULL, NULL, itemsite_id, itemlocdist_order_type, itemlocdist_order_id, itemlocdist_id) AS availToDistribute "
             "   FROM   itemlocdist, location, itemsite "
             "  WHERE ( (itemlocdist_itemsite_id=itemsite_id)"
             "    AND (itemsite_loccntrl)"
             "    AND (itemsite_warehous_id=location_warehous_id)"
             "    AND (location_id=itemsite_location_id) "
             "    AND (itemlocdist_id=:itemlocdist_id) ) ");
   q.bindValue(":itemlocdist_id", _itemlocdistid);
   q.exec();
   if (q.first())
   {
       qty = q.value("qty").toDouble();
       availToDistribute = q.value("availToDistribute").toDouble();
   }

   if(qty < 0 && availToDistribute < qAbs(qty))
   {
       if (QMessageBox::question(this,  tr("Distribute to Default"),
                                tr("It appears you are trying to distribute more "
                                   "than is available to be distributed. "
                                   "<p>Are you sure you want to distribute this quantity?"),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            distribOk = true;
       else
            distribOk = false;
   }
   if(distribOk)
   {
      if(_mode == cIncludeLotSerial)
        q.prepare("SELECT distributeToDefaultItemLoc(:itemlocdist_id) AS result;");
      else
        q.prepare("SELECT distributeToDefault(:itemlocdist_id) AS result;");
      q.bindValue(":itemlocdist_id", _itemlocdistid);
      q.exec();
      sFillList();
      //prevent default from been changed after default distribute
      //stopping the operator from thinking the inventory has been posted
      //to the new default
      _locations->setEnabled(false);
      return true;
    }
    else
       return false;
}

void distributeInventory::sDefaultAndPost()
{
  if(sDefault())
    sPost();
}

void distributeInventory::sFillList()
{
  q.prepare( "SELECT itemsite_id, "
             "       COALESCE(itemsite_location_id,-1) AS itemsite_location_id,"
             "       formatlotserialnumber(itemlocdist_ls_id) AS lotserial,"
             "       (itemlocdist_order_type || ' ' || formatSoItemNumber(itemlocdist_order_id)) AS order,"
             "       (itemsite_controlmethod IN ('L', 'S')) AS lscontrol,"
             "       parent.itemlocdist_qty AS qtytodistribute,"
             "       ( ( SELECT COALESCE(SUM(child.itemlocdist_qty), 0)"
             "             FROM itemlocdist AS child"
             "            WHERE (child.itemlocdist_itemlocdist_id=parent.itemlocdist_id) ) ) AS qtytagged,"
             "       (parent.itemlocdist_qty - ( SELECT COALESCE(SUM(child.itemlocdist_qty), 0)"
             "                                     FROM itemlocdist AS child"
             "                                    WHERE (child.itemlocdist_itemlocdist_id=parent.itemlocdist_id) ) ) AS qtybalance "
             "FROM itemsite, itemlocdist AS parent "
             "WHERE ( (itemlocdist_itemsite_id=itemsite_id)"
             " AND (itemlocdist_id=:itemlocdist_id) );" );
  q.bindValue(":itemlocdist_id", _itemlocdistid);
  q.exec();
  if (q.first())
  {
    _item->setItemsiteid(q.value("itemsite_id").toInt());
    _lotSerial->setText(q.value("lotserial").toString());
    _order->setText(q.value("order").toString());
    _qtyToDistribute->setDouble(q.value("qtytodistribute").toDouble());
    _qtyTagged->setDouble(q.value("qtytagged").toDouble());
    _qtyRemaining->setDouble(q.value("qtybalance").toDouble());

    if ( (q.value("itemsite_location_id").toInt() != -1) &&
         ( (_mode == cNoIncludeLotSerial) || ( (_mode == cIncludeLotSerial) && (!q.value("lscontrol").toBool()) ) ) )
    {
      _default->setEnabled(TRUE);
      _defaultAndPost->setEnabled(TRUE);
    }
    else
    {
      _default->setEnabled(FALSE);
      _defaultAndPost->setEnabled(FALSE);
    }

    ParameterList params;

    if (_mode == cNoIncludeLotSerial)
      params.append("cNoIncludeLotSerial");
    else if (_mode == cIncludeLotSerial)
      params.append("cIncludeLotSerial");

    if (_taggedOnly->isChecked())
      params.append("showOnlyTagged");
      
    if (_qtyOnly->isChecked())
      params.append("showQtyOnly");

    params.append("locationType",   cLocation);
    params.append("itemlocType",    cItemloc);
    params.append("yes",            tr("Yes"));
    params.append("no",             tr("No"));
    params.append("na",             tr("N/A"));
    params.append("undefined",      tr("Undefined"));
    params.append("itemlocdist_id", _itemlocdistid);
    params.append("itemsite_id",    q.value("itemsite_id").toInt());

    MetaSQLQuery mql = mqlLoad("distributeInventory", "locations");
    q = mql.toQuery(params);

    _itemloc->populate(q, true);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void distributeInventory::sBcDistribute()
{
  if (_bc->text().isEmpty())
  {
    QMessageBox::warning(this, tr("No Bar Code scanned"),
			 tr("<p>Cannot search for Items by Bar Code without a "
			    "Bar Code."));
    _bc->setFocus();
    return;
  }

  q.prepare( "SELECT itemloc_id "
	     "FROM  itemlocdist, itemloc, itemsite, ls "
	     "WHERE ((itemlocdist_itemsite_id=itemloc_itemsite_id)"
	     "  AND  (itemloc_itemsite_id=itemsite_id)"
             "  AND  (itemsite_controlmethod IN ('L', 'S'))"
             "  AND  (itemsite_item_id=ls_item_id)"
             "  AND  (itemloc_ls_id=ls_id)"
             "  AND  (itemsite_warehous_id=:warehous_id)"
	     "  AND  (ls_number=:lotserial)"
	     "  AND  (itemlocdist_id=:itemlocdist_id));");

  q.bindValue(":itemlocdist_id", _itemlocdistid);
  q.bindValue(":lotserial",      _bc->text());
  q.bindValue(":warehous_id",    _warehouse->id());
  q.exec();

  if(!q.first())
  {
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    QMessageBox::warning(this, tr("No Match Found"),
			 tr("<p>No available lines match the specified Barcode.")); 
    _bc->clear();
    return;
  }

  ParameterList params;
  params.append("itemlocdist_id",        q.value("itemloc_id"));
  params.append("source_itemlocdist_id", _itemlocdistid);
  params.append("qty",                   _bcQty->text());
  params.append("distribute");

  distributeToLocation newdlg(this, "", TRUE);
  if (newdlg.set(params) != NoError)
    return;

  _bc->clear();
  if (_controlMethod == "S")
    _bcQty->setText("1");
  else
    _bcQty->clear();
  sFillList();
  _bc->setFocus();
}

void distributeInventory::sCatchLotSerialNumber(const QString plotserial)
{
  _bc->setText(plotserial);
  if (_controlMethod == "S")
    _bcDistribute->setFocus();
  else
    _bcQty->setFocus();
}

void distributeInventory::sBcChanged(const QString p)
{
  _post->setDefault(p.isEmpty());
  _bcDistribute->setDefault(! p.isEmpty());
}

void distributeInventory::sPopulateDefaultSelector()
{
   XSqlQuery query;
   query.prepare(" SELECT itemsite_id, itemsite_loccntrl, itemsite_location_id"
                 "  FROM itemsite, itemlocdist"
                 "  WHERE (itemsite_id=itemlocdist_itemsite_id)"
                 "    AND (itemlocdist_id=:itemlocdist_id)");
   query.bindValue(":itemlocdist_id", _itemlocdistid);
   query.exec();
   if(query.first())
   {
      _itemsite_id = query.value("itemsite_id").toInt();
      if(_itemsite_id > -1
         && query.value("itemsite_loccntrl").toBool()
         && query.value("itemsite_location_id").toInt() != -1)
        {
            _locationDefaultLit->show();
            _locations->show();
            XSqlQuery loclist;
            loclist.prepare( " SELECT location_id, formatLocationName(location_id) AS locationname "
                           " FROM location "
                           " WHERE ( (location_warehous_id=:warehous_id)"
                           "   AND (NOT location_restrict) ) "
                           " UNION SELECT location_id, formatLocationName(location_id) AS locationname "
                           "  FROM location, locitem, itemsite"
                           "  WHERE ( (location_warehous_id=:warehous_id)"
                           "    AND (location_restrict)"
                           "    AND (locitem_location_id=location_id)"
                           "    AND (locitem_item_id=itemsite_item_id)"
                           "    AND (itemsite_id=:itemsite_id))"
                           " ORDER BY locationname;" );
            loclist.bindValue(":warehous_id", _warehouse->id());
            loclist.bindValue(":itemsite_id", _itemsite_id);
            loclist.exec();
            _locations->populate(loclist);
            if (!loclist.first())
            {
                _locationDefaultLit->hide();
                _locations->hide();
            }
            else
            {
               //Allow default location update with correct privileges
               if (_privileges->check("MaintainItemSites"))
                   _locations->setEnabled(true);
               else
                   _locations->setEnabled(false);

               XSqlQuery dfltLocation;
               dfltLocation.prepare( " SELECT itemsite_location_id"
                                     " FROM itemsite"
                                     " WHERE (itemsite_id=:itemsite_id)");
              dfltLocation.bindValue(":itemsite_id", _itemsite_id);
              dfltLocation.exec();
              if (!dfltLocation.first())
                 _locations->setId(-1);
              else
                 _locations->setId(dfltLocation.value("itemsite_location_id").toInt());

              connect(_locations,   SIGNAL(newID(int)),    this, SLOT(sChangeDefaultLocation()));
            }
        }
        else
        {
          _locationDefaultLit->hide();
          _locations->hide();
        }
    }
    else
    {
       _locationDefaultLit->hide();
       _locations->hide();
    }
}

void distributeInventory::sChangeDefaultLocation()
{
   XSqlQuery query;
   query.prepare( " UPDATE itemsite"
                  " SET itemsite_location_id=:itemsite_location_id"
                  " WHERE (itemsite_id=:itemsite_id);" );
   query.bindValue(":itemsite_location_id", _locations->id());
   query.bindValue(":itemsite_id", _itemsite_id);
   query.exec();
   sFillList();
   if (query.lastError().type() != QSqlError::NoError)
      {
        systemError(this, query.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
}
