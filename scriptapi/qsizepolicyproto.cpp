/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsizepolicyproto.h"

QScriptValue QSizePolicyControlTypetoScriptValue(QScriptEngine *engine, const enum QSizePolicy::ControlType &p)
{ return QScriptValue(engine, (int)p); }

void QSizePolicyControlTypefromScriptValue(const QScriptValue &obj, enum QSizePolicy::ControlType &p)
{ p = (enum QSizePolicy::ControlType)obj.toInt32(); }

QScriptValue QSizePolicyPolicytoScriptValue(QScriptEngine *engine, const enum QSizePolicy::Policy &p)
{ return QScriptValue(engine, (int)p); }

void QSizePolicyPolicyfromScriptValue(const QScriptValue &obj, enum QSizePolicy::Policy &p)
{ p = (enum QSizePolicy::Policy)obj.toInt32(); }

QScriptValue QSizePolicyPolicyFlagtoScriptValue(QScriptEngine *engine, const enum QSizePolicy::PolicyFlag &p)
{ return QScriptValue(engine, (int)p); }

void QSizePolicyPolicyFlagfromScriptValue(const QScriptValue &obj, enum QSizePolicy::PolicyFlag &p)
{ p = (enum QSizePolicy::PolicyFlag)obj.toInt32(); }

void setupQSizePolicy(QScriptEngine *engine)
{
  QScriptValue proto = engine->newObject();

  //enum ControlType
  proto.setProperty("DefaultType", QSizePolicy::DefaultType);
  proto.setProperty("ButtonBox",   QSizePolicy::ButtonBox);
  proto.setProperty("CheckBox",    QSizePolicy::CheckBox);
  proto.setProperty("ComboBox",    QSizePolicy::ComboBox);
  proto.setProperty("Frame",       QSizePolicy::Frame);
  proto.setProperty("GroupBox",    QSizePolicy::GroupBox);
  proto.setProperty("Label",       QSizePolicy::Label);
  proto.setProperty("Line",        QSizePolicy::Line);
  proto.setProperty("LineEdit",    QSizePolicy::LineEdit);
  proto.setProperty("PushButton",  QSizePolicy::PushButton);
  proto.setProperty("RadioButton", QSizePolicy::RadioButton);
  proto.setProperty("Slider",      QSizePolicy::Slider);
  proto.setProperty("SpinBox",     QSizePolicy::SpinBox);
  proto.setProperty("TabWidget",   QSizePolicy::TabWidget);
  proto.setProperty("ToolButton",  QSizePolicy::ToolButton);

  //enum Policy
  proto.setProperty("Fixed",           QSizePolicy::Fixed);
  proto.setProperty("Minimum",         QSizePolicy::Minimum);
  proto.setProperty("Maximum",         QSizePolicy::Maximum);
  proto.setProperty("Preferred",       QSizePolicy::Preferred);
  proto.setProperty("Expanding",       QSizePolicy::Expanding);
  proto.setProperty("MinimumExpanding",QSizePolicy::MinimumExpanding);
  proto.setProperty("Ignored",         QSizePolicy::Ignored);

  // enum PolicyFlag
  proto.setProperty("GrowFlag",   QSizePolicy::GrowFlag);
  proto.setProperty("ExpandFlag", QSizePolicy::ExpandFlag);
  proto.setProperty("ShrinkFlag", QSizePolicy::ShrinkFlag);
  proto.setProperty("IgnoreFlag", QSizePolicy::IgnoreFlag);

  engine->globalObject().setProperty("QSizePolicy", proto);
}
