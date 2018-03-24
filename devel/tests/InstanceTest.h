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
    class Sub{};

signals:

private slots:

    void aRegisteredInstanceIsAccessible();
    void anUnregisteredInstanceIsNotAccessible();
    void anUnregisteredSubInstanceIsNotAccessible();
    void aRegisteredSubInstanceIsAccessible();

    void aDerivedInstanceIsAccessibleWithoutSlicing();
    void aDerivedSubInstanceIsAccessibleWithoutSlicing();

    void gettingNullThrowsWithoutHandler();
    void gettingNullInvokesInstalledUntypeHandler();
    void gettingNullInvokesInstalledTypeHandlerBeforeUntyped();


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

#endif // INSTANCETEST_H
