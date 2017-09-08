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

#include "sshlogger.h"

#include <QtDebug>

SshLogger::SshLogger(const QString &logfile) :
    QPlainTextEdit(), m_logFile(logfile)
{
}

void SshLogger::append(const QString& text)
{
    appendPlainText(text);
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    if (m_logFile.isOpen())
        m_logFile.write(text.toLatin1());
}

