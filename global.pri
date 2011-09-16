#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#

#
# This file is included by all the other project files
# and is where options or configurations that affect all
# of the projects can be place.
#

#
# This is the relative directory path to the openrpt project.
#

exists(../../../openrpt) {
    OPENRPT_DIR = ../../../openrpt
}
exists(../../openrpt) {
    OPENRPT_DIR = ../../openrpt
}
exists(../openrpt) {
    OPENRPT_DIR = ../openrpt
}

! exists($${OPENRPT_DIR}) {
    error("Could not set the OPENRPT_DIR qmake variable.")
}

exists(../../../openrpt) {
    OPENRPT_BLD = ../../../openrpt-build-desktop
}
exists(../../openrpt) {
    OPENRPT_BLD = ../../openrpt-build-desktop
}
exists(../openrpt) {
    OPENRPT_BLD = ../openrpt-build-desktop
}

exists(../../../csvimp) {
    CSVIMP_DIR = ../../../csvimp
}
exists(../../csvimp) {
    CSVIMP_DIR = ../../csvimp
}
exists(../csvimp) {
    CSVIMP_DIR = ../csvimp
}

! exists($${CSVIMP_DIR}) {
    error("Could not set the CSVIMP_DIR qmake variable.")
}

INCLUDEPATH += ../$${OPENRPT_DIR}/common ../$${OPENRPT_DIR}/OpenRPT/renderer ../$${OPENRPT_DIR}/OpenRPT/wrtembed ../$${OPENRPT_DIR}/MetaSQL ../$${OPENRPT_DIR}/MetaSQL/tmp ../$${CSVIMP_DIR}/csvimpcommon
exists($${OPENRPT_BLD}) {
  INCLUDEPATH += ../$${OPENRPT_BLD}/common ../$${OPENRPT_BLD}/OpenRPT/renderer ../$${OPENRPT_BLD}/OpenRPT/wrtembed ../$${OPENRPT_BLD}/MetaSQL ../$${OPENRPT_BLD}/MetaSQL/tmp ../$${CSVIMP_BLD}/csvimpcommon
}
DEPENDPATH  += $${INCLUDEPATH}

! exists($${OPENRPT_BLD}) {
    OPENRPT_BLD = $${OPENRPT_DIR}
}

CONFIG += release thread
#CONFIG += debug

macx:exists(macx.pri) {
  include(macx.pri)
}

win32:exists(win32.pri) {
  include(win32.pri)
}

unix:exists(unix.pri) {
  include(unix.pri)
}

