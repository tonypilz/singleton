#ifndef OPTIONALVALUETEST_H
#define OPTIONALVALUETEST_H

#include <QObject>
#include <QtTest/QtTest>

class OptionalValueTest : public QObject
{
    Q_OBJECT
public:
    explicit OptionalValueTest(QObject *parent = nullptr);

    struct A{};

signals:

private slots:

    void defaultConstructedValueNotSet();
    void accessingInvalidValueThrows();
    void assignedValueIsValid();
    void ussettingAValueMakesItInvalid();

};

#endif // OPTIONALVALUETEST_H
