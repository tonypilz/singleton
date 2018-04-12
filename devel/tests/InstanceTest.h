#ifndef INSTANCETEST_H
#define INSTANCETEST_H

#include <QObject>
#include <QtTest/QtTest>

class InstanceTest : public QObject
{
    Q_OBJECT
public:
    explicit InstanceTest(QObject *parent = 0);

signals:

private slots:

    void aRegisteredInstanceIsAccessible();
    void anUnregisteredInstanceIsNotAccessible();

    void aDerivedInstanceIsAccessibleWithoutSlicing();

    void gettingNullInvokesCustomHandler();


    void functionWillBeCalledDirectlyIfInstanceDefined();

    void unavailableFunctionWillNotBeCalledDirectlyIfInstanceUndefined();

    void functionWillBeCalledIfInstanceIsDefined();

    void undefinedFunctionWillBeCalledIfInstanceGetsUndefined();

    void ifAvailable_willBeCalledDirectlyOnce();
    void ifAvailable_willBeCalledIndirectlyOnce();
    void becomesUnavailable_willNotBeCalledDirectlyOnce();
    void becomesUnavailable_willBeCalledIndirectlyOnce();

    void recursiveQueuingWorks();
    void registerForDestructionWorks();

    void instanceRefWorks();

    void operatorNewNotUsedOnFinishedFunctions();

    void registeredInstanceAccessDoesNotInvokeOperatorNew();
    void unregisteredInstanceAccessDoesNotInvokeOperatorNew();

};

#endif // INSTANCETEST_H
