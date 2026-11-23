#include "sw_global.h"
#include "sw_project.h"
#include "sw_window_main.h"
#include "sw_dialog_instructions_page.h"
#include "sw_colors.h"
#include "sw_window_utilities.h"

// inline so the export folder holds only pages and images, images stay on white since parts lists use black text
static const char swHTMLStyleSheet[] = R"(
			/* colors and corner radius shared by every page */

			:root {
				color-scheme: light dark;
				--page: #f6f7f9;
				--text: #1f2328;
				--muted: #656d76;
				--card: #ffffff;
				--line: #d0d7de;
				--link: #0969da;
				--radius: 12px;
			}

			/* colors when the reader system is in dark mode */

			@media (prefers-color-scheme: dark) {

				:root {
					--page: #16181c;
					--text: #e6edf3;
					--muted: #8d96a0;
					--line: #30363d;
					--link: #4493f8;
				}

			}

			/* page width, spacing and text font */

			body {
				max-width: 1100px;
				margin: 0 auto;
				padding: 32px 16px;
				background: var(--page);
				color: var(--text);
				font: 16px/1.7 system-ui, -apple-system, "Segoe UI", sans-serif;
				text-align: center;
			}

			/* page title */

			h1 {
				margin: 0 0 32px;
				font-size: 1.75em;
				font-weight: 600;
			}

			/* step and parts list images on their card */

			img {
				max-width: 100%;
				height: auto;
				padding: 16px;
				box-sizing: border-box;
				background: var(--card);
				border: 1px solid var(--line);
				border-radius: var(--radius);
			}

			/* spacing under paragraphs */

			p {
				margin: 0 0 16px;
			}

			/* links */

			a {
				color: var(--link);
				text-decoration: none;
			}

			/* links under the pointer */

			a:hover {
				text-decoration: underline;
			}

			/* line above the created by footer */

			hr {
				border: 0;
				border-top: 1px solid var(--line);
				margin: 32px 0;
			}

			/* previous, index, next and parts used links of one step per page */

			body > a, body > b {
				display: inline-block;
				margin: 0 8px;
			}

			/* created by text in the footer */

			i {
				color: var(--muted);
				font-weight: 400;
			}

			/* single page, pages snap into view while scrolling */

			html:has(section) {
				scroll-snap-type: y mandatory;
			}

			/* single page, the title and the footer snap too */

			html:has(section) h1, footer {
				scroll-snap-align: start;
			}

			/* single page, one instruction page per screen */

			section {
				min-height: 100vh;
				box-sizing: border-box;
				display: flex;
				flex-direction: column;
				align-items: center;
				justify-content: center;
				padding: 16px 0;
				scroll-snap-align: center;
				scroll-snap-stop: always;
			}

			/* single page, page image fits the screen height */

			section img {
				padding: 0;
				width: auto;
				max-height: calc(100vh - 32px);
			}

			/* page navigation, on the right by default */

			nav {
				position: fixed;
				top: 50%;
				right: 16px;
				transform: translateY(-50%);
				display: flex;
				flex-direction: column;
				gap: 4px;
				padding: 4px;
				background: var(--page);
				border: 1px solid var(--line);
				border-radius: var(--radius);
				opacity: 0.85;
				transition: opacity 0.2s;
			}

			/* page navigation on the left */

			nav.left {
				right: auto;
				left: 16px;
			}

			/* page navigation at the bottom */

			nav.bottom {
				top: auto;
				right: 50%;
				bottom: 16px;
				transform: translateX(50%);
				flex-direction: row;
			}

			/* page navigation while used */

			nav:hover, nav:focus-within {
				opacity: 1;
			}

			/* each page navigation button */

			nav button {
				width: 40px;
				height: 40px;
				border: 0;
				border-radius: 8px;
				background: none;
				color: var(--text);
				cursor: pointer;
			}

			/* page navigation icons */

			nav svg {
				width: 24px;
				height: 24px;
				fill: none;
				stroke: currentColor;
				stroke-width: 2;
				stroke-linecap: round;
				stroke-linejoin: round;
				vertical-align: middle;
			}

			/* page navigation button under the pointer */

			nav button:hover {
				background: var(--line);
			}

			/* printing leaves the page navigation out */

			@media print {

				nav {
					display: none;
				}

			}

)";


// moves to the page next to the one on screen
static const char swHTMLScript[] = R"(
	const sections_page = [...document.querySelectorAll("section")];

	document.querySelectorAll("nav button").forEach((button_navigation) => {

		button_navigation.addEventListener("click", () => {

			const index_current = Math.max(0, sections_page.findIndex((section_page) => section_page.getBoundingClientRect().bottom > innerHeight / 2));
			const target_navigation = button_navigation.dataset.target;
			let index_target = index_current + Number(target_navigation);

			if (target_navigation === "first") {
				index_target = 0;
			}

			if (target_navigation === "last") {
				index_target = sections_page.length - 1;
			}

			sections_page[Math.min(Math.max(index_target, 0), sections_page.length - 1)].scrollIntoView({ behavior: "smooth", block: "center" });

		});

	});

)";

// use to list the instruction colors of an override color row
static std::vector<swInstructionsPropertyType> swGetHTMLOverrideColorTypes(int Row)
{
	// rows in the order of the export dialog, then box border and box background
	static const swInstructionsPropertyType RowTypes[] = { swInstructionsPropertyType::PageBackgroundColor, swInstructionsPropertyType::CoverBackgroundColor, swInstructionsPropertyType::BillOfMaterialsBackgroundColor, swInstructionsPropertyType::CoverTitleColor, swInstructionsPropertyType::PLIThumbnailBackgroundColor, swInstructionsPropertyType::NoticeTextColor, swInstructionsPropertyType::FadeColor, swInstructionsPropertyType::HighlightColor, swInstructionsPropertyType::StepBackgroundColor, swInstructionsPropertyType::StepNumberColor, swInstructionsPropertyType::PageNumberColor, swInstructionsPropertyType::PLITextColor, swInstructionsPropertyType::StepDividerColor, swInstructionsPropertyType::PointerColor };

	// some rows also color the submodel pages, the author and the parts list title
	if (Row < static_cast<int>(std::size(RowTypes)) && RowTypes[Row] == swInstructionsPropertyType::PageBackgroundColor)
		return { RowTypes[Row], swInstructionsPropertyType::SubmodelBackgroundColor, swInstructionsPropertyType::BlankPageBackgroundColor };

	if (Row < static_cast<int>(std::size(RowTypes)) && RowTypes[Row] == swInstructionsPropertyType::CoverTitleColor)
		return { RowTypes[Row], swInstructionsPropertyType::CoverAuthorColor, swInstructionsPropertyType::PartCountColor };

	if (Row < static_cast<int>(std::size(RowTypes)) && RowTypes[Row] == swInstructionsPropertyType::PageNumberColor)
		return { RowTypes[Row], swInstructionsPropertyType::PagePartCountColor };

	if (Row < static_cast<int>(std::size(RowTypes)) && RowTypes[Row] == swInstructionsPropertyType::StepNumberColor)
		return { RowTypes[Row], swInstructionsPropertyType::BillOfMaterialsTitleColor };

	if (Row < static_cast<int>(std::size(RowTypes)))
		return { RowTypes[Row] };

	std::vector<swInstructionsPropertyType> BoxColorTypes;

	// page boxes have their own rows
	for (int Element = 0; Element < static_cast<int>(swInstructionsBoxElement::FinalModel); Element++)
	{
		const swInstructionsBoxTypes BoxTypes = swGetBoxTypes(static_cast<swInstructionsBoxElement>(Element));
		BoxColorTypes.push_back(Row == static_cast<int>(std::size(RowTypes)) ? BoxTypes.BorderColor : BoxTypes.BackgroundColor);
	}

	return BoxColorTypes;
}

QString swGetHTMLStyleSheet()
{
	// the sheet without the indent it takes inside the page
	return QString::fromLatin1(swHTMLStyleSheet).replace(QLatin1String("\n\t\t\t"), QLatin1String("\n")).trimmed() + QLatin1Char('\n');
}

swHTMLExportOptions::swHTMLExportOptions(const Project* Project)
{
	QString FileName = Project->GetFileName();

	if (!FileName.isEmpty())
		PathName = QFileInfo(FileName).canonicalPath();

	int ImageOptions = swGetProfileInteger(SW_PROFILE_HTML_IMAGE_OPTIONS);
	int HTMLOptions = swGetProfileInteger(SW_PROFILE_HTML_OPTIONS);

	SubModels = (HTMLOptions & (SW_HTML_SUBMODELS)) != 0;
	CurrentOnly = (HTMLOptions & SW_HTML_CURRENT_ONLY) != 0;
	SinglePage = (HTMLOptions & SW_HTML_SINGLE_PAGE) != 0;
	IndexPage = (HTMLOptions & SW_HTML_INDEX) != 0;
	StepImagesWidth = swGetProfileInteger(SW_PROFILE_HTML_IMAGE_WIDTH);
	StepImagesHeight = swGetProfileInteger(SW_PROFILE_HTML_IMAGE_HEIGHT);
	PartsListStep = (HTMLOptions & SW_HTML_LIST_STEP) != 0;
	PartsListEnd = (HTMLOptions & SW_HTML_LIST_END) != 0;
	ArrowPosition = swGetProfileInteger(SW_PROFILE_HTML_ARROW_POSITION);
	Display = swGetProfileInteger(SW_PROFILE_HTML_DISPLAY);
	OverrideInstructions = (HTMLOptions & SW_HTML_OVERRIDE) != 0;
	BackgroundColor = swQColorFromRGBA(swGetProfileInteger(SW_PROFILE_HTML_BACKGROUND_COLOR));

	// the old transparent background check box becomes a transparent color
	if (ImageOptions & SW_IMAGE_TRANSPARENT)
		BackgroundColor.setAlpha(0);
	StyleSheet = swGetProfileString(SW_PROFILE_HTML_STYLE_SHEET);

	const QStringList ColorNames = swGetProfileStringList(SW_PROFILE_HTML_OVERRIDE_COLORS);
	const swInstructionsProperties DefaultProperties = swInstructions::CreateDefaultProperties();

	// unsaved rows start from the instructions defaults
	for (int Row = 0; Row < SW_HTML_OVERRIDE_COLOR_COUNT; Row++)
	{
		const QColor Color(ColorNames.value(Row));
		OverrideColors[Row] = Color.isValid() ? Color : swQColorFromRGBA(DefaultProperties[static_cast<int>(swGetHTMLOverrideColorTypes(Row).front())].Value.toUInt());
	}
}

void swHTMLExportOptions::SaveDefaults()
{
	int HTMLOptions = 0;

	if (SubModels)
		HTMLOptions |= SW_HTML_SUBMODELS;
	if (CurrentOnly)
		HTMLOptions |= SW_HTML_CURRENT_ONLY;
	if (SinglePage)
		HTMLOptions |= SW_HTML_SINGLE_PAGE;
	if (IndexPage)
		HTMLOptions |= SW_HTML_INDEX;
	if (PartsListStep)
		HTMLOptions |= SW_HTML_LIST_STEP;
	if (PartsListEnd)
		HTMLOptions |= SW_HTML_LIST_END;
	if (OverrideInstructions)
		HTMLOptions |= SW_HTML_OVERRIDE;

	swSetProfileInteger(SW_PROFILE_HTML_IMAGE_OPTIONS, 0);
	swSetProfileInteger(SW_PROFILE_HTML_OPTIONS, HTMLOptions);
	swSetProfileInteger(SW_PROFILE_HTML_IMAGE_WIDTH, StepImagesWidth);
	swSetProfileInteger(SW_PROFILE_HTML_IMAGE_HEIGHT, StepImagesHeight);
	swSetProfileInteger(SW_PROFILE_HTML_ARROW_POSITION, ArrowPosition);
	swSetProfileInteger(SW_PROFILE_HTML_DISPLAY, Display);
	swSetProfileInteger(SW_PROFILE_HTML_BACKGROUND_COLOR, swRGBAFromQColor(BackgroundColor));
	swSetProfileString(SW_PROFILE_HTML_STYLE_SHEET, StyleSheet);

	QStringList ColorNames;

	for (const QColor& Color : OverrideColors)
		ColorNames.append(Color.name(QColor::HexArgb));

	swSetProfileStringList(SW_PROFILE_HTML_OVERRIDE_COLORS, ColorNames);
}

swResult<void> Project::ExportHTML(const swHTMLExportOptions& Options)
{
	std::set<swModel*> Models;

	if (Options.CurrentOnly)
		Models.insert(mActiveModel);
	else if (Options.SubModels)
	{
		Models.insert(mActiveModel);
		mActiveModel->GetSubModels(Models);
	}
	else
	{
		for (const std::unique_ptr<swModel>& Model : mModels)
			Models.insert(Model.get());
	}

	for (const swModel* Model : Models)
		Model->WaitForAssets();

	const QString ProjectTitle = GetTitle();
	auto MakeSafeFileName = [](const QString& Name)
	{
		const QByteArray Utf8Name = Name.toUtf8();
		QString SafeName;
		SafeName.reserve(Utf8Name.size() * 3);

		for (char Byte : Utf8Name)
		{
			const unsigned char Value = static_cast<unsigned char>(Byte);

			if ((Value >= 'A' && Value <= 'Z') || (Value >= 'a' && Value <= 'z') || (Value >= '0' && Value <= '9') || Value == '-')
				SafeName.append(QLatin1Char(Value));
			else
				SafeName.append(QString("_%1").arg(Value, 2, 16, QLatin1Char('0')));
		}

		if (SafeName.isEmpty())
			return QString("Model");

		if (SafeName.size() > 80)
		{
			const QByteArray Digest = QCryptographicHash::hash(Utf8Name, QCryptographicHash::Sha256).toHex();
			SafeName = SafeName.left(60) + '-' + QString::fromLatin1(Digest.left(16));
		}

		// windows reserves device names
		const QString UpperName = SafeName.toUpper();
		const bool NumberedDevice = UpperName.size() == 4 && (UpperName.startsWith(QLatin1String("COM")) || UpperName.startsWith(QLatin1String("LPT"))) &&
			UpperName[3] >= QLatin1Char('1') && UpperName[3] <= QLatin1Char('9');

		if (UpperName == QLatin1String("CON") || UpperName == QLatin1String("PRN") || UpperName == QLatin1String("AUX") || UpperName == QLatin1String("NUL") || NumberedDevice)
			SafeName.prepend('_');

		return SafeName;
	};

	const QString ProjectBaseName = MakeSafeFileName(QFileInfo(ProjectTitle).completeBaseName());

	// keep every export in its own folder named after the model
	QDir Dir(QDir(Options.PathName).filePath(ProjectBaseName));

	if (!Dir.mkpath(QLatin1String(".")))
		return swUnexpected(tr("Could not create folder '%1'.").arg(QDir::toNativeSeparators(Dir.absolutePath())));

	// the entry page of the export
	const QString IndexFileName = QLatin1String("index.html");
	const QString ModelIndexSuffix = QLatin1String("-index.html");
	auto GetModelIndexFileName = [&](const QString& BaseName)
	{
		return Models.size() > 1 ? BaseName + ModelIndexSuffix : IndexFileName;
	};
	std::map<const swModel*, QString> ModelBaseNames;
	int ModelOrdinal = 0;

	for (const std::unique_ptr<swModel>& Model : mModels)
	{
		if (Models.find(Model.get()) == Models.end())
			continue;

		QString BaseName = ProjectBaseName;

		if (Models.size() > 1)
			BaseName += QString("-%1-%2").arg(++ModelOrdinal).arg(MakeSafeFileName(Model->GetProperties().mFileName));

		ModelBaseNames.emplace(Model.get(), std::move(BaseName));
	}

	auto AddPartsListImage = [&Dir](QTextStream& Stream, swModel* Model, swStep Step, const QString& BaseName) -> swResult<void>
	{
		swPartsList PartsList;

		if (Step == 0)
			Model->GetPartsList(gDefaultColor, true, false, PartsList);
		else
			Model->GetPartsListForStep(Step, gDefaultColor, PartsList, false);

		if (PartsList.empty())
			return swResult<void>();

		const swResult<QImage> Image = Model->GetPartsListImage(1024, 512, Step, SW_RGBA(255, 255, 255, 0), SW_RGBA(255, 255, 255, 0), QFont("Arial", 16), Qt::black, true);

		if (!Image)
			return swUnexpected(Image.error());

		QString ImageName = BaseName + QLatin1String("-parts.png");
		QString FileName = QFileInfo(Dir, ImageName).absoluteFilePath();
		QImageWriter Writer(FileName);

		if (!Writer.write(Image.value()))
			return swUnexpected(tr("Could not write file '%1':\n%2").arg(FileName, Writer.errorString()));

		Stream << QString::fromLatin1("<p><IMG SRC=\"%1\" /></p><br><br>\n").arg(ImageName.toHtmlEscaped());

		return swResult<void>();
	};

	// the image card takes the image color so its rounded corners frame the image
	QString CardStyle = !Options.SinglePage && Options.BackgroundColor.alpha() ? QString::fromLatin1("\t\t\t:root {\n\t\t\t\t--card: %1;\n\t\t\t}\n\n").arg(Options.BackgroundColor.name()) : QString();

	// an empty style sheet keeps the built-in one
	QString StyleSheet;

	// lines take the indent of the style element
	for (const QString& Line : (Options.StyleSheet.isEmpty() ? swGetHTMLStyleSheet() : Options.StyleSheet).split(QLatin1Char('\n')))
		StyleSheet += Line.trimmed().isEmpty() ? QLatin1String("\n") : QLatin1String("\t\t\t") + Line + QLatin1Char('\n');

	auto WriteHeader = [&CardStyle, &StyleSheet](QTextStream& Stream, const QString& Title)
	{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
		Stream.setCodec("UTF-8");
#endif
		Stream << QString::fromLatin1("<!DOCTYPE html>\n<HTML>\n\t<HEAD>\n\t\t<META CHARSET=\"UTF-8\">\n\t\t<META NAME=\"viewport\" CONTENT=\"width=device-width, initial-scale=1\">\n\t\t<TITLE>%1</TITLE>\n\t\t<STYLE>\n%2%3\t\t</STYLE>\n\t</HEAD>\n<BODY>\n<CENTER>\n<H1>%1</H1>\n").arg(Title.toHtmlEscaped(), StyleSheet, CardStyle);
	};

	const QString CreatedBy = tr("Created by %1").toHtmlEscaped().arg(QLatin1String("<A HREF=\"https://github.com/nathaneltitane/studworx\">StudWorx</A>"));

	// one scrolling page of the instructions window pages
	if (Options.SinglePage)
	{
		swInstructions Instructions(this);

		// export overrides change this copy and never the model
		QObject::disconnect(&Instructions, nullptr, &Instructions, nullptr);

		if (Options.OverrideInstructions)
		{
			std::vector<swInstructionsPropertyType> OverrideTypes = { swInstructionsPropertyType::ShowStepPLI, swInstructionsPropertyType::ShowBillOfMaterials, swInstructionsPropertyType::StepImageBackgroundColor };

			for (int Row = 0; Row < SW_HTML_OVERRIDE_COLOR_COUNT; Row++)
				for (swInstructionsPropertyType Type : swGetHTMLOverrideColorTypes(Row))
					OverrideTypes.push_back(Type);

			// step and page values would win over the defaults
			for (swInstructionsPropertyType Type : OverrideTypes)
			{
				for (auto& [Model, InstructionsModel] : Instructions.mModels)
					for (swInstructionsProperties& Properties : InstructionsModel.StepProperties)
						Properties[static_cast<int>(Type)].Mode = swInstructionsPropertyMode::NotSet;

				for (auto& [Key, Properties] : Instructions.mPagePropertyOverrides)
					Properties[static_cast<int>(Type)].Mode = swInstructionsPropertyMode::NotSet;
			}

			for (int Row = 0; Row < SW_HTML_OVERRIDE_COLOR_COUNT; Row++)
				for (swInstructionsPropertyType Type : swGetHTMLOverrideColorTypes(Row))
					Instructions.SetDefaultColor(Type, Options.OverrideColors[Row]);

			// the step images box takes the image background after the box background
			Instructions.SetDefaultColor(swInstructionsPropertyType::StepImageBackgroundColor, Options.BackgroundColor);
			Instructions.SetDefaultBool(swInstructionsPropertyType::ShowStepPLI, Options.PartsListStep);
			Instructions.SetDefaultBool(swInstructionsPropertyType::ShowBillOfMaterials, Options.PartsListEnd);
		}

		// display shapes lay the copy out again at the width and height ratio, keeping the page area so text keeps its size
		if (Options.Display)
		{
			swInstructionsPageSetup& PageSetup = Instructions.mPageSetup;
			const float PageArea = PageSetup.Width * PageSetup.Height;
			const float DisplayRatio = static_cast<float>(Options.StepImagesWidth) / Options.StepImagesHeight;

			PageSetup.Width = sqrtf(PageArea * DisplayRatio);
			PageSetup.Height = sqrtf(PageArea / DisplayRatio);
			Instructions.Update();
		}

		const QSize ImageSize = QSizeF(Instructions.mPageSetup.Width, Instructions.mPageSetup.Height).scaled(QSizeF(Options.StepImagesWidth, Options.StepImagesHeight), Qt::KeepAspectRatio).toSize();
		// page cards round like the step model box, in percent of the page so they match at any shown size
		const qreal StepImageRadius = Instructions.GetBoxStyle(swInstructionsBoxElement::StepImage, nullptr, 0).GetBackgroundRadius();
		CardStyle = QString::fromLatin1("\t\t\tsection img {\n\t\t\t\tborder-radius: %1% / %2%;\n\t\t\t}\n\n").arg(100.0 * StepImageRadius / Instructions.mPageSetup.Width, 0, 'f', 3).arg(100.0 * StepImageRadius / Instructions.mPageSetup.Height, 0, 'f', 3);

		QString FileName = QFileInfo(Dir, IndexFileName).absoluteFilePath();
		QFile File(FileName);

		if (!File.open(QIODevice::WriteOnly))
			return swUnexpected(tr("Could not write file '%1':\n%2").arg(FileName, File.errorString()));

		QTextStream Stream(&File);

		WriteHeader(Stream, tr("Instructions for %1").arg(ProjectTitle));

		for (size_t PageIndex = 0; PageIndex < Instructions.mPages.size(); PageIndex++)
		{
			const swInstructionsPage& Page = Instructions.mPages[PageIndex];
			QString StepAnchors;

			if (Page.Type == swInstructionsPageType::Blank)
				continue;

			if (Page.Type == swInstructionsPageType::Steps)
			{
				for (const swInstructionsStep& Step : Page.Steps)
				{
					if (Step.FinalModel || Models.find(Step.Model) == Models.end())
						continue;

					const QString StepPrefix = Models.size() > 1 ? ModelBaseNames.at(Step.Model) + '-' : QString();
					StepAnchors += QString::fromLatin1("<A ID=\"%1step-%2\"></A>").arg(StepPrefix.toHtmlEscaped(), QString::number(Step.Step));
				}

				if (StepAnchors.isEmpty())
					continue;
			}
			else if (Page.Model && Models.find(Page.Model) == Models.end())
				continue;

			QGraphicsScene Scene;
			// twice the shown size keeps pages sharp on high density screens
			QImage Image(ImageSize * qMin(2.0, static_cast<double>(SW_MAXIMUM_IMAGE_DIMENSION) / qMax(ImageSize.width(), ImageSize.height())), QImage::Format_ARGB32_Premultiplied);
			Image.fill(Instructions.GetPageBackgroundColor(PageIndex));

			swUpdateInstructionsPageScene(&Instructions, &Page, &Scene);

			QPainter Painter(&Image);
			Painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
			Scene.render(&Painter, QRectF(QPointF(), QSizeF(Image.size())), Scene.sceneRect(), Qt::KeepAspectRatio);
			Painter.end();

			const QString PageNumber = QString::number(PageIndex + 1);
			const QString ImageName = QString::fromLatin1("%1-page-%2.png").arg(ProjectBaseName, QString("%1").arg(PageIndex + 1, 2, 10, QLatin1Char('0')));
			const QString ImageFileName = QFileInfo(Dir, ImageName).absoluteFilePath();
			QImageWriter Writer(ImageFileName);

			if (!Writer.write(Image))
				return swUnexpected(tr("Could not write file '%1':\n%2").arg(ImageFileName, Writer.errorString()));

			Stream << QString::fromLatin1("<SECTION ID=\"page-%1\">%2<IMG SRC=\"%3\" ALT=\"%4\" WIDTH=%5 HEIGHT=%6></SECTION>\n").arg(PageNumber, StepAnchors, ImageName.toHtmlEscaped(), tr("Page %1").arg(PageNumber).toHtmlEscaped(), QString::number(ImageSize.width()), QString::number(ImageSize.height()));
		}

		Stream << QString::fromLatin1("</CENTER>\n<FOOTER><HR><B><I>%1</I></B></FOOTER>\n").arg(CreatedBy);
		// arrow position index of the export dialog
		const char* ArrowPositions[] = { "right", "left", "bottom" };
		Stream << QString::fromLatin1("<NAV CLASS=\"%1\">\n").arg(QLatin1String(ArrowPositions[qBound(0, Options.ArrowPosition, int(std::size(ArrowPositions)) - 1)]));

		const std::tuple<const char*, QString, const char*> Arrows[] = { { "first", tr("First Page"), "M6 5h12M12 19V9M7 14l5-5 5 5" }, { "-1", tr("Previous Page"), "M12 19V5M7 10l5-5 5 5" }, { "1", tr("Next Page"), "M12 5v14M7 14l5 5 5-5" }, { "last", tr("Last Page"), "M6 19h12M12 5v10M7 10l5 5 5-5" } };

		for (const auto& [Target, Label, Symbol] : Arrows)
			Stream << QString::fromLatin1("\t<BUTTON TYPE=\"button\" DATA-TARGET=\"%1\" TITLE=\"%2\" ARIA-LABEL=\"%2\"><SVG VIEWBOX=\"0 0 24 24\"><PATH D=\"%3\"/></SVG></BUTTON>\n").arg(QLatin1String(Target), Label.toHtmlEscaped(), QLatin1String(Symbol));

		Stream << QString::fromLatin1("</NAV>\n<SCRIPT>\n%1</SCRIPT>\n</BODY>\n</HTML>\n").arg(QLatin1String(swHTMLScript));

		return swResult<void>();
	}

	for (swModel* Model : Models)
	{
		const QString& BaseName = ModelBaseNames.at(Model);
		swStep LastStep = Model->GetLastStep();
		QString PageTitle;

		if (Models.size() > 1)
			PageTitle = Model->GetProperties().mFileName;
		else
			PageTitle = ProjectTitle;

		if (Options.IndexPage)
		{
			QString FileName = QFileInfo(Dir, GetModelIndexFileName(BaseName)).absoluteFilePath();
			QFile File(FileName);

			if (!File.open(QIODevice::WriteOnly))
				return swUnexpected(tr("Could not write file '%1':\n%2").arg(FileName, File.errorString()));

			QTextStream Stream(&File);

			WriteHeader(Stream, tr("Instructions for %1").arg(PageTitle));

			for (swStep Step = 1; Step <= LastStep; Step++)
				Stream << QString::fromLatin1("<A HREF=\"%1-%2.html\">%3<BR>\n</A>").arg(BaseName, QString("%1").arg(Step, 2, 10, QLatin1Char('0')), tr("Step %1").arg(QString::number(Step)).toHtmlEscaped());

			if (Options.PartsListEnd)
				Stream << QString::fromLatin1("<A HREF=\"%1-parts.html\">%2</A><BR>\n").arg(BaseName, tr("Parts Used").toHtmlEscaped());

			Stream << QString::fromLatin1("</CENTER>\n<BR><HR><BR><B><I>%1</I></B><BR></HTML>\n").arg(CreatedBy);
		}

		for (swStep Step = 1; Step <= LastStep; Step++)
		{
			QString StepString = QString::fromLatin1("%1").arg(Step, 2, 10, QLatin1Char('0'));
			QString FileName = QFileInfo(Dir, QString("%1-%2.html").arg(BaseName, StepString)).absoluteFilePath();
			QFile File(FileName);

			if (!File.open(QIODevice::WriteOnly))
				return swUnexpected(tr("Could not write file '%1':\n%2").arg(FileName, File.errorString()));

			QTextStream Stream(&File);

			WriteHeader(Stream, tr("%1 - Step %2").arg(PageTitle, QString::number(Step)));
			Stream << QString::fromLatin1("<IMG SRC=\"%1-%2.png\" ALT=\"%3\" WIDTH=%4 HEIGHT=%5><BR><BR>\n").arg(BaseName.toHtmlEscaped(), StepString, tr("Step %1").arg(StepString).toHtmlEscaped(), QString::number(Options.StepImagesWidth), QString::number(Options.StepImagesHeight));

			if (Options.PartsListStep)
			{
				const swResult<void> PartsImageSaved = AddPartsListImage(Stream, Model, Step, QString("%1-%2").arg(BaseName, StepString));

				if (!PartsImageSaved)
					return PartsImageSaved;
			}

			Stream << QLatin1String("</CENTER>\n<BR><HR><BR>");
			if (Step != 1)
				Stream << QString::fromLatin1("<A HREF=\"%1-%2.html\">%3</A> ").arg(BaseName, QString("%1").arg(Step - 1, 2, 10, QLatin1Char('0')), tr("Previous").toHtmlEscaped());

			if (Options.IndexPage)
				Stream << QString::fromLatin1("<A HREF=\"%1\">%2</A> ").arg(GetModelIndexFileName(BaseName).toHtmlEscaped(), tr("Index").toHtmlEscaped());

			if (Step != LastStep)
				Stream << QString::fromLatin1("<A HREF=\"%1-%2.html\">%3</A>").arg(BaseName, QString("%1").arg(Step + 1, 2, 10, QLatin1Char('0')), tr("Next").toHtmlEscaped());
			else if (Options.PartsListEnd)
				Stream << QString::fromLatin1("<A HREF=\"%1-parts.html\">%2</A>").arg(BaseName, tr("Parts Used").toHtmlEscaped());

			Stream << QLatin1String("<BR></HTML>\n");
		}

		if (Options.PartsListEnd)
		{
			QString FileName = QFileInfo(Dir, BaseName + QLatin1String("-parts.html")).absoluteFilePath();
			QFile File(FileName);

			if (!File.open(QIODevice::WriteOnly))
				return swUnexpected(tr("Could not write file '%1':\n%2").arg(FileName, File.errorString()));

			QTextStream Stream(&File);

			WriteHeader(Stream, tr("Parts used by %1").arg(PageTitle));

			const swResult<void> PartsImageSaved = AddPartsListImage(Stream, Model, 0, BaseName);

			if (!PartsImageSaved)
				return PartsImageSaved;

			Stream << QLatin1String("</CENTER>\n<BR><HR><BR>");
			Stream << QString::fromLatin1("<A HREF=\"%1-%2.html\">%3</A> ").arg(BaseName, QString("%1").arg(LastStep, 2, 10, QLatin1Char('0')), tr("Previous").toHtmlEscaped());

			if (Options.IndexPage)
				Stream << QString::fromLatin1("<A HREF=\"%1\">%2</A> ").arg(GetModelIndexFileName(BaseName).toHtmlEscaped(), tr("Index").toHtmlEscaped());

			Stream << QLatin1String("<BR></HTML>\n");
		}

		QString StepImageBaseName = QFileInfo(Dir, BaseName + QLatin1String("-%1.png")).absoluteFilePath();
		const swResult<void> ImagesSaved = Model->SaveStepImages(StepImageBaseName, true, false, Options.StepImagesWidth, Options.StepImagesHeight, 1, LastStep, swImageBackground::Transparent, Options.BackgroundColor);

		if (!ImagesSaved)
			return ImagesSaved;
	}

	if (Models.size() > 1)
	{
		QString FileName = QFileInfo(Dir, IndexFileName).absoluteFilePath();
		QFile File(FileName);

		if (!File.open(QIODevice::WriteOnly))
			return swUnexpected(tr("Could not write file '%1':\n%2").arg(FileName, File.errorString()));

		QTextStream Stream(&File);

		WriteHeader(Stream, tr("Instructions for %1").arg(ProjectTitle));

		for (const swModel* Model : Models)
		{
			const QString& BaseName = ModelBaseNames.at(Model);

			if (Options.IndexPage)
			{
				FileName = GetModelIndexFileName(BaseName);
				Stream << QString::fromLatin1("<p><a href=\"%1\">%2</a>").arg(FileName.toHtmlEscaped(), Model->GetProperties().mFileName.toHtmlEscaped());
			}
			else
			{
				swStep LastStep = Model->GetLastStep();

				Stream << QString::fromLatin1("<p>%1</p>").arg(Model->GetProperties().mFileName.toHtmlEscaped());

				for (swStep Step = 1; Step <= LastStep; Step++)
					Stream << QString::fromLatin1("<A HREF=\"%1-%2.html\">%3<BR>\n</A>").arg(BaseName, QString("%1").arg(Step, 2, 10, QLatin1Char('0')), tr("Step %1").arg(QString::number(Step)).toHtmlEscaped());

				if (Options.PartsListEnd)
					Stream << QString::fromLatin1("<A HREF=\"%1-parts.html\">%2</A><BR>\n").arg(BaseName, tr("Parts Used").toHtmlEscaped());
			}
		}

		Stream << QString::fromLatin1("</CENTER>\n<BR><HR><BR><B><I>%1</I></B><BR></HTML>\n").arg(CreatedBy);
	}

	return swResult<void>();
}
