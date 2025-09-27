#include "shoppage.h"
#include "ui_shoppage.h"

#include <QRegularExpression>
#include <QListWidget>
#include <QStackedWidget>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QCursor>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

/*
 * 统一命名：全部使用 Product_xxx JSON 字段：
 * Product_id / Product_name / Product_price / Product_amount / Product_sales
 * Product_classification / Product_about / Product_istimelimited / Product_pictureaddress
 */

namespace {
    constexpr const char* FIELD_ID = "Product_id";
    constexpr const char* FIELD_NAME = "Product_name";
    constexpr const char* FIELD_PRICE = "Product_price";
    constexpr const char* FIELD_AMOUNT = "Product_amount";
    constexpr const char* FIELD_SALES = "Product_sales";
    constexpr const char* FIELD_CLASSIFICATION = "Product_classification";
    constexpr const char* FIELD_ABOUT = "Product_about";
} // namespace

/*
 * ShopPage 负责：
 * 1. 初始化商品分类（首次 showEvent）
 * 2. 分类 → QStackedWidget 多页
 * 3. 双击 / 搜索弹出 ProductPage
 * 4. 右键刷新当前分类
 */

ShopPage::ShopPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ShopPage)
{
    ui->setupUi(this);
    isCached = false;

    m_stackedwidget = new QStackedWidget();
    ui->horizontalLayout->addWidget(m_stackedwidget);

    connect(ui->list_classification, &QListWidget::currentRowChanged,
        this, &ShopPage::updateOneClass);
    connect(this, &ShopPage::signal_refresh,
        this, &ShopPage::updateOneClass);
    connect(ui->btn_search, &QPushButton::clicked,
        this, &ShopPage::searchProduct);
}

ShopPage::~ShopPage()
{
    delete ui;
}

void ShopPage::showEvent(QShowEvent* event)
{
    if (!isCached) {
        initClassification();
        isCached = true;
    }
    QWidget::showEvent(event);
}

/*
 * 拉取所有商品分类（distinct Product_classification）
 */
void ShopPage::initClassification()
{
    if (!client) {
        QMessageBox::warning(nullptr, tr("错误"), tr("客户端有误！"));
        return;
    }

    const QString FLAG_INSKIND = "02";
    const QString FLAG_INS = "04";

    QJsonObject obj;
    obj.insert("want", QJsonValue(QString(FIELD_CLASSIFICATION)));
    obj.insert("isDistinct", QJsonValue(QString("true")));

    QByteArray data = client->sendCHTTPMsg(FLAG_CHARACTER + FLAG_INSKIND + FLAG_INS, obj);
    QString flag = client->parseHead(data);
    if (flag[0] != '1') {
        client->error(flag[0], flag.mid(1));
        return;
    }

    QJsonArray result = client->parseResponse(data);
    if (!result.isEmpty()) {
        ui->list_classification->clear();
        for (int i = 0; i < m_stackedwidget->count(); ++i) {
            QWidget* w = m_stackedwidget->widget(i);
            m_stackedwidget->removeWidget(w);
            w->deleteLater();
        }

        for (int i = 0; i < result.size(); ++i) {
            const QString str = result.at(i).toObject().value(FIELD_CLASSIFICATION).toString();
            ui->list_classification->addItem(new QListWidgetItem(str));
            m_stackedwidget->addWidget(new QListWidget());
        }

        stack_isCached.clear();
        stack_isCached.resize(result.size());
        stack_isCached.fill(false);

        ui->list_classification->setCurrentRow(0);
    }
}

/*
 * 加载指定分类（未缓存则请求服务器）
 */
void ShopPage::updateOneClass(int currow)
{
    if (currow < 0 || currow >= stack_isCached.size())
        return;

    if (!client) {
        QMessageBox::warning(nullptr, tr("错误"), tr("客户端有误！"));
        return;
    }

    if (stack_isCached.at(currow)) {
        m_stackedwidget->setCurrentIndex(currow);
        return;
    }

    QListWidgetItem* catItem = ui->list_classification->item(currow);
    if (!catItem)
        return;

    const QString wantclass = catItem->text();

    const QString FLAG_INSKIND = "02";
    const QString FLAG_INS = "04";

    QJsonObject obj;
    obj.insert("want", QJsonValue(QString("*")));
    obj.insert("isDistinct", QJsonValue(QString("false")));
    obj.insert("restriction",
        QJsonValue(QString("%1='%2'").arg(FIELD_CLASSIFICATION, wantclass)));

    QByteArray data = client->sendCHTTPMsg(FLAG_CHARACTER + FLAG_INSKIND + FLAG_INS, obj);
    QString flag = client->parseHead(data);
    if (flag[0] != '1') {
        client->error(flag[0], flag.mid(1));
        return;
    }

    QJsonArray result = client->parseResponse(data);

    QListWidget* newList = new QListWidget();
    newList->setStyleSheet(
        "QListWidget{border:1px;background:#EEE;border-radius:5px;}"
        "QListWidget::Item{ background:white;height:80px;}"
        "QListWidget::Item:hover{background:rgb(246,246,247);}"
    );
    newList->setUniformItemSizes(true);
    newList->setSelectionMode(QAbstractItemView::NoSelection);

    if (!result.isEmpty()) {
        for (int i = 0; i < result.size(); ++i) {
            QJsonObject objItem = result[i].toObject();
            Product* newproduct = new Product(objItem);

            if (productlist.contains(newproduct->id)) {
                delete productlist.value(newproduct->id);
            }
            productlist.insert(newproduct->id, newproduct);

            auto* listItem = new QListWidgetItem(newList);
            auto* widget = new ProductItem();
            connect(widget, &ProductItem::signal_proitem_dblclick,
                this, &ShopPage::showProduct);

            widget->setLabNumColor(false);
            widget->setLabNumText("$" + objItem.value(FIELD_PRICE).toString());
            widget->setLabNameText(objItem.value(FIELD_NAME).toString());
            widget->setLabMessageText(objItem.value(FIELD_ABOUT).toString());
            widget->setLabrankingText("销量:" + objItem.value(FIELD_SALES).toString());
            widget->setId(objItem.value(FIELD_ID).toString());

            newList->setItemWidget(listItem, widget);
        }
    }

    if (QWidget* old = m_stackedwidget->widget(currow)) {
        m_stackedwidget->removeWidget(old);
        old->deleteLater();
    }
    m_stackedwidget->insertWidget(currow, newList);
    m_stackedwidget->setCurrentIndex(currow);

    stack_isCached[currow] = true;

    qDebug() << "updateOneClass finished:" << wantclass << currow;
}

/*
 * 打开商品详情
 */
void ShopPage::showProduct(int id)
{
    Product* p = productlist.value(id, nullptr);
    if (!p) {
        QMessageBox::warning(this, tr("错误"), tr("商品不存在或已被刷新移除"));
        return;
    }
    ProductPage* page = new ProductPage(p);
    connect(page, &ProductPage::signal_addToCart,
        this, &ShopPage::addToCart);
    page->show();
}

/*
 * 转发到外界
 */
void ShopPage::addToCart(Product* product)
{
    if (!product) return;
    emit signal_addToCart(product);
}

/*
 * 右键刷新当前分类
 */
void ShopPage::contextMenuEvent(QContextMenuEvent* event)
{
    Q_UNUSED(event);
    QAction* actionRefresh = new QAction(tr("刷新"), this);
    QMenu menu;
    menu.addAction(actionRefresh);
    QAction* selected = menu.exec(QCursor::pos());
    if (selected == actionRefresh) {
        int idx = m_stackedwidget->currentIndex();
        if (idx >= 0 && idx < stack_isCached.size()) {
            stack_isCached[idx] = false;
            emit signal_refresh(idx);
        }
    }
}


void ShopPage::searchProduct()
{
	const QString key = ui->lineEdit->text().trimmed();
	if (key.isEmpty()) return;

	Product* found = nullptr;
	for (auto it = productlist.cbegin(); it != productlist.cend(); ++it) {
		Product* p = it.value();
		if (p && p->name.contains(key, Qt::CaseInsensitive)) { found = p; break; }
	}

	if (!found) {
		QMessageBox::information(this, tr("提示"), tr("未找到相关商品"));
		return;
	}

	if (m_activeProductPage) {
		if (m_activeProductPage->productId() == found->id) {
			m_activeProductPage->raise();
			m_activeProductPage->activateWindow();
			return;
		}
		m_activeProductPage->close();
		m_activeProductPage = nullptr;
	}

	m_activeProductPage = ProductPage::simpleCenter(found, this);
	connect(m_activeProductPage, &ProductPage::signal_addToCart,
		this, &ShopPage::addToCart);
	connect(m_activeProductPage, &QObject::destroyed,
		this, [this] { m_activeProductPage = nullptr; });
}