/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ITEMIMAGES_H
#define ITEMIMAGES_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_itemImages.h"

class itemImages : public XWidget, public Ui::itemImages
{
    Q_OBJECT

public:
    itemImages(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~itemImages();

    virtual void init();

public slots:
    virtual enum SetResponse set( ParameterList & pParams );
    virtual void sPrevious();
    virtual void sNext();
    virtual void sFillList();
    virtual void loadImage();

protected slots:
    virtual void languageChange();

private:
    XSqlQuery _images;

};

#endif // ITEMIMAGES_H
