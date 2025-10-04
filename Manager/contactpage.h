#ifndef CONTACTPAGE_H
#define CONTACTPAGE_H

#include <QWidget>
#include <QUdpSocket>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QTimer>
#include <QDateTime>
#include <QTextCursor>
#include <QScrollBar>
#include <QKeyEvent>
#include <QDebug>
#include "shoppingmanager.h"

namespace Ui {
	class ContactPage;
}

class ContactPage : public QWidget
{
	Q_OBJECT

public:
	explicit ContactPage(QWidget* parent = nullptr);
	~ContactPage();

	// 供外部（ShoppingManager）注入 manager，并建立与主题信号的连接
	void attachManager(ShoppingManager* m);

	ShoppingManager* manager = nullptr;      // 显式初始化，避免未定义值

protected:
	void keyPressEvent(QKeyEvent* event) override;

private:
	Ui::ContactPage* ui;
	QUdpSocket* udpsocket = nullptr;
	qintptr port = 0;
	int messageCount = 0;
	QTimer* statusTimer = nullptr;

	bool managerThemeConnected = false;      // 防止重复连接

	// 私有方法
	bool appendMsg(QJsonObject obj);
	void setupConnections();             // 设置信号槽连接（不包含 manager 相关）
	void updateMessageStats();           // 更新消息统计
	void updateOnlineStatus();           // 更新在线状态
	void formatMessage(const QString& name, const QString& msg, bool isFromManager = false);
	void scrollToBottom();              // 滚动到底部
	void validateInput();               // 验证输入
	QString getCurrentTime();           // 获取当前时间

private slots:
	void receiveMessage();
	void sendMessage();
	void onInputTextChanged();          // 输入文本改变
	void onThemeChanged(bool isDarkMode); // 主题改变响应

signals:
	void messageSent(QString message);               // 消息发送信号
	void messageReceived(QString from, QString message); // 消息接收信号
};

#endif // CONTACTPAGE_H