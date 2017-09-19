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

#include "sshrestarter.h"

#include <QtDebug>
#include <QFileInfo>

SshRestarter::SshRestarter(const QString &config) :
    QWidget()
{
    connect_mapper = new QSignalMapper(this);
    connect(connect_mapper, SIGNAL(mapped(int)), this, SLOT(command(int)));

    ready_mapper = new QSignalMapper(this);
    connect(ready_mapper, SIGNAL(mapped(int)), this, SLOT(ready(int)));

    error_mapper = new QSignalMapper(this);
    connect(error_mapper, SIGNAL(mapped(int)), this, SLOT(error(int)));

    disconnect_mapper = new QSignalMapper(this);
    connect(disconnect_mapper, SIGNAL(mapped(int)), this, SLOT(disconnect(int)));

    readSettings(config);
    loadConfiguration();
    logger = new SshLogger(m_logfile);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(updateUI());
    vbox->addWidget(logger);
    setLayout(vbox);

    setWindowTitle(QString("%1 (%2 hosts)").arg(tr("SSH Restarter")).arg(m_connections.size()));
}

void SshRestarter::loadConfiguration()
{
    QSettings settings(m_config, QSettings::IniFormat);

    qInfo() << m_config;

    bool hosts_exists = true;
    if (!settings.childGroups().contains("hosts")) {
        hosts_exists = false;
        qInfo() << settings.childGroups();
    }

    int size = settings.beginReadArray("hosts");
    if (!hosts_exists || size < 1)
        size = 5;

    int size_of_saved = m_default_timeouts.size();
    if (size != size_of_saved) {
        qInfo() << "Number of saved command timeouts (is" << m_default_timeouts.size() << ") is not same as number of hosts (is" << size << ")";
        while (m_default_timeouts.size() > size)
            m_default_timeouts.removeLast();
    }

    while (!m_connections.isEmpty())
        delete m_connections.takeFirst();
    m_commands.clear();

    QString def_user = settings.value("user", "root").toString();
    QString def_pass = settings.value("pass", "pass").toString();
    QString def_command = settings.value("command", "restart.sh").toString();
    QString def_identity = settings.value("identity", "private.key").toString();
    int def_timeout = settings.value("timeout", 30).toInt();
    int def_port = settings.value("port", 22).toInt();
    m_logfile = settings.value("log").toString();
    m_default_timeout = settings.value("command_timeout", 15).toInt();

    qInfo().nospace() << "default user: " << def_user;
    qInfo().nospace() << "default pass: " << def_pass;
    qInfo().nospace() << "default port: " << def_port;
    qInfo().nospace() << "default timeout: " << def_timeout;
    qInfo().nospace() << "default command: " << def_command;
    qInfo().nospace() << "default command timeout: " << m_default_timeout;

    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        int n = i + 1;
        QSsh::SshConnectionParameters params;
        QString def_host = QString("host%1").arg(n);

        params.host = settings.value("host", def_host.toLatin1()).toString();
        params.userName = settings.value("user", def_user).toString();
        params.password = settings.value("pass", def_pass).toString();
        params.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePassword;
        QString identity = settings.value("identity", def_identity).toString();
        if (identity.startsWith ("~/"))
            identity.replace (0, 1, QDir::homePath());
        QFile identity_file(identity);
        params.timeout = settings.value("timeout", def_timeout).toInt();
        params.port = settings.value("port", def_port).toInt();
        params.privateKeyFile = identity;
        QString command = settings.value("command", def_command).toString();

        if (i >= size_of_saved)
            m_default_timeouts.append(m_default_timeout);

        qInfo().nospace() << "\n[host" << n << "]";
        qInfo().nospace() << n << "/host: " << params.host;
        qInfo().nospace() << n << "/user: " << params.userName;
        qInfo().nospace() << n << "/pass: " << params.password;
        qInfo().nospace() << n << "/command: " << command;
        if(identity_file.exists()) {
            params.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePublicKey;
            qInfo().nospace() << n << "/identity: " << params.privateKeyFile;
        }
        qInfo().nospace() << n << "/port: " << params.port;
        qInfo().nospace() << n << "/timeout: " << params.timeout;

        QSsh::SshConnection* p_connection = new QSsh::SshConnection(params, this);
        m_connections.append(p_connection);
        m_commands.append(command);
        connect(p_connection, SIGNAL(connected()), connect_mapper, SLOT(map()));
        connect_mapper->setMapping(p_connection, i);
        connect(p_connection, SIGNAL(dataAvailable(const QString&)), ready_mapper, SLOT(map()));
        ready_mapper->setMapping(p_connection, i);
        connect(p_connection, SIGNAL(error(QSsh::SshError)), error_mapper, SLOT(map()));
        error_mapper->setMapping(p_connection, i);
        connect(p_connection, SIGNAL(disconnected()), disconnect_mapper, SLOT(map()));
        disconnect_mapper->setMapping(p_connection, i);
    }
    settings.endArray();
}

void SshRestarter::saveConfiguration()
{
    QSettings settings(m_config, QSettings::IniFormat);

    settings.beginWriteArray("hosts");
    for (int i = 0; i < m_connections.size(); ++i) {
        settings.setArrayIndex(i);

        settings.setValue("host", m_connections.at(i)->connectionParameters().host);
        settings.setValue("user", m_connections.at(i)->connectionParameters().userName);
        settings.setValue("pass", m_connections.at(i)->connectionParameters().password);
        QString identity = m_connections.at(i)->connectionParameters().privateKeyFile;
        if (!identity.isEmpty())
            settings.setValue("identity", identity);
    }
    settings.endArray();
}

QHBoxLayout* SshRestarter::updateUI()
{
    //saveSettings(); //test

    /*if (layout != 0)
    {
        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != 0)
            layout->removeItem(item);
        delete layout;
    }

    while (!m_groupboxes.isEmpty())
        delete m_groupboxes.takeFirst();*/

    QSignalMapper* mapper = new QSignalMapper(this);
    QSignalMapper* timerMapper = new QSignalMapper(this);
    QSignalMapper* startMapper = new QSignalMapper(this);
    QSignalMapper* stopMapper = new QSignalMapper(this);
    QHBoxLayout *hbox = new QHBoxLayout;
    for (int i = 0; i < m_connections.size(); ++i) {
        int n = i + 1;
        QGroupBox *box = createGroup(m_connections.at(i)->connectionParameters().host, n, mapper, timerMapper, startMapper, stopMapper);
        hbox->addWidget(box);
        m_groupboxes.append(box);
    }
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(restart(int)));
    connect(timerMapper, SIGNAL(mapped(int)), this, SLOT(timeout(int)));
    connect(startMapper, SIGNAL(mapped(int)), this, SLOT(start(int)));
    connect(stopMapper, SIGNAL(mapped(int)), this, SLOT(stop(int)));

    qDebug() << "m_timer_state_on_start size is" << m_timer_state_on_start.size();
    for (int i = 0; i < m_timer_state_on_start.size(); ++i) {
        if (m_timers.size() <= i)
            break;
        QTimer* timer = m_timers.at(i);
        if (m_timer_state_on_start.at(i)) 
            timer->start();
    }

    return hbox;
}

QGroupBox *SshRestarter::createGroup(QString host, int n, QSignalMapper* mapper, QSignalMapper* timerMapper, QSignalMapper* startMapper, QSignalMapper* stopMapper)
{
    QGroupBox *groupBox = new QGroupBox(host);
    int default_timeout = m_default_timeouts.at(n-1);

    QPushButton *restartButton = new QPushButton(QString("%1&%2").arg(tr("Restart")).arg(n));
    QPushButton *timerStartButton = new QPushButton(QString("%1%2").arg(tr("Start")).arg(n));
    QPushButton *timerStopButton = new QPushButton(QString("%1%2").arg(tr("Stop")).arg(n));
    QLineEdit *lineEdit = new QLineEdit(QString("%1").arg(default_timeout));
    QLCDNumber *lcdNumber = new QLCDNumber();
    QVBoxLayout *vbox = new QVBoxLayout;
    QHBoxLayout *hboxStart = new QHBoxLayout;
    QHBoxLayout *hboxStop = new QHBoxLayout;

    QTimer *timer = new QTimer();
    timer->setInterval(1000);
    m_timers.append(timer);
    m_timer_intervals.append(lineEdit);
    m_timer_lcd_numbers.append(lcdNumber);
    int default_start_number = default_timeout;
    if (default_start_number < 0)
        default_start_number = 0;
    if (default_start_number > 99999)
        default_start_number = 99999;
    m_timer_numbers.append(default_start_number);

    connect(restartButton, SIGNAL(clicked()), mapper, SLOT(map()));
    mapper->setMapping(restartButton, n-1);
    connect(timer, SIGNAL(timeout()), timerMapper, SLOT(map()));
    timerMapper->setMapping(timer, n-1);

    connect(timerStartButton, SIGNAL(clicked()), startMapper, SLOT(map()));
    startMapper->setMapping(timerStartButton, n-1);
    connect(timerStopButton, SIGNAL(clicked()), stopMapper, SLOT(map()));
    stopMapper->setMapping(timerStopButton, n-1);

    lineEdit->setAlignment(Qt::AlignRight);

    vbox->addWidget(restartButton);
    hboxStart->addStretch(0);
    hboxStart->addWidget(lineEdit);
    hboxStart->addWidget(timerStartButton);
    vbox->addLayout(hboxStart);
    hboxStop->addStretch(0);
    hboxStop->addWidget(lcdNumber);
    hboxStop->addWidget(timerStopButton);
    vbox->addLayout(hboxStop);
    vbox->addStrut(0);
    groupBox->setLayout(vbox);

    return groupBox;
}

void SshRestarter::restart(int i)
{
    ConnectionPtr conn = m_connections.at(i);
    QString host = conn->connectionParameters().host;

    qInfo() << "Restart" << host;

    logger->append(QString("restart: %1").arg(host));
    if (conn->state() != State::Connected) {
        if (conn->state() != State::Connecting) {
            qInfo() << "try to connect";
            if (!conn->errorString().isEmpty())
                qInfo() << " connect error string:" << conn->errorString();
            conn->connectToHost();
        } else {
            qInfo() << "connect already in process";
        }
    } else {
        command(i);
    }

    if (m_timers.at(i)->isActive()) {
        int number = m_default_timeouts.at(i);
        qInfo() << "Start on restart from" << m_timer_numbers.at(i) << "to" << number;
        m_timer_numbers.replace(i, number);
    }
}

void SshRestarter::command(int i)
{
    ConnectionPtr conn = m_connections.at(i);
    QString host = conn->connectionParameters().host;

    qInfo() << "Command" << host;

    QString command = m_commands.at(i);

    if (!conn->errorString().isEmpty())
        qInfo() << " command error string:" << conn->errorString();

    logger->append(QString("%1: %2").arg(host).arg(command));
    if (conn->state() == State::Connected) {
        conn->createRemoteProcess(command.toLatin1());
    } else {
        if (conn->state() != State::Connecting)
            conn->connectToHost();
    }
}

void SshRestarter::ready(int i)
{
    ConnectionPtr conn = m_connections.at(i);
    QString host = conn->connectionParameters().host;

    qInfo() << "Data ready" << host;

    logger->append(QString("ready: %1").arg(host));
}

void SshRestarter::error(int i)
{
    ConnectionPtr conn = m_connections.at(i);
    QString host = conn->connectionParameters().host;

    qInfo() << "Error" << host;

    logger->append(QString("error: %1 (%2)").arg(conn->errorString()).arg(conn->state()));
}

void SshRestarter::disconnect(int i)
{
    ConnectionPtr conn = m_connections.at(i);
    QString host = conn->connectionParameters().host;

    qInfo() << "Disconnect" << host;

    logger->append(QString("disconnected: %2").arg(host));
}

void SshRestarter::start(int i)
{
    ConnectionPtr conn = m_connections.at(i);
    QTimer* timer = m_timers.at(i);
    QLineEdit* lineEdit = m_timer_intervals.at(i);
    QString host = conn->connectionParameters().host;

    int number = lineEdit->displayText().toInt();
    if (number <= 0) {
        qInfo() << "Bad timeout for" << host;
        logger->append(QString("bad number for: %1").arg(host));
        return;
    }
    if (number > 99999) {
        qInfo() << "Timeout for" << host << "too long";
        logger->append(QString("too long number for: %1 ").arg(host));
        return;
    }
    m_default_timeouts.replace(i, number);
    m_timer_numbers.replace(i, number);
    qInfo() << "Start timer for" << host << "to" << number;
    timer->start();

    logger->append(QString("start for: %1").arg(host));

    updateLCD(i, number);
}

void SshRestarter::stop(int i)
{
    ConnectionPtr conn = m_connections.at(i);
    QTimer* timer = m_timers.at(i);
    QString host = conn->connectionParameters().host;

    qInfo() << "Stop timer for " << host;
    timer->stop();

    logger->append(QString("stop for: %1").arg(host));

    m_timer_numbers.replace(i, 0);
    updateLCD(i, 0);
}

void SshRestarter::timeout(int i)
{
    ConnectionPtr conn = m_connections.at(i);
    QTimer* timer = m_timers.at(i);
    QString host = conn->connectionParameters().host;
    int number = m_timer_numbers.at(i);

    if (number <= 0)
    {
        qInfo() << "Timer for" << host << "should be stoped";
        timer->stop();
        updateLCD(i, 0);
        return;
    }

    if (--number > 0)
    {
        qDebug() << "timeout for" << host;
    } else {
        number = m_default_timeouts.at(i);
        qInfo() << "Restart by timer for" << host;
        logger->append(QString("restart by timer for: %1").arg(host));
    }

    m_timer_numbers.replace(i, number);
    updateLCD(i, number);
}

void SshRestarter::updateLCD(int i, int number)
{
    QLCDNumber* lcdNumber = m_timer_lcd_numbers.at(i);
    lcdNumber->display(number);
}

void SshRestarter::writeSettings()
{
    QSettings settings("SarFSC", "SshRestarter");

    settings.beginGroup("Window");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();

    settings.beginGroup("Configuration");
    settings.setValue("config", m_config);
    settings.beginWriteArray("hosts");
    for (int i = 0; i < m_default_timeouts.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("timeout", m_default_timeouts.at(i));
    }
    for (int i = 0; i < m_timers.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("timer_state", m_timers.at(i)->isActive() ? "true" : "false");
    }
    settings.endArray();
    settings.endGroup();
}

void SshRestarter::readSettings(const QString &config)
{
    QSettings settings("SarFSC", "SshRestarter");

    settings.beginGroup("Window");
    resize(settings.value("size", QSize(400, 400)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();

    settings.beginGroup("Configuration");
    QString def_config = QApplication::applicationDirPath() + "/config.ini";
    m_config = settings.value("config", def_config).toString();
    if (!config.isEmpty())
        m_config = config;
    int size = settings.beginReadArray("hosts");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        m_default_timeouts.append(settings.value("timeout", m_default_timeout).toInt());
        QString timerState = settings.value("timer_state", "false").toString();
        if (timerState == "true") {
            qDebug() << "timer_state for" << i << "is" << timerState;
            m_timer_state_on_start.append(true);
        }
        else
            m_timer_state_on_start.append(false);
    }
    settings.endArray();
    settings.endGroup();
}

void SshRestarter::closeEvent(QCloseEvent *event)
{
    if (reallyQuit()) {
        writeSettings();
        saveConfiguration();
        event->accept();
    } else {
        event->ignore();
    }
}

bool SshRestarter::reallyQuit()
{
    QMessageBox msgBox(QMessageBox::Question,
                       this->windowTitle() + tr(": Confirmation"),
                       tr("Do you really want to quit?"),
                       QMessageBox::Ok | QMessageBox::Cancel,
                       this, Qt::Dialog);

    //msgBox.setDefaultButton(QMessageBox::Cancel);

    int result = msgBox.exec();

    qDebug() << "The return value is: " << result;
    return result == QMessageBox::Ok;
}
