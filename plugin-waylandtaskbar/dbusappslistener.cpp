#include "dbusappslistener.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDebug>
#include <QThread>

static QString mDBusInterfaceName() {
  QByteArray waylandInterface = QByteArray("lxqt.WindowsList.") + qgetenv("WAYLAND_DISPLAY");
  return QString::fromLocal8Bit(waylandInterface.replace("-",""));
}

static QString mDBusInterfacePath() {
  return QStringLiteral("/WindowsList");
}

DBusAppListener::DBusAppListener(QObject *parent) : QObject(parent) 
{
  qDebug() << "[DBusAppListener::DBusAppListener()] Conecting to DBus";
  auto mConnection = QDBusConnection::sessionBus();
  for(int n = 0; n < 3 && !mConnection.isConnected(); n++) {
    mConnection = QDBusConnection::sessionBus();
    QThread::sleep(5);
  } 
  if(!mConnection.isConnected()) {
    qWarning("Cannot connect to the D-Bus session bus.\n"
        "To start it, run:\n"
        "\teval `dbus-launch --auto-syntax`\n");
  } else {
    qDebug() << "[DBusAppListener::DBusAppListener] " << mDBusInterfaceName();
    const int ATTEMPTS = 5;
    for (int i = 0; i < ATTEMPTS; i++) {
      QDBusInterface iface(mDBusInterfaceName(), mDBusInterfacePath(), mDBusInterfaceName(), mConnection);
      if(iface.isValid()) {
        iface.connection().connect(mDBusInterfaceName(), mDBusInterfacePath(), mDBusInterfaceName(), 
            QStringLiteral("WindowOpened"), this, SLOT(onWindowOpened(uint)));
        iface.connection().connect(mDBusInterfaceName(), mDBusInterfacePath(), mDBusInterfaceName(), 
            QStringLiteral("WindowActivated"), this, SLOT(onWindowActivated(uint)));
        iface.connection().connect(mDBusInterfaceName(), mDBusInterfacePath(), mDBusInterfaceName(), 
            QStringLiteral("WindowAppIdChanged"), this, SLOT(onWindowAppIdChanged(uint,QString)));
        iface.connection().connect(mDBusInterfaceName(), mDBusInterfacePath(), mDBusInterfaceName(), 
            QStringLiteral("WindowClosed"), this, SLOT(onWindowClosed(uint)));
        iface.connection().connect(mDBusInterfaceName(), mDBusInterfacePath(), mDBusInterfaceName(), 
            QStringLiteral("WindowFullscreened"), this, SLOT(onWindowFullscreened(uint)));
        iface.connection().connect(mDBusInterfaceName(), mDBusInterfacePath(), mDBusInterfaceName(), 
            QStringLiteral("WindowMaximized"), this, SLOT(onWindowMaximized(uint)));
        iface.connection().connect(mDBusInterfaceName(), mDBusInterfacePath(), mDBusInterfaceName(), 
            QStringLiteral("WindowMinimized"), this, SLOT(onWindowMinimized(uint)));
        iface.connection().connect(mDBusInterfaceName(), mDBusInterfacePath(), mDBusInterfaceName(), 
            QStringLiteral("WindowTitleChanged"), this, SLOT(onWindowTitleChanged(uint,QString)));
        i = ATTEMPTS;
      } else
        QThread::sleep(5);
    }
  }
}

uint DBusAppListener::callUint(const QString method, uint id)
{
  auto mConnection = QDBusConnection::sessionBus();
  if(!mConnection.isConnected()) {
    qWarning("Cannot connect to the D-Bus session bus.\n"
        "To start it, run:\n"
        "\teval `dbus-launch --auto-syntax`\n");
  } else {
    QDBusMessage m = QDBusMessage::createMethodCall(
        mDBusInterfaceName(), 
        mDBusInterfacePath(), 
        mDBusInterfaceName(), 
        method);
    m << id;
    qDebug() << "[DBusAppListener::callUint] " << method << " id " << id;
    QDBusMessage response = mConnection.call(m);
    qDebug() << "[DBusAppListener::callUint] " << method << " id " << id << " response";
    for(QVariant arg : response.arguments()) {
      return arg.toInt();
    }
  }
  return 0;
}

QString DBusAppListener::callString(const QString method, uint id)
{
  auto mConnection = QDBusConnection::sessionBus();
  if(!mConnection.isConnected()) {
    qWarning("Cannot connect to the D-Bus session bus.\n"
        "To start it, run:\n"
        "\teval `dbus-launch --auto-syntax`\n");
  } else {
    QDBusMessage m = QDBusMessage::createMethodCall(
        mDBusInterfaceName(), 
        mDBusInterfacePath(), 
        mDBusInterfaceName(), 
        method);
    m << id;
    QDBusMessage response = mConnection.call(m);
    for(QVariant arg : response.arguments()) {
      return arg.toString();
    }
  }
  return QString();
}

QString DBusAppListener::getWindowTitle(uint id)
{
  return callString(QStringLiteral("WindowTitle"), id);
}


QString DBusAppListener::getWindowAppId(uint id)
{
  return callString(QStringLiteral("WindowAppId"), id);
}

QString DBusAppListener::getWindowState(uint id)
{
  return callString(QStringLiteral("WindowState"), id);
}

void DBusAppListener::windowChangeFullscreen(uint id)
{
  callUint(QStringLiteral("WindowChangeFullscreen"), id);
}

void DBusAppListener::windowChangeMaximized(uint id)
{
  callUint(QStringLiteral("WindowChangeMaximized"), id);
}


void DBusAppListener::windowChangeMinimized(uint id)
{
  callUint(QStringLiteral("WindowChangeMinimized"), id);
}

void DBusAppListener::windowSetActive(uint id)
{
  callUint(QStringLiteral("WindowSetActive"), id);
}

void DBusAppListener::windowClose(uint id)
{
  callUint(QStringLiteral("WindowClose"), id);
}

QList<uint> DBusAppListener::getWindowList()
{
  QList<uint> ids;
  auto mConnection = QDBusConnection::sessionBus();
  if(!mConnection.isConnected()) {
    qWarning("Cannot connect to the D-Bus session bus.\n"
        "To start it, run:\n"
        "\teval `dbus-launch --auto-syntax`\n");
  } else {
    QDBusMessage m = QDBusMessage::createMethodCall(
        mDBusInterfaceName(), 
        mDBusInterfacePath(), 
        mDBusInterfaceName(), 
        QStringLiteral("WindowList"));
    QDBusMessage response = mConnection.call(m);
    for(QVariant arg : response.arguments()) {
      qDebug() << "[DBusAppListener::getWindowList] found " << arg; 
      QDBusArgument *darg = static_cast<QDBusArgument*>((void *)arg.data());
      darg->beginArray();
      QList<QVariant> args = arg.toList();
      while(!darg->atEnd()) {
        uint id;
        *darg >> id;
        ids << id;
        qDebug() << "[DBusAppListener::getWindowList] found " << id; 
      }
      darg->endArray();
    }
    return ids;
  }
  return ids;
}


void DBusAppListener::onWindowActivated(uint id)
{
  emit windowActivated(id);
}

void DBusAppListener::onWindowAppIdChanged(uint id, QString value)
{
  emit windowAppIdChanged(id, value);
}

void DBusAppListener::onWindowOpened(uint id)
{
  qDebug() << "[DBusAppListener::onWindowOpened]"; 
  emit windowOpened(id);
}

void DBusAppListener::onWindowClosed(uint id)
{
  emit windowClosed(id);
}

void DBusAppListener::onWindowFullscreened(uint id)
{
  emit windowFullscreened(id);
}

void DBusAppListener::onWindowMaximized(uint id)
{
  emit windowMaximized(id);
}

void DBusAppListener::onWindowMinimized(uint id)
{
  emit windowMinimized(id);
}

void DBusAppListener::onWindowTitleChanged(uint id, QString title)
{
  emit windowTitleChanged(id, title);
}
