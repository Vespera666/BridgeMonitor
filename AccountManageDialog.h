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
    void onPromoteClicked();
    void onDemoteClicked();
    void onDeleteClicked();

private:
    void refreshTable();
    QString selectedUser() const;

    UserManager *m_userManager;
    QTableWidget *m_table;
    QLabel *m_infoLabel;
};

#endif // ACCOUNTMANAGEDIALOG_H
