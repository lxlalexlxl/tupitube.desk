/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *                                                                         *
 *   2019:                                                                 *
 *    Alejandro Carrasco                                                   *
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

#include "stepsviewer.h"

#include <QLineEdit>
#ifdef TUP_DEBUG
#include <QDebug>
#endif

StepsViewer::StepsViewer(QWidget *parent) : QTableWidget(parent)
{
    verticalHeader()->hide();

    records = 0;
    setColumnCount(4);
    setColumnWidth(0, 70);
    setColumnWidth(1, 60);
    setColumnWidth(2, 20); 
    setColumnWidth(3, 20);

    setHorizontalHeaderLabels(QStringList() << tr("Interval") << tr("Frames") << tr("") << tr(""));

    setMinimumWidth(174);
    setMaximumHeight(800);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    plusButton = new QList<TPushButton*>();
    minusButton = new QList<TPushButton*>();
}

StepsViewer::~StepsViewer()
{
    delete plusButton;
    delete minusButton;
}

QSize StepsViewer::sizeHint() const
{
    return QSize(maximumWidth(), maximumHeight());
}

void StepsViewer::loadPath(const QGraphicsPathItem *pathItem, QList<int> intervals)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[StepsViewer::loadPath()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    frames = intervals;
    records = frames.count();

    // Set of key points which define the path
    path = pathItem->path();
    points = path.toFillPolygon();

    if (points.isEmpty())
        return;

    points.removeLast();

    // This list contains the (green) key points of the path
    calculateKeys();
    calculateGroups();

    int total = frames.count();
    for (int row=0; row < total; row++) { // Processing every segment
         QList<QPointF> block = pointBlocks.at(row);
         int framesCount = frames.at(row);
         int size = block.size();
         QList<QPointF> segment;

         if (size > 2) {
             int delta = size/(framesCount-1);
             int pos = delta;
             if (row==0) {
                 framesCount--;
                 segment.append(block.at(0));
             } else {
                 delta = size/framesCount;
             }

             if (framesCount > 2) {
                 for (int i=1; i < framesCount; i++) { // calculating points set for the segment j
                      segment << block.at(pos);
                      pos += delta;
                 }
             } else {
                 if (row > 0)
                     segment << block.at(pos);
             }

             segment << keys.at(row);
         } else {
             QPointF init = block.at(0);
             int range = framesCount;
             if (row == 0) {
                 range--;
                 segment << init;
             } else {
                 init = keys.at(row-1);
             }

             if (row == 0 && range == 1)
                 segment << keys.at(row);
             else
                 segment.append(calculateSegmentPoints(init, keys.at(row), range));
         } 

         segments << segment;
         addTableRow(row, segment.count());
    }

    loadTweenPoints();
}

void StepsViewer::setPath(const QGraphicsPathItem *pathItem)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[StepsViewer::setPath()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (!pathItem) {
        #ifdef TUP_DEBUG
            QString msg = "StepsViewer::setPath() - Fatal Error: pathItem is NULL!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif

        return;
    }

    // Set of key points which define the path 
    path = pathItem->path();
    points = path.toFillPolygon();

    if (!points.isEmpty())
        points.removeLast();

    calculateKeys();

    if (records < keys.size()) { // A new table row must be added. Last segment must be calculated
        records = keys.size();
        int row = records - 1;

        QList<QPointF> segment;
        QPointF pInit;
        int framesCount = 4;
        int range = framesCount;
        if (row == 0) {
            framesCount++;
            pInit = points.at(0);
            segment << pInit;
        } else {
            pInit = keys.at(row-1);
        }

        frames << framesCount;
        addTableRow(row, framesCount);

        segment.append(calculateSegmentPoints(pInit, keys.at(row), range));
        segments << segment;
    } else { // A node was edited, segments must be recalculated
        // Recalculating segments
        calculateGroups();

        int total = frames.count();
        for (int row=0; row < total; row++) { // Processing every segment
             QList<QPointF> block = pointBlocks.at(row);
             int framesCount = frames.at(row);
             int size = block.size();
             QList<QPointF> segment; 

             if (size > 2) {
                 if (size < framesCount) { // There are less available points than path points
                     int range = size;
                     QList<QPointF> input = block;
                     while (range < framesCount) {
                            QList<QPointF> newBlock;
                            for (int i=0; i<input.size()-1; i++) {
                                 QPointF step = input.at(i+1) - input.at(i);
                                 QPointF middle = input.at(i) + QPointF(step.x()/2, step.y()/2);
                                 newBlock << input.at(i) << middle;
                            }
                            newBlock << input.last();
                            range = newBlock.size();
                            input = newBlock;
                     }
                     size = input.size();
                     block = input;
                 } 

                 if (row==0) {
                     framesCount--;
                     segment.append(block.at(0));
                 }

                 int delta = size/framesCount;
                 int pos = delta;
                 if (framesCount > 2) {
                     int modDelta = 0;
                     int module = size % framesCount;
                     if (module > 0)
                         modDelta = framesCount/module;

                     int modPos = 1;
                     int modCounter = 1;

                     for (int i=1; i < framesCount; i++) { // calculating points set for the segment j
                          if (module > 0) {
                              if (i == modPos && modCounter < module) {
                                  pos++;
                                  modPos += modDelta;
                                  modCounter++;
                              }
                          }
                          segment << block.at(pos);
                          pos += delta;
                     }
                 } else {
                     if (row > 0)
                         segment << block.at(pos);
                     else // when frames == 3
                         segment << block.at(size/2);
                 }

                 segment << keys.at(row);
             } else {
                 QPointF init = block.at(0);
                 if (row == 0) {
                     framesCount--;
                     segment << init;
                 } else {
                     init = keys.at(row-1);
                 }

                 if (row == 0 && framesCount == 1)
                     segment << keys.at(row);
                 else
                     segment.append(calculateSegmentPoints(init, keys.at(row), framesCount));
             }  

             segments.replace(row, segment);
        }
    }

    loadTweenPoints();
}

// +/- frames slot and text/input slot 
void StepsViewer::updatePathSection(int column, int row)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[StepsViewer::updatePathSection()]";
        #else
            T_FUNCINFO << " column: " <<  column << " - row: " << row;
        #endif
    #endif

    QTableWidgetItem *cell = item(row, 1);
    int framesCount = cell->text().toInt();

    calculateKeys();
    calculateGroups();

    QList<QPointF> block = pointBlocks.at(row);
    int range = block.size();

    if (column == 2) // Plus button clicked
        framesCount += 1;

    if (column == 3) // Minus button clicked 
        framesCount -= 1;

    if (row == 0) {
        if (framesCount < 2)
            framesCount = 2;
    } else {
        if (framesCount < 1)
            framesCount = 1;
    }

    frames.replace(row, framesCount);
    QList<QPointF> segment;

    if (range > 2) {
        if (range < framesCount) { // There are less available points than path points
            int size = range;
            QList<QPointF> input = block;
            while (size < framesCount) {
                   QList<QPointF> newBlock;
                   for (int i=0; i<input.size()-1; i++) {
                        QPointF step = input.at(i+1) - input.at(i);  
                        QPointF middle = input.at(i) + QPointF(step.x()/2, step.y()/2); 
                        newBlock << input.at(i) << middle;
                   }
                   newBlock << input.last();
                   size = newBlock.size(); 
                   input = newBlock;
            }

            range = input.size();
            block = input;
        }

        if (row==0) {
            framesCount--;
            segment.append(block.at(0));
        }
        int delta = range/framesCount;
        int pos = delta;

        if (framesCount > 2) {
            int module = range % framesCount;
            int modDelta = 0;  
            if (module > 0)
                modDelta = framesCount/module;
            int modPos = 1;
            int modCounter = 1;

            for (int i=1; i < framesCount; i++) { // calculating points set for the segment j
                 if (module > 0) {
                     if (i == modPos && modCounter < module) {
                         pos++;
                         modCounter++;
                         modPos += modDelta;
                     }
                 }
                 segment << block.at(pos);
                 pos += delta;
            }
        } else {
            if (row > 0)
                segment << block.at(pos);
            else  // when frames == 3
                segment << block.at(range/2);
        }

        segment << keys.at(row);
    } else {
        QPointF init = block.at(0);
        if (row == 0) {
            framesCount--;
            segment << init;
        } else {
            init = keys.at(row-1);
        }

        if (row == 0 && framesCount == 1)
            segment << keys.at(row);
        else 
            segment.append(calculateSegmentPoints(init, keys.at(row), framesCount));
    }

    cell->setText(QString::number(segment.count()));
    segments.replace(row, segment);

    loadTweenPoints();

    emit totalHasChanged(totalSteps());
}

QVector<TupTweenerStep *> StepsViewer::steps()
{
    QVector<TupTweenerStep *> stepsVector;
    int i = 0;
    foreach (QList<QPointF> segment, segments) {
        foreach (QPointF point, segment) {
            TupTweenerStep *step = new TupTweenerStep(i);
            step->setPosition(point);
            stepsVector << step;
            i++;
        }
    }

    return stepsVector;
}

int StepsViewer::totalSteps()
{
    int total = 0;
    for (int i=0; i < frames.count(); i++)
         total += frames.at(i);

    return total;
}

void StepsViewer::loadTweenPoints()
{
    tweenPointsList.clear();
    foreach (QList<QPointF> segment, segments) {
        foreach (QPointF point, segment) 
             tweenPointsList << point;
    }
}

QList<QPointF> StepsViewer::tweenPoints()
{
    return tweenPointsList;
}

QString StepsViewer::intervals()
{
    QString output = ""; 
    foreach(int interval, frames)
            output += QString::number(interval) + ",";

    output.chop(1);
    return output;
}

void StepsViewer::clearInterface()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[StepsViewer::clearInterface()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    records = 0;
    frames.clear();
    segments.clear();
    tweenPointsList.clear();

    int size = rowCount() - 1;
    for (int i=size; i >= 0; i--)
         removeRow(i);

    plusButton->clear();
    minusButton->clear();
}

QList<QPointF> StepsViewer::calculateSegmentPoints(QPointF begin, QPointF end, int total)
{
    QList<QPointF> pathPoints;

    qreal m = (end.y() - begin.y())/(end.x() - begin.x());
    qreal b = begin.y() - (m*begin.x());
    qreal delta = (end.x() - begin.x())/total; 
    qreal x = begin.x();

    QPointF dot;
    qreal y;
    for (int i=0; i<(total-1); i++) {
         x += delta;
         y = m*x + b;
         dot.setX(x);
         dot.setY(y);
         pathPoints.append(dot);
    }

    pathPoints.append(end);

    return pathPoints;
}

void StepsViewer::addTableRow(int row, int frames)  
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[StepsViewer::addTableRow()]";
        #else
            T_FUNCINFO;
            tWarning() << "row: " << row;
            tWarning() << "frames: " << frames;
        #endif
    #endif

    setRowCount(rowCount() + 1);

    QTableWidgetItem *intervalItem = new QTableWidgetItem();
    intervalItem->setTextAlignment(Qt::AlignCenter);
    intervalItem->setText(QString::number(row + 1));
    intervalItem->setFlags(intervalItem->flags() & ~Qt::ItemIsEditable);

    QTableWidgetItem *framesItem = new QTableWidgetItem();
    framesItem->setTextAlignment(Qt::AlignCenter);
    framesItem->setText(QString::number(frames));

    plusButton->append(new TPushButton(this, "+", 2, row));
    connect(plusButton->at(row), SIGNAL(clicked(int, int)), this, SLOT(updatePathSection(int, int)));
    minusButton->append(new TPushButton(this, "-", 3, row));
    connect(minusButton->at(row), SIGNAL(clicked(int, int)), this, SLOT(updatePathSection(int, int)));

    setItem(row, 0, intervalItem);
    setItem(row, 1, framesItem);
    setCellWidget(row, 2, plusButton->at(row));
    setCellWidget(row, 3, minusButton->at(row));

    setRowHeight(row, 20);
}

// Store all the points of the current path
void StepsViewer::calculateKeys()
{
    keys.clear();
    int total = path.elementCount();
    int count = 0;

    for (int i = 1; i < total; i++) {
         QPainterPath::Element e = path.elementAt(i);
         if (e.type != QPainterPath::CurveToElement) {
             if ((e.type == QPainterPath::CurveToDataElement) &&
                 (path.elementAt(i-1).type == QPainterPath::CurveToElement))
                 continue;

             QPointF point(e.x, e.y);
             keys.append(point);
             count++;
         }
    }
}

// Calculate blocks of points per segment 
void StepsViewer::calculateGroups()
{
    pointBlocks.clear();

    int index = 0;
    int total = points.size();
    QList<QPointF> segment;

    for (int i=0; i < total; i++) { // Counting points between keys and saving key indexes
         QPointF point = points.at(i);
         if (point == keys.at(index)) {
             segment << point;
             pointBlocks << segment;
             index++;
             segment = QList<QPointF>();
        } else {
             segment << point;
        }
    }
}

void StepsViewer::commitData(QWidget *editor)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[StepsViewer::commitData()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);

    if (lineEdit) {
        QString value = lineEdit->text();
        bool ok;
        int framesCount = value.toInt(&ok, 10);

        if (ok) {
            value = QString::number(framesCount);
            int row = currentRow();
            int column = currentColumn();
            QTableWidgetItem *cell = item(row, column);
            cell->setText(value);
            updatePathSection(column, row);
        } else {
            #ifdef TUP_DEBUG
                QString msg = "input value: " + value;
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tWarning() << msg;
                #endif
            #endif
        }
    }
}

void StepsViewer::undoSegment(const QPainterPath painterPath)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[StepsViewer::undoSegment()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    path = painterPath;

    points = path.toFillPolygon();
    if (!points.isEmpty())
        points.removeLast();

    calculateKeys();
    calculateGroups();

    if (!frames.isEmpty()) {
        undoFrames << frames.last();
        frames.removeLast();
    }

    records--;

    if (!segments.isEmpty()) {
        undoSegments << segments.last();
        segments.removeLast();
    }

    updateSegments();

    if (!plusButton->isEmpty())
        plusButton->removeLast();

    if (!minusButton->isEmpty())
        minusButton->removeLast();

    removeRow(rowCount()-1);

    emit totalHasChanged(totalSteps());
}

void StepsViewer::redoSegment(const QPainterPath painterPath)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[StepsViewer::redoSegment()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    path = painterPath;

    points = path.toFillPolygon();
    points.removeLast();

    calculateKeys();
    calculateGroups();

    frames << undoFrames.last();
    undoFrames.removeLast();
    records++;

    segments << undoSegments.last();
    undoSegments.removeLast();

    updateSegments();

    addTableRow(rowCount(), frames.last());
}

void StepsViewer::updateSegments(const QPainterPath painterPath)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[StepsViewer::undoSegment()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    path = painterPath;

    points = path.toFillPolygon();
    if (!points.isEmpty())
        points.removeLast();

    calculateKeys();
    calculateGroups();
    updateSegments();
}

void StepsViewer::updateSegments()
{
    int total = frames.count();
    for (int row=0; row < total; row++) { // Processing every segment
         QList<QPointF> block = pointBlocks.at(row);
         int framesCount = frames.at(row);
         int size = block.size();
         QList<QPointF> segment; 

         if (size > 2) {
             if (size < framesCount) { // There are less available points than path points
                 int range = size;
                 QList<QPointF> input = block;
                 while (range < framesCount) {
                        QList<QPointF> newBlock;
                        for (int i=0; i<input.size()-1; i++) {
                             QPointF step = input.at(i+1) - input.at(i);
                             QPointF middle = input.at(i) + QPointF(step.x()/2, step.y()/2);
                             newBlock << input.at(i) << middle;
                        }
                        newBlock << input.last();
                        range = newBlock.size();
                        input = newBlock;
                 }
                 size = input.size();
                 block = input;
             } 

             if (row==0) {
                 framesCount--;
                 segment.append(block.at(0));
             }

             int delta = size/framesCount;
             int pos = delta;
             if (framesCount > 2) {
                 int modDelta = 0;
                 int module = size % framesCount;
                 if (module > 0)
                     modDelta = framesCount/module;

                 int modPos = 1;
                 int modCounter = 1;

                 for (int i=1; i < framesCount; i++) { // calculating points set for the segment j
                      if (module > 0) {
                          if (i == modPos && modCounter < module) {
                              pos++;
                              modPos += modDelta;
                              modCounter++;
                          }
                      }
                      segment << block.at(pos);
                      pos += delta;
                 }
             } else {
                 if (row > 0)
                     segment << block.at(pos);
                 else // when frames == 3
                     segment << block.at(size/2);
             }

             segment << keys.at(row);
         } else {
             QPointF init = block.at(0);
             if (row == 0) {
                 framesCount--;
                 segment << init;
             } else {
                 init = keys.at(row-1);
             }

             if (row == 0 && framesCount == 1)
                 segment << keys.at(row);
             else
                 segment.append(calculateSegmentPoints(init, keys.at(row), framesCount));
         }  

         segments.replace(row, segment);
    }

    loadTweenPoints();
}



