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
#include "tapplicationproperties.h"

#include <QLabel>
#include <QBoxLayout>
#include <QTextEdit>
#include <QSpinBox>
#include <QDir>
#include <QCheckBox>
#include <QPushButton>

/**
 * @author Gustav Gonzalez 
*/

class TUPITUBE_PLUGIN SelectionSettings : public QWidget
{
    Q_OBJECT

    public:
        enum Action { AlignAction = 0, FlipsAction, OrderAction, GroupAction, PosAction, RotateAction, ScaleAction };
        enum Align { hAlign = 1, vAlign, totalAlign };
        enum Flip { Vertical = 1, Horizontal, Crossed };
        enum Order { ToBack = 0, ToFront, ToBackOneLevel, ToFrontOneLevel };
        enum Group { GroupItems = 0, UngroupItems };

        SelectionSettings(QWidget *parent = nullptr);
        ~SelectionSettings();

        void enableFormControls(bool flag);
        void setPos(int x, int y);
        void setProportionState(int flag);
        bool formIsVisible();

     signals:
        void callAlignAction(SelectionSettings::Align align);
        void callFlip(SelectionSettings::Flip flip);
        void callOrderAction(SelectionSettings::Order action);
        void callGroupAction(SelectionSettings::Group action);
        void positionUpdated(int x, int y);
        void rotationUpdated(int angle);
        void scaleUpdated(double xFactor, double yFactor);
        void activateProportion(bool flag);
        void objectHasBeenReset();

     public slots:
        void updateRotationAngle(int angle);
        void updateScaleFactor(double x, double y);

     private slots:
        void showActionPanel(int index);

        void alignObjectHorizontally();
        void alignObjectVertically();
        void alignObjectAbsolutely();
        void vFlip();
        void hFlip();
        void cFlip();
        void sendToBack();
        void sendToBackOneLevel();
        void sendToFront();
        void sendToFrontOneLevel();
        void openTipPanel();
        void notifyXMovement(int x);
        void notifyYMovement(int y);
        void notifyRotation(int angle);
        void notifyXScale(double factor);
        void notifyYScale(double factor);
        void groupItems();
        void ungroupItems();
        void enableProportion(int flag);
        void enablePasteOnMouse(int flag);
        void enablePasteOnMouse();

    private:
        void setLargetInterface();
        void setCompactInterface();
        void updatePanel(int index);

        QBoxLayout * setAlignBlock();
        QBoxLayout * setFlipsBlock();
        QBoxLayout * setOrderBlock();
        QBoxLayout * setGroupBlock();
        QBoxLayout * setPosBlock();
        QBoxLayout * setRotateBlock();
        QBoxLayout * setScaleBlock();
        QBoxLayout * setPasteBlock();

        QStringList buttonLabels;
        QPushButton * actionButton[7];
        QWidget * actionWidget[7];
        QBoxLayout *actionLayout[7];

        QSpinBox *xPosField;
        QSpinBox *yPosField;
        QSpinBox *angleField;
        QDoubleSpinBox *factorXField;
        QDoubleSpinBox *factorYField;
        QCheckBox *propCheck;
        QCheckBox *pasteCheck;

        QPushButton *tips;
        QWidget *formPanel;
        int currentX;
        int currentY;
        int currentAngle;
        double currentXFactor;
        double currentYFactor;
        QTextEdit *helpComponent;
        bool isVisible;

        int iconSize;
};

#endif
