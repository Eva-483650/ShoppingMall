#include "loginwgt.h"
#include "ui_loginwgt.h"
#include <QtDebug>

LoginWgt::LoginWgt(ShoppingManager *p) :
    ui(new Ui::LoginWgt)
{
    ui->setupUi(this);
    this->setStyleSheet("QLineEdit::hover{background-color:rgb(0, 170, 255);border:0.5px solid rgb(0, 255, 255);border-width:0.5px;}");
    client = p;

	// 用户名: 中文(汉字) + 英文数字
	QRegularExpression us("^[\\p{Han}A-Za-z0-9]+$");               // 或: "^[A-Za-z0-9\\x{4E00}-\\x{9FA5}]+$"
	// 密码: 仅英文字母数字
	QRegularExpression ps("^[A-Za-z0-9]+$");

    auto *validatorUser = new QRegularExpressionValidator(us, this);
    auto *validatorPass = new QRegularExpressionValidator(ps, this);
    ui->line_user->setValidator(validatorUser);
    ui->line_password->setValidator(validatorPass);

    connect(ui->btn_login, SIGNAL(clicked()), this, SLOT(loginUser()));
    connect(this, SIGNAL(signal_login(QJsonObject)), client, SLOT(someoneLogin(QJsonObject)));
}

LoginWgt::~LoginWgt()
{
    delete ui;
}

void LoginWgt::paintEvent(QPaintEvent * /*event*/)
{
    QPainter paint_win(this);
    QPixmap map_win;
    map_win.load(":/images/5.png");
    paint_win.drawPixmap(0,0,this->width(),this->height(),map_win);
}

void LoginWgt::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_Return){
        ui->btn_login->click();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void LoginWgt::loginUser(){
    client->connectTo();
    if(!client->getConnected()){
        QMessageBox::warning(nullptr,"错误","未连接到网络！");
        return;
    }
    QString _username = ui->line_user->text();
    QString _password = ui->line_password->text();
    if(!_username.isEmpty() && !_password.isEmpty()){
        QJsonObject obj;
        QString sql = QString("manager_name = '%1' AND manager_password = '%2'")
                          .arg(_username, _password);
        obj.insert("want", QJsonValue("*"));
        obj.insert("isDistinct", QJsonValue("true"));
        obj.insert("restriction", QJsonValue(sql));
        QByteArray data = client->sendCHTTPMsg("20101", obj);//登入
        QString flag = client->parseHead(data);
        if(flag.isEmpty() || flag[0] != '1'){
            if(!flag.isEmpty())
                client->error(flag[0], flag.mid(1));
            else
                QMessageBox::warning(nullptr,"错误","返回数据异常！");
            return;
        }
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
    if(_username.isEmpty()){
        QMessageBox::warning(nullptr,"错误","用户名为空！");
    }
    if(_password.isEmpty()){
        QMessageBox::warning(nullptr,"错误","密码为空！");
    }
}


