#ifndef CONDITIONALSINGLESHOTOPERATIONSTEST_H
#define CONDITIONALSINGLESHOTOPERATIONSTEST_H

#include <QObject>
#include <QtTest/QtTest>

class ConditionalSingleShotOperationsTest : public QObject
{
    Q_OBJECT
public:
    explicit ConditionalSingleShotOperationsTest(QObject *parent = nullptr);

    struct A{};

signals:

private slots:

    void argumentIsPassedToEachElement();
    void falseConditionsAreRexecuted();
    void trueConditionsAreExecutedOnce();
    void recursiveExecutionWorks();
    void recursiveExecutionWorks1();



};

#endif // CONDITIONALSINGLESHOTOPERATIONSTEST_H
