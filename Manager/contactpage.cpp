#include "contactpage.h"
#include "ui_contactpage.h"

ContactPage::ContactPage(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::ContactPage)
{
	ui->setupUi(this);

	// 初始化UDP Socket
	port = 2227;
	udpsocket = new QUdpSocket(this);
	udpsocket->bind(port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

	// 初始化状态定时器
	statusTimer = new QTimer(this);
	statusTimer->start(30000); // 每30秒更新一次状态

	// 初始化UI状态
	updateMessageStats();
	updateOnlineStatus();

	// 设置消息显示区域
	ui->messageDisplay->setReadOnly(true);
	ui->messageDisplay->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->messageDisplay->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// 设置输入区域
	ui->messageInput->setTabChangesFocus(true);
	ui->messageInput->setAcceptRichText(false);

	// 初始化发送按钮状态
	ui->sendButton->setEnabled(false);

	// 仅连接与自身直接相关的信号，manager 相关延迟到 attachManager()
	setupConnections();

	// 添加欢迎消息
	formatMessage("系统", "消息管理中心已启动，准备接收消息...", false);

	qDebug() << "ContactPage 初始化完成，监听端口:" << port;
}

ContactPage::~ContactPage()
{
	if (udpsocket) {
		udpsocket->close();
	}
	delete ui;
}

void ContactPage::attachManager(ShoppingManager* m)
{
	manager = m;
	if (!manager || managerThemeConnected)
		return;

	// 连接主题变化
	connect(manager, &ShoppingManager::themeChanged,
		this, &ContactPage::onThemeChanged);
	managerThemeConnected = true;

	// 同步一次当前主题（需要 ShoppingManager::isDarkMode()）
	onThemeChanged(manager->m_isDarkMode);
}

void ContactPage::setupConnections()
{
	connect(udpsocket, &QUdpSocket::readyRead, this, &ContactPage::receiveMessage);
	connect(ui->sendButton, &QPushButton::clicked, this, &ContactPage::sendMessage);
	connect(ui->messageInput, &QTextEdit::textChanged, this, &ContactPage::onInputTextChanged);
	connect(statusTimer, &QTimer::timeout, this, &ContactPage::updateOnlineStatus);
}

void ContactPage::keyPressEvent(QKeyEvent* event)
{
	if (event->modifiers() == Qt::ControlModifier &&
		(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
		if (ui->sendButton->isEnabled()) {
			sendMessage();
		}
		return;
	}
	QWidget::keyPressEvent(event);
}

void ContactPage::receiveMessage()
{
	QByteArray res;
	quint16 p = 0;
	QHostAddress targetaddr;

	while (udpsocket->hasPendingDatagrams()) {
		res.resize(udpsocket->pendingDatagramSize());
		udpsocket->readDatagram(res.data(), res.size(), &targetaddr, &p);

		QJsonParseError jsonError;
		QJsonDocument document = QJsonDocument::fromJson(res, &jsonError);

		if (!document.isNull() && (jsonError.error == QJsonParseError::NoError) && document.isObject()) {
			QJsonObject object = document.object();
			appendMsg(object);

			if (object.contains("name") && object.contains("msg")) {
				emit messageReceived(object.value("name").toString(),
					object.value("msg").toString());
			}
		}
	}

	qDebug() << "UDP消息接收完成，来源:" << targetaddr.toString() << ":" << p;
}

void ContactPage::sendMessage()
{
	QString message = ui->messageInput->toPlainText().trimmed();
	if (message.isEmpty()) {
		qDebug() << "消息内容为空，取消发送";
		return;
	}

	if (!manager || !manager->getPerson()) {
		qDebug() << "管理员信息未初始化";
		ui->messageDisplay->append("⚠️ 系统错误：管理员信息未初始化");
		return;
	}

	QJsonObject content;
	content.insert("msg", QJsonValue(message));
	content.insert("name", QJsonValue(manager->getPerson()->name + "(管理员)"));
	content.insert("time", QJsonValue(getCurrentTime()));
	content.insert("type", QJsonValue("admin"));

	QJsonDocument document(content);
	QByteArray data = document.toJson();

	qint64 bytesWritten = udpsocket->writeDatagram(data, data.size(), QHostAddress::LocalHost, 227);

	if (bytesWritten > 0) {
		formatMessage(manager->getPerson()->name + "(管理员)", message, true);
		ui->messageInput->clear();
		emit messageSent(message);
		qDebug() << "消息发送成功，字节数:" << bytesWritten;
	}
	else {
		ui->messageDisplay->append("❌ 消息发送失败");
		qDebug() << "消息发送失败";
	}
}

bool ContactPage::appendMsg(QJsonObject obj)
{
	QString name = obj.value("name").toString();
	QString msg = obj.value("msg").toString();

	if (name.isEmpty() || msg.isEmpty())
		return false;

	bool isFromManager = name.contains("(管理员)");
	if (isFromManager && manager && manager->getPerson()) {
		QString currentManagerName = manager->getPerson()->name + "(管理员)";
		if (name == currentManagerName)
			return true; // 自己的消息已本地显示
	}

	formatMessage(name, msg, false);
	return true;
}

void ContactPage::formatMessage(const QString& name, const QString& msg, bool isFromManager)
{
	QString timestamp = getCurrentTime();
	QString formattedMessage;

	if (isFromManager) {
		formattedMessage = QString(
			"<div style='text-align: right; margin: 8px 0;'>"
			"<span style='color: #666; font-size: 10pt;'>%1</span><br>"
			"<span style='background: #a18cd1; color: white; padding: 8px 12px; "
			"border-radius: 15px 5px 15px 15px; font-size: 11pt; display: inline-block;'>"
			"%2</span><br>"
			"<span style='color: #8b78c4; font-size: 9pt;'>%3</span>"
			"</div>").arg(name, msg.toHtmlEscaped(), timestamp);
	}
	else {
		formattedMessage = QString(
			"<div style='text-align: left; margin: 8px 0;'>"
			"<span style='color: #666; font-size: 10pt;'>%1</span><br>"
			"<span style='background: #f0f0f0; color: #333; padding: 8px 12px; "
			"border-radius: 5px 15px 15px 15px; font-size: 11pt; display: inline-block;'>"
			"%2</span><br>"
			"<span style='color: #999; font-size: 9pt;'>%3</span>"
			"</div>").arg(name, msg.toHtmlEscaped(), timestamp);
	}

	ui->messageDisplay->append(formattedMessage);
	messageCount++;
	updateMessageStats();
	scrollToBottom();
}

void ContactPage::updateMessageStats()
{
	ui->statsLabel->setText(QString("共 %1 条消息").arg(messageCount));
}

void ContactPage::updateOnlineStatus()
{
	QString currentTime = QDateTime::currentDateTime().toString("hh:mm:ss");
	ui->statusLabel->setText(QString("在线 • %1").arg(currentTime));
	qDebug() << "状态已更新:" << currentTime;
}

void ContactPage::scrollToBottom()
{
	if (!ui->messageDisplay) return;
	QScrollBar* scrollBar = ui->messageDisplay->verticalScrollBar();
	if (scrollBar)
		scrollBar->setValue(scrollBar->maximum());
}

void ContactPage::validateInput()
{
	QString text = ui->messageInput->toPlainText().trimmed();
	ui->sendButton->setEnabled(!text.isEmpty() && text.length() <= 500);
}

void ContactPage::onInputTextChanged()
{
	validateInput();
	int length = ui->messageInput->toPlainText().length();
	if (length > 500) {
		QString text = ui->messageInput->toPlainText().left(500);
		ui->messageInput->setPlainText(text);
		QTextCursor cursor = ui->messageInput->textCursor();
		cursor.movePosition(QTextCursor::End);
		ui->messageInput->setTextCursor(cursor);
	}
}

void ContactPage::onThemeChanged(bool isDarkMode)
{
	if (isDarkMode) {
		ui->statusIndicator->setStyleSheet("background: #FF5722; border-radius: 6px;");
	}
	else {
		ui->statusIndicator->setStyleSheet("background: #4CAF50; border-radius: 6px;");
	}
	qDebug() << "消息页面主题已更新:" << (isDarkMode ? "夜间模式" : "日间模式");
}

QString ContactPage::getCurrentTime()
{
	return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}