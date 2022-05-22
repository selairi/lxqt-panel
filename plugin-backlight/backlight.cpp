/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Copyright: 2020 LXQt team
 * Authors:
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "backlight.h"
#include <QEvent>
#include <lxqt-globalkeys.h>
#include <LXQt/Notification>
#include "../panel/pluginsettings.h"

#define DEFAULT_UP_SHORTCUT "XF86MonBrightnessUp"
#define DEFAULT_DOWN_SHORTCUT "XF86MonBrightnessDown"

LXQtBacklight::LXQtBacklight(const ILXQtPanelPluginStartupInfo &startupInfo):
        QObject(),
        ILXQtPanelPlugin(startupInfo)
{
    m_backlight = new LXQt::Backlight(this);
    m_notification = new LXQt::Notification(QLatin1String(""), this);

    m_backlightButton = new QToolButton();
    // use our own icon
    m_backlightButton->setIcon(QIcon::fromTheme(QStringLiteral("brightnesssettings")));

    connect(m_backlightButton, &QToolButton::clicked, this, &LXQtBacklight::showSlider);

    m_backlightSlider = nullptr;

    m_keyBacklightUp = GlobalKeyShortcut::Client::instance()->addAction(
        QString(), QStringLiteral("/panel/%1/up").arg(settings()->group()),
        tr("Increase backlight"), this);
    if (m_keyBacklightUp) {
        connect(m_keyBacklightUp, &GlobalKeyShortcut::Action::registrationFinished,
            this, &LXQtBacklight::shortcutRegistered);
        connect(m_keyBacklightUp, &GlobalKeyShortcut::Action::activated,
            this, &LXQtBacklight::handleShortcutBacklightUp);
    }
    m_keyBacklightDown = GlobalKeyShortcut::Client::instance()->addAction(
        QString(), QStringLiteral("/panel/%1/down").arg(settings()->group()),
        tr("Decrease backlight"), this);
    if (m_keyBacklightDown) {
        connect(m_keyBacklightDown, &GlobalKeyShortcut::Action::registrationFinished,
            this, &LXQtBacklight::shortcutRegistered);
        connect(m_keyBacklightDown, &GlobalKeyShortcut::Action::activated,
            this, &LXQtBacklight::handleShortcutBacklightDown);
    }
}


LXQtBacklight::~LXQtBacklight()
{
    delete m_backlightButton;
}


QWidget *LXQtBacklight::widget()
{
    return m_backlightButton;
}

void LXQtBacklight::deleteSlider()
{
    // Free slider and timer to save memory.
    if(m_backlightSlider) {
        m_backlightSlider->deleteLater();
        m_backlightSlider = nullptr;
    }
}

void LXQtBacklight::showSlider(bool)
{
    if(! m_backlightSlider) {
        m_backlightSlider = new SliderDialog(m_backlightButton, m_backlight);
        connect(m_backlightSlider, &SliderDialog::dialogClosed, this, &LXQtBacklight::deleteSlider);
    }
    QSize size = m_backlightSlider->sizeHint();
    QRect rect = calculatePopupWindowPos(size);
    m_backlightSlider->setGeometry(rect);
    //m_backlightSlider->updateBacklight();
    m_backlightSlider->show();
    m_backlightSlider->setFocus();
}

void LXQtBacklight::shortcutRegistered()
{
    GlobalKeyShortcut::Action * const shortcut = qobject_cast<GlobalKeyShortcut::Action*>(sender());

    QString shortcutNotRegistered;

    if (shortcut == m_keyBacklightUp) {
        disconnect(m_keyBacklightUp, &GlobalKeyShortcut::Action::registrationFinished,
            this, &LXQtBacklight::shortcutRegistered);

        if (m_keyBacklightUp->shortcut().isEmpty())
        {
            m_keyBacklightUp->changeShortcut(QStringLiteral(DEFAULT_UP_SHORTCUT));
            if (m_keyBacklightUp->shortcut().isEmpty())
                shortcutNotRegistered = QStringLiteral(" '") + QStringLiteral(DEFAULT_UP_SHORTCUT) +
                    QStringLiteral("'");
        }
    } else if (shortcut == m_keyBacklightDown) {
        disconnect(m_keyBacklightDown, &GlobalKeyShortcut::Action::registrationFinished,
            this, &LXQtBacklight::shortcutRegistered);

        if (m_keyBacklightDown->shortcut().isEmpty())
        {
            m_keyBacklightDown->changeShortcut(QStringLiteral(DEFAULT_DOWN_SHORTCUT));
            if (m_keyBacklightDown->shortcut().isEmpty())
                shortcutNotRegistered += QStringLiteral(" '") + QStringLiteral(DEFAULT_DOWN_SHORTCUT) +
                    QStringLiteral("'");
        }
    }

    if(!shortcutNotRegistered.isEmpty())
    {
        m_notification->setSummary(
            tr("Backlight Control: The following shortcuts can not be registered: %1")
                .arg(shortcutNotRegistered));
        m_notification->update();
    }

    m_notification->setTimeout(1000);
    m_notification->setUrgencyHint(LXQt::Notification::UrgencyLow);
}

void LXQtBacklight::handleShortcutBacklightDown()
{
    int step = getBacklightStep();
    setBacklightStep(step - 1);
}

void LXQtBacklight::handleShortcutBacklightUp()
{
    int step = getBacklightStep();
    setBacklightStep(step + 1);
}


// Number of backlight steps
#define N_BACKLIGHT_STEPS 20
// The first step is related to low backlight intensity.
// This first step is divided in N_BACKLIGHT_LOW_STEPS steps.
// As the eye has got a logarithm response, the first step is specially noticed,
// although are intended to vary the same degree of intensity.
#define N_BACKLIGHT_LOW_STEPS 20

void LXQtBacklight::setBacklightStep(int value)
{
    int minBacklight = 0;
    int maxBacklight = m_backlight->getMaxBacklight();
    int interval = maxBacklight - minBacklight;
    int backlightAtFirstStep = N_BACKLIGHT_LOW_STEPS;
    if(interval > N_BACKLIGHT_STEPS && value > N_BACKLIGHT_LOW_STEPS)
        value = (int)((float)((value - N_BACKLIGHT_LOW_STEPS) * (maxBacklight - backlightAtFirstStep)) / (float)(N_BACKLIGHT_STEPS) + 0.5) + backlightAtFirstStep;
    m_backlight->setBacklight(value);
}

int LXQtBacklight::getBacklightStep()
{
    int minBacklight = 0;
    int maxBacklight = m_backlight->getMaxBacklight();
    int interval = maxBacklight - minBacklight;
    int backlightAtFirstStep = N_BACKLIGHT_LOW_STEPS;
    int value = m_backlight->getBacklight();
    if(interval > N_BACKLIGHT_STEPS && value > backlightAtFirstStep)
        value = (int)( (float)((value - N_BACKLIGHT_LOW_STEPS) * N_BACKLIGHT_STEPS) / (float)(maxBacklight - backlightAtFirstStep) + 0.5 ) + N_BACKLIGHT_LOW_STEPS;
    return value;
}

