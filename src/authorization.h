#ifndef AUTHORIZATION_H
#define AUTHORIZATION_H

#include <QWidget>
#include <memory>
#include <QPushButton>
#include <QLineEdit>
#include "models.h"
#include <QFormLayout>
#include <QStackedWidget>
#include <QMessageBox>

class LoginWidget;
class RegistrationWidget;
class AuthorizationWidget;


class AuthorizationWidget : public QWidget{
    Q_OBJECT
public:
    explicit AuthorizationWidget(QWidget* parent = nullptr);

signals:
    void LoginReq(UserModel user);
    void RegReq(UserModel user);

private:
    LoginWidget* login;
    RegistrationWidget* registration;
    QStackedWidget* stack;

    void setupUI();
};

class LoginWidget : public QWidget{
    Q_OBJECT
public:
    explicit LoginWidget(QWidget* parent = nullptr);

signals:
    void loginRequested(UserModel userInf);

private:
    std::unique_ptr<QLineEdit> usernameEdit;
    std::unique_ptr<QLineEdit> passwordEdit;
};

class RegistrationWidget : public QWidget{
    Q_OBJECT
public:
    explicit RegistrationWidget(QWidget* parent = nullptr);

signals:
    void RegistrationRequested(UserModel userInf);

private:
    std::unique_ptr<QLineEdit> usernameEdit;
    std::unique_ptr<QLineEdit> passwordEdit;
    std::unique_ptr<QLineEdit> emailEdit;
    std::unique_ptr<QLineEdit> fullnameEdit;
};




#endif // AUTHORIZATION_H
