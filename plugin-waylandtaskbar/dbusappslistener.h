#ifndef __DBUSAPPSLISTENER_H__
#define __DBUSAPPSLISTENER_H__

#include <QDBusConnection>

class DBusAppListener : public QObject 
{
  Q_OBJECT
  public:
    DBusAppListener(QObject *parent);

    QList<uint> getWindowList();
    QString getWindowTitle(uint id);
    QString getWindowAppId(uint id);
    void windowChangeFullscreen(uint id);
    void windowChangeMaximized(uint id);
    void windowChangeMinimized(uint id);
    void windowSetActive(uint id);
    void windowClose(uint id);
    QString getWindowState(uint id);

  public slots:
    void onWindowActivated(uint id);
    void onWindowAppIdChanged(uint id, QString value);
    void onWindowOpened(uint id);
    void onWindowClosed(uint id);
    void onWindowFullscreened(uint id);
    void onWindowMaximized(uint id);
    void onWindowMinimized(uint id);
    void onWindowTitleChanged(uint id, QString title);   

  signals:
    void windowActivated(uint id);
    void windowAppIdChanged(uint id, QString value);
    void windowOpened(uint id);
    void windowClosed(uint id);
    void windowFullscreened(uint id);
    void windowMaximized(uint id);
    void windowMinimized(uint id);
    void windowTitleChanged(uint id, QString title);
  
  private:
    QString callString(const QString method, uint id);
    uint callUint(const QString method, uint id);
};

#endif
