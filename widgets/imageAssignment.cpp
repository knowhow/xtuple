/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QVariant>

#include "imageAssignment.h"
#include "imageview.h"

imageAssignment::imageAssignment(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : QDialog(parent, fl)
{
  setupUi(this);

  setObjectName(name ? name : "imageAssignment");
  setModal(modal);

  // signals and slots connections
  connect(_image, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _image->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft, true, "image_name" );
  _image->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "image_descrip" );

  _imageassid = -1;
  _mode = cNew;
  _source = Documents::Uninitialized;
  _sourceid = -1;

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
imageAssignment::~imageAssignment()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void imageAssignment::languageChange()
{
  retranslateUi(this);
}

void imageAssignment::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("sourceType", &valid);
  if (valid)
  {
    _source = (enum Documents::DocumentSources)param.toInt();
    if (_source != Documents::Item)
    {
      _purpose->setCurrentIndex(3);
      _purpose->hide();
      _purposeLit->hide();
    }
  }

  param = pParams.value("source_id", &valid);
  if (valid)
    _sourceid = param.toInt();

  param = pParams.value("imageass_id", &valid);
  if (valid)
  {
    _imageassid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _purpose->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
      _purpose->setEnabled(FALSE);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _save->hide();
      _new->hide();
      _purpose->setEnabled(FALSE);
      _image->setEnabled(FALSE);
    }
  }
}

void imageAssignment::sSave()
{
  QString purpose;
  if (_purpose->currentIndex() == 0)  
    purpose = "I";
  if (_purpose->currentIndex() == 1)  
    purpose = "P";
  if (_purpose->currentIndex() == 2)  
    purpose = "E";
  if (_purpose->currentIndex() == 3)  
    purpose = "M";

  XSqlQuery newImage;

  if (_mode == cNew)
  {
    _imageassid = -1;

    if (purpose != "M")
    {
      XSqlQuery imageassid;
      imageassid.prepare( "SELECT imageass_id "
                           "FROM imageass "
                           "WHERE ( (imageass_source_id=:source_id)"
                           " AND ( imageass_source=:source) "
                           " AND (imageass_purpose=:imageass_purpose) );" );
      imageassid.bindValue(":source", Documents::_documentMap[_source].ident);
      imageassid.bindValue(":source_id", _sourceid);
      imageassid.bindValue(":imageass_purpose", purpose);
      imageassid.exec();
      if (imageassid.first())
      {
        _imageassid = imageassid.value("imageass_id").toInt();
        _mode = cEdit;
      }
    }
      
    if (_imageassid == -1)
    {
      XSqlQuery imageassid;
      imageassid.prepare("SELECT NEXTVAL('imageass_imageass_id_seq') AS _imageass_id;");
      imageassid.exec();
      if (imageassid.first())
        _imageassid = imageassid.value("_imageass_id").toInt();
    }

    newImage.prepare( "INSERT INTO imageass "
                      "( imageass_id, imageass_source, imageass_source_id, imageass_purpose, imageass_image_id ) "
                      "VALUES "
                      "( :imageass_id, :imageass_source, :imageass_source_id, :imageass_purpose, :imageass_image_id );" );
  }

  if (_mode == cEdit)
    newImage.prepare( "UPDATE imageass "
                      "SET imageass_purpose=:imageass_purpose, imageass_image_id=:imageass_image_id "
                      "WHERE (imageass_id=:imageass_id);" );

  newImage.bindValue(":imageass_id", _imageassid);
  newImage.bindValue(":imageass_source", Documents::_documentMap[_source].ident);
  newImage.bindValue(":imageass_source_id", _sourceid);
  newImage.bindValue(":imageass_image_id", _image->id());
  newImage.bindValue(":imageass_purpose", purpose);

  newImage.exec();

  done(_imageassid);
}

void imageAssignment::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  imageview newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void imageAssignment::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("image_id", _image->id());

  imageview newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void imageAssignment::sFillList()
{
  _image->populate( "SELECT image_id, image_name, firstLine(image_descrip) AS image_descrip "
                    "FROM image "
                    "ORDER BY image_name;" );
}

void imageAssignment::populate()
{
  XSqlQuery imageass;
  imageass.prepare( "SELECT imageass_purpose, imageass_image_id, imageass_source "
                     "FROM imageass "
                     "WHERE (imageass_id=:imageass_id);" );
  imageass.bindValue(":imageass_id", _imageassid);
  imageass.exec();
  if (imageass.first())
  {
    if (imageass.value("imageass_source").toString() == "I")
    {
      QString purpose = imageass.value("imageass_purpose").toString();

      if (purpose == "I")
        _purpose->setCurrentIndex(0);
      else if (purpose == "P")
        _purpose->setCurrentIndex(1);
      else if (purpose == "E")
        _purpose->setCurrentIndex(2);
      else if (purpose == "M")
        _purpose->setCurrentIndex(3);
    }
    else
    {
      _purpose->setCurrentIndex(3);
      _purpose->hide();
      _purposeLit->hide();
    }

    _image->setId(imageass.value("imageass_image_id").toInt());
  }
}

