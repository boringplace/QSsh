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

#include <iostream>
#include "sshrestarter.h"

void showSyntax(int);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString cfg;

    if (argc > 2) {
        showSyntax(1);
    }

    if (argc == 2) {
        cfg = argv[1];
    }

    SshRestarter *restarter = new SshRestarter(cfg);
    restarter->setWindowFlags(Qt::Window);
    restarter->show();

    return a.exec();
}


void showSyntax(int ret)
{
    std::cerr << "Syntax: " << std::endl;
    std::cerr << "         SshRestarter [config.ini]" << std::endl;
    exit(ret);
}

