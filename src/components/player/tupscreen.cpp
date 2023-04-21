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

#include "tupscreen.h"
#include "tuplibrary.h"
#include "tupsoundlayer.h"

#include <QUrl>
#include <QApplication>

TupScreen::TupScreen(TupProject *work, const QSize viewSize, bool sizeChanged, QWidget *parent) : QFrame(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen()] - viewSize -> " << viewSize;
    #endif

    project = work;
    library = work->getLibrary();

    isScaled = sizeChanged;
    screenDimension = viewSize;

    cyclicAnimation = false;
    fps = 24;
    sceneIndex = 0;
    currentFramePosition = 0;

    playerIsActive = false;
    playMode = Forward;
    mute = false;
    renderOn = false;

    timer = new QTimer(this);
    playBackTimer = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(advance()));
    connect(playBackTimer, SIGNAL(timeout()), this, SLOT(back()));

    initPhotogramsArray();

    updateSceneIndex(0);
    updateFirstFrame();
}

TupScreen::~TupScreen()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupScreen()]";
    #endif

    timer->stop();
    playBackTimer->stop();

    newList.clear();

    clearPhotograms();
    clearScenesArrays();

    renderControl.clear();

    delete timer;
    timer = nullptr;
    delete playBackTimer;
    playBackTimer = nullptr;
    delete renderer;
    renderer = nullptr;
}

// Clean a photogram array if the scene has changed
void TupScreen::resetSceneFromList(int scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::resetSceneFromList()]";
    #endif

    if (scene > -1) {
        if (renderControl.at(scene)) {
            renderControl.replace(scene, false);
            animationList.replace(scene, newList);
        }
    } else {
        initPhotogramsArray();
    }

    resize(screenDimension);
}

void TupScreen::clearPhotograms()
{
    for (int i=0; i<photograms.count(); i++)
        photograms[i] = QImage();
    photograms.clear();
}

void TupScreen::clearScenesArrays()
{
    renderControl.clear();
    for (int i=0; i<animationList.count(); i++) {
        for (int j=0; j<animationList[i].count(); j++)
            animationList[i][j] = QImage();
    }
    animationList.clear();
}

void TupScreen::releaseAudioResources()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::releaseAudioResources()]";
    #endif

    soundRecords.clear();
    while(!soundPlayer.isEmpty()) {
        QMediaPlayer *player = soundPlayer.takeFirst();
        player->stop();
        player->setSource(QUrl());
        delete player;
        player = nullptr;
    }
}

void TupScreen::initPhotogramsArray()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::initPhotogramsArray()]";
    #endif

    renderControl.clear();
    animationList.clear();

    for (int i=0; i < project->scenesCount(); i++) {
         renderControl.insert(i, false);
         animationList.insert(i, newList);
    }
}

void TupScreen::setFPS(int speed)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::setFPS()]";
    #endif

    fps = speed;

    if (playMode == Forward) {
        if (timer->isActive()) {
            timer->stop();
            play();
        }
    } else { // Backward
        if (playBackTimer->isActive()) {
            playBackTimer->stop();
            playBack();
        }
    }
}

void TupScreen::paintEvent(QPaintEvent *)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::paintEvent()]";
    #endif
    */

    if (!mute && !renderOn) {
        if (photograms.count() > 1) {
            if (playerIsActive && (playMode == Forward))
                playSoundAt(currentFramePosition);
        }
    }

    if (!firstShoot) {
        if (currentFramePosition > -1 && currentFramePosition < photograms.count())
            currentPhotogram = photograms[currentFramePosition];
    } else {
        firstShoot = false;
    }

    QPainter painter;
    if (painter.begin(this)) {
        if (!currentPhotogram.isNull()) {
            painter.drawImage(imagePos, currentPhotogram);
        } else {
            #ifdef TUP_DEBUG
                QString msg = "[TupScreen::paintEvent()] - Photogram is NULL (index: "
                              + QString::number(currentFramePosition) + "/"
                              + QString::number(photograms.count()) + ")";
                qWarning() << msg;
            #endif
        }
    }

    // SQA: Border for the player. Useful for some tests
    // painter.setPen(QPen(Qt::gray, 0.5, Qt::SolidLine));
    // painter.drawRect(x, y, currentPhotogram.size().width()-1, k->renderCamera.size().height()-1);
}

void TupScreen::play()
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupScreen::play()] - Playing at " << fps << " FPS";
    #endif

    if (playMode == Backward) {
        playMode = Forward;
        if (playBackTimer->isActive())
            playBackTimer->stop();
    }

    playerIsActive = true;
    currentFramePosition = 0;

    if (!timer->isActive()) {
        if (!renderControl.at(sceneIndex))
            render();

        // No frames to play
        if (photograms.count() == 1)
            return;

        if (renderControl.at(sceneIndex))
            timer->start(1000 / fps);
    }
}

void TupScreen::playBack()
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupScreen::playBack()] - Starting procedure...";
    #endif

    if (photograms.count() == 1)
        return;

    if (playMode == Forward) {
        stopSounds();

        playMode = Backward;
        if (timer->isActive())
            timer->stop();
    }

    playerIsActive = true;
    currentFramePosition = photograms.count() - 1;

    if (!playBackTimer->isActive()) {
        if (!renderControl.at(sceneIndex)) {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            render();
            QApplication::restoreOverrideCursor();
        }

        if (renderControl.at(sceneIndex))
            playBackTimer->start(1000 / fps);
    }
}

bool TupScreen::isPlaying()
{
    return playerIsActive;
}

PlayMode TupScreen::getPlaymode()
{
    return playMode;
}

void TupScreen::pause()
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupScreen::pause()] - Pausing player!";
    #endif

    if (playerIsActive) {
        stopAnimation();
    } else {
        if (photograms.isEmpty())
            render();

        // No frames to play
        if (photograms.count() == 1)
            return;

        playerIsActive = true;
        if (playMode == Forward)
            timer->start(1000 / fps);
        else
            playBackTimer->start(1000 / fps);
    }
}

void TupScreen::stop()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::stop()] - Stopping player!";
        qDebug() << "[TupScreen::stop()] - playMode -> " << playMode;
    #endif

    stopAnimation();

    if (playMode == Forward)
        currentFramePosition = 0;
    else
        currentFramePosition = photograms.count() - 1;

    if (currentFramePosition == 0)
        emit frameChanged(1);
    else
        emit frameChanged(currentFramePosition);

    repaint();
}

void TupScreen::stopAnimation()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::stopAnimation()] - playMode -> " << playMode;
    #endif

    playerIsActive = false;

    if (playMode == Forward) {
        stopSounds();

        if (timer) {
            if (timer->isActive())
                timer->stop();
        }
    } else {
        if (playBackTimer) {
            if (playBackTimer->isActive())
                playBackTimer->stop();
        }
    }
}

void TupScreen::nextFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::nextFrame()]";
    #endif

    if (playerIsActive)
        stopAnimation();

    if (!renderControl.at(sceneIndex))
        render();

    currentFramePosition += 1;

    if (currentFramePosition == photograms.count())
        currentFramePosition = 0;

    emit frameChanged(currentFramePosition + 1);

    repaint();
}

void TupScreen::previousFrame()
{
    /* 
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::previousFrame()]";
    #endif
    */

    if (playerIsActive)
        stopAnimation();

    if (!renderControl.at(sceneIndex))
        render();

    currentFramePosition -= 1;

    if (currentFramePosition < 0)
        currentFramePosition = photograms.count() - 1;

    emit frameChanged(currentFramePosition + 1);

    repaint();
}

void TupScreen::advance()
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::advance()]";
    #endif
    */

    if (cyclicAnimation && currentFramePosition >= photograms.count()) {
        currentFramePosition = -1;
        stopSounds();
    }

    if (currentFramePosition < photograms.count()) {
        repaint();
        currentFramePosition++;
        emit frameChanged(currentFramePosition);
    } else if (!cyclicAnimation) {
        stop();
    }
}

void TupScreen::back()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::back()]";
    #endif

    if (cyclicAnimation && currentFramePosition < 0)
        currentFramePosition = photograms.count() - 1;

    if (currentFramePosition >= 0) {
        repaint();
        currentFramePosition--;
    } else if (!cyclicAnimation) {
        stop();
    }
}

void TupScreen::frameResponse(TupFrameResponse *)
{
}

void TupScreen::layerResponse(TupLayerResponse *response)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::layerResponse()] - response->getAction() -> " << response->getAction();
    #endif

    switch (response->getAction()) {
        case TupProjectRequest::AddLipSync:
        case TupProjectRequest::UpdateLipSync:
        {
            #ifdef TUP_DEBUG
                qDebug() << "[TupScreen::layerResponse()] - Lipsync call. Updating sound records...";
            #endif
            loadSoundRecords();
        }
        break;
    }
}

void TupScreen::sceneResponse(TupSceneResponse *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::sceneResponse()]";
    #endif

    int index = event->getSceneIndex();

    switch (event->getAction()) {
        case TupProjectRequest::Add:
          {
              addPhotogramsArray(index);
          }
        break;
        case TupProjectRequest::Remove:
          {
              if (index < 0)
                  break;

              renderControl.removeAt(index);
              animationList.removeAt(index);

              if (index == project->scenesCount())
                  index--;

              updateSceneIndex(index);
          }
        break;
        case TupProjectRequest::Reset:
          {
              renderControl.replace(index, false);
              animationList.replace(index, newList);

              clearPhotograms();
              photograms = newList;
          }
        break;
        case TupProjectRequest::Select:
          {
              updateSceneIndex(index);
          }
        break;
        default: 
        break;
    }
}

void TupScreen::projectResponse(TupProjectResponse *)
{
}

void TupScreen::itemResponse(TupItemResponse *)
{
}

void TupScreen::libraryResponse(TupLibraryResponse *response)
{
    #ifdef TUP_DEBUG
       qDebug() << "[TupScreen::libraryResponse()]";
    #endif

    Q_UNUSED(response)

    /*
    QString id = response->getArg().toString();
    #ifdef TUP_DEBUG
       qDebug() << "[TupScreen::libraryResponse()] - id -> " << id;
    #endif

    switch (response->getAction()) {
       case TupProjectRequest::Remove:
       {
           if (response->symbolType() == TupLibraryObject::Audio) {
               qDebug() << "[TupScreen::libraryResponse()] - Removing item -> " << id;
               int size = soundRecords.count();
               qDebug() << "[TupScreen::libraryResponse()] - size -> " << size;
               for (int i=0; i<size; i++) {
                   SoundResource soundRecord = soundRecords.at(i);
                   qDebug() << "[TupScreen::libraryResponse()] - Sound resource key ->" << soundRecord.path;
               }
           }
       }
       break;
       default:
       break;
    }
    */
}

void TupScreen::render()
{
    #ifdef TUP_DEBUG
       qDebug() << "[TupScreen::render()]";
    #endif

    renderOn = true;
    emit isRendering(0);

    TupScene *scene = project->sceneAt(sceneIndex);
    if (scene) {
        clearPhotograms();

        renderer = new TupAnimationRenderer(library);
        renderer->setScene(scene, project->getDimension(), scene->getBgColor());
        int i = 1;
        while (renderer->nextPhotogram()) {
            renderized = QImage(project->getDimension(), QImage::Format_RGB32);
            painter = new QPainter(&renderized);
            painter->setRenderHint(QPainter::Antialiasing);

            renderer->render(painter);
            painter->end();
            painter = nullptr;
            delete painter;

            if (isScaled)
                photograms << renderized.scaledToWidth(screenDimension.width(), Qt::SmoothTransformation);
            else
                photograms << renderized;

            emit isRendering(i);
            i++;
        }

        animationList.replace(sceneIndex, photograms);
        renderControl.replace(sceneIndex, true);

        renderer = nullptr;
        delete renderer;
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::render()] - Fatal Error: Scene is NULL! -> index: "
                       << sceneIndex;
        #endif
    }

    emit isRendering(0); 
    renderOn = false;
}

QSize TupScreen::sizeHint() const
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::sizeHint()]";
    #endif

    return currentPhotogram.size();
}

void TupScreen::resizeEvent(QResizeEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::resizeEvent()]";
    #endif

    if (sceneIndex > -1) {
        currentFramePosition = 0;
        clearPhotograms();
        photograms = animationList.at(sceneIndex);
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::resizeEvent()] - "
                          "Error: Current index is invalid -> " << sceneIndex;
        #endif
    }

    QFrame::resizeEvent(event);
}

void TupScreen::setLoop(bool loop)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::setLoop()]";
    #endif

    cyclicAnimation = loop;
}

void TupScreen::updateSceneIndex(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::updateSceneIndex()]";
    #endif

    sceneIndex = index;
    if (sceneIndex > -1 && sceneIndex < animationList.count()) {
        currentFramePosition = 0;
        clearPhotograms();
        photograms = animationList.at(sceneIndex);
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::updateSceneIndex()] - "
                          "Error: Can't set current photogram array -> " << sceneIndex;
        #endif
    }
}

int TupScreen::currentSceneIndex()
{
    return sceneIndex;
}

TupScene *TupScreen::currentScene()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::currentScene()]";
    #endif

    if (sceneIndex > -1) {
        return project->sceneAt(sceneIndex);
    } else {
        if (project->scenesCount() == 1) {
            sceneIndex = 0;
            return project->sceneAt(0);
        } 
    }

    return nullptr;
}

int TupScreen::sceneTotalFrames()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::sceneTotalFrames()]";
    #endif

    TupScene *scene;
    if (sceneIndex > -1) {
        scene = project->sceneAt(sceneIndex);
        if (scene)
            return scene->totalPhotograms();
    } else {
        if (project->scenesCount() == 1) {
            sceneIndex = 0;
            scene = project->sceneAt(0);
            return scene->totalPhotograms();
        }
    }

    return 0;
}

// Update and paint the first image of the current scene
void TupScreen::updateAnimationArea()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::updateAnimationArea()]";
    #endif

    if (sceneIndex > -1 && sceneIndex < animationList.count()) {
        currentFramePosition = 0;
        clearPhotograms();
        photograms = animationList.at(sceneIndex);
        updateFirstFrame();
        update();
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::updateAnimationArea()] - "
                          "Fatal Error: Can't access to scene index: " << sceneIndex;
        #endif
    }
}

// Prepare the first photogram of the current scene to be painted
void TupScreen::updateFirstFrame()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::updateFirstFrame()]";
    #endif

    if (sceneIndex > -1 && sceneIndex < animationList.count()) {
        TupScene *scene = project->sceneAt(sceneIndex);
        if (scene) { 
            loadSoundRecords();

            renderer = new TupAnimationRenderer(library);
            renderer->setScene(scene, project->getDimension(), scene->getBgColor());
            renderer->renderPhotogram(0);

            renderized = QImage(project->getDimension(), QImage::Format_RGB32);

            QPainter *painter = new QPainter(&renderized);
            painter->setRenderHint(QPainter::Antialiasing);
            renderer->render(painter);

            if (isScaled)
                currentPhotogram = renderized.scaledToWidth(screenDimension.width(),
                                                            Qt::SmoothTransformation);
            else
                currentPhotogram = renderized;

            int x = (frameSize().width() - currentPhotogram.size().width()) / 2;
            int y = (frameSize().height() - currentPhotogram.size().height()) / 2;
            imagePos = QPoint(x, y);

            firstShoot = true;

            delete painter;
            painter = nullptr;

            delete renderer;
            renderer = nullptr;
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupScreen::updateFirstFrame()] - "
                              "Fatal Error: Null scene at index -> " << sceneIndex;
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupScreen::updateFirstFrame()] - "
                          "Fatal Error: Can't access to scene index -> " << sceneIndex;
        #endif
    }
}

void TupScreen::addPhotogramsArray(int scene)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::addPhotogramsArray()]";
    #endif

    if (scene > -1) {
        renderControl.insert(scene, false);
        animationList.insert(scene, newList);
    }
}

void TupScreen::loadSoundRecords()
{
    #ifdef TUP_DEBUG
        qDebug() << "---";
        qDebug() << "[TupScreen::loadSoundRecords()]";
    #endif

    releaseAudioResources();

    // Loading effect sounds
    QList<SoundResource> effectsList = project->soundResourcesList();
    int total = effectsList.count();

    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::loadSoundRecords()] - Loading sound effects...";
    #endif

    for (int i=0; i<total; i++)  {
        SoundResource sound = effectsList.at(i);
        soundRecords << sound;
        #ifdef TUP_DEBUG
            qDebug() << "[TupScreen::loadSoundRecords()] - Audio loaded! -> " << sound.path;
            qDebug() << "[TupScreen::loadSoundRecords()] - Audio frame -> " << sound.frame;
        #endif
        soundPlayer << new QMediaPlayer();
    }

    #ifdef TUP_DEBUG
        qDebug() << "*** Sound items total -> " << soundRecords.size();
        qDebug() << "---";
    #endif
}

void TupScreen::playSoundAt(int frame)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::playSoundAt()] - frame -> " << frame;
    #endif
    */

    int size = soundRecords.count();
    for (int i=0; i<size; i++) {
        SoundResource soundRecord = soundRecords.at(i);
        if (!soundRecord.muted) {
            if (frame == (soundRecord.frame - 1)) {
                if (i < soundPlayer.count()) {
                    if (soundPlayer.at(i)->playbackState() != QMediaPlayer::PlayingState) {
                        #ifdef TUP_DEBUG
                            qWarning() << "[TupScreen::playSoundAt()] - Playing file -> " << soundRecord.path;
                            qWarning() << "[TupScreen::playSoundAt()] - frame -> " << frame;
                        #endif
                        soundPlayer.at(i)->setSource(QUrl::fromLocalFile(soundRecord.path));
                        soundPlayer.at(i)->play();
                    }
                } else {
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupScreen::playSoundAt()] - Fatal Error: "
                        "No sound file was found at -> " << soundRecord.path;
                    #endif
                }
            }
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupScreen::playSoundAt()] - "
                              "Sound file is muted -> " << soundRecord.path;
            #endif
        }
    }
}

void TupScreen::enableMute(bool flag)
{
    mute = flag;

    if (mute) {
        stopSounds();
    } else {
       if (playerIsActive) {
           stop();
           play();
       }
    }
}

void TupScreen::stopSounds()
{
    int size = soundRecords.count();
    for (int i=0; i<size; i++)
        soundPlayer.at(i)->stop();
}

void TupScreen::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    emit activePause();
}

bool TupScreen::removeSoundTrack(const QString &soundKey)
{
    int size = soundRecords.count();
    #ifdef TUP_DEBUG
        qDebug() << "[TupScreen::removeSoundTrack()] - soundKey -> " << soundKey;
        qDebug() << "[TupScreen::removeSoundTrack()] - sounds list size -> " << size;
    #endif

    for (int i=0; i<size; i++) {
        SoundResource soundRecord = soundRecords.at(i);
        if (soundKey.compare(soundRecord.key) == 0) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupScreen::removeSoundTrack()] - "
                            "Found! Sound resource path -> " << soundRecord.path;
            #endif

            soundRecords.takeAt(i);
            QMediaPlayer *player = soundPlayer.takeAt(i);
            player->stop();
            player->setSource(QUrl());
            delete player;
            player = nullptr;

            return true;
        }
    }

    return false;
}
