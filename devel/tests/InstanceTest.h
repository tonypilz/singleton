#ifndef INSTANCETEST_H
#define INSTANCETEST_H

#include <QObject>
#include <QtTest/QtTest>

class InstanceTest : public QObject
{
    Q_OBJECT
public:
    explicit InstanceTest(QObject *parent = 0);

private:

    class A{};

signals:

private slots:

    void aRegisteredInstanceIsAccessible();
    void anUnregisteredInstanceIsNotAccessible();

    void aDerivedInstanceIsAccessibleWithoutSlicing();

    void gettingNullThrowsWithoutHandler();
    void gettingNullInvokesInstalledUntypeHandler();
    void gettingNullInvokesInstalledTypeHandlerBeforeUntyped();


    void functionWillBeCalledDirectlyIfInstanceDefined();

    void functionWillBeCalledDirectlyIfInstanceUndefined();

    void functionWillBeCalledIfInstanceIsDefined();

    void functionWillBeCalledIfInstanceIsUndefined();

    void functionWillBeCalledOnlyOnceDirectly();
    void functionWillBeCalledOnlyOnceIndirectly();

    void conditionalFunctionWillBeCalledDirectlyIfInstanceDefined();
    void conditionalFunctionWillBeCalledIfInstanceDefined();

    void functionsWithDifferentConditionsWillBeCalledOnInstanceChange();

    void recursiveQueuingWorks();
    void registerForDestructionWorks();

    void instanceRefWorks();

    void instanceBeforeIsAvailableToDefferedOperations();

};

#endif // INSTANCETEST_H
