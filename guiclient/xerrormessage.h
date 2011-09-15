/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XERRORMESSAGE_H
#define XERRORMESSAGE_H

#include <QDialog>
#include "qcheckbox.h"
#include "qlabel.h"
#include "qtextedit.h"
#include "qpushbutton.h"
#include "qstringlist.h"
#include <qhash.h>

class XErrorMessage: public QDialog
{
    Q_OBJECT
public:
    explicit XErrorMessage(QWidget* parent = 0);
    ~XErrorMessage();

public Q_SLOTS:
    void showMessage(const QString &message);

protected:
    void done(int);
    void changeEvent(QEvent *e);
    
    void retranslateStrings();
    bool nextPending(); 
    
private:
    QPushButton * _ok;
    QCheckBox * _again;
    QTextEdit * _errors;
    QLabel * _icon;
    QStringList _pending;
    QString _currentMessage;
    QString _settingsName;

};


#endif // XERRORMESSAGE_H
