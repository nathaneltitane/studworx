#include "sw_tests.h"

void swTests::TranslucentImagesKeepColor_data()
{
	QTest::addColumn<int>("Samples");
	QTest::newRow("single-sample") << 1;
	QTest::newRow("multisample") << 4;
}

// Saves a Trans-Clear model with the StudWorx binary, once on a transparent background and once on
// an opaque one. Translucent pixels must stay white rather than darkened by the cleared background,
// and opaque images must keep the background color. SW_TEST_BINARY is the studworx program, and
// SW_TEST_NATIVE_DISPLAY=1 renders on the running display instead of offscreen.
void swTests::TranslucentImagesKeepColor()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QFETCH(int, Samples);

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());

	QFile Model(Folder.filePath("model.ldr"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 Test\r\n"
		"4 47 -20 0 -20 20 0 -20 20 0 20 -20 0 20\r\n"
		"4 47 -20 4 -20 20 4 -20 20 4 20 -20 4 20\r\n");
	Model.close();

	// A throwaway config sets the samples and keeps the user's settings out of the images.
	QVERIFY(QDir().mkpath(Folder.filePath("config/StudWorx")));
	QFile Config(Folder.filePath("config/StudWorx/StudWorx.conf"));
	QVERIFY(Config.open(QIODevice::WriteOnly));
	Config.write(QString("[Settings]\nAASamples=%1\n").arg(Samples).toLatin1());
	Config.close();

	QProcessEnvironment ImageEnvironment = Environment;
	ImageEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ImageEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ImageEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));

	if (Environment.value("SW_TEST_NATIVE_DISPLAY") != QLatin1String("1"))
		ImageEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	for (const bool Transparent : { true, false })
	{
		const QString FileName = Folder.filePath(Transparent ? "transparent.png" : "opaque.png");
		QStringList Arguments = { Model.fileName(), "--shading", "flat", "--line-width", "0", "--camera-angles", "30", "45", "-w", "64", "--height", "64", "-i", FileName };

		if (Transparent)
			Arguments << "--transparent";

		QProcess Render;
		Render.setProcessEnvironment(ImageEnvironment);
		Render.setProcessChannelMode(QProcess::MergedChannels);
		Render.start(Binary, Arguments);
		QVERIFY2(Render.waitForFinished(120000), "the image was not saved");
		QVERIFY2(Render.exitCode() == 0, Render.readAll().constData());

		const QImage Image = QImage(FileName).convertToFormat(QImage::Format_ARGB32);
		QCOMPARE(Image.size(), QSize(64, 64));

		if (Transparent)
		{
			QCOMPARE(Image.pixel(0, 0), QRgb(0));
			int TranslucentPixels = 0;

			for (int Y = 0; Y < Image.height(); Y++)
				for (int X = 0; X < Image.width(); X++)
				{
					const QRgb Pixel = Image.pixel(X, Y);

					if (qAlpha(Pixel) <= 100 || qAlpha(Pixel) >= 240)
						continue;

					QVERIFY2(qRed(Pixel) > 245 && qGreen(Pixel) > 245 && qBlue(Pixel) > 245, qPrintable(QString("pixel %1,%2 is %3").arg(X).arg(Y).arg(Pixel, 8, 16, QLatin1Char('0'))));
					TranslucentPixels++;
				}

			QVERIFY(TranslucentPixels > 100);
		}
		else
		{
			// the default background is dark gray, never black
			const QRgb Corner = Image.pixel(0, 0);
			QCOMPARE(qAlpha(Corner), 255);
			QVERIFY(qRed(Corner) > 20 && qGreen(Corner) > 20 && qBlue(Corner) > 20);
		}
	}
}

// use to measure the opaque pixels of an image
static QRect swTestOpaqueBounds(const QImage& Image)
{
	QRect Bounds;

	for (int Y = 0; Y < Image.height(); Y++)
		for (int X = 0; X < Image.width(); X++)
			if (qAlpha(Image.pixel(X, Y)) > 128)
				Bounds |= QRect(X, Y, 1, 1);

	return Bounds;
}

// Saves a long bar from the side and from the end. With --camera-orbit both pictures must keep the
// bar centered at the same height, while --camera-angles fits each view and zooms in on the end.
void swTests::CameraOrbitKeepsModelInPlace()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());

	// a bar 160 long, 16 high and 16 deep
	QFile Model(Folder.filePath("bar.ldr"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 Bar\r\n"
		"4 4 -80 -8 -8 80 -8 -8 80 -8 8 -80 -8 8\r\n"
		"4 4 -80 8 -8 80 8 -8 80 8 8 -80 8 8\r\n"
		"4 4 -80 -8 -8 80 -8 -8 80 8 -8 -80 8 -8\r\n"
		"4 4 -80 -8 8 80 -8 8 80 8 8 -80 8 8\r\n"
		"4 4 -80 -8 -8 -80 -8 8 -80 8 8 -80 8 -8\r\n"
		"4 4 80 -8 -8 80 -8 8 80 8 8 80 8 -8\r\n");
	Model.close();

	QProcessEnvironment ImageEnvironment = Environment;
	ImageEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	ImageEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	ImageEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));

	if (Environment.value("SW_TEST_NATIVE_DISPLAY") != QLatin1String("1"))
		ImageEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	// use to save the bar from one longitude
	auto SaveBounds = [&](const QString& Option, int Longitude)
	{
		const QString FileName = Folder.filePath(QString("%1-%2.png").arg(Option.mid(2)).arg(Longitude));
		QProcess Render;
		Render.setProcessEnvironment(ImageEnvironment);
		Render.setProcessChannelMode(QProcess::MergedChannels);
		Render.start(Binary, { Model.fileName(), "--transparent", Option, "0", QString::number(Longitude), "-w", "128", "--height", "128", "-i", FileName });

		if (!Render.waitForFinished(120000) || Render.exitCode() != 0)
			return QRect();

		return swTestOpaqueBounds(QImage(FileName).convertToFormat(QImage::Format_ARGB32));
	};

	const QRect OrbitSide = SaveBounds("--camera-orbit", 0);
	const QRect OrbitEnd = SaveBounds("--camera-orbit", 90);
	const QRect FitSide = SaveBounds("--camera-angles", 0);
	const QRect FitEnd = SaveBounds("--camera-angles", 90);

	QVERIFY2(!OrbitSide.isEmpty() && !OrbitEnd.isEmpty() && !FitSide.isEmpty() && !FitEnd.isEmpty(), "an image was not saved");

	// one turn axis through the image center
	QVERIFY2(qAbs(OrbitSide.center().x() - 64) <= 2 && qAbs(OrbitEnd.center().x() - 64) <= 2, qPrintable(QString("centers %1 and %2").arg(OrbitSide.center().x()).arg(OrbitEnd.center().x())));
	QVERIFY2(qAbs(OrbitSide.center().y() - OrbitEnd.center().y()) <= 2, qPrintable(QString("heights at %1 and %2").arg(OrbitSide.center().y()).arg(OrbitEnd.center().y())));

	// the near end looks a little bigger in perspective, fitting each view zooms far in
	QVERIFY2(OrbitEnd.height() <= OrbitSide.height() * 3 / 2, qPrintable(QString("orbit heights %1 and %2").arg(OrbitSide.height()).arg(OrbitEnd.height())));
	QVERIFY2(FitEnd.height() > FitSide.height() * 3, qPrintable(QString("fitted heights %1 and %2").arg(FitSide.height()).arg(FitEnd.height())));
}
