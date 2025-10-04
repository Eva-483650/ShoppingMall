#include "meitem.h"
#include "ui_meitem.h"
#include <QPixmap>
#include <QDebug>

MeItem::MeItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MeItem)
{
    ui->setupUi(this);
}

MeItem::~MeItem()
{
    delete ui;
}

void MeItem::setKey(QString str){
    ui->lab_key->setText(str);
}

void MeItem::setVal(int num){
    ui->lab_val->setNum(num);
}

void MeItem::setVal(QString str){
    ui->lab_val->setText(str);
}

void MeItem::setPic(QString picadd){
    QString add = ":/images/icons/";
    QPixmap pixmap(add + picadd);

    if(pixmap.isNull()){
        pixmap = QPixmap(add + "picerror.png");
        if(pixmap.isNull()) {
            qDebug() << "无法加载图片:" << add + picadd;
            return;
        }
    }

    // 直接设置图片到QLabel，自动缩放到50x50
    ui->label_icon->setPixmap(pixmap);
}
