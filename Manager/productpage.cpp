#include "productpage.h"
#include "ui_productpage.h"
// 在构造函数中初始化网络管理器
ProductPage::ProductPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ProductPage),
    isCached(false),
    totalProductCount(0),
    selectedCount(0)
{
    ui->setupUi(this);

    // 初始化网络管理器
    networkManager = new QNetworkAccessManager(this);

    // 创建本地数据模型
    createLocalModel();

    // 初始化UI状态
    updateStatistics();
    updateSelectionInfo();

    qDebug() << "ProductPage 初始化完成";
}

ProductPage::~ProductPage()
{
    delete ui;
}

void ProductPage::createLocalModel()
{
    // 保留原有的数据模型用于数据管理
    localModel = new QStandardItemModel(this);

    // 创建现代化UI
    createModernUI();
}

void ProductPage::createModernUI()
{
    // 隐藏原来的表格
    ui->tableView->hide();

    // 创建滚动区域和卡片容器
    scrollArea = new QScrollArea(ui->tableWidget);
    cardContainer = new QWidget();
    cardLayout = new QVBoxLayout(cardContainer);

    // 设置滚动区域
    scrollArea->setWidget(cardContainer);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 设置样式
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "    background: transparent;"
        "    border: none;"
        "}"
        "QScrollArea > QWidget > QWidget {"
        "    background: transparent;"
        "}"
        );

    // 将滚动区域添加到表格容器中
    QVBoxLayout* tableMainLayout = qobject_cast<QVBoxLayout*>(ui->tableWidget->layout());
    if (tableMainLayout) {
        // 在表头后面插入筛选器
        setupFilters();
        tableMainLayout->insertLayout(1, filtersLayout);

        // 添加滚动区域
        tableMainLayout->addWidget(scrollArea);
    }

    cardLayout->setSpacing(15);
    cardLayout->setContentsMargins(10, 10, 10, 10);
    cardLayout->addStretch(); // 底部弹性空间
}

void ProductPage::setupFilters()
{
    filtersLayout = new QHBoxLayout();

    // 分类筛选器
    QLabel* categoryLabel = new QLabel("分类筛选:");
    categoryLabel->setStyleSheet("color: #6c5b7b; font: 600 10pt '微软雅黑';");

    categoryFilter = new QComboBox();
    categoryFilter->addItems({"全部分类", "手机数码", "服饰鞋帽", "美妆护肤", "家居家电", "电脑办公", "其他"});

    QString comboStyle =
        "QComboBox {"
        "    background: rgba(255, 255, 255, 0.9);"
        "    border: 2px solid rgba(214, 201, 247, 0.5);"
        "    border-radius: 8px;"
        "    padding: 8px 12px;"
        "    font: 10pt '微软雅黑';"
        "    color: #5a4c74;"
        "    min-width: 100px;"
        "}"
        "QComboBox:hover {"
        "    border: 2px solid #a18cd1;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 20px;"
        "}"
        "QComboBox QAbstractItemView {"
        "    background: white;"
        "    border: 1px solid #d6c9f7;"
        "    border-radius: 8px;"
        "    selection-background-color: #e8e6fb;"
        "}";

    categoryFilter->setStyleSheet(comboStyle);

    // 排序筛选器
    QLabel* sortLabel = new QLabel("排序:");
    sortLabel->setStyleSheet("color: #6c5b7b; font: 600 10pt '微软雅黑';");

    sortFilter = new QComboBox();
    sortFilter->addItems({"默认排序", "价格从低到高", "价格从高到低", "销量从高到低", "库存从多到少", "按名称排序"});
    sortFilter->setStyleSheet(comboStyle);

    // 清除筛选按钮
    QPushButton* clearFiltersBtn = new QPushButton("清除筛选");
    clearFiltersBtn->setStyleSheet(
        "QPushButton {"
        "    background: rgba(184, 164, 230, 0.3);"
        "    border: 2px solid rgba(161, 140, 209, 0.5);"
        "    border-radius: 8px;"
        "    padding: 8px 15px;"
        "    font: 600 10pt '微软雅黑';"
        "    color: #7a6599;"
        "}"
        "QPushButton:hover {"
        "    background: rgba(184, 164, 230, 0.5);"
        "    border: 2px solid #a18cd1;"
        "}"
        );

    filtersLayout->addWidget(categoryLabel);
    filtersLayout->addWidget(categoryFilter);
    filtersLayout->addSpacing(20);
    filtersLayout->addWidget(sortLabel);
    filtersLayout->addWidget(sortFilter);
    filtersLayout->addSpacing(15);
    filtersLayout->addWidget(clearFiltersBtn);
    filtersLayout->addStretch();

    // 连接信号
    connect(categoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProductPage::onCategoryFilterChanged);
    connect(sortFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProductPage::onSortFilterChanged);
    connect(clearFiltersBtn, &QPushButton::clicked, this, &ProductPage::clearAllFilters);
}

void ProductPage::createProductCard(const QJsonObject& product)
{
    QWidget* card = new QWidget();
    card->setFixedHeight(120);
    card->setStyleSheet(
        "QWidget {"
        "    background: rgba(255, 255, 255, 0.95);"
        "    border-radius: 12px;"
        "    border: 1px solid rgba(214, 201, 247, 0.4);"
        "}"
        "QWidget:hover {"
        "    background: rgba(255, 255, 255, 1.0);"
        "    border: 2px solid rgba(161, 140, 209, 0.6);"
        "}"
        );

    QHBoxLayout* cardMainLayout = new QHBoxLayout(card);
    cardMainLayout->setContentsMargins(15, 10, 15, 10);
    cardMainLayout->setSpacing(15);

    // 左侧：商品图片
    QLabel* imageLabel = new QLabel();
    imageLabel->setFixedSize(80, 80);
    imageLabel->setStyleSheet(
        "QLabel {"
        "    background: qlineargradient(spread:pad,x1:0,y1:0,x2:1,y2:1,"
        "                stop:0 rgba(184, 164, 230, 0.3),"
        "                stop:1 rgba(161, 140, 209, 0.3));"
        "    border-radius: 8px;"
        "    border: 1px solid rgba(161, 140, 209, 0.3);"
        "    color: #8b78c4;"
        "    font: 600 9pt '微软雅黑';"
        "}"
        );
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setScaledContents(true); // 启用缩放内容

    // 加载商品图片
    loadProductImage(imageLabel, product.value("Product_pictureaddress").toString());
    // 中间：商品信息
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);

    // 商品名称和ID
    QHBoxLayout* titleLayout = new QHBoxLayout();
    QLabel* nameLabel = new QLabel(product.value("Product_name").toString());
    nameLabel->setStyleSheet("color: #4a4a4a; font: 700 13pt '微软雅黑';");

    QLabel* idLabel = new QLabel(QString("ID: %1").arg(product.value("Product_id").toString()));
    idLabel->setStyleSheet("color: #8b78c4; font: 9pt '微软雅黑'; background: rgba(139, 120, 196, 0.1); padding: 2px 6px; border-radius: 8px;");

    titleLayout->addWidget(nameLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(idLabel);

    // 分类和限时标签
    QHBoxLayout* tagsLayout = new QHBoxLayout();
    QLabel* categoryLabel = new QLabel(product.value("Product_classification").toString());
    categoryLabel->setStyleSheet("color: #6c5b7b; font: 9pt '微软雅黑'; background: rgba(108, 91, 123, 0.1); padding: 2px 6px; border-radius: 6px;");

    QString isTimeLimited = product.value("Product_istimelimited").toString();
    if (isTimeLimited == "是") {
        QLabel* limitedLabel = new QLabel("⏰ 限时");
        limitedLabel->setStyleSheet("color: #ff6b6b; font: 600 8pt '微软雅黑'; background: rgba(255, 107, 107, 0.1); padding: 2px 4px; border-radius: 6px;");
        tagsLayout->addWidget(limitedLabel);
    }

    tagsLayout->addWidget(categoryLabel);
    tagsLayout->addStretch();

    // 商品描述
    QString description = product.value("Product_about").toString();
    if (description.length() > 40) {
        description = description.left(37) + "...";
    }
    QLabel* descLabel = new QLabel(description.isEmpty() ? "暂无描述" : description);
    descLabel->setStyleSheet("color: #7a7a7a; font: 9pt '微软雅黑';");

    infoLayout->addLayout(titleLayout);
    infoLayout->addLayout(tagsLayout);
    infoLayout->addWidget(descLabel);
    infoLayout->addStretch();

    // 右侧：价格和库存信息
    QVBoxLayout* statsLayout = new QVBoxLayout();
    statsLayout->setSpacing(6);

    // 价格
    QLabel* priceLabel = new QLabel(QString("¥%1").arg(product.value("Product_price").toString()));
    priceLabel->setStyleSheet("color: #e74c3c; font: 700 16pt '微软雅黑';");
    priceLabel->setAlignment(Qt::AlignRight);

    // 库存和销量
    QHBoxLayout* numbersLayout = new QHBoxLayout();
    numbersLayout->setSpacing(2);

    QVBoxLayout* stockLayout = new QVBoxLayout();
    QLabel* stockTitle = new QLabel("库存");
    stockTitle->setStyleSheet("color: #8b78c4; font: 9pt '微软雅黑';");
    stockTitle->setAlignment(Qt::AlignCenter);
    QLabel* stockValue = new QLabel(product.value("Product_amount").toString());
    stockValue->setStyleSheet("color: #4a4a4a; font: 700 10pt '微软雅黑';");
    stockValue->setAlignment(Qt::AlignCenter);
    stockLayout->addWidget(stockTitle);
    stockLayout->addWidget(stockValue);

    QVBoxLayout* salesLayout = new QVBoxLayout();
    QLabel* salesTitle = new QLabel("销量");
    salesTitle->setStyleSheet("color: #8b78c4; font: 9pt '微软雅黑';");
    salesTitle->setAlignment(Qt::AlignCenter);
    QLabel* salesValue = new QLabel(product.value("Product_sales").toString());
    salesValue->setStyleSheet("color: #4a4a4a; font: 700 10pt '微软雅黑';");
    salesValue->setAlignment(Qt::AlignCenter);
    salesLayout->addWidget(salesTitle);
    salesLayout->addWidget(salesValue);

    numbersLayout->addLayout(stockLayout);
    numbersLayout->addLayout(salesLayout);

    // 操作按钮
    QHBoxLayout* actionsLayout = new QHBoxLayout();
    QPushButton* editBtn = new QPushButton("📝");
    QPushButton* deleteBtn = new QPushButton("🗑️");

    QString buttonStyle =
        "QPushButton {"
        "    background: rgba(184, 164, 230, 0.2);"
        "    border: 1px solid rgba(161, 140, 209, 0.4);"
        "    border-radius: 12px;"
        "    padding: 4px;"
        "    font: 10pt;"
        "    min-width: 22px;"
        "    max-width: 22px;"
        "    min-height: 22px;"
        "    max-height: 22px;"
        "}"
        "QPushButton:hover {"
        "    background: rgba(161, 140, 209, 0.3);"
        "    border: 2px solid #a18cd1;"
        "}";

    editBtn->setStyleSheet(buttonStyle);
    deleteBtn->setStyleSheet(buttonStyle);
    editBtn->setToolTip("编辑商品");
    deleteBtn->setToolTip("删除商品");

    actionsLayout->addStretch();
    actionsLayout->addWidget(editBtn);
    actionsLayout->addWidget(deleteBtn);

    statsLayout->addWidget(priceLabel);
    statsLayout->addLayout(numbersLayout);
    statsLayout->addLayout(actionsLayout);

    // 组装卡片
    cardMainLayout->addWidget(imageLabel);
    cardMainLayout->addLayout(infoLayout, 1);
    cardMainLayout->addLayout(statsLayout);

    // 在底部弹性空间之前插入卡片
    cardLayout->insertWidget(cardLayout->count() - 1, card);

    // 连接按钮事件
    connect(editBtn, &QPushButton::clicked, [this, product]() {
        QString productId = product.value("Product_id").toString();
        QString productName = product.value("Product_name").toString();
        showEditProductDialog(product);
    });

    connect(deleteBtn, &QPushButton::clicked, [this, product]() {
        QString productId = product.value("Product_id").toString();
        QString productName = product.value("Product_name").toString();

        int ret = QMessageBox::question(this, "确认删除",
                                        QString("确定要删除商品 '%1' 吗？").arg(productName),
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            if (deleteProductFromServer(productId)) {
                QMessageBox::information(this, "删除成功", "商品已成功删除！");
                loadAllProducts(); // 重新加载数据
            }
        }
    });
}

void ProductPage::clearProductCards()
{
    while (cardLayout->count() > 1) {
        QLayoutItem* item = cardLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void ProductPage::displayProducts(const QJsonArray& products)
{
    localModel->clear();
    clearProductCards();

    QStringList headers;
    headers << "商品ID" << "商品名称" << "价格(元)" << "库存" << "销量"
            << "分类" << "限时商品" << "商品描述" << "图片地址";
    localModel->setHorizontalHeaderLabels(headers);

    for (const QJsonValue& value : products) {
        QJsonObject product = value.toObject();
        addProductToModel(product);
    }

    totalProductCount = products.size();
    updateStatistics();
}

void ProductPage::addProductToModel(const QJsonObject& product)
{
    QList<QStandardItem*> items;
    items.append(new QStandardItem(product.value("Product_id").toString()));
    items.append(new QStandardItem(product.value("Product_name").toString()));
    items.append(new QStandardItem(product.value("Product_price").toString()));
    items.append(new QStandardItem(product.value("Product_amount").toString()));
    items.append(new QStandardItem(product.value("Product_sales").toString()));
    items.append(new QStandardItem(product.value("Product_classification").toString()));
    items.append(new QStandardItem(product.value("Product_istimelimited").toString()));
    items.append(new QStandardItem(product.value("Product_about").toString()));
    items.append(new QStandardItem(product.value("Product_pictureaddress").toString()));

    items[0]->setEditable(false);
    items[4]->setEditable(false);

    localModel->appendRow(items);
    createProductCard(product);
}

void ProductPage::showEvent(QShowEvent* event)
{
    if (!isCached && manager) {
        setupConnections();
        loadAllProducts();
        isCached = true;
        qDebug() << "ProductPage 首次显示，数据已加载";
    }
    QWidget::showEvent(event);
}

void ProductPage::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_F5) {
        refreshTable();
        return;
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (ui->searchInput->hasFocus()) {
            searchProduct();
            return;
        }
    }

    QWidget::keyPressEvent(event);
}

void ProductPage::setupConnections()
{
    // 简化后的按钮连接
    connect(ui->searchButton, &QPushButton::clicked, this, &ProductPage::searchProduct);
    connect(ui->addButton, &QPushButton::clicked, this, &ProductPage::addProduct);
    connect(ui->refreshButton, &QPushButton::clicked, this, &ProductPage::loadAllProducts);

    // 搜索输入框
    connect(ui->searchInput, &QLineEdit::textChanged, this, &ProductPage::onSearchTextChanged);
    connect(ui->searchInput, &QLineEdit::returnPressed, this, &ProductPage::searchProduct);

    // 主题变化
    if (manager) {
        connect(manager, &ShoppingManager::themeChanged, this, &ProductPage::onThemeChanged);
    }
}

void ProductPage::loadAllProducts()
{
    if (!manager || !manager->getConnected()) {
        QMessageBox::warning(this, "错误", "未连接到服务器！");
        return;
    }

    QJsonObject requestBody;
    requestBody.insert("want", "*");
    requestBody.insert("isDistinct", "false");

    qDebug() << "发送获取所有商品请求...";

    QByteArray response = manager->sendCHTTPMsg("20204", requestBody);
    QString flag = manager->parseHead(response);

    if (flag.isEmpty() || flag[0] != '1') {
        if (!flag.isEmpty()) {
            manager->error(flag[0], flag.mid(1));
        } else {
            QMessageBox::warning(this, "错误", "获取商品数据失败！");
        }
        return;
    }

    allProducts = manager->parseResponse(response);
    qDebug() << "商品数据加载完成，数量:" << allProducts.size();

    displayProducts(allProducts);

    if (totalProductCount > 0) {
        ui->tableHeaderLabel->setText(QString("商品列表 (共%1个商品)").arg(totalProductCount));
    } else {
        ui->tableHeaderLabel->setText("商品列表 (暂无数据)");
    }
}

void ProductPage::searchProduct()
{
    applyFilters();
}

void ProductPage::applyFilters()
{
    QJsonArray results = allProducts;

    QString searchText = ui->searchInput->text().trimmed();
    if (!searchText.isEmpty()) {
        results = filterBySearch(results, searchText);
    }

    if (categoryFilter->currentIndex() > 0) {
        QString selectedCategory = categoryFilter->currentText();
        results = filterByCategory(results, selectedCategory);
    }

    if (sortFilter->currentIndex() > 0) {
        results = sortProducts(results, sortFilter->currentIndex());
    }

    displayProducts(results);

    QString title = "商品列表";
    if (!searchText.isEmpty()) {
        title = QString("搜索结果: '%1'").arg(searchText);
    }
    if (categoryFilter->currentIndex() > 0) {
        title += QString(" - %1").arg(categoryFilter->currentText());
    }
    title += QString(" (共%1个商品)").arg(results.size());

    ui->tableHeaderLabel->setText(title);
}

// 实现上线商品功能
void ProductPage::addProduct()
{
    showAddProductDialog();
}

void ProductPage::showAddProductDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("上线新商品");
    dialog.setFixedSize(500, 600);
    dialog.setStyleSheet(
        "QDialog { background: rgba(255, 255, 255, 0.95); border-radius: 15px; }"
        "QLabel { color: #6c5b7b; font: 600 11pt '微软雅黑'; }"
        "QLineEdit, QTextEdit, QComboBox, QSpinBox { "
        "    background: rgba(255, 255, 255, 0.8);"
        "    border: 2px solid rgba(214, 201, 247, 0.5);"
        "    border-radius: 8px;"
        "    padding: 8px;"
        "    font: 10pt '微软雅黑';"
        "    color: #5a4c74;"
        "}"
        "QLineEdit:focus, QTextEdit:focus, QComboBox:focus, QSpinBox:focus {"
        "    border: 2px solid #a18cd1;"
        "}"
        "QPushButton {"
        "    background: qlineargradient(spread:pad,x1:0,y1:0,x2:1,y2:0,"
        "                stop:0 #b8a4e6, stop:1 #a18cd1);"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 10px 20px;"
        "    font: 600 11pt '微软雅黑';"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(spread:pad,x1:0,y1:0,x2:1,y2:0,"
        "                stop:0 #a18cd1, stop:1 #9179c4);"
        "}"
        "QPushButton#cancelButton {"
        "    background: rgba(184, 164, 230, 0.3);"
        "    border: 2px solid rgba(161, 140, 209, 0.5);"
        "    color: #7a6599;"
        "}"
        );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 30, 30, 30);

    // 标题
    QLabel* titleLabel = new QLabel("📦 添加新商品");
    titleLabel->setStyleSheet("font: 700 16pt '微软雅黑'; color: #6c5b7b;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // 表单布局
    QGridLayout* formLayout = new QGridLayout();
    formLayout->setSpacing(12);

    // 商品名称
    QLabel* nameLabel = new QLabel("商品名称*:");
    QLineEdit* nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("请输入商品名称");
    formLayout->addWidget(nameLabel, 0, 0);
    formLayout->addWidget(nameEdit, 0, 1);

    // 商品价格
    QLabel* priceLabel = new QLabel("商品价格*:");
    QSpinBox* priceSpinBox = new QSpinBox();
    priceSpinBox->setRange(0, 999999);
    priceSpinBox->setSuffix(" 元");
    formLayout->addWidget(priceLabel, 1, 0);
    formLayout->addWidget(priceSpinBox, 1, 1);

    // 库存数量
    QLabel* stockLabel = new QLabel("库存数量*:");
    QSpinBox* stockSpinBox = new QSpinBox();
    stockSpinBox->setRange(0, 999999);
    stockSpinBox->setSuffix(" 件");
    formLayout->addWidget(stockLabel, 2, 0);
    formLayout->addWidget(stockSpinBox, 2, 1);

    // 商品分类
    QLabel* categoryLabel = new QLabel("商品分类*:");
    QComboBox* categoryCombo = new QComboBox();
    categoryCombo->addItems({"手机数码", "服饰鞋帽", "美妆护肤", "家居家电", "电脑办公", "其他"});
    formLayout->addWidget(categoryLabel, 3, 0);
    formLayout->addWidget(categoryCombo, 3, 1);

    // 是否限时
    QLabel* limitedLabel = new QLabel("限时商品:");
    QComboBox* limitedCombo = new QComboBox();
    limitedCombo->addItems({"否", "是"});
    formLayout->addWidget(limitedLabel, 4, 0);
    formLayout->addWidget(limitedCombo, 4, 1);

    // 图片地址
    QLabel* imageLabel = new QLabel("图片地址:");
    QLineEdit* imageEdit = new QLineEdit();
    imageEdit->setPlaceholderText("请输入图片URL（可选）");
    formLayout->addWidget(imageLabel, 5, 0);
    formLayout->addWidget(imageEdit, 5, 1);

    layout->addLayout(formLayout);

    // 商品描述
    QLabel* descLabel = new QLabel("商品描述:");
    QTextEdit* descEdit = new QTextEdit();
    descEdit->setPlaceholderText("请输入商品详细描述...");
    descEdit->setMaximumHeight(100);
    layout->addWidget(descLabel);
    layout->addWidget(descEdit);

    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* cancelButton = new QPushButton("取消");
    cancelButton->setObjectName("cancelButton");
    QPushButton* submitButton = new QPushButton("上线商品");

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(submitButton);
    layout->addLayout(buttonLayout);

    // 连接按钮事件
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(submitButton, &QPushButton::clicked, [&]() {
        // 验证必填字段
        if (nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "错误", "请输入商品名称！");
            return;
        }
        if (priceSpinBox->value() <= 0) {
            QMessageBox::warning(&dialog, "错误", "请设置商品价格！");
            return;
        }

        // 构建商品数据
        QJsonObject product;
        product.insert("Product_name", nameEdit->text().trimmed());
        product.insert("Product_price", QString::number(priceSpinBox->value()));
        product.insert("Product_amount", QString::number(stockSpinBox->value()));
        product.insert("Product_sales", "0");
        product.insert("Product_classification", categoryCombo->currentText());
        product.insert("Product_istimelimited", limitedCombo->currentText());
        product.insert("Product_about", descEdit->toPlainText());
        product.insert("Product_pictureaddress", imageEdit->text().trimmed());

        // 发送到服务器
        if (addProductToServer(product)) {
            QMessageBox::information(&dialog, "成功", "商品上线成功！");
            dialog.accept();
            loadAllProducts(); // 刷新列表
        } else {
            QMessageBox::warning(&dialog, "失败", "商品上线失败，请重试！");
        }
    });

    dialog.exec();
}

void ProductPage::showEditProductDialog(const QJsonObject& product)
{
    QMessageBox::information(this, "编辑商品",
                             QString("编辑商品功能开发中...\n\n商品: %1")
                                 .arg(product.value("Product_name").toString()));
}

// 保留其他必要的方法实现
QJsonArray ProductPage::filterBySearch(const QJsonArray& products, const QString& searchText)
{
    QJsonArray filtered;
    QString searchLower = searchText.toLower();

    for (const QJsonValue& value : products) {
        QJsonObject product = value.toObject();
        QString name = product.value("Product_name").toString().toLower();
        QString category = product.value("Product_classification").toString().toLower();
        QString id = product.value("Product_id").toString();
        QString about = product.value("Product_about").toString().toLower();

        if (name.contains(searchLower) ||
            category.contains(searchLower) ||
            id.contains(searchLower) ||
            about.contains(searchLower)) {
            filtered.append(value);
        }
    }

    return filtered;
}

QJsonArray ProductPage::filterByCategory(const QJsonArray& products, const QString& category)
{
    if (category == "全部分类") {
        return products;
    }

    QJsonArray filtered;
    for (const QJsonValue& value : products) {
        QJsonObject product = value.toObject();
        if (product.value("Product_classification").toString() == category) {
            filtered.append(value);
        }
    }
    return filtered;
}

QJsonArray ProductPage::sortProducts(const QJsonArray& products, int sortType)
{
    QList<QJsonObject> productList;
    for (const QJsonValue& value : products) {
        productList.append(value.toObject());
    }

    switch (sortType) {
    case 1: // 价格从低到高
        std::sort(productList.begin(), productList.end(), [](const QJsonObject& a, const QJsonObject& b) {
            return a.value("Product_price").toString().toInt() < b.value("Product_price").toString().toInt();
        });
        break;
    case 2: // 价格从高到低
        std::sort(productList.begin(), productList.end(), [](const QJsonObject& a, const QJsonObject& b) {
            return a.value("Product_price").toString().toInt() > b.value("Product_price").toString().toInt();
        });
        break;
    case 3: // 销量从高到低
        std::sort(productList.begin(), productList.end(), [](const QJsonObject& a, const QJsonObject& b) {
            return a.value("Product_sales").toString().toInt() > b.value("Product_sales").toString().toInt();
        });
        break;
    case 4: // 库存从多到少
        std::sort(productList.begin(), productList.end(), [](const QJsonObject& a, const QJsonObject& b) {
            return a.value("Product_amount").toString().toInt() > b.value("Product_amount").toString().toInt();
        });
        break;
    case 5: // 按名称排序
        std::sort(productList.begin(), productList.end(), [](const QJsonObject& a, const QJsonObject& b) {
            return a.value("Product_name").toString() < b.value("Product_name").toString();
        });
        break;
    }

    QJsonArray result;
    for (const QJsonObject& product : productList) {
        result.append(product);
    }
    return result;
}

bool ProductPage::addProductToServer(const QJsonObject& product)
{
    if (!manager || !manager->getConnected()) {
        return false;
    }

    QByteArray response = manager->sendCHTTPMsg("20201", product);
    QString flag = manager->parseHead(response);

    if (!flag.isEmpty() && flag[0] == '1') {
        qDebug() << "商品添加成功";
        return true;
    } else {
        qDebug() << "商品添加失败:" << flag.mid(1);
        return false;
    }
}

bool ProductPage::deleteProductFromServer(const QString& productId)
{
    if (!manager || !manager->getConnected()) {
        return false;
    }

    QJsonObject deleteObj;
    deleteObj.insert("restriction", QString("Product_id = %1").arg(productId));

    QByteArray response = manager->sendCHTTPMsg("20203", deleteObj);
    QString flag = manager->parseHead(response);

    if (!flag.isEmpty() && flag[0] == '1') {
        qDebug() << "商品删除成功";
        return true;
    } else {
        qDebug() << "商品删除失败:" << flag.mid(1);
        return false;
    }
}

void ProductPage::updateStatistics()
{
    ui->countLabel->setText(QString::number(totalProductCount));
}

void ProductPage::updateSelectionInfo()
{
    ui->selectionLabel->setText(QString("已选择 %1 项").arg(selectedCount));
}

void ProductPage::refreshTable()
{
    loadAllProducts();
    ui->searchInput->clear();
    if (categoryFilter) categoryFilter->setCurrentIndex(0);
    if (sortFilter) sortFilter->setCurrentIndex(0);
}

void ProductPage::onSearchTextChanged()
{
    QString text = ui->searchInput->text();
    ui->searchButton->setEnabled(true);

    if (text.trimmed().isEmpty()) {
        applyFilters();
    }
}

void ProductPage::onCategoryFilterChanged(int index)
{
    qDebug() << "分类筛选改变，索引:" << index;
    applyFilters();
}

void ProductPage::onSortFilterChanged(int index)
{
    qDebug() << "排序方式改变，索引:" << index;
    applyFilters();
}

void ProductPage::clearAllFilters()
{
    if (categoryFilter) categoryFilter->setCurrentIndex(0);
    if (sortFilter) sortFilter->setCurrentIndex(0);
    ui->searchInput->clear();
    displayProducts(allProducts);
    ui->tableHeaderLabel->setText(QString("商品列表 (共%1个商品)").arg(allProducts.size()));
}

void ProductPage::onThemeChanged(bool isDarkMode)
{
    qDebug() << "商品页面主题已更新:" << (isDarkMode ? "夜间模式" : "日间模式");
}

// 移除这些在简化UI中不存在的方法
void ProductPage::submitChange() { /* 已移除 */ }
void ProductPage::backChange() { /* 已移除 */ }
void ProductPage::delProduct() { /* 已移除 */ }
void ProductPage::onSelectionChanged() { /* 已移除 */ }
void ProductPage::onTableDoubleClicked(const QModelIndex& index) { /* 已移除 */ }
void ProductPage::showProductDetails(int row) { /* 已移除 */ }
void ProductPage::saveProductChanges() { /* 已移除 */ }
void ProductPage::deleteSelectedProducts() { /* 已移除 */ }


// 加载商品图片
// 加载商品图片 - 针对本地资源优化
void ProductPage::loadProductImage(QLabel* imageLabel, const QString& imagePath)
{
    // 设置默认图片
    setDefaultProductImage(imageLabel);

    // 如果没有图片路径，直接返回
    if (imagePath.isEmpty()) {
        return;
    }

    // 构建完整的图片路径
    QString imageUrl = getImageUrl(imagePath);

    qDebug() << "加载商品图片:" << imageUrl;

    // 如果是资源文件，直接加载
    if (imageUrl.startsWith(":/")) {
        QPixmap pixmap(imageUrl);

        if (!pixmap.isNull()) {
            // 成功加载本地资源图片
            QPixmap scaledPixmap = pixmap.scaled(imageLabel->size(),
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation);
            imageLabel->setPixmap(scaledPixmap);
            imageLabel->setStyleSheet(
                "QLabel {"
                "    border-radius: 8px;"
                "    border: 1px solid rgba(161, 140, 209, 0.3);"
                "}"
                );
            qDebug() << "本地资源图片加载成功:" << imageUrl;
        } else {
            qDebug() << "本地资源图片加载失败:" << imageUrl;
            setDefaultProductImage(imageLabel);
        }
        return;
    }

    // 如果是网络图片，使用网络请求（保留您原有的网络加载逻辑）
   // loadNetworkImage(imageLabel, imageUrl);
}


// 构建图片URL
QString ProductPage::getImageUrl(const QString& imagePath)
{
    // 如果已经是完整的资源路径，直接返回
    if (imagePath.startsWith(":/")) {
        return imagePath;
    }

    // 如果是网络URL，直接返回
    // if (imagePath.startsWith("http://") || imagePath.startsWith("https://")) {
    //     return imagePath;
    // }

    // 如果路径为空，返回空
    if (imagePath.trimmed().isEmpty()) {
        return QString();
    }

    // 构建Qt资源路径
    QString resourcePath = ":/images/products/";

    // 清理路径
    QString cleanPath = imagePath;

    // 如果路径已经包含完整的资源路径，避免重复
    if (cleanPath.startsWith(":/images/products/")) {
        return cleanPath;
    }

    // 如果只包含部分路径，去掉重复部分
    if (cleanPath.startsWith("images/products/")) {
        cleanPath = cleanPath.mid(16); // 去掉 "images/products/" 前缀
    } else if (cleanPath.startsWith("products/")) {
        cleanPath = cleanPath.mid(9); // 去掉 "products/" 前缀
    }

    // 如果路径以 / 开头，去掉
    if (cleanPath.startsWith("/")) {
        cleanPath = cleanPath.mid(1);
    }

    // 构建完整的资源路径
    QString fullPath = resourcePath + cleanPath;

    qDebug() << "构建图片资源路径:" << imagePath << " -> " << fullPath;

    return fullPath;
}

// 设置默认商品图
void ProductPage::setDefaultProductImage(QLabel* imageLabel)
{
    imageLabel->setStyleSheet(
        "QLabel {"
        "    background: qlineargradient(spread:pad,x1:0,y1:0,x2:1,y2:1,"
        "                stop:0 rgba(184, 164, 230, 0.3),"
        "                stop:1 rgba(161, 140, 209, 0.3));"
        "    border-radius: 8px;"
        "    border: 1px solid rgba(161, 140, 209, 0.3);"
        "    color: #8b78c4;"
        "    font: 600 9pt '微软雅黑';"
        "}"
        );
    imageLabel->setPixmap(QPixmap()); // 清空图片
    imageLabel->setText("📦\n商品图片");
}
