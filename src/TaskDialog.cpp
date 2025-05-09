#include "TaskDialog.h"


TaskDialog::TaskDialog(QDialog *parent) : QDialog(parent),
    task_name(std::make_unique<QLineEdit>()),
    task_description(std::make_unique<QTextEdit>()),
    deadline_input(std::make_unique<QDateEdit>()),
    add_button(std::make_unique<QPushButton>()),
    cancel_button(std::make_unique<QPushButton>())
{
    QVBoxLayout* layout = new QVBoxLayout;

    task_name->setPlaceholderText("Описание задачи...");

    deadline_input->setDisplayFormat("dd.MM.yyyy");
    deadline_input->setDate(QDate::currentDate());
    deadline_input->setCalendarPopup(true);

    ///Buttons
    QHBoxLayout* button_layout = new QHBoxLayout;
    add_button->setText("Добавить");
    QObject::connect(add_button.get(), &QPushButton::clicked, this, &TaskDialog::accept);
    cancel_button->setText("Отмена");
    QObject::connect(cancel_button.get(), &QPushButton::clicked, this, &TaskDialog::reject);

    button_layout->addWidget(add_button.get());
    button_layout->addWidget(cancel_button.get());

    layout->addWidget(new QLabel("Название:"));
    layout->addWidget(this->task_name.get());
    layout->addWidget(new QLabel("Описание:"));
    layout->addWidget(this->task_description.get());
    layout->addWidget(new QLabel("Срок выполнения"));
    layout->addWidget(this->deadline_input.get());
    layout->addLayout(button_layout);


    this->setLayout(layout);
}
