/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef numbergencombobox_h
#define numbergencombobox_h

#include <QComboBox>

#include "widgets.h"

#include <xsqlquery.h>

class QScriptEngine;
class QStandardItemModel;

void setupNumberGenComboBox(QScriptEngine *engine);

class XTUPLEWIDGETS_EXPORT NumberGenComboBox : public QComboBox
{
  Q_OBJECT

  Q_ENUMS(GenMethod)
  Q_FLAGS(GenMethods)

  Q_PROPERTY(GenMethods allowedMethods READ allowedMethods WRITE setAllowedMethods )
  Q_PROPERTY(QString    automaticText  READ automaticText  WRITE setAutomaticText  )
  Q_PROPERTY(QString    manualText     READ manualText     WRITE setManualText     )
  Q_PROPERTY(QString    overrideText   READ overrideText   WRITE setOverrideText   )
  Q_PROPERTY(QString    sharedText     READ sharedText     WRITE setSharedText     )

  public:
    NumberGenComboBox(QWidget *parent = 0);
    virtual ~NumberGenComboBox();

    enum GenMethod { Manual   = 0x01, Automatic = 0x02,
                     Override = 0x04, Shared    = 0x08 };
    Q_DECLARE_FLAGS(GenMethods, GenMethod)

    Q_INVOKABLE void append(GenMethod method, QString text, QString code = QString());
    Q_INVOKABLE GenMethods allowedMethods() const;
    Q_INVOKABLE QString    automaticText()  const;
    Q_INVOKABLE QString    manualText()     const;
    Q_INVOKABLE GenMethod  method()         const;
    Q_INVOKABLE QString    methodCode()     const;
    Q_INVOKABLE QString    overrideText()   const;
    Q_INVOKABLE QString    sharedText()     const;

    Q_INVOKABLE static QString methodToCode(const GenMethod method);

  public slots:
    void       handleIndexChanged(const int index);
    void       setAllowedMethods(const GenMethods methods);
    void       setAutomaticText(const QString text);
    void       setManualText(const QString text);
    void       setMethod(const GenMethod method);
    void       setMethod(const QString methodCode);
    void       setOverrideText(const QString text);
    void       setSharedText(const QString text);

  signals:
    void newMethod(GenMethod);
    void newMethod(int);
    void newMethodCode(QString);

  protected:
    int  indexOfMethod(const GenMethod method) const;

  private:
    GenMethods          _allowedMethods;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(NumberGenComboBox::GenMethods);

Q_DECLARE_METATYPE(NumberGenComboBox*)
// TODO: is this necessary for script exposure?
Q_DECLARE_METATYPE(enum NumberGenComboBox::GenMethod)

#endif

