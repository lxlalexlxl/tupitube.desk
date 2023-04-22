/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
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

#include "tupbrushstatus.h"

#include <QHBoxLayout>
#include <QLabel>

TupBrushStatus::TupBrushStatus(const QString &label, TColorCell::FillType context, const QPixmap &pix)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(QMargins(2, 2, 2, 2));
    layout->setSpacing(2);

    brushCell = new TColorCell(context, QBrush(Qt::black), QSize(20, 20));
    brushCell->setEnabled(false);

    QLabel *icon = new QLabel("");
    icon->setToolTip(label);
    icon->setPixmap(pix);

    layout->addWidget(icon);
    layout->addSpacing(3);
    layout->addWidget(brushCell);
}

TupBrushStatus::~TupBrushStatus()
{
}

void TupBrushStatus::setColor(const QPen &pen)
{
    brushCell->setBrush(pen.brush());
}

void TupBrushStatus::setColor(const QBrush &brush)
{
    brushCell->setBrush(brush);
}

void TupBrushStatus::setTooltip(const QString &tip)
{
    brushCell->setToolTip(tip);
}
