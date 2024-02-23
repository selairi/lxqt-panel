/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Copyright: 2011 Razor team
 *            2014 LXQt team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *   Maciej Płaza <plaza.maciej@gmail.com>
 *   Kuzma Shapran <kuzma.shapran@gmail.com>
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

#include "lxqttaskgroup.h"
#include "lxqttaskbar.h"

#include <QDebug>
#include <QMimeData>
#include <QFocusEvent>
#include <QDragLeaveEvent>
#include <QStringBuilder>
#include <QMenu>
#include <XdgIcon>
#include <KWindowSystem/KX11Extras>
#include <QX11Info>
#include <functional>

/************************************************

 ************************************************/
LXQtTaskGroup::LXQtTaskGroup(const QString &groupName, uint window, DBusAppListener *dbusAppListener, LXQtTaskBar2 *parent)
    : LXQtTaskButton(window, dbusAppListener, parent, parent),
    mGroupName(groupName),
    mPreventPopup(false),
    mSingleButton(true)
{
    qDebug() << "[LXQtTaskGroup::LXQtTaskGroup] New group for window " << window;
    Q_ASSERT(parent);
    mDbusAppListener = dbusAppListener;
    mPopup = new LXQtGroupPopup(this);

    qDebug() << "[LXQtTaskGroup::LXQtTaskGroup] New group for groupName " << groupName;

    setObjectName(groupName);
    setText(groupName);

    connect(this,                  &LXQtTaskGroup::clicked,               this, &LXQtTaskGroup::onClicked);
    //connect(KX11Extras::self(),    &KX11Extras::currentDesktopChanged,    this, &LXQtTaskGroup::onDesktopChanged);
    //connect(KX11Extras::self(),    &KX11Extras::activeWindowChanged,      this, &LXQtTaskGroup::onActiveWindowChanged);
    connect(mDbusAppListener,      &DBusAppListener::windowActivated,    this, &LXQtTaskGroup::onActiveWindowChanged);
    connect(mDbusAppListener,      &DBusAppListener::windowClosed,    this, &LXQtTaskGroup::onWindowRemoved);
    connect(mDbusAppListener,      &DBusAppListener::windowTitleChanged,    this, &LXQtTaskGroup::onWindowTitleChanged);
    connect(parent,                &LXQtTaskBar2::buttonRotationRefreshed, this, &LXQtTaskGroup::setAutoRotation);
    connect(parent,                &LXQtTaskBar2::refreshIconGeometry,     this, &LXQtTaskGroup::refreshIconsGeometry);
    connect(parent,                &LXQtTaskBar2::buttonStyleRefreshed,    this, &LXQtTaskGroup::setToolButtonsStyle);
    connect(parent,                &LXQtTaskBar2::showOnlySettingChanged,  this, &LXQtTaskGroup::refreshVisibility);
    connect(parent,                &LXQtTaskBar2::popupShown,              this, &LXQtTaskGroup::groupPopupShown);
}

/************************************************

 ************************************************/
void LXQtTaskGroup::contextMenuEvent(QContextMenuEvent *event)
{
    setPopupVisible(false, true);
    if (mSingleButton)
    {
        LXQtTaskButton::contextMenuEvent(event);
        return;
    }
    mPreventPopup = true;
    QMenu * menu = new QMenu(tr("Group"));
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction *a = menu->addAction(XdgIcon::fromTheme(QStringLiteral("process-stop")), tr("Close group"));
    connect(a,    &QAction::triggered, this, &LXQtTaskGroup::closeGroup);
    connect(menu, &QMenu::aboutToHide, this, [this] {
        mPreventPopup = false;
    });
    menu->setGeometry(plugin()->panel()->calculatePopupWindowPos(mapToGlobal(event->pos()), menu->sizeHint()));
    plugin()->willShowWindow(menu);
    menu->show();
}

/************************************************

 ************************************************/
void LXQtTaskGroup::closeGroup()
{
    for (LXQtTaskButton *button : qAsConst(mButtonHash) )
        if (button->isOnDesktop(KX11Extras::currentDesktop()))
            button->closeApplication();
}

/************************************************

 ************************************************/
LXQtTaskButton * LXQtTaskGroup::addWindow(uint id)
{
    if (mButtonHash.contains(id))
        return mButtonHash.value(id);

    LXQtTaskButton *btn = new LXQtTaskButton(id, mDbusAppListener, parentTaskBar2(), mPopup);
    btn->setToolButtonStyle(popupButtonStyle());

    //if (btn->isApplicationActive())
    {
        btn->setChecked(true);
        setChecked(true);
    }

    mButtonHash.insert(id, btn);
    mPopup->addButton(btn);
    qDebug() << "\033[42m;[LXQtTaskGroup::addWindow]\033[0m; " << id << mButtonHash.contains(id);

    connect(btn, &LXQtTaskButton::clicked, this, &LXQtTaskGroup::onChildButtonClicked);
    refreshVisibility();

    return btn;
}

/************************************************

 ************************************************/
LXQtTaskButton * LXQtTaskGroup::checkedButton() const
{
    for (LXQtTaskButton* button : qAsConst(mButtonHash))
        if (button->isChecked())
            return button;

    return nullptr;
}

/************************************************

 ************************************************/
LXQtTaskButton * LXQtTaskGroup::getNextPrevChildButton(bool next, bool circular)
{
    LXQtTaskButton *button = checkedButton();
    int idx = mPopup->indexOf(button);
    int inc = next ? 1 : -1;
    idx += inc;

    // if there is no checked button, get the first one if next equals true
    // or the last one if not
    if (!button)
    {
        idx = -1;
        if (next)
        {
            for (int i = 0; i < mPopup->count() && idx == -1; i++)
                if (mPopup->itemAt(i)->widget()->isVisibleTo(mPopup))
                    idx = i;
        }
        else
        {
            for (int i = mPopup->count() - 1; i >= 0 && idx == -1; i--)
                if (mPopup->itemAt(i)->widget()->isVisibleTo(mPopup))
                    idx = i;
        }
    }

    if (circular)
        idx = (idx + mButtonHash.count()) % mButtonHash.count();
    else if (mPopup->count() <= idx || idx < 0)
        return nullptr;

    // return the next or the previous child
    QLayoutItem *item = mPopup->itemAt(idx);
    if (item)
    {
        button = qobject_cast<LXQtTaskButton*>(item->widget());
        if (button->isVisibleTo(mPopup))
            return button;
    }

    return nullptr;
}

/************************************************

 ************************************************/
void LXQtTaskGroup::onActiveWindowChanged(uint window)
{
  qDebug() << "[LXQtTaskGroup::onActiveWindowChanged] " << window << " " << mDbusAppListener->getWindowTitle(window);
    LXQtTaskButton *button = mButtonHash.value(window, nullptr);
    for (LXQtTaskButton *btn : qAsConst(mButtonHash))
        btn->setChecked(false);

    if (button)
    {
        qDebug() << "[LXQtTaskGroup::onActiveWindowChanged] Window found " << window << " " << mDbusAppListener->getWindowTitle(window);
        qDebug() << "[LXQtTaskGroup::onActiveWindowChanged] Window found " << window << " " << button->text();
        button->setChecked(true);
        if (button->hasUrgencyHint())
            button->setUrgencyHint(false);
    } else {
        qDebug() << "[LXQtTaskGroup::onActiveWindowChanged] Window not found " << window << " " << mDbusAppListener->getWindowTitle(window);
        for(uint id : mButtonHash.keys()) {
          qDebug() << "[LXQtTaskGroup::onActiveWindowChanged] Windows " << id << " " << mDbusAppListener->getWindowTitle(id);
        }
    }
    setChecked(nullptr != button);
}

/************************************************

 ************************************************/
void LXQtTaskGroup::onWindowTitleChanged(uint window, QString title)
{
  qDebug() << "[LXQtTaskGroup::onWindowTitleChanged] " << window << " " << title;
  LXQtTaskButton *button = mButtonHash.value(window, nullptr);
  if (button) {
    qDebug() << "\033[43m;[LXQtTaskGroup::onWindowTitleChanged]\033[0m;" << window << " old title " << button->text();
    button->updateText();
    if(buttonsCount() == 1)
      setText(title);
  } else
    qDebug() << "[LXQtTaskGroup::onWindowTitleChanged] Window not found " << window << " " << title << " " << mButtonHash.contains(window);
}

/************************************************

 ************************************************/
void LXQtTaskGroup::onDesktopChanged(int /*number*/)
{
    refreshVisibility();
}

/************************************************

 ************************************************/
void LXQtTaskGroup::onWindowRemoved(uint window)
{
    if (mButtonHash.contains(window))
    {
        LXQtTaskButton *button = mButtonHash.value(window);
        qDebug() << "\033[41m;[LXQtTaskGroup::onWindowRemoved]\033[0m; " << window << " " << button->text();
        mButtonHash.remove(window);
        mPopup->removeWidget(button);
        button->deleteLater();

        if (mButtonHash.count())
            regroup();
        else
        {
            if (isVisible())
                emit visibilityChanged(false);
            hide();
            emit groupBecomeEmpty(groupName());
        }
    }
}

/************************************************

 ************************************************/
void LXQtTaskGroup::onChildButtonClicked()
{
    setPopupVisible(false, true);
}

/************************************************

 ************************************************/
Qt::ToolButtonStyle LXQtTaskGroup::popupButtonStyle() const
{
    // do not set icons-only style in the buttons in the group,
    // as they'll be indistinguishable
    const Qt::ToolButtonStyle style = toolButtonStyle();
    return style == Qt::ToolButtonIconOnly ? Qt::ToolButtonTextBesideIcon : style;
}

/************************************************

 ************************************************/
void LXQtTaskGroup::setToolButtonsStyle(Qt::ToolButtonStyle style)
{
    setToolButtonStyle(style);

    const Qt::ToolButtonStyle styleInPopup = popupButtonStyle();
    for (auto & button : mButtonHash)
    {
        button->setToolButtonStyle(styleInPopup);
    }
}

/************************************************

 ************************************************/
int LXQtTaskGroup::buttonsCount() const
{
    return mButtonHash.count();
}

/************************************************

 ************************************************/
int LXQtTaskGroup::visibleButtonsCount() const
{
    int i = 0;
    for (LXQtTaskButton *btn : qAsConst(mButtonHash))
        if (btn->isVisibleTo(mPopup))
            i++;
    return i;
}

/************************************************

 ************************************************/
void LXQtTaskGroup::draggingTimerTimeout()
{
    if (mSingleButton)
        setPopupVisible(false);
}

/************************************************

 ************************************************/
void LXQtTaskGroup::onClicked(bool)
{
    if (visibleButtonsCount() > 1)
    {
        setChecked(mButtonHash.contains(KX11Extras::activeWindow()));
        setPopupVisible(true);
    }
}

/************************************************

 ************************************************/
void LXQtTaskGroup::regroup()
{
    int cont = visibleButtonsCount();
    recalculateFrameIfVisible();

    if (cont == 1)
    {
        mSingleButton = true;
        // Get first visible button
        LXQtTaskButton * button = nullptr;
        for (LXQtTaskButton *btn : qAsConst(mButtonHash))
        {
            if (btn->isVisibleTo(mPopup))
            {
                button = btn;
                break;
            }
        }

        if (button)
        {
            setText(button->text());
            setToolTip(button->toolTip());
            setWindowId(button->windowId());
        }
    }
    else if (cont == 0)
        hide();
    else
    {
        mSingleButton = false;
        QString t = QString(QStringLiteral("%1 - %2 windows")).arg(mGroupName).arg(cont);
        setText(t);
        setToolTip(parentTaskBar2()->isShowGroupOnHover() ? QString() : t);
    }
}

/************************************************

 ************************************************/
void LXQtTaskGroup::recalculateFrameIfVisible()
{
    if (mPopup->isVisible())
    {
        recalculateFrameSize();
        if (plugin()->panel()->position() == ILXQtPanel::PositionBottom)
            recalculateFramePosition();
    }
}

/************************************************

 ************************************************/
void LXQtTaskGroup::setAutoRotation(bool value, ILXQtPanel::Position position)
{
    for (LXQtTaskButton *button : qAsConst(mButtonHash))
        button->setAutoRotation(false, position);

    LXQtTaskButton::setAutoRotation(value, position);
}

/************************************************

 ************************************************/
void LXQtTaskGroup::refreshVisibility()
{
    bool will = false;
    LXQtTaskBar2 const * taskbar = parentTaskBar2();
    const int showDesktop = taskbar->showDesktopNum();
    for(LXQtTaskButton * btn : qAsConst(mButtonHash))
    {
        bool visible = taskbar->isShowOnlyOneDesktopTasks() ? btn->isOnDesktop(0 == showDesktop ? KX11Extras::currentDesktop() : showDesktop) : true;
        visible &= taskbar->isShowOnlyCurrentScreenTasks() ? btn->isOnCurrentScreen() : true;
        visible &= taskbar->isShowOnlyMinimizedTasks() ? btn->isMinimized() : true;
        btn->setVisible(visible);
        will |= visible;
    }

    bool is = isVisible();
    setVisible(will);
    regroup();

    if (is != will)
        emit visibilityChanged(will);
}

/************************************************

 ************************************************/
QMimeData * LXQtTaskGroup::mimeData()
{
    QMimeData *mimedata = new QMimeData;
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    stream << groupName();
    mimedata->setData(mimeDataFormat(), byteArray);
    return mimedata;
}

/************************************************

 ************************************************/
void LXQtTaskGroup::setPopupVisible(bool visible, bool fast)
{
    if (visible && !mPreventPopup && !mSingleButton)
    {
        if (!mPopup->isVisible())
        {
            // setup geometry
            recalculateFrameSize();
            recalculateFramePosition();
        }

        plugin()->willShowWindow(mPopup);
        mPopup->show();
        emit popupShown(this);
    }
    else
        mPopup->hide(fast);
}

/************************************************

 ************************************************/
void LXQtTaskGroup::refreshIconsGeometry()
{
    QRect rect = geometry();
    rect.moveTo(mapToGlobal(QPoint(0, 0)));

    if (mSingleButton)
    {
        refreshIconGeometry(rect);
        return;
    }

    for(LXQtTaskButton *but : qAsConst(mButtonHash))
    {
        but->refreshIconGeometry(rect);
        but->setIconSize(QSize(plugin()->panel()->iconSize(), plugin()->panel()->iconSize()));
    }
}

/************************************************

 ************************************************/
QSize LXQtTaskGroup::recalculateFrameSize()
{
    int height = recalculateFrameHeight();
    mPopup->setMaximumHeight(1000);
    mPopup->setMinimumHeight(0);

    int hh = recalculateFrameWidth();
    mPopup->setMaximumWidth(hh);
    mPopup->setMinimumWidth(0);

    QSize newSize(hh, height);
    mPopup->resize(newSize);

    return newSize;
}

/************************************************

 ************************************************/
int LXQtTaskGroup::recalculateFrameHeight() const
{
    int cont = visibleButtonsCount();
    int h = !plugin()->panel()->isHorizontal() && parentTaskBar2()->isAutoRotate() ? width() : height();
    return cont * h + (cont + 1) * mPopup->spacing();
}

/************************************************

 ************************************************/
int LXQtTaskGroup::recalculateFrameWidth() const
{
    const QFontMetrics fm = fontMetrics();
    int max = 100 * fm.horizontalAdvance(QLatin1Char(' ')); // elide after the max width
    int txtWidth = 0;
    for (LXQtTaskButton *btn : qAsConst(mButtonHash))
        txtWidth = qMax(fm.horizontalAdvance(btn->text()), txtWidth);
    return iconSize().width() + qMin(txtWidth, max) + 30/* give enough room to margins and borders*/;
}

/************************************************

 ************************************************/
QPoint LXQtTaskGroup::recalculateFramePosition()
{
    // Set position
    int x_offset = 0, y_offset = 0;
    switch (plugin()->panel()->position())
    {
    case ILXQtPanel::PositionTop:
        y_offset += height();
        break;
    case ILXQtPanel::PositionBottom:
        y_offset = -recalculateFrameHeight();
        break;
    case ILXQtPanel::PositionLeft:
        x_offset += width();
        break;
    case ILXQtPanel::PositionRight:
        x_offset = -recalculateFrameWidth();
        break;
    }

    QPoint pos = mapToGlobal(QPoint(x_offset, y_offset));
    mPopup->move(pos);

    return pos;
}

/************************************************

 ************************************************/
void LXQtTaskGroup::leaveEvent(QEvent *event)
{
    setPopupVisible(false);
    QToolButton::leaveEvent(event);
}

/************************************************

 ************************************************/
void LXQtTaskGroup::enterEvent(QEvent *event)
{
    QToolButton::enterEvent(event);

    if (sDraggging)
        return;

    if (parentTaskBar2()->isShowGroupOnHover())
        setPopupVisible(true);
}

/************************************************

 ************************************************/
void LXQtTaskGroup::dragEnterEvent(QDragEnterEvent *event)
{
    // only show the popup if we aren't dragging a taskgroup
    if (!event->mimeData()->hasFormat(mimeDataFormat()))
    {
        setPopupVisible(true);
    }
    LXQtTaskButton::dragEnterEvent(event);
}

/************************************************

 ************************************************/
void LXQtTaskGroup::dragLeaveEvent(QDragLeaveEvent *event)
{
    // if draggind something into the taskgroup or the taskgroups' popup,
    // do not close the popup
    if (!sDraggging)
        setPopupVisible(false);
    LXQtTaskButton::dragLeaveEvent(event);
}

void LXQtTaskGroup::mouseMoveEvent(QMouseEvent* event)
{
    // if dragging the taskgroup, do not show the popup
    if (event->buttons() & Qt::LeftButton)
        setPopupVisible(false, true);
    LXQtTaskButton::mouseMoveEvent(event);
}

/************************************************

 ************************************************/

void LXQtTaskGroup::mouseReleaseEvent(QMouseEvent* event)
{
    // do nothing on left button release if there is a group
    if (event->button() == Qt::LeftButton && visibleButtonsCount() == 1)
        LXQtTaskButton::mouseReleaseEvent(event);
    else
        QToolButton::mouseReleaseEvent(event);
}

/************************************************

 ************************************************/

void LXQtTaskGroup::wheelEvent(QWheelEvent* event)
{
    if (mSingleButton)
    {
        LXQtTaskButton::wheelEvent(event);
        return;
    }
    // if there are multiple buttons, just show the popup
    setPopupVisible(true);
    QToolButton::wheelEvent(event);
}

/************************************************

 ************************************************/
bool LXQtTaskGroup::onWindowChanged(WId window, NET::Properties prop, NET::Properties2 prop2)
{ // returns true if the class is preserved
    bool needsRefreshVisibility{false};
    QVector<LXQtTaskButton *> buttons;
    if (mButtonHash.contains(window))
        buttons.append(mButtonHash.value(window));

    // If group is based on that window properties must be changed also on button group
    if (window == windowId())
        buttons.append(this);

    if (!buttons.isEmpty())
    {
        // if class is changed the window won't belong to our group any more
        if (parentTaskBar2()->isGroupingEnabled() && prop2.testFlag(NET::WM2WindowClass))
        {
            KWindowInfo info(window, NET::Properties(), NET::WM2WindowClass);
            if (QString::fromUtf8(info.windowClassClass()) != mGroupName)
            {
                onWindowRemoved(window);
                return false;
            }
        }
        // window changed virtual desktop
        if (prop.testFlag(NET::WMDesktop) || prop.testFlag(NET::WMGeometry))
        {
            if (parentTaskBar2()->isShowOnlyOneDesktopTasks()
                    || parentTaskBar2()->isShowOnlyCurrentScreenTasks())
            {
                needsRefreshVisibility = true;
            }
        }

        if (prop.testFlag(NET::WMVisibleName) || prop.testFlag(NET::WMName))
            std::for_each(buttons.begin(), buttons.end(), std::mem_fn(&LXQtTaskButton::updateText));

        // XXX: we are setting window icon geometry -> don't need to handle NET::WMIconGeometry
        // Icon of the button can be based on windowClass
        if (prop.testFlag(NET::WMIcon) || prop2.testFlag(NET::WM2WindowClass))
            std::for_each(buttons.begin(), buttons.end(), std::mem_fn(&LXQtTaskButton::updateIcon));

        bool set_urgency = false;
        bool urgency = false;
        if (prop2.testFlag(NET::WM2Urgency))
        {
            set_urgency = true;
            urgency = NETWinInfo(QX11Info::connection(), window, QX11Info::appRootWindow(), NET::Properties{}, NET::WM2Urgency).urgency();
        }
        if (prop.testFlag(NET::WMState))
        {
            KWindowInfo info{window, NET::WMState};
            if (!set_urgency)
                urgency = NETWinInfo(QX11Info::connection(), window, QX11Info::appRootWindow(), NET::Properties{}, NET::WM2Urgency).urgency();
            std::for_each(buttons.begin(), buttons.end(), std::bind(&LXQtTaskButton::setUrgencyHint, std::placeholders::_1, urgency || info.hasState(NET::DemandsAttention)));
            set_urgency = false;
            if (info.hasState(NET::SkipTaskbar))
                onWindowRemoved(window);

            if (parentTaskBar2()->isShowOnlyMinimizedTasks())
            {
                needsRefreshVisibility = true;
            }
        }
        if (set_urgency)
            std::for_each(buttons.begin(), buttons.end(), std::bind(&LXQtTaskButton::setUrgencyHint, std::placeholders::_1, urgency));
    }

    if (needsRefreshVisibility)
        refreshVisibility();

    return true;
}

/************************************************

 ************************************************/
void LXQtTaskGroup::groupPopupShown(LXQtTaskGroup * const sender)
{
    //close all popups (should they be visible because of close delay)
    if (this != sender && isVisible())
            setPopupVisible(false, true/*fast*/);
}