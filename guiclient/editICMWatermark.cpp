/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "editICMWatermark.h"

#include <qvariant.h>

/*
 *  Constructs a editICMWatermark as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
editICMWatermark::editICMWatermark(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_save, SIGNAL(clicked()), this, SLOT(accept()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
editICMWatermark::~editICMWatermark()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void editICMWatermark::languageChange()
{
    retranslateUi(this);
}


enum SetResponse editICMWatermark::set(ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("watermark", &valid);
  if (valid)
    _watermark->setText(param.toString());

  param = pParams.value("showPrices", &valid);
  if (valid)
    _showPrices->setChecked(param.toBool());

  return NoError;
}


QString editICMWatermark::watermark()
{
  return _watermark->text();
}


bool editICMWatermark::showPrices()
{
  return _showPrices->isChecked();
}
