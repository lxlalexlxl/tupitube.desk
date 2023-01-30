﻿/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef TUPPROJECTSCANNER_H 
#define TUPPROJECTSCANNER_H

#include "tglobal.h"
#include "tuplibraryobject.h"

#include <QObject>
#include <QDomNode>

class TUPITUBE_EXPORT TupProjectScanner : public QObject
{
    Q_OBJECT

    public:
        TupProjectScanner();
        ~TupProjectScanner();

        virtual bool read(const QString &filename, const QString &tempFolder = QString());

        QString getProjectName() const;
        int scenesCount();
        QList<QString> sceneNamesList();
        bool isLibraryEmpty();

        struct LibraryObject {
            QString key;
            TupLibraryObject::ObjectType type;
            QString path;
        };

        struct Folder {
            QString key;
            QList<LibraryObject> objects;
            QList<Folder> folders;
        };
        Folder getLibrary();

    private:
        QString readSceneName(const QString &xml) const;
        bool scanLibrary(const QString &xml);
        bool storeObject();

        QString projectName;
        int scenesTotal;
        QList<QString> scenesLabels;

        Folder library;
        QList<LibraryObject> objects;
        int objectsTotal;
        LibraryObject object;

        bool scanObjects(const QString &folderId, const QString &xml);
        bool scanObject(QDomNode xml);
};

#endif
