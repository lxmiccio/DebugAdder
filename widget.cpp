#include "widget.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QSplitter>
#include <QTextStream>
#include <QVBoxLayout>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    m_input = new QTextEdit();
    m_input->setLineWrapMode(QTextEdit::NoWrap);

    m_output = new QTextEdit();
    m_output->setLineWrapMode(QTextEdit::NoWrap);

    m_button = new QPushButton("Procedi");
    QObject::connect(m_button, SIGNAL(clicked(bool)), this, SLOT(onButtonClicked()));

    QSplitter* splitter = new QSplitter();
    splitter->addWidget(m_input);
    splitter->addWidget(m_output);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(splitter);
    layout->addWidget(m_button);

    setLayout(layout);
}

Widget::~Widget()
{

}

void Widget::onButtonClicked()
{
    QString text = m_input->toPlainText().replace("\n", "\r\n");

    QFile output("input.txt");

    if(output.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream streamOut(&output);
        streamOut << text;

        output.close();
    }

    QFile file("input.txt");
    QString source;

    if(file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream streamIn(&file);

        while(!streamIn.atEnd())
        {
            QString line = streamIn.readLine();
            source.append(line + "\r\n");

            if(line.startsWith("//----------------------------------------------------------------------------"))
            {
                QString prototype;

                QString beforeParenthesis = streamIn.readLine();
                source.append(prototype + "\r\n");

                QString parenthesis = streamIn.readLine();
                source.append(parenthesis + "\r\n");

                while(!parenthesis.startsWith("{"))
                {
                    if(parenthesis.indexOf("::") != -1)
                    {
                        prototype = parenthesis;
                    }
                    else
                    {
                        beforeParenthesis = parenthesis;
                    }

                    parenthesis = streamIn.readLine();
                    source.append(parenthesis + "\r\n");
                }

                if(prototype == NULL)
                {
                    prototype = beforeParenthesis;
                }

                if(!prototype.startsWith(" "))
                {
                    int spaceIndex = prototype.indexOf(" ");
                    int doublePointIndex = prototype.indexOf("::");
                    int leftParenthesisIndex = prototype.indexOf("(");
                    QStringList parametersName;

                    QString parameters = prototype.mid(leftParenthesisIndex);
                    parameters = parameters.mid(1, parameters.indexOf(")") - 1);

                    if(parameters != "")
                    {
                        foreach(const QString& parameter, parameters.split(","))
                        {
                            bool typeRead = false;
                            bool pointerRead = false;

                            foreach(const QString& parameterName, parameter.split(" "))
                            {
                                if(typeRead && parameterName.indexOf("const") == -1 && parameterName != "")
                                {
                                    if(pointerRead)
                                    {
                                        parametersName << QString("*%1").arg(parameterName);
                                    }
                                    else
                                    {
                                        parametersName << parameterName;
                                    }
                                }

                                if(parameterName.indexOf("*") != -1)
                                {
                                    pointerRead = true;
                                }

                                if(parameterName != "" && parameterName.indexOf("const") == -1 && parameterName != "" && parameterName != "*" && parameterName != "&")
                                {
                                    typeRead = true;
                                }
                            }
                        }
                    }

                    QString returnType(prototype.mid(0, spaceIndex));
                    QString namespaceName(prototype.mid(spaceIndex + 1, doublePointIndex - spaceIndex - 1));
                    QString methodName(prototype.mid(doublePointIndex + 2, leftParenthesisIndex - doublePointIndex - 2));

                    qDebug() << returnType << " " << namespaceName << "::" << methodName;

                    source.append(QString("    /* INSTRUMENTATION FOR LEVEL 2 INTEGRATION TESTS\r\n"));
                    source.append(QString("    qDebug() << \"%1 - %2\";\r\n").arg(namespaceName).arg(methodName));

                    foreach(QString parameterName, parametersName)
                    {
                        source.append(QString("    qDebug() << \"%1 - %2 - %3: \" << %3;\r\n").arg(namespaceName).arg(methodName).arg(parameterName));
                    }

                    source.append("\r\n");



                    //Check return type
                    if(returnType.indexOf("void") == -1 && returnType.indexOf("::") == -1 && returnType != "")
                    {
                        QString returnLine = streamIn.readLine();

                        while(returnLine.indexOf("return ") == -1 && returnLine.indexOf("return(") == -1)
                        {
                            source.append(returnLine + "\r\n");
                            returnLine = streamIn.readLine();
                        }

                        QString returnParameter = returnLine;

                        while(returnParameter.startsWith(" "))
                        {
                            returnParameter = returnParameter.mid(1, returnParameter.length());
                        }

                        returnParameter = returnParameter.left(returnParameter.length() - 1);
                        returnParameter = returnParameter.mid(returnParameter.indexOf(" ") + 1, returnParameter.length());
                        returnParameter.left(returnParameter.length() - 1);

                        source.append(QString("    /* INSTRUMENTATION FOR LEVEL 2 INTEGRATION TESTS\r\n"));
                        source.append(QString("    qDebug() << \"%1 - %2 - %3: \" << %3;\r\n\r\n").arg(namespaceName).arg(methodName).arg(returnParameter));
                        source.append(returnLine + "\r\n");
                    }
                }
            }
        }

        file.close();
    }

    m_output->setText(source);



    if(QFileInfo::exists("input.txt"))
    {
        QFile::remove("input.txt");
    }
}
