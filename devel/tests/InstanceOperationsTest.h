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

    void functionWillBeCalledDirectlyIfInstanceUndefined();
    void functionWillBeCalledDirectlyIfSubInstanceUndefined();

    void functionWillBeCalledIfInstanceIsDefined();
    void functionWillBeCalledIfSubInstanceIsDefined();

    void functionWillBeCalledIfInstanceIsUndefined();
    void functionWillBeCalledIfSubInstanceIsUndefined();

    void functionWillBeCalledOnlyOnceDirectly();
    void functionWillBeCalledOnlyOnceIndirectly();

    void conditionalFunctionWillBeCalledDirectlyIfInstanceDefined();
    void conditionalFunctionWillBeCalledIfInstanceDefined();

    void conditionalFunctionWillBeCalledDirectlyIfSubInstanceDefined();
    void conditionalFunctionWillBeCalledIfSubInstanceDefined();

    void functionsWithDifferentConditionsWillBeCalledOnSubInstanceChange();

    void recursiveQueuingWorks();
    void registerForDestructionWorks();

};

#endif // INSTANCECALLTEST_H
