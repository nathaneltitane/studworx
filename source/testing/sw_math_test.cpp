#include "sw_global.h"
#include "sw_tests.h"
#include "sw_math.h"

// QCOMPARE takes tiny floats for zero, the very residue these tests look for
#define SW_COMPARE_EXACT(Actual, Expected) QVERIFY2((Actual) == (Expected), qPrintable(QString("%1 is not %2").arg(Actual, 0, 'g', 9).arg(Expected, 0, 'g', 9)))

// use to compare two orientations entry by entry
static void CompareMatrix(const swMatrix33& Actual, const swMatrix33& Expected)
{
	for (int Row = 0; Row < 3; Row++)
		for (int Column = 0; Column < 3; Column++)
			SW_COMPARE_EXACT(Actual[Row][Column], Expected[Row][Column]);
}

void swTests::AxisAlignedRotations_data()
{
	QTest::addColumn<int>("Axis");
	QTest::addColumn<int>("Increment");
	QTest::addColumn<bool>("AxisAngle");
	QTest::addColumn<bool>("Relative");

	for (int Axis = 0; Axis < 3; Axis++)
		for (int Increment : { -90, -30, -15, 15, 30, 90 })
			for (bool AxisAngle : { false, true })
				for (bool Relative : { false, true })
					QTest::newRow(qPrintable(QString("%1-%2-%3-%4").arg(Axis).arg(Increment).arg(AxisAngle).arg(Relative))) << Axis << Increment << AxisAngle << Relative;
}

// Turns in small steps must land on exact quarter turns, and moving along the turned axes must keep
// whole coordinates.
void swTests::AxisAlignedRotations()
{
	QFETCH(int, Axis);
	QFETCH(int, Increment);
	QFETCH(bool, AxisAngle);
	QFETCH(bool, Relative);

	swVector3 Unit(0.0f, 0.0f, 0.0f);
	Unit[Axis] = 1.0f;

	swMatrix33 Rotation;

	if (AxisAngle)
		Rotation = swMatrix33FromAxisAngle(Unit, Increment * SW_DEGREES_TO_RADIANS);
	else if (Axis == 0)
		Rotation = swMatrix33RotationX(Increment * SW_DEGREES_TO_RADIANS);
	else if (Axis == 1)
		Rotation = swMatrix33RotationY(Increment * SW_DEGREES_TO_RADIANS);
	else
		Rotation = swMatrix33RotationZ(Increment * SW_DEGREES_TO_RADIANS);

	const swVector3 Center(100.0f, 200.0f, 300.0f);
	const swVector3 Offset(20.0f, 40.0f, 60.0f);
	swMatrix44 TransformCenter(swMatrix33Identity(), Center);
	swMatrix44 TransformOrbit(swMatrix33Identity(), Center + Offset);

	swMatrix33 QuarterTurn = swMatrix33Identity();
	const int AxisFirst = (Axis + 1) % 3;
	const int AxisSecond = (Axis + 2) % 3;
	QuarterTurn[AxisFirst][AxisFirst] = 0.0f;
	QuarterTurn[AxisSecond][AxisSecond] = 0.0f;
	QuarterTurn[AxisFirst][AxisSecond] = Increment > 0 ? 1.0f : -1.0f;
	QuarterTurn[AxisSecond][AxisFirst] = -QuarterTurn[AxisFirst][AxisSecond];

	const int StepsPerQuarter = 90 / std::abs(Increment);

	for (int Step = 1; Step <= StepsPerQuarter * 4; Step++)
	{
		for (swMatrix44* Transform : { &TransformCenter, &TransformOrbit })
		{
			const swMatrix33 Orientation(*Transform);
			const swMatrix33 Frame = Relative ? swMatrix33AffineInverse(Orientation) : swMatrix33Identity();
			const swMatrix33 Delta = Relative ? swMultiply(Rotation, Orientation) : Rotation;

			*Transform = swRotateAroundCenter(*Transform, Delta, Center, Frame);
		}

		if (Step % StepsPerQuarter != 0)
			continue;

		swMatrix33 Expected = swMatrix33Identity();

		for (int Turn = 0; Turn < Step / StepsPerQuarter; Turn++)
			Expected = swMultiply(Expected, QuarterTurn);

		CompareMatrix(swMatrix33(TransformCenter), Expected);
		CompareMatrix(swMatrix33(TransformOrbit), Expected);

		// Positions are stored as floats, so an orbit through angled steps keeps a few of the last
		// digits of its coordinates: a thousandth of an LDU is far below anything visible.
		const swVector3 PositionOrbit = Center + swMultiply(Offset, Expected);
		constexpr float OrbitTolerance = 0.001f;

		for (int Component = 0; Component < 3; Component++)
		{
			SW_COMPARE_EXACT(TransformCenter.GetTranslation()[Component], Center[Component]);
			QVERIFY(fabsf(TransformOrbit.GetTranslation()[Component] - PositionOrbit[Component]) < OrbitTolerance);
		}

		// a grid move along the part's own axis
		const swVector3 Moved = TransformCenter.GetTranslation() + swMultiply(swVector3(20.0f, 0.0f, 0.0f), swMatrix33(TransformCenter));

		for (int Component = 0; Component < 3; Component++)
			SW_COMPARE_EXACT(Moved[Component], Center[Component] + 20.0f * Expected[0][Component]);
	}

	// pivots turn the same way
	swMatrix33 Pivot = swMatrix33Identity();

	for (int Step = 0; Step < StepsPerQuarter; Step++)
		Pivot = swMatrix33Clean(swMultiply(Rotation, Pivot));

	CompareMatrix(Pivot, QuarterTurn);
}

// Snapping takes every signed axis order, mirrored ones included, and leaves real angles alone.
void swTests::AxisAlignedRotationTolerance()
{
	for (int AxisFirst = 0; AxisFirst < 3; AxisFirst++)
	{
		for (int AxisSecond = 0; AxisSecond < 3; AxisSecond++)
		{
			if (AxisFirst == AxisSecond)
				continue;

			for (int Signs = 0; Signs < 8; Signs++)
			{
				swMatrix33 Expected = swMatrix33Scale(swVector3(0.0f, 0.0f, 0.0f));
				Expected[0][AxisFirst] = (Signs & 1) ? -1.0f : 1.0f;
				Expected[1][AxisSecond] = (Signs & 2) ? -1.0f : 1.0f;
				Expected[2][3 - AxisFirst - AxisSecond] = (Signs & 4) ? -1.0f : 1.0f;

				swMatrix33 Perturbed = Expected;

				for (int Row = 0; Row < 3; Row++)
					for (int Column = 0; Column < 3; Column++)
						Perturbed[Row][Column] += 0.0000005f;

				Perturbed.SnapToAxisAligned();
				CompareMatrix(Perturbed, Expected);
			}
		}
	}

	for (float Degrees : { 0.001f, 15.0f, 30.0f, 45.0f, 89.999f, 90.001f })
	{
		const swMatrix33 Original = swMatrix33RotationZ(Degrees * SW_DEGREES_TO_RADIANS);
		swMatrix33 Rotation = Original;

		Rotation.SnapToAxisAligned();
		CompareMatrix(Rotation, Original);
	}

	// whole numbers on a repeated axis are not an orientation
	const swMatrix33 Original(swVector3(1.0f, 0.0f, 0.0f), swVector3(1.0f, 0.0f, 0.0f), swVector3(0.0f, 0.0f, 1.0f));
	swMatrix33 Invalid = Original;

	Invalid.SnapToAxisAligned();
	CompareMatrix(Invalid, Original);
}
