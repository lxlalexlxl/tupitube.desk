/***************************************************************************
 *   Project TUPI: Magia 2D                                                *
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
 *   the Free Software Foundation; either version 3 of the License, or     *
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

#include "ktitemtweener.h"
#include "ktsvg2qt.h"
#include "kdebug.h"

#include <QGraphicsItem>
#include <QHash>

#define VERIFY_STEP(index) if (index > k->frames || k->frames == 0) { \
                               kWarning("items") << "Invalid step " << index << " for tweening, maximun step are " << k->frames << "; In " << __FUNCTION__; \
                               return; }

#define STEP(index) index / (double)k->frames

struct KTItemTweener::Private
{
    Private() : frames(0) {}
    
    QString name;
    KTItemTweener::Type type;
    int initFrame;
    int frames;
    QPointF originPoint;

    // Position Tween
    QString path;

    // Rotation Tween
    KTItemTweener::RotationType rotationType;
    KTItemTweener::RotateDirection rotateDirection;
    int rotateSpeed;
    int rotateLoop;
    int rotateReverseLoop;
    int rotateStartDegree;
    int rotateEndDegree;

    // Scale Tween
    KTItemTweener::ScaleAxes scaleAxes;
    double scaleFactor;
    int scaleIterations;
    int scaleLoop;
    int scaleReverseLoop;

    // Opacity Tween
    double initOpacityFactor;
    double endOpacityFactor;
    int opacityIterations;
    int opacityLoop;
    int opacityReverseLoop;

    // Color Tween
    QColor initialColor;
    QColor endingColor;
    int colorIterations;
    int colorLoop;
    int colorReverseLoop;

    QHash<int, KTTweenerStep *> steps; // TODO: remove when Qt 4.3

    inline KTTweenerStep *step(int step)
    {
        KTTweenerStep *currentStep = steps[step];
        if (!currentStep) {
            currentStep = new KTTweenerStep(step);
            steps.insert(step, currentStep);
        }
        
        return currentStep;
    }
};

KTItemTweener::KTItemTweener() : k(new Private)
{
    k->frames = 0;
}

KTItemTweener::~KTItemTweener()
{
    qDeleteAll(k->steps);
    delete k;
}

QString KTItemTweener::name()
{
    return k->name;
}

KTItemTweener::Type KTItemTweener::type()
{
    return k->type;
}

void KTItemTweener::addStep(const KTTweenerStep &step)
{
    int counter = step.index();
    
    VERIFY_STEP(counter);
    
    if (step.has(KTTweenerStep::Position))
        setPosAt(counter, step.position());
    
    if (step.has(KTTweenerStep::Scale))
        setScaleAt(counter, step.horizontalScale(), step.verticalScale());
    
    if (step.has(KTTweenerStep::Translation))
        setTranslationAt(counter, step.xTranslation(), step.yTranslation());
    
    if (step.has(KTTweenerStep::Shear))
        setScaleAt(counter, step.horizontalShear(), step.verticalShear());
    
    if (step.has(KTTweenerStep::Rotation))
        setRotationAt(counter, step.rotation());

    if (step.has(KTTweenerStep::Opacity))
        setOpacityAt(counter, step.opacity());
}

KTTweenerStep * KTItemTweener::stepAt(int index)
{
    return k->step(index);
}

void KTItemTweener::setPosAt(int index, const QPointF &pos)
{
    VERIFY_STEP(index);
    k->step(index)->setPosition(pos);
}

void KTItemTweener::setScaleAt(int index, double sx, double sy)
{
    VERIFY_STEP(index);
    k->step(index)->setScale(sx, sy);
}

void KTItemTweener::setTranslationAt(int index, double x, double y)
{
    VERIFY_STEP(index);
    k->step(index)->setTranslation(x, y);
}

void KTItemTweener::setRotationAt(int index, double angle)
{
    VERIFY_STEP(index);
    k->step(index)->setRotation(angle);
}

void KTItemTweener::setOpacityAt(int index, double opacity)
{
    VERIFY_STEP(index);
    k->step(index)->setOpacity(opacity);
}

void KTItemTweener::setColorAt(int index, int red, int green, int blue)
{
    VERIFY_STEP(index);
    k->step(index)->setColor(red, green, blue);
}

void KTItemTweener::setFrames(int frames)
{
    k->frames = frames;
}

int KTItemTweener::frames() const
{
    return k->frames;
}

int KTItemTweener::startFrame()
{
    return k->initFrame;
}

QPointF KTItemTweener::transformOriginPoint()
{
    return k->originPoint;
}

void KTItemTweener::fromXml(const QString &xml)
{
    QDomDocument doc;

    if (doc.setContent(xml)) {
        QDomElement root = doc.documentElement();

        k->name = root.attribute("name");
        k->type = KTItemTweener::Type(root.attribute("type").toInt());
        k->initFrame = root.attribute("init").toInt();
        k->frames = root.attribute("frames").toInt();

        QString origin = root.attribute("origin"); // [x,y]
        QStringList list = origin.split(",");
        double x = list.first().toDouble();
        double y = list.last().toDouble();

        k->originPoint = QPointF(x, y); 

        if (k->type == KTItemTweener::Position || k->type == KTItemTweener::All)
            k->path = root.attribute("coords");

        if (k->type == KTItemTweener::Rotation || k->type == KTItemTweener::All) {
            k->rotationType = KTItemTweener::RotationType(root.attribute("rotationType").toInt()); 
            kFatal() << "KTItemTweener::fromXml() - Rotation Type: " << k->rotationType;

            k->rotateSpeed = root.attribute("rotateSpeed").toInt();

            if (k->rotationType == KTItemTweener::Continuos) {
                k->rotateDirection = KTItemTweener::RotateDirection(root.attribute("rotateDirection").toInt());
            } else if (k->rotationType == KTItemTweener::Partial) {
                       k->rotateLoop = root.attribute("rotateLoop").toInt();
                       k->rotateStartDegree = root.attribute("rotateStartDegree").toInt();
                       k->rotateEndDegree = root.attribute("rotateEndDegree").toInt();
                       k->rotateReverseLoop = root.attribute("rotateReverseLoop").toInt();
            }
        }

        if (k->type == KTItemTweener::Scale || k->type == KTItemTweener::All) {
            k->scaleAxes = KTItemTweener::ScaleAxes(root.attribute("scaleAxes").toInt()); 
            k->scaleFactor = root.attribute("scaleFactor").toDouble(); 
            k->scaleIterations = root.attribute("scaleIterations").toInt();
            k->scaleLoop = root.attribute("scaleLoop").toInt();
            k->scaleReverseLoop = root.attribute("scaleReverseLoop").toInt();
        }

        if (k->type == KTItemTweener::Opacity || k->type == KTItemTweener::All) {
            k->initOpacityFactor = root.attribute("initOpacityFactor").toDouble();
            k->endOpacityFactor = root.attribute("endOpacityFactor").toDouble();
            k->opacityIterations = root.attribute("opacityIterations").toInt();
            k->opacityLoop = root.attribute("opacityLoop").toInt();
            k->opacityReverseLoop = root.attribute("opacityReverseLoop").toInt();
        }

        QDomNode node = root.firstChild();
        
        while (!node.isNull()) {
               QDomElement e = node.toElement();
            
               if (!e.isNull()) {
                   if (e.tagName() == "step") {
                       QString stepDoc;
                       {
                           QTextStream ts(&stepDoc);
                           ts << node;
                       }
                    
                       KTTweenerStep *step = new KTTweenerStep(0);
                       step->fromXml(stepDoc);
                    
                       addStep(*step);
                    
                       delete step;
                   }
               }
            
               node = node.nextSibling();
        }
    }
}

QDomElement KTItemTweener::toXml(QDomDocument &doc) const
{
    QDomElement root = doc.createElement("tweening");
    root.setAttribute("name", k->name);
    root.setAttribute("type", k->type);
    root.setAttribute("init", k->initFrame);
    root.setAttribute("frames", k->frames);
    root.setAttribute("origin", QString::number(k->originPoint.x()) + "," + QString::number(k->originPoint.y()));

    if (k->type == KTItemTweener::Position || k->type == KTItemTweener::All) {
        root.setAttribute("coords", k->path);
    }

    if (k->type == KTItemTweener::Rotation || k->type == KTItemTweener::All) {
        kFatal() << "KTItemTweener::toXml() - Rotation Type: " << k->rotationType;
        root.setAttribute("rotationType", k->rotationType);
        root.setAttribute("rotateSpeed", k->rotateSpeed);

        if (k->rotationType == KTItemTweener::Continuos) {
            root.setAttribute("rotateDirection", k->rotateDirection); 
        } else if (k->rotationType == KTItemTweener::Partial) {
                   root.setAttribute("rotateLoop", k->rotateLoop);
                   root.setAttribute("rotateStartDegree", k->rotateStartDegree);
                   root.setAttribute("rotateEndDegree", k->rotateEndDegree); 
                   root.setAttribute("rotateReverseLoop", k->rotateReverseLoop);
        }
    }

    if (k->type == KTItemTweener::Scale || k->type == KTItemTweener::All) {
        root.setAttribute("scaleAxes", k->scaleAxes);
        root.setAttribute("scaleFactor", k->scaleFactor);
        root.setAttribute("scaleIterations", k->scaleIterations);
        root.setAttribute("scaleLoop", k->scaleLoop);
        root.setAttribute("scaleReverseLoop", k->scaleReverseLoop);
    }

    if (k->type == KTItemTweener::Opacity || k->type == KTItemTweener::All) {
        root.setAttribute("initOpacityFactor", k->initOpacityFactor); 
        root.setAttribute("endOpacityFactor", k->endOpacityFactor); 
        root.setAttribute("opacityIterations", k->opacityIterations);
        root.setAttribute("opacityLoop", k->opacityLoop);
        root.setAttribute("opacityReverseLoop", k->opacityReverseLoop);
    } 
 
    foreach (KTTweenerStep *step, k->steps.values())
             root.appendChild(step->toXml(doc));
    
    return root;
}

QGraphicsPathItem *KTItemTweener::graphicsPath() const
{
    QGraphicsPathItem *item = new QGraphicsPathItem();
    QPainterPath path;
    KTSvg2Qt::svgpath2qtpath(k->path, path);

    item->setPath(path);

    return item;
}

QString KTItemTweener::tweenType()
{
    QString type = ""; 
    switch (k->type) {
            case KTItemTweener::Position :
                 type = QString(tr("Position Tween"));
                 break;
            case KTItemTweener::Rotation :
                 type = QString(tr("Rotation Tween"));
                 break;
            case KTItemTweener::Scale :
                 type = QString(tr("Scale Tween"));
                 break;
            case KTItemTweener::Shear :
                 type = QString(tr("Shear Tween"));
                 break;
            case KTItemTweener::Opacity :
                 type = QString(tr("Opacity Tween"));
                 break;
            case KTItemTweener::Coloring :
                 type = QString(tr("Coloring Tween"));
                 break;
            case KTItemTweener::All :
                 type = QString(tr("Compound Tween"));
                 break;

    }

    return type;
}

KTItemTweener::RotationType KTItemTweener::tweenRotationType()
{
    return k->rotationType;
}

int KTItemTweener::tweenRotateSpeed()
{
    return k->rotateSpeed;
}

bool KTItemTweener::tweenRotateLoop()
{
    return k->rotateLoop;
}

KTItemTweener::RotateDirection KTItemTweener::tweenRotateDirection()
{
    return k->rotateDirection;
}

int KTItemTweener::tweenRotateStartDegree()
{
    return k->rotateStartDegree;
}

int KTItemTweener::tweenRotateEndDegree()
{
    return k->rotateEndDegree;
}

bool KTItemTweener::tweenRotateReverseLoop()
{
    return k->rotateReverseLoop;
}

KTItemTweener::ScaleAxes KTItemTweener::tweenScaleAxes()
{
    return k->scaleAxes;
}

double KTItemTweener::tweenScaleFactor()
{
    return k->scaleFactor;
}

int KTItemTweener::tweenScaleIterations()
{
    return k->scaleIterations;
}

int KTItemTweener::tweenScaleLoop()
{
    return k->scaleLoop;
}

int KTItemTweener::tweenScaleReverseLoop()
{
    return k->scaleReverseLoop;
}

double KTItemTweener::tweenOpacityInitialFactor()
{
    return k->initOpacityFactor;
}

double KTItemTweener::tweenOpacityEndingFactor()
{
    return k->endOpacityFactor;
}

int KTItemTweener::tweenOpacityIterations()
{
    return k->opacityIterations;
}

int KTItemTweener::tweenOpacityLoop() 
{
    return k->opacityLoop;
}

int KTItemTweener::tweenOpacityReverseLoop()
{
    return k->opacityReverseLoop;
}

QColor KTItemTweener::tweenInitialColor()
{
    return k->initialColor;
}

QColor KTItemTweener::tweenEndingColor()
{
    return k->endingColor;
}

int KTItemTweener::tweenColorIterations()
{
    return k->colorIterations; 
}

int KTItemTweener::tweenColorLoop()
{
    return k->colorLoop;
}

int KTItemTweener::tweenColorReverseLoop()
{
    return k->colorReverseLoop;
}

