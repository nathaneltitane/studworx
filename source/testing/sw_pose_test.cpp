#include "sw_global.h"
#include "sw_tests.h"
#include "sw_math.h"
#include "sw_parser_shadow.h"

static swConnectionCylinder ParseCylinder(const char* Line, swConnectionKind Kind)
{
	const std::optional<swShadowLine> Parsed = swShadowParser::ParseLine(QLatin1String(Line));

	if (!Parsed || Parsed->Cylinders.empty() || Parsed->Cylinders.front().Kind != Kind)
		return swConnectionCylinder();

	return Parsed->Cylinders.front();
}

void swTests::ShadowParsesJointKinds()
{
	const swConnectionCylinder Clip = ParseCylinder("0 !LDCAD SNAP_CLP [gender=F] [radius=4] [length=8] [pos=0 8 0]", swConnectionKind::Clip);
	const swConnectionCylinder Finger = ParseCylinder("0 !LDCAD SNAP_FGR [gender=M] [radius=6] [seq=4 4] [pos=0 0 0]", swConnectionKind::Finger);
	const swConnectionCylinder Ball = ParseCylinder("0 !LDCAD SNAP_GEN [gender=M] [bounding=sph 8] [pos=0 0 0]", swConnectionKind::Ball);

	QCOMPARE(Clip.GetRadius(), 4.0f);
	QCOMPARE(Clip.GetLength(), 8.0f);
	QCOMPARE(Finger.GetRadius(), 6.0f);
	QCOMPARE(Finger.GetLength(), 8.0f);
	QCOMPARE(Ball.GetRadius(), 8.0f);
	QVERIFY(!swShadowParser::ParseLine(QLatin1String("0 !LDCAD SNAP_GEN [gender=M] [bounding=box 8 8 8]")));
}

// A round cylinder of a sub-file keeps its pivot and axis when the sub-file is placed.
void swTests::ShadowPlacesSubFileJoint()
{
	swConnectionCylinder Cylinder = ParseCylinder("0 !LDCAD SNAP_CYL [group=clkRot] [gender=M] [secs=R 9.5 5] [pos=1 2 3]", swConnectionKind::Cylinder);
	const std::array<float, 9> Turn = { 0, 1, 0, -1, 0, 0, 0, 0, 1 };
	const swMatrix44 Placement = swShadowParser::GetLDrawMatrix(Turn.data(), swVector3(10.0f, 20.0f, 30.0f));
	const swVector3 Local = swMatrix44LDrawToStudWorx(Cylinder.Transform).GetTranslation();

	QCOMPARE(Local, swVector3(1.0f, 3.0f, -2.0f));

	Cylinder.Transform = swMultiply(Cylinder.Transform, Placement);
	QVERIFY(swShadowParser::NormalizeFrame(Cylinder));

	// pivot and axis follow the placement, in LDraw axes
	const swMatrix44 Frame = swMatrix44LDrawToStudWorx(Cylinder.Transform);
	const swVector3 Pivot = Frame.GetTranslation();
	const swVector3 Axis = swNormalize(swMultiply30(swVector3(0.0f, 0.0f, 1.0f), Frame));
	const swVector3 ExpectedPivot = swMultiply31(swVector3(1.0f, 2.0f, 3.0f), Placement);
	const swVector3 ExpectedAxis = swMultiply30(swVector3(0.0f, -1.0f, 0.0f), Placement);

	QVERIFY(swLength(Pivot - swVector3(ExpectedPivot[0], ExpectedPivot[2], -ExpectedPivot[1])) < 0.001f);
	QVERIFY(swLength(Axis - swVector3(ExpectedAxis[0], ExpectedAxis[2], -ExpectedAxis[1])) < 0.001f);
}

void swTests::PoseBlendsRotations()
{
	const swVector3 HingeAxis(0.0f, 0.0f, 1.0f);
	const swVector3 Hinge = swBlendRotation(swVector3(0.0f, 0.0f, 0.0f), swVector3(0.0f, 0.0f, 90.0f), 0.5f, &HingeAxis);
	const swVector3 Ball = swBlendRotation(swVector3(0.0f, 0.0f, 0.0f), swVector3(90.0f, 0.0f, 0.0f), 0.5f, nullptr);

	QVERIFY(fabsf(Hinge[2] - 45.0f) < 0.001f);
	QVERIFY(swLength(Ball - swVector3(45.0f, 0.0f, 0.0f)) < 0.01f);
	QVERIFY(swLength(swBlendRotation(Ball, Ball, 0.0f, nullptr) - Ball) < 0.01f);
}

// Saves a model with a joint and poses, which must come back unchanged.
void swTests::PoseJointSavesAndLoads()
{
	const QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	const QString Binary = Environment.value("SW_TEST_BINARY");

	if (Binary.isEmpty())
		QSKIP("SW_TEST_BINARY is not set");

	QTemporaryDir Folder;
	QVERIFY(Folder.isValid());
	QVERIFY(QDir(Folder.path()).mkpath("library/parts"));

	QFile Part(Folder.filePath("library/parts/3001.dat"));
	QVERIFY(Part.open(QIODevice::WriteOnly));
	Part.write("0 Brick  2 x  4\r\n0 Name: 3001.dat\r\n4 16 -40 0 -20 40 0 -20 40 0 20 -40 0 20\r\n");
	Part.close();

	const QByteArray Joint = "0 !SW JOINT PART HINGE 1.0000 2.0000 3.0000 0.0000 0.0000 1.0000 0.0000 0.0000 30.0000";
	const QByteArray Pose = "0 !SW POSE \"Open\" \"sub.ldr\" 0 0.0000 0.0000 30.0000";

	QFile Model(Folder.filePath("model.mpd"));
	QVERIFY(Model.open(QIODevice::WriteOnly));
	Model.write("0 FILE main.ldr\r\n0 main\r\n" + Pose + "\r\n" + Joint + "\r\n1 4 0 0 0 1 0 0 0 1 0 0 0 1 sub.ldr\r\n0 NOFILE\r\n0 FILE sub.ldr\r\n1 1 0 0 0 1 0 0 0 1 0 0 0 1 3001.dat\r\n0 NOFILE\r\n");
	Model.close();

	QProcessEnvironment SaveEnvironment = Environment;
	SaveEnvironment.insert("XDG_CONFIG_HOME", Folder.filePath("config"));
	SaveEnvironment.insert("XDG_DATA_HOME", Folder.filePath("data"));
	SaveEnvironment.insert("XDG_CACHE_HOME", Folder.filePath("cache"));
	SaveEnvironment.insert("QT_QPA_PLATFORM", "offscreen");

	QProcess Save;
	Save.setProcessEnvironment(SaveEnvironment);
	Save.setProcessChannelMode(QProcess::MergedChannels);
	Save.start(Binary, { "--library", Folder.filePath("library"), Model.fileName(), "--undo", Folder.filePath("saved.mpd") });
	QVERIFY(Save.waitForFinished(120000));
	QVERIFY2(Save.exitCode() == 0, Save.readAll().constData());

	QFile Saved(Folder.filePath("saved.mpd"));
	QVERIFY(Saved.open(QIODevice::ReadOnly));
	const QStringList Lines = QString::fromUtf8(Saved.readAll()).split("\r\n");

	QVERIFY(Lines.contains(QString::fromUtf8(Joint)));
	QVERIFY(Lines.contains(QString::fromUtf8(Pose)));
}
