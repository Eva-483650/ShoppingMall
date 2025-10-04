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

    // 设置侧边导航
    ui->SideNav->setBarRadious(5);
    ui->SideNav->setItemRadious(5);
    ui->SideNav->setItemStartColor(QColor(191, 65, 249));
    ui->SideNav->setItemEndColor((QColor(187, 83, 217)));
    ui->SideNav->setOrientation(Qt::Vertical);
    ui->SideNav->addItem(tr("商品管理"));
    ui->SideNav->addItem(tr("订单管理"));
    ui->SideNav->addItem(tr("消息管理"));
    ui->SideNav->addItem(tr("资产管理"));
    ui->SideNav->setEnableKeyMove(true);
    ui->SideNav->moveTo(0);
    ui->PageStack->setCurrentIndex(0);

    // 设置页面管理器引用
    ui->ProductP->manager = this;
    ui->OrderP->manager = this;
    ui->ContactP->manager = this;
    ui->ProsessP->manager = this;

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

// 保持原有的其他方法不变
void ShoppingManager::changePage(qintptr index){
    qintptr pagecount = ui->PageStack->count();
    if(index >= pagecount){
        ui->PageStack->setCurrentIndex(0);
    } else {
        ui->PageStack->setCurrentIndex(index);
    }
}

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
    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(data, &jsonError);
    if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        if (doucment.isObject()) {
            QJsonObject object = doucment.object();
            if (object.contains("head")) {
                QString head = object.value("head").toString();
                if(object.contains("result")){
                    QJsonValue result = object.value("result");
                    if(result.isArray()){
                        return result.toArray();
                    }
                    qDebug()<<"result:"<<object.value("result");
                }
                else{QMessageBox::warning(nullptr,tr("错误"),tr("接受数据错误！"));}
            }
        }
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
