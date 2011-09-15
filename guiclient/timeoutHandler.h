/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

/*
 *     This class attaches itself to the qApp event filter
 * and listens for events to come across.  In addition it
 * listens for a timer that fires once a minute and no user
 * generated events have occured in an amount of time specified
 * by the user preferences value then a dialog will be popped up
 * notifing the user that they have 60 secs to cancel the shutdown
 * Sequence or their client will be terminated.
 */
#ifndef __TIMEOUT_H__
#define __TIMEOUT_H__

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QEvent>

class TimeoutHandler : public QObject {
    Q_OBJECT
public:
    TimeoutHandler(QObject* = 0, const char* = 0);
    virtual ~TimeoutHandler();

    bool eventFilter(QObject*, QEvent*);

    bool isActive();
    int idleMinutes();
    void setIdleMinutes(int);

    void reset();

signals:
    void timeout();

protected:
    QTimer _timer;     // 1-minute timer
    QTime  _lastEvent; // last event time

protected slots:
    void sCheckTimeout();

private:
    int _timeoutValue;
};

#endif

