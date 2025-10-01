#include "orderitem.h"
#include "ui_orderitem.h"

// 修改OrderItem的构造函数，优化布局
OrderItem::OrderItem(QJsonObject obj) :
    ui(new Ui::OrderItem)
{
    ui->setupUi(this);

    // 优化OrderItem的布局
    optimizeItemLayout();

    if(obj.contains("pro_name")){
        setProname(obj.value("pro_name").toString());
    }
    if(obj.contains("pro_pictureaddress")){
        setPicture(obj.value("pro_pictureaddress").toString());
    }
    if(obj.contains("orderitem_num")){
        setPronum(obj.value("orderitem_num").toString());
    }
    if(obj.contains("orderitem_pro_price")){
        setProprice(obj.value("orderitem_pro_price").toString());
    }
    if(obj.contains("orderitem_tolprice")){
        setProtolprice(obj.value("orderitem_tolprice").toString());
    }
}

// 添加OrderItem布局优化方法
void OrderItem::optimizeItemLayout() {
    // 设置整体最小高度
    this->setMinimumHeight(100);

    // 调整主布局的边距和间距
    if (ui->horizontalLayout_5) {
        ui->horizontalLayout_5->setSpacing(10);
        ui->horizontalLayout_5->setContentsMargins(10, 5, 10, 5);
    }

    // 设置商品名称标签的最小宽度和换行
    if (ui->lab_name) {
        ui->lab_name->setMinimumWidth(150);
        ui->lab_name->setWordWrap(true);
        ui->lab_name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }

    // 优化价格、数量、总价的显示容器宽度
    if (ui->widget_2) {  // 单价容器
        ui->widget_2->setMinimumWidth(120);
        ui->widget_2->setMaximumWidth(150);
    }

    if (ui->widget_3) {  // 数量容器
        ui->widget_3->setMinimumWidth(100);
        ui->widget_3->setMaximumWidth(120);
    }

    if (ui->widget_4) {  // 总价容器
        ui->widget_4->setMinimumWidth(120);
        ui->widget_4->setMaximumWidth(150);
    }
}

OrderItem::~OrderItem()
{
    delete ui;
}


void OrderItem::setPicture(QString url){
    QString add = ":/images/products/";
    QPixmap pixmap(add+url);
    if(pixmap.isNull()){
        add += "picerror.jpg";
    }
    else{
        add += url;
    }
   QString str = "QWidget{border-image:url("+add+");}";
   ui->widget->setStyleSheet(str);
}

void OrderItem::setProname(QString name){
    ui->lab_name->setText(name);
}

void OrderItem::setPronum(QString num){
    ui->lab_num->setText(num);
}

void OrderItem::setProprice(QString price){
    ui->lab_price->setText(price);
}

void OrderItem::setProtolprice(QString tolprice){
    ui->lab_tolprice->setText(tolprice);
}
