#ifndef TASKDIALOG_H
#define TASKDIALOG_H
#include <QDialog>
#include <memory.h>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDate>
#include <QLabel>

class TaskDialog : public QDialog{
    Q_OBJECT
public:
    TaskDialog(QDialog* parent = nullptr);
private:
    std::unique_ptr<QLineEdit> task_name;
    std::unique_ptr<QTextEdit> task_description;
    std::unique_ptr<QDateEdit> deadline_input;
    std::unique_ptr<QPushButton> add_button;
    std::unique_ptr<QPushButton> cancel_button;
};


#endif // TASKDIALOG_H
