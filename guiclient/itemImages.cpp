/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemImages.h"

#include <qvariant.h>
#include <quuencode.h>
//#include <qstatusbar.h>
#include <qimage.h>

/*
 *  Constructs a itemImages as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
itemImages::itemImages(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_prev, SIGNAL(clicked()), this, SLOT(sPrevious()));
    connect(_next, SIGNAL(clicked()), this, SLOT(sNext()));
    connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemImages::~itemImages()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemImages::languageChange()
{
    retranslateUi(this);
}


void itemImages::init()
{
//  statusBar()->hide();

#ifndef Q_WS_MAC
  _prev->setMaximumWidth(25);
  _next->setMaximumWidth(25);
#endif
}

enum SetResponse itemImages::set(ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  return NoError;
}

void itemImages::sPrevious()
{
  _images.prev();
  loadImage();
}

void itemImages::sNext()
{
  _images.next();
  loadImage();
}

void itemImages::sFillList()
{
  _images.prepare( "SELECT imageass_id, image_data, image_descrip,"
                   "       CASE WHEN (imageass_purpose='I') THEN :inventoryDescription"
                   "            WHEN (imageass_purpose='P') THEN :productDescription"
                   "            WHEN (imageass_purpose='E') THEN :engineeringReference"
                   "            WHEN (imageass_purpose='M') THEN :miscellaneous"
                   "            ELSE :other"
                   "       END AS purpose "
                   "FROM imageass, image "
                   "WHERE ( (imageass_image_id=image_id)"
                   " AND (imageass_source_id=:item_id) "
                   " AND (imageass_source='I') ) "
                   "ORDER BY image_name;" );
  _images.bindValue(":item_id", _item->id());
  _images.bindValue(":inventoryDescription", tr("Inventory Description"));
  _images.bindValue(":productDescription", tr("Product Description"));
  _images.bindValue(":engineeringReference", tr("Engineering Reference"));
  _images.bindValue(":miscellaneous", tr("Miscellaneous"));
  _images.bindValue(":other", tr("Other"));
  _images.exec();
  if (_images.first())
    loadImage();
  else
  {
    _prev->setEnabled(FALSE);
    _next->setEnabled(FALSE);
    _image->clear();
  }
}

void itemImages::loadImage()
{
  _prev->setEnabled(_images.at() != 0);
  _next->setEnabled(_images.at() < (_images.size() - 1));

  _description->setText(_images.value("purpose").toString() + " - " + _images.value("image_descrip").toString());

  QImage image;
  image.loadFromData(QUUDecode(_images.value("image_data").toString()));
  _image->setPixmap(QPixmap::fromImage(image));
}

