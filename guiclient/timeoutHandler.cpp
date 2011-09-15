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

#include "timeoutHandler.h"

#include <QApplication>
#include <QEvent>

/*
 * This will initialize the TimeoutHandler which will be disabled
 * at first. You must connect the timeout signal to a slot in your
 * application then call setIdleMinutes() with a value greater than
 * zero to make the TimeoutHandler Active.
 */
TimeoutHandler::TimeoutHandler(QObject* parent, const char* name)
  : QObject(parent), _timer(this) {
    if(name)
      setObjectName(name);

    _timeoutValue = 0; // 0ms (disabled)
    // this will keep track of our elapsed time
    _lastEvent.start();

    // connect the timer to ourselves
    connect(&_timer, SIGNAL(timeout()), this, SLOT(sCheckTimeout()));

    // install this object into the applications
    // event filter so that we can see everything
    // that happens
    qApp->installEventFilter(this);
}

/*
 * Virtual Deconstructor
 */
TimeoutHandler::~TimeoutHandler() {
    if(_timer.isActive())
        _timer.stop();
}

/*
 * Returns true if this TimeoutHandler object is currently
 * active; otherwise, false will be returned
 */
bool TimeoutHandler::isActive() {
    return _timer.isActive();
}

/*
 * Returns the number of minutes a user can be idle
 * before the timeout() signal will be issued.
 */
int TimeoutHandler::idleMinutes() {
    if(_timeoutValue == 0)
        return 0;
    return ((_timeoutValue / 1000) / 60);
}

/*
 * Sets the number of minutes a user can idle before
 * the timeout() signal will be issued. If you pass
 * in a value of zero (0) or less it will disable the
 * TimeoutHandler class and no signals will be generated.
 */
void TimeoutHandler::setIdleMinutes(int minutes) {
    if(minutes < 1) {
        _timeoutValue = 0;
        if(_timer.isActive())
            _timer.stop();
    } else {
        _timeoutValue = minutes * 60000; // convert from minutes to milliseconds
        if(!_timer.isActive())
            _timer.start(60000);
    }
    _lastEvent.restart();
}

/*
 * This is the overrided method from QObject that receives the event
 * notifications that we setup to listen for (qApp) and on certain
 * events will reset the _lastEvent QTime object to the current time.
 */
bool TimeoutHandler::eventFilter(QObject* /*watched*/, QEvent* event) {
    QEvent::Type eType = event->type();
    if( eType == QEvent::MouseButtonPress || eType == QEvent::KeyPress ) {
        _lastEvent.restart(); // reset our elapsed timer
    }

    // ALWAYS RETURN FALSE so that the event gets
    // passed on to the next widget for processing
    return false;
}

/*
 * This slot is called by the QTimer object _timer each minute
 * and compares the elapsed time of from now and the last event
 * to the _timeoutValue.  If the elapsed time exceeds the _timeoutValue
 * the signal timeout() will be issued.
 *
 * NOTE: The elapsed time value is not reset here. If the TimeoutHandler
 *       is not manually reset or if no Events are received before the next
 *       time the QTimer object _timer calls this function then timeout()
 *       will be issued again..
 */
void TimeoutHandler::sCheckTimeout() {
    if(_lastEvent.elapsed() > _timeoutValue) {
        // the user is at or over the whatever idle time they
        // have been allowed so now we need to deal with it
        timeout();
    }
}

/*
 * Resets the QTime object value _lastEvent.
 */
void TimeoutHandler::reset() {
    _lastEvent.restart();
}

