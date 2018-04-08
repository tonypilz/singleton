#ifndef REGISTRATIONTEST_H
#define REGISTRATIONTEST_H

#include <QObject>
#include <QtTest/QtTest>

class RegistrationTest : public QObject
{
    Q_OBJECT
public:
    explicit RegistrationTest(QObject *parent = 0);

signals:

private slots:

    void leavingTheScopeOfASingleInstanceRegistrationDeregistersInstance();

    void SingleInstanceRegistrationAllowsOnlySingleRegistration();

    void ReplacingInstanceRegistrationReplacesInstanceTemporarily();

    void registrationsCanBeChanged();

    void InstanceBasicallyWorks();

    void InstanceBasicallyWorksWithBaseclass();

    void InstanceBasicallyWorksWithArgs();

    void TestInstanceBasicallyWorks();


    void InstanceWorksWithStdMap();

    void privateConstructorsCanBeUsed();
    void privateConstructorsCanBeUsedWithMacro();


};

#endif // REGISTRATIONTEST_H
