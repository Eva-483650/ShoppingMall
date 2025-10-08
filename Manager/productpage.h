#ifndef PRODUCTPAGE_H
#define PRODUCTPAGE_H

#include <QWidget>
#include <QStandardItemModel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QGridLayout>
#include <QKeyEvent>
#include <QDir>
#include "../shoppingmanager.h"
#include "customcombobox.h"
#include "customspinbox.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ProductPage; }
QT_END_NAMESPACE

class ProductPage : public QWidget
{
    Q_OBJECT

public:
    ProductPage(QWidget* parent = nullptr);
    ~ProductPage();

    void setManager(ShoppingManager* mgr) { manager = mgr; }
	ShoppingManager* manager = nullptr;


protected:
    void showEvent(QShowEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void loadAllProducts();
    void searchProduct();
    void addProduct();
    void refreshTable();
    void onSearchTextChanged();
    void onCategoryFilterChanged(int index);
    void onSortFilterChanged(int index);
    void clearAllFilters();
    void onThemeChanged(bool isDarkMode);

private:
    Ui::ProductPage* ui;
    QStandardItemModel* localModel = nullptr;
    QNetworkAccessManager* networkManager = nullptr;

    // 现代化UI组件
    QScrollArea* scrollArea = nullptr;
    QWidget* cardContainer = nullptr;
    QVBoxLayout* cardLayout = nullptr;
    QHBoxLayout* filtersLayout = nullptr;
    CustomComboBox* categoryFilter = nullptr;
    CustomComboBox* sortFilter = nullptr;

    // 数据
    QJsonArray allProducts;
    bool isCached;
    int totalProductCount;
    int selectedCount;

    // UI创建方法
    void createLocalModel();
    void createModernUI();
    void setupFilters();
    void setupConnections();

    // 商品卡片
    void createProductCard(const QJsonObject& product);
    void clearProductCards();

    // 数据处理
    void displayProducts(const QJsonArray& products);
    void addProductToModel(const QJsonObject& product);
    void applyFilters();

    // 筛选和排序
    QJsonArray filterBySearch(const QJsonArray& products, const QString& searchText);
    QJsonArray filterByCategory(const QJsonArray& products, const QString& category);
    QJsonArray sortProducts(const QJsonArray& products, int sortType);

    // 图片处理
    void loadProductImage(QLabel* imageLabel, const QString& imagePath);
    QString getImageUrl(const QString& imagePath);
    void setDefaultProductImage(QLabel* imageLabel);

    // 服务器操作
    bool addProductToServer(const QJsonObject& product);
    bool updateProductToServer(const QJsonObject& product);
    bool deleteProductFromServer(const QString& productId);

    // 对话框
    void showAddProductDialog();
    void showEditProductDialog(const QJsonObject& product);

    // 统计更新
    void updateStatistics();
    void updateSelectionInfo();

};

#endif // PRODUCTPAGE_H
