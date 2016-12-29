#ifndef WIDGET_H
#define WIDGET_H

#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

class Widget : public QWidget
{
    Q_OBJECT

    public:
        Widget(QWidget *parent = 0);
        ~Widget();

    private slots:
        void onButtonClicked();

    private:
        QTextEdit* m_input;
        QTextEdit* m_output;
        QPushButton* m_button;
};

#endif // WIDGET_H
