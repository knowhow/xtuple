/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <parameter.h>

#include "widgets.h"
#include "ui_imageview.h"

class imageview : public QDialog, public Ui::imageview
{
    Q_OBJECT

public:
    imageview(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~imageview();

    QPushButton* _save;

public slots:
    virtual void set( const ParameterList & pParams );
    virtual void populate();
    virtual void sSave();
    virtual void sFileList();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _imageviewid;
    QImage __imageview;
    QLabel *_imageview;

};

#endif // IMAGEVIEW_H
