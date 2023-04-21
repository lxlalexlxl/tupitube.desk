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

#ifndef TCIRCLEBUTTON_H
#define TCIRCLEBUTTON_H

#include "tglobal.h"

#include <QPushButton>
#include <QStyleOptionButton>
#include <QPainter>
#include <QRegion>
#include <QRect>
#include <QPaintEvent>
#include <QMoveEvent>
#include <QApplication>
#include <QBitmap>
#include <QPainterPath>
#include <QTimer>
#include <QPaintEngine>

class T_GUI_EXPORT TCircleButton : public QPushButton
{
    Q_OBJECT

    public:
        TCircleButton(int diameter, bool animate = true, QWidget *parent = nullptr);
        ~TCircleButton();
        
        QStyleOptionButton styleOption() const;
        QSize sizeHint() const;
        
    protected:
        /**
         * Paints the button
         * @param e 
         */
        void paintEvent(QPaintEvent *e);
        
        /**
         * Init animation
         */
        void enterEvent(QEnterEvent *);
        /**
         * End animation
         */
        void leaveEvent(QEvent *);
        
        /**
         * Creates mask of button
         */
        virtual void paintMask();
        
    private slots:
        void animate();
        
    private:
        QPixmap m_mask;
        QPixmap m_pix;
        int m_diameter : 22;
        
        class Animator;
        Animator *m_animator;
};

#endif
