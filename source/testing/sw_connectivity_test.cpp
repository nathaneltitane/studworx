#include "sw_global.h"
#include "sw_tests.h"
#include "sw_parser_shadow.h"

// Made-up parts, described with SNAP_CYL lines like those of the LDCad shadow library, in LDraw
// coordinates: studs point up along -Y.
static std::vector<swConnectionCylinder> MakePart(std::initializer_list<const char*> Lines)
{
	std::vector<swConnectionCylinder> Cylinders;

	for (const char* Line : Lines)
	{
		const std::optional<swShadowLine> Parsed = swShadowParser::ParseLine(QLatin1String(Line));

		if (Parsed)
			Cylinders.insert(Cylinders.end(), Parsed->Cylinders.begin(), Parsed->Cylinders.end());
	}

	return Cylinders;
}

static std::vector<swConnectionCylinder> MakePlate(int Width, int Length)
{
	const std::string Grid = QString("C %1 C %2 20 20").arg(Length).arg(Width).toStdString();
	const std::string Studs = "0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 6 4] [grid=" + Grid + "]";
	const std::string AntiStuds = "0 !LDCAD SNAP_CYL [gender=F] [caps=one] [secs=S 6 4] [pos=0 8 0] [grid=" + Grid + "]";

	return MakePart({ Studs.c_str(), AntiStuds.c_str() });
}

static std::vector<swConnectionCylinder> MakeBrick1x2()
{
	return MakePart({
		"0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 6 4] [grid=C 2 1 20 0]",
		"0 !LDCAD SNAP_CYL [gender=F] [caps=one] [secs=S 6 20] [pos=0 24 0] [grid=C 2 1 20 0]",
		"0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 4 20] [pos=0 4 0] [ori=-1 0 0 0 -1 0 0 0 1]",
	});
}

static std::vector<swConnectionCylinder> MakeBar3L()
{
	return MakePart({ "0 !LDCAD SNAP_CYL [gender=M] [caps=none] [secs=R 4 60] [center=true] [slide=true] [pos=0 30 0]" });
}

static const swConnectionCylinder* FindShape(const std::vector<swConnectionCylinder>& Cylinders, swConnectionGender Gender, float Radius)
{
	for (const swConnectionCylinder& Cylinder : Cylinders)
		if (Cylinder.Gender == Gender && Cylinder.GetRadius() == Radius)
			return &Cylinder;

	return nullptr;
}

void swTests::ShadowParsesLines()
{
	const std::pair<const char*, size_t> Lines[] =
	{
		{ "0 !LDCAD SNAP_CYL [gender=F] [caps=one] [secs=S 6 20] [pos=0 24 0] [grid=C 2 1 20 0]", 2 },
		{ "0 !LDCAD SNAP_CYL [gender=M] [secs=R 6 4] [pos=0 x 0]", 0 },
		{ "0 !LDCAD SNAP_CYL [gender=M] [secs=R 6] [pos=0 0 0]", 0 },
		{ "0 !LDCAD SNAP_CYL [gender=M] [secs=R 6 4] [ori=1 0 0]", 0 },
		{ "0 !LDCAD SNAP_CYL [gender=M] [secs=R 6 4] [grid=C 2 1 20]", 0 },
		{ "0 !LDCAD SNAP_CYL [gender=M] [secs=R 6 4] [grid=2 99999 20 20]", 0 },
		{ "0 !LDCAD SNAP_CYL [gender=M] [secs=R 6 4] [pos=0 inf 0]", 0 },
	};

	for (const auto& [Line, Expected] : Lines)
		QVERIFY2(MakePart({ Line }).size() == Expected, Line);

	const std::vector<swConnectionCylinder> Bar = MakeBar3L();

	QCOMPARE(Bar.size(), size_t(1));
	QVERIFY(Bar[0].Center && !Bar[0].ClosedStart && !Bar[0].ClosedEnd);
	QCOMPARE(Bar[0].GetExtent(), std::make_pair(-30.0f, 30.0f));

	// caps=one closes the base of a male shape and the far end of a female one.
	const std::vector<swConnectionCylinder> Plate = MakePlate(1, 1);
	const swConnectionCylinder* Stud = FindShape(Plate, swConnectionGender::Male, 6.0f);
	const swConnectionCylinder* AntiStud = FindShape(Plate, swConnectionGender::Female, 6.0f);

	QVERIFY(Stud && AntiStud);
	QVERIFY(Stud->ClosedStart && !Stud->ClosedEnd);
	QVERIFY(!AntiStud->ClosedStart && AntiStud->ClosedEnd);
	QVERIFY(swConnectivity::AreCompatible(*Stud, *AntiStud));
}

void swTests::ShadowParsesIncludesAndClears()
{
	const std::optional<swShadowLine> Include = swShadowParser::ParseLine(QLatin1String("0 !LDCAD SNAP_INCL [ref=s/3001s01.dat] [pos=0 8 0] [grid=2 1 20 0]"));

	QVERIFY(Include && Include->Type == swShadowLineType::Include);
	QCOMPARE(Include->Reference, std::string("s/3001s01.dat"));
	QCOMPARE(Include->Placements.size(), size_t(2));
	QCOMPARE(Include->Placements[1].GetTranslation()[0], 20.0f);
	QCOMPARE(Include->Placements[1].GetTranslation()[1], 8.0f);

	const std::optional<swShadowLine> ClearAll = swShadowParser::ParseLine(QLatin1String("0  !LDCAD   SNAP_CLEAR"));
	const std::optional<swShadowLine> ClearId = swShadowParser::ParseLine(QLatin1String("0 !LDCAD SNAP_CLEAR [id=studC]"));

	QVERIFY(ClearAll && ClearAll->Type == swShadowLineType::Clear && ClearAll->Id.isEmpty());
	QVERIFY(ClearId && ClearId->Type == swShadowLineType::Clear && ClearId->Id == QLatin1String("studC"));

	const char* Ignored[] =
	{
		"0 //!LDCAD SNAP_CYL [gender=M] [secs=R 6 4]",
		"0 !LDCAD SNAP_CLP [radius=x] [length=8]",
		"0 !LDCAD SNAP_INCL [pos=0 0 0]",
		"0 !LDCAD SNAP_CYL [gender=M] [secs=R 6]",
		"1 16 0 0 0 1 0 0 0 1 0 0 0 1 stud.dat",
		"",
	};

	for (const char* Line : Ignored)
		QVERIFY2(!swShadowParser::ParseLine(QLatin1String(Line)), Line);
}

void swTests::ShadowReadsSubFileReferences()
{
	const std::optional<std::pair<std::string, swMatrix44>> Stud = swShadowParser::ParseReference("1 16 10 -4 -10 1 0 0 0 -5 0 0 0 1 stud4.dat");

	QVERIFY(Stud);
	QCOMPARE(Stud->first, std::string("stud4.dat"));
	QCOMPARE(Stud->second.GetTranslation()[0], 10.0f);
	QCOMPARE(Stud->second.GetTranslation()[1], -4.0f);
	QCOMPARE(Stud->second.GetTranslation()[2], -10.0f);
	QCOMPARE(Stud->second.r[1][1], -5.0f);

	QVERIFY(swShadowParser::ParseReference("1 0x2FF0000 0 0 0 1 0 0 0 1 0 0 0 1 s\\3001s01.dat"));

	const char* Ignored[] =
	{
		"0 !LDCAD SNAP_INCL [ref=stud.dat]",
		"2 24 0 0 0 10 0 0",
		"1 16 0 0 0 1 0 0 0 1 stud.dat",
	};

	for (const char* Line : Ignored)
		QVERIFY2(!swShadowParser::ParseReference(Line), Line);
}

void swTests::ShadowScalesAndMirrorsSubFileShapes()
{
	// A shape used from a sub-file stretched 5 times along Y keeps the stretch only with scale=YOnly.
	const swMatrix44 Stretched = swShadowParser::GetLDrawMatrix(std::array<float, 9>{ 1, 0, 0, 0, 5, 0, 0, 0, 1 }.data(), swVector3(0.0f, 0.0f, 0.0f));
	swConnectionCylinder Pin = MakePart({ "0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 4 4] [scale=YOnly]" }).front();
	swConnectionCylinder Stud = MakePart({ "0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 6 4]" }).front();

	Pin.Transform = swMultiply(Pin.Transform, Stretched);
	Stud.Transform = swMultiply(Stud.Transform, Stretched);

	QVERIFY(swShadowParser::NormalizeFrame(Pin));
	QCOMPARE(Pin.GetLength(), 20.0f);
	QVERIFY(fabsf(Pin.Transform.Determinant() - 1.0f) < 0.001f);
	QVERIFY(!swShadowParser::NormalizeFrame(Stud));

	// A mirrored shape is corrected by flipping a radius axis (mirror=cor), or dropped (mirror=none).
	swConnectionCylinder Mirrored = MakePart({ "0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 6 4]" }).front();
	swConnectionCylinder NotMirrored = MakePart({ "0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 6 4] [mirror=none]" }).front();
	const swMatrix44 Mirror = swShadowParser::GetLDrawMatrix(std::array<float, 9>{ -1, 0, 0, 0, 1, 0, 0, 0, 1 }.data(), swVector3(10.0f, 0.0f, 0.0f));

	Mirrored.Transform = swMultiply(Mirrored.Transform, Mirror);
	NotMirrored.Transform = swMultiply(NotMirrored.Transform, Mirror);

	QVERIFY(swShadowParser::NormalizeFrame(Mirrored));
	QVERIFY(fabsf(Mirrored.Transform.Determinant() - 1.0f) < 0.001f);
	QCOMPARE(Mirrored.Transform.GetTranslation(), swVector3(10.0f, 0.0f, 0.0f));
	QVERIFY(!swShadowParser::NormalizeFrame(NotMirrored));
}

void swTests::ConnectivityFitsShapesAlongTheirAxis()
{
	const std::vector<swConnectionCylinder> Plate = MakePlate(1, 1);
	const swConnectionCylinder* Stud = FindShape(Plate, swConnectionGender::Male, 6.0f);
	const swConnectionCylinder* AntiStud = FindShape(Plate, swConnectionGender::Female, 6.0f);

	// A stud goes all the way into an anti stud and no further.
	QVERIFY((swConnectivity::GetInsertRanges(*Stud, *AntiStud) == std::vector<std::pair<float, float>>({ { 0.0f, 0.0f } })));

	// A 3L bar through a hole open at both ends keeps the 16 long hole covered.
	const swConnectionCylinder Bar = MakeBar3L().front();
	const swConnectionCylinder OpenHole = MakePart({ "0 !LDCAD SNAP_CYL [gender=F] [caps=none] [secs=R 4 16] [center=true]" }).front();

	QVERIFY((swConnectivity::GetInsertRanges(Bar, OpenHole) == std::vector<std::pair<float, float>>({ { -22.0f, 22.0f } })));

	// Into a 10 deep tube closed at the far end, the bar goes to the end.
	const swConnectionCylinder Tube = MakePart({ "0 !LDCAD SNAP_CYL [gender=F] [caps=one] [secs=R 4 10] [slide=true]" }).front();

	QVERIFY((swConnectivity::GetInsertRanges(Bar, Tube) == std::vector<std::pair<float, float>>({ { -20.0f, -20.0f } })));

	// A round plate's stud goes into a round brick's anti stud with the plate's wider edge outside.
	const swConnectionCylinder RoundPlate = MakePart({ "0 !LDCAD SNAP_CYL [gender=M] [caps=none] [secs=R 10 3   R 6 4] [slide=true] [pos=0 3 0]" }).front();
	const swConnectionCylinder RoundBrick = MakePart({ "0 !LDCAD SNAP_CYL [gender=F] [caps=none] [secs=R 6 20   R 4 8] [pos=0 24 0]" }).front();

	QVERIFY((swConnectivity::GetInsertRanges(RoundPlate, RoundBrick) == std::vector<std::pair<float, float>>({ { -3.0f, -3.0f } })));

	// A technic pin goes halfway into a hole from either side, but not so far that its wider
	// collar is in the hole; its friction ridges may squeeze into the hole.
	const swConnectionCylinder Pin = MakePart({ "0 !LDCAD SNAP_CYL [gender=M] [caps=none] [secs=L_ 6.25 2   R 6 16   R 8 4   R 6 16   _L 6.25 2] [center=true] [slide=true]" }).front();
	const swConnectionCylinder PinHole = MakePart({ "0 !LDCAD SNAP_CYL [gender=F] [caps=none] [secs=R 8 2   R 6 16   R 8 2] [center=true] [slide=true]" }).front();

	QVERIFY((swConnectivity::GetInsertRanges(Pin, PinHole) == std::vector<std::pair<float, float>>({ { -10.0f, -10.0f }, { 10.0f, 10.0f } })));

	// Without countersinks the collar stays outside the hole.
	const swConnectionCylinder PlainHole = MakePart({ "0 !LDCAD SNAP_CYL [gender=F] [caps=none] [secs=R 6 20] [center=true]" }).front();

	QVERIFY((swConnectivity::GetInsertRanges(Pin, PlainHole) == std::vector<std::pair<float, float>>({ { -12.0f, -12.0f }, { 12.0f, 12.0f } })));
}

void swTests::ConnectivityConnectsSeatedShapes()
{
	// A 1x2 brick connects to a plate when its anti studs sit on two of the plate's studs, also
	// turned a quarter, but not half a stud off, lifted, or with its pin on top of a stud.
	const std::vector<swConnectionCylinder> Plate = MakePlate(6, 8);
	const std::vector<swConnectionCylinder> Brick = MakeBrick1x2();
	const auto At = [](float x, float y, float z, float Angle)
	{
		swMatrix44 Transform = swMatrix44RotationZ(Angle);
		Transform.SetTranslation(swVector3LDrawToStudWorx(swVector3(x, y, z)));
		return Transform;
	};
	const std::tuple<const char*, swMatrix44, bool> Cases[] =
	{
		{ "seated", At(-60.0f, -24.0f, -50.0f, 0.0f), true },
		{ "turned", At(-70.0f, -24.0f, -40.0f, SW_PI / 2.0f), true },
		{ "half a stud off", At(-50.0f, -24.0f, -50.0f, 0.0f), false },
		{ "lifted", At(-60.0f, -28.0f, -50.0f, 0.0f), false },
		{ "on its pin", At(-70.0f, -4.0f, -50.0f, 0.0f), false },
	};

	for (const auto& [Case, Transform, Expected] : Cases)
	{
		QVERIFY2(swConnectivity::AreConnected(Brick, Transform, Plate, swMatrix44Identity()) == Expected, Case);
		QVERIFY2(swConnectivity::AreConnected(Plate, swMatrix44Identity(), Brick, Transform) == Expected, Case);
	}

	// An anti stud at a stud but turned across it doesn't connect.
	const std::vector<swConnectionCylinder> Stud = MakePart({ "0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 6 4]" });
	const std::vector<swConnectionCylinder> AntiStud = MakePart({ "0 !LDCAD SNAP_CYL [gender=F] [caps=one] [secs=S 6 4]" });
	const std::vector<swConnectionCylinder> CrossedAntiStud = MakePart({ "0 !LDCAD SNAP_CYL [gender=F] [caps=one] [secs=S 6 4] [ori=1 0 0 0 0 -1 0 1 0]" });

	QVERIFY(swConnectivity::AreConnected(Stud, swMatrix44Identity(), AntiStud, swMatrix44Identity()));
	QVERIFY(!swConnectivity::AreConnected(Stud, swMatrix44Identity(), CrossedAntiStud, swMatrix44Identity()));

	// A 3L bar through a hole open at both ends connects wherever it covers the hole.
	const std::vector<swConnectionCylinder> Bar = MakeBar3L();
	const std::vector<swConnectionCylinder> Hole = MakePart({ "0 !LDCAD SNAP_CYL [gender=F] [caps=none] [secs=R 4 16] [center=true] [pos=0 30 0]" });

	QVERIFY(swConnectivity::AreConnected(Bar, swMatrix44Identity(), Hole, swMatrix44Identity()));
	QVERIFY(swConnectivity::AreConnected(Bar, swMatrix44Translation(swVector3(0.0f, 0.0f, 20.0f)), Hole, swMatrix44Identity()));
	QVERIFY(!swConnectivity::AreConnected(Bar, swMatrix44Translation(swVector3(0.0f, 0.0f, 40.0f)), Hole, swMatrix44Identity()));

	// A round plate connects under a round brick with a hollow stud, not inside it, where its edge
	// would be in the narrower hole.
	const std::vector<swConnectionCylinder> RoundPlate = MakePart({ "0 !LDCAD SNAP_CYL [gender=M] [caps=none] [secs=R 10 3   R 6 4] [slide=true] [pos=0 3 0]" });
	const std::vector<swConnectionCylinder> RoundBrick = MakePart({ "0 !LDCAD SNAP_CYL [gender=F] [caps=none] [secs=R 6 20   R 4 8] [pos=0 24 0]" });

	QVERIFY(swConnectivity::AreConnected(RoundPlate, swMatrix44Translation(swVector3LDrawToStudWorx(swVector3(0.0f, 24.0f, 0.0f))), RoundBrick, swMatrix44Identity()));
	QVERIFY(!swConnectivity::AreConnected(RoundPlate, swMatrix44Identity(), RoundBrick, swMatrix44Identity()));
	QVERIFY(!swConnectivity::AreConnected(RoundPlate, swMatrix44Translation(swVector3LDrawToStudWorx(swVector3(0.0f, 16.0f, 0.0f))), RoundBrick, swMatrix44Identity()));

	// A part made of several library parts connects through any one of them.
	const swMatrix44 Away = swMatrix44Translation(swVector3(0.0f, 0.0f, 400.0f));
	const std::vector<std::pair<const std::vector<swConnectionCylinder>*, swMatrix44>> BarAndStud = { { &Stud, Away }, { &Bar, swMatrix44Identity() } };
	const std::vector<std::pair<const std::vector<swConnectionCylinder>*, swMatrix44>> HoleAndAntiStud = { { &AntiStud, swMatrix44Identity() }, { &Hole, swMatrix44Identity() } };
	const std::vector<std::pair<const std::vector<swConnectionCylinder>*, swMatrix44>> HoleAway = { { &Hole, Away } };

	QVERIFY(swConnectivity::AreConnected(BarAndStud, HoleAndAntiStud));
	QVERIFY(swConnectivity::AreConnected(HoleAndAntiStud, BarAndStud));
	QVERIFY(!swConnectivity::AreConnected(BarAndStud, HoleAway));

	// The seated pair gives its axis, along the bar, and the bar's position.
	swVector3 Axis;
	swVector3 Point;

	QVERIFY(swConnectivity::GetConnectionAxis(BarAndStud, HoleAndAntiStud, Axis, Point));
	QVERIFY(fabsf(fabsf(Axis.z) - 1.0f) < 0.01f);
	QVERIFY(fabsf(Point.x) < 0.01f && fabsf(Point.y) < 0.01f);
	QVERIFY(!swConnectivity::GetConnectionAxis(BarAndStud, HoleAway, Axis, Point));

	// The box around the bar's shape takes in its length and radius.
	swBoundingBox Box = { swVector3(FLT_MAX, FLT_MAX, FLT_MAX), swVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX) };

	swConnectivity::AddToBoundingBox(Bar, swMatrix44Translation(swVector3(10.0f, 0.0f, 0.0f)), Box);

	QVERIFY(swLength(Box.Min - swVector3(6.0f, -4.0f, -64.0f)) < 0.01f);
	QVERIFY(swLength(Box.Max - swVector3(14.0f, 4.0f, 4.0f)) < 0.01f);
}

void swTests::ConnectivityMarksNearbyOpenEnds()
{
	// A 6x8 plate hovered at a stud tip marks four of the five stud tips within a stud, pointing
	// up, and the anti stud right below at the bottom, pointing down, each at its open end.
	const std::vector<swConnectionCylinder> Plate = MakePlate(6, 8);
	const swConnectionShapes Shapes = { { &Plate, swMatrix44Identity() } };
	const swVector3 Up = swVector3LDrawToStudWorx(swVector3(0.0f, -1.0f, 0.0f));
	const swVector3 Tip = swVector3LDrawToStudWorx(swVector3(-10.0f, -4.0f, -10.0f));
	const std::vector<swConnectionMarker> Markers = swConnectivity::GetMarkers(Shapes, Tip, 20.0f, 4);

	QCOMPARE(Markers.size(), size_t(5));
	QCOMPARE(std::count_if(Markers.begin(), Markers.end(), [](const swConnectionMarker& Marker) { return Marker.Gender == swConnectionGender::Female; }), 1);

	for (const swConnectionMarker& Marker : Markers)
	{
		const bool Male = Marker.Gender == swConnectionGender::Male;

		QVERIFY(!Marker.TwoSided);
		QVERIFY(swLength(Marker.Direction - (Male ? Up : -Up)) < 0.001f);
		QCOMPARE(Marker.Shape, Male ? swConnectionShape::Round : swConnectionShape::Square);
		QVERIFY(swLength(Marker.Position - Tip) <= 20.0f);
	}

	QVERIFY(swLength(Markers.front().Position - Tip) < 0.001f);

	// A hole open at both ends marks both openings, a closed bar its middle, and far ends nothing.
	const std::vector<swConnectionCylinder> Hole = MakePart({ "0 !LDCAD SNAP_CYL [gender=F] [caps=none] [secs=A 4 16] [center=true]" });
	const std::vector<swConnectionCylinder> Pin = MakePart({ "0 !LDCAD SNAP_CYL [gender=M] [caps=two] [secs=R 4 16] [center=true]" });

	QCOMPARE(swConnectivity::GetMarkers({ { &Hole, swMatrix44Identity() } }, swVector3(0.0f, 0.0f, 0.0f), 20.0f, 4).size(), size_t(2));
	QCOMPARE(swConnectivity::GetMarkers({ { &Hole, swMatrix44Identity() } }, swVector3(0.0f, 0.0f, 0.0f), 20.0f, 4).front().Shape, swConnectionShape::Axle);
	QCOMPARE(swConnectivity::GetMarkers({ { &Pin, swMatrix44Identity() } }, swVector3(0.0f, 0.0f, 0.0f), 20.0f, 4).size(), size_t(1));
	QVERIFY(swConnectivity::GetMarkers({ { &Pin, swMatrix44Identity() } }, swVector3(0.0f, 0.0f, 0.0f), 20.0f, 4).front().TwoSided);
	QVERIFY(swConnectivity::GetMarkers({ { &Pin, swMatrix44Identity() } }, swVector3(100.0f, 0.0f, 0.0f), 20.0f, 4).empty());
}

void swTests::ConnectivitySnapsDraggedParts()
{
	// A 1x2 brick dragged near a plate stud, off by a few units, lifted, turned 80 degrees and
	// tipped, seats on the plate turned a quarter, and nothing seats far away or without shapes.
	const std::vector<swConnectionCylinder> Plate = MakePlate(6, 8);
	const std::vector<swConnectionCylinder> Brick = MakeBrick1x2();
	const swConnectionShapes PlateShapes = { { &Plate, swMatrix44Identity() } };
	const swConnectionShapes BrickShapes = { { &Brick, swMatrix44Identity() } };
	const swVector3 Point = swVector3LDrawToStudWorx(swVector3(-70.0f, 0.0f, -50.0f));
	swMatrix44 Dragged = swMultiply(swMatrix44RotationX(0.2f), swMatrix44RotationZ(80.0f * SW_DEGREES_TO_RADIANS));
	Dragged.SetTranslation(swVector3LDrawToStudWorx(swVector3(-66.0f, -30.0f, -38.0f)));
	swConnectionSnap Snap;

	QVERIFY(swConnectivity::FindSnap(BrickShapes, Dragged, PlateShapes, Point, 20.0f, Snap));
	QVERIFY(Snap.Cylinder && Snap.Cylinder->Gender == swConnectionGender::Female);
	QVERIFY(swConnectivity::AreConnected(Brick, Snap.Transform, Plate, swMatrix44Identity()));
	QVERIFY(!Snap.Hinge);
	QVERIFY(fabsf(fabsf(swDot(swMultiply30(swVector3(1.0f, 0.0f, 0.0f), Snap.Transform), swVector3(0.0f, 1.0f, 0.0f))) - 1.0f) < 0.001f);

	QVERIFY(!swConnectivity::FindSnap(BrickShapes, Dragged, PlateShapes, swVector3(500.0f, 0.0f, 0.0f), 20.0f, Snap));
	QVERIFY(!Snap.Cylinder);
	QVERIFY(!swConnectivity::FindSnap({}, Dragged, PlateShapes, Point, 20.0f, Snap));
	QVERIFY(!swConnectivity::FindSnap(BrickShapes, Dragged, { { nullptr, swMatrix44Identity() } }, Point, 20.0f, Snap));

	// A 3L bar seats sideways in a hole, keeping its slide along the hole, free to turn there.
	const std::vector<swConnectionCylinder> Bar = MakeBar3L();
	const std::vector<swConnectionCylinder> Hole = MakePart({ "0 !LDCAD SNAP_CYL [gender=F] [caps=none] [secs=R 4 16] [center=true] [pos=0 30 0] [ori=1 0 0 0 0 -1 0 1 0]" });
	swMatrix44 Across = swMatrix44RotationY(1.3f);
	Across.SetTranslation(swVector3(3.0f, 2.0f, 5.0f));

	QVERIFY(swConnectivity::FindSnap({ { &Bar, swMatrix44Identity() } }, Across, { { &Hole, swMatrix44Identity() } }, swVector3LDrawToStudWorx(swVector3(0.0f, 30.0f, 0.0f)), 40.0f, Snap));
	QVERIFY(swConnectivity::AreConnected(Bar, Snap.Transform, Hole, swMatrix44Identity()));
	QVERIFY(Snap.Hinge);

	// A 3L bar dropped beside a clip, as the library writes it without a gender, seats in the clip.
	const std::vector<swConnectionCylinder> Clip = MakePart({ "0 !LDCAD SNAP_CLP [radius=4] [length=8] [center=true] [pos=0 0 -10] [ori=0 -1 0 -1 0 0 0 0 -1]" });
	swMatrix44 Beside = swMatrix44RotationX(0.4f);
	Beside.SetTranslation(swVector3(-4.0f, 9.0f, 3.0f));

	QCOMPARE(Clip.size(), size_t(1));
	QVERIFY(Clip[0].Gender == swConnectionGender::Female);
	QVERIFY(swConnectivity::FindSnap({ { &Bar, swMatrix44Identity() } }, Beside, { { &Clip, swMatrix44Identity() } }, swVector3LDrawToStudWorx(swVector3(0.0f, 0.0f, -10.0f)), 20.0f, Snap));
	QVERIFY(swConnectivity::AreConnected(Bar, Snap.Transform, Clip, swMatrix44Identity()));
	QVERIFY(Snap.Hinge);

	// A ball dropped beside a socket of its size seats on the socket center, keeping its turn.
	const std::vector<swConnectionCylinder> Ball = MakePart({ "0 !LDCAD SNAP_GEN [gender=M] [bounding=sph 8] [match=size] [placement=free] [pos=0 -10 0]" });
	const std::vector<swConnectionCylinder> Socket = MakePart({ "0 !LDCAD SNAP_GEN [gender=F] [bounding=sph 8] [match=size] [placement=free] [pos=20 0 0]" });
	const std::vector<swConnectionCylinder> BigSocket = MakePart({ "0 !LDCAD SNAP_GEN [group=techBallJnt] [gender=F] [bounding=sph 12.7] [placement=free] [pos=20 0 0]" });
	swMatrix44 Tilted = swMatrix44RotationX(0.4f);
	Tilted.SetTranslation(swVector3(10.0f, 6.0f, 3.0f));

	QVERIFY(Ball.size() == 1 && Socket.size() == 1);
	QVERIFY(Ball[0].Gender == swConnectionGender::Male && Socket[0].Gender == swConnectionGender::Female);
	QVERIFY(!swConnectivity::AreCompatible(Ball[0], BigSocket[0]));
	QVERIFY(swConnectivity::FindSnap({ { &Ball, swMatrix44Identity() } }, Tilted, { { &Socket, swMatrix44Identity() } }, swVector3LDrawToStudWorx(swVector3(20.0f, 0.0f, 0.0f)), 20.0f, Snap));
	QVERIFY(swConnectivity::AreConnected(Ball, Snap.Transform, Socket, swMatrix44Identity()));
	QVERIFY(swMatrix33Similar(swMatrix33(Snap.Transform), swMatrix33(Tilted)));
	QVERIFY(!Snap.Hinge);

	// Hinge brick top fingers fill the base gaps, centered by default, and turn as a hinge.
	const std::vector<swConnectionCylinder> HingeBase = MakePart({ "0 !LDCAD SNAP_FGR [group=hgBrC] [genderOfs=F] [seq=18 4 18] [radius=4] [pos=0 10 0] [ori=0 -1 0 1 0 0 0 0 1]" });
	const std::vector<swConnectionCylinder> HingeTop = MakePart({ "0 !LDCAD SNAP_FGR [group=hgBrC] [genderOfs=M] [seq=18 4 18] [radius=4] [pos=0 10 0] [ori=0 -1 0 1 0 0 0 0 1]" });
	swMatrix44 Lifted = swMatrix44RotationZ(0.3f);
	Lifted.SetTranslation(swVector3(1.0f, 3.0f, 6.0f));

	QVERIFY(HingeBase.size() == 1 && HingeTop.size() == 1);
	QVERIFY(HingeTop[0].Center && HingeTop[0].Sections.size() == 3);
	QVERIFY(swConnectivity::AreCompatible(HingeTop[0], HingeBase[0]));
	QVERIFY(!swConnectivity::AreCompatible(HingeTop[0], HingeTop[0]));
	QVERIFY(swConnectivity::FindSnap({ { &HingeTop, swMatrix44Identity() } }, Lifted, { { &HingeBase, swMatrix44Identity() } }, swVector3LDrawToStudWorx(swVector3(0.0f, 10.0f, 0.0f)), 20.0f, Snap));
	QVERIFY(swConnectivity::AreConnected(HingeTop, Snap.Transform, HingeBase, swMatrix44Identity()));
	QVERIFY(swLength(Snap.Transform.GetTranslation()) < 0.01f);
	QVERIFY(Snap.Hinge);

	// A single finger with no radius sits in a dual finger gap, a door in its frame, both male first.
	const std::vector<swConnectionCylinder> Dual = MakePart({ "0 !LDCAD SNAP_FGR [group=lckHng] [genderOfs=M] [seq=4.5 8 4.5]" });
	const std::vector<swConnectionCylinder> Single = MakePart({ "0 !LDCAD SNAP_FGR [group=lckHng] [genderOfs=M] [seq=8]" });
	const std::vector<swConnectionCylinder> Frame = MakePart({ "0 !LDCAD SNAP_FGR [group=door] [genderOfs=M] [seq=2 132 2] [radius=4]" });
	const std::vector<swConnectionCylinder> Door = MakePart({ "0 !LDCAD SNAP_FGR [group=door] [genderOfs=M] [seq=132] [radius=4]" });

	QVERIFY(Dual.size() == 1 && Single.size() == 1 && Frame.size() == 1 && Door.size() == 1);
	QCOMPARE(swConnectivity::GetInsertRanges(Single[0], Dual[0]), (std::vector<std::pair<float, float>>{ { 0.0f, 0.0f } }));
	QCOMPARE(swConnectivity::GetInsertRanges(Door[0], Frame[0]), (std::vector<std::pair<float, float>>{ { 0.0f, 0.0f } }));
	QVERIFY(!swConnectivity::AreCompatible(Single[0], Door[0]));
	QVERIFY(!swConnectivity::AreCompatible(Dual[0], Frame[0]));

	// A 1x2x5 brick (2454b) dropped on a side stud of a 1x2x2/3 brick with side studs (4595) turns
	// onto the side studs, not onto the bottom tube next to them.
	const std::vector<swConnectionCylinder> SideStuds = MakePart({
		"0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 6 4] [grid=1 C 2 0 20]",
		"0 !LDCAD SNAP_CYL [gender=F] [caps=one] [secs=S 6 12   R 4 8] [pos=0 16 0] [grid=1 C 2 0 20]",
		"0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 6 4] [pos=10 10 0] [ori=0 -1 0 1 0 0 0 0 1] [grid=1 C 2 0 20]",
		"0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 6 4] [pos=-10 10 0] [ori=0 1 0 -1 0 0 0 0 1] [grid=1 C 2 0 20]",
	});
	const std::vector<swConnectionCylinder> TallBrick = MakePart({
		"0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 6 4] [grid=C 2 1 20 0]",
		"0 !LDCAD SNAP_CYL [gender=F] [caps=one] [secs=S 6 116] [pos=0 120 0] [grid=C 2 1 20 0]",
	});
	swMatrix44 Upright = swMatrix44Identity();
	Upright.SetTranslation(swVector3LDrawToStudWorx(swVector3(14.0f, 10.0f, 10.0f)));

	QVERIFY(swConnectivity::FindSnap({ { &TallBrick, swMatrix44Identity() } }, Upright, { { &SideStuds, swMatrix44Identity() } }, swVector3LDrawToStudWorx(swVector3(14.0f, 10.0f, 10.0f)), 20.0f, Snap));
	QVERIFY(Snap.Cylinder && Snap.Cylinder->Gender == swConnectionGender::Female);
	QVERIFY(swConnectivity::AreConnected(TallBrick, Snap.Transform, SideStuds, swMatrix44Identity()));
	QVERIFY(swDot(swMultiply30(swVector3LDrawToStudWorx(swVector3(0.0f, -1.0f, 0.0f)), Snap.Transform), swVector3LDrawToStudWorx(swVector3(1.0f, 0.0f, 0.0f))) > 0.99f);

	// A brick with studs on top and sides, dragged beside a side hole, seats the facing side stud,
	// and seats its top stud instead when grabbed there.
	const std::vector<swConnectionCylinder> SideHole = MakePart({ "0 !LDCAD SNAP_CYL [gender=F] [caps=one] [secs=S 6 4] [pos=10 0 0] [ori=0 1 0 -1 0 0 0 0 1]" });
	const std::vector<swConnectionCylinder> StudsEverywhere = MakePart({
		"0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 6 4] [grid=1 C 2 0 20]",
		"0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 6 4] [pos=10 10 0] [ori=0 -1 0 1 0 0 0 0 1] [grid=1 C 2 0 20]",
		"0 !LDCAD SNAP_CYL [gender=M] [caps=one] [secs=R 6 4] [pos=-10 10 0] [ori=0 1 0 -1 0 0 0 0 1] [grid=1 C 2 0 20]",
	});
	const swConnectionShapes HoleShapes = { { &SideHole, swMatrix44Identity() } };
	const swConnectionShapes StudShapes = { { &StudsEverywhere, swMatrix44Identity() } };
	const swVector3 StudUp = swVector3LDrawToStudWorx(swVector3(0.0f, -1.0f, 0.0f));
	const swVector3 HolePoint = swVector3LDrawToStudWorx(swVector3(12.0f, 0.0f, 0.0f));
	swMatrix44 Near = swMatrix44Identity();
	Near.SetTranslation(swVector3LDrawToStudWorx(swVector3(20.0f, -10.0f, 10.0f)));

	QVERIFY(swConnectivity::FindSnap(StudShapes, Near, HoleShapes, HolePoint, 20.0f, Snap));
	QVERIFY(swDot(swMultiply30(swVector3(0.0f, 0.0f, 1.0f), Snap.Frame), StudUp) < 0.5f);

	QVERIFY(swConnectivity::FindSnap(StudShapes, Near, HoleShapes, HolePoint, 20.0f, Snap, swVector3LDrawToStudWorx(swVector3(0.0f, -2.0f, -10.0f))));
	QVERIFY(swDot(swMultiply30(swVector3(0.0f, 0.0f, 1.0f), Snap.Frame), StudUp) > 0.99f);
	QVERIFY(swConnectivity::AreConnected(StudsEverywhere, Snap.Transform, SideHole, swMatrix44Identity()));
}
