/***************************************************************************
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

#ifndef TAPPLICATIONPROPERTIES_H
#define TAPPLICATIONPROPERTIES_H

#include "tglobal.h"

#include <QString>
#include <QDir>
#include <QLocale>
#include <QApplication>

class T_CORE_EXPORT TApplicationProperties
{
    protected:
        TApplicationProperties();
        virtual ~TApplicationProperties();

    public:
        void setHomeDir(const QString &path);
        void setBinDir(const QString &path);
        void setShareDir(const QString &path);
        void setDataDir(const QString &path);
        void setThemeDir(const QString &path);
        void setPluginDir(const QString &path);
        void setCacheDir(const QString &path);
        void setRepositoryDir(const QString &path);
        void setVersion(const QString &version);
        void setRevision(const QString &revision);
        void setCodeName(const QString &code);
        void setVectorBgDir(const QString &path);
        void setRasterResourcesDir(const QString &path);
        void setProjectDir(const QString &projectName);
        void setTempProjectDir(const QString &tempFolder, const QString &projectName);

        virtual QString homeDir() const;
        virtual QString binDir() const;
        virtual QString shareDir() const;
        virtual QString dataDir() const;
        virtual QString themeDir() const;
        virtual QString rasterResourcesDir() const;
        virtual QString rasterBgDir() const;
        virtual QString vectorBgDir() const;
        virtual QString pluginDir() const;
        virtual QString configDir() const;
        virtual QString cacheDir() const;
        virtual QString repositoryDir() const;
        virtual QString version() const;
        virtual QString revision() const;
        virtual QString codeName() const;
        virtual QString projectDir() const;

        static TApplicationProperties *instance();
		
    private:
        static TApplicationProperties *s_instance;

        QString homePath;
        QString binPath;
        QString sharePath;
        QString dataPath;
        QString themePath;
        QString vectorBgPath;
        QString rasterResourcesPath;
        QString repositoryPath;
        QString pluginPath;
        QString versionStr;
        QString codeNameStr;
        QString revisionStr;
        QString cachePath;
        QString projectPath;
};

#define kAppProp TApplicationProperties::instance()
#define HOME_DIR kAppProp->homeDir()
#define BIN_DIR kAppProp->binDir()
#define SHARE_DIR kAppProp->shareDir()
#define DATA_DIR kAppProp->dataDir()
#define THEME_DIR kAppProp->themeDir()
#define VECTOR_BG_DIR kAppProp->vectorBgDir()
#define RASTER_RESOURCES_DIR kAppProp->rasterResourcesDir()
#define RASTER_BG_DIR kAppProp->rasterBgDir()
#define CONFIG_DIR kAppProp->configDir()
#define PLUGINS_DIR kAppProp->pluginDir()
#define CACHE_DIR kAppProp->cacheDir()
#define REPOSITORY_DIR kAppProp->repositoryDir()
#define PROJECT_DIR kAppProp->projectDir()

#endif
