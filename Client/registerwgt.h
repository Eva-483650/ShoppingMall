#ifndef REGISTERWGT_H
#define REGISTERWGT_H

#include <QWidget>
#include <QPixmap>

class QLineEdit;
class QLabel;
class QToolButton;
class QProgressBar;
class QComboBox;
class QPushButton;
class QCheckBox;
class QStackedWidget;
class QGraphicsOpacityEffect;
class ShoppingClient;

namespace Ui {
class RegisterWgt;
}

class RegisterWgt : public QWidget
{
    Q_OBJECT
public:
    explicit RegisterWgt(ShoppingClient *client, QWidget *parent=nullptr);
    ~RegisterWgt() override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onNext();
    void onBack();
    void onRegister();                 // 最后一步提交
    void onPasswordChanged(const QString &);
    void onAnyFieldChanged();
    void onTogglePwd1();
    void onTogglePwd2();
    void onSelectAvatar();

private:
    Ui::RegisterWgt *ui;
    ShoppingClient  *m_client;
    void paintEvent(QPaintEvent *ev);

    int  m_currentStep = 0;            // 0,1,2
    const int m_totalSteps = 3;
    QPixmap m_avatarPixmap;

    // 初始化
    void initUi();
    void initConnections();
    void applyStepStyle(int step, bool first=false);
    void updateProgress();
    void goToStep(int step, bool forward);
    void fillSummary();
    void updateFinalButtonEnabled();
    void updatePasswordStrength(const QString &pwd);

    // 校验
    bool validateStep0(QString *errMsg=nullptr) const; // 账号页
    bool validateStep1(QString *errMsg=nullptr) const; // 资料页（可选项很少强制）
    bool validateStep2(QString *errMsg=nullptr) const; // 协议
    bool passwordBasicOk(const QString &pwd) const;

    // 动效/工具
    void animatePage(QWidget *oldPage, QWidget *newPage, bool forward);
    void pulseDot(QLabel *dot);
    void showInlineMessage(const QString &msg);
    void shake(QWidget *w);
    void fadeAndClose();

    // 密码工具
    void toggleEcho(QLineEdit *le, QToolButton *btn);

};

#endif // REGISTERWGT_H
