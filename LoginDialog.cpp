#include "LoginDialog.h"
#include "RegisterDialog.h"
#include "UserManager.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

LoginDialog::LoginDialog(UserManager *um, QWidget *parent)
    : QDialog(parent)
    , m_userManager(um)
{
    setWindowTitle(QStringLiteral("用户登录"));
    setFixedSize(320, 220);

    // ── 控件 ──
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText(QStringLiteral("请输入用户名"));

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText(QStringLiteral("请输入密码"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->hide();

    auto *registerBtn = new QPushButton(QStringLiteral("注册"), this);
    auto *loginBtn = new QPushButton(QStringLiteral("登录"), this);
    auto *cancelBtn = new QPushButton(QStringLiteral("退出"), this);

    // ── 表单 ──
    auto *formLayout = new QFormLayout;
    formLayout->addRow(QStringLiteral("用户名："), m_usernameEdit);
    formLayout->addRow(QStringLiteral("密  码："), m_passwordEdit);

    // ── 按钮行 ──
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(registerBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(loginBtn);
    btnLayout->addWidget(cancelBtn);

    // ── 顶层 ──
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_errorLabel);
    mainLayout->addLayout(btnLayout);

    // ── 信号 ──
    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(registerBtn, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
}

void LoginDialog::onLoginClicked()
{
    QString user = m_usernameEdit->text().trimmed();
    QString pwd = m_passwordEdit->text();

    if (user.isEmpty() || pwd.isEmpty()) {
        m_errorLabel->setText(QStringLiteral("用户名和密码不能为空"));
        m_errorLabel->show();
        return;
    }

    if (m_userManager->verify(user, pwd)) {
        m_loggedUser = user;
        m_loggedRole = m_userManager->roleOf(user);
        accept();
    } else {
        m_errorLabel->setText(QStringLiteral("用户名或密码错误"));
        m_errorLabel->show();
        m_passwordEdit->clear();
        m_passwordEdit->setFocus();
    }
}

void LoginDialog::onRegisterClicked()
{
    RegisterDialog dlg(m_userManager, this);
    dlg.exec(); // 注册弹窗，用户注册完自动关闭
}

QString LoginDialog::loggedUsername() const
{
    return m_loggedUser;
}

QString LoginDialog::loggedRole() const
{
    return m_loggedRole;
}
