/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _imageCluster_h

#define _imageCluster_h

#include "virtualCluster.h"

#include <QImage>

class ImageInfo : public VirtualInfo
{
    Q_OBJECT

    public:
      ImageInfo(QWidget*, Qt::WindowFlags = 0);
};

class ImageList : public VirtualList
{
    Q_OBJECT

    public:
      ImageList(QWidget*, Qt::WindowFlags = 0);
};

class ImageSearch : public VirtualSearch
{
    Q_OBJECT

    public:
      ImageSearch(QWidget*, Qt::WindowFlags = 0);
};


class XTUPLEWIDGETS_EXPORT ImageClusterLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT

  public:
    ImageClusterLineEdit(QWidget*, const char* = 0);

  protected:
    virtual VirtualInfo   *infoFactory();
    virtual VirtualList   *listFactory();
    virtual VirtualSearch *searchFactory();
};

class XTUPLEWIDGETS_EXPORT ImageCluster : public VirtualCluster
{
  Q_OBJECT

  Q_PROPERTY(bool numberVisible READ numberVisible WRITE setNumberVisible)

  public:
      ImageCluster(QWidget*, const char* = 0);

  public slots:
      virtual void clear();
      virtual void sRefresh();
      virtual bool numberVisible()     const { return _number->isVisible(); };
      virtual void setNumberVisible(const bool p);

  protected slots:

  private:
    QLabel *_image;
    QPixmap _nullPixmap;
};

#endif
