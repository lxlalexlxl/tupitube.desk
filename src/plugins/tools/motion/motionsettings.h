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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "tglobal.h"
#include "tuptoolplugin.h"
#include "tradiobuttongroup.h"
#include "timagebutton.h"
#include "stepsviewer.h"

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QBoxLayout>
#include <QGraphicsPathItem>

class TupItemTweener;

class TUPITUBE_PLUGIN MotionSettings : public QWidget 
{
    Q_OBJECT

    public:
        enum Mode { Selection = 0, Path};
        MotionSettings(QWidget *parent = nullptr);
        ~MotionSettings();

        void setParameters(const QString &name, int framesCount, int startFrame);
        void setParameters(TupItemTweener *currentTween);
        void initStartCombo(int totalFrames, int currentIndex);
        void setStartFrame(int currentIndex);
        int startFrame();

        void updateSteps(const QGraphicsPathItem *path);
        QString tweenToXml(int currentScene, int currentLayer, int currentFrame, QPointF point, QString &path);
        int totalSteps();
        QList<QPointF> tweenPoints();
        void activateMode(TupToolPlugin::EditMode mode);
        void clearData();
        void notifySelection(bool flag);
        int startComboSize();
        void enableInitCombo(bool enable);
        QString currentTweenName() const;
        void updateSegments(const QPainterPath path);

        void undoSegment(const QPainterPath path);
        void redoSegment(const QPainterPath path);
        void enableSaveOption(bool flag);
        int stepsTotal();

        int getPathThickness();
        QColor getPathColor() const;

    private slots:
        void emitOptionChanged(int option);
        void applyTween();
        void updateTotalLabel(int total);
        void setPathColor();

    signals:
        void clickedCreatePath();
        void clickedSelect();
        void clickedResetTween();
        void clickedApplyTween();
        void startingFrameChanged(int);
        void framesTotalChanged();
        void pathThicknessChanged(int);
        void pathColorUpdated(const QColor &);
        
    private:
        void setInnerForm();
        void activeInnerForm(bool enable);
        void setEditMode();
        QGridLayout * pathSettingsPanel();
        QColor setButtonColor(QPushButton *button, const QColor &currentColor) const;

        QWidget *innerPanel;
        QBoxLayout *layout;

        QLabel *endingLabel;
        QLineEdit *input;
        TRadioButtonGroup *options;
        StepsViewer *stepViewer;
        QSpinBox *initSpinBox;
        QLabel *totalLabel;
        bool selectionDone;
        TupToolPlugin::Mode mode;

        QColor pathColor;
        QSpinBox *pathThickness;
        QPushButton *pathColorButton;

        TImageButton *applyButton;
        TImageButton *remove;
        QMargins margins;
};

#endif
