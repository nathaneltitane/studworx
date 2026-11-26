#include "sw_global.h"
#include "sw_tests.h"
#include "sw_string.h"

void swTests::TextCaseChangesLetters()
{
	const QString Text = QStringLiteral("mvf-01d SANDSTORM fighter. lego® group! name@mail.com");

	QCOMPARE(swChangeTextCase(Text, swTextCase::Upper), QStringLiteral("MVF-01D SANDSTORM FIGHTER. LEGO® GROUP! NAME@MAIL.COM"));
	QCOMPARE(swChangeTextCase(Text, swTextCase::Lower), QStringLiteral("mvf-01d sandstorm fighter. lego® group! name@mail.com"));
	QCOMPARE(swChangeTextCase(Text, swTextCase::Sentence), QStringLiteral("Mvf-01d sandstorm fighter. Lego® group! Name@mail.com"));
	QCOMPARE(swChangeTextCase(Text, swTextCase::Title), QStringLiteral("Mvf-01d Sandstorm Fighter. Lego® Group! Name@mail.com"));
	QCOMPARE(swChangeTextCase(QString(), swTextCase::Title), QString());
}
