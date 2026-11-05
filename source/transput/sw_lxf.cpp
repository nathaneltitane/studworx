#include "sw_global.h"
#include "sw_lxf.h"
#include "sw_library.h"
#include "sw_application.h"
#include "sw_window_main.h"
#include "sw_object_part.h"
#include "sw_file.h"
#include "sw_colors.h"
#include <QDomDocument>

// lego digital designer to ldraw mappings, lists keep the source order
static const QString swDesignerMappingResource = QStringLiteral(":/resources/data/lego-digital-designer.json");
static const QString swDesignerKeyMaterials = QStringLiteral("Materials");
static const QString swDesignerKeyBricks = QStringLiteral("Bricks");
static const QString swDesignerKeyTransformations = QStringLiteral("Transformations");
static constexpr int swDesignerTransformationSize = 7;

static QJsonObject swLoadDesignerMapping()
{
	QFile File(swDesignerMappingResource);

	if (!File.open(QIODevice::ReadOnly))
		return QJsonObject();

	return QJsonDocument::fromJson(File.readAll()).object();
}

// use to pick the last listed part found in the library, else the last listed
static QString swGetDesignerPart(const QJsonArray& Candidates, Project* Project)
{
	for (qsizetype CandidateIdx = Candidates.size() - 1; CandidateIdx >= 0; CandidateIdx--)
		if (swGetPartsLibrary()->FindPartData(Candidates[CandidateIdx].toString().toLatin1().constData(), Project))
			return Candidates[CandidateIdx].toString();

	return Candidates.isEmpty() ? QString() : Candidates.last().toString();
}

// use to pick the last listed known color, else the last listed
static int swGetDesignerColor(const QJsonArray& Candidates)
{
	for (qsizetype CandidateIdx = Candidates.size() - 1; CandidateIdx >= 0; CandidateIdx--)
	{
		const int ColorCode = Candidates[CandidateIdx].toInt();

		for (const swColor& Color : gColorList)
			if (static_cast<int>(Color.Code) == ColorCode)
				return ColorCode;
	}

	return Candidates.isEmpty() ? 16 : Candidates.last().toInt();
}

bool swImportLXFMLFile(const QString& FileData, Project* Project, std::vector<swPart*>& Parts, std::vector<std::vector<swPart*>>& Groups)
{
	const QJsonObject Mapping = swLoadDesignerMapping();

	if (Mapping.isEmpty())
		return false;

	const QJsonObject Materials = Mapping[swDesignerKeyMaterials].toObject();
	const QJsonObject Bricks = Mapping[swDesignerKeyBricks].toObject();
	const QJsonObject Transformations = Mapping[swDesignerKeyTransformations].toObject();

	QDomDocument Document;
	Document.setContent(FileData);

	QDomElement Root = Document.documentElement();
	if (Root.tagName() != QLatin1String("LXFML"))
		return false;

	for (QDomNode SectionNode = Root.firstChild(); !SectionNode.isNull(); SectionNode = SectionNode.nextSibling())
	{
		QDomElement SectionElement = SectionNode.toElement();
		if (SectionElement.tagName() != QLatin1String("Bricks"))
			continue;

		for (QDomNode BrickNode = SectionElement.firstChild(); !BrickNode.isNull(); BrickNode = BrickNode.nextSibling())
		{
			QDomElement BrickElement = BrickNode.toElement();
			if (BrickElement.tagName() != QLatin1String("Brick"))
				continue;

			int NumBrickParts = 0;

			for (QDomNode PartNode = BrickElement.firstChild(); !PartNode.isNull(); PartNode = PartNode.nextSibling())
			{
				QDomElement PartElement = PartNode.toElement();
				if (PartElement.tagName() != QLatin1String("Part"))
					continue;

				bool BoneFound = false;
				swMatrix44 WorldMatrix;

				for (QDomNode BoneNode = PartElement.firstChild(); !BoneNode.isNull(); BoneNode = BoneNode.nextSibling())
				{
					QDomElement BoneElement = BoneNode.toElement();
					if (BoneElement.tagName() != QLatin1String("Bone"))
						continue;

					QString BoneTransform = BoneElement.attribute(QLatin1String("transformation"));
					QStringList BoneElements = BoneTransform.split(',');

					if (BoneElements.size() != 12)
						continue;

					WorldMatrix[0][0] = BoneElements[0].toFloat();
					WorldMatrix[0][1] = BoneElements[1].toFloat();
					WorldMatrix[0][2] = BoneElements[2].toFloat();
					WorldMatrix[0][3] = 0.0f;
					WorldMatrix[1][0] = BoneElements[3].toFloat();
					WorldMatrix[1][1] = BoneElements[4].toFloat();
					WorldMatrix[1][2] = BoneElements[5].toFloat();
					WorldMatrix[1][3] = 0.0f;
					WorldMatrix[2][0] = BoneElements[6].toFloat();
					WorldMatrix[2][1] = BoneElements[7].toFloat();
					WorldMatrix[2][2] = BoneElements[8].toFloat();
					WorldMatrix[2][3] = 0.0f;
					WorldMatrix[3][0] = BoneElements[9].toFloat();
					WorldMatrix[3][1] = BoneElements[10].toFloat();
					WorldMatrix[3][2] = BoneElements[11].toFloat();
					WorldMatrix[3][3] = 1.0f;

					BoneFound = true;
					break;
				}

				if (!BoneFound)
					continue;

				QString LegoID = PartElement.attribute(QLatin1String("designID"));
				int Material = PartElement.attribute(QLatin1String("materials")).split(',').first().toInt();

				// unlisted designs share their ldraw number
				QString PartName = swGetDesignerPart(Bricks[QString::number(LegoID.toInt())].toArray(), Project);

				if (PartName.isEmpty())
					PartName = LegoID + QLatin1String(".dat");

				swPartDataReference PartData = swGetPartsLibrary()->AcquirePartData(PartName.toLatin1().constData(), Project, true, false);
				const int ColorCode = swGetDesignerColor(Materials[QString::number(Material)].toArray());
				const QJsonArray Offset = Transformations[PartName].toArray();

				if (Offset.size() == swDesignerTransformationSize)
				{
					const swVector3 TransformTranslation(Offset[0].toDouble(), Offset[1].toDouble(), Offset[2].toDouble());
					const swVector4 AxisAngle(Offset[3].toDouble(), Offset[4].toDouble(), Offset[5].toDouble(), Offset[6].toDouble());

					swMatrix44 Transform = swMatrix44FromAxisAngle(swVector3(AxisAngle[0], AxisAngle[1], AxisAngle[2]), -AxisAngle[3]);
					Transform.SetTranslation(swMultiply30(-TransformTranslation, Transform));

					WorldMatrix = swMultiply(Transform, WorldMatrix);
				}

				WorldMatrix = swMatrix44(swVector4(WorldMatrix[0][0], -WorldMatrix[0][1], -WorldMatrix[0][2], 0.0f), swVector4(-WorldMatrix[1][0], WorldMatrix[1][1], WorldMatrix[1][2], 0.0f),
										 swVector4(-WorldMatrix[2][0], WorldMatrix[2][1], WorldMatrix[2][2], 0.0f), swVector4(WorldMatrix[3][0] * 25.0f, -WorldMatrix[3][1] * 25.0f, -WorldMatrix[3][2] * 25.0f, 1.0f));

				swPart* Part = new swPart(nullptr);
				Part->SetPartData(std::move(PartData), QString(), false, true);
				Part->Initialize(swMatrix44LDrawToStudWorx(WorldMatrix), 1);
				Part->SetColorCode(ColorCode);
				Parts.push_back(Part);
				NumBrickParts++;
			}

			if (NumBrickParts > 1)
			{
				std::vector<swPart*> Group;
				for (size_t PartIdx = Parts.size() - NumBrickParts; PartIdx < Parts.size(); PartIdx++)
					Group.push_back(Parts[PartIdx]);
				Groups.push_back(Group);
			}
		}
	}

	return true;
}
