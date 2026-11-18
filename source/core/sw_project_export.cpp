#include "sw_global.h"
#include "sw_data_part.h"
#include "sw_project.h"
#include "sw_window_main.h"
#include "sw_library.h"
#include "sw_file.h"
#include "sw_dialog_instructions_page.h"
#include "sw_command_meta.h"
#include "sw_colors.h"

swResult<void> Project::ExportCurrentStep(const QString& FileName)
{
	const swModel* Model = swGetActiveModel();
	const swResult<QByteArray> Bytes = SerializeModel(Model, false, Model->GetCurrentStep());

	if (!Bytes)
		return swUnexpected(Bytes.error());

	// the blender add-on reads leocad metas
	const QByteArray Output = swWriteCommandMeta(Bytes.value(), swCommandMetaFormat::LeoCAD);
	QSaveFile File(FileName);

	if (!File.open(QIODevice::WriteOnly) || File.write(Output) != Output.size() || !File.commit())
		return swUnexpected(tr("Could not write file '%1':\n%2").arg(FileName, File.errorString()));

	swSetProfileString(SW_PROFILE_PROJECTS_PATH, QFileInfo(FileName).absolutePath());
	return swResult<void>();
}

swResult<void> Project::ExportModel(const QString& FileName, swModel* Model) const
{
	const swResult<QByteArray> Bytes = SerializeModel(Model, false, 0);

	if (!Bytes)
		return swUnexpected(Bytes.error());

	QSaveFile File(FileName);

	if (!File.open(QIODevice::WriteOnly) || File.write(Bytes.value()) != Bytes.value().size() || !File.commit())
		return swUnexpected(tr("Could not write file '%1':\n%2").arg(FileName, File.errorString()));

	return swResult<void>();
}

QString Project::MakeExportNameFragment(const QString& Name)
{
	QString Identifier;
	bool AddSeparator = false;

	for (QChar Character : Name)
	{
		const ushort Value = Character.unicode();
		const bool IsAsciiLetter = (Value >= 'A' && Value <= 'Z') || (Value >= 'a' && Value <= 'z');
		const bool IsAsciiDigit = Value >= '0' && Value <= '9';

		if (IsAsciiLetter || IsAsciiDigit)
		{
			if (AddSeparator && !Identifier.isEmpty() && !Identifier.endsWith(QLatin1Char('_')))
				Identifier.append(QLatin1Char('_'));

			Identifier.append(Character);
			AddSeparator = false;
		}
		else
			AddSeparator = true;
	}

	if (Identifier.isEmpty())
		Identifier = QLatin1String("Unnamed");

	if (Identifier.size() > 80)
	{
		const QByteArray Hash = QCryptographicHash::hash(Name.toUtf8(), QCryptographicHash::Sha256).toHex();
		Identifier = Identifier.left(67) + QLatin1Char('_') + QString::fromLatin1(Hash.left(12));
	}

	return Identifier;
}

QString Project::MakeUniqueExportIdentifier(const QString& BaseName, std::set<QString>& UsedNames, const QString& ReservedSuffix)
{
	QString Identifier = BaseName;
	int Suffix = 2;

	while (UsedNames.find(Identifier) != UsedNames.end() ||
		(!ReservedSuffix.isEmpty() && UsedNames.find(Identifier + ReservedSuffix) != UsedNames.end()))
		Identifier = BaseName + QLatin1Char('_') + QString::number(Suffix++);

	UsedNames.insert(Identifier);

	if (!ReservedSuffix.isEmpty())
		UsedNames.insert(Identifier + ReservedSuffix);

	return Identifier;
}

QString Project::GetExportFileName(const QString& FileName, const QString& DefaultExtension, const QString& DialogTitle, const QString& DialogFilter) const
{
	if (!FileName.isEmpty())
		return FileName;

	QString SaveFileName;

	if (!mFileName.isEmpty())
		SaveFileName = mFileName;
	else
		SaveFileName = GetTitle();

	QString Extension = QFileInfo(SaveFileName).suffix().toLower();

	if (Extension.isEmpty())
		SaveFileName += "." + DefaultExtension;
	else if (Extension != DefaultExtension && SaveFileName.length() > 4)
	{
		SaveFileName = SaveFileName.left(SaveFileName.length() - Extension.length() - 1);
		SaveFileName += "." + DefaultExtension;
	}

	return QFileDialog::getSaveFileName(gMainWindow, DialogTitle, SaveFileName, DialogFilter);
}

swResult<void> Project::Export3DStudio(const QString& FileName)
{
	const std::vector<swModelPartsEntry> ModelParts = GetModelParts();

	QString SaveFileName = GetExportFileName(FileName, "3ds", tr("Export 3D Studio"), tr("3DS Files (*.3ds);;All Files (*)"));

	if (SaveFileName.isEmpty())
		return swUnexpected(QString());

	swDiskFile File(SaveFileName);

	if (!File.Open(QIODevice::WriteOnly))
		return swUnexpected(tr("Could not open file '%1' for writing.").arg(SaveFileName));

	long M3DStart = File.GetPosition();
	File.WriteU16(0x4D4D); // CHK_M3DMAGIC
	File.WriteU32(0);

	File.WriteU16(0x0002); // CHK_M3D_VERSION
	File.WriteU32(10);
	File.WriteU32(3);

	long MDataStart = File.GetPosition();
	File.WriteU16(0x3D3D); // CHK_MDATA
	File.WriteU32(0);

	File.WriteU16(0x3D3E); // CHK_MESH_VERSION
	File.WriteU32(10);
	File.WriteU32(3);

	const int MaterialNameLength = 11;
	char MaterialName[32];

	for (size_t ColorIdx = 0; ColorIdx < gColorList.size(); ColorIdx++)
	{
		swColor* Color = &gColorList[ColorIdx];

		snprintf(MaterialName, sizeof(MaterialName), "Material%03d", (int)ColorIdx);

		long MaterialStart = File.GetPosition();
		File.WriteU16(0xAFFF); // CHK_MAT_ENTRY
		File.WriteU32(0);

		File.WriteU16(0xA000); // CHK_MAT_NAME
		File.WriteU32(6 + MaterialNameLength + 1);
		File.WriteBuffer(MaterialName, MaterialNameLength + 1);

		File.WriteU16(0xA010); // CHK_MAT_AMBIENT
		File.WriteU32(24);

		File.WriteU16(0x0011); // CHK_COLOR_24
		File.WriteU32(9);

		File.WriteU8(floor(255.0 * Color->Value[0] + 0.5));
		File.WriteU8(floor(255.0 * Color->Value[1] + 0.5));
		File.WriteU8(floor(255.0 * Color->Value[2] + 0.5));

		File.WriteU16(0x0012); // CHK_LIN_COLOR_24
		File.WriteU32(9);

		File.WriteU8(floor(255.0 * Color->Value[0] + 0.5));
		File.WriteU8(floor(255.0 * Color->Value[1] + 0.5));
		File.WriteU8(floor(255.0 * Color->Value[2] + 0.5));

		File.WriteU16(0xA020); // CHK_MAT_AMBIENT
		File.WriteU32(24);

		File.WriteU16(0x0011); // CHK_COLOR_24
		File.WriteU32(9);

		File.WriteU8(floor(255.0 * Color->Value[0] + 0.5));
		File.WriteU8(floor(255.0 * Color->Value[1] + 0.5));
		File.WriteU8(floor(255.0 * Color->Value[2] + 0.5));

		File.WriteU16(0x0012); // CHK_LIN_COLOR_24
		File.WriteU32(9);

		File.WriteU8(floor(255.0 * Color->Value[0] + 0.5));
		File.WriteU8(floor(255.0 * Color->Value[1] + 0.5));
		File.WriteU8(floor(255.0 * Color->Value[2] + 0.5));

		File.WriteU16(0xA030); // CHK_MAT_SPECULAR
		File.WriteU32(24);

		File.WriteU16(0x0011); // CHK_COLOR_24
		File.WriteU32(9);

		File.WriteU8(floor(255.0 * 0.9f + 0.5));
		File.WriteU8(floor(255.0 * 0.9f + 0.5));
		File.WriteU8(floor(255.0 * 0.9f + 0.5));

		File.WriteU16(0x0012); // CHK_LIN_COLOR_24
		File.WriteU32(9);

		File.WriteU8(floor(255.0 * 0.9f + 0.5));
		File.WriteU8(floor(255.0 * 0.9f + 0.5));
		File.WriteU8(floor(255.0 * 0.9f + 0.5));

		File.WriteU16(0xA040); // CHK_MAT_SHININESS
		File.WriteU32(14);

		File.WriteU16(0x0030); // CHK_INT_PERCENTAGE
		File.WriteU32(8);

		File.WriteS16((quint8)floor(100.0 * 0.25 + 0.5));

		File.WriteU16(0xA041); // CHK_MAT_SHIN2PCT
		File.WriteU32(14);

		File.WriteU16(0x0030); // CHK_INT_PERCENTAGE
		File.WriteU32(8);

		File.WriteS16((quint8)floor(100.0 * 0.05 + 0.5));

		File.WriteU16(0xA050); // CHK_MAT_TRANSPARENCY
		File.WriteU32(14);

		File.WriteU16(0x0030); // CHK_INT_PERCENTAGE
		File.WriteU32(8);

		File.WriteS16((quint8)floor(100.0 * (1.0f - Color->Value[3]) + 0.5));

		File.WriteU16(0xA052); // CHK_MAT_XPFALL
		File.WriteU32(14);

		File.WriteU16(0x0030); // CHK_INT_PERCENTAGE
		File.WriteU32(8);

		File.WriteS16((quint8)floor(100.0 * 0.0 + 0.5));

		File.WriteU16(0xA053); // CHK_MAT_REFBLUR
		File.WriteU32(14);

		File.WriteU16(0x0030); // CHK_INT_PERCENTAGE
		File.WriteU32(8);

		File.WriteS16((quint8)floor(100.0 * 0.2 + 0.5));

		File.WriteU16(0xA100); // CHK_MAT_SHADING
		File.WriteU32(8);

		File.WriteS16(3);

		File.WriteU16(0xA084); // CHK_MAT_SELF_ILPCT
		File.WriteU32(14);

		File.WriteU16(0x0030); // CHK_INT_PERCENTAGE
		File.WriteU32(8);

		File.WriteS16((quint8)floor(100.0 * 0.0 + 0.5));

		File.WriteU16(0xA081); // CHK_MAT_TWO_SIDE
		File.WriteU32(6);

		File.WriteU16(0xA087); // CHK_MAT_WIRE_SIZE
		File.WriteU32(10);

		File.WriteFloat(1.0f);

		long MaterialEnd = File.GetPosition();
		File.Seek(MaterialStart + 2, SEEK_SET);
		File.WriteU32(MaterialEnd - MaterialStart);
		File.Seek(MaterialEnd, SEEK_SET);
	}

	const swModelProperties& Properties = mModels[0]->GetProperties();

	File.WriteU16(0x0100); // CHK_MASTER_SCALE
	File.WriteU32(10);

	File.WriteFloat(1.0f);

	File.WriteU16(0x1400); // CHK_LO_SHADOW_BIAS
	File.WriteU32(10);

	File.WriteFloat(1.0f);

	File.WriteU16(0x1420); // CHK_SHADOW_MAP_SIZE
	File.WriteU32(8);

	File.WriteS16(512);

	File.WriteU16(0x1450); // CHK_SHADOW_FILTER
	File.WriteU32(10);

	File.WriteFloat(3.0f);

	File.WriteU16(0x1460); // CHK_RAY_BIAS
	File.WriteU32(10);

	File.WriteFloat(1.0f);

	File.WriteU16(0x1500); // CHK_O_CONSTS
	File.WriteU32(18);

	File.WriteFloat(0.0f);
	File.WriteFloat(0.0f);
	File.WriteFloat(0.0f);

	File.WriteU16(0x2100); // CHK_AMBIENT_LIGHT
	File.WriteU32(42);

	File.WriteU16(0x0010); // CHK_COLOR_F
	File.WriteU32(18);

	File.WriteFloats(Properties.mAmbientColor.GetFloats(), 3);

	File.WriteU16(0x0013); // CHK_LIN_COLOR_F
	File.WriteU32(18);

	File.WriteFloats(Properties.mAmbientColor.GetFloats(), 3);

	File.WriteU16(0x1200); // CHK_SOLID_BGND
	File.WriteU32(42);

	File.WriteU16(0x0010); // CHK_COLOR_F
	File.WriteU32(18);

	const swPreferences& Preferences = swGetPreferences();
	swVector3 BackgroundSolidColor = swVector3FromColor(Preferences.mBackgroundSolidColor);

	File.WriteFloats(BackgroundSolidColor.GetFloats(), 3);

	File.WriteU16(0x0013); // CHK_LIN_COLOR_F
	File.WriteU32(18);

	File.WriteFloats(BackgroundSolidColor.GetFloats(), 3);

	File.WriteU16(0x1201); // LIB3DS_USE_SOLID_BGND
	File.WriteU32(6);

	int NumParts = 0;
	for (const swModelPartsEntry& ModelPart : ModelParts)
	{
		swMesh* Mesh = !ModelPart.Mesh ? ModelPart.PartData->GetMesh() : ModelPart.Mesh;

		if (!Mesh || Mesh->mIndexType == GL_UNSIGNED_INT)
			continue;

		long NamedObjectStart = File.GetPosition();
		File.WriteU16(0x4000); // CHK_NAMED_OBJECT
		File.WriteU32(0);

		char Name[32];
		snprintf(Name, sizeof(Name), "Part%.3d", NumParts);
		NumParts++;
		File.WriteBuffer(Name, strlen(Name) + 1);

		long TriObjectStart = File.GetPosition();
		File.WriteU16(0x4100); // CHK_N_TRI_OBJECT
		File.WriteU32(0);

		File.WriteU16(0x4110); // CHK_POINT_ARRAY
		File.WriteU32(8 + 12 * Mesh->mNumVertices);

		File.WriteU16(Mesh->mNumVertices);

		swVertex* Verts = (swVertex*)Mesh->mVertexData;
		const swMatrix44& ModelWorld = ModelPart.WorldMatrix;

		for (int VertexIdx = 0; VertexIdx < Mesh->mNumVertices; VertexIdx++)
		{
			swVector3 Pos = swMultiply31(Verts[VertexIdx].Position, ModelWorld);
			File.WriteFloat(Pos[0]);
			File.WriteFloat(Pos[1]);
			File.WriteFloat(Pos[2]);
		}

		File.WriteU16(0x4160); // CHK_MESH_MATRIX
		File.WriteU32(54);

		swMatrix44 Matrix = swMatrix44Identity();
		File.WriteFloats(Matrix[0].GetFloats(), 3);
		File.WriteFloats(Matrix[1].GetFloats(), 3);
		File.WriteFloats(Matrix[2].GetFloats(), 3);
		File.WriteFloats(Matrix[3].GetFloats(), 3);

		File.WriteU16(0x4165); // CHK_MESH_COLOR
		File.WriteU32(7);

		File.WriteU8(0);

		long FaceArrayStart = File.GetPosition();
		File.WriteU16(0x4120); // CHK_FACE_ARRAY
		File.WriteU32(0);

		int NumTriangles = 0;

		for (int SectionIdx = 0; SectionIdx < Mesh->mLods[SW_MESH_LOD_HIGH].NumSections; SectionIdx++)
		{
			swMeshSection* Section = &Mesh->mLods[SW_MESH_LOD_HIGH].Sections[SectionIdx];

			if (Section->PrimitiveType != SW_MESH_TRIANGLES && Section->PrimitiveType != SW_MESH_TEXTURED_TRIANGLES)
				continue;

			NumTriangles += Section->DrawCount / 3;
		}

		File.WriteU16(NumTriangles);

		for (int SectionIdx = 0; SectionIdx < Mesh->mLods[SW_MESH_LOD_HIGH].NumSections; SectionIdx++)
		{
			swMeshSection* Section = &Mesh->mLods[SW_MESH_LOD_HIGH].Sections[SectionIdx];

			if (Section->PrimitiveType != SW_MESH_TRIANGLES && Section->PrimitiveType != SW_MESH_TEXTURED_TRIANGLES)
				continue;

			quint16* Indices = (quint16*)Mesh->mIndexData + Section->DrawOffset / sizeof(quint16);

			for (int IndexIdx = 0; IndexIdx < Section->DrawCount; IndexIdx += 3)
			{
				File.WriteU16(Indices[IndexIdx + 0]);
				File.WriteU16(Indices[IndexIdx + 1]);
				File.WriteU16(Indices[IndexIdx + 2]);
				File.WriteU16(7);
			}
		}

		NumTriangles = 0;

		for (int SectionIdx = 0; SectionIdx < Mesh->mLods[SW_MESH_LOD_HIGH].NumSections; SectionIdx++)
		{
			swMeshSection* Section = &Mesh->mLods[SW_MESH_LOD_HIGH].Sections[SectionIdx];

			if (Section->PrimitiveType != SW_MESH_TRIANGLES && Section->PrimitiveType != SW_MESH_TEXTURED_TRIANGLES)
				continue;

			int MaterialIndex = Section->ColorIndex == gDefaultColor ? ModelPart.ColorIndex : Section->ColorIndex;

			File.WriteU16(0x4130); // CHK_MSH_MAT_GROUP
			File.WriteU32(6 + MaterialNameLength + 1 + 2 + 2 * Section->DrawCount / 3);

			snprintf(MaterialName, sizeof(MaterialName), "Material%03d", MaterialIndex);
			File.WriteBuffer(MaterialName, MaterialNameLength + 1);

			File.WriteU16(Section->DrawCount / 3);

			for (int IndexIdx = 0; IndexIdx < Section->DrawCount; IndexIdx += 3)
				File.WriteU16(NumTriangles++);
		}

		long FaceArrayEnd = File.GetPosition();
		File.Seek(FaceArrayStart + 2, SEEK_SET);
		File.WriteU32(FaceArrayEnd - FaceArrayStart);
		File.Seek(FaceArrayEnd, SEEK_SET);

		long TriObjectEnd = File.GetPosition();
		File.Seek(TriObjectStart + 2, SEEK_SET);
		File.WriteU32(TriObjectEnd - TriObjectStart);
		File.Seek(TriObjectEnd, SEEK_SET);

		long NamedObjectEnd = File.GetPosition();
		File.Seek(NamedObjectStart + 2, SEEK_SET);
		File.WriteU32(NamedObjectEnd - NamedObjectStart);
		File.Seek(NamedObjectEnd, SEEK_SET);
	}

	long MDataEnd = File.GetPosition();
	File.Seek(MDataStart + 2, SEEK_SET);
	File.WriteU32(MDataEnd - MDataStart);
	File.Seek(MDataEnd, SEEK_SET);

	long KFDataStart = File.GetPosition();
	File.WriteU16(0xB000); // CHK_KFDATA
	File.WriteU32(0);

	File.WriteU16(0xB00A); // LIB3DS_KFHDR
	File.WriteU32(6 + 2 + 1 + 4);

	File.WriteS16(5);
	File.WriteU8(0);
	File.WriteS32(100);

	long KFDataEnd = File.GetPosition();
	File.Seek(KFDataStart + 2, SEEK_SET);
	File.WriteU32(KFDataEnd - KFDataStart);
	File.Seek(KFDataEnd, SEEK_SET);

	long M3DEnd = File.GetPosition();
	File.Seek(M3DStart + 2, SEEK_SET);
	File.WriteU32(M3DEnd - M3DStart);
	File.Seek(M3DEnd, SEEK_SET);

	return swResult<void>();
}

swResult<swBrickLinkExport> Project::ExportBrickLink(const QString& FileName, const swBrickLinkOptions& Options)
{
	swPartsList PartsList;

	if (!mModels.empty())
		mModels[0]->GetPartsList(gDefaultColor, true, false, PartsList);

	if (PartsList.empty())
		return swUnexpected(tr("Nothing to export."));

	QString SaveFileName = GetExportFileName(FileName, "xml", tr("Export BrickLink"), tr("XML Files (*.xml);;All Files (*)"));

	if (SaveFileName.isEmpty())
		return swUnexpected(QString());

	const swResult<int> Lots = swExportBrickLink(SaveFileName, PartsList, Options);

	if (!Lots)
		return swUnexpected(QString(Lots.error()));

	return swBrickLinkExport{ SaveFileName, Lots.value() };
}

swResult<void> Project::ExportCSV(const QString& FileName)
{
	swPartsList PartsList;

	if (!mModels.empty())
		mModels[0]->GetPartsList(gDefaultColor, true, false, PartsList);

	if (PartsList.empty())
		return swUnexpected(tr("Nothing to export."));

	QString SaveFileName = GetExportFileName(FileName, "csv", tr("Export CSV"), tr("CSV Files (*.csv);;All Files (*)"));

	if (SaveFileName.isEmpty())
		return swUnexpected(QString());

	return swExportCSV(SaveFileName, PartsList);
}

swResult<void> swExportCSV(const QString& FileName, const swPartsList& PartsList)
{
	swDiskFile CSVFile(FileName);
	char Line[1024];

	if (!CSVFile.Open(QIODevice::WriteOnly))
		return swUnexpected(QCoreApplication::translate("Project", "Could not open file '%1' for writing.").arg(FileName));

	CSVFile.WriteLine("Part Name,Color,Quantity,Part ID,Color Code\n");

	swPartsLibrary* Library = swGetPartsLibrary();

	// moved part aliases list as the parts they moved to
	for (const auto& PartIt : Library ? Library->ResolveMovedParts(PartsList) : PartsList)
	{
		const swPartData* PartData = PartIt.first;

		for (const auto& ColorIt : PartIt.second)
		{
			std::string Description = PartData->m_strDescription;
			Description.erase(std::remove(Description.begin(), Description.end(), ','), Description.end());

			snprintf(Line, sizeof(Line), "\"%s\",\"%s\",%d,%s,%d\n", Description.c_str(), gColorList[ColorIt.first].Name, ColorIt.second, PartData->mFileName, gColorList[ColorIt.first].Code);
			CSVFile.WriteLine(Line);
		}
	}

	return swResult<void>();
}

swResult<void> Project::ExportWavefront(const QString& FileName)
{
	const std::vector<swModelPartsEntry> ModelParts = GetModelParts();

	QString SaveFileName = GetExportFileName(FileName, QLatin1String("obj"), tr("Export Wavefront"), tr("Wavefront Files (*.obj);;All Files (*)"));

	if (SaveFileName.isEmpty())
		return swUnexpected(QString());

	swDiskFile OBJFile(SaveFileName);
	char Line[1024];

	if (!OBJFile.Open(QIODevice::WriteOnly))
		return swUnexpected(tr("Could not open file '%1' for writing.").arg(SaveFileName));

	OBJFile.WriteLine("# Model exported from StudWorx\n");

	QFileInfo SaveInfo(SaveFileName);
	QString MaterialFileName = QDir(SaveInfo.absolutePath()).absoluteFilePath(SaveInfo.completeBaseName() + QLatin1String(".mtl"));

	snprintf(Line, sizeof(Line), "#\n\nmtllib %s\n\n", QFileInfo(MaterialFileName).fileName().toLatin1().constData());
	OBJFile.WriteLine(Line);

	swDiskFile MaterialFile(MaterialFileName);

	if (!MaterialFile.Open(QIODevice::WriteOnly))
		return swUnexpected(tr("Could not open file '%1' for writing.").arg(MaterialFileName));

	MaterialFile.WriteLine("# Colors used by StudWorx\n\n");

	for (const swColor& Color : gColorList)
	{
		if (Color.Translucent)
			snprintf(Line, sizeof(Line), "newmtl %s\nKd %.2f %.2f %.2f\nD %.2f\n\n", Color.SafeName, Color.Value[0], Color.Value[1], Color.Value[2], Color.Value[3]);
		else
			snprintf(Line, sizeof(Line), "newmtl %s\nKd %.2f %.2f %.2f\n\n", Color.SafeName, Color.Value[0], Color.Value[1], Color.Value[2]);

		MaterialFile.WriteLine(Line);
	}

	auto WritePositions=[&Line, &OBJFile](const auto* Verts, int VertexCount, const swMatrix44& ModelWorld)
	{
		for (int VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
		{
			swVector3 Position = swMultiply31(Verts[VertexIndex].Position, ModelWorld);
			snprintf(Line, sizeof(Line), "v %.2f %.2f %.2f\n", Position[0], Position[1], Position[2]);
			OBJFile.WriteLine(Line);
		}
	};

	auto WriteNormals=[&Line, &OBJFile](const auto* Verts, int VertexCount, const swMatrix44& ModelWorld)
	{
		for (int VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
		{
			swVector3 Normal = swMultiply30(swUnpackNormal(Verts[VertexIndex].Normal), ModelWorld);
			snprintf(Line, sizeof(Line), "vn %.2f %.2f %.2f\n", Normal[0], Normal[1], Normal[2]);
			OBJFile.WriteLine(Line);
		}
	};

	for (const swModelPartsEntry& ModelPart : ModelParts)
	{
		swMesh* Mesh = !ModelPart.Mesh ? ModelPart.PartData->GetMesh() : ModelPart.Mesh;

		if (!Mesh)
			continue;

		const swVertex* Verts = static_cast<const swVertex*>(Mesh->mVertexData);

		WritePositions(Verts, Mesh->mNumVertices, ModelPart.WorldMatrix);

		const swVertexTextured* VertsTextured = reinterpret_cast<const swVertexTextured*>(Verts + Mesh->mNumVertices);

		WritePositions(VertsTextured, Mesh->mNumTexturedVertices, ModelPart.WorldMatrix);

		OBJFile.WriteLine("#\n\n");
	}

	for (const swModelPartsEntry& ModelPart : ModelParts)
	{
		swMesh* Mesh = !ModelPart.Mesh ? ModelPart.PartData->GetMesh() : ModelPart.Mesh;

		if (!Mesh)
			continue;

		const swVertex* Verts = static_cast<const swVertex*>(Mesh->mVertexData);

		WriteNormals(Verts, Mesh->mNumVertices, ModelPart.WorldMatrix);

		const swVertexTextured* VertsTextured = reinterpret_cast<const swVertexTextured*>(Verts + Mesh->mNumVertices);

		WriteNormals(VertsTextured, Mesh->mNumTexturedVertices, ModelPart.WorldMatrix);

		OBJFile.WriteLine("#\n\n");
	}

	int PartCount = 0;
	quint32 BaseVertex = 1;

	for (const swModelPartsEntry& ModelPart : ModelParts)
	{
		swMesh* Mesh = !ModelPart.Mesh ? ModelPart.PartData->GetMesh() : ModelPart.Mesh;

		if (!Mesh)
			continue;

		snprintf(Line, sizeof(Line), "g Part%.3d\n", PartCount++);
		OBJFile.WriteLine(Line);

		Mesh->ExportWavefrontIndices(OBJFile, ModelPart.ColorIndex, BaseVertex);
		BaseVertex += Mesh->mNumVertices + Mesh->mNumTexturedVertices;
	}

	return swResult<void>();
}

swResult<void> Project::SaveImage(const swImageDialogOptions& Options)
{
	QString FilePath = Options.FilePath;
	QString Extension = QFileInfo(FilePath).suffix();

	if (!Extension.isEmpty())
		swSetProfileString(SW_PROFILE_IMAGE_EXTENSION, FilePath.right(Extension.length() + 1));

	// step animations hold every step in one file
	const bool StepFiles = Options.Start != Options.End && !Options.Turntable.IsStepping();

	if (StepFiles)
		FilePath = FilePath.insert(FilePath.length() - Extension.length() - 1, QLatin1String("%1"));

	return mActiveModel->SaveFramedImages(FilePath, StepFiles, Options.Width, Options.Height, Options.Start, Options.End, Options.Background, Options.Framing, Options.Turntable);
}
