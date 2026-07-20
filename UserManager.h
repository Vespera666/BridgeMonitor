#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QMap>
#include <QString>
#include <QStringList>


struct UserInfo
{
    QString password;
    QString role;
};

class UserManager
{
public:
    explicit UserManager(const QString &filePath);

    bool verify(const QString &username, const QString &password) const;

    bool registerUser(const QString &username, const QString &password);

    QString roleOf(const QString &username) const;

    QStringList allUsernames() const;
    QString roleOfUser(const QString &username) const;

    bool deleteUser(const QString &username);

    void changeRole(const QString &username, const QString &newRole);

private:
    void load();
    void save();

    QString m_filePath;
    QMap<QString, UserInfo> m_users;
};

#endif // USERMANAGER_H
