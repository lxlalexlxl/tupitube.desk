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

#include "tupmicmanager.h"
#include "tosd.h"
#include "talgorithm.h"

#include <QDir>
#include <QGridLayout>
#include <QGroupBox>
#include <QTimer>
#include <QMessageBox>
#include <QMediaDevices>
#include <QMediaFormat>
#include <QAudioDevice>

// static qreal getMaxValue(const QAudioFormat &format);
// static QVector<qreal> getBufferLevels(const QAudioBuffer &buffer);
// template <class T> static QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels);

TupMicManager::TupMicManager()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::TupMicManager()]";
    #endif

    initRecorder();
    setupUI();
    setConnections();
}

TupMicManager::~TupMicManager()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::~TupMicManager()]";
    #endif

    resetMediaPlayer();

    delete audioOutput;
    delete micRecorder;
    delete audioLevel1;
    delete audioLevel2;
}

void TupMicManager::initRecorder()
{
    audioname = TAlgorithm::randomString(8);
    micRecorder = new QMediaRecorder(this);
    captureSession.setRecorder(micRecorder);
    captureSession.setAudioInput(new QAudioInput(this));

    recording = false;
    secCounter = 0;
    player << new QMediaPlayer;
    audioOutput = new QAudioOutput;
    player.at(0)->setAudioOutput(audioOutput);
    audioOutput->setVolume(100);
    extension = ".m4a";
}

void TupMicManager::setupUI()
{
    centralWidget = new QWidget(this);
    QGridLayout *mainLayout = new QGridLayout(centralWidget);

    QGridLayout *formLayout = new QGridLayout;

    QLabel *nameLabel = new QLabel(centralWidget);
    nameLabel->setText(tr("Record Name:"));

    formLayout->addWidget(nameLabel, 0, 0, 1, 1);

    nameInput = new TInputField(tr("Audio 01"));
    connect(nameInput, SIGNAL(inputFilled(bool)), this, SLOT(enableRecordButton(bool)));

    formLayout->addWidget(nameInput, 0, 1, 1, 1);

    QLabel *deviceLabel = new QLabel(centralWidget);
    deviceLabel->setText(tr("Input Device:"));

    formLayout->addWidget(deviceLabel, 1, 0, 1, 1);

    QLabel *durationLabel = new QLabel(centralWidget);
    durationLabel->setText(tr("Duration:"));

    formLayout->addWidget(durationLabel, 2, 0, 1, 1);

    durationInput = new TInputField("0 " + tr("sec"));
    connect(durationInput, SIGNAL(inputFilled(bool)), this, SLOT(enableRecordButton(bool)));

    formLayout->addWidget(durationInput, 2, 1, 1, 1);

    audioDevCombo = new QComboBox(centralWidget);
    audioDevCombo->addItem(tr("Default"), QVariant(QString()));
    foreach(QAudioDevice device, QMediaDevices::audioInputs())
        audioDevCombo->addItem(device.description(), QVariant::fromValue(device));

    formLayout->addWidget(audioDevCombo, 1, 1, 1, 1);
    mainLayout->addLayout(formLayout, 0, 0, 1, 3);

    controlsWidget = new QWidget;
    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->addStretch();

    recordButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/record.png")), "", controlsWidget);
    recordButton->setToolTip(tr("Record"));
    recordButton->setMinimumWidth(60);
    connect(recordButton, SIGNAL(clicked()), this, SLOT(toggleRecord()));

    controlsLayout->addWidget(recordButton);

    pauseButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/pause.png")), "", controlsWidget);
    pauseButton->setToolTip(tr("Pause"));
    pauseButton->setMinimumWidth(60);
    pauseButton->setEnabled(false);
    connect(pauseButton, SIGNAL(clicked()), this, SLOT(togglePause()));

    controlsLayout->addWidget(pauseButton);
    controlsLayout->addStretch();

    controlsWidget->setLayout(controlsLayout);

    playerWidget = new QWidget;
    QHBoxLayout *playerLayout = new QHBoxLayout;

    playButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/play.png")), "", playerWidget);
    playButton->setToolTip(tr("Play"));
    connect(playButton, SIGNAL(clicked()), this, SLOT(playRecording()));

    playerLayout->addWidget(playButton);

    discardButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/delete.png")), "", playerWidget);
    discardButton->setToolTip(tr("Discard"));
    connect(discardButton, SIGNAL(clicked()), this, SLOT(discardRecording()));

    playerLayout->addWidget(discardButton);
    playerWidget->setLayout(playerLayout);
    playerWidget->setVisible(false);

    bottomWidget = new QWidget;
    QVBoxLayout *bottomLayout = new QVBoxLayout;

    QLabel *levelLabel = new QLabel(bottomWidget);
    levelLabel->setAlignment(Qt::AlignCenter);
    levelLabel->setText(tr("Audio Level"));

    bottomLayout->addWidget(levelLabel);

    levelsScreenLayout = new QVBoxLayout();

    audioLevel1 = new TupMicLevel(bottomWidget);
    levelsScreenLayout->addWidget(audioLevel1);
    audioLevel2 = new TupMicLevel(bottomWidget);
    levelsScreenLayout->addWidget(audioLevel2);

    audioLevelsIncluded = true;

    bottomLayout->addLayout(levelsScreenLayout);

    statusLabel = new QLabel(bottomWidget);
    statusLabel->setText(" ");

    bottomLayout->addWidget(statusLabel);
    bottomLayout->addStretch(1);
    bottomWidget->setLayout(bottomLayout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(centralWidget);
    layout->addWidget(controlsWidget);
    layout->addWidget(playerWidget);
    layout->addWidget(bottomWidget);
    layout->addStretch(1);
    setLayout(layout);
}

void TupMicManager::setConnections()
{
    connect(micRecorder, &QMediaRecorder::durationChanged, this, &TupMicManager::updateProgress);
    connect(micRecorder, &QMediaRecorder::recorderStateChanged, this, &TupMicManager::onStateChanged);
    connect(micRecorder, &QMediaRecorder::errorChanged, this, &TupMicManager::showErrorMessage);
}

void TupMicManager::updateProgress(qint64 duration)
{
    if (micRecorder->error() != QMediaRecorder::NoError || duration < 2000)
        return;

    recordTime = QString::number(duration/1000);
    statusLabel->setText(tr("Recording...") + " " + recordTime + " " + tr("sec"));
}

void TupMicManager::onStateChanged(QMediaRecorder::RecorderState state)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::onStateChanged()] - state -> " << state;
    #endif

    QString statusMessage = "";

    switch (state) {
        case QMediaRecorder::RecordingState:
            recording = true;
            statusMessage = tr("Recording...");
            durationInput->setText("...");
            recordButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/stop.png")));
            recordButton->setToolTip(tr("Stop"));
            pauseButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/pause.png")));
            pauseButton->setToolTip(tr("Pause"));
            break;
        case QMediaRecorder::PausedState:
            recording = false;
            statusMessage = tr("Paused");
            recordButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/stop.png")));
            recordButton->setToolTip(tr("Stop"));
            pauseButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/resume.png")));
            pauseButton->setToolTip(tr("Resume"));
            break;
        case QMediaRecorder::StoppedState:
            recording = false;
            statusMessage = tr("Stopped");
            recordButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/record.png")));
            recordButton->setToolTip(tr("Record"));
            pauseButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/pause.png")));
            pauseButton->setToolTip(tr("Pause"));
            controlsWidget->setVisible(false);
            playerWidget->setVisible(true);

            emit soundReady(true);
            break;
    }

    pauseButton->setEnabled(micRecorder->recorderState() != QMediaRecorder::StoppedState);
    if (micRecorder->error() == QMediaRecorder::NoError)
        statusLabel->setText(statusMessage);
}

void TupMicManager::toggleRecord()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::toggleRecord()] - state ->" << micRecorder->recorderState();
    #endif

    QString filename = CACHE_DIR + audioname + extension;
    if (micRecorder->recorderState() == QMediaRecorder::StoppedState) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupMicManager::toggleRecord()] - Recording audio to file ->" << filename;
        #endif

        nameInput->setReadOnly(true);
        captureSession.audioInput()->setDevice(audioDevCombo->currentData().value<QAudioDevice>());

        QMediaFormat format;
        format.setFileFormat(QMediaFormat::MP3);
        format.setAudioCodec(QMediaFormat::AudioCodec::AAC);
        micRecorder->setMediaFormat(format);

        micRecorder->setAudioSampleRate(44100); // Standard 44.1kHz
        micRecorder->setAudioBitRate(128000);
        micRecorder->setAudioChannelCount(2);
        micRecorder->setQuality(QMediaRecorder::VeryHighQuality);
        micRecorder->setEncodingMode(QMediaRecorder::ConstantQualityEncoding);
        micRecorder->setOutputLocation(QUrl::fromLocalFile(filename));
        micRecorder->record();
    } else {
        micRecorder->stop();
        durationInput->setText(recordTime + " " + tr("sec"));
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        playButton->setEnabled(false);
        discardButton->setEnabled(false);
        QTimer::singleShot(1000, this, SLOT(enablePlayButton()));

        #ifdef TUP_DEBUG
            qDebug() << "[TupMicManager::toggleRecord()] - Stop recording...";
            if (QFile::exists(filename))
                qDebug() << "[TupMicManager::toggleRecord()] - "
                            "Audio file was created successfully! -> " << filename;
            else
                qWarning() << "[TupMicManager::toggleRecord()] - "
                              "Fatal Error: Audio file wasn't created -> " << filename;
        #endif
    }
}

void TupMicManager::enablePlayButton()
{
    playButton->setEnabled(true);
    discardButton->setEnabled(true);
    QApplication::restoreOverrideCursor();
}

void TupMicManager::togglePause()
{
    if (micRecorder->recorderState() != QMediaRecorder::PausedState)
        micRecorder->pause();
    else
        micRecorder->record();
}

void TupMicManager::discardRecording()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::discardRecording()]";
    #endif

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Question"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("Confirm Action."));
    msgBox.setInformativeText(tr("Do you want to delete this sound recording?"));

    msgBox.addButton(QString(tr("Ok")), QMessageBox::AcceptRole);
    msgBox.addButton(QString(tr("Cancel")), QMessageBox::RejectRole);
    msgBox.show();

    int ret = msgBox.exec();
    if (ret == QMessageBox::RejectRole)
        return;

    statusLabel->setText(tr(""));

    nameInput->setReadOnly(false);
    nameInput->setText(tr("Audio 01"));
    nameInput->setFocus();

    durationInput->setText("0 " + tr("sec"));

    playerWidget->setVisible(false);
    controlsWidget->setVisible(true);

    player.at(0)->setSource(QUrl());

    QString filename = CACHE_DIR + audioname + extension;
    if (QFile::exists(filename)) {
        if (!QFile::remove(filename)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupMicManager::discardRecording()] - Fatal Error: Can't remove file -> " << filename;
            #endif
            TOsd::self()->display(TOsd::Error, tr("Can't remove audio file!"));
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupMicManager::discardRecording()] - Fatal Error: Audio file doesn't exist -> " << filename;
        #endif
        TOsd::self()->display(TOsd::Error, tr("Audio file doesn't exist!"));
    }

    emit soundReady(false);
}

void TupMicManager::showErrorMessage()
{
    statusLabel->setText(micRecorder->errorString());
}

void TupMicManager::clearMicLevels()
{
    for (int i = 0; i < micLevels.size(); ++i)
        micLevels.at(i)->setLevel(0);
}

void TupMicManager::handleBuffer(const QAudioBuffer& buffer)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::handleBuffer()]";
    #endif
    */

    if (audioLevelsIncluded) {
        levelsScreenLayout->removeWidget(audioLevel1);
        levelsScreenLayout->removeWidget(audioLevel2);
        audioLevelsIncluded = false;
    }

    if (micLevels.count() != buffer.format().channelCount()) {
        qDeleteAll(micLevels);
        micLevels.clear();
        for (int i = 0; i < buffer.format().channelCount(); ++i) {
            TupMicLevel *level = new TupMicLevel(centralWidget);
            micLevels.append(level);
            levelsScreenLayout->addWidget(level);
        }
    }

    /*
    QVector<qreal> levels = getBufferLevels(buffer);
    for (int i = 0; i < levels.count(); ++i)
        micLevels.at(i)->setLevel(levels.at(i));
    */
}

void TupMicManager::enableRecordButton(bool enabled)
{
    recordButton->setEnabled(enabled);
}

void TupMicManager::playRecording()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::playRecording()] - player->state() -> " << player.at(0)->playbackState();
    #endif

    if (player.at(0)->playbackState() == QMediaPlayer::StoppedState) {
        QString filename = CACHE_DIR + audioname + extension;
        #ifdef TUP_DEBUG
            qDebug() << "[TupMicManager::playRecording()] - Setting audio path for player -> " << filename;
        #endif
        if (QFile::exists(filename)) {
            timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, QOverload<>::of(&TupMicManager::trackPlayerStatus));
            timer->start(500);

            playButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/stop.png")));
            discardButton->setEnabled(false);

            player.at(0)->setSource(QUrl::fromLocalFile(filename));
            audioOutput->setVolume(100);
            player.at(0)->play();
            statusLabel->setText(tr("Playing..."));
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupMicManager::playRecording()] - "
                              "Fatal Error: Audio file doesn't exist -> " << filename;
            #endif
            TOsd::self()->display(TOsd::Error, tr("Audio file doesn't exist!"));
        }
    } else if (player.at(0)->playbackState() == QMediaPlayer::PlayingState) {
        player.at(0)->stop();

        playButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/play.png")));
        discardButton->setEnabled(true);
    }
}

void TupMicManager::trackPlayerStatus()
{
    secCounter += 500.0;

    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::checkPlayerStatus()] - status -> " << player.at(0)->playbackState();
        qDebug() << "milsec counter -> " << secCounter;
    #endif

    qreal duration = player.at(0)->duration();

    if (secCounter >= duration) {
        secCounter = 0;
        timer->stop();
        player.at(0)->stop();
        statusLabel->setText("");
        playButton->setIcon(QIcon(QPixmap(THEME_DIR + "icons/play.png")));
        discardButton->setEnabled(true);
        clearMicLevels();
    } else {
        QString prefix = "";
        int timer = secCounter/1000;
        if (timer < 10)
            prefix = "0";
        QString time = QString::number(timer);
        statusLabel->setText(tr("Playing...") + " " + prefix + time + " " + tr("sec"));
    }
}

QString TupMicManager::getRecordPath()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::getRecordPath()]";
    #endif

    resetMediaPlayer();
    if (!audioname.isEmpty()) {
        QString filename = CACHE_DIR + audioname + extension;
        #ifdef TUP_DEBUG
            qDebug() << "[TupMicManager::getRecordPath()] - filename -> " << filename;
        #endif
        if (QFile::exists(filename)) {
            return filename;
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupMicManager::getRecordPath()] - Fatal Error: Record path doesn't exist!";
            #endif
            TOsd::self()->display(TOsd::Error, tr("Audio file doesn't exist!"));
        }
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupMicManager::getRecordPath()] - Fatal Error: Record name is empty!";
        #endif
    }

    return "";
}

bool TupMicManager::isRecording()
{
    return recording;
}

void TupMicManager::cancelRecording()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::cancelRecording()]";
    #endif

    if (micRecorder->recorderState() == QMediaRecorder::RecordingState) {
        disconnect(micRecorder, SIGNAL(durationChanged(qint64)), this, SLOT(updateProgress(qint64)));
        disconnect(micRecorder, SIGNAL(stateChanged(QMediaRecorder::State)),
                   this, SLOT(onStateChanged(QMediaRecorder::State)));
        disconnect(micRecorder, &QMediaRecorder::errorChanged, this, &TupMicManager::showErrorMessage);
        micRecorder->stop();
    } else if (player.at(0)->playbackState() == QMediaPlayer::PlayingState) {
        player.at(0)->stop();
    }

    player.at(0)->setSource(QUrl());

    QString filename = CACHE_DIR + audioname + extension;
    if (QFile::exists(filename)) {
        if (QFile::remove(filename)) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupMicManager::cancelRecording()] - "
                              "Temporary file was removed succesfully! -> " << filename;
            #endif
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupMicManager::playRecording()] - Fatal Error: Can't remove audio file -> " << filename;
            #endif
            TOsd::self()->display(TOsd::Error, tr("Can't remove audio file!"));
        }
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupMicManager::playRecording()] - Fatal Error: Audio file doesn't exist -> " << filename;
        #endif
        TOsd::self()->display(TOsd::Error, tr("Audio file doesn't exist!"));
    }
}

void TupMicManager::resetMediaPlayer()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::resetMediaPlayer()]";
    #endif

    while(!player.isEmpty()) {
        QMediaPlayer *audio = player.takeFirst();
        audio->stop();
        audio->setSource(QUrl());
        delete audio;
        audio = nullptr;
    }
}

/*
void TupMicManager::updateStatus(QMediaRecorder::Status status)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupMicManager::updateStatus()] - status -> " << status;
    #endif

    QString statusMessage;

    switch (status) {
        case QMediaRecorder::RecordingStatus:
            statusMessage = tr("Recording...");
            durationInput->setText("...");
            break;
        case QMediaRecorder::PausedStatus:
            clearMicLevels();
            statusMessage = tr("Paused");
            break;
        case QMediaRecorder::UnloadedStatus:
        case QMediaRecorder::LoadedStatus:
            clearMicLevels();
            statusMessage = tr("Stopped");
        default:
            break;
    }

    if (micRecorder->error() == QMediaRecorder::NoError)
        statusLabel->setText(statusMessage);
}

static QVariant boxValue(const QComboBox *box)
{
    int idx = box->currentIndex();
    if (idx == -1)
        return QVariant();

    return box->itemData(idx);
}

qreal getMaxValue(const QAudioFormat &format)
{
    if (!format.isValid())
        return qreal(0);

    if (format.codec() != "audio/pcm")
        return qreal(0);

    switch (format.sampleFormat()) {
        case QAudioFormat::Unknown:
            break;
        case QAudioFormat::Float:
            if (format.sampleRate() != 32)
                return qreal(0);
            return qreal(1.00003);
        case QAudioFormat::Int16:
        case QAudioFormat::Int32:
            if (format.sampleRate() == 32)
                return qreal(INT_MAX);
            if (format.sampleRate() == 16)
                return qreal(SHRT_MAX);
            if (format.sampleRate() == 8)
                return qreal(CHAR_MAX);
            break;
        case QAudioFormat::UInt8:
            if (format.sampleRate() == 32)
                return qreal(UINT_MAX);
            if (format.sampleRate() == 16)
                return qreal(USHRT_MAX);
            if (format.sampleRate() == 8)
                return qreal(UCHAR_MAX);
            break;
        default:
            break;
    }

    return qreal(0);
}

QVector<qreal> getBufferLevels(const QAudioBuffer& buffer)
{
    QVector<qreal> levels;

    if (!buffer.format().isValid()) { //  || buffer.format().byteOrder() != QAudioFormat::LittleEndian)
        return levels;

    // if (buffer.format().codec() != "audio/pcm")
    //     return levels;

    int channelCount = buffer.format().channelCount();
    levels.fill(0, channelCount);
    qreal maxValue = getMaxValue(buffer.format());
    if (qFuzzyCompare(maxValue, qreal(0)))
        return levels;

    // switch (buffer.format().sampleType()) {
    switch (buffer.format().sampleRate()) {
        case QAudioFormat::Unknown:
        case QAudioFormat::UInt8:
            if (buffer.format().sampleRate() == 32)
                levels = getBufferLevels(buffer.constData<quint32>(), buffer.frameCount(), channelCount);
            if (buffer.format().sampleRate() == 16)
                levels = getBufferLevels(buffer.constData<quint16>(), buffer.frameCount(), channelCount);
            if (buffer.format().sampleRate() == 8)
                levels = getBufferLevels(buffer.constData<quint8>(), buffer.frameCount(), channelCount);
            for (int i = 0; i < levels.size(); ++i)
                levels[i] = qAbs(levels.at(i) - maxValue / 2) / (maxValue / 2);
        break;
        case QAudioFormat::Float:
            if (buffer.format().sampleRate() == 32) {
                levels = getBufferLevels(buffer.constData<float>(), buffer.frameCount(), channelCount);
                for (int i = 0; i < levels.size(); ++i)
                    levels[i] /= maxValue;
            }
        break;
        case QAudioFormat::Int32:
            if (buffer.format().sampleRate() == 32)
                levels = getBufferLevels(buffer.constData<qint32>(), buffer.frameCount(), channelCount);
            if (buffer.format().sampleRate() == 16)
                levels = getBufferLevels(buffer.constData<qint16>(), buffer.frameCount(), channelCount);
            if (buffer.format().sampleRate() == 8)
                levels = getBufferLevels(buffer.constData<qint8>(), buffer.frameCount(), channelCount);
            for (int i = 0; i < levels.size(); ++i)
                levels[i] /= maxValue;
            break;
    }

    return levels;
}

template <class T> QVector<qreal> getBufferLevels(const T *buffer, int frames, int channels)
{
    QVector<qreal> max_values;
    max_values.fill(0, channels);

    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < channels; ++j) {
            qreal value = qAbs(qreal(buffer[i * channels + j]));
            if (value > max_values.at(j))
                max_values.replace(j, value);
        }
    }

    return max_values;
}
*/
