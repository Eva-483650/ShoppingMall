#include "mepage.h"
#include "ui_mepage.h"
#include <QtDebug>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

MePage::MePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MePage),
    someone(nullptr)
{
    ui->setupUi(this);

    // 设置默认头像
    setUserAvatar(":/images/login/avatar.jpg");

}

MePage::~MePage()
{
    delete ui;
}

void MePage::getSomeone(Person* p){
    someone = p;
    qDebug() << "getsomeone!";

    if(someone) {
        ui->lab_name->setText(someone->name);

        // 设置用户头像
        setUserAvatar(":/images/login/avatar.jpg");

        // 添加用户信息项目
        this->addMeItem("性别", someone->gender, "1.png");
        this->addMeItem("余额", QString::number(someone->getmoney()), "2.png");
        this->addMeItem("地址", someone->address, "3.png");
    }
}

void MePage::setUserAvatar(const QString &imagePath) {
    QPixmap originalPixmap(imagePath);

    if(originalPixmap.isNull()) {
        qDebug() << "无法加载头像图片:" << imagePath;
        return;
    }

    // 创建圆形头像
    int size = 80;
    QPixmap roundedPixmap(size, size);
    roundedPixmap.fill(Qt::transparent);

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 创建圆形裁剪路径
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);

    // 绘制缩放后的图片
    QPixmap scaledPixmap = originalPixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    painter.drawPixmap(0, 0, scaledPixmap);

    // 设置到label_avatar
    ui->label_avatar->setPixmap(roundedPixmap);
    ui->label_avatar->setAlignment(Qt::AlignCenter);
}

void MePage::addMeItem(QString key, QString val, QString pic_add){
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    MeItem *nitem = new MeItem();

    nitem->setKey(key);
    nitem->setVal(val);

    if(pic_add != ""){
        nitem->setPic(pic_add);
    }

    // 设置item大小
    item->setSizeHint(QSize(0, 120));

    ui->listWidget->setItemWidget(item, nitem);
    this->infolist.insert(key, nitem); // 将项目加入map中
}

void MePage::updateUserMoney(int num){
    MeItem *item = infolist.value("余额");
    if(item) {
        item->setVal(QString::number(num));
    }
}
