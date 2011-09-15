/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSingleLevelBOM.h"

#include <QVariant>

dspSingleLevelBOM::dspSingleLevelBOM(QWidget* parent, const char*, Qt::WFlags fl)
  : dspBOMBase(parent, "dspSingleLevelBOM", fl)
{
  setWindowTitle(tr("Single Level Bill of Materials"));
  setReportName("SingleLevelBOM");

  list()->addColumn(tr("#"),                   30,      Qt::AlignCenter,true, "bomitem_seqnumber" );
  list()->addColumn(tr("Item Number"),    _itemColumn,  Qt::AlignLeft,  true, "item_number"   );
  list()->addColumn(tr("Description"),         -1,      Qt::AlignLeft,  true, "itemdescription"   );
  list()->addColumn(tr("Issue UOM"),       _uomColumn,   Qt::AlignCenter,true, "issueuomname" );
  list()->addColumn(tr("Issue Fxd. Qty."),  _qtyColumn,  Qt::AlignRight, true, "bomitem_qtyfxd"  );
  list()->addColumn(tr("Issue Qty. Per"),  _qtyColumn,   Qt::AlignRight, true, "bomitem_qtyper"  );
  list()->addColumn(tr("Inv. UOM"),        _uomColumn,   Qt::AlignCenter,true, "invuomname" );
  list()->addColumn(tr("Inv. Fxd. Qty."),   _qtyColumn,  Qt::AlignRight, true, "invqtyfxd"  );
  list()->addColumn(tr("Inv. Qty. Per"),   _qtyColumn,   Qt::AlignRight, true, "invqtyper"  );
  list()->addColumn(tr("Scrap %"),        _prcntColumn, Qt::AlignRight, true, "bomitem_scrap"  );
  list()->addColumn(tr("Effective"),      _dateColumn,  Qt::AlignCenter,true, "bomitem_effective" );
  list()->addColumn(tr("Expires"),        _dateColumn,  Qt::AlignCenter,true, "bomitem_expires" );
  list()->addColumn(tr("ECN #"),          _itemColumn,  Qt::AlignLeft,  true, "bomitem_ecn"   );
  list()->addColumn(tr("Notes"),          _itemColumn,  Qt::AlignLeft,  false, "bomitem_notes"   );
  list()->addColumn(tr("Reference"),      _itemColumn,  Qt::AlignLeft,  false, "bomitem_ref"   );
}

bool dspSingleLevelBOM::setParams(ParameterList &params)
{
  params.append("bySingleLvl");
  return dspBOMBase::setParams(params);
}
