#include "AccountManageDialog.h"
#include "StyleConstants.h"
#include "UserManager.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

AccountManageDialog::AccountManageDialog(UserManager *um, QWidget *parent)
    : QDialog(parent)
    , m_userManager(um)
{
    setWindowTitle(QStringLiteral("账号管理"));
    setMinimumSize(450, 350);


    m_table = new QTableWidget(0, 2, this);
    m_table->setHorizontalHeaderLabels({QStringLiteral("用户名"), QStringLiteral("角色")});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);

    m_infoLabel = new QLabel(this);

    auto *promoteBtn = new QPushButton(QStringLiteral("提升为工程师"), this);
    auto *demoteBtn = new QPushButton(QStringLiteral("降为分析师"), this);
    auto *deleteBtn = new QPushButton(QStringLiteral("删除用户"), this);
    auto *closeBtn = new QPushButton(QStringLiteral("关闭"), this);

    // 按钮样式
    StyleConstants::applyCssClass(deleteBtn, "btn-danger");
    StyleConstants::applyCssClass(closeBtn, "btn-secondary");

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(promoteBtn);
    btnLayout->addWidget(demoteBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_table);
    mainLayout->addWidget(m_infoLabel);
    mainLayout->addLayout(btnLayout);


    connect(promoteBtn, &QPushButton::clicked, this, &AccountManageDialog::onPromoteClicked);
    connect(demoteBtn, &QPushButton::clicked, this, &AccountManageDialog::onDemoteClicked);
    connect(deleteBtn, &QPushButton::clicked, this, &AccountManageDialog::onDeleteClicked);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    refreshTable();
}

void AccountManageDialog::refreshTable()
{
    QStringList users = m_userManager->allUsernames();
    m_table->setRowCount(users.size());

    for (int i = 0; i < users.size(); i++) {
        m_table->setItem(i, 0, new QTableWidgetItem(users[i]));
        m_table->setItem(i, 1, new QTableWidgetItem(m_userManager->roleOfUser(users[i])));
    }
    m_infoLabel->setText(QStringLiteral("共 %1 个用户").arg(users.size()));
}

QString AccountManageDialog::selectedUser() const
{
    int row = m_table->currentRow();
    if (row < 0)
        return {};
    return m_table->item(row, 0)->text();
}

void AccountManageDialog::onPromoteClicked()
{
    QString user = selectedUser();
    if (user.isEmpty())
        return;

    m_userManager->changeRole(user, "engineer");
    refreshTable();
}

void AccountManageDialog::onDemoteClicked()
{
    QString user = selectedUser();
    if (user.isEmpty())
        return;


    if (m_userManager->roleOfUser(user) == "admin") {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("不能降级管理员"));
        return;
    }

    m_userManager->changeRole(user, "analyst");
    refreshTable();
}

void AccountManageDialog::onDeleteClicked()
{
    QString user = selectedUser();
    if (user.isEmpty())
        return;


    if (m_userManager->roleOfUser(user) == "admin") {
        QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("不能删除管理员账号"));
        return;
    }

    int ret = QMessageBox::question(this,
                                    QStringLiteral("确认"),
                                    QStringLiteral("确定要删除用户 \"%1\" 吗？").arg(user));
    if (ret == QMessageBox::Yes) {
        m_userManager->deleteUser(user);
        refreshTable();
    }
}
