#include "shoppingclient.h"
#include "ui_shoppingclient.h"
#include <QJsonDocument>
#include <QMessageBox>
#include <QDebug>

QString FLAG_CHARACTER = "1";//客户端标识
QString PIC_PATH = ":/images/";

ShoppingClient::ShoppingClient(QWidget* parent)
	: QWidget(parent),
	ui(new Ui::ShoppingClient)
{
	ui->setupUi(this);
	ui->BottomNav->setBarRadious(5);
	ui->BottomNav->setItemRadious(5);
	ui->BottomNav->addItem(tr("首页"));
	ui->BottomNav->addItem(tr("商城"));
	ui->BottomNav->addItem(tr("消息"));
	ui->BottomNav->addItem(tr("购物车"));
	ui->BottomNav->addItem(tr("订单"));
	ui->BottomNav->addItem(tr("个人中心"));
	ui->BottomNav->setEnableKeyMove(true);
	ui->BottomNav->moveTo(0);
	ui->PageStack->setCurrentIndex(0);

	m_socket = new QTcpSocket(this);   // 绑定 parent 防止泄漏

	ui->HomeP->setClient(this);
	ui->ShopP->client = this;
	ui->CartP->client = this;
	ui->OrderP->client = this;
	ui->ContactP->client = this;

	helpConnect();
	isconnected = false;
	logined_user = nullptr;            // 再次显式，强调初始化
}

ShoppingClient::~ShoppingClient()
{
	delete ui;

}

void ShoppingClient::closeEvent(QCloseEvent* event) {
	qDebug() << "客户端窗口关闭";
	disConnect();
	return QWidget::closeEvent(event);
}

void ShoppingClient::helpConnect() {
	connect(ui->BottomNav, SIGNAL(itemClicked(qintptr, QString)), this, SLOT(changePage(qintptr)));
	connect(ui->ShopP, SIGNAL(signal_addToCart(Product*)), ui->CartP, SLOT(addToCart(Product*)));
	connect(this, SIGNAL(signal_someonelogin(Person*)), ui->MeP, SLOT(getSomeone(Person*)));
	connect(ui->CartP, SIGNAL(signal_updateUserMoney(int)), ui->MeP, SLOT(updateUserMoney(int)));
}

void ShoppingClient::changePage(qintptr index) {
	qintptr pagecount = ui->PageStack->count();
	if (index >= pagecount) {
		ui->PageStack->setCurrentIndex(0);
	}
	else {
		ui->PageStack->setCurrentIndex(index);
	}
	return;
}

void ShoppingClient::setServerIP(QString ip) {
	server_IP = ip;
	return;
}

void ShoppingClient::setServerPort(qintptr port) {
	server_port = port;
	return;
}

bool ShoppingClient::connectTo() {
	setServerIP("127.0.0.1");
	setServerPort(520);//设80会成功=。=、因为自己电脑默认80是开的

	m_socket->connectToHost(server_IP, server_port);
	if (!m_socket->waitForConnected(3000)) {
		qDebug() << "连接服务器失败！";
		QMessageBox::warning(this, tr("Eva的通信"), tr("连接服务端失败!"), QMessageBox::Yes);
		return false;
	}
	else {
		qDebug() << "连接服务器成功！";
		//sendCHTTPMsg("10010",QJsonObject());
		isconnected = true;
	}
	return true;
}

bool ShoppingClient::disConnect() {
	m_socket->disconnectFromHost();
	if (m_socket->state() == QTcpSocket::UnconnectedState) {
		qDebug() << "客户端断开连接成功！";
		isconnected = false;
	}
	return true;
}

QByteArray ShoppingClient::sendCHTTPMsg(QString CHTTP, QJsonObject jsonobj) {
	QJsonObject content;
	content.insert("head", QJsonValue(CHTTP));//设置报文头部
	content.insert("body", QJsonValue(jsonobj));//设置报文内容
	QJsonDocument document;
	document.setObject(content);
	QByteArray arr = document.toJson();
	this->m_socket->write(arr);
	if (m_socket->waitForReadyRead(5000)) {
		QByteArray data = m_socket->readAll();
		qDebug() << "客户端已成功发送消息并收到响应";
		return data;
	}
	else {
		QMessageBox::warning(nullptr, tr("错误！"), tr("sendCHTTP出错！"));
		return QByteArray();
	}
}

QJsonArray ShoppingClient::parseResponse(QByteArray data) {
	QJsonParseError jsonError;
	QJsonDocument doucment = QJsonDocument::fromJson(data, &jsonError);  // 转化为 JSON 文档
	if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {  // 解析未发生错误
		if (doucment.isObject()) {  // JSON 文档为对象
			QJsonObject object = doucment.object();  // 转化为对象
			if (object.contains("head")) {
				QString head = object.value("head").toString();
				if (object.contains("result")) {
					QJsonValue result = object.value("result");
					if (result.isArray()) {
						return result.toArray();
					}
					qDebug() << "result:" << object.value("result");
				}
				else { QMessageBox::warning(nullptr, tr("错误"), tr("接受数据错误！")); }
			}
		}
	}
	return QJsonArray();
}

QString ShoppingClient::parseHead(QByteArray data) {
	QJsonParseError jsonError;
	QJsonDocument doucment = QJsonDocument::fromJson(data, &jsonError);  // 转化为 JSON 文档
	if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {  // 解析未发生错误
		if (doucment.isObject()) {  // JSON 文档为对象
			QJsonObject object = doucment.object();  // 转化为对象
			if (object.contains("head")) {
				QString head = object.value("head").toString();
				if (head[0] == "1") {
					return "1";
				}
				else if (head[0] == "2") {
					QString errmsg = object.value("error").toString();
					return "2" + errmsg;
				}
				else if (head[0] == "3") {
					QString errmsg = object.value("error").toString();
					return "3" + errmsg;
				}
			}
		}
	}
	return "";
}

void ShoppingClient::error(QChar character, QString errmsg) {
	if (character == "2") {
		qDebug() << errmsg;
		QMessageBox::warning(nullptr, "客户端错误", "客户端错误:" + errmsg);
	}
	else if (character == "3") {
		qDebug() << errmsg;
		QMessageBox::warning(nullptr, "服务端错误", "服务端错误:" + errmsg);
	}
}

void ShoppingClient::someoneLogin(QJsonObject obj)
{
	qDebug() << "[someoneLogin] raw =" << obj;

	QJsonObject lower;
	for (auto it = obj.begin(); it != obj.end(); ++it)
		lower.insert(it.key().toLower(), it.value());

	if (!logined_user)
		logined_user = new Person();
	logined_user->updateFromJson(lower);

	if (logined_user->getId() == 0) {
		if (obj.contains("User_id"))
			logined_user->setId(obj.value("User_id").toString().toInt());
		else if (obj.contains("user_id"))
			logined_user->setId(obj.value("user_id").toString().toInt());
	}

	if (logined_user->getId() <= 0) {
		qWarning() << "[someoneLogin] invalid user id";
		QMessageBox::warning(this, "登录异常", "登录响应未包含有效用户ID");
		return;
	}

	qDebug() << "[someoneLogin] success id=" << logined_user->getId()
		<< "name=" << logined_user->name;

	emit signal_someonelogin(logined_user);
	emit userLoggedIn(logined_user->getId());

	this->show();
}


Person* ShoppingClient::getPerson() {
	return this->logined_user;
}

bool ShoppingClient::getConnected() {
	return  this->isconnected;
}