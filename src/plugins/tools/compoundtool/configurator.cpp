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

#include "configurator.h"
#include "tweenmanager.h"
#include "buttonspanel.h"

#include "ktitemtweener.h"
#include "kosd.h"
#include "kdebug.h"

#include <QLabel>
#include <QBoxLayout>

struct Configurator::Private
{
    QBoxLayout *layout;
    QBoxLayout *settingsLayout;
    TweenerPanel *tweenList;
    TweenManager *tweenManager;
    ButtonsPanel *controlPanel;

    KTItemTweener *currentTween;

    int framesTotal;
    int currentFrame;

    TweenerPanel::Mode mode;
    GuiState state;
};

Configurator::Configurator(QWidget *parent) : QFrame(parent), k(new Private)
{
    k->framesTotal = 1;
    k->currentFrame = 0;

    k->mode = TweenerPanel::View;
    k->state = Manager;

    k->layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    k->layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QLabel *title = new QLabel(tr("Compound Tween"));
    title->setAlignment(Qt::AlignHCenter);
    title->setFont(QFont("Arial", 8, QFont::Bold));

    k->layout->addWidget(title);

    k->settingsLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    k->settingsLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    k->settingsLayout->setMargin(0);
    k->settingsLayout->setSpacing(0);

    setTweenManagerPanel();
    setButtonsPanel();
    setTweenerPanel();

    k->layout->addLayout(k->settingsLayout);
    k->layout->addStretch(2);
}

Configurator::~Configurator()
{
    delete k;
}

void Configurator::loadTweenList(QList<QString> tweenList)
{
    k->tweenManager->loadTweenList(tweenList);
    if (tweenList.count() > 0)
        activeButtonsPanel(true);
}

void Configurator::setTweenerPanel()
{
    k->tweenList = new TweenerPanel(this);

    connect(k->tweenList, SIGNAL(clickedSelect()), this, SIGNAL(clickedSelect()));
    connect(k->tweenList, SIGNAL(clickedTweenProperties()), this, SIGNAL(clickedTweenProperties()));
    connect(k->tweenList, SIGNAL(clickedResetTween()), this, SLOT(closeTweenList()));

    k->settingsLayout->addWidget(k->tweenList);
    activeTweenerPanel(false);

    /*
    k->settingsPanel = new Settings(this);

    connect(k->settingsPanel, SIGNAL(startingPointChanged(int)), this, SIGNAL(startingPointChanged(int)));
    connect(k->settingsPanel, SIGNAL(clickedSelect()), this, SIGNAL(clickedSelect()));
    connect(k->settingsPanel, SIGNAL(clickedDefineProperties()), this, SIGNAL(clickedDefineProperties()));
    connect(k->settingsPanel, SIGNAL(clickedApplyTween()), this, SLOT(applyItem()));
    connect(k->settingsPanel, SIGNAL(clickedResetTween()), this, SLOT(closeTweenList()));

    k->settingsLayout->addWidget(k->settingsPanel);

    activePropertiesPanel(false);
    */
}

void Configurator::activeTweenerPanel(bool enable)
{
    if (enable)
        k->tweenList->show();
    else
        k->tweenList->hide();
}

void Configurator::setCurrentTween(KTItemTweener *currentTween)
{
    k->currentTween = currentTween;
}

void Configurator::setTweenManagerPanel()
{
    k->tweenManager = new TweenManager(this);
    connect(k->tweenManager, SIGNAL(addNewTween(const QString &)), this, SLOT(addTween(const QString &)));
    connect(k->tweenManager, SIGNAL(editCurrentTween(const QString &)), this, SLOT(editTween()));
    connect(k->tweenManager, SIGNAL(removeCurrentTween(const QString &)), this, SLOT(removeTween(const QString &)));
    connect(k->tweenManager, SIGNAL(getTweenData(const QString &)), this, SLOT(updateTweenData(const QString &)));

    k->settingsLayout->addWidget(k->tweenManager);
    k->state = Manager;
}

void Configurator::activeTweenManagerPanel(bool enable)
{
    if (enable)
        k->tweenManager->show();
    else
        k->tweenManager->hide();

    if (k->tweenManager->listSize() > 0)
        activeButtonsPanel(enable);
}

void Configurator::setButtonsPanel()
{
    k->controlPanel = new ButtonsPanel(this);
    connect(k->controlPanel, SIGNAL(clickedEditTween()), this, SLOT(editTween()));
    connect(k->controlPanel, SIGNAL(clickedRemoveTween()), this, SLOT(removeTween()));

    k->settingsLayout->addWidget(k->controlPanel);

    activeButtonsPanel(false);
}

void Configurator::activeButtonsPanel(bool enable)
{
    if (enable)
        k->controlPanel->show();
    else
        k->controlPanel->hide();
}

void Configurator::initStartCombo(int framesTotal, int currentFrame)
{
    k->framesTotal = framesTotal;
    k->currentFrame = currentFrame;
    // k->settingsPanel->initStartCombo(framesTotal, currentFrame);
}

void Configurator::setStartFrame(int currentIndex)
{
    k->currentFrame = currentIndex;
    // k->settingsPanel->setStartFrame(currentIndex);
}

int Configurator::startComboSize()
{
    // return k->settingsPanel->startComboSize();
       return 1;
}

void Configurator::updateSteps(const QGraphicsPathItem *path)
{
    // k->settingsPanel->updateSteps(path);
}

QString Configurator::tweenToXml(int currentFrame, QPointF point, QString &route)
{
    // return k->settingsPanel->tweenToXml(currentFrame, point);
    return "";
}

int Configurator::totalSteps()
{
    // return k->settingsPanel->totalSteps();
    return 1;
}

void Configurator::activateSelectionMode()
{
    // k->settingsPanel->activateSelectionMode();
}

void Configurator::addTween(const QString &name)
{
    activeTweenManagerPanel(false);

    k->mode = TweenerPanel::Add;
    k->state = Configurator::TweenerList;

    k->tweenList->setParameters(name, k->framesTotal, k->currentFrame);
    activeTweenerPanel(true);

    emit setMode(k->mode);
}

void Configurator::editTween()
{
    activeTweenManagerPanel(false);

    k->mode = TweenerPanel::Edit;
    k->state = TweenerList;

    // k->settingsPanel->notifySelection(true);
    // k->settingsPanel->setParameters(k->currentTween);
    activeTweenerPanel(true);

    emit setMode(k->mode);
}

void Configurator::removeTween()
{
    QString name = k->tweenManager->currentTweenName();
    k->tweenManager->removeItemFromList();

    removeTween(name);
}

void Configurator::removeTween(const QString &name)
{
    if (k->tweenManager->listSize() == 0)
        activeButtonsPanel(false);

    emit clickedRemoveTween(name);
}

QString Configurator::currentTweenName() const
{
    QString oldName = k->tweenManager->currentTweenName();
    // QString newName = k->settingsPanel->currentTweenName();
    QString newName = oldName;

    if (oldName.compare(newName) != 0)
        k->tweenManager->updateTweenName(newName);

    return newName;
}

void Configurator::notifySelection(bool flag)
{
    k->tweenList->notifySelection(flag);
}

void Configurator::cleanData()
{
    // k->settingsPanel->cleanData();
}

void Configurator::closeTweenList()
{
    if (k->mode == TweenerPanel::Add) {
        k->tweenManager->removeItemFromList();
    } else if (k->mode == TweenerPanel::Edit) {
        closeTweenerPanel();
    }

    emit clickedResetInterface();

    closeTweenerPanel();
}

void Configurator::closeTweenerPanel()
{
    if (k->state == TweenerList) {
        activeTweenManagerPanel(true);
        activeTweenerPanel(false);
        k->mode = TweenerPanel::View;
        k->state = Manager;
    }
}

TweenerPanel::Mode Configurator::mode()
{
    return k->mode;
}

void Configurator::applyItem()
{
     k->mode = TweenerPanel::Edit;
     emit clickedApplyTween();
}

void Configurator::resetUI()
{
    k->tweenManager->resetUI();
    closeTweenerPanel();
    // k->settingsPanel->notifySelection(false);
}

void Configurator::updateTweenData(const QString &name)
{
    emit getTweenData(name);
}
