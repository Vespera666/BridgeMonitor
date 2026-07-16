#include "RegisterDialog.h"
#include "UserManager.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

RegisterDialog::RegisterDialog(UserManager *um, QWidget *parent)
    : QDialog(parent), m_userManager(um)
{
    setWindowTitle(QStringLiteral("注册新用户"));
    setFixedSize(320, 230);

    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText(QStringLiteral("请输入用户名"));

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText(QStringLiteral("请输入密码"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    m_confirmEdit = new QLineEdit(this);
    m_confirmEdit->setPlaceholderText(QStringLiteral("请再次输入密码"));
    m_confirmEdit->setEchoMode(QLineEdit::Password);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->hide();

    auto *okBtn     = new QPushButton(QStringLiteral("确认注册"), this);
    auto *cancelBtn = new QPushButton(QStringLiteral("取消"),     this);

    auto *formLayout = new QFormLayout;
    formLayout->addRow(QStringLiteral("用户名："),   m_usernameEdit);
    formLayout->addRow(QStringLiteral("密  码："),   m_passwordEdit);
    formLayout->addRow(QStringLiteral("确认密码："), m_confirmEdit);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_errorLabel);
    mainLayout->addLayout(btnLayout);

    connect(okBtn,     &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_confirmEdit, &QLineEdit::returnPressed, this, &RegisterDialog::onRegisterClicked);
}

void RegisterDialog::onRegisterClicked()
{
    QString user = m_usernameEdit->text().trimmed();
    QString pwd  = m_passwordEdit->text();
    QString cfm  = m_confirmEdit->text();

    if (user.isEmpty() || pwd.isEmpty()) {
        m_errorLabel->setText(QStringLiteral("用户名和密码不能为空"));
        m_errorLabel->show();
        return;
    }

    if (pwd != cfm) {
        m_errorLabel->setText(QStringLiteral("两次输入的密码不一致"));
        m_errorLabel->show();
        return;
    }

    if (!m_userManager->registerUser(user, pwd)) {
        m_errorLabel->setText(QStringLiteral("用户名已被注册"));
        m_errorLabel->show();
        return;
    }

    accept();  // 注册成功
}
