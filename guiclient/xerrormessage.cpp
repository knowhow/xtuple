/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xerrormessage.h"

#include "guiclient.h"

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>  
#include <QPixmap>
#include <QPushButton>
#include <QTextEdit>

XErrorMessage::XErrorMessage(QWidget * parent)
    : QDialog(parent)
{
    QGridLayout * grid = new QGridLayout(this);
    _icon = new QLabel(this);
    _icon->setPixmap(QMessageBox::standardIcon(QMessageBox::Information));
    _icon->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    grid->addWidget(_icon, 0, 0, Qt::AlignTop);
    _errors = new QTextEdit(this);
    grid->addWidget(_errors, 0, 1);
    _again = new QCheckBox(this);
    _again->setChecked(true);
    grid->addWidget(_again, 1, 1, Qt::AlignTop);
    _ok = new QPushButton(this);
    connect(_ok, SIGNAL(clicked()), this, SLOT(accept()));
    _ok->setFocus();
    grid->addWidget(_ok, 2, 0, 1, 2, Qt::AlignCenter);
    grid->setColumnStretch(1, 42);
    grid->setRowStretch(0, 42);
    retranslateStrings();
}

XErrorMessage::~XErrorMessage()
{
}

void XErrorMessage::done(int a)
{
    if (!_again->isChecked() && !_currentMessage.isEmpty())
        _preferences->set("doNotShow/" + _currentMessage, true);
    _currentMessage.clear();
    if (!nextPending())
        QDialog::done(a);
}

bool XErrorMessage::nextPending()
{
    while (!_pending.isEmpty()) 
    {
        QString p = _pending.takeFirst();
        if (!p.isEmpty() && !_preferences->boolean("doNotShow/" + p)) 
        {
            _errors->setHtml(p);
            _currentMessage = p;
            return true;
        }
    }
    return false;
}

void XErrorMessage::showMessage(const QString &message)
{
    if (_preferences->boolean("doNotShow/" + message))
        return;
    _pending.append(message);
    if (!isVisible() && nextPending())
        show();
}

void XErrorMessage::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange)
        retranslateStrings();

    QDialog::changeEvent(e);
}

void XErrorMessage::retranslateStrings()
{
    _again->setText(XErrorMessage::tr("&Show this message again"));
    _ok->setText(XErrorMessage::tr("&OK"));
}
