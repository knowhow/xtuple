/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "uoms.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <QSqlError>
#include <parameter.h>
#include <openreports.h>
#include "storedProcErrorLookup.h"
#include "uom.h"

/*
 *  Constructs a uoms as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
uoms::uoms(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_uoms, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

  if (_privileges->check("MaintainUOMs"))
  {
    connect(_uoms, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_uoms, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_uoms, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_uoms, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _uoms->addColumn(tr("UOM"),         _itemColumn, Qt::AlignCenter, true,  "uom_name" );
  _uoms->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "uom_descrip"   );
  _uoms->addColumn(tr("Item Weight"), _itemColumn, Qt::AlignLeft,   true,  "uom_item_weight"   );

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
uoms::~uoms()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void uoms::languageChange()
{
  retranslateUi(this);
}

void uoms::sFillList()
{
  _uoms->populate( "SELECT uom.*,"
                   "       CASE WHEN (NOT uom_item_weight) THEN '' END AS uom_item_weight_qtdisplayrole "
                   "  FROM uom "
                   " ORDER BY uom_name;"  );
}

void uoms::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  uom newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void uoms::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("uom_id", _uoms->id());

  uom newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void uoms::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("uom_id", _uoms->id());

  uom newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void uoms::sDelete()
{
  q.prepare( "SELECT deleteUOM(:uom_id) AS result;" );
  q.bindValue(":uom_id", _uoms->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteUOM", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void uoms::sPrint()
{
  orReport report("UOMs");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

