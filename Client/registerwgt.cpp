#include "registerwgt.h"
#include "ui_registerwgt.h"

#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <QProgressBar>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QStackedWidget>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QFileDialog>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QEasingCurve>
#include <QTimer>
#include <QPainter>

// 你的网络客户端
#include "shoppingclient.h"

RegisterWgt::RegisterWgt(ShoppingClient *client, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::RegisterWgt),
    m_client(client)
{
    ui->setupUi(this);
    initUi();
    initConnections();
    applyStepStyle(0, true);
    updateProgress();
}

void RegisterWgt::paintEvent(QPaintEvent *ev) {
    QWidget::paintEvent(ev);
    QPainter p(this);
    QPixmap bg(":/images/login/bg_register_wizard.jpg");
    if (!bg.isNull()) {
        p.drawPixmap(rect(), bg); // 自动拉伸填满
    }
    // else {
    //     // 兜底渐变
    //     QLinearGradient g(rect().topLeft(), rect().bottomRight());
    //     g.setColorAt(0, QColor("#efeafd"));
    //     g.setColorAt(1, QColor("#c6b4ef"));
    //     p.fillRect(rect(), g);
    // }
    // 可选叠加一层很淡的面纱：
    // p.fillRect(rect(), QColor(255,255,255,40));
}

RegisterWgt::~RegisterWgt()
{
    delete ui;
}

/* ================= 初始化 ================= */

void RegisterWgt::initUi()
{
    // 给三个圆点打上样式 class
    for (QLabel *dot : {ui->dot1, ui->dot2, ui->dot3}) {
        if (dot) {
            dot->setProperty("class", "stepDot");
            dot->style()->unpolish(dot);
            dot->style()->polish(dot);
        }
    }

    // 第一个高亮
    ui->dot1->setProperty("current", true);
    ui->dot1->style()->unpolish(ui->dot1);
    ui->dot1->style()->polish(ui->dot1);

    // 密码字段校验器（可选：允许字母数字特殊符号）
    QRegularExpression userRe("^[A-Za-z0-9_\\-]{0,30}$");
    ui->line_name->setValidator(new QRegularExpressionValidator(userRe, this));

    // 邮箱基本格式即时提示（最终校验仍在 validateStep0）
    // 给强度标签初值
    ui->lab_pwdStrengthText->setText("弱");

    // 初始按钮状态
    ui->btn_back->setEnabled(false);
    ui->btn_next->setEnabled(true);
    ui->btn_register->setEnabled(false); // 只有第三步且协议勾选后才启用

    // 让“完成注册”按钮使用逻辑名称：btn_register（UI 已是）
    // 隐藏消息初始
    ui->lab_msg->clear();
}

void RegisterWgt::initConnections()
{
    connect(ui->btn_next,      &QPushButton::clicked, this, &RegisterWgt::onNext);
    connect(ui->btn_back,      &QPushButton::clicked, this, &RegisterWgt::onBack);
    connect(ui->btn_register,  &QPushButton::clicked, this, &RegisterWgt::onRegister);

    connect(ui->btn_showPwd1,  &QToolButton::clicked, this, &RegisterWgt::onTogglePwd1);
    connect(ui->btn_showPwd2,  &QToolButton::clicked, this, &RegisterWgt::onTogglePwd2);

    connect(ui->line_password, &QLineEdit::textChanged, this, &RegisterWgt::onPasswordChanged);
    connect(ui->line_repassword,&QLineEdit::textChanged,this, &RegisterWgt::onPasswordChanged);
    connect(ui->line_name,     &QLineEdit::textChanged, this, &RegisterWgt::onAnyFieldChanged);
    connect(ui->line_email,    &QLineEdit::textChanged, this, &RegisterWgt::onAnyFieldChanged);

    connect(ui->combo_gender,  &QComboBox::currentTextChanged, this, &RegisterWgt::onAnyFieldChanged);
    connect(ui->chk_newsletter,&QCheckBox::stateChanged, this, &RegisterWgt::onAnyFieldChanged);
    connect(ui->chk_agree,     &QCheckBox::stateChanged, this, &RegisterWgt::onAnyFieldChanged);

    connect(ui->btn_selectAvatar, &QPushButton::clicked, this, &RegisterWgt::onSelectAvatar);
}

/* ================= 步骤流程 ================= */

void RegisterWgt::onNext()
{
    QString err;
    if (m_currentStep == 0 && !validateStep0(&err)) { showInlineMessage(err); shake(ui->centralCard); return; }
    if (m_currentStep == 1 && !validateStep1(&err)) { showInlineMessage(err); shake(ui->centralCard); return; }

    if (m_currentStep < m_totalSteps - 1) {
        goToStep(m_currentStep + 1, true);
    }
}

void RegisterWgt::onBack()
{
    if (m_currentStep > 0) {
        goToStep(m_currentStep - 1, false);
    }
}

void RegisterWgt::goToStep(int step, bool forward)
{
    if (step < 0 || step >= m_totalSteps) return;

    QWidget *oldPage = ui->stepsStack->currentWidget();
    QWidget *newPage = ui->stepsStack->widget(step);

    animatePage(oldPage, newPage, forward);
    ui->stepsStack->setCurrentWidget(newPage);

    m_currentStep = step;
    applyStepStyle(step);
    updateProgress();

    // 按钮状态
    ui->btn_back->setEnabled(step > 0);
    ui->btn_next->setEnabled(step < m_totalSteps - 1);
    ui->btn_register->setEnabled(step == m_totalSteps - 1 && ui->chk_agree->isChecked());

    if (step == 2) { // 填充确认
        fillSummary();
    }
    showInlineMessage(QString());
}

void RegisterWgt::applyStepStyle(int step, bool first)
{
    QList<QLabel*> dots { ui->dot1, ui->dot2, ui->dot3 };
    for (int i=0;i<dots.size();++i) {
        dots[i]->setProperty("current", i==step);
        dots[i]->style()->unpolish(dots[i]);
        dots[i]->style()->polish(dots[i]);
    }
    if (!first) pulseDot(dots[step]);
}

void RegisterWgt::updateProgress()
{
    // 百分比：0→0%， 1→50%， 2→100%
    int percent = int( (m_currentStep / double(m_totalSteps - 1)) * 100.0 );
    ui->progressSteps->setValue(percent);
}

void RegisterWgt::fillSummary()
{
    ui->lab_summary_user->setText(ui->line_name->text());
    ui->lab_summary_email->setText(ui->line_email->text());
    ui->lab_summary_gender->setText(ui->combo_gender->currentText());
    ui->lab_summary_address->setText(ui->line_address->text().isEmpty() ? "(未填写)" : ui->line_address->text());
    ui->lab_summary_newsletter->setText(ui->chk_newsletter->isChecked() ? "是" : "否");
}

void RegisterWgt::updateFinalButtonEnabled()
{
    if (m_currentStep == 2) {
        ui->btn_register->setEnabled(ui->chk_agree->isChecked());
    }
}

/* ================= 校验 ================= */

bool RegisterWgt::passwordBasicOk(const QString &pwd) const
{
    if (pwd.length() < 6) return false;
    bool hasAlpha=false, hasDigit=false;
    for (auto ch : pwd) {
        if (ch.isLetter()) hasAlpha = true;
        if (ch.isDigit())  hasDigit = true;
    }
    return hasAlpha && hasDigit;
}

bool RegisterWgt::validateStep0(QString *errMsg) const
{
    QString u = ui->line_name->text().trimmed();
    QString e = ui->line_email->text().trimmed();
    QString p = ui->line_password->text();
    QString c = ui->line_repassword->text();

    if (u.size() < 3 || u.size() > 20) {
        if (errMsg) *errMsg = "用户名长度应为 3~20。";
        return false;
    }
    QRegularExpression mailRe(R"(^[A-Za-z0-9._%+\-]+@[A-Za-z0-9.\-]+\.[A-Za-z]{2,}$)");
    if (!mailRe.match(e).hasMatch()) {
        if (errMsg) *errMsg = "邮箱格式无效。";
        return false;
    }
    if (!passwordBasicOk(p)) {
        if (errMsg) *errMsg = "密码至少6位，且包含字母和数字。";
        return false;
    }
    if (p != c) {
        if (errMsg) *errMsg = "两次输入的密码不一致。";
        return false;
    }
    return true;
}

bool RegisterWgt::validateStep1(QString *errMsg) const
{
    Q_UNUSED(errMsg);
    // 资料页本例不做强校验，可加逻辑
    return true;
}

bool RegisterWgt::validateStep2(QString *errMsg) const
{
    if (!ui->chk_agree->isChecked()) {
        if (errMsg) *errMsg = "请勾选用户协议。";
        return false;
    }
    return true;
}

/* ================= 密码强度 ================= */

void RegisterWgt::updatePasswordStrength(const QString &pwd)
{
    int score = 0;
    if (pwd.length() >= 6) score += 25;
    if (pwd.length() >= 10) score += 15;
    if (pwd.contains(QRegularExpression("[A-Z]"))) score += 15;
    if (pwd.contains(QRegularExpression("[a-z]"))) score += 15;
    if (pwd.contains(QRegularExpression("[0-9]"))) score += 15;
    if (pwd.contains(QRegularExpression("[^A-Za-z0-9]"))) score += 15;
    if (score > 100) score = 100;

    ui->pwdStrength->setValue(score);

    QString label;
    if (score < 30) label = "弱";
    else if (score < 60) label = "中";
    else if (score < 85) label = "良";
    else label = "强";
    ui->lab_pwdStrengthText->setText(label);
}

/* ================= 槽 ================= */

void RegisterWgt::onPasswordChanged(const QString &txt)
{
    Q_UNUSED(txt);
    updatePasswordStrength(ui->line_password->text());
    if (m_currentStep == 0) {
        QString err;
        validateStep0(&err);
        showInlineMessage(err);
    }
}

void RegisterWgt::onAnyFieldChanged()
{
    if (m_currentStep == 0) {
        QString err;
        validateStep0(&err);
        showInlineMessage(err);
    } else if (m_currentStep == 2) {
        QString err;
        validateStep2(&err);
        showInlineMessage(err);
    } else {
        showInlineMessage(QString());
    }
    updateFinalButtonEnabled();
}

void RegisterWgt::onTogglePwd1()
{
    toggleEcho(ui->line_password, ui->btn_showPwd1);
}

void RegisterWgt::onTogglePwd2()
{
    toggleEcho(ui->line_repassword, ui->btn_showPwd2);
}

void RegisterWgt::onSelectAvatar()
{
    QString path = QFileDialog::getOpenFileName(this, tr("选择头像"), QString(), tr("图片 (*.png *.jpg *.jpeg *.bmp)"));
    if (path.isEmpty()) return;
    QPixmap pix(path);
    if (pix.isNull()) return;
    m_avatarPixmap = pix.scaled(140,140, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->lab_avatarPreview->setPixmap(m_avatarPixmap);

    // 轻微放大动画
    auto *anim = new QPropertyAnimation(ui->lab_avatarPreview, "geometry", this);
    QRect g = ui->lab_avatarPreview->geometry();
    anim->setDuration(300);
    anim->setStartValue(g);
    anim->setKeyValueAt(0.5, QRect(g.x()-4, g.y()-4, g.width()+8, g.height()+8));
    anim->setEndValue(g);
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void RegisterWgt::onRegister()
{
    QString err;
    if (!validateStep2(&err)) {
        showInlineMessage(err);
        shake(ui->centralCard);
        return;
    }
    // 合并之前两步校验，防止直接跳到最后
    if (!validateStep0(&err)) { showInlineMessage("账号信息无效，请返回修改。"); return; }

    if (!m_client || !m_client->getConnected()) {
        QMessageBox::warning(this,"错误","未连接到网络！");
        return;
    }

    if (QMessageBox::question(this,"确认","确认提交注册？",
                              QMessageBox::Yes|QMessageBox::Cancel,
                              QMessageBox::Yes) != QMessageBox::Yes)
        return;

    // 收集数据
    QString name     = ui->line_name->text().trimmed();
    QString password = ui->line_password->text();
    QString email    = ui->line_email->text().trimmed(); // 也可以发送
    QString gender   = ui->combo_gender->currentText();
    QString address  = ui->line_address->text().trimmed();
    bool    subMail  = ui->chk_newsletter->isChecked();

    QJsonObject obj;
    obj.insert("user_name", name);
    obj.insert("user_password", password);
    obj.insert("user_address", address);
    obj.insert("user_gender", gender);
    obj.insert("user_email", email);
    obj.insert("newsletter", subMail);

    QByteArray data = m_client->sendCHTTPMsg("10102", obj);
    QString flag = m_client->parseHead(data);

    if (flag.startsWith('1')) {
        QMessageBox::information(this,"成功","注册成功！");
        fadeAndClose();
    } else if (flag.startsWith('2')) {
        QMessageBox::warning(this,"失败","客户端错误: " + flag.mid(1));
    } else if (flag.startsWith('3')) {
        QMessageBox::warning(this,"失败","服务端错误: " + flag.mid(1));
    } else {
        QMessageBox::warning(this,"失败","未知状态码: " + flag);
    }
}

/* ================= 工具/动效 ================= */

void RegisterWgt::toggleEcho(QLineEdit *le, QToolButton *btn)
{
    bool isPwd = le->echoMode() == QLineEdit::Password;
    le->setEchoMode(isPwd ? QLineEdit::Normal : QLineEdit::Password);
    btn->setText(isPwd ? "🙈" : "👁");
}

void RegisterWgt::animatePage(QWidget *oldPage, QWidget *newPage, bool forward)
{
    if (!oldPage || !newPage || oldPage==newPage) return;
    int w = ui->stepsStack->width();

    QPoint oldStart(0,0);
    QPoint oldEnd (forward ? -w : w, 0);
    QPoint newStart(forward ? w : -w, 0);
    QPoint newEnd(0,0);

    newPage->move(newStart);

    auto *animOut = new QPropertyAnimation(oldPage, "pos", this);
    animOut->setDuration(420);
    animOut->setStartValue(oldStart);
    animOut->setEndValue(oldEnd);
    animOut->setEasingCurve(QEasingCurve::OutCubic);

    auto *animIn = new QPropertyAnimation(newPage, "pos", this);
    animIn->setDuration(420);
    animIn->setStartValue(newStart);
    animIn->setEndValue(newEnd);
    animIn->setEasingCurve(QEasingCurve::OutCubic);

    auto *group = new QParallelAnimationGroup(this);
    group->addAnimation(animOut);
    group->addAnimation(animIn);
    connect(group, &QParallelAnimationGroup::finished, this, [=]{
        newPage->move(0,0);
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void RegisterWgt::pulseDot(QLabel *dot)
{
    if (!dot) return;
    auto *eff = new QGraphicsOpacityEffect(dot);
    dot->setGraphicsEffect(eff);
    auto *anim = new QPropertyAnimation(eff, "opacity", this);
    anim->setDuration(500);
    anim->setKeyValueAt(0.0, 0.3);
    anim->setKeyValueAt(0.5, 1.0);
    anim->setKeyValueAt(1.0, 1.0);
    connect(anim,&QPropertyAnimation::finished,this,[=]{ dot->setGraphicsEffect(nullptr); });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void RegisterWgt::showInlineMessage(const QString &msg)
{
    ui->lab_msg->setText(msg);
}

void RegisterWgt::shake(QWidget *w)
{
    if (!w) return;
    auto *anim = new QPropertyAnimation(w, "pos", this);
    QPoint base = w->pos();
    anim->setDuration(300);
    anim->setKeyValueAt(0.0, base);
    anim->setKeyValueAt(0.15, base + QPoint(-8,0));
    anim->setKeyValueAt(0.30, base + QPoint(8,0));
    anim->setKeyValueAt(0.45, base + QPoint(-5,0));
    anim->setKeyValueAt(0.60, base + QPoint(5,0));
    anim->setKeyValueAt(0.75, base + QPoint(-2,0));
    anim->setKeyValueAt(1.0, base);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void RegisterWgt::fadeAndClose()
{
    auto *eff = new QGraphicsOpacityEffect(ui->centralCard);
    ui->centralCard->setGraphicsEffect(eff);
    auto *anim = new QPropertyAnimation(eff,"opacity",this);
    anim->setDuration(450);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::InOutCubic);
    connect(anim,&QPropertyAnimation::finished,this,[this]{ close(); });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

/* ================= resize：可扩展（目前不用放眼睛按钮，因为 UI 已自带按钮） ================= */
void RegisterWgt::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // 若想随窗口改动再自适应额外元素，可在这里加逻辑
}
