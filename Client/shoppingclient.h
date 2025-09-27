#ifndef SHOPPINGCLIENT_H
#define SHOPPINGCLIENT_H

#include <QWidget>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>
#include "person.h"
#include "slidenavigation.h"

extern QString FLAG_CHARACTER;
extern QString PIC_PATH;

namespace Ui { class ShoppingClient; }

class ShoppingClient : public QWidget
{
	Q_OBJECT
public:
	explicit ShoppingClient(QWidget* parent = nullptr);
	~ShoppingClient();

	void setServerIP(QString ip);
	void setServerPort(qintptr port);

	bool connectTo();
	bool disConnect();

	QByteArray sendCHTTPMsg(QString CHTTP, QJsonObject jsonobj);
	QJsonArray parseResponse(QByteArray data);
	QString parseHead(QByteArray data);

	void error(QChar character, QString errmsg);
	bool getConnected();

	Person* getPerson();

protected:
	void closeEvent(QCloseEvent* event) override;

signals:
	void signal_someonelogin(Person*);
	void userLoggedIn(int userId);     // 新增：广播登录完成

private slots:
	void changePage(qintptr index);
	void someoneLogin(QJsonObject obj);

private:
	void helpConnect();

private:
	Ui::ShoppingClient* ui = nullptr;
	QTcpSocket* m_socket = nullptr;

	QString server_IP;
	qintptr server_port = 0;

	Person* logined_user = nullptr;    // 初始化
	bool isconnected = false;
};

#endif // SHOPPINGCLIENT_H