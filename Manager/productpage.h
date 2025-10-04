#ifndef PRODUCTPAGE_H
#define PRODUCTPAGE_H

#include <QWidget>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QShowEvent>
#include <QTimer>
#include <QDebug>
#include <QKeyEvent>
#include <QItemSelectionModel>
#include <QModelIndexList>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QProgressDialog>
#include <QApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QStandardItem>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <algorithm>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QTimer>
#include "shoppingmanager.h"

namespace Ui {
	class ProductPage;
}

class ProductPage : public QWidget
{
	Q_OBJECT

public:
	explicit ProductPage(QWidget* parent = nullptr);
	~ProductPage();
	ShoppingManager* manager;

protected:
	void showEvent(QShowEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;

private:
	Ui::ProductPage* ui;
	QStandardItemModel* localModel;
	bool isCached;
	int totalProductCount;
	int selectedCount;
    QNetworkAccessManager* networkManager;

    void loadProductImage(QLabel* imageLabel, const QString& imagePath);
    QString getImageUrl(const QString& imagePath);
    void setDefaultProductImage(QLabel* imageLabel);

	// 数据存储
	QJsonArray allProducts;       // 存储所有商品数据
	QJsonArray filteredProducts;  // 存储筛选后的数据
	QList<QJsonObject> selectedProducts; // 存储选中的商品

	// UI组件
	QScrollArea* scrollArea;
	QWidget* cardContainer;
	QVBoxLayout* cardLayout;
	QHBoxLayout* filtersLayout;
	QComboBox* categoryFilter;
	QComboBox* sortFilter;

	// 私有方法
	void setupConnections();
	void createLocalModel();
	void updateStatistics();
	void updateSelectionInfo();
	void addProductToModel(const QJsonObject& product);
	QString formatSearchFilter(const QString& searchText);
	bool validateProductData(int row);
	void showProductDetails(int row);
	QJsonObject getProductFromRow(int row);
	void setRowData(int row, const QJsonObject& product);

	// 现代化UI相关
	void createModernUI();
	void createProductCard(const QJsonObject& product);
	void clearProductCards();
	void setupFilters();
	void displayProducts(const QJsonArray& products);

	// 筛选和排序
	void applyFilters();
	QJsonArray filterByCategory(const QJsonArray& products, const QString& category);
	QJsonArray filterBySearch(const QJsonArray& products, const QString& searchText);
	QJsonArray sortProducts(const QJsonArray& products, int sortType);
    void showEditProductDialog(const QJsonObject& product);
	// 商品操作
	bool addProductToServer(const QJsonObject& product);
	bool updateProductOnServer(const QJsonObject& product, const QString& productId);
	bool deleteProductFromServer(const QString& productId);
    void showAddProductDialog();
private slots:
	void submitChange();
	void backChange();
	void searchProduct();
	void addProduct();
	void delProduct();
	void refreshTable();
	void onSearchTextChanged();
	void onSelectionChanged();
	void onTableDoubleClicked(const QModelIndex& index);
	void onThemeChanged(bool isDarkMode);

	// 网络请求槽函数
	void loadAllProducts();
	void saveProductChanges();
	void deleteSelectedProducts();

	// 筛选器槽函数
	void onCategoryFilterChanged(int index);
	void onSortFilterChanged(int index);
	void clearAllFilters();

signals:
	void productAdded(int productId);
	void productDeleted(int productId);
	void productUpdated(int productId);
	void selectionChanged(int count);
};

#endif // PRODUCTPAGE_H
