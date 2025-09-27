#include "loginwgt.h"
#include "ui_loginwgt.h"
#include<QtDebug>
LoginWgt::LoginWgt(ShoppingClient *p) :
    ui(new Ui::LoginWgt)
{
    ui->setupUi(this);
    this->setStyleSheet("QLineEdit::hover{background-color:rgb(0, 170, 255);border:0.5px solid rgb(0, 255, 255);border-width:0.5px;}");
    client = p;
    QRegularExpression ps("^[A-Za-z0-9]+$");//由数字和26个英文字母组成的字符串
    QRegularExpression us("^[A-Za-z0-9\u4e00-\u9fa5]+$");
    QRegularExpressionValidator *latitude1 = new QRegularExpressionValidator(us, this);
    QRegularExpressionValidator *latitude2 = new QRegularExpressionValidator(ps, this);
    ui->line_user->setValidator(latitude1);
    ui->line_password->setValidator(latitude2);
    connect(ui->btn_login,SIGNAL(clicked()),this,SLOT(loginUser()));
    connect(ui->btn_register,SIGNAL(clicked()),this,SLOT(registerUser()));
    connect(this,SIGNAL(signal_login(QJsonObject)),client,SLOT(someoneLogin(QJsonObject)));
    connect(ui->btn_togglePwd, &QToolButton::clicked, this, [=]{
        bool isPassword = ui->line_password->echoMode() == QLineEdit::Password;
        ui->line_password->setEchoMode(isPassword ? QLineEdit::Normal : QLineEdit::Password);
        ui->btn_togglePwd->setText(isPassword ? "🙈" : "👁");
    });

    ui->lab_register_link->setText(
        R"(<span style="color:#6a5a84;">还没有账号？</span>
       <a href="register" style="text-decoration:none;color:#8b78c4;">立即注册 &gt;</a>)");
    ui->lab_register_link->setTextFormat(Qt::RichText);
    ui->lab_register_link->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->lab_register_link->setOpenExternalLinks(false);
    ui->lab_register_link->setCursor(Qt::PointingHandCursor);


}

LoginWgt::~LoginWgt()
{
    delete ui;
}

void LoginWgt::paintEvent(QPaintEvent*)
{
    QPainter paint_win(this);
    QPixmap map_win;
    map_win.load(":/images/login/bg_login.jpg");
    paint_win.drawPixmap(0,0,this->width(),this->height(),map_win);
}

void LoginWgt::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_Return){
        ui->btn_login->click();
    }
}

void LoginWgt::loginUser(){
    client->connectTo();
    if(!client->getConnected()){QMessageBox::warning(nullptr,"错误","未连接到网络！");return;}
    QString _username = ui->line_user->text();
    QString _password = ui->line_password->text();
    if(_username != "" && _password != ""){
        QJsonObject obj;
        QString sql = QString("user_name = '%1' AND user_password = '%2'").arg(_username).arg(_password);
        qDebug()<<sql;
        obj.insert("want",QJsonValue("*"));
        obj.insert("isDistinct",QJsonValue("true"));
        obj.insert("restriction",QJsonValue(sql));
        QByteArray data = client->sendCHTTPMsg("10101",obj);//登入
        QString flag = client->parseHead(data);
        if(flag[0] != "1"){client->error(flag[0],flag.mid(1));return;}
        QJsonArray result = client->parseResponse(data);
        if(!result.isEmpty()){
            QJsonObject val = result[0].toObject();
            emit signal_login(val);
            this->close();
        }else{
            QMessageBox::warning(nullptr,"错误","用户不存在或密码错误！");
            return;
        }
    }
    if(_username == ""){
        QMessageBox::warning(nullptr,"错误","用户名为空！");
    }
    if(_password == ""){
        QMessageBox::warning(nullptr,"错误","密码为空！");
    }
    //qDebug()<<_password;
}

void LoginWgt::registerUser(){
    RegisterWgt *p = new RegisterWgt(client);
    p->show();
}


void LoginWgt::on_lab_register_link_linkActivated(const QString &link)
{
    qDebug() << "Register link click:" << link; // 应输出 "register"
    RegisterWgt *dlg = new RegisterWgt(client);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

