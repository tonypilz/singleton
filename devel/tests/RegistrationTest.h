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
    void leavingTheScopeOfASubSingleInstanceRegistrationDeregistersInstance();

    void SingleInstanceRegistrationAllowsOnlySingleRegistration();
    void singleInstanceSubRegistrationAllowsOnlySingleRegistration();

    void ReplacingInstanceRegistrationReplacesInstanceTemporarily();
    void replacingInstanceSubRegistrationReplacesInstanceTemporarily();

    void registrationsCanBeChanged();
    void registrationsSubCanBeChanged();

    void registerdInstanceWorks();
    void registerdInstanceWorksSub();

    void registerdInstanceWorksWithArgsSub();


};

#endif // REGISTRATIONTEST_H
