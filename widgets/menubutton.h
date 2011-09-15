/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __MENUBUTTON_H__
#define __MENUBUTTON_H__

#include <QWidget>
#include <QActionEvent>

#include "widgets.h"
#include "guiclientinterface.h"
#include "ui_menubutton.h"

class QScriptEngine;

void setupMenuButton(QScriptEngine *engine);

class XTUPLEWIDGETS_EXPORT MenuButton : public QWidget, public Ui::MenuButton
{
  Q_OBJECT
  Q_PROPERTY(QString image            READ image         WRITE setImage)
  Q_PROPERTY(QString label            READ label         WRITE setLabel)
  Q_PROPERTY(QString actionName       READ actionName    WRITE setAction)

  public:
    MenuButton(QWidget * = 0);
    ~MenuButton();

    static GuiClientInterface *_guiClientInterface;

    QString image() const;
    void setImage(const QString & image);

    QString label() const;
    void setLabel(const QString & text );

    QString actionName() const;
    void setAction(const QString & name );

  public slots:
    void setAction(QAction *action);
    
  protected:
    void actionEvent(QActionEvent *);
    void showEvent(QShowEvent *);

  private:
    QAction *_action;
    QString _image;
    bool _shown;
};

Q_DECLARE_METATYPE(MenuButton*)

#endif
