#ifndef INSTANCECALLTEST_H
#define INSTANCECALLTEST_H

#include <QObject>
#include <QtTest/QtTest>

class InstanceOperationsTest : public QObject
{
    Q_OBJECT
public:
    explicit InstanceOperationsTest(QObject *parent = 0);

signals:

private slots:

    void functionWillBeCalledDirectlyIfInstanceDefined();
    void functionWillBeCalledDirectlyIfSubInstanceDefined();

    void functionWillBeCalledIfInstanceIsDefined();
    void functionWillBeCalledIfSubInstanceIsDefined();

    void functionWillBeCalledOnlyOnceDirectly();
    void functionWillBeCalledOnlyOnceIndirectly();

    void conditionalFunctionWillBeCalledDirectlyIfInstanceDefined();
    void conditionalFunctionWillBeCalledIfInstanceDefined();

    void conditionalFunctionWillBeCalledDirectlyIfSubInstanceDefined();
    void conditionalFunctionWillBeCalledIfSubInstanceDefined();

    void functionsWithDifferentConditionsWillBeCalledOnSubInstanceChange();

    void recursiveQueuingWorks();

};

#endif // INSTANCECALLTEST_H
