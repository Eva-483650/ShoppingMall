#include "shoppingmanager.h"
#include "ui_shoppingmanager.h"
#include<QDebug>

QString FLAG_CHARACTER = "2";//服务端标识

ShoppingManager::ShoppingManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShoppingManager),
    m_isDarkMode(false)  // 初始化黑夜模式状态
{
    ui->setupUi(this);

    // 设置美化的侧边导航
    setupSideNavigation();

    // 设置页面管理器引用（去掉资产管理页面）
    ui->ProductP->manager = this;
    ui->OrderP->manager = this;
    ui->ContactP->manager = this;
    // 注释掉资产管理页面
    // ui->ProsessP->manager = this;

    // 设置主题切换功能
    setupThemeToggle();

    // 加载保存的主题设置
    loadThemeSettings();

    helpConnect();
    m_socket = new QTcpSocket();
    isconnected = false;
}

ShoppingManager::~ShoppingManager()
{
    saveThemeSettings();  // 析构时保存设置
    delete ui;
}

void ShoppingManager::setupSideNavigation()
{
    // 设置导航栏基础样式
    ui->SideNav->setBarRadious(8);
    ui->SideNav->setItemRadious(8);

    // 设置渐变色彩 - 更现代的紫色主题
    ui->SideNav->setItemStartColor(QColor(139, 120, 196));  // 深紫色
    ui->SideNav->setItemEndColor(QColor(184, 164, 230));    // 浅紫色

    ui->SideNav->setOrientation(Qt::Vertical);

    // 添加带图标的导航项
    ui->SideNav->addItem("📦 " + tr("商品管理"));
    ui->SideNav->addItem("📋 " + tr("订单管理"));
    ui->SideNav->addItem("💬 " + tr("消息管理"));
    // 移除资产管理项

    ui->SideNav->setEnableKeyMove(true);
    ui->SideNav->moveTo(0);
    ui->PageStack->setCurrentIndex(0);

    // 设置导航栏样式
    ui->SideNav->setStyleSheet(
        "QListWidget {"
        "    background: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,"
        "                stop:0 rgba(161, 140, 209, 0.95),"
        "                stop:1 rgba(106, 91, 123, 0.95));"
        "    border: none;"
        "    border-radius: 12px;"
        "    padding: 10px;"
        "    font-family: '微软雅黑';"
        "    font-size: 11pt;"
        "    font-weight: 600;"
        "}"
        "QListWidget::item {"
        "    background: rgba(255, 255, 255, 0.1);"
        "    border: 1px solid rgba(255, 255, 255, 0.2);"
        "    border-radius: 8px;"
        "    padding: 12px 16px;"
        "    margin: 3px 0px;"
        "    color: white;"
        "    min-height: 20px;"
        "}"
        "QListWidget::item:hover {"
        "    background: rgba(255, 255, 255, 0.2);"
        "    border: 1px solid rgba(255, 255, 255, 0.4);"
        "    transform: translateX(3px);"
        "}"
        "QListWidget::item:selected {"
        "    background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,"
        "                stop:0 rgba(255, 255, 255, 0.3),"
        "                stop:1 rgba(255, 255, 255, 0.1));"
        "    border: 1px solid rgba(255, 255, 255, 0.5);"
        "    color: white;"
        "    font-weight: bold;"
        "}"
        "QListWidget::item:selected:hover {"
        "    background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,"
        "                stop:0 rgba(255, 255, 255, 0.4),"
        "                stop:1 rgba(255, 255, 255, 0.2));"
        "}"
        );
}

void ShoppingManager::helpConnect(){
    connect(ui->SideNav,SIGNAL(itemClicked(qintptr,QString)),this,SLOT(changePage(qintptr)));

    // 连接主题切换按钮（确保UI中有这个按钮）
    if (ui->themeToggleBtn) {
        connect(ui->themeToggleBtn, &QPushButton::clicked, this, &ShoppingManager::toggleDarkMode);
    }
}

void ShoppingManager::setupThemeToggle()
{
    // 如果UI中没有主题切换按钮，这里可以动态创建或者忽略
    // 主要逻辑在helpConnect中处理
    qDebug() << "主题切换功能已设置";
}

void ShoppingManager::toggleDarkMode()
{
    m_isDarkMode = !m_isDarkMode;

    qDebug() << "切换到" << (m_isDarkMode ? "黑夜模式" : "日间模式");

    // 播放切换动画
    animateThemeTransition();

    // 延迟应用主题，让动画效果更明显
    QTimer::singleShot(150, this, [this]() {
        applyDarkMode(m_isDarkMode);
        saveThemeSettings();
    });
}

void ShoppingManager::applyDarkMode(bool enabled)
{
    // 设置属性用于CSS选择器
    this->setProperty("darkMode", enabled);
    if (ui->titleWidget) {
        ui->titleWidget->setProperty("darkMode", enabled);
    }
    if (ui->themeToggleBtn) {
        ui->themeToggleBtn->setProperty("darkMode", enabled);

        // 更新按钮文本和图标
        if (enabled) {
            ui->themeToggleBtn->setText("☀️ 日间");
            ui->themeToggleBtn->setToolTip("切换到日间模式");
        } else {
            ui->themeToggleBtn->setText("🌙 夜间");
            ui->themeToggleBtn->setToolTip("切换到夜间模式");
        }
    }

    // 根据主题模式更新导航栏样式
    updateNavigationTheme(enabled);

    // 强制刷新样式
    this->style()->unpolish(this);
    this->style()->polish(this);

    if (ui->titleWidget) {
        ui->titleWidget->style()->unpolish(ui->titleWidget);
        ui->titleWidget->style()->polish(ui->titleWidget);
    }

    if (ui->themeToggleBtn) {
        ui->themeToggleBtn->style()->unpolish(ui->themeToggleBtn);
        ui->themeToggleBtn->style()->polish(ui->themeToggleBtn);
    }

    // 通知子页面更新主题
    emit themeChanged(enabled);

    qDebug() << "主题模式已应用:" << (enabled ? "黑夜模式" : "日间模式");
}

void ShoppingManager::updateNavigationTheme(bool darkMode)
{
    QString navStyle;

    if (darkMode) {
        // 夜间模式 - 深色主题
        navStyle =
            "QListWidget {"
            "    background: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,"
            "                stop:0 rgba(45, 45, 45, 0.95),"
            "                stop:1 rgba(25, 25, 25, 0.95));"
            "    border: 1px solid rgba(100, 100, 100, 0.3);"
            "    border-radius: 12px;"
            "    padding: 10px;"
            "    font-family: '微软雅黑';"
            "    font-size: 11pt;"
            "    font-weight: 600;"
            "}"
            "QListWidget::item {"
            "    background: rgba(70, 70, 70, 0.3);"
            "    border: 1px solid rgba(100, 100, 100, 0.2);"
            "    border-radius: 8px;"
            "    padding: 12px 16px;"
            "    margin: 3px 0px;"
            "    color: #E0E0E0;"
            "    min-height: 20px;"
            "}"
            "QListWidget::item:hover {"
            "    background: rgba(100, 100, 100, 0.4);"
            "    border: 1px solid rgba(150, 150, 150, 0.4);"
            "    color: white;"
            "}"
            "QListWidget::item:selected {"
            "    background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,"
            "                stop:0 rgba(139, 120, 196, 0.6),"
            "                stop:1 rgba(106, 91, 123, 0.6));"
            "    border: 1px solid rgba(184, 164, 230, 0.5);"
            "    color: white;"
            "    font-weight: bold;"
            "}"
            "QListWidget::item:selected:hover {"
            "    background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,"
            "                stop:0 rgba(139, 120, 196, 0.8),"
            "                stop:1 rgba(106, 91, 123, 0.8));"
            "}";
    } else {
        // 日间模式 - 亮色主题
        navStyle =
            "QListWidget {"
            "    background: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1,"
            "                stop:0 rgba(161, 140, 209, 0.95),"
            "                stop:1 rgba(106, 91, 123, 0.95));"
            "    border: none;"
            "    border-radius: 12px;"
            "    padding: 10px;"
            "    font-family: '微软雅黑';"
            "    font-size: 11pt;"
            "    font-weight: 600;"
            "}"
            "QListWidget::item {"
            "    background: rgba(255, 255, 255, 0.15);"
            "    border: 1px solid rgba(255, 255, 255, 0.25);"
            "    border-radius: 8px;"
            "    padding: 12px 16px;"
            "    margin: 3px 0px;"
            "    color: white;"
            "    min-height: 20px;"
            "}"
            "QListWidget::item:hover {"
            "    background: rgba(255, 255, 255, 0.25);"
            "    border: 1px solid rgba(255, 255, 255, 0.4);"
            "    transform: translateX(2px);"
            "}"
            "QListWidget::item:selected {"
            "    background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,"
            "                stop:0 rgba(255, 255, 255, 0.35),"
            "                stop:1 rgba(255, 255, 255, 0.15));"
            "    border: 1px solid rgba(255, 255, 255, 0.6);"
            "    color: white;"
            "    font-weight: bold;"
            "}"
            "QListWidget::item:selected:hover {"
            "    background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,"
            "                stop:0 rgba(255, 255, 255, 0.45),"
            "                stop:1 rgba(255, 255, 255, 0.25));"
            "}";
    }

    ui->SideNav->setStyleSheet(navStyle);
}

void ShoppingManager::animateThemeTransition()
{
    // 创建淡入淡出动画效果
    auto *effect = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(effect);

    auto *animation = new QPropertyAnimation(effect, "opacity", this);
    animation->setDuration(300);
    animation->setStartValue(1.0);
    animation->setKeyValueAt(0.5, 0.7);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    connect(animation, &QPropertyAnimation::finished, this, [this]() {
        this->setGraphicsEffect(nullptr);
    });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void ShoppingManager::saveThemeSettings()
{
    QSettings settings("EvaShoppingMall", "ManagerSettings");
    settings.setValue("darkMode", m_isDarkMode);
    qDebug() << "主题设置已保存:" << (m_isDarkMode ? "黑夜模式" : "日间模式");
}

void ShoppingManager::loadThemeSettings()
{
    QSettings settings("EvaShoppingMall", "ManagerSettings");
    m_isDarkMode = settings.value("darkMode", false).toBool();
    QString themeStyle = settings.value("themeStyle", "Ubuntu").toString();

    // 应用保存的主题
    applyDarkMode(m_isDarkMode);

    qDebug() << "主题设置已加载:" << (m_isDarkMode ? "黑夜模式" : "日间模式");
}

void ShoppingManager::onThemeChanged(int index)
{
    ///QString themeName = ui->comboBox->itemText(index);
    //qDebug() << "主题风格已更改为:" << themeName;

    // 这里可以添加具体的主题风格切换逻辑
    // 例如加载不同的CSS文件或调整特定样式

    saveThemeSettings();
}

// 修改页面切换逻辑，适应去掉资产管理页面后的页面数量
void ShoppingManager::changePage(qintptr index){
    // 现在只有3个页面：商品管理(0)、订单管理(1)、消息管理(2)
    qintptr pagecount = 3; // 减少页面数量

    if(index >= pagecount){
        ui->PageStack->setCurrentIndex(0);
    } else {
        ui->PageStack->setCurrentIndex(index);
    }

    qDebug() << "切换到页面:" << index;
}

// 保持原有的其他方法不变
bool ShoppingManager::connectToDataBase(QString SQLkind,QString ip,int port,QString dbname,QString username,QString password){
    if(QSqlDatabase::contains("qt_sql_default_connection")){
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else{
        db = QSqlDatabase::addDatabase(SQLkind);
    }
    db.setHostName(ip);
    db.setPort(port);
    db.setDatabaseName(dbname);
    db.setUserName(username);
    db.setPassword(password);
    if(db.open()){
        qDebug()<<"连接数据库成功！";
        return true;
    } else {
        qDebug()<<"连接数据库失败！";
    }
    return false;
}

QSqlDatabase ShoppingManager::getDataBase(){
    return this->db;
}

void ShoppingManager::closeEvent(QCloseEvent *event){
    qDebug()<<"业务端窗口关闭";
    saveThemeSettings(); // 关闭窗口时保存设置
    disConnect();
    return QWidget::closeEvent(event);
}

void ShoppingManager::setServerIP(QString ip){
    server_IP = ip;
}

void ShoppingManager::setServerPort(qintptr port){
    server_port = port;
}

bool ShoppingManager::connectTo(){
    setServerIP("127.0.0.1");
    setServerPort(8080);
    m_socket->connectToHost(server_IP,server_port);
    if(!m_socket->waitForConnected(3000)){
        qDebug()<<"连接服务器失败！";
        QMessageBox::warning(this,tr("潮海波的通信"),tr("连接服务端失败!"),QMessageBox::Yes);
        return false;
    }
    else {
        qDebug()<<"连接服务器成功！";
        this->connectToDataBase("QMYSQL","127.0.0.1",3306,"ShoppingMall","root","chb20020309");
        isconnected = true;
    }
    return true;
}

bool ShoppingManager::disConnect(){
    m_socket->disconnectFromHost();
    if(m_socket->state() == QTcpSocket::UnconnectedState){
        isconnected = false;
        qDebug()<<"业务端断开连接成功！";
    }
    return true;
}

QByteArray ShoppingManager::sendCHTTPMsg(QString CHTTP, QJsonObject jsonobj){
    QJsonObject content;
    content.insert("head",QJsonValue(CHTTP));
    content.insert("body",QJsonValue(jsonobj));
    QJsonDocument document;
    document.setObject(content);
    QByteArray arr = document.toJson();
    this->m_socket->write(arr);
    if(m_socket->waitForReadyRead(5000)){
        QByteArray data = m_socket->readAll();
        qDebug()<<"客户端已成功发送消息并收到响应";
        return data;
    }else{
        QMessageBox::warning(nullptr,tr("错误！"),tr("sendCHTTP出错！"));
        return QByteArray();
    }
}

QJsonArray ShoppingManager::parseResponse(QByteArray data){
    qDebug() << "收到服务器原始数据:" << QString(data);

    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(data, &jsonError);

    if (!document.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        if (document.isObject()) {
            QJsonObject object = document.object();
            qDebug() << "解析后的JSON对象:" << object;

            if (object.contains("head")) {
                QString head = object.value("head").toString();
                qDebug() << "响应头:" << head;

                if(object.contains("result")){
                    QJsonValue result = object.value("result");
                    qDebug() << "result字段类型:" << result.type();
                    qDebug() << "result内容:" << result;

                    if(result.isArray()){
                        QJsonArray resultArray = result.toArray();
                        qDebug() << "返回数组长度:" << resultArray.size();

                        // 打印数组中的每个元素
                        for(int i = 0; i < resultArray.size(); ++i) {
                            qDebug() << "数组元素" << i << ":" << resultArray[i].toObject();
                        }

                        return resultArray;
                    } else {
                        qDebug() << "result不是数组类型，实际类型:" << result.type();
                    }
                    qDebug()<<"result:"<<object.value("result");
                }
                else{
                    qDebug() << "响应中不包含result字段";
                    qDebug() << "响应的所有字段:" << object.keys();
                    QMessageBox::warning(nullptr,tr("错误"),tr("接受数据错误！"));
                }
            } else {
                qDebug() << "响应中不包含head字段";
            }
        } else {
            qDebug() << "响应不是JSON对象";
        }
    } else {
        qDebug() << "JSON解析错误:" << jsonError.errorString();
    }
    return QJsonArray();
}

QString ShoppingManager::parseHead(QByteArray data){
    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(data, &jsonError);
    if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        if (doucment.isObject()) {
            QJsonObject object = doucment.object();
            if (object.contains("head")) {
                QString head = object.value("head").toString();
                if(head[0] == "1"){
                    return "1";
                }
                else if(head[0] == "2"){
                    QString errmsg = object.value("error").toString();
                    return "2"+errmsg;
                }
                else if(head[0] == "3"){
                    QString errmsg = object.value("error").toString();
                    return "3"+errmsg;
                }
            }
        }
    }
    return "";
}

void ShoppingManager::error(QChar character, QString errmsg){
    if(character == '2'){
        qDebug()<<errmsg;
        QMessageBox::warning(nullptr,"客户端错误","客户端错误:"+errmsg);
    }
    else if(character == '3'){
        qDebug()<<errmsg;
        QMessageBox::warning(nullptr,"服务端错误","服务端错误:"+errmsg);
    }
}

void ShoppingManager::someoneLogin(QJsonObject obj){
    this->show();
    this->logined_user = new Person(obj);
    qDebug()<<obj<<"Login!";
}

Person* ShoppingManager::getPerson(){
    return this->logined_user;
}

bool ShoppingManager::getConnected(){
    return this->isconnected;
}
