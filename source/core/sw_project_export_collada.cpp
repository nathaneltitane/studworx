#include "sw_global.h"
#include "sw_data_part.h"
#include "sw_project.h"
#include "sw_file_zip.h"
#include "sw_colors.h"
#include "sw_context.h"

swResult<void> Project::ExportCOLLADA(const QString& FileName)
{
	const std::vector<swModelPartsEntry> ModelParts = GetModelParts();

	QString SaveFileName = GetExportFileName(FileName, "dae", tr("Export COLLADA"), tr("COLLADA Files (*.dae);;All Files (*)"));

	if (SaveFileName.isEmpty())
		return swUnexpected(QString());

	QFile File(SaveFileName);

	if (!File.open(QIODevice::WriteOnly))
		return swUnexpected(tr("Could not open file '%1' for writing.").arg(SaveFileName));

	QTextStream Stream(&File);

	return WriteCOLLADA(Stream, ModelParts, false, false);
}

swResult<void> Project::ExportKMZ(const QString& FileName)
{
	const std::vector<swModelPartsEntry> ModelParts = GetModelParts();

	if (ModelParts.empty())
		return swUnexpected(tr("Nothing to export."));

	QString SaveFileName = GetExportFileName(FileName, "kmz", tr("Export KMZ"), tr("KMZ Files (*.kmz);;All Files (*)"));

	if (SaveFileName.isEmpty())
		return swUnexpected(QString());

	QByteArray COLLADA;
	QBuffer Buffer(&COLLADA);
	Buffer.open(QIODevice::WriteOnly);
	QTextStream Stream(&Buffer);

	swResult<void> Result = WriteCOLLADA(Stream, ModelParts, true, swGetPreferences().mKMZVisibleSurfaces);

	if (!Result)
		return Result;

	Stream.flush();

	// collada entry named after the kmz file with spaces as underscores
	const QString ModelFileName = QFileInfo(SaveFileName).completeBaseName().replace(QLatin1Char(' '), QLatin1Char('_')) + QLatin1String(".dae");

	// kml index pointing at the collada model - entries dated 2000-01-01 so exports repeat byte for byte
	const QByteArray KML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<kml><Placemark><Model><Link><href>" + ModelFileName.toHtmlEscaped().toUtf8() + "</href></Link></Model></Placemark></kml>\n";
	constexpr quint32 DosDate = ((2000 - 1980) << 25) | (1 << 21) | (1 << 16);
	swZipFile ZipFile;

	if (!ZipFile.OpenWrite(SaveFileName))
		return swUnexpected(tr("Could not open file '%1' for writing.").arg(SaveFileName));

	if (!ZipFile.AddFile("doc.kml", KML, DosDate) || !ZipFile.AddFile(ModelFileName.toUtf8().constData(), COLLADA, DosDate) || !ZipFile.CloseWrite())
		return swUnexpected(tr("Could not write file '%1'.").arg(SaveFileName));

	return swResult<void>();
}

swResult<void> Project::WriteCOLLADA(QTextStream& Stream, const std::vector<swModelPartsEntry>& ModelParts, bool FixedDates, bool VisibleSurfaces)
{
	const QString Date = FixedDates ? QStringLiteral("2000-01-01T00:00:00") : QDateTime::currentDateTime().toString(Qt::ISODate);

	Stream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	Stream << "<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">\n";

	Stream << "<asset>\n";
	Stream << "\t<created>" << Date << "</created>\n";
	Stream << "\t<modified>" << Date << "</modified>\n";
	Stream << "<unit name=\"StudWorx\" meter=\"0.0004\" />\n";
	Stream << "\t<up_axis>Z_UP</up_axis>\n";
	Stream << "</asset>\n";

	std::vector<QString> ColorIDs(gColorList.size());
	std::set<QString> UsedColorIDs;

	for (size_t ColorIndex = 0; ColorIndex < gColorList.size(); ColorIndex++)
	{
		const QString BaseName = QLatin1String("Color_") + MakeExportNameFragment(QString::fromLatin1(gColorList[ColorIndex].SafeName));
		QString Identifier = BaseName;

		if (UsedColorIDs.find(Identifier) != UsedColorIDs.end())
			Identifier += QLatin1Char('_') + QString::number(ColorIndex);

		ColorIDs[ColorIndex] = MakeUniqueExportIdentifier(Identifier, UsedColorIDs, QString());
	}

	const bool TransparentMarked = swGetPreferences().mMarkColladaTransparency;

	Stream << "<library_effects>\n";

	for (size_t ColorIndex = 0; ColorIndex < gColorList.size(); ColorIndex++)
	{
		const swColor& Color = gColorList[ColorIndex];
		const QString& ColorName = ColorIDs[ColorIndex];

		Stream << QString("\t<effect id=\"%1-phong\">\n").arg(ColorName);
		Stream << "\t\t<profile_COMMON>\n";
		Stream << "\t\t\t<technique sid=\"phong1\">\n";
		Stream << "\t\t\t\t<phong>\n";
		Stream << "\t\t\t\t\t<emission>\n";
		Stream << "\t\t\t\t\t\t<color>0.0 0.0 0.0 0.0</color>\n";
		Stream << "\t\t\t\t\t</emission>\n";
		Stream << "\t\t\t\t\t<ambient>\n";
		Stream << QString("\t\t\t\t\t\t<color>%1 %2 %3 1.0</color>\n").arg(QString::number(Color.Value[0]), QString::number(Color.Value[1]), QString::number(Color.Value[2]));
		Stream << "\t\t\t\t\t</ambient>\n";
		Stream << "\t\t\t\t\t<diffuse>\n";
		Stream << QString("\t\t\t\t\t\t<color>%1 %2 %3 1.0</color>\n").arg(QString::number(Color.Value[0]), QString::number(Color.Value[1]), QString::number(Color.Value[2]));
		Stream << "\t\t\t\t\t</diffuse>\n";
		Stream << "\t\t\t\t\t<specular>\n";
		Stream << "\t\t\t\t\t\t<color>0.9 0.9 0.9 1.0</color>\n";
		Stream << "\t\t\t\t\t</specular>\n";
		Stream << "\t\t\t\t\t<shininess>\n";
		Stream << "\t\t\t\t\t\t<float>20.0</float>\n";
		Stream << "\t\t\t\t\t</shininess>\n";
		Stream << (TransparentMarked ? "\t\t\t\t\t<transparent opaque=\"A_ONE\">\n" : "\t\t\t\t\t<transparent>\n");
		Stream << QString("\t\t\t\t\t\t<color>%1 %2 %3 %4</color>\n").arg(QString::number(Color.Value[0]), QString::number(Color.Value[1]), QString::number(Color.Value[2]), QString::number(Color.Value[3]));
		Stream << "\t\t\t\t\t</transparent>\n";
		Stream << "\t\t\t\t\t<transparency>\n";
		Stream << "\t\t\t\t\t\t<float>1.0</float>\n";
		Stream << "\t\t\t\t\t</transparency>\n";
		Stream << "\t\t\t\t</phong>\n";
		Stream << "\t\t\t</technique>\n";
		Stream << "\t\t</profile_COMMON>\n";
		Stream << "\t</effect>\n";
	}

	Stream << "</library_effects>\n";
	Stream << "<library_materials>\n";

	for (size_t ColorIndex = 0; ColorIndex < gColorList.size(); ColorIndex++)
	{
		const QString& ColorName = ColorIDs[ColorIndex];
		Stream << QString("\t<material id=\"%1-material\">\n").arg(ColorName);
		Stream << QString("\t\t<instance_effect url=\"#%1-phong\" />\n").arg(ColorName);
		Stream << "\t</material>\n";
	}

	Stream << "</library_materials>\n";

	if (VisibleSurfaces)
		return WriteCOLLADASurfaces(Stream, ModelParts, ColorIDs);

	Stream << "<library_geometries>\n";
	std::map<const swMesh*, QString> MeshIDs;
	std::set<QString> UsedMeshIDs;

	for (const swModelPartsEntry& ModelPart : ModelParts)
	{
		swMesh* Mesh = !ModelPart.Mesh ? ModelPart.PartData->GetMesh() : ModelPart.Mesh;

		if (!Mesh)
			continue;

		if (MeshIDs.find(Mesh) != MeshIDs.end())
			continue;

		const QString BaseName = QLatin1String("Part_") + MakeExportNameFragment(QString::fromUtf8(ModelPart.PartData->mFileName));
		const QString ID = MakeUniqueExportIdentifier(BaseName, UsedMeshIDs, QString());
		MeshIDs.emplace(Mesh, ID);
		const int NumVertices = Mesh->mNumVertices + Mesh->mNumTexturedVertices;

		Stream << QString("\t<geometry id=\"%1\">\n").arg(ID);
		Stream << "\t\t<mesh>\n";

		Stream << QString("\t\t\t<source id=\"%1-pos\">\n").arg(ID);
		Stream << QString("\t\t\t\t<float_array id=\"%1-pos-array\" count=\"%2\">\n").arg(ID, QString::number(NumVertices * 3));

		const swVertex* Verts = static_cast<const swVertex*>(Mesh->mVertexData);

		for (int VertexIdx = 0; VertexIdx < Mesh->mNumVertices; VertexIdx++)
		{
			const swVector3& Position = Verts[VertexIdx].Position;
			Stream << QString("\t\t\t\t\t%1 %2 %3\n").arg(QString::number(Position.x), QString::number(Position.y), QString::number(Position.z));
		}

		const swVertexTextured* TexturedVerts = reinterpret_cast<const swVertexTextured*>(Verts + Mesh->mNumVertices);

		for (int VertexIdx = 0; VertexIdx < Mesh->mNumTexturedVertices; VertexIdx++)
		{
			const swVector3& Position = TexturedVerts[VertexIdx].Position;
			Stream << QString("\t\t\t\t\t%1 %2 %3\n").arg(QString::number(Position.x), QString::number(Position.y), QString::number(Position.z));
		}

		Stream << "\t\t\t\t</float_array>\n";
		Stream << "\t\t\t\t<technique_common>\n";
		Stream << QString("\t\t\t\t\t<accessor source=\"#%1-pos-array\" count=\"%2\" stride=\"3\">\n").arg(ID, QString::number(NumVertices));
		Stream << "\t\t\t\t\t\t<param name=\"X\" type=\"float\" />\n";
		Stream << "\t\t\t\t\t\t<param name=\"Y\" type=\"float\" />\n";
		Stream << "\t\t\t\t\t\t<param name=\"Z\" type=\"float\" />\n";
		Stream << "\t\t\t\t\t</accessor>\n";
		Stream << "\t\t\t\t</technique_common>\n";
		Stream << "\t\t\t</source>\n";

		Stream << QString("\t\t\t<source id=\"%1-normal\">\n").arg(ID);
		Stream << QString("\t\t\t\t<float_array id=\"%1-normal-array\" count=\"%2\">\n").arg(ID, QString::number(NumVertices * 3));

		for (int VertexIdx = 0; VertexIdx < Mesh->mNumVertices; VertexIdx++)
		{
			swVector3 Normal = swUnpackNormal(Verts[VertexIdx].Normal);
			Stream << QString("\t\t\t\t\t%1 %2 %3\n").arg(QString::number(Normal.x), QString::number(Normal.y), QString::number(Normal.z));
		}

		for (int VertexIdx = 0; VertexIdx < Mesh->mNumTexturedVertices; VertexIdx++)
		{
			swVector3 Normal = swUnpackNormal(TexturedVerts[VertexIdx].Normal);
			Stream << QString("\t\t\t\t\t%1 %2 %3\n").arg(QString::number(Normal.x), QString::number(Normal.y), QString::number(Normal.z));
		}

		Stream << "\t\t\t\t</float_array>\n";
		Stream << "\t\t\t\t<technique_common>\n";
		Stream << QString("\t\t\t\t\t<accessor source=\"#%1-normal-array\" count=\"%2\" stride=\"3\">\n").arg(ID, QString::number(NumVertices));
		Stream << "\t\t\t\t\t\t<param name=\"X\" type=\"float\" />\n";
		Stream << "\t\t\t\t\t\t<param name=\"Y\" type=\"float\" />\n";
		Stream << "\t\t\t\t\t\t<param name=\"Z\" type=\"float\" />\n";
		Stream << "\t\t\t\t\t</accessor>\n";
		Stream << "\t\t\t\t</technique_common>\n";
		Stream << "\t\t\t</source>\n";

		Stream << QString("\t\t\t<vertices id=\"%1-vertices\">\n").arg(ID);
		Stream << QString("\t\t\t\t<input semantic=\"POSITION\" source=\"#%1-pos\"/>\n").arg(ID);
		Stream << "\t\t\t</vertices>\n";

		for (int SectionIdx = 0; SectionIdx < Mesh->mLods[SW_MESH_LOD_HIGH].NumSections; SectionIdx++)
		{
			swMeshSection* Section = &Mesh->mLods[SW_MESH_LOD_HIGH].Sections[SectionIdx];
			int Offset = 0;

			if (Section->PrimitiveType == SW_MESH_TRIANGLES)
				Offset = 0;
			else if (Section->PrimitiveType == SW_MESH_TEXTURED_TRIANGLES)
				Offset = Mesh->mNumVertices;
			else
				continue;

			const QString& ColorName = ColorIDs[Section->ColorIndex];

			if (Mesh->mIndexType == GL_UNSIGNED_SHORT)
			{
				quint16* Indices = (quint16*)Mesh->mIndexData + Section->DrawOffset / sizeof(quint16);

				Stream << QString("\t\t\t<triangles count=\"%1\" material=\"%2\">\n").arg(QString::number(Section->DrawCount / 3), ColorName);
				Stream << QString("\t\t\t<input semantic=\"VERTEX\" source=\"#%1-vertices\" offset=\"0\" />\n").arg(ID);
				Stream << QString("\t\t\t<input semantic=\"NORMAL\" source=\"#%1-normal\" offset=\"0\" />\n").arg(ID);
				Stream << "\t\t\t<p>\n";

				for (int Idx = 0; Idx < Section->DrawCount; Idx += 3)
				{
					QString idx1 = QString::number(Indices[Idx + 0] + Offset);
					QString idx2 = QString::number(Indices[Idx + 1] + Offset);
					QString idx3 = QString::number(Indices[Idx + 2] + Offset);

					Stream << QString("\t\t\t\t %1 %2 %3\n").arg(idx1, idx2, idx3);
				}
			}
			else
			{
				quint32* Indices = (quint32*)Mesh->mIndexData + Section->DrawOffset / sizeof(quint32);

				Stream << QString("\t\t\t<triangles count=\"%1\" material=\"%2\">\n").arg(QString::number(Section->DrawCount / 3), ColorName);
				Stream << QString("\t\t\t<input semantic=\"VERTEX\" source=\"#%1-vertices\" offset=\"0\" />\n").arg(ID);
				Stream << QString("\t\t\t<input semantic=\"NORMAL\" source=\"#%1-normal\" offset=\"0\" />\n").arg(ID);
				Stream << "\t\t\t<p>\n";

				for (int Idx = 0; Idx < Section->DrawCount; Idx += 3)
				{
					QString idx1 = QString::number(Indices[Idx + 0] + Offset);
					QString idx2 = QString::number(Indices[Idx + 1] + Offset);
					QString idx3 = QString::number(Indices[Idx + 2] + Offset);

					Stream << QString("\t\t\t\t %1 %2 %3\n").arg(idx1, idx2, idx3);
				}
			}

			Stream << "\t\t\t\t</p>\n";
			Stream << "\t\t\t</triangles>\n";
		}

		Stream << "\t\t</mesh>\n";
		Stream << "\t</geometry>\n";
	}

	Stream << "</library_geometries>\n";
	Stream << "<library_visual_scenes>\n";
	Stream << "\t<visual_scene id=\"DefaultScene\">\n";

	for (const swModelPartsEntry& ModelPart : ModelParts)
	{
		swMesh* Mesh = !ModelPart.Mesh ? ModelPart.PartData->GetMesh() : ModelPart.Mesh;

		if (!Mesh)
			continue;

		const QString& ID = MeshIDs.at(Mesh);

		Stream << "\t\t<node>\n";
		Stream << "\t\t\t<matrix>\n";

		const swMatrix44& Matrix = ModelPart.WorldMatrix;
		Stream << QString("\t\t\t\t%1 %2 %3 %4\n").arg(QString::number(Matrix[0][0]), QString::number(Matrix[1][0]), QString::number(Matrix[2][0]), QString::number(Matrix[3][0]));
		Stream << QString("\t\t\t\t%1 %2 %3 %4\n").arg(QString::number(Matrix[0][1]), QString::number(Matrix[1][1]), QString::number(Matrix[2][1]), QString::number(Matrix[3][1]));
		Stream << QString("\t\t\t\t%1 %2 %3 %4\n").arg(QString::number(Matrix[0][2]), QString::number(Matrix[1][2]), QString::number(Matrix[2][2]), QString::number(Matrix[3][2]));
		Stream << QString("\t\t\t\t%1 %2 %3 %4\n").arg(QString::number(Matrix[0][3]), QString::number(Matrix[1][3]), QString::number(Matrix[2][3]), QString::number(Matrix[3][3]));

		Stream << "\t\t\t</matrix>\n";
		Stream << QString("\t\t\t<instance_geometry url=\"#%1\">\n").arg(ID);
		Stream << "\t\t\t\t<bind_material>\n";
		Stream << "\t\t\t\t\t<technique_common>\n";

		for (int SectionIdx = 0; SectionIdx < Mesh->mLods[SW_MESH_LOD_HIGH].NumSections; SectionIdx++)
		{
			swMeshSection* Section = &Mesh->mLods[SW_MESH_LOD_HIGH].Sections[SectionIdx];

			if (Section->PrimitiveType != SW_MESH_TRIANGLES && Section->PrimitiveType != SW_MESH_TEXTURED_TRIANGLES)
				continue;

			const QString& SourceColorName = ColorIDs[Section->ColorIndex];
			const QString* TargetColorName;
			if (Section->ColorIndex == gDefaultColor)
				TargetColorName = &ColorIDs[ModelPart.ColorIndex];
			else
				TargetColorName = &SourceColorName;

			Stream << QString("\t\t\t\t\t\t<instance_material symbol=\"%1\" target=\"#%2-material\"/>\n").arg(SourceColorName, *TargetColorName);
		}

		Stream << "\t\t\t\t\t</technique_common>\n";
		Stream << "\t\t\t\t</bind_material>\n";
		Stream << "\t\t\t</instance_geometry>\n";
		Stream << "\t\t</node>\n";
	}

	Stream << "\t</visual_scene>\n";
	Stream << "</library_visual_scenes>\n";
	Stream << "<scene>\n";
	Stream << "\t<instance_visual_scene url=\"#DefaultScene\"/>\n";
	Stream << "</scene>\n";

	Stream << "</COLLADA>\n";

	return swResult<void>();
}

// Use to merge the parts into one mesh per color without the faces hidden from every direction.
swResult<void> Project::WriteCOLLADASurfaces(QTextStream& Stream, const std::vector<swModelPartsEntry>& ModelParts, const std::vector<QString>& ColorIDs)
{
	std::vector<swVector3> Positions;
	std::vector<swVector3> Normals;
	std::vector<int> TriangleColors;
	std::vector<bool> TriangleTranslucent;
	std::vector<int> ColorOrder;

	for (const swModelPartsEntry& ModelPart : ModelParts)
	{
		const swMesh* Mesh = !ModelPart.Mesh ? ModelPart.PartData->GetMesh() : ModelPart.Mesh;

		if (!Mesh)
			continue;

		const swVertex* Verts = static_cast<const swVertex*>(Mesh->mVertexData);
		const swVertexTextured* TexturedVerts = reinterpret_cast<const swVertexTextured*>(Verts + Mesh->mNumVertices);

		for (int SectionIdx = 0; SectionIdx < Mesh->mLods[SW_MESH_LOD_HIGH].NumSections; SectionIdx++)
		{
			const swMeshSection* Section = &Mesh->mLods[SW_MESH_LOD_HIGH].Sections[SectionIdx];

			if (Section->PrimitiveType != SW_MESH_TRIANGLES && Section->PrimitiveType != SW_MESH_TEXTURED_TRIANGLES)
				continue;

			const int ColorIndex = Section->ColorIndex == gDefaultColor ? ModelPart.ColorIndex : Section->ColorIndex;
			const bool Translucent = swIsColorTranslucent(ColorIndex);

			if (std::find(ColorOrder.begin(), ColorOrder.end(), ColorIndex) == ColorOrder.end())
				ColorOrder.push_back(ColorIndex);

			for (int Idx = 0; Idx < Section->DrawCount; Idx++)
			{
				const quint32 Index = Mesh->mIndexType == GL_UNSIGNED_SHORT ? ((const quint16*)Mesh->mIndexData)[Section->DrawOffset / sizeof(quint16) + Idx] : ((const quint32*)Mesh->mIndexData)[Section->DrawOffset / sizeof(quint32) + Idx];
				const swVector3& Position = Section->PrimitiveType == SW_MESH_TRIANGLES ? Verts[Index].Position : TexturedVerts[Index].Position;
				const quint32 Normal = Section->PrimitiveType == SW_MESH_TRIANGLES ? Verts[Index].Normal : TexturedVerts[Index].Normal;

				Positions.push_back(swMultiply31(Position, ModelPart.WorldMatrix));
				Normals.push_back(swNormalize(swMultiply30(swUnpackNormal(Normal), ModelPart.WorldMatrix)));
			}

			TriangleColors.insert(TriangleColors.end(), Section->DrawCount / 3, ColorIndex);
			TriangleTranslucent.insert(TriangleTranslucent.end(), Section->DrawCount / 3, Translucent);
		}
	}

	swContext* Context = swContext::GetGlobalOffscreenContext();

	if (!Context)
		return swUnexpected(tr("Could not create the OpenGL context."));

	const std::vector<bool> Visible = Context->FindVisibleTriangles(Positions, TriangleTranslucent);

	if (Visible.size() != TriangleColors.size())
		return swUnexpected(tr("Could not find the visible surfaces."));

	// fixed decimals - positions in hundredths and normals in thousandths
	const auto FormatValue = [](qint64 Value, double Scale, int Decimals)
	{
		if (!Value)
			return QStringLiteral("0");

		QString Text = QString::number(Value / Scale, 'f', Decimals);

		while (Text.endsWith(QLatin1Char('0')))
			Text.chop(1);

		if (Text.endsWith(QLatin1Char('.')))
			Text.chop(1);

		return Text;
	};

	QString Nodes;

	Stream << "<library_geometries>\n";

	for (size_t MaterialIndex = 0; MaterialIndex < ColorOrder.size(); MaterialIndex++)
	{
		const int ColorIndex = ColorOrder[MaterialIndex];
		std::map<std::array<qint64, 6>, int> VertexIndices;
		std::vector<std::array<qint64, 6>> VertexValues;
		std::vector<int> Indices;

		for (size_t TriangleIndex = 0; TriangleIndex < TriangleColors.size(); TriangleIndex++)
		{
			if (!Visible[TriangleIndex] || TriangleColors[TriangleIndex] != ColorIndex)
				continue;

			for (size_t Corner = TriangleIndex * 3; Corner < TriangleIndex * 3 + 3; Corner++)
			{
				const swVector3& Position = Positions[Corner];
				const swVector3& Normal = Normals[Corner];
				const std::array<qint64, 6> Key = { qRound64(Position.x * 100.0), qRound64(Position.y * 100.0), qRound64(Position.z * 100.0), qRound64(Normal.x * 1000.0), qRound64(Normal.y * 1000.0), qRound64(Normal.z * 1000.0) };
				const auto Insert = VertexIndices.emplace(Key, static_cast<int>(VertexValues.size()));

				if (Insert.second)
					VertexValues.push_back(Key);

				Indices.push_back(Insert.first->second);
			}
		}

		if (Indices.empty())
			continue;

		const QString ID = QString("mesh-%1").arg(MaterialIndex);
		const QString VertexCount = QString::number(VertexValues.size());

		Stream << QString("\t<geometry id=\"%1\">\n").arg(ID);
		Stream << "\t\t<mesh>\n";

		for (int Source = 0; Source < 2; Source++)
		{
			const QString SourceID = ID + (Source ? QLatin1String("-normal") : QLatin1String("-pos"));

			Stream << QString("\t\t\t<source id=\"%1\">\n").arg(SourceID);
			Stream << QString("\t\t\t\t<float_array id=\"%1-array\" count=\"%2\">\n").arg(SourceID, QString::number(VertexValues.size() * 3));

			for (const std::array<qint64, 6>& Values : VertexValues)
			{
				if (Source)
					Stream << QString("\t\t\t\t\t%1 %2 %3\n").arg(FormatValue(Values[3], 1000.0, 3), FormatValue(Values[4], 1000.0, 3), FormatValue(Values[5], 1000.0, 3));
				else
					Stream << QString("\t\t\t\t\t%1 %2 %3\n").arg(FormatValue(Values[0], 100.0, 2), FormatValue(Values[1], 100.0, 2), FormatValue(Values[2], 100.0, 2));
			}

			Stream << "\t\t\t\t</float_array>\n";
			Stream << "\t\t\t\t<technique_common>\n";
			Stream << QString("\t\t\t\t\t<accessor source=\"#%1-array\" count=\"%2\" stride=\"3\">\n").arg(SourceID, VertexCount);
			Stream << "\t\t\t\t\t\t<param name=\"X\" type=\"float\" />\n";
			Stream << "\t\t\t\t\t\t<param name=\"Y\" type=\"float\" />\n";
			Stream << "\t\t\t\t\t\t<param name=\"Z\" type=\"float\" />\n";
			Stream << "\t\t\t\t\t</accessor>\n";
			Stream << "\t\t\t\t</technique_common>\n";
			Stream << "\t\t\t</source>\n";
		}

		Stream << QString("\t\t\t<vertices id=\"%1-vertices\">\n").arg(ID);
		Stream << QString("\t\t\t\t<input semantic=\"POSITION\" source=\"#%1-pos\"/>\n").arg(ID);
		Stream << "\t\t\t</vertices>\n";
		Stream << QString("\t\t\t<triangles count=\"%1\" material=\"surface\">\n").arg(QString::number(Indices.size() / 3));
		Stream << QString("\t\t\t<input semantic=\"VERTEX\" source=\"#%1-vertices\" offset=\"0\" />\n").arg(ID);
		Stream << QString("\t\t\t<input semantic=\"NORMAL\" source=\"#%1-normal\" offset=\"0\" />\n").arg(ID);
		Stream << "\t\t\t<p>\n";

		for (size_t Idx = 0; Idx < Indices.size(); Idx += 3)
			Stream << QString("\t\t\t\t %1 %2 %3\n").arg(QString::number(Indices[Idx]), QString::number(Indices[Idx + 1]), QString::number(Indices[Idx + 2]));

		Stream << "\t\t\t\t</p>\n";
		Stream << "\t\t\t</triangles>\n";
		Stream << "\t\t</mesh>\n";
		Stream << "\t</geometry>\n";

		Nodes += "\t\t<node>\n";
		Nodes += QString("\t\t\t<instance_geometry url=\"#%1\">\n").arg(ID);
		Nodes += QString("\t\t\t\t<bind_material><technique_common><instance_material symbol=\"surface\" target=\"#%1-material\"/></technique_common></bind_material>\n").arg(ColorIDs[ColorIndex]);
		Nodes += "\t\t\t</instance_geometry>\n";
		Nodes += "\t\t</node>\n";
	}

	Stream << "</library_geometries>\n";
	Stream << "<library_visual_scenes>\n";
	Stream << "\t<visual_scene id=\"DefaultScene\">\n";
	Stream << Nodes;
	Stream << "\t</visual_scene>\n";
	Stream << "</library_visual_scenes>\n";
	Stream << "<scene>\n";
	Stream << "\t<instance_visual_scene url=\"#DefaultScene\"/>\n";
	Stream << "</scene>\n";
	Stream << "</COLLADA>\n";

	return swResult<void>();
}
