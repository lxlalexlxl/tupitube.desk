/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustav Gonzalez / xtingray                                           *
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

#include "tupbasiccamerainterface.h"
#include "tseparator.h"
#include "talgorithm.h"
#include "tosd.h"

#include <QIcon>
#include <QDir>
#include <QBoxLayout>
#include <QPushButton>
#include <QScreen>

/*
TupBasicCameraInterface::TupBasicCameraInterface(const QString &title, QList<QCameraInfo> cameraDevices, QComboBox *devicesCombo,
                                                 int cameraIndex, const QSize cameraSize, int i, QWidget *parent) : QFrame(parent)
*/
TupBasicCameraInterface::TupBasicCameraInterface(const QString &title, QComboBox *devicesCombo,
                                                 int cameraIndex, const QSize cameraSize, int i, QWidget *parent) : QFrame(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBasicCameraInterface()]";
    #endif

    setWindowTitle(tr("TupiTube Camera Manager") + " | " + tr("Current resolution:") + " " + title);
    setWindowIcon(QIcon(QPixmap(THEME_DIR + "icons/camera.png")));

    counter = i;
    path = randomPath();
    QSize displaySize = cameraSize;
    widgetStack = new QStackedWidget();
    QScreen *screen = QGuiApplication::screens().at(0);
    int desktopWidth = screen->geometry().width();

    if (cameraSize.width() > desktopWidth) {
        int width = desktopWidth / 2;
        int height = width * cameraSize.height() / cameraSize.width();
        displaySize = QSize(width, height);
    } else {
        int maxWidth = 640;
        if (desktopWidth > 800)
            maxWidth = 800;

        if (cameraSize.width() > maxWidth) {
            int height = maxWidth * cameraSize.height() / cameraSize.width();
            displaySize = QSize(maxWidth, height);
        }
    }

    /*
    if (cameraDevices.count() == 1) {
        // QCameraInfo device = cameraDevices.at(0);
        QCamera *camera = new QCamera(device);
        //  QCameraImageCapture *imageCapture = new QCameraImageCapture(camera);
        //  connect(imageCapture, SIGNAL(imageSaved(int, const QString)),
        //          this, SLOT(imageSavedFromCamera(int, const QString)));
        // QCameraViewfinder *viewfinder = new QCameraViewfinder;
        // viewfinder->setFixedSize(displaySize);
        // camera->setViewfinder(viewfinder);
        cameras << camera;
        // imageCaptors << imageCapture;

        // widgetStack->addWidget(viewfinder);
    } else {
        for (int i=0; i < cameraDevices.size(); i++) {
             QCameraInfo device = cameraDevices.at(i);
             QCamera *camera = new QCamera(device); 
             QCameraImageCapture *imageCapture = new QCameraImageCapture(camera);
             connect(imageCapture, SIGNAL(imageSaved(int, const QString)),
                     this, SLOT(imageSavedFromCamera(int, const QString)));

             QCameraViewfinder *viewfinder = new QCameraViewfinder;
             viewfinder->setFixedSize(displaySize);
             camera->setViewfinder(viewfinder);
             cameras << camera;
             imageCaptors << imageCapture;
             widgetStack->addWidget(viewfinder);
        }
    } 
    */
    widgetStack->setCurrentIndex(cameraIndex);

    QWidget *menuWidget = new QWidget;
    QBoxLayout *menuLayout = new QBoxLayout(QBoxLayout::TopToBottom, menuWidget);

    QLabel *devicesLabel = new QLabel;
    QString deviceString = tr("Cameras");
    if (devicesCombo->count() == 1)
        deviceString = tr("Camera");
    devicesLabel->setText(deviceString);
    devicesLabel->setAlignment(Qt::AlignHCenter);

    QPushButton *clickButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/photo.png")), "");
    clickButton->setIconSize(QSize(20, 20));
    clickButton->setToolTip(tr("Take picture"));
    connect(clickButton, SIGNAL(clicked()), this, SLOT(takePicture()));

    menuLayout->addWidget(devicesLabel);
    if (devicesCombo->count() == 1) {
        QLabel *deviceDesc = new QLabel;
        QFont font = deviceDesc->font();
        font.setBold(true);
        deviceDesc->setFont(font);
        deviceDesc->setText(devicesCombo->itemText(0));
        menuLayout->addWidget(deviceDesc);
    } else {
        menuLayout->addWidget(devicesCombo);
    } 

    QPushButton *exitButton = new QPushButton(QIcon(QPixmap(THEME_DIR + "icons/exit.png")), "");
    exitButton->setIconSize(QSize(20, 20));
    exitButton->setToolTip(tr("Close manager"));
    exitButton->setShortcut(Qt::Key_Escape);
    connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));

    devicesCombo->setCurrentIndex(cameraIndex);
    menuLayout->addWidget(new TSeparator(Qt::Horizontal));
    menuLayout->addWidget(clickButton);
    menuLayout->addWidget(exitButton);
    menuLayout->addStretch(2);

    connect(devicesCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCameraDevice(int)));

    QBoxLayout *dialogLayout = new QBoxLayout(QBoxLayout::LeftToRight, this); 
    dialogLayout->addWidget(widgetStack);
    dialogLayout->addWidget(menuWidget);

    currentCamera = (QCamera *) cameras.at(cameraIndex);
    // currentImageCaptor = (QCameraImageCapture *) imageCaptors.at(cameraIndex);
    currentCamera->start();
}

TupBasicCameraInterface::~TupBasicCameraInterface()
{
    #ifdef TUP_DEBUG
        qDebug() << "[~TupBasicCameraInterface()]";
    #endif
}

void TupBasicCameraInterface::closeEvent(QCloseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBasicCameraInterface::closeEvent()]";
    #endif

    Q_UNUSED(event)

    /*
    if (currentCamera->state() == QCamera::ActiveState)
        currentCamera->stop();
    */

    QDir dir(path);
    foreach (QString file, dir.entryList(QStringList() << "*.jpg")) {
        QString absolute = dir.absolutePath() + "/" + file;
        QFile::remove(absolute);
    }

    if (! dir.rmdir(dir.absolutePath())) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupBasicCameraInterface::closeEvent()} - Fatal Error: Can't remove pictures directory -> " << dir.absolutePath();
        #endif
    }

    emit closed();
}

QString TupBasicCameraInterface::randomPath()
{
    QString imgPath = CACHE_DIR + TAlgorithm::randomString(8);
    QDir dir;
    if (!dir.mkdir(imgPath)) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupBasicCameraInterface::randomPath()] - Fatal Error: Can't create pictures directory -> " << imgPath;
        #endif

        imgPath = "";
        TOsd::self()->display(TOsd::Error, tr("Can't create pictures directory"));
    }

    return imgPath;
}

void TupBasicCameraInterface::takePicture()
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBasicCameraInterface::takePicture()]";
    #endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QString prev = "pic";
    if (counter < 10)
        prev += "00";
    if (counter >= 10 && counter < 100)
        prev += "0";

    QString imagePath = path + "/" + prev + QString::number(counter) + ".jpg";

    // on half pressed shutter button
    // currentCamera->searchAndLock();

    // on shutter button pressed
    // currentImageCaptor->capture(imagePath);

    // on shutter button released
    // currentCamera->unlock();
}

void TupBasicCameraInterface::changeCameraDevice(int index)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBasicCameraInterface::changeCameraDevice()] - index -> " << index;
    #endif

    currentCamera->stop();
    widgetStack->setCurrentIndex(index);
    currentCamera = (QCamera *) cameras.at(index);
    // currentImageCaptor = (QCameraImageCapture *) imageCaptors.at(index);

    currentCamera->start();
}

void TupBasicCameraInterface::imageSavedFromCamera(int id, const QString folder)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupBasicCameraInterface::imageSavedFromCamera()] - id -> " << id;
    #endif

    Q_UNUSED(id)

    if (folder.isEmpty())
        return;

    emit pictureHasBeenSelected(counter, folder);
    counter++;
    QApplication::restoreOverrideCursor();
}
