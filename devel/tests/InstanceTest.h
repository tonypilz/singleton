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

    void gettingNullThrowsWithoutHandler();
    void gettingNullInvokesInstalledUntypeHandler();
    void gettingNullInvokesInstalledTypeHandlerBeforeUntyped();


    void functionWillBeCalledDirectlyIfInstanceDefined();

    void unavailableFunctionWillNotBeCalledDirectlyIfInstanceUndefined();

    void functionWillBeCalledIfInstanceIsDefined();

    void undefinedFunctionWillBeCalledIfInstanceGetsUndefined();

    void functionWillBeCalledOnlyOnceDirectly();
    void functionWillBeCalledOnlyOnceIndirectly();

    void conditionalFunctionWillBeCalledDirectlyIfInstanceDefined();
    void conditionalFunctionWillBeCalledIfInstanceDefined();

    void functionsWithDifferentConditionsWillBeCalledOnInstanceChange();

    void recursiveQueuingWorks();
    void registerForDestructionWorks();

    void instanceRefWorks();

    void instanceBeforeIsAvailableToDefferedOperations();

    void operatorNewNotUsedOnFinishedFunctions();

    void registeredInstanceAccessDoesNotInvokeOperatorNew();
    void unregisteredInstanceAccessDoesNotInvokeOperatorNew();

};

#endif // INSTANCETEST_H
