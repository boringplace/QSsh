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

#ifndef SSHRESTARTER_H
#define SSHRESTARTER_H

#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "sshconnection.h"
#include "sshlogger.h"

typedef QSsh::SshConnection::State State;

/// SSH Service restarter
class SshRestarter : public QWidget
{
    Q_OBJECT
public:

    explicit SshRestarter(const QString &config);

signals:

private slots:
    void restart(int i);
    void command(int i);
    void ready(int i);
    void disconnect(int i);
    void error(int i);

private:
    QString m_config;
    QString m_logfile;

    typedef QSharedPointer<QSsh::SshRemoteProcess> ProcessPtr;
    typedef QSsh::SshConnection* ConnectionPtr;

    QList<ConnectionPtr> m_connections;
    QStringList m_commands;
    QList<ProcessPtr> m_processes;

    QList<QGroupBox*> m_groupboxes;
    SshLogger *logger;
    QSignalMapper* connect_mapper;
    QSignalMapper* ready_mapper;
    QSignalMapper* error_mapper;
    QSignalMapper* disconnect_mapper;

    void loadSettings();
    void saveSettings();
    QHBoxLayout *updateUI();

    QGroupBox *createGroup(QString host, int n, QSignalMapper* mapper);
};

#endif // SSHRESTARTER_H
