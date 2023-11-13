/***************************************************************************
 *   Project TUPITUBE DESK                           *
 *   Project Contact: info@maefloresta.com                   *
 *   Project Website: http://www.maefloresta.com                    *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>         *
 *                                             *
 *   Developers:                                      *
 *   2010:                                     *
 *    Gustavo Gonzalez / xtingray                            *
 *                                             *
 *   KTooN's versions:                                * 
 *                                             *
 *   2006:                                     *
 *    David Cuadrado                                  *
 *    Jorge Cuadrado                                  *
 *   2003:                                     *
 *    Fernado Roldan                                  *
 *    Simena Dinas                                    *
 *                                             *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                  *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                     *
 *                                             *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                   *
 *                                             *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "rotationsettings.h"
#include "tuivalues.h"
#include "tupitemtweener.h"
#include "tuptweenerstep.h"
#include "tseparator.h"
#include "tosd.h"
#include "talgorithm.h"

#include <math.h>

RotationSettings::RotationSettings(QWidget *parent) : QWidget(parent)
{
    selectionDone = false;
    propertiesDone = false;
    rotationType = TupItemTweener::Continuos;
    stepsCounter = 0;

    QPair<int, int> dimension = TAlgorithm::screenDimension();
    screenHeight = dimension.second;

    layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    nameLabel = new QLabel(tr("Name") + ": ");
    input = new QLineEdit;

    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    nameLayout->setMargin(0);
    nameLayout->setSpacing(0);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(input);

    options = new TRadioButtonGroup(tr("Options"), Qt::Vertical);
    options->addItem(tr("Select object"), 0);
    options->addItem(tr("Set Properties"), 1);
    connect(options, SIGNAL(clicked(int)), this, SLOT(emitOptionChanged(int)));

    apply = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/apply.png"), 22);
    connect(apply, SIGNAL(clicked()), this, SLOT(applyTween()));

    remove = new TImageButton(QPixmap(kAppProp->themeDir() + "icons/close.png"), 22);
    connect(remove, SIGNAL(clicked()), this, SIGNAL(clickedResetTween()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(10);
    buttonsLayout->addWidget(apply);
    buttonsLayout->addWidget(remove);

    layout->addLayout(nameLayout);
    layout->addWidget(options);

    setInnerForm();

    if (screenHeight < HD_HEIGHT) {
        smallFont = this->font();
        smallFont.setPointSize(8);
        setSmallFont();
    }

    layout->addSpacing(10);
    layout->addLayout(buttonsLayout);
    layout->setSpacing(5);

    activateMode(TupToolPlugin::Selection);
}

RotationSettings::~RotationSettings()
{
}

void RotationSettings::setInnerForm()
{
    tabWidget = new QTabWidget();

    basicPanel = new QWidget;

    QBoxLayout *innerLayout = new QBoxLayout(QBoxLayout::TopToBottom, basicPanel);
    innerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    startingLabel = new QLabel(tr("Starting at frame") + ": ");
    startingLabel->setAlignment(Qt::AlignVCenter);

    initFrame = new QSpinBox();
    initFrame->setEnabled(false);
    initFrame->setMaximum(999);
    connect(initFrame, SIGNAL(valueChanged(int)), this, SLOT(updateRangeFromInit(int)));
 
    endingLabel = new QLabel(tr("Ending at frame") + ": ");
    endingLabel->setAlignment(Qt::AlignVCenter);

    endFrame = new QSpinBox();
    endFrame->setEnabled(true);
    endFrame->setValue(1);
    endFrame->setMaximum(999);
    connect(endFrame, SIGNAL(valueChanged(int)), this, SLOT(updateRangeFromEnd(int)));

    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->setAlignment(Qt::AlignHCenter);
    startLayout->setMargin(0);
    startLayout->setSpacing(0);
    startLayout->addWidget(startingLabel);
    startLayout->addWidget(initFrame);

    QHBoxLayout *endLayout = new QHBoxLayout;
    endLayout->setAlignment(Qt::AlignHCenter);
    endLayout->setMargin(0);
    endLayout->setSpacing(0);
    endLayout->addWidget(endingLabel);
    endLayout->addWidget(endFrame);

    totalLabel = new QLabel(tr("Frames Total") + ": 1");
    totalLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    QHBoxLayout *totalLayout = new QHBoxLayout;
    totalLayout->setAlignment(Qt::AlignHCenter);
    totalLayout->setMargin(0);
    totalLayout->setSpacing(0);
    totalLayout->addWidget(totalLabel);

    speedLabel = new QLabel(tr("Speed (Degrees/Frame)") + ": ");
    speedLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    degreesPerFrame = new QDoubleSpinBox;
    degreesPerFrame->setEnabled(true);
    degreesPerFrame->setDecimals(2);
    degreesPerFrame->setMinimum(0.01);
    degreesPerFrame->setMaximum(360);
    degreesPerFrame->setSingleStep(0.05);
    degreesPerFrame->setValue(1);

    QVBoxLayout *speedLayout = new QVBoxLayout;
    speedLayout->setAlignment(Qt::AlignHCenter);
    speedLayout->setMargin(0);
    speedLayout->setSpacing(0);
    speedLayout->addWidget(speedLabel);

    QVBoxLayout *speedLayout2 = new QVBoxLayout;
    speedLayout2->setAlignment(Qt::AlignHCenter);
    speedLayout2->setMargin(0);
    speedLayout2->setSpacing(0);
    speedLayout2->addWidget(degreesPerFrame);

    QBoxLayout *clockLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    clockLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    clockLayout->setMargin(0);
    clockLayout->setSpacing(0);

    directionLabel = new QLabel(tr("Direction") + ": ");
    directionLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    clockCombo = new QComboBox();
    clockCombo->addItem(tr("Clockwise"));
    clockCombo->addItem(tr("Counterclockwise"));

    clockLayout->addSpacing(5);
    clockLayout->addWidget(directionLabel);
    clockLayout->addWidget(clockCombo);
    clockLayout->addSpacing(5);

    advancedPanel = new QWidget;

    rotationTypeCombo = new QComboBox();
    rotationTypeCombo->addItem(tr("Continuous"));
    rotationTypeCombo->addItem(tr("Partial"));

    connect(rotationTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(refreshForm(int)));

    typeLabel = new QLabel(tr("Type") + ": ");
    typeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QHBoxLayout *typeLayout = new QHBoxLayout;
    typeLayout->setAlignment(Qt::AlignHCenter);
    typeLayout->setMargin(0);
    typeLayout->setSpacing(0);
    typeLayout->addWidget(typeLabel);
    typeLayout->addWidget(rotationTypeCombo);

    innerLayout->addLayout(startLayout);
    innerLayout->addLayout(endLayout);
    innerLayout->addLayout(totalLayout);
    innerLayout->addWidget(new TSeparator(Qt::Horizontal));
    innerLayout->addLayout(speedLayout);
    innerLayout->addLayout(speedLayout2);
    innerLayout->addLayout(clockLayout);

    QBoxLayout *advancedLayout = new QBoxLayout(QBoxLayout::TopToBottom, advancedPanel);
    advancedLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    advancedLayout->addLayout(typeLayout);

    setRangeForm();
    advancedLayout->addWidget(rangePanel);

    tabWidget->addTab(basicPanel, tr("Basic"));
    tabWidget->addTab(advancedPanel, tr("Advanced"));
    layout->addWidget(tabWidget);

    activeInnerForm(false);
}

void RotationSettings::setSmallFont()
{
    nameLabel->setFont(smallFont);
    options->setFont(smallFont);
    tabWidget->setFont(smallFont);
    startingLabel->setFont(smallFont);
    endingLabel->setFont(smallFont);
    totalLabel->setFont(smallFont);
    typeLabel->setFont(smallFont);
    speedLabel->setFont(smallFont);
    directionLabel->setFont(smallFont);
    clockCombo->setFont(smallFont);
}

void RotationSettings::activeInnerForm(bool enable)
{
    if (enable && !tabWidget->isVisible()) {
        propertiesDone = true;
        tabWidget->show();
    } else {
        propertiesDone = false;
        tabWidget->hide();
    }
}

void RotationSettings::setRangeForm()
{
    rangePanel = new QWidget;
    QBoxLayout *rangeLayout = new QBoxLayout(QBoxLayout::TopToBottom, rangePanel);
    rangeLayout->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    rangeLayout->setMargin(0);
    rangeLayout->setSpacing(0);
 
    QLabel *rangeLabel = new QLabel(tr("Degrees Range") + ": ");
    rangeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QLabel *startLabel = new QLabel(tr("Start at") + ": ");
    startLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    rangeStart = new QSpinBox;
    rangeStart->setEnabled(true);
    rangeStart->setMinimum(0);
    rangeStart->setMaximum(360);
    connect(rangeStart, SIGNAL(valueChanged(int)), this, SLOT(checkRange(int)));

    QHBoxLayout *startLayout = new QHBoxLayout;
    startLayout->setAlignment(Qt::AlignHCenter);
    startLayout->setMargin(0);
    startLayout->setSpacing(0);
    startLayout->addWidget(startLabel);
    startLayout->addWidget(rangeStart);

    QLabel *endLabel = new QLabel(tr("Finish at") + ": ");
    endLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    rangeEnd = new QSpinBox;
    rangeEnd->setEnabled(true);
    rangeEnd->setMinimum(0);
    rangeEnd->setMaximum(360);
    connect(rangeEnd, SIGNAL(valueChanged(int)), this, SLOT(checkRange(int)));

    QHBoxLayout *endLayout = new QHBoxLayout;
    endLayout->setAlignment(Qt::AlignHCenter);
    endLayout->setMargin(0);
    endLayout->setSpacing(0);
    endLayout->addWidget(endLabel);
    endLayout->addWidget(rangeEnd);

    rangeLoopBox = new QCheckBox(tr("Loop"), rangePanel);

    connect(rangeLoopBox, SIGNAL(stateChanged(int)), this, SLOT(updateReverseCheckbox(int)));

    QVBoxLayout *loopLayout = new QVBoxLayout;
    loopLayout->setAlignment(Qt::AlignHCenter);
    loopLayout->setMargin(0);
    loopLayout->setSpacing(0);
    loopLayout->addWidget(rangeLoopBox);
    reverseLoopBox = new QCheckBox(tr("Loop with Reverse"), rangePanel);

    connect(reverseLoopBox, SIGNAL(stateChanged(int)), this, SLOT(updateRangeCheckbox(int)));

    QVBoxLayout *reverseLayout = new QVBoxLayout;
    reverseLayout->setAlignment(Qt::AlignHCenter);
    reverseLayout->setMargin(0);
    reverseLayout->setSpacing(0);
    reverseLayout->addWidget(reverseLoopBox);

    rangeLayout->addWidget(rangeLabel);
    rangeLayout->addLayout(startLayout);
    rangeLayout->addLayout(endLayout);
    rangeLayout->addSpacing(5);
    rangeLayout->addLayout(loopLayout);
    rangeLayout->addLayout(reverseLayout);

    activeRangeForm(false);
}

void RotationSettings::activeRangeForm(bool enable)
{
    if (enable && !rangePanel->isVisible())
        rangePanel->show();
    else
        rangePanel->hide();
}

// Adding new Tween
void RotationSettings::setParameters(const QString &name, int framesCount, int initFrame)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Settings::setParameters()] - Adding new tween...";
    #endif

    mode = TupToolPlugin::Add;
    input->setText(name);

    activateMode(TupToolPlugin::Selection);

    // Resetting interface
    rotationTypeCombo->setCurrentIndex(0);
    clockCombo->setCurrentIndex(0);
    rangeStart->setValue(0);
    rangeEnd->setValue(10);
    rangeLoopBox->setChecked(false);
    reverseLoopBox->setChecked(false);
    degreesPerFrame->setValue(1.0);

    apply->setToolTip(tr("Save Tween"));
    remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close.png"));
    remove->setToolTip(tr("Cancel Tween"));

    initStartCombo(framesCount, initFrame);
}

// Editing current Tween
void RotationSettings::setParameters(TupItemTweener *currentTween)
{
    #ifdef TUP_DEBUG
        qDebug() << "[Settings::setParameters()] - Editing current tween...";
    #endif

    setEditMode();
    activateMode(TupToolPlugin::Properties);

    input->setText(currentTween->getTweenName());

    initFrame->setEnabled(true);
    initFrame->setValue(currentTween->getInitFrame());

    endFrame->setValue(currentTween->getInitFrame() + currentTween->getFrames());

    int end = endFrame->value();
    updateRangeFromEnd(end);

    rotationTypeCombo->setCurrentIndex(currentTween->tweenRotationType());
    degreesPerFrame->setValue(currentTween->tweenRotateSpeed());
    clockCombo->setCurrentIndex(currentTween->tweenRotateDirection());

    if (currentTween->tweenRotationType() == TupItemTweener::Partial) {
        rangeStart->setValue(currentTween->tweenRotateStartDegree());
        rangeEnd->setValue(currentTween->tweenRotateEndDegree());

        rangeLoopBox->setChecked(currentTween->tweenRotateLoop());
        reverseLoopBox->setChecked(currentTween->tweenRotateReverseLoop());
    }
}

void RotationSettings::initStartCombo(int framesCount, int currentIndex)
{
    initFrame->clear();
    endFrame->clear();

    initFrame->setMinimum(1);
    initFrame->setMaximum(framesCount);
    initFrame->setValue(currentIndex + 1);

    endFrame->setMinimum(1);
    endFrame->setValue(framesCount);
}

void RotationSettings::setStartFrame(int currentIndex)
{
    initFrame->setValue(currentIndex + 1);
    int end = endFrame->value();
    if (end < currentIndex+1)
        endFrame->setValue(currentIndex + 1);
}

int RotationSettings::startFrame()
{
    return initFrame->value() - 1;
}

int RotationSettings::startComboSize()
{
    return initFrame->maximum();
}

int RotationSettings::totalSteps()
{
    return endFrame->value() - (initFrame->value() - 1);
}

void RotationSettings::setEditMode()
{
    mode = TupToolPlugin::Edit;
    apply->setToolTip(tr("Update Tween"));
    remove->setIcon(QPixmap(kAppProp->themeDir() + "icons/close_properties.png"));
    remove->setToolTip(tr("Close Tween Properties"));
}

void RotationSettings::applyTween()
{
    if (!selectionDone) {
        TOsd::self()->display(TOsd::Warning, tr("You must select at least one object!"));
        #ifdef TUP_DEBUG
            qDebug() << "Settings::applyTween() - You must select at least one object!";
        #endif

        return;
    }

    if (!propertiesDone) {
        TOsd::self()->display(TOsd::Warning, tr("You must set Tween properties first!"));
        #ifdef TUP_DEBUG
            qDebug() << "Settings::applyTween() - You must set Tween properties first!";
        #endif

        return;
    }

    if (rotationType == TupItemTweener::Partial) {
        int start = rangeStart->value();
        int end = rangeEnd->value();
        if (start == end) {
            TOsd::self()->display(TOsd::Warning, tr("Angle range must be greater than 0!"));
            #ifdef TUP_DEBUG
                qDebug() << "Settings::applyTween() - Angle range must be greater than 0!";
            #endif

            return;
        }

        int range = abs(start - end); 
        if (range < degreesPerFrame->value()) {
            TOsd::self()->display(TOsd::Warning, tr("Angle range must be greater than Speed!"));
            #ifdef TUP_DEBUG
                qDebug() << "Settings::applyTween() - Angle range must be greater than Speed!";
            #endif

            return;
        }
    }

    // SQA: Verify Tween is really well applied before call setEditMode!
    setEditMode();

    if (!initFrame->isEnabled())
        initFrame->setEnabled(true);

    checkFramesRange();
    emit clickedApplyTween();
}

void RotationSettings::notifySelection(bool flag)
{
    selectionDone = flag;
}

QString RotationSettings::currentTweenName() const
{
    QString tweenName = input->text();
    if (tweenName.length() > 0)
        input->setFocus();

    return tweenName;
}

void RotationSettings::emitOptionChanged(int option)
{
    switch (option) {
        case 0:
        {
            activeInnerForm(false);
            emit clickedSelect();
        }
        break;
        case 1:
        {
            if (selectionDone) {
                activeInnerForm(true);
                emit clickedDefineAngle();
            } else {
                options->setCurrentIndex(0);
                TOsd::self()->display(TOsd::Warning, tr("Select objects for Tweening first!"));
                #ifdef TUP_DEBUG
                    qDebug() << "Settings::emitOptionChanged() - You must set Tween properties first!";
                #endif
            }
        }
    }
}

QString RotationSettings::tweenToXml(int currentScene, int currentLayer, int currentFrame, QPointF point)
{
    QDomDocument doc;

    QDomElement root = doc.createElement("tweening");
    root.setAttribute("name", currentTweenName());
    root.setAttribute("type", TupItemTweener::Rotation);
    root.setAttribute("initFrame", currentFrame);
    root.setAttribute("initLayer", currentLayer);
    root.setAttribute("initScene", currentScene);
    
    root.setAttribute("frames", stepsCounter);

    root.setAttribute("origin", QString::number(point.x()) + "," + QString::number(point.y()));
    root.setAttribute("rotationType", rotationType);
    double speed = degreesPerFrame->value();

    root.setAttribute("rotateSpeed", QString::number(speed));

    int direction = clockCombo->currentIndex();
    root.setAttribute("rotateDirection", direction);

    if (rotationType == TupItemTweener::Continuos) {
        double angle = 0;
        for (int i=0; i < stepsCounter; i++) {
            TupTweenerStep *step = new TupTweenerStep(i);
            step->setRotation(angle);

            root.appendChild(step->toXml(doc));
            if (direction == TupItemTweener::Clockwise)
                angle += speed;
            else
                angle -= speed;
        }
    } else if (rotationType == TupItemTweener::Partial) {
        bool loop = rangeLoopBox->isChecked();
        if (loop)
            root.setAttribute("rotateLoop", "1");
        else
            root.setAttribute("rotateLoop", "0");

        int start = rangeStart->value();
        root.setAttribute("rotateStartDegree", start);

        int end = rangeEnd->value();
        root.setAttribute("rotateEndDegree", end);

        bool reverse = reverseLoopBox->isChecked();
        if (reverse)
            root.setAttribute("rotateReverseLoop", "1");
        else
            root.setAttribute("rotateReverseLoop", "0");

        double angle = start;
        bool token = false;

        double distance = 0;
        if (direction == TupItemTweener::Clockwise) {
            if (start > end)
                distance = 360 - (start - end);
            else
                distance = end - start;
        } else { // CounterClockwise
            if (start > end)
                distance = start - end;
            else
                distance = 360 - (end - start);
        }

        double counter = 0; 
        double go = distance;
        double back = distance - (2*speed);

        for (int i=0; i < stepsCounter; i++) {
             TupTweenerStep *step = new TupTweenerStep(i);
             step->setRotation(angle);
             root.appendChild(step->toXml(doc));

             if (!token) { // going on initial direction
                 if (counter < distance) {
                     if (direction == TupItemTweener::Clockwise)
                         angle += speed;
                     else
                         angle -= speed;

                     if (end < start) {
                         if (angle >= 360)
                             angle = angle - 360;
                     }
                 }
             } else { // returning back
                 if (counter < distance) {
                     if (direction == TupItemTweener::Clockwise)
                         angle -= speed;
                     else
                         angle += speed;

                     if (end < start) {
                         if (angle < 0)
                             angle = 360 - fabs(angle);
                     }
                 }
             }

             if (reverse) {
                 if (counter >= distance) {
                     token = !token;
                     counter = 0;

                     if (direction == TupItemTweener::Clockwise) {
                         angle -= speed;
                         if (angle < 0)
                             angle = 360 - fabs(angle);
                     } else {
                         angle += speed;
                         if (angle >= 360)
                             angle = angle - 360;
                     }

                     if (token)
                         distance = back;
                     else
                         distance = go;
                 } else {
                     counter += speed;
                 }
             } else if (loop && counter >= distance) {
                 angle = start;
                 counter = 0;
             } else {
                 counter += speed;
             }
        }
    }
    doc.appendChild(root);

    return doc.toString();
}

void RotationSettings::activateMode(TupToolPlugin::EditMode mode)
{
    options->setCurrentIndex(mode);
}

void RotationSettings::refreshForm(int type)
{
    if (type == 0) {
        rotationType = TupItemTweener::Continuos;
        activeRangeForm(false);
    } else {
        rotationType = TupItemTweener::Partial;
        activeRangeForm(true);
    }
}

void RotationSettings::checkFramesRange()
{
    int begin = initFrame->value();
    int end = endFrame->value();
       
    if (begin > end) {
        initFrame->blockSignals(true);
        endFrame->blockSignals(true);
        int tmp = end;
        end = begin;
        begin = tmp;
        initFrame->setValue(begin);
        endFrame->setValue(end);
        initFrame->blockSignals(false);
        endFrame->blockSignals(false);
    }

    stepsCounter = end - begin + 1;
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(stepsCounter));
}

void RotationSettings::updateRangeCheckbox(int state)
{
    Q_UNUSED(state)

    if (reverseLoopBox->isChecked() && rangeLoopBox->isChecked())
        rangeLoopBox->setChecked(false);
}

void RotationSettings::updateReverseCheckbox(int state)
{
    Q_UNUSED(state)

    if (reverseLoopBox->isChecked() && rangeLoopBox->isChecked())
        reverseLoopBox->setChecked(false);
}

void RotationSettings::checkRange(int index)
{
    Q_UNUSED(index)

    int start = rangeStart->value();
    int end = rangeEnd->value();

    if (start == end) {
        if (rangeEnd->value() == 360)
            rangeStart->setValue(rangeStart->value() - 1);
        else
            rangeEnd->setValue(rangeEnd->value() + 1);
    }
}

void RotationSettings::updateRangeFromInit(int begin)
{
    int end = endFrame->value();
    stepsCounter = end - begin + 1;
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(stepsCounter));
}

void RotationSettings::updateRangeFromEnd(int end)
{
    int begin = initFrame->value();
    stepsCounter = end - begin + 1;
    totalLabel->setText(tr("Frames Total") + ": " + QString::number(stepsCounter));
}
