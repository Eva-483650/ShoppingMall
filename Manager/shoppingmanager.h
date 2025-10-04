#ifndef SHOPPINGMANAGER_H
#define SHOPPINGMANAGER_H

#include <QWidget>
#include <QSettings>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include "slidenavigation.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTcpSocket>
#include <QUdpSocket>
#include<QMessageBox>
#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonArray>
#include <QMouseEvent>
#include <QPainter>
#include<QJsonParseError>
#include"person.h"

extern QString FLAG_CHARACTER;

namespace Ui {
class ShoppingManager;
}

class ShoppingManager : public QWidget
{
    Q_OBJECT

public:
    explicit ShoppingManager(QWidget *parent = nullptr);
    ~ShoppingManager();
    QSqlDatabase getDataBase();
    QByteArray sendCHTTPMsg(QString CHTTP,QJsonObject jsonobj);
    QJsonArray parseResponse(QByteArray data);
    QString parseHead(QByteArray data);
    void error(QChar character,QString errmsg);
    Person* getPerson();
    bool getConnected();
    bool connectTo();
    bool disConnect();
	// 主题相关成员
	bool m_isDarkMode;               // 黑夜模式状态
public slots:
    void changePage(qintptr index);
    void someoneLogin(QJsonObject obj);

private slots:
    void toggleDarkMode();           // 切换黑夜模式
    void onThemeChanged(int index);  // 主题风格改变

signals:
    void themeChanged(bool isDarkMode);  // 主题改变信号

private:
    Ui::ShoppingManager *ui;
    void helpConnect();
    QSqlDatabase db;
    QTcpSocket *m_socket;
    QString server_IP;//服务端IP地址
    qintptr server_port;//服务端端口
    Person *logined_user;
    bool isconnected;

    

    void setServerIP(QString ip);
    void setServerPort(qintptr port);
    void closeEvent(QCloseEvent *event);
    bool connectToDataBase(QString SQLkind,QString ip,int port,QString dbname,QString username,QString password);

    // 主题功能方法
    void setupThemeToggle();         // 设置主题切换
    void applyDarkMode(bool enabled); // 应用黑夜模式
    void saveThemeSettings();        // 保存主题设置
    void loadThemeSettings();        // 加载主题设置
    void animateThemeTransition();   // 主题切换动画
};

#endif // SHOPPINGMANAGER_H
