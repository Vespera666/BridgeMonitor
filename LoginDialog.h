#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class UserManager;

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(UserManager *um, QWidget *parent = nullptr);

    QString loggedUsername() const;
    QString loggedRole() const;

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    UserManager *m_userManager;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QLabel *m_errorLabel;

    QString m_loggedUser;
    QString m_loggedRole;
};

#endif // LOGINDIALOG_H
