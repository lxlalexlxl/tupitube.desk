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

#include "tupscene.h"
#include "tupgraphicobject.h"
#include "tupsvgitem.h"
#include "tupsoundlayer.h"
#include "tupitemgroup.h"
#include "tupprojectloader.h"
#include "tupitemfactory.h"

TupScene::TupScene(TupProject *parent, int index, const QSize size, const QColor &color) : QObject(parent)
{
    sceneIndex = index;
    dimension = size;
    bgColor = color;
    fps = 24;
    isLocked = false;
    layerCount = 0;
    layers = Layers();
    isVisible = true;

    storyboard = new TupStoryboard;
    background = new TupBackground(this, sceneIndex, size);
}

TupScene::~TupScene()
{
    delete storyboard;
    delete background;

    tweeningGraphicObjects.clear();
    tweeningSvgObjects.clear();
    layers.clear();
    undoLayers.clear();
    soundLayers.clear();
}

void TupScene::setSceneName(const QString &name)
{
    sceneName = name;
}

void TupScene::setFPS(const int value)
{
    fps = value;
}

int TupScene::getFPS()
{
    return fps;
}

void TupScene::setBgColor(const QColor color)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::setBgColor()] - color -> " << color.name();
    #endif

    bgColor = color;
}

QColor TupScene::getBgColor() const
{
    return bgColor;
}

void TupScene::setSceneLocked(bool locked)
{
    isLocked = locked;
}

QString TupScene::getSceneName() const
{
    return sceneName;
}

bool TupScene::isSceneLocked() const
{
    return isLocked;
}

void TupScene::setVisibility(bool visible)
{
    isVisible = visible;
}

bool TupScene::isSceneVisible() const
{
    return isVisible;
}

Layers TupScene::getLayers() const
{
    return layers;
}

int TupScene::layersCount() const
{
    return layerCount;
}

SoundLayers TupScene::getSoundLayers() const
{
    return soundLayers;
}

QSize TupScene::getDimension()
{
    return dimension;
}

void TupScene::setBasicStructure()
{
    layerCount++;
    TupLayer *layer = new TupLayer(this, layerCount-1);
    layer->setLayerName(tr("Layer 1"));
    layer->createFrame(tr("Frame"), 0, false);

    layers.insert(0, layer);
}

// Set the layers list, this function overwrites the old layers

void TupScene::setLayers(const Layers &sLayers)
{
    layers = sLayers;
}

TupLayer *TupScene::createLayer(QString name, int position, bool loadingFromFile)
{
    if (position < 0 || position > layers.count()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupScene::createLayer()] - Invalid index -> " << position;
        #endif    
        
        return nullptr;
    }

    layerCount++;
    TupLayer *layer = new TupLayer(this, layerCount - 1);
    layer->setLayerName(name);
    layers.insert(position, layer);

    if (loadingFromFile)
        TupProjectLoader::createLayer(project()->visualIndexOf(this), position, layer->getLayerName(), project());

    return layer;
}

void TupScene::addLayer(const QString &xml)
{
    TupLayer *layer = new TupLayer(this, layers.count());
    layer->fromXml(xml);
    layers << layer;
}

TupSoundLayer *TupScene::createSoundLayer(int position, bool loadingFromFile)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::createSoundLayer()] - position: " << position;
    #endif    
    
    if (position < 0 || position > soundLayers.count()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupScene::createSoundLayer()] - Fatal Error: Index incorrect!";
        #endif        
        return nullptr;
    }

    TupSoundLayer *layer = new TupSoundLayer(this);
    layerCount++;

    layer->setLayerName(tr("Sound layer %1").arg(layerCount));

    soundLayers.insert(position, layer);

    if (loadingFromFile)
        TupProjectLoader::createSoundLayer(objectIndex(), position, layer->getLayerName(), project());

    return layer;
}

bool TupScene::restoreLayer(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::restoreLayer()] - index -> " << index;
    #endif

    if (undoLayers.count() > 0) {
        TupLayer *layer = undoLayers.takeLast();
        if (layer) {
            if (layer->framesCount() == 0)
                layer->restoreFrame(0);
            layers.insert(index, layer);
            layerCount++;
            return true;
        }
        return false;
    }

    return false;
}

bool TupScene::removeLayer(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::removeLayer()] - layers total -> " << layers.size();
        qDebug() << "[TupScene::removeLayer()] - index -> " << index;
    #endif

    if (!layers.isEmpty()) {
        TupLayer *layer = this->layerAt(index);
        if (layer) {
            // removeTweensFromLayer(index + 1);
            removeTweensFromLayer(index);
            undoLayers << layers.takeAt(index);
            layerCount--;

            return true;
        }
    }

    return false;
}

// Return the layer at the index requested

TupLayer *TupScene::layerAt(int index) const
{
    if (index < 0 || index >= layers.count()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupScene::layerAt()] - Fatal Error: LAYERS TOTAL -> " << layers.count();
            qDebug() << "[TupScene::layerAt()] - Fatal Error: index out of bound -> " << index;
            qDebug() << "[TupScene::layerAt()] - Fatal Error: The layer requested doesn't exist anymore";
        #endif

        return nullptr;
    }

    return layers.value(index);
}

TupSoundLayer *TupScene::soundLayer(int index) const
{
    if (index < 0 || index >= soundLayers.count()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupScene::soundLayer()] - Fatal Error: index out of bound -> " << index;
        #endif

        return nullptr;
    }

    return soundLayers.value(index);
}

void TupScene::fromXml(const QString &xml)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::fromXml()]";
    #endif

    QDomDocument doc;
    if (!doc.setContent(xml)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupScene::fromXml()] - Error while processing XML file! -> " << xml;
        #endif

        return;
    }

    QDomElement root = doc.documentElement();
    setSceneName(root.attribute("name"));
    setFPS(root.attribute("fps", "24").toInt());

    QString color = root.attribute("bgcolor");
    if (!color.isEmpty())
        setBgColor(QColor(color));

    QDomNode n = root.firstChild();
    while (!n.isNull()) {
           QDomElement e = n.toElement();
           if (!e.isNull()) {
               if (e.tagName() == "layer") {
                   int layerIndex = layers.count();
                   TupLayer *layer = createLayer(e.attribute("name"), layerIndex, true);

                   if (layer) {
                       QString newDoc;
                       {
                         QTextStream ts(&newDoc);
                         ts << n;
                       }
                       layer->fromXml(newDoc);
                   }
               } else if (e.tagName() == "background") {
                   QString newDoc;
                   {
                     QTextStream ts(&newDoc);
                     ts << n;
                   }
                   background->fromXml(newDoc);
               } else if (e.tagName() == "soundlayer") {
                   int pos = soundLayers.count();
                   TupSoundLayer *layer = createSoundLayer(pos, true);

                   if (layer) {
                       QString newDoc;
                        {
                          QTextStream ts(&newDoc);
                          ts << n;
                        }
                       layer->fromXml(newDoc);
                   }
               } else if (e.tagName() == "storyboard") {
                   QString newDoc;
                   {
                     QTextStream ts(&newDoc);
                     ts << n;
                   }
                   storyboard->fromXml(newDoc);
               }
           }

           n = n.nextSibling();

    } // end while
}

QDomElement TupScene::toXml(QDomDocument &doc) const
{
    QDomElement root = doc.createElement("scene");
    root.setAttribute("name", sceneName);
    root.setAttribute("fps", fps);
    root.setAttribute("bgcolor", bgColor.name());

    root.appendChild(storyboard->toXml(doc));
    root.appendChild(background->toXml(doc));

    int total = layers.size();
    for (int i = 0; i < total; ++i)
        root.appendChild(layers.at(i)->toXml(doc));

    total = soundLayers.size();
    for (int i = 0; i < total; ++i)
        root.appendChild(soundLayers.at(i)->toXml(doc));

    return root;
}

bool TupScene::moveLayer(int from, int to)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::moveLayer()] - from -> " << from;
        qDebug() << "[TupScene::moveLayer()] - to -> " << to;
    #endif

    if (from < 0 || from >= layers.count() || to < 0 || to >= layers.count()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupScene::moveLayer()] - Fatal Error: Layer index out of bound -> " << to;
        #endif

        return false;
    }

    TupLayer *sourceLayer = layers[from];
    sourceLayer->updateLayerIndex(to + 1);

    TupLayer *targetLayer = layers[to];
    targetLayer->updateLayerIndex(from + 1);

    Frames frames = sourceLayer->getFrames(); 
    int totalFrames = frames.size();
    int zLevelIndex = (to + BG_LAYERS) * ZLAYER_LIMIT;
    for (int i = 0; i < totalFrames; i++) {
         TupFrame *frame = frames.at(i);
         frame->updateZLevel(zLevelIndex);
    }

    frames = targetLayer->getFrames();
    totalFrames = frames.size();
    zLevelIndex = (from + BG_LAYERS) * ZLAYER_LIMIT;
    for (int i = 0; i < totalFrames; i++) {
         TupFrame *frame = frames.at(i);
         frame->updateZLevel(zLevelIndex);
    }

    layers.swapItemsAt(from, to);

    return true;
}

int TupScene::objectIndex() const
{
    if (TupProject *project = dynamic_cast<TupProject *>(parent()))
        return project->visualIndexOf(const_cast<TupScene *>(this));

    return -1;
}

int TupScene::visualIndexOf(TupLayer *layer) const
{
    return layers.indexOf(layer);
}

TupProject *TupScene::project() const
{
    return static_cast<TupProject *>(parent());
}

void TupScene::addTweenObject(int layerIndex, TupGraphicObject *object)
{
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->addTweenObject(object);
}

void TupScene::addTweenObject(int layerIndex, TupSvgItem *object)
{
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->addTweenObject(object);
}

void TupScene::updateTweenObject(int layerIndex, int objectIndex, TupGraphicObject *object)
{
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->updateTweenObject(objectIndex, object);
}

void TupScene::updateTweenObject(int layerIndex, int objectIndex, TupSvgItem *object)
{
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->updateTweenObject(objectIndex, object);
}

void TupScene::removeTweenObject(int layerIndex, TupGraphicObject *object)
{
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->removeTweenObject(object);
}

void TupScene::removeTweenObject(int layerIndex, TupSvgItem *object)
{
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->removeTweenObject(object);
}

QList<TupGraphicObject *> TupScene::getTweeningGraphicObjects(int layerIndex) const
{
    QList<TupGraphicObject *> list;
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        list = layer->getTweeningGraphicObjects();

    return list;
}

QList<TupSvgItem *> TupScene::getTweeningSvgObjects(int layerIndex) const
{
    QList<TupSvgItem *> list;
    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        list = layer->getTweeningSvgObjects();

    return list;
}

bool TupScene::tweenExists(const QString &name, TupItemTweener::Type type)
{
    if (layers.count()) {
        foreach(TupLayer *layer, layers) {
            QList<TupGraphicObject *> objectList = layer->getTweeningGraphicObjects();
            foreach (TupGraphicObject *object, objectList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                        return true;
                }
            }

            QList<TupSvgItem *> svgList = layer->getTweeningSvgObjects();
            foreach (TupSvgItem *object, svgList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                        return true;
                }
            }
        }
    }

    return false;
}

bool TupScene::removeTween(const QString &name, TupItemTweener::Type type)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::removeTween()] - name ->" << name;
    #endif

    bool found = false;
    if (layers.count()) {
        foreach(TupLayer *layer, layers) {
            QList<TupGraphicObject *> objectList = layer->getTweeningGraphicObjects();
            foreach (TupGraphicObject *object, objectList) {
                QList<TupItemTweener *> list = object->tweensList();
                int total = list.count();
                for (int i=0; i < total; i++) {
                     TupItemTweener *tween = list.at(i);
                     if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type)) {
                         object->removeTween(i);
                         if (total == 1)
                             removeTweenObject(layer->layerIndex(), object);
                         found = true;
                     }
                }
            }

            QList<TupSvgItem *> svgList = layer->getTweeningSvgObjects();
            foreach (TupSvgItem *object, svgList) {
                QList<TupItemTweener *> list = object->tweensList();
                int total = list.count();
                for (int i=0; i < total; i++) {
                     TupItemTweener *tween = list.at(i);
                     if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type)) {
                         object->removeTween(i);
                         if (total == 1)
                             removeTweenObject(layer->layerIndex(), object);
                         found = true;
                     }
                }
            }
        }
    }

    return found;
}

void TupScene::removeTweensFromLayer(int layerIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::removeTweensFromLayer()]";
    #endif

    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->removeAllTweens();
}

void TupScene::removeTweensFromFrame(int layerIndex, int frameIndex)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::removeTweensFromFrame()]" << frameIndex;
    #endif

    TupLayer *layer = layerAt(layerIndex);
    if (layer)
        layer->removeTweensFromFrame(frameIndex);
}

TupItemTweener *TupScene::tween(const QString &name, TupItemTweener::Type type)
{
    if (!layers.isEmpty()) {
        foreach(TupLayer *layer, layers) {
            QList<TupGraphicObject *> objectList = layer->getTweeningGraphicObjects();
            foreach (TupGraphicObject *object, objectList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                        return tween;
                }
            }

            QList<TupSvgItem *> svgList = layer->getTweeningSvgObjects();
            foreach (TupSvgItem *object, svgList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                        return tween;
                }
            }
        }
    }

    return nullptr;
}

QList<QString> TupScene::getTweenNames(TupItemTweener::Type type)
{
    QList<QString> names;
    if (!layers.isEmpty()) {
        foreach(TupLayer *layer, layers) {
            QList<TupGraphicObject *> objectList = layer->getTweeningGraphicObjects();
            foreach (TupGraphicObject *object, objectList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if (tween->getType() == type && !names.contains(tween->getTweenName()))
                        names.append(tween->getTweenName());
                }
            }

            QList<TupSvgItem *> svgList = layer->getTweeningSvgObjects();
            foreach (TupSvgItem *object, svgList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if (tween->getType() == type && !names.contains(tween->getTweenName()))
                         names.append(tween->getTweenName());
                }
            }
        }
    }

    return names;
}

QList<QGraphicsItem *> TupScene::getItemsFromTween(const QString &name, TupItemTweener::Type type)
{
    QList<QGraphicsItem *> items;
    if (layers.count()) {
        foreach(TupLayer *layer, layers) {
            QList<TupGraphicObject *> objectList = layer->getTweeningGraphicObjects();
            foreach (TupGraphicObject *object, objectList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                        items.append(object->item());
                }
            }

            QList<TupSvgItem *> svgList = layer->getTweeningSvgObjects();
            foreach (TupSvgItem *object, svgList) {
                QList<TupItemTweener *> list = object->tweensList();
                foreach(TupItemTweener *tween, list) {
                    if ((tween->getTweenName().compare(name) == 0) && (tween->getType() == type))
                        items.append(object);
                }
            }
        }
    }

    return items;
}

int TupScene::framesCount()
{
    int total = 0;
    if (layers.count()) {
        foreach (TupLayer *layer, layers) {
            int frames = layer->framesCount();
            if (frames > total)
                total = frames;
        }
    }

    return total;
}

bool TupScene::frameIsEmpty(int layerIndex, int frameIndex)
{
    TupLayer *layer = layerAt(layerIndex);
    if (layer) {
        TupFrame *frame = layer->frameAt(frameIndex);
        return frame->isEmpty();
    }

    return true;
}

TupBackground* TupScene::sceneBackground()
{
    return background;
}

void TupScene::setSceneBackground(TupBackground *bg)
{
    background = bg;
}

void TupScene::reset(const QString &name)
{
    sceneName = name;
    bgColor = Qt::white;
    background = new TupBackground(this, sceneIndex, dimension);
    layers.clear();
    tweeningGraphicObjects.clear();
    tweeningSvgObjects.clear();

    layerCount = 1;
    TupLayer *layer = new TupLayer(this, 0);
    layer->setLayerName(tr("Layer %1").arg(1));
    layer->createFrame(tr("Frame"), 0, false);

    layers.insert(0, layer);
}

void TupScene::clear()
{
    if (background) {
        background->clearBackground();

        background = nullptr;
        delete background;
    }

    for (int i=0; i<layers.count(); i++) {
         TupLayer *layer = layers.takeAt(i);
         layer->clear();

         layer = nullptr;
         delete layer;
    }

    layerCount = 1;
    layers.clear();
    tweeningGraphicObjects.clear();
    tweeningSvgObjects.clear();
}

void TupScene::setStoryboard(TupStoryboard *sb)
{
    storyboard = sb;
}

TupStoryboard * TupScene::getStoryboard()
{
    return storyboard;
}

void TupScene::insertStoryboardScene(int index)
{
    storyboard->insertScene(index);
}

void TupScene::appendStoryboardScene()
{
    storyboard->appendScene();
}

void TupScene::moveStoryboardScene(int oldIndex, int newIndex)
{
    storyboard->moveScene(oldIndex, newIndex);
}

void TupScene::resetStoryboardScene(int index)
{
    storyboard->resetScene(index);
}

void TupScene::removeStoryboardScene(int index)
{
    storyboard->removeScene(index);
}

QList<QString> TupScene::getLipSyncNames()
{
    QList<QString> names;

    if (layers.count()) {
        // foreach (TupLayer *layer, layers) {
        for (int i=0; i < layers.size(); i++) {
            TupLayer *layer = layers.at(i);
            if (layer->lipSyncCount() > 0) {
                Mouths mouths = layer->getLipSyncList();
                foreach (TupLipSync *lipsync, mouths)
                    names << lipsync->getLipSyncName();
            }
        }
    }

    return names;
}

bool TupScene::lipSyncExists(const QString &name)
{
    if (layers.count()) {
        foreach (TupLayer *layer, layers) {
            if (layer->lipSyncCount() > 0) {
                Mouths mouths = layer->getLipSyncList();
                foreach (TupLipSync *lipsync, mouths) {
                    if (lipsync->getLipSyncName().compare(name) == 0)
                        return true;
                }
            }
        }
    }

    return false;
}

int TupScene::getLipSyncLayerIndex(const QString &name)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::getLipSyncLayerIndex()] - name -> " << name;
    #endif

    int index = 0;
    if (layers.count()) {
        for (int i=0; i < layers.size(); i++) {
            TupLayer *layer = layers.at(i);
            if (layer->lipSyncCount() > 0) {
                Mouths mouths = layer->getLipSyncList();
                foreach (TupLipSync *lipsync, mouths) {
                    if (lipsync->getLipSyncName().compare(name) == 0) {
                        index = i;
                        break;
                    }
                }
            } else {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupScene::getLipSyncLayerIndex()] - Warning: No lip-sync records at layer -> " << i;
                #endif
            }
        }
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScene::getLipSyncLayerIndex()] - Warning: No layers available!";
        #endif
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::getLipSyncLayerIndex()] - index -> " << index;
    #endif

    return index;
}

TupLipSync * TupScene::getLipSync(const QString &name)
{
    TupLipSync *record = nullptr;

    if (layers.count()) {
        foreach (TupLayer *layer, layers) {
            if (layer->lipSyncCount() > 0) {
                Mouths mouths = layer->getLipSyncList();
                foreach (TupLipSync *lipsync, mouths) {
                    if (lipsync->getLipSyncName().compare(name) == 0)
                        return lipsync;
                }
            }
        }
    }

    return record;
}

bool TupScene::updateLipSync(TupLipSync *lipsync)
{
    QString name = lipsync->getLipSyncName();
    #ifdef TUP_DEBUG
        qDebug() << "[TupScene::updateLipSync()] - Lip-sync name -> " << name;
    #endif

    if (layers.count()) {
        QString name = lipsync->getLipSyncName();
        foreach (TupLayer *layer, layers) {
            if (layer->lipSyncCount() > 0) {
                Mouths mouths = layer->getLipSyncList();
                for (int i=0; i < mouths.size(); i++) {
                    TupLipSync *record = mouths.at(i);
                     if (record->getLipSyncName().compare(name) == 0)
                         return layer->updateLipSync(i, lipsync);
                }
            }
        }
    }

    #ifdef TUP_DEBUG
        qWarning() << "[TupScene::updateLipSync()] - Fatal Error: Can't update lip-sync record -> " << name;
    #endif

    return false;
}

bool TupScene::removeLipSync(const QString &name)
{
    if (layers.count()) {
        foreach (TupLayer *layer, layers) {
            if (layer->removeLipSync(name))
                return true;
        }
    }

    return false;
}

int TupScene::lipSyncTotal()
{
    int total = 0;
    if (layers.count()) {
        foreach (TupLayer *layer, layers)
            total += layer->lipSyncCount();
    }
    return total;
}

Mouths TupScene::getLipSyncList()
{
    Mouths list;
    if (layers.count()) {
        foreach (TupLayer *layer, layers) {
            if (layer->lipSyncCount() > 0) {
                Mouths mouths = layer->getLipSyncList();
                list.append(mouths);
            }
        }
    }

    return list;
}

int TupScene::photogramsTotal()
{
    int total = 0;
    int totalLayers = layers.size();
    for (int i = 0; i < totalLayers; i++)
        total = qMax(total, layers.at(i)->getFrames().count());

    return total;
}

void TupScene::updateRasterBackground(TupProject::Mode spaceContext, const QString &imgPath)
{
    background->updateRasterBgImage(spaceContext, imgPath);
}
