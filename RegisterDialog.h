#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>

class UserManager;

class RegisterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RegisterDialog(UserManager *um, QWidget *parent = nullptr);

private slots:
    void onRegisterClicked();

private:
    UserManager *m_userManager;
    QLineEdit   *m_usernameEdit;
    QLineEdit   *m_passwordEdit;
    QLineEdit   *m_confirmEdit;
    QLabel      *m_errorLabel;
};

#endif // REGISTERDIALOG_H
