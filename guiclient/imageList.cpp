/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "imageList.h"

#include <qvariant.h>

/*
 *  Constructs a imageList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
imageList::imageList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
    connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
    connect(_image, SIGNAL(valid(bool)), _select, SLOT(setEnabled(bool)));
    connect(_image, SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
imageList::~imageList()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void imageList::languageChange()
{
    retranslateUi(this);
}


void imageList::init()
{
  _image->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft, true, "image_name" );
  _image->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "image_descrip" );
}

enum SetResponse imageList::set(ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("image_id", &valid);
  if (valid)
    _imageid = param.toInt();
  else
    _imageid = -1;

  _image->populate( "SELECT image_id, image_name, firstLine(image_descrip) AS image_descrip "
                    "FROM image "
                    "ORDER BY image_name;", _imageid );

  return NoError;
}

void imageList::sClose()
{
  done(_imageid);
}

void imageList::sSelect()
{
  done(_image->id());
}

