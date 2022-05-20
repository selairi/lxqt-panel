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

#include <QVBoxLayout>
#include <QFrame>
#include <QEvent>
#include <QDebug>
#include "sliderdialog.h"

// Number of backlight steps
#define N_BACKLIGHT_STEPS 20
// The first step is related to low backlight intensity.
// This first step is divided in N_BACKLIGHT_LOW_STEPS steps.
// As the eye has got a logarithm response, the first step is specially noticed,
// although are intended to vary the same degree of intensity.
#define N_BACKLIGHT_LOW_STEPS 20

SliderDialog::SliderDialog(QWidget *parent, LXQt::Backlight *backlight) : QDialog(parent, Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint | Qt::Popup | Qt::X11BypassWindowManagerHint)
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint | Qt::Popup | Qt::X11BypassWindowManagerHint);
    m_backlight = backlight;

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(2);

    m_upButton = new QToolButton();
    m_upButton->setText(QStringLiteral("☀"));
    m_upButton->setAutoRepeat(true);
    layout->addWidget(m_upButton, 0, Qt::AlignHCenter);

    m_slider = new QSlider(this);
    layout->addWidget(m_slider, 0, Qt::AlignHCenter);

    m_downButton = new QToolButton();
    m_downButton->setText(QStringLiteral("☼"));
    m_downButton->setAutoRepeat(true);
    layout->addWidget(m_downButton, 0, Qt::AlignHCenter);

    if(m_backlight->isBacklightAvailable()) {
        int minBacklight = 0;
        int maxBacklight = m_backlight->getMaxBacklight();
        int interval = maxBacklight - minBacklight;
        if(interval <= N_BACKLIGHT_STEPS) {
            m_slider->setMaximum(maxBacklight);
            m_slider->setMinimum(minBacklight);
        } else {
            m_slider->setMaximum(N_BACKLIGHT_STEPS + N_BACKLIGHT_LOW_STEPS);
            m_slider->setMinimum(0);
        }
        updateBacklight();
    } else {
        m_slider->setValue(0);
        m_slider->setEnabled(false);
        m_upButton->setEnabled(false);
        m_downButton->setEnabled(false);
    }

    connect(m_slider,     &QSlider::valueChanged, this, &SliderDialog::sliderValueChanged);
    connect(m_upButton,   &QToolButton::clicked,  this, &SliderDialog::upButtonClicked);
    connect(m_downButton, &QToolButton::clicked,  this, &SliderDialog::downButtonClicked);
}


void SliderDialog::sliderValueChanged(int value)
{
    setBacklight(m_backlight, value);
}


void SliderDialog::updateBacklight()
{
    int minBacklight = 0;
    int maxBacklight = m_backlight->getMaxBacklight();
    int interval = maxBacklight - minBacklight;
    int value = m_backlight->getBacklight();
    if(interval > N_BACKLIGHT_STEPS && value > N_BACKLIGHT_LOW_STEPS)
        value = (value * (N_BACKLIGHT_STEPS + N_BACKLIGHT_LOW_STEPS)) / maxBacklight;
    m_slider->setValue(value);
}


void SliderDialog::downButtonClicked(bool)
{
    m_slider->setValue(m_slider->value() - 1);
}


void SliderDialog::upButtonClicked(bool)
{
    m_slider->setValue(m_slider->value() + 1);
}


bool SliderDialog::event(QEvent * event)
{
    if(event->type() == QEvent::WindowDeactivate || event->type() == QEvent::Hide) {
        hide();
        emit dialogClosed();
    }
    return QDialog::event(event);
}

void SliderDialog::setBacklight(LXQt::Backlight *backlight, int value)
{
    int minBacklight = 0;
    int maxBacklight = backlight->getMaxBacklight();
    int interval = maxBacklight - minBacklight;
    if(interval > N_BACKLIGHT_STEPS && value > N_BACKLIGHT_LOW_STEPS)
        value = (value * maxBacklight) / (N_BACKLIGHT_STEPS + N_BACKLIGHT_LOW_STEPS);
    backlight->setBacklight(value);
}

