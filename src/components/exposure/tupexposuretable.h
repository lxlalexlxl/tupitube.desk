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

#ifndef TUPEXPOSURETABLE_H
#define TUPEXPOSURETABLE_H

#include "tglobal.h"
#include "tupexposureheader.h"

#include <QTableWidget>
#include <QPainter>
#include <QItemDelegate>
#include <QLineEdit>
#include <QMouseEvent>
#include <QMenu>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLineEdit>
#include <cmath>

class TupExposureVerticalHeader;

class TUPITUBE_EXPORT TupExposureTable : public QTableWidget
{
    Q_OBJECT

    friend class TupExposureVerticalHeader;
    friend class TupExposureItemDelegate;

    public:
        enum Attribute
        {
          IsEmpty = 1000
        };

        enum FrameType 
        {
          Unset = 0,
          Empty,
          Used,
          UsedWithTween
        };

        TupExposureTable(int fps, QWidget *parent = nullptr);
        ~TupExposureTable();

        void updateFPS(int fps);
        int currentLayer() const;
        int currentFrame() const;
        void insertLayer(int index, const QString &name);
        void insertFrame(int layerIndex, int frameIndex, const QString &name, bool external);
        void removeLayer(int layerIndex);
        void removeFrame(int layerIndex, int frameIndex);
        void exchangeFrame(int oldPosLayer, int oldPosFrame, int newPosLayer, int newPosFrame, bool external);
        void moveLayer(int oldPosLayer, int newPosLayer);

        // void setLockLayer(int layerIndex,  bool locked);
        void setLayerVisibility(int visualIndex, bool visibility);
        // void setLayerOpacity(int layerIndex, double opacity);
        // void storeUndoLayerOpacity(int layerIndex, double opacity);
        int usedFrames(int column) const;

        QString frameName(int layerIndex, int frameIndex);
        void setFrameName(int layerIndex, int frameIndex, const QString & name);
        void setLayerName(int layerIndex, const QString & name);

        void selectFrame(int layerIndex, int frameIndex);
        void selectFrame(int layerIndex, int frameIndex, const QString &selection);
        void updateSelection(int layerIndex, int frameIndex);

        int layersCount();
        int framesCount();
        int framesCountAtCurrentLayer();
        int framesCountAtLayer(int layer);

        void setSinglePopUpMenu(QMenu *single);
        
        void notifyCellClicked(int frame, int layer);
        void reset();

        TupExposureTable::FrameType frameState(int layerIndex, int frameIndex);
        void updateFrameState(int layerIndex, int frameIndex, TupExposureTable::FrameType value);

        QList<int> currentSelection();
        void updateSceneView(int layerIndex, int frameIndex);

    private slots:
        void markUsedFrames(int frameIndex,  int layerIndex);
        void requestFrameRenaming(QTableWidgetItem *item);
        void requestFrameSelection(int currentRow, int currentColumn, int previousRow, int previousColumn);
        void requestLayerMove(int logicalIndex, int oldVisualIndex, int newVisualIndex);
        void updateLayerSelection(int layerIndex);

    protected:
        bool edit(const QModelIndex & index, EditTrigger trigger, QEvent *event);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        // void mouseReleaseEvent(QMouseEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void mouseDoubleClickEvent(QMouseEvent *event);
        void enterEvent(QEnterEvent *event);
        void leaveEvent(QEvent *event);

    protected slots:
        void commitData(QWidget *editor);

    signals:
        void frameUsed(int layerIndex, int frameIndex);
        void frameRenamed(int layerIndex, int frameIndex,const QString & name);
        void frameSelected(int layerIndex, int frameIndex);
        void selectionCopied();
        void selectionPasted();
        void selectionRemoved();
        void frameExtended(int layerIndex, int frameIndex);

        void layerNameChanged(int layerIndex, const QString & name);
        void layerMoved(int oldIndex, int newIndex);
        void layerVisibilityChanged(int visualIndexLayer, bool visibility);

        void newPerspective(int);

    private:
        bool layerIndexIsValid(int layerIndex);
        bool frameIndexIsValid(int frameIndex);
        void markNextFrame(int frameIndex, int layerIndex);

        TupExposureHeader *header;
        QMenu *singleMenu;
        // QMenu *multipleMenu;
        bool removingLayer;
        bool isLocalRequest;
        int uiTheme;
        bool isEditing;

        // QList<double> opacityControl;
        // QList<double> undoOpacities;
};

#endif
