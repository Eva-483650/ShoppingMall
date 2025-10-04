#include "loginwgt.h"
#include "ui_loginwgt.h"
#include <QtDebug>
#include <QMessageBox>
#include <QPainter>
#include <QKeyEvent>

LoginWgt::LoginWgt(ShoppingManager *p) :
    ui(new Ui::LoginWgt)
{
    ui->setupUi(this);

    // 设置基础样式（如果需要额外的样式覆盖）
    this->setStyleSheet("QLineEdit::hover{background-color:rgb(0, 170, 255);border:0.5px solid rgb(0, 255, 255);border-width:0.5px;}");

    client = p;

    // 设置验证器
    // 用户名: 中文(汉字) + 英文数字
    QRegularExpression us("^[\\p{Han}A-Za-z0-9]+$");
    // 密码: 仅英文字母数字
    QRegularExpression ps("^[A-Za-z0-9]+$");

    auto *validatorUser = new QRegularExpressionValidator(us, this);
    auto *validatorPass = new QRegularExpressionValidator(ps, this);
    ui->line_user->setValidator(validatorUser);
    ui->line_password->setValidator(validatorPass);

    // 管理员登录界面专用设置
    setupManagerUI();

    // 连接信号槽
    connect(ui->btn_login, SIGNAL(clicked()), this, SLOT(loginUser()));
    connect(this, SIGNAL(signal_login(QJsonObject)), client, SLOT(someoneLogin(QJsonObject)));

    // 密码显示/隐藏按钮
    connect(ui->btn_togglePwd, &QToolButton::clicked, this, [=]{
        bool isPassword = ui->line_password->echoMode() == QLineEdit::Password;
        ui->line_password->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
        ui->btn_togglePwd->setText(isPassword ? "🙈" : "👁");
    });
}

LoginWgt::~LoginWgt()
{
    delete ui;
}

void LoginWgt::setupManagerUI()
{
    // 修改标题为管理员登录
    if (ui->lab_title) {
        ui->lab_title->setText("管理员登录");
    }

    // 修改输入框提示文本
    ui->line_user->setPlaceholderText("管理员账号");
    ui->line_password->setPlaceholderText("请输入管理员密码");

    // 隐藏注册相关按钮和链接（管理员不需要注册功能）
    if (ui->btn_register) {
        ui->btn_register->hide();
    }

    if (ui->lab_register_link) {
        ui->lab_register_link->hide();
    }

    // 修改记住我选项的文本
    if (ui->chk_remember) {
        ui->chk_remember->setText("记住管理员账号");
    }

    // 可以添加管理员专用的样式标识
    this->setObjectName("ManagerLoginWgt");

    // 设置窗口标题
    this->setWindowTitle("管理员登录 - Eva的商城管理系统");

    // 调整登录按钮文本
    ui->btn_login->setText("管理员登录");
}

void LoginWgt::paintEvent(QPaintEvent * /*event*/)
{
    QPainter paint_win(this);
    QPixmap map_win;
    // 可以使用不同的背景图片来区分管理员和用户界面
    map_win.load(":/images/bg_admin_login.jpg"); // 如果有专用背景图
    if (map_win.isNull()) {
        map_win.load(":/images/bg_login.jpg"); // 备用背景图
    }
    paint_win.drawPixmap(0, 0, this->width(), this->height(), map_win);
}

void LoginWgt::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        ui->btn_login->click();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void LoginWgt::loginUser()
{
    // 连接到服务器
    if (!client->connectTo()) {
        QMessageBox::warning(this, "连接错误", "无法连接到服务器！");
        return;
    }

    if (!client->getConnected()) {
        QMessageBox::warning(this, "错误", "未连接到网络！");
        return;
    }

    QString _username = ui->line_user->text().trimmed();
    QString _password = ui->line_password->text();

    // 输入验证
    if (_username.isEmpty()) {
        QMessageBox::warning(this, "错误", "管理员账号不能为空！");
        ui->line_user->setFocus();
        return;
    }

    if (_password.isEmpty()) {
        QMessageBox::warning(this, "错误", "密码不能为空！");
        ui->line_password->setFocus();
        return;
    }

    // 构建登录请求
    QJsonObject obj;
    QString sql = QString("manager_name = '%1' AND manager_password = '%2'")
                      .arg(_username.replace("'", "''"))  // 防止SQL注入
                      .arg(_password.replace("'", "''"));

    obj.insert("want", QJsonValue("*"));
    obj.insert("isDistinct", QJsonValue("true"));
    obj.insert("restriction", QJsonValue(sql));

    qDebug() << "管理员登录请求SQL:" << sql;

    // 发送登录请求 (管理员登录协议码: 20101)
    QByteArray data = client->sendCHTTPMsg("20101", obj);
    QString flag = client->parseHead(data);

    if (flag.isEmpty()) {
        QMessageBox::warning(this, "错误", "服务器响应异常！");
        return;
    }

    if (flag[0] != '1') {
        if (flag.length() > 1) {
            client->error(flag[0], flag.mid(1));
        } else {
            QMessageBox::warning(this, "登录失败", "登录验证失败！");
        }
        return;
    }

    // 解析登录结果
    QJsonArray result = client->parseResponse(data);
    if (!result.isEmpty()) {
        QJsonObject managerData = result[0].toObject();

        qDebug() << "管理员登录成功:" << managerData;

        // 记住管理员账号功能
        if (ui->chk_remember && ui->chk_remember->isChecked()) {
            // 这里可以保存管理员账号到配置文件
            saveManagerCredentials(_username);
        }

        // 发射登录成功信号
        emit signal_login(managerData);

        // 显示登录成功消息
        // QMessageBox::information(this, "登录成功",
        //                          QString("欢迎回来，管理员 %1！")
        //                              .arg(managerData.value("manager_name").toString()));

        this->close();
    } else {
        QMessageBox::warning(this, "登录失败", "管理员账号或密码错误！\n请检查您的凭据。");
        ui->line_password->clear();
        ui->line_user->setFocus();
    }
}

void LoginWgt::saveManagerCredentials(const QString &username)
{
    // 保存管理员账号到配置文件的实现
    // 注意：不要保存密码，只保存用户名
    QSettings settings("EvaShoppingMall", "AdminLogin");
    settings.setValue("lastManagerName", username);
    settings.setValue("rememberManager", true);
}

void LoginWgt::loadSavedCredentials()
{
    // 加载保存的管理员账号
    QSettings settings("EvaShoppingMall", "AdminLogin");
    if (settings.value("rememberManager", false).toBool()) {
        QString savedName = settings.value("lastManagerName", "").toString();
        if (!savedName.isEmpty()) {
            ui->line_user->setText(savedName);
            ui->chk_remember->setChecked(true);
            ui->line_password->setFocus(); // 焦点设置到密码框
        }
    }
}

void LoginWgt::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 窗口显示时加载保存的凭据
    loadSavedCredentials();
}
