/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qtsetup.h"

void setupQt(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  qScriptRegisterMetaType(engine, AlignmentFlagtoScriptValue,	AlignmentFlagfromScriptValue);
  widget.setProperty("AlignLeft", QScriptValue(engine, Qt::AlignLeft), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AlignRight", QScriptValue(engine, Qt::AlignRight), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AlignHCenter", QScriptValue(engine, Qt::AlignHCenter), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AlignJustify", QScriptValue(engine, Qt::AlignJustify), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AlignTop", QScriptValue(engine, Qt::AlignTop), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AlignBottom", QScriptValue(engine, Qt::AlignBottom), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AlignVCenter", QScriptValue(engine, Qt::AlignVCenter), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AlignCenter", QScriptValue(engine, Qt::AlignCenter), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AlignAbsolute", QScriptValue(engine, Qt::AlignAbsolute), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AlignLeading", QScriptValue(engine, Qt::AlignLeading), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AlignTrailing", QScriptValue(engine, Qt::AlignTrailing), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AlignHorizontal_Mask", QScriptValue(engine, Qt::AlignHorizontal_Mask), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AlignVertical_Mask", QScriptValue(engine, Qt::AlignVertical_Mask), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, AnchorAttributetoScriptValue,	AnchorAttributefromScriptValue);
  widget.setProperty("AnchorName", QScriptValue(engine, Qt::AnchorName), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AnchorHref", QScriptValue(engine, Qt::AnchorHref), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ApplicationAttributetoScriptValue,	ApplicationAttributefromScriptValue);
  widget.setProperty("AA_ImmediateWidgetCreation", QScriptValue(engine, Qt::AA_ImmediateWidgetCreation), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AA_MSWindowsUseDirect3DByDefault", QScriptValue(engine, Qt::AA_MSWindowsUseDirect3DByDefault), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AA_DontShowIconsInMenus", QScriptValue(engine, Qt::AA_DontShowIconsInMenus), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AA_NativeWindows", QScriptValue(engine, Qt::AA_NativeWindows), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AA_DontCreateNativeWidgetSiblings", QScriptValue(engine, Qt::AA_DontCreateNativeWidgetSiblings), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AA_MacPluginApplication", QScriptValue(engine, Qt::AA_MacPluginApplication), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ArrowTypetoScriptValue,	ArrowTypefromScriptValue);
  widget.setProperty("NoArrow", QScriptValue(engine, Qt::NoArrow), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UpArrow", QScriptValue(engine, Qt::UpArrow), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DownArrow", QScriptValue(engine, Qt::DownArrow), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LeftArrow", QScriptValue(engine, Qt::LeftArrow), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RightArrow", QScriptValue(engine, Qt::RightArrow), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, AspectRatioModetoScriptValue,	AspectRatioModefromScriptValue);
  widget.setProperty("IgnoreAspectRatio", QScriptValue(engine, Qt::IgnoreAspectRatio), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("KeepAspectRatio", QScriptValue(engine, Qt::KeepAspectRatio), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("KeepAspectRatioByExpanding", QScriptValue(engine, Qt::KeepAspectRatioByExpanding), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, AxistoScriptValue,	AxisfromScriptValue);
  widget.setProperty("XAxis", QScriptValue(engine, Qt::XAxis), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("YAxis", QScriptValue(engine, Qt::YAxis), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ZAxis", QScriptValue(engine, Qt::ZAxis), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, BGModetoScriptValue,	BGModefromScriptValue);
  widget.setProperty("TransparentMode", QScriptValue(engine, Qt::TransparentMode), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("OpaqueMode", QScriptValue(engine, Qt::OpaqueMode), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, BrushStyletoScriptValue,	BrushStylefromScriptValue);
  widget.setProperty("NoBrush", QScriptValue(engine, Qt::NoBrush), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SolidPattern", QScriptValue(engine, Qt::SolidPattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Dense1Pattern", QScriptValue(engine, Qt::Dense1Pattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Dense2Pattern", QScriptValue(engine, Qt::Dense2Pattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Dense3Pattern", QScriptValue(engine, Qt::Dense3Pattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Dense4Pattern", QScriptValue(engine, Qt::Dense4Pattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Dense5Pattern", QScriptValue(engine, Qt::Dense5Pattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Dense6Pattern", QScriptValue(engine, Qt::Dense6Pattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Dense7Pattern", QScriptValue(engine, Qt::Dense7Pattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("HorPattern", QScriptValue(engine, Qt::HorPattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("VerPattern", QScriptValue(engine, Qt::VerPattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CrossPattern", QScriptValue(engine, Qt::CrossPattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BDiagPattern", QScriptValue(engine, Qt::BDiagPattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("FDiagPattern", QScriptValue(engine, Qt::FDiagPattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DiagCrossPattern", QScriptValue(engine, Qt::DiagCrossPattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LinearGradientPattern", QScriptValue(engine, Qt::LinearGradientPattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ConicalGradientPattern", QScriptValue(engine, Qt::ConicalGradientPattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RadialGradientPattern", QScriptValue(engine, Qt::RadialGradientPattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TexturePattern", QScriptValue(engine, Qt::TexturePattern), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, CaseSensitivitytoScriptValue,	CaseSensitivityfromScriptValue);
  widget.setProperty("CaseInsensitive", QScriptValue(engine, Qt::CaseInsensitive), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CaseSensitive", QScriptValue(engine, Qt::CaseSensitive), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, CheckStatetoScriptValue,	CheckStatefromScriptValue);
  widget.setProperty("Unchecked", QScriptValue(engine, Qt::Unchecked), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PartiallyChecked", QScriptValue(engine, Qt::PartiallyChecked), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Checked", QScriptValue(engine, Qt::Checked), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ClipOperationtoScriptValue,	ClipOperationfromScriptValue);
  widget.setProperty("NoClip", QScriptValue(engine, Qt::NoClip), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ReplaceClip", QScriptValue(engine, Qt::ReplaceClip), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("IntersectClip", QScriptValue(engine, Qt::IntersectClip), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UniteClip", QScriptValue(engine, Qt::UniteClip), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ConnectionTypetoScriptValue,	ConnectionTypefromScriptValue);
  widget.setProperty("DirectConnection", QScriptValue(engine, Qt::DirectConnection), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("QueuedConnection", QScriptValue(engine, Qt::QueuedConnection), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BlockingQueuedConnection", QScriptValue(engine, Qt::BlockingQueuedConnection), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AutoConnection", QScriptValue(engine, Qt::AutoConnection), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ContextMenuPolicytoScriptValue,	ContextMenuPolicyfromScriptValue);
  widget.setProperty("NoContextMenu", QScriptValue(engine, Qt::NoContextMenu), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PreventContextMenu", QScriptValue(engine, Qt::PreventContextMenu), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DefaultContextMenu", QScriptValue(engine, Qt::DefaultContextMenu), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ActionsContextMenu", QScriptValue(engine, Qt::ActionsContextMenu), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CustomContextMenu", QScriptValue(engine, Qt::CustomContextMenu), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, CornertoScriptValue,	CornerfromScriptValue);
  widget.setProperty("TopLeftCorner", QScriptValue(engine, Qt::TopLeftCorner), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TopRightCorner", QScriptValue(engine, Qt::TopRightCorner), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BottomLeftCorner", QScriptValue(engine, Qt::BottomLeftCorner), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BottomRightCorner", QScriptValue(engine, Qt::BottomRightCorner), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, CursorShapetoScriptValue,	CursorShapefromScriptValue);
  widget.setProperty("ArrowCursor", QScriptValue(engine, Qt::ArrowCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UpArrowCursor", QScriptValue(engine, Qt::UpArrowCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CrossCursor", QScriptValue(engine, Qt::CrossCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WaitCursor", QScriptValue(engine, Qt::WaitCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("IBeamCursor", QScriptValue(engine, Qt::IBeamCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SizeVerCursor", QScriptValue(engine, Qt::SizeVerCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SizeHorCursor", QScriptValue(engine, Qt::SizeHorCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SizeBDiagCursor", QScriptValue(engine, Qt::SizeBDiagCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SizeFDiagCursor", QScriptValue(engine, Qt::SizeFDiagCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SizeAllCursor", QScriptValue(engine, Qt::SizeAllCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BlankCursor", QScriptValue(engine, Qt::BlankCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SplitVCursor", QScriptValue(engine, Qt::SplitVCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SplitHCursor", QScriptValue(engine, Qt::SplitHCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PointingHandCursor", QScriptValue(engine, Qt::PointingHandCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ForbiddenCursor", QScriptValue(engine, Qt::ForbiddenCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("OpenHandCursor", QScriptValue(engine, Qt::OpenHandCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ClosedHandCursor", QScriptValue(engine, Qt::ClosedHandCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WhatsThisCursor", QScriptValue(engine, Qt::WhatsThisCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BusyCursor", QScriptValue(engine, Qt::BusyCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BitmapCursor", QScriptValue(engine, Qt::BitmapCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, DateFormattoScriptValue,	DateFormatfromScriptValue);
  widget.setProperty("TextDate", QScriptValue(engine, Qt::TextDate), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ISODate", QScriptValue(engine, Qt::ISODate), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SystemLocaleShortDate", QScriptValue(engine, Qt::SystemLocaleShortDate), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SystemLocaleLongDate", QScriptValue(engine, Qt::SystemLocaleLongDate), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DefaultLocaleShortDate", QScriptValue(engine, Qt::DefaultLocaleShortDate), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DefaultLocaleLongDate", QScriptValue(engine, Qt::DefaultLocaleLongDate), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SystemLocaleDate", QScriptValue(engine, Qt::SystemLocaleDate), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LocaleDate", QScriptValue(engine, Qt::LocaleDate), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LocalDate", QScriptValue(engine, Qt::LocalDate), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, DayOfWeektoScriptValue,	DayOfWeekfromScriptValue);
  widget.setProperty("Monday", QScriptValue(engine, Qt::Monday), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Tuesday", QScriptValue(engine, Qt::Tuesday), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Wednesday", QScriptValue(engine, Qt::Wednesday), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Thursday", QScriptValue(engine, Qt::Thursday), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Friday", QScriptValue(engine, Qt::Friday), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Saturday", QScriptValue(engine, Qt::Saturday), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Sunday", QScriptValue(engine, Qt::Sunday), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, DockWidgetAreatoScriptValue,	DockWidgetAreafromScriptValue);
  widget.setProperty("LeftDockWidgetArea", QScriptValue(engine, Qt::LeftDockWidgetArea), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RightDockWidgetArea", QScriptValue(engine, Qt::RightDockWidgetArea), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TopDockWidgetArea", QScriptValue(engine, Qt::TopDockWidgetArea), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BottomDockWidgetArea", QScriptValue(engine, Qt::BottomDockWidgetArea), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AllDockWidgetAreas", QScriptValue(engine, Qt::AllDockWidgetAreas), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("NoDockWidgetArea", QScriptValue(engine, Qt::NoDockWidgetArea), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, DropActiontoScriptValue,	DropActionfromScriptValue);
  widget.setProperty("CopyAction", QScriptValue(engine, Qt::CopyAction), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MoveAction", QScriptValue(engine, Qt::MoveAction), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LinkAction", QScriptValue(engine, Qt::LinkAction), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ActionMask", QScriptValue(engine, Qt::ActionMask), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("IgnoreAction", QScriptValue(engine, Qt::IgnoreAction), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TargetMoveAction", QScriptValue(engine, Qt::TargetMoveAction), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, EventPrioritytoScriptValue,	EventPriorityfromScriptValue);
  widget.setProperty("HighEventPriority", QScriptValue(engine, Qt::HighEventPriority), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("NormalEventPriority", QScriptValue(engine, Qt::NormalEventPriority), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LowEventPriority", QScriptValue(engine, Qt::LowEventPriority), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, FillRuletoScriptValue,	FillRulefromScriptValue);
  widget.setProperty("OddEvenFill", QScriptValue(engine, Qt::OddEvenFill), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindingFill", QScriptValue(engine, Qt::WindingFill), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, FocusPolicytoScriptValue,	FocusPolicyfromScriptValue);
  widget.setProperty("TabFocus", QScriptValue(engine, Qt::TabFocus), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ClickFocus", QScriptValue(engine, Qt::ClickFocus), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("StrongFocus", QScriptValue(engine, Qt::StrongFocus), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WheelFocus", QScriptValue(engine, Qt::WheelFocus), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("NoFocus", QScriptValue(engine, Qt::NoFocus), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, FocusReasontoScriptValue,	FocusReasonfromScriptValue);
  widget.setProperty("MouseFocusReason", QScriptValue(engine, Qt::MouseFocusReason), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TabFocusReason", QScriptValue(engine, Qt::TabFocusReason), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BacktabFocusReason", QScriptValue(engine, Qt::BacktabFocusReason), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ActiveWindowFocusReason", QScriptValue(engine, Qt::ActiveWindowFocusReason), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PopupFocusReason", QScriptValue(engine, Qt::PopupFocusReason), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ShortcutFocusReason", QScriptValue(engine, Qt::ShortcutFocusReason), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MenuBarFocusReason", QScriptValue(engine, Qt::MenuBarFocusReason), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("OtherFocusReason", QScriptValue(engine, Qt::OtherFocusReason), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, GlobalColortoScriptValue,	GlobalColorfromScriptValue);
  widget.setProperty("white", QScriptValue(engine, Qt::white), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("black", QScriptValue(engine, Qt::black), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("red", QScriptValue(engine, Qt::red), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("darkRed", QScriptValue(engine, Qt::darkRed), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("green", QScriptValue(engine, Qt::green), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("darkGreen", QScriptValue(engine, Qt::darkGreen), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("blue", QScriptValue(engine, Qt::blue), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("darkBlue", QScriptValue(engine, Qt::darkBlue), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cyan", QScriptValue(engine, Qt::cyan), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("darkCyan", QScriptValue(engine, Qt::darkCyan), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("magenta", QScriptValue(engine, Qt::magenta), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("darkMagenta", QScriptValue(engine, Qt::darkMagenta), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("yellow", QScriptValue(engine, Qt::yellow), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("darkYellow", QScriptValue(engine, Qt::darkYellow), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("gray", QScriptValue(engine, Qt::gray), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("darkGray", QScriptValue(engine, Qt::darkGray), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("lightGray", QScriptValue(engine, Qt::lightGray), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("transparent", QScriptValue(engine, Qt::transparent), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("color0", QScriptValue(engine, Qt::color0), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("color1", QScriptValue(engine, Qt::color1), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, HitTestAccuracytoScriptValue,	HitTestAccuracyfromScriptValue);
  widget.setProperty("ExactHit", QScriptValue(engine, Qt::ExactHit), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("FuzzyHit", QScriptValue(engine, Qt::FuzzyHit), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ImageConversionFlagtoScriptValue,	ImageConversionFlagfromScriptValue);
  widget.setProperty("AutoColor", QScriptValue(engine, Qt::AutoColor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ColorOnly", QScriptValue(engine, Qt::ColorOnly), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MonoOnly", QScriptValue(engine, Qt::MonoOnly), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DiffuseDither", QScriptValue(engine, Qt::DiffuseDither), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("OrderedDither", QScriptValue(engine, Qt::OrderedDither), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ThresholdDither", QScriptValue(engine, Qt::ThresholdDither), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ThresholdAlphaDither", QScriptValue(engine, Qt::ThresholdAlphaDither), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("OrderedAlphaDither", QScriptValue(engine, Qt::OrderedAlphaDither), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DiffuseAlphaDither", QScriptValue(engine, Qt::DiffuseAlphaDither), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PreferDither", QScriptValue(engine, Qt::PreferDither), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AvoidDither", QScriptValue(engine, Qt::AvoidDither), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, InputMethodQuerytoScriptValue,	InputMethodQueryfromScriptValue);
  widget.setProperty("ImMicroFocus", QScriptValue(engine, Qt::ImMicroFocus), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ImFont", QScriptValue(engine, Qt::ImFont), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ImCursorPosition", QScriptValue(engine, Qt::ImCursorPosition), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ImSurroundingText", QScriptValue(engine, Qt::ImSurroundingText), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ImCurrentSelection", QScriptValue(engine, Qt::ImCurrentSelection), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ItemDataRoletoScriptValue,	ItemDataRolefromScriptValue);
  widget.setProperty("DisplayRole", QScriptValue(engine, Qt::DisplayRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DecorationRole", QScriptValue(engine, Qt::DecorationRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("EditRole", QScriptValue(engine, Qt::EditRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ToolTipRole", QScriptValue(engine, Qt::ToolTipRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("StatusTipRole", QScriptValue(engine, Qt::StatusTipRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WhatsThisRole", QScriptValue(engine, Qt::WhatsThisRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SizeHintRole", QScriptValue(engine, Qt::SizeHintRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("FontRole", QScriptValue(engine, Qt::FontRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextAlignmentRole", QScriptValue(engine, Qt::TextAlignmentRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BackgroundRole", QScriptValue(engine, Qt::BackgroundRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BackgroundColorRole", QScriptValue(engine, Qt::BackgroundColorRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ForegroundRole", QScriptValue(engine, Qt::ForegroundRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextColorRole", QScriptValue(engine, Qt::TextColorRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CheckStateRole", QScriptValue(engine, Qt::CheckStateRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AccessibleTextRole", QScriptValue(engine, Qt::AccessibleTextRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AccessibleDescriptionRole", QScriptValue(engine, Qt::AccessibleDescriptionRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UserRole", QScriptValue(engine, Qt::UserRole), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ItemFlagtoScriptValue,	ItemFlagfromScriptValue);
  widget.setProperty("NoItemFlags", QScriptValue(engine, Qt::NoItemFlags), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemIsSelectable", QScriptValue(engine, Qt::ItemIsSelectable), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemIsEditable", QScriptValue(engine, Qt::ItemIsEditable), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemIsDragEnabled", QScriptValue(engine, Qt::ItemIsDragEnabled), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemIsDropEnabled", QScriptValue(engine, Qt::ItemIsDropEnabled), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemIsUserCheckable", QScriptValue(engine, Qt::ItemIsUserCheckable), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemIsEnabled", QScriptValue(engine, Qt::ItemIsEnabled), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemIsTristate", QScriptValue(engine, Qt::ItemIsTristate), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ItemSelectionModetoScriptValue,	ItemSelectionModefromScriptValue);
  widget.setProperty("ContainsItemShape", QScriptValue(engine, Qt::ContainsItemShape), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("IntersectsItemShape", QScriptValue(engine, Qt::IntersectsItemShape), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ContainsItemBoundingRect", QScriptValue(engine, Qt::ContainsItemBoundingRect), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("IntersectsItemBoundingRect", QScriptValue(engine, Qt::IntersectsItemBoundingRect), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, KeytoScriptValue,	KeyfromScriptValue);
  widget.setProperty("Key_Escape", QScriptValue(engine, Qt::Key_Escape), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Tab", QScriptValue(engine, Qt::Key_Tab), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Backtab", QScriptValue(engine, Qt::Key_Backtab), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Backspace", QScriptValue(engine, Qt::Key_Backspace), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Return", QScriptValue(engine, Qt::Key_Return), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Enter", QScriptValue(engine, Qt::Key_Enter), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Insert", QScriptValue(engine, Qt::Key_Insert), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Delete", QScriptValue(engine, Qt::Key_Delete), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Pause", QScriptValue(engine, Qt::Key_Pause), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Print", QScriptValue(engine, Qt::Key_Print), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_SysReq", QScriptValue(engine, Qt::Key_SysReq), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Clear", QScriptValue(engine, Qt::Key_Clear), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Home", QScriptValue(engine, Qt::Key_Home), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_End", QScriptValue(engine, Qt::Key_End), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Left", QScriptValue(engine, Qt::Key_Left), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Up", QScriptValue(engine, Qt::Key_Up), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Right", QScriptValue(engine, Qt::Key_Right), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Down", QScriptValue(engine, Qt::Key_Down), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_PageUp", QScriptValue(engine, Qt::Key_PageUp), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_PageDown", QScriptValue(engine, Qt::Key_PageDown), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Shift", QScriptValue(engine, Qt::Key_Shift), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Control", QScriptValue(engine, Qt::Key_Control), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Meta", QScriptValue(engine, Qt::Key_Meta), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Alt", QScriptValue(engine, Qt::Key_Alt), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_AltGr", QScriptValue(engine, Qt::Key_AltGr), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_CapsLock", QScriptValue(engine, Qt::Key_CapsLock), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_NumLock", QScriptValue(engine, Qt::Key_NumLock), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_ScrollLock", QScriptValue(engine, Qt::Key_ScrollLock), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F1", QScriptValue(engine, Qt::Key_F1), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F2", QScriptValue(engine, Qt::Key_F2), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F3", QScriptValue(engine, Qt::Key_F3), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F4", QScriptValue(engine, Qt::Key_F4), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F5", QScriptValue(engine, Qt::Key_F5), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F6", QScriptValue(engine, Qt::Key_F6), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F7", QScriptValue(engine, Qt::Key_F7), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F8", QScriptValue(engine, Qt::Key_F8), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F9", QScriptValue(engine, Qt::Key_F9), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F10", QScriptValue(engine, Qt::Key_F10), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F11", QScriptValue(engine, Qt::Key_F11), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F12", QScriptValue(engine, Qt::Key_F12), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F13", QScriptValue(engine, Qt::Key_F13), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F14", QScriptValue(engine, Qt::Key_F14), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F15", QScriptValue(engine, Qt::Key_F15), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F16", QScriptValue(engine, Qt::Key_F16), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F17", QScriptValue(engine, Qt::Key_F17), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F18", QScriptValue(engine, Qt::Key_F18), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F19", QScriptValue(engine, Qt::Key_F19), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F20", QScriptValue(engine, Qt::Key_F20), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F21", QScriptValue(engine, Qt::Key_F21), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F22", QScriptValue(engine, Qt::Key_F22), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F23", QScriptValue(engine, Qt::Key_F23), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F24", QScriptValue(engine, Qt::Key_F24), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F25", QScriptValue(engine, Qt::Key_F25), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F26", QScriptValue(engine, Qt::Key_F26), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F27", QScriptValue(engine, Qt::Key_F27), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F28", QScriptValue(engine, Qt::Key_F28), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F29", QScriptValue(engine, Qt::Key_F29), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F30", QScriptValue(engine, Qt::Key_F30), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F31", QScriptValue(engine, Qt::Key_F31), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F32", QScriptValue(engine, Qt::Key_F32), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F33", QScriptValue(engine, Qt::Key_F33), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F34", QScriptValue(engine, Qt::Key_F34), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F35", QScriptValue(engine, Qt::Key_F35), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Super_L", QScriptValue(engine, Qt::Key_Super_L), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Super_R", QScriptValue(engine, Qt::Key_Super_R), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Menu", QScriptValue(engine, Qt::Key_Menu), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hyper_L", QScriptValue(engine, Qt::Key_Hyper_L), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hyper_R", QScriptValue(engine, Qt::Key_Hyper_R), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Help", QScriptValue(engine, Qt::Key_Help), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Direction_L", QScriptValue(engine, Qt::Key_Direction_L), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Direction_R", QScriptValue(engine, Qt::Key_Direction_R), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Space", QScriptValue(engine, Qt::Key_Space), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Any", QScriptValue(engine, Qt::Key_Any), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Exclam", QScriptValue(engine, Qt::Key_Exclam), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_QuoteDbl", QScriptValue(engine, Qt::Key_QuoteDbl), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_NumberSign", QScriptValue(engine, Qt::Key_NumberSign), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dollar", QScriptValue(engine, Qt::Key_Dollar), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Percent", QScriptValue(engine, Qt::Key_Percent), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Ampersand", QScriptValue(engine, Qt::Key_Ampersand), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Apostrophe", QScriptValue(engine, Qt::Key_Apostrophe), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_ParenLeft", QScriptValue(engine, Qt::Key_ParenLeft), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_ParenRight", QScriptValue(engine, Qt::Key_ParenRight), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Asterisk", QScriptValue(engine, Qt::Key_Asterisk), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Plus", QScriptValue(engine, Qt::Key_Plus), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Comma", QScriptValue(engine, Qt::Key_Comma), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Minus", QScriptValue(engine, Qt::Key_Minus), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Period", QScriptValue(engine, Qt::Key_Period), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Slash", QScriptValue(engine, Qt::Key_Slash), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_0", QScriptValue(engine, Qt::Key_0), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_1", QScriptValue(engine, Qt::Key_1), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_2", QScriptValue(engine, Qt::Key_2), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_3", QScriptValue(engine, Qt::Key_3), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_4", QScriptValue(engine, Qt::Key_4), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_5", QScriptValue(engine, Qt::Key_5), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_6", QScriptValue(engine, Qt::Key_6), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_7", QScriptValue(engine, Qt::Key_7), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_8", QScriptValue(engine, Qt::Key_8), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_9", QScriptValue(engine, Qt::Key_9), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Colon", QScriptValue(engine, Qt::Key_Colon), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Semicolon", QScriptValue(engine, Qt::Key_Semicolon), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Less", QScriptValue(engine, Qt::Key_Less), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Equal", QScriptValue(engine, Qt::Key_Equal), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Greater", QScriptValue(engine, Qt::Key_Greater), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Question", QScriptValue(engine, Qt::Key_Question), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_At", QScriptValue(engine, Qt::Key_At), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_A", QScriptValue(engine, Qt::Key_A), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_B", QScriptValue(engine, Qt::Key_B), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_C", QScriptValue(engine, Qt::Key_C), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_D", QScriptValue(engine, Qt::Key_D), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_E", QScriptValue(engine, Qt::Key_E), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_F", QScriptValue(engine, Qt::Key_F), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_G", QScriptValue(engine, Qt::Key_G), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_H", QScriptValue(engine, Qt::Key_H), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_I", QScriptValue(engine, Qt::Key_I), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_J", QScriptValue(engine, Qt::Key_J), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_K", QScriptValue(engine, Qt::Key_K), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_L", QScriptValue(engine, Qt::Key_L), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_M", QScriptValue(engine, Qt::Key_M), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_N", QScriptValue(engine, Qt::Key_N), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_O", QScriptValue(engine, Qt::Key_O), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_P", QScriptValue(engine, Qt::Key_P), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Q", QScriptValue(engine, Qt::Key_Q), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_R", QScriptValue(engine, Qt::Key_R), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_S", QScriptValue(engine, Qt::Key_S), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_T", QScriptValue(engine, Qt::Key_T), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_U", QScriptValue(engine, Qt::Key_U), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_V", QScriptValue(engine, Qt::Key_V), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_W", QScriptValue(engine, Qt::Key_W), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_X", QScriptValue(engine, Qt::Key_X), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Y", QScriptValue(engine, Qt::Key_Y), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Z", QScriptValue(engine, Qt::Key_Z), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_BracketLeft", QScriptValue(engine, Qt::Key_BracketLeft), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Backslash", QScriptValue(engine, Qt::Key_Backslash), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_BracketRight", QScriptValue(engine, Qt::Key_BracketRight), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_AsciiCircum", QScriptValue(engine, Qt::Key_AsciiCircum), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Underscore", QScriptValue(engine, Qt::Key_Underscore), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_QuoteLeft", QScriptValue(engine, Qt::Key_QuoteLeft), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_BraceLeft", QScriptValue(engine, Qt::Key_BraceLeft), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Bar", QScriptValue(engine, Qt::Key_Bar), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_BraceRight", QScriptValue(engine, Qt::Key_BraceRight), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_AsciiTilde", QScriptValue(engine, Qt::Key_AsciiTilde), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_nobreakspace", QScriptValue(engine, Qt::Key_nobreakspace), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_exclamdown", QScriptValue(engine, Qt::Key_exclamdown), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_cent", QScriptValue(engine, Qt::Key_cent), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_sterling", QScriptValue(engine, Qt::Key_sterling), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_currency", QScriptValue(engine, Qt::Key_currency), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_yen", QScriptValue(engine, Qt::Key_yen), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_brokenbar", QScriptValue(engine, Qt::Key_brokenbar), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_section", QScriptValue(engine, Qt::Key_section), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_diaeresis", QScriptValue(engine, Qt::Key_diaeresis), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_copyright", QScriptValue(engine, Qt::Key_copyright), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_ordfeminine", QScriptValue(engine, Qt::Key_ordfeminine), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_guillemotleft", QScriptValue(engine, Qt::Key_guillemotleft), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_notsign", QScriptValue(engine, Qt::Key_notsign), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_hyphen", QScriptValue(engine, Qt::Key_hyphen), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_registered", QScriptValue(engine, Qt::Key_registered), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_macron", QScriptValue(engine, Qt::Key_macron), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_degree", QScriptValue(engine, Qt::Key_degree), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_plusminus", QScriptValue(engine, Qt::Key_plusminus), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_twosuperior", QScriptValue(engine, Qt::Key_twosuperior), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_threesuperior", QScriptValue(engine, Qt::Key_threesuperior), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_acute", QScriptValue(engine, Qt::Key_acute), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_mu", QScriptValue(engine, Qt::Key_mu), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_paragraph", QScriptValue(engine, Qt::Key_paragraph), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_periodcentered", QScriptValue(engine, Qt::Key_periodcentered), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_cedilla", QScriptValue(engine, Qt::Key_cedilla), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_onesuperior", QScriptValue(engine, Qt::Key_onesuperior), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_masculine", QScriptValue(engine, Qt::Key_masculine), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_guillemotright", QScriptValue(engine, Qt::Key_guillemotright), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_onequarter", QScriptValue(engine, Qt::Key_onequarter), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_onehalf", QScriptValue(engine, Qt::Key_onehalf), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_threequarters", QScriptValue(engine, Qt::Key_threequarters), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_questiondown", QScriptValue(engine, Qt::Key_questiondown), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Agrave", QScriptValue(engine, Qt::Key_Agrave), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Aacute", QScriptValue(engine, Qt::Key_Aacute), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Acircumflex", QScriptValue(engine, Qt::Key_Acircumflex), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Atilde", QScriptValue(engine, Qt::Key_Atilde), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Adiaeresis", QScriptValue(engine, Qt::Key_Adiaeresis), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Aring", QScriptValue(engine, Qt::Key_Aring), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_AE", QScriptValue(engine, Qt::Key_AE), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Ccedilla", QScriptValue(engine, Qt::Key_Ccedilla), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Egrave", QScriptValue(engine, Qt::Key_Egrave), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Eacute", QScriptValue(engine, Qt::Key_Eacute), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Ecircumflex", QScriptValue(engine, Qt::Key_Ecircumflex), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Ediaeresis", QScriptValue(engine, Qt::Key_Ediaeresis), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Igrave", QScriptValue(engine, Qt::Key_Igrave), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Iacute", QScriptValue(engine, Qt::Key_Iacute), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Icircumflex", QScriptValue(engine, Qt::Key_Icircumflex), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Idiaeresis", QScriptValue(engine, Qt::Key_Idiaeresis), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_ETH", QScriptValue(engine, Qt::Key_ETH), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Ntilde", QScriptValue(engine, Qt::Key_Ntilde), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Ograve", QScriptValue(engine, Qt::Key_Ograve), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Oacute", QScriptValue(engine, Qt::Key_Oacute), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Ocircumflex", QScriptValue(engine, Qt::Key_Ocircumflex), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Otilde", QScriptValue(engine, Qt::Key_Otilde), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Odiaeresis", QScriptValue(engine, Qt::Key_Odiaeresis), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_multiply", QScriptValue(engine, Qt::Key_multiply), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Ooblique", QScriptValue(engine, Qt::Key_Ooblique), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Ugrave", QScriptValue(engine, Qt::Key_Ugrave), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Uacute", QScriptValue(engine, Qt::Key_Uacute), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Ucircumflex", QScriptValue(engine, Qt::Key_Ucircumflex), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Udiaeresis", QScriptValue(engine, Qt::Key_Udiaeresis), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Yacute", QScriptValue(engine, Qt::Key_Yacute), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_THORN", QScriptValue(engine, Qt::Key_THORN), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_ssharp", QScriptValue(engine, Qt::Key_ssharp), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_division", QScriptValue(engine, Qt::Key_division), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_ydiaeresis", QScriptValue(engine, Qt::Key_ydiaeresis), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Multi_key", QScriptValue(engine, Qt::Key_Multi_key), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Codeinput", QScriptValue(engine, Qt::Key_Codeinput), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_SingleCandidate", QScriptValue(engine, Qt::Key_SingleCandidate), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_MultipleCandidate", QScriptValue(engine, Qt::Key_MultipleCandidate), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_PreviousCandidate", QScriptValue(engine, Qt::Key_PreviousCandidate), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Mode_switch", QScriptValue(engine, Qt::Key_Mode_switch), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Kanji", QScriptValue(engine, Qt::Key_Kanji), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Muhenkan", QScriptValue(engine, Qt::Key_Muhenkan), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Henkan", QScriptValue(engine, Qt::Key_Henkan), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Romaji", QScriptValue(engine, Qt::Key_Romaji), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hiragana", QScriptValue(engine, Qt::Key_Hiragana), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Katakana", QScriptValue(engine, Qt::Key_Katakana), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hiragana_Katakana", QScriptValue(engine, Qt::Key_Hiragana_Katakana), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Zenkaku", QScriptValue(engine, Qt::Key_Zenkaku), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hankaku", QScriptValue(engine, Qt::Key_Hankaku), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Zenkaku_Hankaku", QScriptValue(engine, Qt::Key_Zenkaku_Hankaku), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Touroku", QScriptValue(engine, Qt::Key_Touroku), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Massyo", QScriptValue(engine, Qt::Key_Massyo), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Kana_Lock", QScriptValue(engine, Qt::Key_Kana_Lock), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Kana_Shift", QScriptValue(engine, Qt::Key_Kana_Shift), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Eisu_Shift", QScriptValue(engine, Qt::Key_Eisu_Shift), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Eisu_toggle", QScriptValue(engine, Qt::Key_Eisu_toggle), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hangul", QScriptValue(engine, Qt::Key_Hangul), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hangul_Start", QScriptValue(engine, Qt::Key_Hangul_Start), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hangul_End", QScriptValue(engine, Qt::Key_Hangul_End), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hangul_Hanja", QScriptValue(engine, Qt::Key_Hangul_Hanja), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hangul_Jamo", QScriptValue(engine, Qt::Key_Hangul_Jamo), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hangul_Romaja", QScriptValue(engine, Qt::Key_Hangul_Romaja), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hangul_Jeonja", QScriptValue(engine, Qt::Key_Hangul_Jeonja), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hangul_Banja", QScriptValue(engine, Qt::Key_Hangul_Banja), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hangul_PreHanja", QScriptValue(engine, Qt::Key_Hangul_PreHanja), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hangul_PostHanja", QScriptValue(engine, Qt::Key_Hangul_PostHanja), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hangul_Special", QScriptValue(engine, Qt::Key_Hangul_Special), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Grave", QScriptValue(engine, Qt::Key_Dead_Grave), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Acute", QScriptValue(engine, Qt::Key_Dead_Acute), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Circumflex", QScriptValue(engine, Qt::Key_Dead_Circumflex), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Tilde", QScriptValue(engine, Qt::Key_Dead_Tilde), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Macron", QScriptValue(engine, Qt::Key_Dead_Macron), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Breve", QScriptValue(engine, Qt::Key_Dead_Breve), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Abovedot", QScriptValue(engine, Qt::Key_Dead_Abovedot), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Diaeresis", QScriptValue(engine, Qt::Key_Dead_Diaeresis), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Abovering", QScriptValue(engine, Qt::Key_Dead_Abovering), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Doubleacute", QScriptValue(engine, Qt::Key_Dead_Doubleacute), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Caron", QScriptValue(engine, Qt::Key_Dead_Caron), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Cedilla", QScriptValue(engine, Qt::Key_Dead_Cedilla), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Ogonek", QScriptValue(engine, Qt::Key_Dead_Ogonek), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Iota", QScriptValue(engine, Qt::Key_Dead_Iota), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Voiced_Sound", QScriptValue(engine, Qt::Key_Dead_Voiced_Sound), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Semivoiced_Sound", QScriptValue(engine, Qt::Key_Dead_Semivoiced_Sound), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Belowdot", QScriptValue(engine, Qt::Key_Dead_Belowdot), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Hook", QScriptValue(engine, Qt::Key_Dead_Hook), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Dead_Horn", QScriptValue(engine, Qt::Key_Dead_Horn), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Back", QScriptValue(engine, Qt::Key_Back), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Forward", QScriptValue(engine, Qt::Key_Forward), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Stop", QScriptValue(engine, Qt::Key_Stop), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Refresh", QScriptValue(engine, Qt::Key_Refresh), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_VolumeDown", QScriptValue(engine, Qt::Key_VolumeDown), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_VolumeMute", QScriptValue(engine, Qt::Key_VolumeMute), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_VolumeUp", QScriptValue(engine, Qt::Key_VolumeUp), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_BassBoost", QScriptValue(engine, Qt::Key_BassBoost), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_BassUp", QScriptValue(engine, Qt::Key_BassUp), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_BassDown", QScriptValue(engine, Qt::Key_BassDown), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_TrebleUp", QScriptValue(engine, Qt::Key_TrebleUp), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_TrebleDown", QScriptValue(engine, Qt::Key_TrebleDown), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_MediaPlay", QScriptValue(engine, Qt::Key_MediaPlay), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_MediaStop", QScriptValue(engine, Qt::Key_MediaStop), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_MediaPrevious", QScriptValue(engine, Qt::Key_MediaPrevious), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_MediaNext", QScriptValue(engine, Qt::Key_MediaNext), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_MediaRecord", QScriptValue(engine, Qt::Key_MediaRecord), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_HomePage", QScriptValue(engine, Qt::Key_HomePage), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Favorites", QScriptValue(engine, Qt::Key_Favorites), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Search", QScriptValue(engine, Qt::Key_Search), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Standby", QScriptValue(engine, Qt::Key_Standby), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_OpenUrl", QScriptValue(engine, Qt::Key_OpenUrl), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_LaunchMail", QScriptValue(engine, Qt::Key_LaunchMail), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_LaunchMedia", QScriptValue(engine, Qt::Key_LaunchMedia), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Launch0", QScriptValue(engine, Qt::Key_Launch0), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Launch1", QScriptValue(engine, Qt::Key_Launch1), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Launch2", QScriptValue(engine, Qt::Key_Launch2), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Launch3", QScriptValue(engine, Qt::Key_Launch3), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Launch4", QScriptValue(engine, Qt::Key_Launch4), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Launch5", QScriptValue(engine, Qt::Key_Launch5), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Launch6", QScriptValue(engine, Qt::Key_Launch6), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Launch7", QScriptValue(engine, Qt::Key_Launch7), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Launch8", QScriptValue(engine, Qt::Key_Launch8), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Launch9", QScriptValue(engine, Qt::Key_Launch9), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_LaunchA", QScriptValue(engine, Qt::Key_LaunchA), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_LaunchB", QScriptValue(engine, Qt::Key_LaunchB), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_LaunchC", QScriptValue(engine, Qt::Key_LaunchC), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_LaunchD", QScriptValue(engine, Qt::Key_LaunchD), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_LaunchE", QScriptValue(engine, Qt::Key_LaunchE), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_LaunchF", QScriptValue(engine, Qt::Key_LaunchF), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_MediaLast", QScriptValue(engine, Qt::Key_MediaLast), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_unknown", QScriptValue(engine, Qt::Key_unknown), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Call", QScriptValue(engine, Qt::Key_Call), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Context1", QScriptValue(engine, Qt::Key_Context1), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Context2", QScriptValue(engine, Qt::Key_Context2), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Context3", QScriptValue(engine, Qt::Key_Context3), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Context4", QScriptValue(engine, Qt::Key_Context4), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Flip", QScriptValue(engine, Qt::Key_Flip), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Hangup", QScriptValue(engine, Qt::Key_Hangup), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_No", QScriptValue(engine, Qt::Key_No), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Select", QScriptValue(engine, Qt::Key_Select), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Yes", QScriptValue(engine, Qt::Key_Yes), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Execute", QScriptValue(engine, Qt::Key_Execute), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Printer", QScriptValue(engine, Qt::Key_Printer), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Play", QScriptValue(engine, Qt::Key_Play), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Sleep", QScriptValue(engine, Qt::Key_Sleep), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Zoom", QScriptValue(engine, Qt::Key_Zoom), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Key_Cancel", QScriptValue(engine, Qt::Key_Cancel), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, KeyboardModifiertoScriptValue,	KeyboardModifierfromScriptValue);
  widget.setProperty("NoModifier", QScriptValue(engine, Qt::NoModifier), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ShiftModifier", QScriptValue(engine, Qt::ShiftModifier), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ControlModifier", QScriptValue(engine, Qt::ControlModifier), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AltModifier", QScriptValue(engine, Qt::AltModifier), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MetaModifier", QScriptValue(engine, Qt::MetaModifier), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("KeypadModifier", QScriptValue(engine, Qt::KeypadModifier), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("GroupSwitchModifier", QScriptValue(engine, Qt::GroupSwitchModifier), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, LayoutDirectiontoScriptValue,	LayoutDirectionfromScriptValue);
  widget.setProperty("LeftToRight", QScriptValue(engine, Qt::LeftToRight), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RightToLeft", QScriptValue(engine, Qt::RightToLeft), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, MaskModetoScriptValue,	MaskModefromScriptValue);
  widget.setProperty("MaskInColor", QScriptValue(engine, Qt::MaskInColor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MaskOutColor", QScriptValue(engine, Qt::MaskOutColor), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, MatchFlagtoScriptValue, MatchFlagfromScriptValue);
  qScriptRegisterMetaType(engine, MatchFlagstoScriptValue,MatchFlagsfromScriptValue);
  widget.setProperty("MatchExactly", QScriptValue(engine, Qt::MatchExactly), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MatchFixedString", QScriptValue(engine, Qt::MatchFixedString), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MatchContains", QScriptValue(engine, Qt::MatchContains), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MatchStartsWith", QScriptValue(engine, Qt::MatchStartsWith), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MatchEndsWith", QScriptValue(engine, Qt::MatchEndsWith), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MatchCaseSensitive", QScriptValue(engine, Qt::MatchCaseSensitive), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MatchRegExp", QScriptValue(engine, Qt::MatchRegExp), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MatchWildcard", QScriptValue(engine, Qt::MatchWildcard), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MatchWrap", QScriptValue(engine, Qt::MatchWrap), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MatchRecursive", QScriptValue(engine, Qt::MatchRecursive), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ModifiertoScriptValue,	ModifierfromScriptValue);
  widget.setProperty("SHIFT", QScriptValue(engine, Qt::SHIFT), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("META", QScriptValue(engine, Qt::META), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CTRL", QScriptValue(engine, Qt::CTRL), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ALT", QScriptValue(engine, Qt::ALT), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UNICODE_ACCEL", QScriptValue(engine, Qt::UNICODE_ACCEL), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, MouseButtontoScriptValue,	MouseButtonfromScriptValue);
  widget.setProperty("NoButton", QScriptValue(engine, Qt::NoButton), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LeftButton", QScriptValue(engine, Qt::LeftButton), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RightButton", QScriptValue(engine, Qt::RightButton), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MidButton", QScriptValue(engine, Qt::MidButton), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("XButton1", QScriptValue(engine, Qt::XButton1), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("XButton2", QScriptValue(engine, Qt::XButton2), QScriptValue::ReadOnly | QScriptValue::Undeletable);

/* Designer problem -- this line causes problems with the embedded designer
                       and testing has shown that it is not required as the conversion
                       appear to work correctly without it.
  qScriptRegisterMetaType(engine, OrientationtoScriptValue,	OrientationfromScriptValue);
*/
  widget.setProperty("Horizontal", QScriptValue(engine, Qt::Horizontal), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Vertical", QScriptValue(engine, Qt::Vertical), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, PenCapStyletoScriptValue,	PenCapStylefromScriptValue);
  widget.setProperty("SquareCap", QScriptValue(engine, Qt::SquareCap), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("FlatCap", QScriptValue(engine, Qt::FlatCap), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SquareCap", QScriptValue(engine, Qt::SquareCap), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RoundCap", QScriptValue(engine, Qt::RoundCap), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, PenJoinStyletoScriptValue,	PenJoinStylefromScriptValue);
  widget.setProperty("BevelJoin", QScriptValue(engine, Qt::BevelJoin), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MiterJoin", QScriptValue(engine, Qt::MiterJoin), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BevelJoin", QScriptValue(engine, Qt::BevelJoin), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RoundJoin", QScriptValue(engine, Qt::RoundJoin), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SvgMiterJoin", QScriptValue(engine, Qt::SvgMiterJoin), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, PenStyletoScriptValue,	PenStylefromScriptValue);
  widget.setProperty("SolidLine", QScriptValue(engine, Qt::SolidLine), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DashDotLine", QScriptValue(engine, Qt::DashDotLine), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("NoPen", QScriptValue(engine, Qt::NoPen), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SolidLine", QScriptValue(engine, Qt::SolidLine), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DashLine", QScriptValue(engine, Qt::DashLine), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DotLine", QScriptValue(engine, Qt::DotLine), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DashDotLine", QScriptValue(engine, Qt::DashDotLine), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DashDotDotLine", QScriptValue(engine, Qt::DashDotDotLine), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CustomDashLine", QScriptValue(engine, Qt::CustomDashLine), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ScrollBarPolicytoScriptValue,	ScrollBarPolicyfromScriptValue);
  widget.setProperty("ScrollBarAsNeeded", QScriptValue(engine, Qt::ScrollBarAsNeeded), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ScrollBarAlwaysOff", QScriptValue(engine, Qt::ScrollBarAlwaysOff), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ScrollBarAlwaysOn", QScriptValue(engine, Qt::ScrollBarAlwaysOn), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ShortcutContexttoScriptValue,	ShortcutContextfromScriptValue);
  widget.setProperty("WidgetShortcut", QScriptValue(engine, Qt::WidgetShortcut), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WidgetWithChildrenShortcut", QScriptValue(engine, Qt::WidgetWithChildrenShortcut), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowShortcut", QScriptValue(engine, Qt::WindowShortcut), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ApplicationShortcut", QScriptValue(engine, Qt::ApplicationShortcut), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, SizeHinttoScriptValue,	SizeHintfromScriptValue);
  widget.setProperty("MinimumSize", QScriptValue(engine, Qt::MinimumSize), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PreferredSize", QScriptValue(engine, Qt::PreferredSize), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MaximumSize", QScriptValue(engine, Qt::MaximumSize), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MinimumDescent", QScriptValue(engine, Qt::MinimumDescent), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, SizeModetoScriptValue,	SizeModefromScriptValue);
  widget.setProperty("AbsoluteSize", QScriptValue(engine, Qt::AbsoluteSize), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RelativeSize", QScriptValue(engine, Qt::RelativeSize), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, SortOrdertoScriptValue,	SortOrderfromScriptValue);
  widget.setProperty("AscendingOrder", QScriptValue(engine, Qt::AscendingOrder), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("DescendingOrder", QScriptValue(engine, Qt::DescendingOrder), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, TextElideModetoScriptValue,	TextElideModefromScriptValue);
  widget.setProperty("ElideLeft", QScriptValue(engine, Qt::ElideLeft), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ElideRight", QScriptValue(engine, Qt::ElideRight), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ElideMiddle", QScriptValue(engine, Qt::ElideMiddle), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ElideNone", QScriptValue(engine, Qt::ElideNone), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ElideMiddle", QScriptValue(engine, Qt::ElideMiddle), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, TextFlagtoScriptValue,	TextFlagfromScriptValue);
  widget.setProperty("TextSingleLine", QScriptValue(engine, Qt::TextSingleLine), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextDontClip", QScriptValue(engine, Qt::TextDontClip), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextExpandTabs", QScriptValue(engine, Qt::TextExpandTabs), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextShowMnemonic", QScriptValue(engine, Qt::TextShowMnemonic), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextWordWrap", QScriptValue(engine, Qt::TextWordWrap), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextWrapAnywhere", QScriptValue(engine, Qt::TextWrapAnywhere), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextHideMnemonic", QScriptValue(engine, Qt::TextHideMnemonic), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextDontPrint", QScriptValue(engine, Qt::TextDontPrint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextIncludeTrailingSpaces", QScriptValue(engine, Qt::TextIncludeTrailingSpaces), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextJustificationForced", QScriptValue(engine, Qt::TextJustificationForced), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, TextFormattoScriptValue,	TextFormatfromScriptValue);
  widget.setProperty("PlainText", QScriptValue(engine, Qt::PlainText), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RichText", QScriptValue(engine, Qt::RichText), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AutoText", QScriptValue(engine, Qt::AutoText), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LogText", QScriptValue(engine, Qt::LogText), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, TextInteractionFlagtoScriptValue,	TextInteractionFlagfromScriptValue);
  widget.setProperty("NoTextInteraction", QScriptValue(engine, Qt::NoTextInteraction), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextSelectableByMouse", QScriptValue(engine, Qt::TextSelectableByMouse), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextSelectableByKeyboard", QScriptValue(engine, Qt::TextSelectableByKeyboard), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LinksAccessibleByMouse", QScriptValue(engine, Qt::LinksAccessibleByMouse), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LinksAccessibleByKeyboard", QScriptValue(engine, Qt::LinksAccessibleByKeyboard), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextEditable", QScriptValue(engine, Qt::TextEditable), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextEditorInteraction", QScriptValue(engine, Qt::TextEditorInteraction), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TextBrowserInteraction", QScriptValue(engine, Qt::TextBrowserInteraction), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, TimeSpectoScriptValue,	TimeSpecfromScriptValue);
  widget.setProperty("LocalTime", QScriptValue(engine, Qt::LocalTime), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UTC", QScriptValue(engine, Qt::UTC), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("OffsetFromUTC", QScriptValue(engine, Qt::OffsetFromUTC), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ToolBarAreatoScriptValue,	ToolBarAreafromScriptValue);
  widget.setProperty("LeftToolBarArea", QScriptValue(engine, Qt::LeftToolBarArea), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RightToolBarArea", QScriptValue(engine, Qt::RightToolBarArea), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TopToolBarArea", QScriptValue(engine, Qt::TopToolBarArea), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BottomToolBarArea", QScriptValue(engine, Qt::BottomToolBarArea), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AllToolBarAreas", QScriptValue(engine, Qt::AllToolBarAreas), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("NoToolBarArea", QScriptValue(engine, Qt::NoToolBarArea), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, ToolButtonStyletoScriptValue,	ToolButtonStylefromScriptValue);
  widget.setProperty("ToolButtonIconOnly", QScriptValue(engine, Qt::ToolButtonIconOnly), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ToolButtonTextOnly", QScriptValue(engine, Qt::ToolButtonTextOnly), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ToolButtonTextBesideIcon", QScriptValue(engine, Qt::ToolButtonTextBesideIcon), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ToolButtonTextUnderIcon", QScriptValue(engine, Qt::ToolButtonTextUnderIcon), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, TransformationModetoScriptValue,	TransformationModefromScriptValue);
  widget.setProperty("FastTransformation", QScriptValue(engine, Qt::FastTransformation), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SmoothTransformation", QScriptValue(engine, Qt::SmoothTransformation), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, UIEffecttoScriptValue,	UIEffectfromScriptValue);
  widget.setProperty("UI_AnimateMenu", QScriptValue(engine, Qt::UI_AnimateMenu), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UI_FadeMenu", QScriptValue(engine, Qt::UI_FadeMenu), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UI_AnimateCombo", QScriptValue(engine, Qt::UI_AnimateCombo), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UI_AnimateTooltip", QScriptValue(engine, Qt::UI_AnimateTooltip), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UI_FadeTooltip", QScriptValue(engine, Qt::UI_FadeTooltip), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UI_AnimateToolBox", QScriptValue(engine, Qt::UI_AnimateToolBox), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, WhiteSpaceModetoScriptValue,	WhiteSpaceModefromScriptValue);
  widget.setProperty("WhiteSpaceNormal", QScriptValue(engine, Qt::WhiteSpaceNormal), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WhiteSpacePre", QScriptValue(engine, Qt::WhiteSpacePre), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WhiteSpaceNoWrap", QScriptValue(engine, Qt::WhiteSpaceNoWrap), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, WidgetAttributetoScriptValue,	WidgetAttributefromScriptValue);
  widget.setProperty("WA_AcceptDrops", QScriptValue(engine, Qt::WA_AcceptDrops), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_AlwaysShowToolTips", QScriptValue(engine, Qt::WA_AlwaysShowToolTips), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_ContentsPropagated", QScriptValue(engine, Qt::WA_ContentsPropagated), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_CustomWhatsThis", QScriptValue(engine, Qt::WA_CustomWhatsThis), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_DeleteOnClose", QScriptValue(engine, Qt::WA_DeleteOnClose), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_Disabled", QScriptValue(engine, Qt::WA_Disabled), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_ForceDisabled", QScriptValue(engine, Qt::WA_ForceDisabled), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_ForceUpdatesDisabled", QScriptValue(engine, Qt::WA_ForceUpdatesDisabled), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_GroupLeader", QScriptValue(engine, Qt::WA_GroupLeader), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_Hover", QScriptValue(engine, Qt::WA_Hover), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_InputMethodEnabled", QScriptValue(engine, Qt::WA_InputMethodEnabled), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_KeyboardFocusChange", QScriptValue(engine, Qt::WA_KeyboardFocusChange), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_KeyCompression", QScriptValue(engine, Qt::WA_KeyCompression), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_LayoutOnEntireRect", QScriptValue(engine, Qt::WA_LayoutOnEntireRect), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_LayoutUsesWidgetRect", QScriptValue(engine, Qt::WA_LayoutUsesWidgetRect), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_MacNoClickThrough", QScriptValue(engine, Qt::WA_MacNoClickThrough), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_MacOpaqueSizeGrip", QScriptValue(engine, Qt::WA_MacOpaqueSizeGrip), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_MacShowFocusRect", QScriptValue(engine, Qt::WA_MacShowFocusRect), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_MacNormalSize", QScriptValue(engine, Qt::WA_MacNormalSize), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_MacSmallSize", QScriptValue(engine, Qt::WA_MacSmallSize), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_MacMiniSize", QScriptValue(engine, Qt::WA_MacMiniSize), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_MacVariableSize", QScriptValue(engine, Qt::WA_MacVariableSize), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_MacBrushedMetal", QScriptValue(engine, Qt::WA_MacBrushedMetal), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_Mapped", QScriptValue(engine, Qt::WA_Mapped), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_MouseNoMask", QScriptValue(engine, Qt::WA_MouseNoMask), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_MouseTracking", QScriptValue(engine, Qt::WA_MouseTracking), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_Moved", QScriptValue(engine, Qt::WA_Moved), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_MSWindowsUseDirect3D", QScriptValue(engine, Qt::WA_MSWindowsUseDirect3D), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_NoBackground", QScriptValue(engine, Qt::WA_NoBackground), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_NoChildEventsForParent", QScriptValue(engine, Qt::WA_NoChildEventsForParent), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_NoChildEventsFromChildren", QScriptValue(engine, Qt::WA_NoChildEventsFromChildren), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_NoMouseReplay", QScriptValue(engine, Qt::WA_NoMouseReplay), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_NoMousePropagation", QScriptValue(engine, Qt::WA_NoMousePropagation), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_TransparentForMouseEvents", QScriptValue(engine, Qt::WA_TransparentForMouseEvents), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_NoSystemBackground", QScriptValue(engine, Qt::WA_NoSystemBackground), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_OpaquePaintEvent", QScriptValue(engine, Qt::WA_OpaquePaintEvent), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_OutsideWSRange", QScriptValue(engine, Qt::WA_OutsideWSRange), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_PaintOnScreen", QScriptValue(engine, Qt::WA_PaintOnScreen), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_PaintOutsidePaintEvent", QScriptValue(engine, Qt::WA_PaintOutsidePaintEvent), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_PaintUnclipped", QScriptValue(engine, Qt::WA_PaintUnclipped), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_PendingMoveEvent", QScriptValue(engine, Qt::WA_PendingMoveEvent), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_PendingResizeEvent", QScriptValue(engine, Qt::WA_PendingResizeEvent), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_QuitOnClose", QScriptValue(engine, Qt::WA_QuitOnClose), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_Resized", QScriptValue(engine, Qt::WA_Resized), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_RightToLeft", QScriptValue(engine, Qt::WA_RightToLeft), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_SetCursor", QScriptValue(engine, Qt::WA_SetCursor), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_SetFont", QScriptValue(engine, Qt::WA_SetFont), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_SetPalette", QScriptValue(engine, Qt::WA_SetPalette), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_SetStyle", QScriptValue(engine, Qt::WA_SetStyle), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_ShowModal", QScriptValue(engine, Qt::WA_ShowModal), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_StaticContents", QScriptValue(engine, Qt::WA_StaticContents), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_StyleSheet", QScriptValue(engine, Qt::WA_StyleSheet), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_TranslucentBackground", QScriptValue(engine, Qt::WA_TranslucentBackground), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_UnderMouse", QScriptValue(engine, Qt::WA_UnderMouse), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_UpdatesDisabled", QScriptValue(engine, Qt::WA_UpdatesDisabled), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_WindowModified", QScriptValue(engine, Qt::WA_WindowModified), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_WindowPropagation", QScriptValue(engine, Qt::WA_WindowPropagation), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_MacAlwaysShowToolWindow", QScriptValue(engine, Qt::WA_MacAlwaysShowToolWindow), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_SetLocale", QScriptValue(engine, Qt::WA_SetLocale), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_StyledBackground", QScriptValue(engine, Qt::WA_StyledBackground), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_ShowWithoutActivating", QScriptValue(engine, Qt::WA_ShowWithoutActivating), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_NativeWindow", QScriptValue(engine, Qt::WA_NativeWindow), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_DontCreateNativeAncestors", QScriptValue(engine, Qt::WA_DontCreateNativeAncestors), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_X11NetWmWindowTypeDesktop", QScriptValue(engine, Qt::WA_X11NetWmWindowTypeDesktop), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_X11NetWmWindowTypeDock", QScriptValue(engine, Qt::WA_X11NetWmWindowTypeDock), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_X11NetWmWindowTypeToolBar", QScriptValue(engine, Qt::WA_X11NetWmWindowTypeToolBar), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_X11NetWmWindowTypeMenu", QScriptValue(engine, Qt::WA_X11NetWmWindowTypeMenu), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_X11NetWmWindowTypeUtility", QScriptValue(engine, Qt::WA_X11NetWmWindowTypeUtility), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_X11NetWmWindowTypeSplash", QScriptValue(engine, Qt::WA_X11NetWmWindowTypeSplash), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_X11NetWmWindowTypeDialog", QScriptValue(engine, Qt::WA_X11NetWmWindowTypeDialog), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_X11NetWmWindowTypeDropDownMenu", QScriptValue(engine, Qt::WA_X11NetWmWindowTypeDropDownMenu), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_X11NetWmWindowTypePopupMenu", QScriptValue(engine, Qt::WA_X11NetWmWindowTypePopupMenu), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_X11NetWmWindowTypeToolTip", QScriptValue(engine, Qt::WA_X11NetWmWindowTypeToolTip), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_X11NetWmWindowTypeNotification", QScriptValue(engine, Qt::WA_X11NetWmWindowTypeNotification), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_X11NetWmWindowTypeCombo", QScriptValue(engine, Qt::WA_X11NetWmWindowTypeCombo), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_X11NetWmWindowTypeDND", QScriptValue(engine, Qt::WA_X11NetWmWindowTypeDND), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WA_MacFrameworkScaled", QScriptValue(engine, Qt::WA_MacFrameworkScaled), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, WindowFrameSectiontoScriptValue,	WindowFrameSectionfromScriptValue);
  widget.setProperty("NoSection", QScriptValue(engine, Qt::NoSection), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LeftSection", QScriptValue(engine, Qt::LeftSection), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TopLeftSection", QScriptValue(engine, Qt::TopLeftSection), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TopSection", QScriptValue(engine, Qt::TopSection), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TopRightSection", QScriptValue(engine, Qt::TopRightSection), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RightSection", QScriptValue(engine, Qt::RightSection), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BottomRightSection", QScriptValue(engine, Qt::BottomRightSection), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BottomSection", QScriptValue(engine, Qt::BottomSection), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BottomLeftSection", QScriptValue(engine, Qt::BottomLeftSection), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TitleBarArea", QScriptValue(engine, Qt::TitleBarArea), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, WindowModalitytoScriptValue,	WindowModalityfromScriptValue);
  widget.setProperty("NonModal", QScriptValue(engine, Qt::NonModal), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowModal", QScriptValue(engine, Qt::WindowModal), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ApplicationModal", QScriptValue(engine, Qt::ApplicationModal), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, WindowStatetoScriptValue,	WindowStatefromScriptValue);
  widget.setProperty("WindowNoState", QScriptValue(engine, Qt::WindowNoState), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowMinimized", QScriptValue(engine, Qt::WindowMinimized), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowMaximized", QScriptValue(engine, Qt::WindowMaximized), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowFullScreen", QScriptValue(engine, Qt::WindowFullScreen), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowActive", QScriptValue(engine, Qt::WindowActive), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  qScriptRegisterMetaType(engine, WindowTypetoScriptValue,	WindowTypefromScriptValue);
  widget.setProperty("Widget", QScriptValue(engine, Qt::Widget), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Window", QScriptValue(engine, Qt::Window), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Dialog", QScriptValue(engine, Qt::Dialog), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Sheet", QScriptValue(engine, Qt::Sheet), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Drawer", QScriptValue(engine, Qt::Drawer), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Popup", QScriptValue(engine, Qt::Popup), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Tool", QScriptValue(engine, Qt::Tool), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ToolTip", QScriptValue(engine, Qt::ToolTip), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SplashScreen", QScriptValue(engine, Qt::SplashScreen), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Desktop", QScriptValue(engine, Qt::Desktop), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SubWindow", QScriptValue(engine, Qt::SubWindow), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MSWindowsFixedSizeDialogHint", QScriptValue(engine, Qt::MSWindowsFixedSizeDialogHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MSWindowsOwnDC", QScriptValue(engine, Qt::MSWindowsOwnDC), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("X11BypassWindowManagerHint", QScriptValue(engine, Qt::X11BypassWindowManagerHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("FramelessWindowHint", QScriptValue(engine, Qt::FramelessWindowHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CustomizeWindowHint", QScriptValue(engine, Qt::CustomizeWindowHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowTitleHint", QScriptValue(engine, Qt::WindowTitleHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowSystemMenuHint", QScriptValue(engine, Qt::WindowSystemMenuHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowMinimizeButtonHint", QScriptValue(engine, Qt::WindowMinimizeButtonHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowMaximizeButtonHint", QScriptValue(engine, Qt::WindowMaximizeButtonHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowMinMaxButtonsHint", QScriptValue(engine, Qt::WindowMinMaxButtonsHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowCloseButtonHint", QScriptValue(engine, Qt::WindowCloseButtonHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowContextHelpButtonHint", QScriptValue(engine, Qt::WindowContextHelpButtonHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("MacWindowToolBarButtonHint", QScriptValue(engine, Qt::MacWindowToolBarButtonHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BypassGraphicsProxyWidget", QScriptValue(engine, Qt::BypassGraphicsProxyWidget), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowShadeButtonHint", QScriptValue(engine, Qt::WindowShadeButtonHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowStaysOnTopHint", QScriptValue(engine, Qt::WindowStaysOnTopHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowStaysOnBottomHint", QScriptValue(engine, Qt::WindowStaysOnBottomHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowOkButtonHint", QScriptValue(engine, Qt::WindowOkButtonHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowCancelButtonHint", QScriptValue(engine, Qt::WindowCancelButtonHint), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WindowType_Mask", QScriptValue(engine, Qt::WindowType_Mask), QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("Qt", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

Q_DECLARE_METATYPE(enum Qt::AlignmentFlag);
Q_DECLARE_METATYPE(enum Qt::AnchorAttribute);
Q_DECLARE_METATYPE(enum Qt::ApplicationAttribute);
Q_DECLARE_METATYPE(enum Qt::ArrowType);
Q_DECLARE_METATYPE(enum Qt::AspectRatioMode);
Q_DECLARE_METATYPE(enum Qt::Axis);
Q_DECLARE_METATYPE(enum Qt::BGMode);
Q_DECLARE_METATYPE(enum Qt::BrushStyle);
Q_DECLARE_METATYPE(enum Qt::CaseSensitivity);
Q_DECLARE_METATYPE(enum Qt::CheckState);
Q_DECLARE_METATYPE(enum Qt::ClipOperation);
Q_DECLARE_METATYPE(enum Qt::ConnectionType);
Q_DECLARE_METATYPE(enum Qt::ContextMenuPolicy);
Q_DECLARE_METATYPE(enum Qt::Corner);
Q_DECLARE_METATYPE(enum Qt::CursorShape);
Q_DECLARE_METATYPE(enum Qt::DateFormat);
Q_DECLARE_METATYPE(enum Qt::DayOfWeek);
Q_DECLARE_METATYPE(enum Qt::DockWidgetArea);
Q_DECLARE_METATYPE(enum Qt::DropAction);
Q_DECLARE_METATYPE(enum Qt::EventPriority);
Q_DECLARE_METATYPE(enum Qt::FillRule);
Q_DECLARE_METATYPE(enum Qt::FocusPolicy);
Q_DECLARE_METATYPE(enum Qt::FocusReason);
Q_DECLARE_METATYPE(enum Qt::GlobalColor);
Q_DECLARE_METATYPE(enum Qt::HitTestAccuracy);
Q_DECLARE_METATYPE(enum Qt::ImageConversionFlag);
Q_DECLARE_METATYPE(enum Qt::InputMethodQuery);
Q_DECLARE_METATYPE(enum Qt::ItemDataRole);
Q_DECLARE_METATYPE(enum Qt::ItemFlag);
Q_DECLARE_METATYPE(enum Qt::ItemSelectionMode);
Q_DECLARE_METATYPE(enum Qt::Key);
Q_DECLARE_METATYPE(enum Qt::KeyboardModifier);
Q_DECLARE_METATYPE(enum Qt::LayoutDirection);
Q_DECLARE_METATYPE(enum Qt::MaskMode);
Q_DECLARE_METATYPE(enum Qt::MatchFlag);
Q_DECLARE_METATYPE(Qt::MatchFlags);
Q_DECLARE_METATYPE(enum Qt::Modifier);
Q_DECLARE_METATYPE(enum Qt::MouseButton);
Q_DECLARE_METATYPE(enum Qt::Orientation);
Q_DECLARE_METATYPE(enum Qt::PenCapStyle);
Q_DECLARE_METATYPE(enum Qt::PenJoinStyle);
Q_DECLARE_METATYPE(enum Qt::PenStyle);
Q_DECLARE_METATYPE(enum Qt::ScrollBarPolicy);
Q_DECLARE_METATYPE(enum Qt::ShortcutContext);
Q_DECLARE_METATYPE(enum Qt::SizeHint);
Q_DECLARE_METATYPE(enum Qt::SizeMode);
Q_DECLARE_METATYPE(enum Qt::SortOrder);
Q_DECLARE_METATYPE(enum Qt::TextElideMode);
Q_DECLARE_METATYPE(enum Qt::TextFlag);
Q_DECLARE_METATYPE(enum Qt::TextFormat);
Q_DECLARE_METATYPE(enum Qt::TextInteractionFlag);
Q_DECLARE_METATYPE(enum Qt::TimeSpec);
Q_DECLARE_METATYPE(enum Qt::ToolBarArea);
Q_DECLARE_METATYPE(enum Qt::ToolButtonStyle);
Q_DECLARE_METATYPE(enum Qt::TransformationMode);
Q_DECLARE_METATYPE(enum Qt::UIEffect);
Q_DECLARE_METATYPE(enum Qt::WhiteSpaceMode);
Q_DECLARE_METATYPE(enum Qt::WidgetAttribute);
Q_DECLARE_METATYPE(enum Qt::WindowFrameSection);
Q_DECLARE_METATYPE(enum Qt::WindowModality);
Q_DECLARE_METATYPE(enum Qt::WindowState);
Q_DECLARE_METATYPE(enum Qt::WindowType);

QScriptValue AlignmentFlagtoScriptValue(QScriptEngine *engine, const enum Qt::AlignmentFlag &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue AnchorAttributetoScriptValue(QScriptEngine *engine, const enum Qt::AnchorAttribute &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ApplicationAttributetoScriptValue(QScriptEngine *engine, const enum Qt::ApplicationAttribute &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ArrowTypetoScriptValue(QScriptEngine *engine, const enum Qt::ArrowType &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue AspectRatioModetoScriptValue(QScriptEngine *engine, const enum Qt::AspectRatioMode &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue AxistoScriptValue(QScriptEngine *engine, const enum Qt::Axis &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue BGModetoScriptValue(QScriptEngine *engine, const enum Qt::BGMode &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue BrushStyletoScriptValue(QScriptEngine *engine, const enum Qt::BrushStyle &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue CaseSensitivitytoScriptValue(QScriptEngine *engine, const enum Qt::CaseSensitivity &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue CheckStatetoScriptValue(QScriptEngine *engine, const enum Qt::CheckState &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ClipOperationtoScriptValue(QScriptEngine *engine, const enum Qt::ClipOperation &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ConnectionTypetoScriptValue(QScriptEngine *engine, const enum Qt::ConnectionType &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ContextMenuPolicytoScriptValue(QScriptEngine *engine, const enum Qt::ContextMenuPolicy &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue CornertoScriptValue(QScriptEngine *engine, const enum Qt::Corner &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue CursorShapetoScriptValue(QScriptEngine *engine, const enum Qt::CursorShape &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue DateFormattoScriptValue(QScriptEngine *engine, const enum Qt::DateFormat &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue DayOfWeektoScriptValue(QScriptEngine *engine, const enum Qt::DayOfWeek &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue DockWidgetAreatoScriptValue(QScriptEngine *engine, const enum Qt::DockWidgetArea &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue DropActiontoScriptValue(QScriptEngine *engine, const enum Qt::DropAction &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue EventPrioritytoScriptValue(QScriptEngine *engine, const enum Qt::EventPriority &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue FillRuletoScriptValue(QScriptEngine *engine, const enum Qt::FillRule &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue FocusPolicytoScriptValue(QScriptEngine *engine, const enum Qt::FocusPolicy &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue FocusReasontoScriptValue(QScriptEngine *engine, const enum Qt::FocusReason &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue GlobalColortoScriptValue(QScriptEngine *engine, const enum Qt::GlobalColor &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue HitTestAccuracytoScriptValue(QScriptEngine *engine, const enum Qt::HitTestAccuracy &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ImageConversionFlagtoScriptValue(QScriptEngine *engine, const enum Qt::ImageConversionFlag &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue InputMethodQuerytoScriptValue(QScriptEngine *engine, const enum Qt::InputMethodQuery &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ItemDataRoletoScriptValue(QScriptEngine *engine, const enum Qt::ItemDataRole &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ItemFlagtoScriptValue(QScriptEngine *engine, const enum Qt::ItemFlag &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ItemSelectionModetoScriptValue(QScriptEngine *engine, const enum Qt::ItemSelectionMode &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue KeytoScriptValue(QScriptEngine *engine, const enum Qt::Key &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue KeyboardModifiertoScriptValue(QScriptEngine *engine, const enum Qt::KeyboardModifier &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue LayoutDirectiontoScriptValue(QScriptEngine *engine, const enum Qt::LayoutDirection &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue MaskModetoScriptValue(QScriptEngine *engine, const enum Qt::MaskMode &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue MatchFlagtoScriptValue(QScriptEngine *engine, const enum Qt::MatchFlag &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue MatchFlagstoScriptValue(QScriptEngine *engine, const Qt::MatchFlags &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ModifiertoScriptValue(QScriptEngine *engine, const enum Qt::Modifier &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue MouseButtontoScriptValue(QScriptEngine *engine, const enum Qt::MouseButton &p)
{
  return QScriptValue(engine, (int)p);
}

/*
QScriptValue OrientationtoScriptValue(QScriptEngine *engine, const enum Qt::Orientation &p)
{
  return QScriptValue(engine, (int)p);
}
*/

QScriptValue PenCapStyletoScriptValue(QScriptEngine *engine, const enum Qt::PenCapStyle &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue PenJoinStyletoScriptValue(QScriptEngine *engine, const enum Qt::PenJoinStyle &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue PenStyletoScriptValue(QScriptEngine *engine, const enum Qt::PenStyle &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ScrollBarPolicytoScriptValue(QScriptEngine *engine, const enum Qt::ScrollBarPolicy &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ShortcutContexttoScriptValue(QScriptEngine *engine, const enum Qt::ShortcutContext &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue SizeHinttoScriptValue(QScriptEngine *engine, const enum Qt::SizeHint &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue SizeModetoScriptValue(QScriptEngine *engine, const enum Qt::SizeMode &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue SortOrdertoScriptValue(QScriptEngine *engine, const enum Qt::SortOrder &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue TextElideModetoScriptValue(QScriptEngine *engine, const enum Qt::TextElideMode &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue TextFlagtoScriptValue(QScriptEngine *engine, const enum Qt::TextFlag &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue TextFormattoScriptValue(QScriptEngine *engine, const enum Qt::TextFormat &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue TextInteractionFlagtoScriptValue(QScriptEngine *engine, const enum Qt::TextInteractionFlag &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue TimeSpectoScriptValue(QScriptEngine *engine, const enum Qt::TimeSpec &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ToolBarAreatoScriptValue(QScriptEngine *engine, const enum Qt::ToolBarArea &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue ToolButtonStyletoScriptValue(QScriptEngine *engine, const enum Qt::ToolButtonStyle &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue TransformationModetoScriptValue(QScriptEngine *engine, const enum Qt::TransformationMode &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue UIEffecttoScriptValue(QScriptEngine *engine, const enum Qt::UIEffect &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue WhiteSpaceModetoScriptValue(QScriptEngine *engine, const enum Qt::WhiteSpaceMode &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue WidgetAttributetoScriptValue(QScriptEngine *engine, const enum Qt::WidgetAttribute &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue WindowFrameSectiontoScriptValue(QScriptEngine *engine, const enum Qt::WindowFrameSection &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue WindowModalitytoScriptValue(QScriptEngine *engine, const enum Qt::WindowModality &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue WindowStatetoScriptValue(QScriptEngine *engine, const enum Qt::WindowState &p)
{
  return QScriptValue(engine, (int)p);
}

QScriptValue WindowTypetoScriptValue(QScriptEngine *engine, const enum Qt::WindowType &p)
{
  return QScriptValue(engine, (int)p);
}

void AlignmentFlagfromScriptValue(const QScriptValue &obj, enum Qt::AlignmentFlag &p)
{
  p = (enum Qt::AlignmentFlag)obj.toInt32();
}

void AnchorAttributefromScriptValue(const QScriptValue &obj, enum Qt::AnchorAttribute &p)
{
  p = (enum Qt::AnchorAttribute)obj.toInt32();
}

void ApplicationAttributefromScriptValue(const QScriptValue &obj, enum Qt::ApplicationAttribute &p)
{
  p = (enum Qt::ApplicationAttribute)obj.toInt32();
}

void ArrowTypefromScriptValue(const QScriptValue &obj, enum Qt::ArrowType &p)
{
  p = (enum Qt::ArrowType)obj.toInt32();
}

void AspectRatioModefromScriptValue(const QScriptValue &obj, enum Qt::AspectRatioMode &p)
{
  p = (enum Qt::AspectRatioMode)obj.toInt32();
}

void AxisfromScriptValue(const QScriptValue &obj, enum Qt::Axis &p)
{
  p = (enum Qt::Axis)obj.toInt32();
}

void BGModefromScriptValue(const QScriptValue &obj, enum Qt::BGMode &p)
{
  p = (enum Qt::BGMode)obj.toInt32();
}

void BrushStylefromScriptValue(const QScriptValue &obj, enum Qt::BrushStyle &p)
{
  p = (enum Qt::BrushStyle)obj.toInt32();
}

void CaseSensitivityfromScriptValue(const QScriptValue &obj, enum Qt::CaseSensitivity &p)
{
  p = (enum Qt::CaseSensitivity)obj.toInt32();
}

void CheckStatefromScriptValue(const QScriptValue &obj, enum Qt::CheckState &p)
{
  p = (enum Qt::CheckState)obj.toInt32();
}

void ClipOperationfromScriptValue(const QScriptValue &obj, enum Qt::ClipOperation &p)
{
  p = (enum Qt::ClipOperation)obj.toInt32();
}

void ConnectionTypefromScriptValue(const QScriptValue &obj, enum Qt::ConnectionType &p)
{
  p = (enum Qt::ConnectionType)obj.toInt32();
}

void ContextMenuPolicyfromScriptValue(const QScriptValue &obj, enum Qt::ContextMenuPolicy &p)
{
  p = (enum Qt::ContextMenuPolicy)obj.toInt32();
}

void CornerfromScriptValue(const QScriptValue &obj, enum Qt::Corner &p)
{
  p = (enum Qt::Corner)obj.toInt32();
}

void CursorShapefromScriptValue(const QScriptValue &obj, enum Qt::CursorShape &p)
{
  p = (enum Qt::CursorShape)obj.toInt32();
}

void DateFormatfromScriptValue(const QScriptValue &obj, enum Qt::DateFormat &p)
{
  p = (enum Qt::DateFormat)obj.toInt32();
}

void DayOfWeekfromScriptValue(const QScriptValue &obj, enum Qt::DayOfWeek &p)
{
  p = (enum Qt::DayOfWeek)obj.toInt32();
}

void DockWidgetAreafromScriptValue(const QScriptValue &obj, enum Qt::DockWidgetArea &p)
{
  p = (enum Qt::DockWidgetArea)obj.toInt32();
}

void DropActionfromScriptValue(const QScriptValue &obj, enum Qt::DropAction &p)
{
  p = (enum Qt::DropAction)obj.toInt32();
}

void EventPriorityfromScriptValue(const QScriptValue &obj, enum Qt::EventPriority &p)
{
  p = (enum Qt::EventPriority)obj.toInt32();
}

void FillRulefromScriptValue(const QScriptValue &obj, enum Qt::FillRule &p)
{
  p = (enum Qt::FillRule)obj.toInt32();
}

void FocusPolicyfromScriptValue(const QScriptValue &obj, enum Qt::FocusPolicy &p)
{
  p = (enum Qt::FocusPolicy)obj.toInt32();
}

void FocusReasonfromScriptValue(const QScriptValue &obj, enum Qt::FocusReason &p)
{
  p = (enum Qt::FocusReason)obj.toInt32();
}

void GlobalColorfromScriptValue(const QScriptValue &obj, enum Qt::GlobalColor &p)
{
  p = (enum Qt::GlobalColor)obj.toInt32();
}

void HitTestAccuracyfromScriptValue(const QScriptValue &obj, enum Qt::HitTestAccuracy &p)
{
  p = (enum Qt::HitTestAccuracy)obj.toInt32();
}

void ImageConversionFlagfromScriptValue(const QScriptValue &obj, enum Qt::ImageConversionFlag &p)
{
  p = (enum Qt::ImageConversionFlag)obj.toInt32();
}

void InputMethodQueryfromScriptValue(const QScriptValue &obj, enum Qt::InputMethodQuery &p)
{
  p = (enum Qt::InputMethodQuery)obj.toInt32();
}

void ItemDataRolefromScriptValue(const QScriptValue &obj, enum Qt::ItemDataRole &p)
{
  p = (enum Qt::ItemDataRole)obj.toInt32();
}

void ItemFlagfromScriptValue(const QScriptValue &obj, enum Qt::ItemFlag &p)
{
  p = (enum Qt::ItemFlag)obj.toInt32();
}

void ItemSelectionModefromScriptValue(const QScriptValue &obj, enum Qt::ItemSelectionMode &p)
{
  p = (enum Qt::ItemSelectionMode)obj.toInt32();
}

void KeyfromScriptValue(const QScriptValue &obj, enum Qt::Key &p)
{
  p = (enum Qt::Key)obj.toInt32();
}

void KeyboardModifierfromScriptValue(const QScriptValue &obj, enum Qt::KeyboardModifier &p)
{
  p = (enum Qt::KeyboardModifier)obj.toInt32();
}

void LayoutDirectionfromScriptValue(const QScriptValue &obj, enum Qt::LayoutDirection &p)
{
  p = (enum Qt::LayoutDirection)obj.toInt32();
}

void MaskModefromScriptValue(const QScriptValue &obj, enum Qt::MaskMode &p)
{
  p = (enum Qt::MaskMode)obj.toInt32();
}

void MatchFlagfromScriptValue(const QScriptValue &obj, enum Qt::MatchFlag &p)
{
  p = (enum Qt::MatchFlag)obj.toInt32();
}

void MatchFlagsfromScriptValue(const QScriptValue &obj, Qt::MatchFlags &p)
{
  p = (Qt::MatchFlags)obj.toInt32();
}

void ModifierfromScriptValue(const QScriptValue &obj, enum Qt::Modifier &p)
{
  p = (enum Qt::Modifier)obj.toInt32();
}

void MouseButtonfromScriptValue(const QScriptValue &obj, enum Qt::MouseButton &p)
{
  p = (enum Qt::MouseButton)obj.toInt32();
}

/*
void OrientationfromScriptValue(const QScriptValue &obj, enum Qt::Orientation &p)
{
  p = (enum Qt::Orientation)obj.toInt32();
}
*/

void PenCapStylefromScriptValue(const QScriptValue &obj, enum Qt::PenCapStyle &p)
{
  p = (enum Qt::PenCapStyle)obj.toInt32();
}

void PenJoinStylefromScriptValue(const QScriptValue &obj, enum Qt::PenJoinStyle &p)
{
  p = (enum Qt::PenJoinStyle)obj.toInt32();
}

void PenStylefromScriptValue(const QScriptValue &obj, enum Qt::PenStyle &p)
{
  p = (enum Qt::PenStyle)obj.toInt32();
}

void ScrollBarPolicyfromScriptValue(const QScriptValue &obj, enum Qt::ScrollBarPolicy &p)
{
  p = (enum Qt::ScrollBarPolicy)obj.toInt32();
}

void ShortcutContextfromScriptValue(const QScriptValue &obj, enum Qt::ShortcutContext &p)
{
  p = (enum Qt::ShortcutContext)obj.toInt32();
}

void SizeHintfromScriptValue(const QScriptValue &obj, enum Qt::SizeHint &p)
{
  p = (enum Qt::SizeHint)obj.toInt32();
}

void SizeModefromScriptValue(const QScriptValue &obj, enum Qt::SizeMode &p)
{
  p = (enum Qt::SizeMode)obj.toInt32();
}

void SortOrderfromScriptValue(const QScriptValue &obj, enum Qt::SortOrder &p)
{
  p = (enum Qt::SortOrder)obj.toInt32();
}

void TextElideModefromScriptValue(const QScriptValue &obj, enum Qt::TextElideMode &p)
{
  p = (enum Qt::TextElideMode)obj.toInt32();
}

void TextFlagfromScriptValue(const QScriptValue &obj, enum Qt::TextFlag &p)
{
  p = (enum Qt::TextFlag)obj.toInt32();
}

void TextFormatfromScriptValue(const QScriptValue &obj, enum Qt::TextFormat &p)
{
  p = (enum Qt::TextFormat)obj.toInt32();
}

void TextInteractionFlagfromScriptValue(const QScriptValue &obj, enum Qt::TextInteractionFlag &p)
{
  p = (enum Qt::TextInteractionFlag)obj.toInt32();
}

void TimeSpecfromScriptValue(const QScriptValue &obj, enum Qt::TimeSpec &p)
{
  p = (enum Qt::TimeSpec)obj.toInt32();
}

void ToolBarAreafromScriptValue(const QScriptValue &obj, enum Qt::ToolBarArea &p)
{
  p = (enum Qt::ToolBarArea)obj.toInt32();
}

void ToolButtonStylefromScriptValue(const QScriptValue &obj, enum Qt::ToolButtonStyle &p)
{
  p = (enum Qt::ToolButtonStyle)obj.toInt32();
}

void TransformationModefromScriptValue(const QScriptValue &obj, enum Qt::TransformationMode &p)
{
  p = (enum Qt::TransformationMode)obj.toInt32();
}

void UIEffectfromScriptValue(const QScriptValue &obj, enum Qt::UIEffect &p)
{
  p = (enum Qt::UIEffect)obj.toInt32();
}

void WhiteSpaceModefromScriptValue(const QScriptValue &obj, enum Qt::WhiteSpaceMode &p)
{
  p = (enum Qt::WhiteSpaceMode)obj.toInt32();
}

void WidgetAttributefromScriptValue(const QScriptValue &obj, enum Qt::WidgetAttribute &p)
{
  p = (enum Qt::WidgetAttribute)obj.toInt32();
}

void WindowFrameSectionfromScriptValue(const QScriptValue &obj, enum Qt::WindowFrameSection &p)
{
  p = (enum Qt::WindowFrameSection)obj.toInt32();
}

void WindowModalityfromScriptValue(const QScriptValue &obj, enum Qt::WindowModality &p)
{
  p = (enum Qt::WindowModality)obj.toInt32();
}

void WindowStatefromScriptValue(const QScriptValue &obj, enum Qt::WindowState &p)
{
  p = (enum Qt::WindowState)obj.toInt32();
}

void WindowTypefromScriptValue(const QScriptValue &obj, enum Qt::WindowType &p)
{
  p = (enum Qt::WindowType)obj.toInt32();
}
