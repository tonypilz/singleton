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
    void anUnregisteredSubInstanceIsNotAccessible();
    void aRegisteredSubInstanceIsAccessible();

    void aDerivedInstanceIsAccessibleWithoutSlicing();
    void aDerivedSubInstanceIsAccessibleWithoutSlicing();

    void gettingNullThrowsWithoutHandler();
    void gettingNullInvokesInstalledUntypeHandler();
    void gettingNullInvokesInstalledTypeHandlerBeforeUntyped();


};

#endif // INSTANCETEST_H
