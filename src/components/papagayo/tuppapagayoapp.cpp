/***************************************************************************
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

#include "tuppapagayoapp.h"
#include "tapplicationproperties.h"
#include "tconfig.h"
#include "tosd.h"
#include "toptionaldialog.h"
#include "tupbreakdowndialog.h"

#include <QAction>
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QBoxLayout>
#include <QFileInfo>
#include <QSettings>
#include <QMimeData>
#include <QFileDialog>
#include <QScrollArea>
#include <QSpacerItem>
#include <QGroupBox>
#include <QLabel>
#include <QScreen>

TupPapagayoApp::TupPapagayoApp(bool extendedUI, int32 fps, const QString &soundFile, QWidget *parent) : QMainWindow(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::TupPapagayoApp()]";
    #endif

    this->extendedUI = extendedUI;
    document = nullptr;
    enableAutoBreakdown = true;
    defaultFps = fps;
    playerStopped = true;

    setUIStyle();

    setupActions();
    setupUI();
    setupMenus();
    setAcceptDrops(true);

    updateActions();

    if (!soundFile.isEmpty())
        openFile(soundFile);
}

TupPapagayoApp::~TupPapagayoApp()
{
    if (document)
        delete document;

    if (waveformView)
        delete waveformView;

    if (mouthView)
        delete mouthView;
}

void TupPapagayoApp::setUIStyle()
{
    QFile file(THEME_DIR + "config/ui.qss");
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QString uiStyleSheet = QLatin1String(file.readAll());
        if (uiStyleSheet.length() > 0)
            setStyleSheet(uiStyleSheet);
        file.close();
    } else {
        #ifdef TUP_DEBUG
            qWarning() << "[TupPapagayoApp()] - Error: Theme file doesn't exist -> "
                       << QString(THEME_DIR + "config/ui.qss");
        #endif
    }
}

void TupPapagayoApp::setupActions()
{
    // Actions
    actionClose = new QAction(this);
    QIcon closeIcon;
    closeIcon.addFile(THEME_DIR + "icons/close.png", QSize(), QIcon::Normal, QIcon::Off);
    actionClose->setIcon(closeIcon);
    actionClose->setText(tr("Close"));
    actionClose->setShortcut(Qt::Key_Escape);
    connect(actionClose, SIGNAL(triggered()), this, SLOT(close()));

    if (extendedUI) {
        actionOpen = new QAction(this);
        QIcon openIcon;
        openIcon.addFile(THEME_DIR + "icons/open.png", QSize(), QIcon::Normal, QIcon::Off);
        actionOpen->setIcon(openIcon);
        actionOpen->setText(tr("Open"));
        actionOpen->setShortcut(QKeySequence(tr("Ctrl+O")));
        connect(actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    }

    actionPlay = new QAction(this);
    playIcon.addFile(THEME_DIR + "icons/play.png", QSize(), QIcon::Normal, QIcon::Off);
    actionPlay->setIcon(playIcon);
    actionPlay->setText(tr("Play"));
    actionPlay->setToolTip(tr("Play"));
    connect(actionPlay, SIGNAL(triggered()), this, SLOT(playVoice()));

    pauseIcon.addFile(THEME_DIR + "icons/pause.png", QSize(), QIcon::Normal, QIcon::Off);

    actionStop = new QAction(this);
    QIcon stopIcon;
    stopIcon.addFile(THEME_DIR + "icons/stop.png", QSize(), QIcon::Normal, QIcon::Off);
    actionStop->setIcon(stopIcon);
    actionStop->setText(tr("Stop"));
    actionStop->setToolTip(tr("Stop"));
    connect(actionStop, SIGNAL(triggered()), this, SLOT(stopVoice()));

    actionZoomIn = new QAction(this);
    QIcon zoomInIcon;
    zoomInIcon.addFile(THEME_DIR + "icons/zoom_in.png", QSize(), QIcon::Normal, QIcon::Off);
    actionZoomIn->setIcon(zoomInIcon);
    actionZoomIn->setText(tr("Zoom In"));
    actionZoomIn->setToolTip(tr("Zoom In"));

    actionZoomOut = new QAction(this);
    QIcon zoomOutIcon;
    zoomOutIcon.addFile(THEME_DIR + "icons/zoom_out.png", QSize(), QIcon::Normal, QIcon::Off);
    actionZoomOut->setIcon(zoomOutIcon);
    actionZoomOut->setText(tr("Zoom Out"));
    actionZoomOut->setToolTip(tr("Zoom Out"));

    actionAutoZoom = new QAction(this);
    QIcon autoZoomIcon;
    autoZoomIcon.addFile(THEME_DIR + "icons/zoom_1.png", QSize(), QIcon::Normal, QIcon::Off);
    actionAutoZoom->setIcon(autoZoomIcon);
    actionAutoZoom->setText(tr("Auto Zoom"));
    actionAutoZoom->setToolTip(tr("Auto Zoom"));
}

void TupPapagayoApp::setupUI()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::setupUI()]";
    #endif

    setWindowTitle(tr("Lip-Sync Manager"));

    // Central Widget
    QWidget *centralWidget = new QWidget(this);

    QHBoxLayout *centralHorizontalLayout = new QHBoxLayout(centralWidget);
    centralHorizontalLayout->setSpacing(6);
    centralHorizontalLayout->setContentsMargins(11, 11, 11, 11);

    QVBoxLayout *innerVerticalLayout = new QVBoxLayout();
    innerVerticalLayout->setSpacing(6);

    QHBoxLayout *firstLineLayout = new QHBoxLayout;

    QScrollArea *scrollArea = new QScrollArea(centralWidget);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);

    waveformView = new TupWaveFormView();
    waveformView->setGeometry(QRect(0, 0, 542, 194));
    scrollArea->setWidget(waveformView);
    waveformView->setScrollArea(scrollArea);

    connect(waveformView, SIGNAL(audioStopped()), this, SLOT(updatePauseButton()));
    connect(actionZoomIn, SIGNAL(triggered()), waveformView, SLOT(zoomIn()));
    connect(actionZoomOut, SIGNAL(triggered()), waveformView, SLOT(zoomOut()));
    connect(actionAutoZoom, SIGNAL(triggered()), waveformView, SLOT(autoZoom()));

    // Mouth Component
    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(6);

    mouthsCombo = new QComboBox;
    mouthsCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Mouth Sample Pack No 1"));
    mouthsCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Mouth Sample Pack No 2"));
    mouthsCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Mouth Sample Pack No 3"));
    mouthsCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Mouth Sample Pack No 4"));
    mouthsCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Mouth Sample Pack No 5"));
    mouthsCombo->addItem(QIcon(THEME_DIR + "icons/frames_mode.png"), tr("Set Mouth Images"));

    verticalLayout->addWidget(mouthsCombo);

    mouthFrame = new QStackedWidget(this);
    QSizePolicy viewSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    viewSizePolicy.setHorizontalStretch(0);
    viewSizePolicy.setVerticalStretch(0);
    viewSizePolicy.setHeightForWidth(mouthFrame->sizePolicy().hasHeightForWidth());

    mouthFrame->setSizePolicy(viewSizePolicy);
    mouthFrame->setMinimumSize(QSize(280, 200));
    mouthFrame->setMaximumWidth(280);

    QWidget *browserWidget = new QWidget;
    QVBoxLayout *browserMainLayout = new QVBoxLayout(browserWidget);

    QHBoxLayout *browserControlsLayout = new QHBoxLayout;
    QPushButton *mouthsButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/bitmap_array.png")),
                                                " " + tr("Load &Images"));
    connect(mouthsButton, SIGNAL(clicked()), this, SLOT(openImagesDialog()));
    mouthsPath = new QLineEdit("");
    mouthsPath->setReadOnly(true);

    mouthView = new TupMouthView(mouthFrame);
    viewSizePolicy.setHeightForWidth(mouthView->sizePolicy().hasHeightForWidth());
    mouthView->setSizePolicy(viewSizePolicy);
    mouthView->setMinimumSize(QSize(280, 200));
    mouthView->setMaximumWidth(280);

    connect(waveformView, SIGNAL(frameChanged(int)), this, SLOT(updateFrame(int)));
    connect(mouthsCombo, SIGNAL(activated(int)), this, SLOT(updateMouthView(int)));

    mouthFrame->addWidget(mouthView);

    customView = new TupCustomizedMouthView;
    viewSizePolicy.setHeightForWidth(customView->sizePolicy().hasHeightForWidth());
    customView->setSizePolicy(viewSizePolicy);
    customView->setMinimumSize(QSize(280, 200));
    customView->setMaximumWidth(280);

    browserControlsLayout->addWidget(mouthsPath);
    browserControlsLayout->addWidget(mouthsButton);

    browserMainLayout->addLayout(browserControlsLayout);
    browserMainLayout->addWidget(customView);
    mouthFrame->addWidget(browserWidget);

    verticalLayout->addWidget(mouthFrame);
    verticalLayout->addStretch();

    firstLineLayout->addWidget(scrollArea);
    firstLineLayout->addLayout(verticalLayout);

    innerVerticalLayout->addLayout(firstLineLayout);

    currentMouthPath = mouthView->getMouthsPath();
    waveformView->setMouthsPath(currentMouthPath);

    // Lateral Panel
    QGroupBox *lateralGroupBox = new QGroupBox(centralWidget);
    lateralGroupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    lateralGroupBox->setTitle(tr("Current Voice"));

    QSizePolicy generalSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    generalSizePolicy.setHorizontalStretch(0);
    generalSizePolicy.setVerticalStretch(0);
    generalSizePolicy.setHeightForWidth(lateralGroupBox->sizePolicy().hasHeightForWidth());

    QVBoxLayout *lateralVerticalLayout = new QVBoxLayout(lateralGroupBox);
    lateralVerticalLayout->setSpacing(6);
    lateralVerticalLayout->setContentsMargins(11, 11, 11, 11);
    QHBoxLayout *voiceHorizontalLayout = new QHBoxLayout();
    voiceHorizontalLayout->setSpacing(6);

    voiceName = new QLineEdit();
    connect(voiceName, SIGNAL(textChanged(QString)), this, SLOT(onVoiceNameChanged()));

    QLabel *voiceLabel = new QLabel(lateralGroupBox);
    voiceLabel->setText(tr("Voice name:"));
    voiceLabel->setBuddy(voiceName);

    voiceHorizontalLayout->addWidget(voiceLabel);
    voiceHorizontalLayout->addWidget(voiceName);

    QSpacerItem *voiceHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    voiceHorizontalLayout->addItem(voiceHorizontalSpacer);
    lateralVerticalLayout->addLayout(voiceHorizontalLayout);

    voiceText = new QPlainTextEdit(lateralGroupBox);
    voiceText->setMaximumHeight(100);
    QSizePolicy voiceSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    voiceSizePolicy.setHorizontalStretch(0);
    voiceSizePolicy.setVerticalStretch(0);
    voiceText->setSizePolicy(voiceSizePolicy);
    connect(voiceText, SIGNAL(textChanged()), this, SLOT(onVoiceTextChanged()));

    QLabel *spokenText = new QLabel(lateralGroupBox);
    spokenText->setText(tr("Spoken text:"));
    spokenText->setBuddy(voiceText);

    lateralVerticalLayout->addWidget(spokenText);
    lateralVerticalLayout->addWidget(voiceText);

    QLabel *phoneticLabel = new QLabel(lateralGroupBox);
    phoneticLabel->setText(tr("Phonetic breakdown:"));

    lateralVerticalLayout->addWidget(phoneticLabel);

    QHBoxLayout *languageHorizontalLayout = new QHBoxLayout;
    languageHorizontalLayout->setSpacing(6);

    languageChoice = new QComboBox();
    languageChoice->addItem(tr("English"));
    languageChoice->addItem(tr("Other Language"));
    connect(languageChoice, SIGNAL(activated(int)), this, SLOT(updateLanguage(int)));
    currentLanguage = English;

    languageHorizontalLayout->addWidget(languageChoice);

    breakdownButton = new QPushButton(lateralGroupBox);
    breakdownButton->setText(tr("Breakdown"));
    connect(breakdownButton, SIGNAL(clicked()), this, SLOT(runManualBreakdownAction()));
    breakdownButton->setEnabled(false);

    okButton = new QPushButton(lateralGroupBox);
    okButton->setMinimumWidth(60);
    okButton->setIcon(QIcon(THEME_DIR + "icons/apply.png"));
    okButton->setToolTip(tr("Save lip-sync record"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(createLipsyncRecord()));

    QPushButton *cancelButton = new QPushButton(lateralGroupBox);
    cancelButton->setMinimumWidth(60);
    cancelButton->setIcon(QIcon(THEME_DIR + "icons/close.png"));
    cancelButton->setToolTip(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));

    languageHorizontalLayout->addWidget(breakdownButton);

    QSpacerItem *languageHorizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    languageHorizontalLayout->addItem(languageHorizontalSpacer);

    languageHorizontalLayout->addWidget(okButton);
    languageHorizontalLayout->addWidget(cancelButton);

    lateralVerticalLayout->addLayout(languageHorizontalLayout);
    innerVerticalLayout->addWidget(lateralGroupBox);
    centralHorizontalLayout->addLayout(innerVerticalLayout);

    setCentralWidget(centralWidget);

    QScreen *screen = QGuiApplication::screens().at(0);
    setMinimumWidth(screen->geometry().width() * 0.7);
}

void TupPapagayoApp::setupMenus()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::setupMenus()]";
    #endif

    QMenuBar *menuBar = new QMenuBar(this);

    QMenu *menuFile = new QMenu(menuBar);
    menuFile->setTitle(tr("File"));

    if (extendedUI)
        menuFile->addAction(actionOpen);
    menuFile->addAction(actionClose);

    menuBar->addAction(menuFile->menuAction());
    setMenuBar(menuBar);

    QToolBar *mainToolBar = new QToolBar(this);
    mainToolBar->setMovable(true);
    mainToolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mainToolBar->setFloatable(false);

    if (extendedUI)
        mainToolBar->addAction(actionOpen);

    mainToolBar->addSeparator();
    mainToolBar->addAction(actionPlay);
    mainToolBar->addAction(actionStop);
    mainToolBar->addSeparator();
    mainToolBar->addAction(actionZoomIn);
    mainToolBar->addAction(actionZoomOut);
    mainToolBar->addAction(actionAutoZoom);

    addToolBar(Qt::TopToolBarArea, mainToolBar);
}

void TupPapagayoApp::openFile(QString filePath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::openFile()] - filePath -> " << filePath;
    #endif

    if (document) {
        delete document;
        document = nullptr;
    }

    document = new TupLipsyncDoc;
    QFileInfo info(filePath);
    if (info.suffix().toLower() == "pgo") {
        document->open(filePath);
    } else {
        document->openAudio(filePath);
        document->setFps(defaultFps);
    }

    if (document->getAudioPlayer() == nullptr) {
        delete document;
        document = nullptr;
        QMessageBox::warning(this, tr("Lip-Sync Manager"),
                             tr("Error opening audio file."),
                             QMessageBox::Ok);
        setWindowTitle(tr("Lip-Sync Manager"));
    } else {
        waveformView->setDocument(document);
        mouthView->setDocument(document);
        customView->setDocument(document);

        document->getAudioPlayer()->setNotifyInterval(17); // 60 fps
        connect(document->getAudioPlayer(), SIGNAL(positionChanged(qint64)), waveformView, SLOT(positionChanged(qint64)));
        connect(document->getAudioPlayer(), SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
                waveformView, SLOT(updateMediaStatus(QMediaPlayer::MediaStatus)));

        if (document->getCurrentVoice()) {
            voiceName->setText(document->getCurrentVoice()->getName());
            enableAutoBreakdown = false;
            voiceText->setPlainText(document->getCurrentVoice()->getText());
            enableAutoBreakdown = true;
        }

        setWindowTitle(tr("Lip-Sync Manager") + " - " + info.fileName());
    }

    updateActions();
}

bool TupPapagayoApp::confirmCloseDocument()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::confirmCloseDocument()]";
    #endif

    if (document && document->isModified()) {
        TOptionalDialog dialog(tr("Do you want to save your changes?"), tr("Confirmation Required"),
                               false, true, this);
        dialog.setModal(true);
        QScreen *screen = QGuiApplication::screens().at(0);
        dialog.move(static_cast<int> ((screen->geometry().width() - dialog.sizeHint().width()) / 2),
                    static_cast<int> ((screen->geometry().height() - dialog.sizeHint().height()) / 2));
        dialog.exec();

        TOptionalDialog::Result result = dialog.getResult();
        if (result == TOptionalDialog::Accepted) {
            createLipsyncRecord();
            return true;
        }

        if (result == TOptionalDialog::Cancelled)
            return false;

        if (result == TOptionalDialog::Discarded)
            return true;
    }

    return true;
}

void TupPapagayoApp::updateLanguage(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updateLanguage()] - index -> " << index;
    #endif

    if (index == TupPapagayoApp::English) {
        currentLanguage = TupPapagayoApp::English;
    } else {
        currentLanguage = TupPapagayoApp::OtherLang;
        buildOtherLanguagePhonemes();
    }
}

void TupPapagayoApp::closeEvent(QCloseEvent *event)
{
    if (confirmCloseDocument()) {
        if (document) {
            delete document;
            document = nullptr;
        }
        event->accept();
    } else {
        event->ignore();
    }
}

void TupPapagayoApp::dragEnterEvent(QDragEnterEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString filePath = urls.first().toLocalFile();
    if (filePath.isEmpty())
        return;

    QFileInfo info(filePath);
    QString extension = info.suffix().toLower();
    if (extension == "mp3" || extension == "wav")
        event->acceptProposedAction();
}

void TupPapagayoApp::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString filePath = urls.first().toLocalFile();

    if (filePath.isEmpty())
        return;

    QFileInfo info(filePath);
    QString extension = info.suffix().toLower();
    if (extension == "mp3" || extension == "wav") {
        event->acceptProposedAction();
        if (confirmCloseDocument())
            openFile(filePath);
    }
}

void TupPapagayoApp::updateActions()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updateActions()]";
    #endif

    bool flag = false;
    if (document)
        flag = true;

    actionPlay->setEnabled(flag);
    actionStop->setEnabled(flag);
    actionZoomIn->setEnabled(flag);
    actionZoomOut->setEnabled(flag);
    actionAutoZoom->setEnabled(flag);

    voiceName->setEnabled(flag);
    voiceText->setEnabled(flag);

    languageChoice->setEnabled(flag);
    mouthsCombo->setEnabled(flag);

    okButton->setEnabled(flag);
}

void TupPapagayoApp::openFile()
{
    if (!confirmCloseDocument())
        return;

    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    tr("Open"), path,
                                                    tr("Audio files (*.mp3 *.wav)"));
    if (filePath.isEmpty())
        return;

    openFile(filePath);
}

void TupPapagayoApp::playVoice()
{
    if (document && document->getAudioPlayer()) {
        if (playerStopped) {
            playerStopped = false;
            actionPlay->setIcon(pauseIcon);
            actionPlay->setText(tr("Pause"));
            actionPlay->setToolTip(tr("Pause"));
            document->playAudio();
        } else {
            updatePauseButton();
            document->pauseAudio();
        }
    }
}

void TupPapagayoApp::stopVoice()
{
    if (document && document->getAudioPlayer()) {
        playerStopped = true;
        actionPlay->setIcon(playIcon);
        actionPlay->setText(tr("Play"));
        actionPlay->setToolTip(tr("Play"));
        document->stopAudio();
    }
}

void TupPapagayoApp::pauseVoice()
{
    if (document && document->getAudioPlayer()) {
        playerStopped = true;
        document->pauseAudio();
    }
}

void TupPapagayoApp::onVoiceNameChanged()
{
    if (!document || !document->getCurrentVoice())
        return;

    document->getCurrentVoice()->setName(voiceName->text());
}

void TupPapagayoApp::loadWordsFromDocument()
{
    wordsList.clear();
    phonemesList.clear();
    QList<LipsyncWord *> words = document->getWords();
    if (!words.isEmpty()) {
        foreach(LipsyncWord *word, words) {
            wordsList << word->getText();
            phonemesList << word->getPhonemesString();
        }
    }
}

void TupPapagayoApp::buildOtherLanguagePhonemes()
{
    QString newText = voiceText->toPlainText();
    if (newText.isEmpty()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::buildOtherLanguagePhonemes()] - Warning: Voice text is empty!";
        #endif
        return;
    }

    QString currentText = document->getVoiceText();
    phonemesList.clear();

    if (currentText.isEmpty()) {
        wordsList = newText.split(" ");
        for(int i=0; i<wordsList.size(); i++)
            phonemesList << "";

        waveformView->update();
    } else {
        if (newText.compare(currentText) != 0) {
            wordsList = newText.split(" ");
            QList<LipsyncWord *> oldWords = document->getWords();
            foreach(QString word, wordsList) {
                if (currentText.contains(word)) {
                    foreach(LipsyncWord *oldWord, oldWords) {
                        QString text = oldWord->getText();
                        if (text.compare(word) == 0)
                            phonemesList << oldWord->getPhonemesString();
                    }
                } else {
                    phonemesList << "";
                }
            }
            waveformView->update();
        } else { // Previous text
            loadWordsFromDocument();
        }
    }
}

void TupPapagayoApp::onVoiceTextChanged()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::onVoiceTextChanged()] - currentLanguage -> " << currentLanguage;
    #endif

    if (!document) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::onVoiceTextChanged()] - Warning: Document is null!";
        #endif
        return;
    }

    if (!document->getCurrentVoice()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::onVoiceTextChanged()] - Warning: Voice is null!";
        #endif
        return;
    }

    QString text = voiceText->toPlainText();
    if (text.isEmpty()) {
        if (breakdownButton->isEnabled())
            breakdownButton->setEnabled(false);

        wordsList.clear();
        phonemesList.clear();

        document->cleanPhrases();
        waveformView->update();

        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::onVoiceTextChanged()] - Warning: Voice text is empty!";
        #endif
        return;
    }

    if (!breakdownButton->isEnabled())
        breakdownButton->setEnabled(true);

    if (currentLanguage == English) {
        document->setVoiceText(text);
        if (enableAutoBreakdown)
            runBreakdownAction(); // this is cool, but it could slow things down by doing constant breakdowns

        loadWordsFromDocument();

        updateActions();
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::onVoiceTextChanged()] - currentLanguage -> " << currentLanguage;
        #endif

        buildOtherLanguagePhonemes();
    }
}

int32 TupPapagayoApp::calculateDuration()
{
    int32 duration = document->getFps() * 10;
    if (document->getAudioExtractor()) {
        real time = document->getAudioExtractor()->duration();
        time *= document->getFps();
        duration = PG_ROUND(time);
    }

    return duration;
}

void TupPapagayoApp::runBreakdownAction()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::runBreakdownProcess()]";
    #endif

    if (!document || !document->getCurrentVoice()) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::onBreakdownProcess()] - Error: No lip-sync document loaded!";
        #endif
        return;
    }

    TupLipsyncDoc::loadDictionaries();
    document->setModifiedFlag(true);
    document->runBreakdown("EN", calculateDuration());

    waveformView->update();
}

void TupPapagayoApp::loadDocumentFromScratch(QStringList phonemes)
{
    int32 duration = calculateDuration();
    LipsyncVoice *voice = new LipsyncVoice(voiceName->text());
    LipsyncPhrase *phrase =  new LipsyncPhrase;
    phrase->setText(voiceText->toPlainText());
    phrase->setStartFrame(0);
    phrase->setEndFrame(duration);

    int wordLength = duration / wordsList.size();
    int wordPos = 0;
    for (int i=0; i<wordsList.size(); i++) {
        LipsyncWord *word = new LipsyncWord;
        word->setText(wordsList.at(i));
        word->setStartFrame(wordPos);
        word->setEndFrame(wordPos + wordLength);
        QString phoneme = phonemes.at(i);
        QStringList phonemeParts = phoneme.split(" ");
        int32 phonemeLength = wordLength / phonemeParts.size();
        int32 phonemePos = wordPos;
        for (int j=0; j<phonemeParts.size(); j++) {
            LipsyncPhoneme *ph = new LipsyncPhoneme;
            ph->setText(phonemeParts.at(j));
            ph->setFrame(phonemePos);
            phonemePos += phonemeLength + 1;
            word->addPhoneme(ph);
        }
        phrase->addWord(word);
        wordPos += wordLength + 1;
    }
    voice->addPhrase(phrase);
    document->setCurrentVoice(voice);
    document->setVoiceText(voiceText->toPlainText());
}

void TupPapagayoApp::runManualBreakdownAction()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::runManualBreakdownProcess()] - currentLanguage -> " << currentLanguage;
    #endif

    if (wordsList.isEmpty()) {
        phonemesList.clear();
        document->cleanPhrases();

        waveformView->update();
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::runManualBreakdownProcess()] - Fatal Error: Voice text is empty!";
        #endif
        TOsd::self()->display(TOsd::Error, tr("Voice text is empty!"));
        return;
    }

    if (currentLanguage == English) {
        runBreakdownAction();
    } else {
        TupBreakdownDialog *breakdownDialog = new TupBreakdownDialog(wordsList, phonemesList,
                                                                     currentMouthPath, this);
        if (breakdownDialog->exec() == QDialog::Accepted) {
            document->setModifiedFlag(true);

            if (document->getPhrasesTotal() == 0) {
                loadDocumentFromScratch(breakdownDialog->phomeneList());
            } else {
                QString newText = voiceText->toPlainText();
                QString currentText = document->getVoiceText();

                if (newText.compare(currentText) != 0) {
                    loadDocumentFromScratch(breakdownDialog->phomeneList());
                } else {
                    LipsyncPhrase *phrase = document->getFirstPhrase();
                    if (phrase) {
                        QList<LipsyncWord *> words = phrase->getWords();
                        QStringList phonemes = breakdownDialog->phomeneList();

                        for (int i = 0; i < words.size(); i++) {
                            LipsyncWord *word = words.at(i);
                            if (word) {
                                QStringList phList = phonemes.at(i).split(' ', Qt::SkipEmptyParts);
                                int32 wordLength = word->getEndFrame() - word->getStartFrame();
                                int32 wordPos = word->getStartFrame();
                                int32 phonemePos = wordPos;
                                int32 phonemeLength = wordLength / phList.size();
                                word->removePhonemes();

                                for (int i = 0; i < phList.size(); i++) {
                                    QString phStr = phList.at(i);
                                    if (phStr.isEmpty())
                                        continue;

                                    LipsyncPhoneme *phoneme = new LipsyncPhoneme;
                                    phoneme->setText(phStr);
                                    phoneme->setFrame(phonemePos);
                                    phonemePos += phonemeLength + 1;
                                    word->addPhoneme(phoneme);
                                }
                            }
                        }
                    }
                }
            }
            waveformView->update();
        }
    }
}

void TupPapagayoApp::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::keyPressEvent()]";
    #endif

    if (event->key() == Qt::Key_Space) {
        playVoice();
    } else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        stopVoice();
    }
}

void TupPapagayoApp::updateMouthView(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updateMouthView()] - index -> " << index;
    #endif

    if (index == 5) {
        if (mouthFrame->currentIndex() == Predefined)
            mouthFrame->setCurrentIndex(Customized);

        if (customView->imagesAresLoaded()) {
            currentMouthPath = customView->getMouthsPath();
            waveformView->setMouthsPath(currentMouthPath);
        } else {
            waveformView->setMouthsPath("");
        }
    } else {
        mouthView->onMouthChanged(index);
        if (mouthFrame->currentIndex() == Customized)
            mouthFrame->setCurrentIndex(Predefined);

        currentMouthPath = mouthView->getMouthsPath();
        waveformView->setMouthsPath(currentMouthPath);
    }
}
void TupPapagayoApp::openImagesDialog()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Choose the images directory..."), path,
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dirPath.isEmpty()) {
        QDir dir(dirPath);
        QStringList imagesList = dir.entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg");
        if (imagesList.size() > 0) {
            if (imagesList.count() == 10) { // Mouths set always contains 10 figures
                QString firstImage = imagesList.at(0);
                int dot = firstImage.lastIndexOf(".");
                QString extension = firstImage.mid(dot);
                for (int32 i = 0; i < TupLipsyncDoc::phonemesListSize(); i++) {
                    QString image = TupLipsyncDoc::getPhonemeAt(i) + extension;
                    QString path = dirPath + "/" +  image;
                    if (!QFile::exists(path)) {
                        TOsd::self()->display(TOsd::Error, tr("Mouth image is missing!"));
                        #ifdef TUP_DEBUG
                            qDebug() << "[TupPapagayoApp::openImagesDialog()] - Fatal Error: Image file is missing -> " << path;
                        #endif
                        return;
                    }
                }

                mouthsPath->setText(dirPath);
                saveDefaultPath(dirPath);
                customView->loadImages(dirPath);
                waveformView->setMouthsPath(dirPath);
            } else {
                TOsd::self()->display(TOsd::Error, tr("Mouth images are incomplete!"));
                #ifdef TUP_DEBUG
                    qDebug() << "[TupPapagayoApp::openImagesDialog()] - Fatal Error: Mouth images are incomplete!";
                #endif
            }
        } else {
            TOsd::self()->display(TOsd::Error, tr("Images directory is empty!"));
            #ifdef TUP_DEBUG
                qDebug() << "[TupPapagayoApp::openImagesDialog()] - Fatal Error: Images directory is empty!";
            #endif
        }
    }
}

void TupPapagayoApp::saveDefaultPath(const QString &dir)
{
    TCONFIG->beginGroup("General");
    TCONFIG->setValue("DefaultPath", dir);
    TCONFIG->sync();
}

void TupPapagayoApp::updateFrame(int frame)
{
    if (mouthFrame->currentIndex() == Predefined)
        mouthView->onFrameChanged(frame);
    else
        customView->onFrameChanged(frame);
}

void TupPapagayoApp::updatePauseButton()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::updatePauseButton()]";
    #endif

    playerStopped = true;
    actionPlay->setIcon(playIcon);
    actionPlay->setText(tr("Play"));
    actionPlay->setToolTip(tr("Play"));
}

void TupPapagayoApp::createLipsyncRecord()
{
    QString title = voiceName->text();
    if (title.isEmpty()) {
        TOsd::self()->display(TOsd::Error, tr("Voice name is empty!"));
        return;
    }

    QString words = voiceText->toPlainText();
    if (words.isEmpty()) {
        TOsd::self()->display(TOsd::Error, tr("Voice text is empty!"));
        return;
    }

    int index = mouthsCombo->currentIndex();
    if (index == 5) {
        QString path = mouthsPath->text();
        if (path.isEmpty()) {
            TOsd::self()->display(TOsd::Error, tr("Customized mouths path is unset!"));
            return;
        }
    }

    if (!document) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupPapagayoApp::createLipsyncRecord()] - Warning: No lip-sync document!";
        #endif
        return;
    }

    document->save();

    #ifdef TUP_DEBUG
        qDebug() << "[TupPapagayoApp::confirmCloseDocument()] - Lip-sync item saved successfully!";
    #endif
    TOsd::self()->display(TOsd::Info, tr("Lip-sync item added!"));
    close();
}
