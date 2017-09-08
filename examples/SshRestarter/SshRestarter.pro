QT       += core network

contains(QT_VERSION, ^5.*) {
    QT += widgets
}

TARGET = SshRestarter
TEMPLATE = app

INCLUDEPATH = $$PWD/../../src/libs/ssh/

SOURCES += \
    main.cpp \
    sshlogger.cpp \
    sshrestarter.cpp

HEADERS  += \
    sshrestarter.h

include(../../qssh.pri) ## Required for IDE_LIBRARY_PATH and qtLibraryName
LIBS += -L$$IDE_LIBRARY_PATH -l$$qtLibraryName(Botan) -l$$qtLibraryName(QSsh)
