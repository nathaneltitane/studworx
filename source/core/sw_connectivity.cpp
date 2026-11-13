#include "sw_global.h"
#include "sw_connectivity.h"

// equal length tolerance
static const float gConnectionTolerance = 0.01f;

// seated shape tolerance
static const float gSeatTolerance = 0.5f;

// use to detect radius extensions
static bool swIsExtension(const swConnectionSection& Section)
{
	return Section.Shape == swConnectionShape::ExtensionStart || Section.Shape == swConnectionShape::ExtensionEnd;
}

// use to test a fit at an offset
bool swConnectivity::FitsAt(const swConnectionCylinder& Male, const swConnectionCylinder& Female, float Offset)
{
	bool Grips = false;
	float MaleStart = Male.GetExtent().first + Offset;

	for (const swConnectionSection& MaleSection : Male.Sections)
	{
		const float MaleEnd = MaleStart + MaleSection.Length;
		float FemaleStart = Female.GetExtent().first;

		for (const swConnectionSection& FemaleSection : Female.Sections)
		{
			const float FemaleEnd = FemaleStart + FemaleSection.Length;
			const float Overlap = std::min(MaleEnd, FemaleEnd) - std::max(MaleStart, FemaleStart);

			if (Overlap > gConnectionTolerance)
			{
				// male never wider than female
				if (MaleSection.Radius > FemaleSection.Radius + gConnectionTolerance && !swIsExtension(MaleSection) && !swIsExtension(FemaleSection))
					return false;

				// shorter section inside the longer grips
				const bool MaleInside = MaleStart > FemaleStart - gConnectionTolerance && MaleEnd < FemaleEnd + gConnectionTolerance;
				const bool FemaleInside = FemaleStart > MaleStart - gConnectionTolerance && FemaleEnd < MaleEnd + gConnectionTolerance;

				if (AreSectionsCompatible(MaleSection, FemaleSection) && (MaleInside || FemaleInside))
					Grips = true;
			}

			FemaleStart = FemaleEnd;
		}

		MaleStart = MaleEnd;
	}

	return Grips;
}

// use to find valid insert offsets
// use to mesh finger sequences, first offset against second
static std::vector<std::pair<float, float>> swGetFingerOffsets(const swConnectionCylinder& Moving, const swConnectionCylinder& Fixed)
{
	const auto GetBoundaries = [](const swConnectionCylinder& Cylinder)
	{
		std::vector<float> Boundaries = { Cylinder.GetExtent().first };

		for (const swConnectionSection& Section : Cylinder.Sections)
			Boundaries.push_back(Boundaries.back() + Section.Length);

		return Boundaries;
	};

	// the finger gender at a point
	const auto IsMaleAt = [](const swConnectionCylinder& Cylinder, const std::vector<float>& Boundaries, float Point)
	{
		const size_t Index = std::upper_bound(Boundaries.begin(), Boundaries.end(), Point) - Boundaries.begin() - 1;

		return (Index % 2 == 0) == (Cylinder.Gender == swConnectionGender::Male);
	};

	const std::vector<float> MovingBoundaries = GetBoundaries(Moving);
	const std::vector<float> FixedBoundaries = GetBoundaries(Fixed);

	if (MovingBoundaries.size() < 2 || FixedBoundaries.size() < 2)
		return {};

	const float Shortest = std::min(MovingBoundaries.back() - MovingBoundaries.front(), FixedBoundaries.back() - FixedBoundaries.front());
	std::vector<std::pair<float, float>> Offsets;

	// the shorter side meshes along its whole length
	for (float MovingBoundary : MovingBoundaries)
	{
		for (float FixedBoundary : FixedBoundaries)
		{
			const float Offset = FixedBoundary - MovingBoundary;
			const float Low = std::max(MovingBoundaries.front() + Offset, FixedBoundaries.front());
			const float High = std::min(MovingBoundaries.back() + Offset, FixedBoundaries.back());

			if (High - Low < Shortest - gConnectionTolerance)
				continue;

			if (std::any_of(Offsets.begin(), Offsets.end(), [Offset](const std::pair<float, float>& Existing) { return fabsf(Existing.first - Offset) < gConnectionTolerance; }))
				continue;

			std::vector<float> Points = { Low, High };

			for (float Boundary : MovingBoundaries)
				if (Boundary + Offset > Low && Boundary + Offset < High)
					Points.push_back(Boundary + Offset);

			for (float Boundary : FixedBoundaries)
				if (Boundary > Low && Boundary < High)
					Points.push_back(Boundary);

			std::sort(Points.begin(), Points.end());
			bool Meshes = true;

			for (size_t PointIndex = 0; Meshes && PointIndex + 1 < Points.size(); PointIndex++)
			{
				if (Points[PointIndex + 1] - Points[PointIndex] < gConnectionTolerance)
					continue;

				const float Middle = (Points[PointIndex] + Points[PointIndex + 1]) / 2.0f;
				Meshes = IsMaleAt(Moving, MovingBoundaries, Middle - Offset) != IsMaleAt(Fixed, FixedBoundaries, Middle);
			}

			if (Meshes)
				Offsets.emplace_back(Offset, Offset);
		}
	}

	return Offsets;
}

std::vector<std::pair<float, float>> swConnectivity::GetInsertRanges(const swConnectionCylinder& Male, const swConnectionCylinder& Female)
{
	// balls seat on the socket center
	if (Male.Kind == swConnectionKind::Ball || Female.Kind == swConnectionKind::Ball)
		return { { 0.0f, 0.0f } };

	if (Male.Kind == swConnectionKind::Finger || Female.Kind == swConnectionKind::Finger)
		return swGetFingerOffsets(Male, Female);

	const auto [MaleStart, MaleEnd] = Male.GetExtent();
	const auto [FemaleStart, FemaleEnd] = Female.GetExtent();
	float Low = FemaleStart - MaleEnd;
	float High = FemaleEnd - MaleStart;

	if (Female.ClosedStart)
		Low = std::max(Low, FemaleStart - MaleStart);

	if (Female.ClosedEnd)
		High = std::min(High, FemaleEnd - MaleEnd);

	if (Male.ClosedStart)
		High = std::min(High, FemaleStart - MaleStart);

	if (Male.ClosedEnd)
		Low = std::max(Low, FemaleEnd - MaleEnd);

	if (Low > High + gConnectionTolerance)
		return {};

	// fit changes at section boundaries
	std::vector<float> MaleBoundaries = { MaleStart };
	std::vector<float> FemaleBoundaries = { FemaleStart };

	for (const swConnectionSection& Section : Male.Sections)
		MaleBoundaries.push_back(MaleBoundaries.back() + Section.Length);

	for (const swConnectionSection& Section : Female.Sections)
		FemaleBoundaries.push_back(FemaleBoundaries.back() + Section.Length);

	std::vector<float> Offsets = { Low, High };

	for (float FemaleBoundary : FemaleBoundaries)
		for (float MaleBoundary : MaleBoundaries)
			Offsets.push_back(FemaleBoundary - MaleBoundary);

	Offsets.erase(std::remove_if(Offsets.begin(), Offsets.end(), [Low, High](float Offset) { return Offset < Low - gConnectionTolerance || Offset > High + gConnectionTolerance; }), Offsets.end());
	std::sort(Offsets.begin(), Offsets.end());
	Offsets.erase(std::unique(Offsets.begin(), Offsets.end(), [](float a, float b) { return b - a < gConnectionTolerance; }), Offsets.end());

	// test boundaries and midpoints
	for (size_t Index = Offsets.size() - 1; Index > 0; Index--)
		Offsets.insert(Offsets.begin() + Index, (Offsets[Index - 1] + Offsets[Index]) / 2.0f);

	std::vector<std::pair<float, float>> Ranges;
	bool Fitted = false;

	for (float Offset : Offsets)
	{
		const bool Fits = FitsAt(Male, Female, Offset);

		if (Fits && Fitted)
			Ranges.back().second = Offset;
		else if (Fits)
			Ranges.emplace_back(Offset, Offset);

		Fitted = Fits;
	}

	return Ranges;
}

bool swConnectivity::AreCompatible(const swConnectionCylinder& a, const swConnectionCylinder& b)
{
	if (a.Group.compare(b.Group, Qt::CaseInsensitive) != 0)
		return false;

	// fingers fill the gaps of the other side
	if (a.Kind == swConnectionKind::Finger || b.Kind == swConnectionKind::Finger)
		return a.Kind == b.Kind && !GetInsertRanges(a, b).empty();

	if (a.Gender == b.Gender)
		return false;

	// balls sit in sockets of their size
	if (a.Kind == swConnectionKind::Ball || b.Kind == swConnectionKind::Ball)
		return a.Kind == b.Kind && fabsf(a.GetRadius() - b.GetRadius()) < gConnectionTolerance;

	const swConnectionCylinder& Male = a.Gender == swConnectionGender::Male ? a : b;
	const swConnectionCylinder& Female = a.Gender == swConnectionGender::Male ? b : a;

	for (const swConnectionSection& MaleSection : Male.Sections)
		for (const swConnectionSection& FemaleSection : Female.Sections)
			if (AreSectionsCompatible(MaleSection, FemaleSection))
				return true;

	return false;
}

// use to match section shapes
bool swConnectivity::AreSectionsCompatible(const swConnectionSection& Male, const swConnectionSection& Female)
{
	if (fabsf(Male.Radius - Female.Radius) >= gConnectionTolerance)
		return false;

	return Male.Shape == Female.Shape || (Male.Shape == swConnectionShape::Round && Female.Shape == swConnectionShape::Square);
}

// use to test axis and depth
bool swConnectivity::IsSeated(const swConnectionCylinder& Cylinder, const swMatrix44& Frame, const swConnectionCylinder& Other, const swMatrix44& OtherFrame)
{
	// balls turn any way in their socket
	if (Cylinder.Kind == swConnectionKind::Ball || Other.Kind == swConnectionKind::Ball)
		return swLength(Frame.GetTranslation() - OtherFrame.GetTranslation()) < gSeatTolerance;

	const swVector3 ShapeAxis(0.0f, 0.0f, 1.0f);
	const swVector3 Axis = swNormalize(swMultiply30(ShapeAxis, Frame));

	if (swDot(swNormalize(swMultiply30(ShapeAxis, OtherFrame)), Axis) < 0.99f)
		return false;

	const swVector3 Offset = Frame.GetTranslation() - OtherFrame.GetTranslation();
	const float Along = swDot(Offset, Axis);

	if (swLength(Offset - Axis * Along) > gSeatTolerance)
		return false;

	const bool IsMale = Cylinder.Gender == swConnectionGender::Male;
	const std::vector<std::pair<float, float>> InsertRanges = IsMale ? GetInsertRanges(Cylinder, Other) : GetInsertRanges(Other, Cylinder);
	const float MaleOffset = IsMale ? Along : -Along;

	return std::any_of(InsertRanges.begin(), InsertRanges.end(), [MaleOffset](const std::pair<float, float>& Range)
	{
		return MaleOffset > Range.first - gSeatTolerance && MaleOffset < Range.second + gSeatTolerance;
	});
}

void swConnectivity::AddToBoundingBox(const std::vector<swConnectionCylinder>& Cylinders, const swMatrix44& Transform, swBoundingBox& Box)
{
	for (const swConnectionCylinder& Cylinder : Cylinders)
	{
		if (Cylinder.Kind != swConnectionKind::Cylinder)
			continue;

		const swMatrix44 Frame = swMultiply(swMatrix44LDrawToStudWorx(Cylinder.Transform), Transform);
		const swVector3 Axis = swNormalize(swMultiply30(swVector3(0.0f, 0.0f, 1.0f), Frame));
		const swVector3 Radius(Cylinder.GetRadius(), Cylinder.GetRadius(), Cylinder.GetRadius());
		const auto [Start, End] = Cylinder.GetExtent();

		for (float Along : { Start, End })
		{
			const swVector3 Point = Frame.GetTranslation() + Axis * Along;

			Box.Min = swMinimum(Box.Min, Point - Radius);
			Box.Max = swMaximum(Box.Max, Point + Radius);
		}
	}
}

bool swConnectivity::AreConnected(const std::vector<swConnectionCylinder>& Cylinders, const swMatrix44& Transform, const std::vector<swConnectionCylinder>& OtherCylinders, const swMatrix44& OtherTransform)
{
	return FindSeat(Cylinders, Transform, OtherCylinders, OtherTransform, nullptr);
}

bool swConnectivity::FindSeat(const std::vector<swConnectionCylinder>& Cylinders, const swMatrix44& Transform, const std::vector<swConnectionCylinder>& OtherCylinders, const swMatrix44& OtherTransform, swMatrix44* SeatFrame)
{
	for (const swConnectionCylinder& Cylinder : Cylinders)
	{
		const swMatrix44 Frame = swMultiply(swMatrix44LDrawToStudWorx(Cylinder.Transform), Transform);

		for (const swConnectionCylinder& OtherCylinder : OtherCylinders)
		{
			if (!AreCompatible(Cylinder, OtherCylinder) || !IsSeated(Cylinder, Frame, OtherCylinder, swMultiply(swMatrix44LDrawToStudWorx(OtherCylinder.Transform), OtherTransform)))
				continue;

			if (SeatFrame)
				*SeatFrame = Frame;

			return true;
		}
	}

	return false;
}

bool swConnectivity::AreConnected(const swConnectionShapes& Shapes, const swConnectionShapes& OtherShapes)
{
	for (const auto& [Cylinders, Transform] : Shapes)
		for (const auto& [OtherCylinders, OtherTransform] : OtherShapes)
			if (AreConnected(*Cylinders, Transform, *OtherCylinders, OtherTransform))
				return true;

	return false;
}

bool swConnectivity::GetConnectionAxis(const swConnectionShapes& Shapes, const swConnectionShapes& OtherShapes, swVector3& Axis, swVector3& Point)
{
	swMatrix44 SeatFrame;

	for (const auto& [Cylinders, Transform] : Shapes)
	{
		for (const auto& [OtherCylinders, OtherTransform] : OtherShapes)
		{
			if (!FindSeat(*Cylinders, Transform, *OtherCylinders, OtherTransform, &SeatFrame))
				continue;

			Axis = swNormalize(swMultiply30(swVector3(0.0f, 0.0f, 1.0f), SeatFrame));
			Point = SeatFrame.GetTranslation();
			return true;
		}
	}

	return false;
}

// use to name the shape at an end
static swConnectionShape swGetEndShape(const swConnectionCylinder& Cylinder, bool AtStart)
{
	const auto Find = [](auto First, auto Last)
	{
		for (auto Section = First; Section != Last; ++Section)
			if (!swIsExtension(*Section))
				return Section->Shape;

		return swConnectionShape::Round;
	};

	return AtStart ? Find(Cylinder.Sections.begin(), Cylinder.Sections.end()) : Find(Cylinder.Sections.rbegin(), Cylinder.Sections.rend());
}

// use to mark the hovered part
std::vector<swConnectionMarker> swConnectivity::GetMarkers(const swConnectionShapes& Shapes, const swVector3& Point, float Range, size_t Count)
{
	std::vector<std::pair<float, swConnectionMarker>> Found;

	for (const auto& [Cylinders, Transform] : Shapes)
	{
		for (const swConnectionCylinder& Cylinder : *Cylinders)
		{
			const swMatrix44 Frame = swMultiply(swMatrix44LDrawToStudWorx(Cylinder.Transform), Transform);
			const swVector3 Axis = swNormalize(swMultiply30(swVector3(0.0f, 0.0f, 1.0f), Frame));
			const auto [Start, End] = Cylinder.GetExtent();

			const auto AddMarker = [&](float Along, const swVector3& Direction, bool AtStart, bool TwoSided)
			{
				const swVector3 Position = Frame.GetTranslation() + Axis * Along;
				const float Distance = swLength(Position - Point);

				if (Distance <= Range)
					Found.push_back({ Distance, { Position, Direction, Cylinder.Gender, Cylinder.Kind, swGetEndShape(Cylinder, AtStart), Cylinder.GetRadius(), Cylinder.GetLength(), TwoSided } });
			};

			// joints and closed shapes show once at their middle
			if (Cylinder.Kind != swConnectionKind::Cylinder || (Cylinder.ClosedStart && Cylinder.ClosedEnd))
			{
				AddMarker((Start + End) / 2.0f, Axis, true, true);
				continue;
			}

			if (!Cylinder.ClosedStart)
				AddMarker(Start, -Axis, true, false);

			if (!Cylinder.ClosedEnd)
				AddMarker(End, Axis, false, false);
		}
	}

	std::stable_sort(Found.begin(), Found.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

	std::vector<swConnectionMarker> Markers;
	size_t MaleCount = 0, FemaleCount = 0;

	for (const auto& [Distance, Marker] : Found)
	{
		size_t& GenderCount = Marker.Gender == swConnectionGender::Male ? MaleCount : FemaleCount;

		if (GenderCount < Count)
		{
			GenderCount++;
			Markers.push_back(Marker);
		}
	}

	return Markers;
}

// use to square a frame
static void swGetFrameAxes(const swMatrix44& Frame, swVector3& X, swVector3& Y, swVector3& Z)
{
	Z = swNormalize(swMultiply30(swVector3(0.0f, 0.0f, 1.0f), Frame));
	X = swMultiply30(swVector3(1.0f, 0.0f, 0.0f), Frame);
	X = swNormalize(X - Z * swDot(X, Z));

	// mirrored frames keep their hand
	const float Hand = swDot(swMultiply30(swVector3(0.0f, 1.0f, 0.0f), Frame), swCross(Z, X)) < 0.0f ? -1.0f : 1.0f;
	Y = swCross(Z, X) * Hand;
}

// use to turn a vector about an axis
static swVector3 swRotateAbout(const swVector3& Vector, const swVector3& Axis, float Radians)
{
	const float Cosine = cosf(Radians);

	return Vector * Cosine + swCross(Axis, Vector) * sinf(Radians) + Axis * (swDot(Axis, Vector) * (1.0f - Cosine));
}

// use to find the shape under a grab
const swConnectionCylinder* swConnectivity::FindGrabbed(const swConnectionShapes& Shapes, const swVector3& Grab)
{
	// a grab on the shape or just beside it

	const float GrabTolerance = 4.0f;
	const swConnectionCylinder* Grabbed = nullptr;
	float BestDistance = FLT_MAX;

	for (const auto& [Cylinders, Transform] : Shapes)
	{
		if (!Cylinders)
			continue;

		for (const swConnectionCylinder& Cylinder : *Cylinders)
		{
			const swMatrix44 Frame = swMultiply(swMatrix44LDrawToStudWorx(Cylinder.Transform), Transform);
			const swVector3 Axis = swNormalize(swMultiply30(swVector3(0.0f, 0.0f, 1.0f), Frame));
			const auto [Start, End] = Cylinder.GetExtent();
			const swVector3 Offset = Grab - Frame.GetTranslation();
			const float Along = std::clamp(swDot(Offset, Axis), Start, End);
			const float Distance = swLength(Offset - Axis * Along) - Cylinder.GetRadius();

			if (Distance <= GrabTolerance && Distance < BestDistance)
			{
				BestDistance = Distance;
				Grabbed = &Cylinder;
			}
		}
	}

	return Grabbed;
}

// use to seat a dragged part
bool swConnectivity::FindSnap(const swConnectionShapes& MovingShapes, const swMatrix44& MovingTransform, const swConnectionShapes& TargetShapes, const swVector3& Point, float Range, swConnectionSnap& Snap, const std::optional<swVector3>& Grab)
{
	// a quarter turn weighs a stud
	const float TurnWeight = 20.0f / (SW_PI / 2.0f);
	const float QuarterTurn = SW_PI / 2.0f;
	double BestScore = DBL_MAX;

	// the shape the part was grabbed by connects first

	const swConnectionCylinder* Grabbed = Grab ? FindGrabbed(MovingShapes, *Grab) : nullptr;

	Snap.Cylinder = nullptr;

	for (const auto& [TargetCylinders, TargetTransform] : TargetShapes)
	{
		if (!TargetCylinders)
			continue;

		for (const swConnectionCylinder& Target : *TargetCylinders)
		{
			const swMatrix44 TargetFrame = swMultiply(swMatrix44LDrawToStudWorx(Target.Transform), TargetTransform);
			const swVector3 TargetPosition = TargetFrame.GetTranslation();
			const float Reach = swLength(TargetPosition - Point);

			if (Reach > Range)
				continue;

			swVector3 TargetX, TargetY, TargetZ;
			swGetFrameAxes(TargetFrame, TargetX, TargetY, TargetZ);
			const swVector3 TargetSide = swCross(TargetZ, TargetX);

			for (const auto& [MovingCylinders, MovingShapeTransform] : MovingShapes)
			{
				if (!MovingCylinders)
					continue;

				for (const swConnectionCylinder& Moving : *MovingCylinders)
				{
					if (!AreCompatible(Moving, Target))
						continue;

					const bool IsMale = Moving.Gender == swConnectionGender::Male;
					const std::vector<std::pair<float, float>> InsertRanges = IsMale ? GetInsertRanges(Moving, Target) : GetInsertRanges(Target, Moving);

					if (InsertRanges.empty())
						continue;

					const swMatrix44 LocalFrame = swMultiply(swMatrix44LDrawToStudWorx(Moving.Transform), MovingShapeTransform);
					const swMatrix44 Frame = swMultiply(LocalFrame, MovingTransform);
					const swVector3 Position = Frame.GetTranslation();
					swVector3 X, Y, Z;
					swGetFrameAxes(Frame, X, Y, Z);

					// the shortest turn onto the target axis
					const swVector3 TurnAxis = swCross(Z, TargetZ);
					const float TurnSine = swLength(TurnAxis);
					const float TurnAngle = atan2f(TurnSine, swDot(Z, TargetZ));
					swVector3 NewX = X;

					if (TurnSine > 0.0001f)
						NewX = swRotateAbout(X, TurnAxis / TurnSine, TurnAngle);
					else if (TurnAngle > QuarterTurn)
						NewX = -X;

					// the twist in quarter turns
					const float Twist = atan2f(swDot(NewX, TargetSide), swDot(NewX, TargetX));
					const float SquareTwist = roundf(Twist / QuarterTurn) * QuarterTurn;
					NewX = TargetX * cosf(SquareTwist) + TargetSide * sinf(SquareTwist);

					const float Hand = swDot(Y, swCross(Z, X)) < 0.0f ? -1.0f : 1.0f;
					swVector3 NewY = swCross(TargetZ, NewX) * Hand;
					swVector3 NewZ = TargetZ;

					// balls keep the part turn
					if (Moving.Kind == swConnectionKind::Ball)
					{
						NewX = X;
						NewY = Y;
						NewZ = Z;
					}

					// the depth in the nearest insert range
					const float Along = swDot(Position - TargetPosition, TargetZ);
					const float MaleOffset = IsMale ? Along : -Along;
					float SeatOffset = InsertRanges.front().first;

					for (const std::pair<float, float>& InsertRange : InsertRanges)
					{
						const float Clamped = std::clamp(MaleOffset, InsertRange.first, InsertRange.second);

						if (fabsf(Clamped - MaleOffset) < fabsf(SeatOffset - MaleOffset))
							SeatOffset = Clamped;
					}

					const swVector3 SeatPosition = TargetPosition + TargetZ * (IsMale ? SeatOffset : -SeatOffset);
					const float Trace = swDot(NewX, X) + swDot(NewY, Y) + swDot(NewZ, Z);
					const float Turn = acosf(std::clamp((Trace - 1.0f) / 2.0f, -1.0f, 1.0f));
					// the connection under the pointer first

					const double Score = (Grabbed && &Moving != Grabbed ? 1.0e7 : 0.0) + roundf(Reach) * 1000.0 + swLength(SeatPosition - Position) / 2.0f + Turn * TurnWeight;

					if (Score >= BestScore)
						continue;

					BestScore = Score;

					// turn about the shape, then seat it
					const swMatrix33 Delta = swMultiply(swMatrix33Transpose(swMatrix33(X, Y, Z)), swMatrix33(NewX, NewY, NewZ));
					const swVector3 Origin = swMultiply(MovingTransform.GetTranslation() - Position, Delta) + SeatPosition;

					Snap.Transform = swMatrix44(swMultiply(swMatrix33(MovingTransform), Delta), Origin);
					Snap.Frame = LocalFrame;
					Snap.Cylinder = &Moving;
				}
			}
		}
	}

	if (!Snap.Cylinder)
		return false;

	// one round axis turns as a hinge
	const auto IsRound = [](const swConnectionCylinder& Cylinder)
	{
		return !Cylinder.Sections.empty() && std::all_of(Cylinder.Sections.begin(), Cylinder.Sections.end(), [](const swConnectionSection& Section) { return Section.Shape == swConnectionShape::Round || swIsExtension(Section); });
	};

	const swMatrix44 SeatFrame = swMultiply(Snap.Frame, Snap.Transform);
	const swVector3 SeatAxis = swNormalize(swMultiply30(swVector3(0.0f, 0.0f, 1.0f), SeatFrame));
	// a ball turns freely, never as a hinge
	Snap.Hinge = Snap.Cylinder->Kind != swConnectionKind::Ball;

	for (const auto& [MovingCylinders, MovingShapeTransform] : MovingShapes)
	{
		if (!MovingCylinders)
			continue;

		for (const swConnectionCylinder& Moving : *MovingCylinders)
		{
			const swMatrix44 Frame = swMultiply(swMatrix44LDrawToStudWorx(Moving.Transform), swMultiply(MovingShapeTransform, Snap.Transform));

			for (const auto& [TargetCylinders, TargetTransform] : TargetShapes)
			{
				if (!TargetCylinders)
					continue;

				for (const swConnectionCylinder& Target : *TargetCylinders)
				{
					if (!AreCompatible(Moving, Target) || !IsSeated(Moving, Frame, Target, swMultiply(swMatrix44LDrawToStudWorx(Target.Transform), TargetTransform)))
						continue;

					const swVector3 Offset = Frame.GetTranslation() - SeatFrame.GetTranslation();
					const bool OnAxis = fabsf(swDot(swNormalize(swMultiply30(swVector3(0.0f, 0.0f, 1.0f), Frame)), SeatAxis)) > 0.99f && swLength(Offset - SeatAxis * swDot(Offset, SeatAxis)) < gSeatTolerance;

					if (!OnAxis || !IsRound(Moving) || !IsRound(Target))
						Snap.Hinge = false;
				}
			}
		}
	}

	return true;
}
