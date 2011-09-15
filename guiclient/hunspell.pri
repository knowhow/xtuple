HEADERS += ../hunspell/w_char.hxx \
           ../hunspell/suggestmgr.hxx \
           ../hunspell/replist.hxx \
           ../hunspell/phonet.hxx \
           ../hunspell/langnum.hxx \
           ../hunspell/hunzip.hxx \
           ../hunspell/hunspell.hxx \
           ../hunspell/hunspell.h \
           ../hunspell/htypes.hxx \
           ../hunspell/hashmgr.hxx \
           ../hunspell/filemgr.hxx \
           ../hunspell/dictmgr.hxx \
           ../hunspell/csutil.hxx \
           ../hunspell/baseaffix.hxx \
           ../hunspell/atypes.hxx \
           ../hunspell/affixmgr.hxx \
           ../hunspell/affentry.hxx \
           ../hunspell/config.h

SOURCES += ../hunspell/utf_info.cxx \
           ../hunspell/suggestmgr.cxx \
           ../hunspell/replist.cxx \
           ../hunspell/phonet.cxx \
           ../hunspell/hunzip.cxx \
           ../hunspell/hunspell.cxx \
           ../hunspell/hashmgr.cxx \
           ../hunspell/filemgr.cxx \
           ../hunspell/dictmgr.cxx \
           ../hunspell/csutil.cxx \
           ../hunspell/affixmgr.cxx \
           ../hunspell/affentry.cxx

DEFINES += HUNSPELL_STATIC
OTHER_FILES += ../hunspell/license.myspell \
               ../hunspell/license.hunspell

