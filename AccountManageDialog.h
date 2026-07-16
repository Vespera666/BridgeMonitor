#ifndef ACCOUNTMANAGEDIALOG_H
#define ACCOUNTMANAGEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTableWidget>

class UserManager;

class AccountManageDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AccountManageDialog(UserManager *um, QWidget *parent = nullptr);

private slots:
    void onPromoteClicked(); // 提升为工程师
    void onDemoteClicked();  // 降为分析师
    void onDeleteClicked();  // 删除用户

private:
    void refreshTable();          // 刷新表格显示
    QString selectedUser() const; // 当前选中的用户名

    UserManager *m_userManager;
    QTableWidget *m_table;
    QLabel *m_infoLabel;
};

#endif // ACCOUNTMANAGEDIALOG_H
