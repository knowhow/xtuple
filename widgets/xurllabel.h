/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef xurllabel_h
#define xurllabel_h

#include <QLabel>
#include <QMouseEvent>

#include "widgets.h"

class XTUPLEWIDGETS_EXPORT XURLLabel : public QLabel
{
  Q_OBJECT
  Q_PROPERTY (QString url READ url WRITE setURL)

  public:
    XURLLabel(QWidget * parent = 0, const char* name = 0);

    virtual ~XURLLabel();

    const QString & url() const;

  public slots:
    void setURL(const QString&);
    void setFont(const QFont&);

  signals:
    void leftClickedURL(const QString&);
    void rightClickedURL(const QString&);
    void middleClickedURL(const QString&);

  protected:
    virtual void mouseReleaseEvent(QMouseEvent*);

  private:
    QString _url;
};

#endif

