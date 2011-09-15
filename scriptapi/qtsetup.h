/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QTSETUP_H__
#define __QTSETUP_H__

#include <Qt>
#include <QtScript>
#include <QTextDocument>

void setupQt(QScriptEngine *engine);

QScriptValue AlignmentFlagtoScriptValue(QScriptEngine *engine, const enum Qt::AlignmentFlag &p);
QScriptValue AnchorAttributetoScriptValue(QScriptEngine *engine, const enum Qt::AnchorAttribute &p);
QScriptValue ApplicationAttributetoScriptValue(QScriptEngine *engine, const enum Qt::ApplicationAttribute &p);
QScriptValue ArrowTypetoScriptValue(QScriptEngine *engine, const enum Qt::ArrowType &p);
QScriptValue AspectRatioModetoScriptValue(QScriptEngine *engine, const enum Qt::AspectRatioMode &p);
QScriptValue AxistoScriptValue(QScriptEngine *engine, const enum Qt::Axis &p);
QScriptValue BGModetoScriptValue(QScriptEngine *engine, const enum Qt::BGMode &p);
QScriptValue BrushStyletoScriptValue(QScriptEngine *engine, const enum Qt::BrushStyle &p);
QScriptValue CaseSensitivitytoScriptValue(QScriptEngine *engine, const enum Qt::CaseSensitivity &p);
QScriptValue CheckStatetoScriptValue(QScriptEngine *engine, const enum Qt::CheckState &p);
QScriptValue ClipOperationtoScriptValue(QScriptEngine *engine, const enum Qt::ClipOperation &p);
QScriptValue ConnectionTypetoScriptValue(QScriptEngine *engine, const enum Qt::ConnectionType &p);
QScriptValue ContextMenuPolicytoScriptValue(QScriptEngine *engine, const enum Qt::ContextMenuPolicy &p);
QScriptValue CornertoScriptValue(QScriptEngine *engine, const enum Qt::Corner &p);
QScriptValue CursorShapetoScriptValue(QScriptEngine *engine, const enum Qt::CursorShape &p);
QScriptValue DateFormattoScriptValue(QScriptEngine *engine, const enum Qt::DateFormat &p);
QScriptValue DayOfWeektoScriptValue(QScriptEngine *engine, const enum Qt::DayOfWeek &p);
QScriptValue DockWidgetAreatoScriptValue(QScriptEngine *engine, const enum Qt::DockWidgetArea &p);
QScriptValue DropActiontoScriptValue(QScriptEngine *engine, const enum Qt::DropAction &p);
QScriptValue EventPrioritytoScriptValue(QScriptEngine *engine, const enum Qt::EventPriority &p);
QScriptValue FillRuletoScriptValue(QScriptEngine *engine, const enum Qt::FillRule &p);
QScriptValue FocusPolicytoScriptValue(QScriptEngine *engine, const enum Qt::FocusPolicy &p);
QScriptValue FocusReasontoScriptValue(QScriptEngine *engine, const enum Qt::FocusReason &p);
QScriptValue GlobalColortoScriptValue(QScriptEngine *engine, const enum Qt::GlobalColor &p);
QScriptValue HitTestAccuracytoScriptValue(QScriptEngine *engine, const enum Qt::HitTestAccuracy &p);
QScriptValue ImageConversionFlagtoScriptValue(QScriptEngine *engine, const enum Qt::ImageConversionFlag &p);
QScriptValue InputMethodQuerytoScriptValue(QScriptEngine *engine, const enum Qt::InputMethodQuery &p);
QScriptValue ItemDataRoletoScriptValue(QScriptEngine *engine, const enum Qt::ItemDataRole &p);
QScriptValue ItemFlagtoScriptValue(QScriptEngine *engine, const enum Qt::ItemFlag &p);
QScriptValue ItemSelectionModetoScriptValue(QScriptEngine *engine, const enum Qt::ItemSelectionMode &p);
QScriptValue KeytoScriptValue(QScriptEngine *engine, const enum Qt::Key &p);
QScriptValue KeyboardModifiertoScriptValue(QScriptEngine *engine, const enum Qt::KeyboardModifier &p);
QScriptValue LayoutDirectiontoScriptValue(QScriptEngine *engine, const enum Qt::LayoutDirection &p);
QScriptValue MaskModetoScriptValue(QScriptEngine *engine, const enum Qt::MaskMode &p);
QScriptValue MatchFlagtoScriptValue(QScriptEngine *engine, const enum Qt::MatchFlag &p);
QScriptValue MatchFlagstoScriptValue(QScriptEngine *engine, const Qt::MatchFlags &p);
QScriptValue ModifiertoScriptValue(QScriptEngine *engine, const enum Qt::Modifier &p);
QScriptValue MouseButtontoScriptValue(QScriptEngine *engine, const enum Qt::MouseButton &p);
//QScriptValue OrientationtoScriptValue(QScriptEngine *engine, const enum Qt::Orientation &p);
QScriptValue PenCapStyletoScriptValue(QScriptEngine *engine, const enum Qt::PenCapStyle &p);
QScriptValue PenJoinStyletoScriptValue(QScriptEngine *engine, const enum Qt::PenJoinStyle &p);
QScriptValue PenStyletoScriptValue(QScriptEngine *engine, const enum Qt::PenStyle &p);
QScriptValue ScrollBarPolicytoScriptValue(QScriptEngine *engine, const enum Qt::ScrollBarPolicy &p);
QScriptValue ShortcutContexttoScriptValue(QScriptEngine *engine, const enum Qt::ShortcutContext &p);
QScriptValue SizeHinttoScriptValue(QScriptEngine *engine, const enum Qt::SizeHint &p);
QScriptValue SizeModetoScriptValue(QScriptEngine *engine, const enum Qt::SizeMode &p);
QScriptValue SortOrdertoScriptValue(QScriptEngine *engine, const enum Qt::SortOrder &p);
QScriptValue TextElideModetoScriptValue(QScriptEngine *engine, const enum Qt::TextElideMode &p);
QScriptValue TextFlagtoScriptValue(QScriptEngine *engine, const enum Qt::TextFlag &p);
QScriptValue TextFormattoScriptValue(QScriptEngine *engine, const enum Qt::TextFormat &p);
QScriptValue TextInteractionFlagtoScriptValue(QScriptEngine *engine, const enum Qt::TextInteractionFlag &p);
QScriptValue TimeSpectoScriptValue(QScriptEngine *engine, const enum Qt::TimeSpec &p);
QScriptValue ToolBarAreatoScriptValue(QScriptEngine *engine, const enum Qt::ToolBarArea &p);
QScriptValue ToolButtonStyletoScriptValue(QScriptEngine *engine, const enum Qt::ToolButtonStyle &p);
QScriptValue TransformationModetoScriptValue(QScriptEngine *engine, const enum Qt::TransformationMode &p);
QScriptValue UIEffecttoScriptValue(QScriptEngine *engine, const enum Qt::UIEffect &p);
QScriptValue WhiteSpaceModetoScriptValue(QScriptEngine *engine, const enum Qt::WhiteSpaceMode &p);
QScriptValue WidgetAttributetoScriptValue(QScriptEngine *engine, const enum Qt::WidgetAttribute &p);
QScriptValue WindowFrameSectiontoScriptValue(QScriptEngine *engine, const enum Qt::WindowFrameSection &p);
QScriptValue WindowModalitytoScriptValue(QScriptEngine *engine, const enum Qt::WindowModality &p);
QScriptValue WindowStatetoScriptValue(QScriptEngine *engine, const enum Qt::WindowState &p);
QScriptValue WindowTypetoScriptValue(QScriptEngine *engine, const enum Qt::WindowType &p);

void AlignmentFlagfromScriptValue(const QScriptValue &obj, enum Qt::AlignmentFlag &p);
void AnchorAttributefromScriptValue(const QScriptValue &obj, enum Qt::AnchorAttribute &p);
void ApplicationAttributefromScriptValue(const QScriptValue &obj, enum Qt::ApplicationAttribute &p);
void ArrowTypefromScriptValue(const QScriptValue &obj, enum Qt::ArrowType &p);
void AspectRatioModefromScriptValue(const QScriptValue &obj, enum Qt::AspectRatioMode &p);
void AxisfromScriptValue(const QScriptValue &obj, enum Qt::Axis &p);
void BGModefromScriptValue(const QScriptValue &obj, enum Qt::BGMode &p);
void BrushStylefromScriptValue(const QScriptValue &obj, enum Qt::BrushStyle &p);
void CaseSensitivityfromScriptValue(const QScriptValue &obj, enum Qt::CaseSensitivity &p);
void CheckStatefromScriptValue(const QScriptValue &obj, enum Qt::CheckState &p);
void ClipOperationfromScriptValue(const QScriptValue &obj, enum Qt::ClipOperation &p);
void ConnectionTypefromScriptValue(const QScriptValue &obj, enum Qt::ConnectionType &p);
void ContextMenuPolicyfromScriptValue(const QScriptValue &obj, enum Qt::ContextMenuPolicy &p);
void CornerfromScriptValue(const QScriptValue &obj, enum Qt::Corner &p);
void CursorShapefromScriptValue(const QScriptValue &obj, enum Qt::CursorShape &p);
void DateFormatfromScriptValue(const QScriptValue &obj, enum Qt::DateFormat &p);
void DayOfWeekfromScriptValue(const QScriptValue &obj, enum Qt::DayOfWeek &p);
void DockWidgetAreafromScriptValue(const QScriptValue &obj, enum Qt::DockWidgetArea &p);
void DropActionfromScriptValue(const QScriptValue &obj, enum Qt::DropAction &p);
void EventPriorityfromScriptValue(const QScriptValue &obj, enum Qt::EventPriority &p);
void FillRulefromScriptValue(const QScriptValue &obj, enum Qt::FillRule &p);
void FocusPolicyfromScriptValue(const QScriptValue &obj, enum Qt::FocusPolicy &p);
void FocusReasonfromScriptValue(const QScriptValue &obj, enum Qt::FocusReason &p);
void GlobalColorfromScriptValue(const QScriptValue &obj, enum Qt::GlobalColor &p);
void HitTestAccuracyfromScriptValue(const QScriptValue &obj, enum Qt::HitTestAccuracy &p);
void ImageConversionFlagfromScriptValue(const QScriptValue &obj, enum Qt::ImageConversionFlag &p);
void InputMethodQueryfromScriptValue(const QScriptValue &obj, enum Qt::InputMethodQuery &p);
void ItemDataRolefromScriptValue(const QScriptValue &obj, enum Qt::ItemDataRole &p);
void ItemFlagfromScriptValue(const QScriptValue &obj, enum Qt::ItemFlag &p);
void ItemSelectionModefromScriptValue(const QScriptValue &obj, enum Qt::ItemSelectionMode &p);
void KeyfromScriptValue(const QScriptValue &obj, enum Qt::Key &p);
void KeyboardModifierfromScriptValue(const QScriptValue &obj, enum Qt::KeyboardModifier &p);
void LayoutDirectionfromScriptValue(const QScriptValue &obj, enum Qt::LayoutDirection &p);
void MaskModefromScriptValue(const QScriptValue &obj, enum Qt::MaskMode &p);
void MatchFlagfromScriptValue(const QScriptValue &obj, enum Qt::MatchFlag &p);
void MatchFlagsfromScriptValue(const QScriptValue &obj, Qt::MatchFlags &p);
void ModifierfromScriptValue(const QScriptValue &obj, enum Qt::Modifier &p);
void MouseButtonfromScriptValue(const QScriptValue &obj, enum Qt::MouseButton &p);
//void OrientationfromScriptValue(const QScriptValue &obj, enum Qt::Orientation &p);
void PenCapStylefromScriptValue(const QScriptValue &obj, enum Qt::PenCapStyle &p);
void PenJoinStylefromScriptValue(const QScriptValue &obj, enum Qt::PenJoinStyle &p);
void PenStylefromScriptValue(const QScriptValue &obj, enum Qt::PenStyle &p);
void ScrollBarPolicyfromScriptValue(const QScriptValue &obj, enum Qt::ScrollBarPolicy &p);
void ShortcutContextfromScriptValue(const QScriptValue &obj, enum Qt::ShortcutContext &p);
void SizeHintfromScriptValue(const QScriptValue &obj, enum Qt::SizeHint &p);
void SizeModefromScriptValue(const QScriptValue &obj, enum Qt::SizeMode &p);
void SortOrderfromScriptValue(const QScriptValue &obj, enum Qt::SortOrder &p);
void TextElideModefromScriptValue(const QScriptValue &obj, enum Qt::TextElideMode &p);
void TextFlagfromScriptValue(const QScriptValue &obj, enum Qt::TextFlag &p);
void TextFormatfromScriptValue(const QScriptValue &obj, enum Qt::TextFormat &p);
void TextInteractionFlagfromScriptValue(const QScriptValue &obj, enum Qt::TextInteractionFlag &p);
void TimeSpecfromScriptValue(const QScriptValue &obj, enum Qt::TimeSpec &p);
void ToolBarAreafromScriptValue(const QScriptValue &obj, enum Qt::ToolBarArea &p);
void ToolButtonStylefromScriptValue(const QScriptValue &obj, enum Qt::ToolButtonStyle &p);
void TransformationModefromScriptValue(const QScriptValue &obj, enum Qt::TransformationMode &p);
void UIEffectfromScriptValue(const QScriptValue &obj, enum Qt::UIEffect &p);
void WhiteSpaceModefromScriptValue(const QScriptValue &obj, enum Qt::WhiteSpaceMode &p);
void WidgetAttributefromScriptValue(const QScriptValue &obj, enum Qt::WidgetAttribute &p);
void WindowFrameSectionfromScriptValue(const QScriptValue &obj, enum Qt::WindowFrameSection &p);
void WindowModalityfromScriptValue(const QScriptValue &obj, enum Qt::WindowModality &p);
void WindowStatefromScriptValue(const QScriptValue &obj, enum Qt::WindowState &p);
void WindowTypefromScriptValue(const QScriptValue &obj, enum Qt::WindowType &p);

#endif
