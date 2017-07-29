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

#ifndef TUPGRAPHICOBJECT_H
#define TUPGRAPHICOBJECT_H

#include "tglobal.h"
#include "tupabstractserializable.h"
#include "tupframe.h"

#include <QObject>
#include <QMatrix>
#include <QGraphicsItem>

class QGraphicsItem;
class TupItemTweener;

/**
 * @author David Cuadrado
*/
class TUPITUBE_EXPORT TupGraphicObject : public QObject, public TupAbstractSerializable
{
    public:
        enum Transformations { ScaleX = 1, ScaleY, Rotate, TranslateX, TranslateY };
        
        TupGraphicObject(QGraphicsItem *item, TupFrame *parent);
        ~TupGraphicObject();

        void setItem(QGraphicsItem *item);
        QGraphicsItem *item() const;
        
        void setObjectName(const QString &name);
        QString objectName() const;
        
        void setTween(TupItemTweener *tween);
        bool hasTween() const;
        void removeTween();
        TupItemTweener *tween() const;
        
        TupFrame *frame() const;
        int objectIndex() const;
        void setFrame(TupFrame *frame);
        int frameIndex();

        bool layerIsVisible();

        void setLastTweenPos(QPointF point);
        QPointF lastTweenPos();

        void setItemZValue(int value);
        int itemZValue();

        bool transformationIsNotEdited();
        void saveInitTransformation();
        void storeItemTransformation(const QString &properties);
        void undoTransformation();
        void redoTransformation();

        bool brushIsNotEdited();
        void saveInitBrush();
        void setBrush(const QString &xml);
        void redoBrushAction();
        void undoBrushAction();

        bool penIsNotEdited();
        void saveInitPen();
        void setPen(const QString &xml);
        void redoPenAction();
        void undoPenAction();

    public:
        virtual void fromXml(const QString &xml);
        virtual QDomElement toXml(QDomDocument &doc)  const;
        
    private:
        void initItemData();
        
        struct Private;
        Private *const k;
};

#endif
