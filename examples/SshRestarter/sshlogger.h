/**************************************************************************
**
** This file is part of QSsh
**
** Copyright (c) 2017 Evgeny Sinelnikov
**
** Contact: sin@altlinux.org
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#ifndef SSHLOGGER_H
#define SSHLOGGER_H

#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QtGui>
#else
#include <QtWidgets>
#endif

/// SSH Logger
class SshLogger : public QPlainTextEdit
{
//    Q_OBJECT
public:

    explicit SshLogger(const QString &logFile);

signals:

public slots:
    void append(const QString& text);

private:
    QFile m_logFile;
};

#endif // SSHLOGGER_H
