#include "authorization.h"

LoginWidget::LoginWidget(QWidget *parent) : QWidget(parent),
    usernameEdit(std::make_unique<QLineEdit>(this)),
    passwordEdit(std::make_unique<QLineEdit>(this))
{
    passwordEdit->setEchoMode(QLineEdit::Password);

    QPushButton* login = new QPushButton("Login", this);
    QObject::connect(login, &QPushButton::clicked, this, [this](){
        UserModel user;
        user.username = this->usernameEdit->text().toStdString();
        user.password = this->passwordEdit->text().toStdString();

        emit this->loginRequested(user);
    });


    QFormLayout* layout = new QFormLayout(this);
    layout->addRow({"Username:"}, usernameEdit.get());
    layout->addRow({"Password:"}, passwordEdit.get());
    layout->addRow(login);

    this->setStyleSheet(R"(
    QWidget {
        background-color: #2d2d2d;
        color: #e0e0e0;
        font-family: 'Segoe UI', Arial, sans-serif;
    }

    QLineEdit {
        background-color: #3d3d3d;
        border: 1px solid #4d4d4d;
        border-radius: 4px;
        padding: 8px;
        color: #e0e0e0;
        selection-background-color: #4a90e2;
    }

    QLineEdit:focus {
        border: 1px solid #4a90e2;
    }

    QPushButton {
        background-color: #4a90e2;
        color: white;
        border: none;
        border-radius: 4px;
        padding: 8px 16px;
        min-width: 80px;
    }

    QPushButton:hover {
        background-color: #5aa0f2;
    }

    QPushButton:pressed {
        background-color: #3a80d2;
    }
)");
}

RegistrationWidget::RegistrationWidget(QWidget *parent):QWidget(parent),
    usernameEdit(std::make_unique<QLineEdit>(this)),
    passwordEdit(std::make_unique<QLineEdit>(this)),
    emailEdit(std::make_unique<QLineEdit>(this)),
    fullnameEdit(std::make_unique<QLineEdit>(this))
{
    passwordEdit->setEchoMode(QLineEdit::Password);

    QPushButton* register_ = new QPushButton("Register", this);
    QObject::connect(register_, &QPushButton::clicked, this, [this](){
        UserModel user;
        user.username = this->usernameEdit->text().toStdString();
        user.password = this->passwordEdit->text().toStdString();
        user.email = this->emailEdit->text().toStdString();
        user.full_name = this->fullnameEdit->text().toStdString();

        emit this->RegistrationRequested(user);
    });


    QFormLayout* layout = new QFormLayout(this);
    layout->addRow({"Username:"}, usernameEdit.get());
    layout->addRow({"Password:"}, passwordEdit.get());
    layout->addRow({"Email:"}, emailEdit.get());
    layout->addRow({"Full name:"}, fullnameEdit.get());
    layout->addRow(register_);

    this->setStyleSheet(R"(
    QWidget {
        background-color: #2d2d2d;
        color: #e0e0e0;
        font-family: 'Segoe UI', Arial, sans-serif;
    }

    QLineEdit {
        background-color: #3d3d3d;
        border: 1px solid #4d4d4d;
        border-radius: 4px;
        padding: 8px;
        color: #e0e0e0;
        selection-background-color: #4a90e2;
    }

    QLineEdit:focus {
        border: 1px solid #4a90e2;
    }

    QPushButton {
        background-color: #4a90e2;
        color: white;
        border: none;
        border-radius: 4px;
        padding: 8px 16px;
        min-width: 80px;
    }

    QPushButton:hover {
        background-color: #5aa0f2;
    }

    QPushButton:pressed {
        background-color: #3a80d2;
    }
)");
}

AuthorizationWidget::AuthorizationWidget(QWidget *parent): QWidget(parent)
{
    login = new LoginWidget(this);
    registration = new RegistrationWidget(this);

    setupUI();
}

void AuthorizationWidget::setupUI()
{
    stack = new QStackedWidget(this);
    stack->addWidget(login);
    stack->addWidget(registration);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(stack);

    // Кнопки переключения между формами
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *loginBtn = new QPushButton("Login", this);
    QPushButton *registerBtn = new QPushButton("Register", this);

    connect(loginBtn, &QPushButton::clicked, [this]() {
        stack->setCurrentIndex(0);
    });

    connect(registerBtn, &QPushButton::clicked, [this]() {
        stack->setCurrentIndex(1);
    });

    connect(login, &LoginWidget::loginRequested, this, &AuthorizationWidget::LoginReq);
    connect(registration, &RegistrationWidget::RegistrationRequested, this, &AuthorizationWidget::RegReq);

    btnLayout->addWidget(loginBtn);
    btnLayout->addWidget(registerBtn);
    layout->addLayout(btnLayout);
}
