#include "sw_global.h"
#include "sw_dialog_missing_parts.h"
#include "sw_application.h"
#include "sw_profile.h"
#include "sw_data_part.h"
#include "sw_library.h"
#include "sw_project.h"
#include "sw_model.h"
#include "sw_window_main.h"

// parts table columns
enum swMissingPartsColumn
{
	SW_MISSING_PARTS_PART,
	SW_MISSING_PARTS_REPLACEMENT,
	SW_MISSING_PARTS_COUNT
};

swMissingPartsDialog::swMissingPartsDialog(QWidget* Parent, std::vector<swMissingPart> MissingParts)
	: QDialog(Parent), mMissingParts(std::move(MissingParts))
{
	const int PartCount = static_cast<int>(mMissingParts.size());

	setWindowTitle(tr("Missing and Moved Parts"));
	setWhatsThis(tr("<p><b>Missing and Moved Parts</b></p><p>Lists the parts of the model that could not be loaded, shown as red-outlined placeholder bricks in the model view, and the parts the library marks as moved (~Moved to) or as an alias (=) of another part.</p><p>A missing part is looked up through the BrickLink mapping file set under Preferences &gt; General &gt; Reference Paths, for parts the library now files under another number. A moved part or an alias is offered the part it points to. Replace Checked swaps the checked parts in every submodel, keeping their colors and positions, then saves the file. The swap can still be undone and saved again.</p><p>The warning can be turned off under Preferences &gt; General &gt; Files, File &gt; Missing and Moved Parts and the status bar icon open this list again.</p>"));

	QVBoxLayout* MainLayout = new QVBoxLayout(this);

	const int FailedCount = static_cast<int>(std::count_if(mMissingParts.begin(), mMissingParts.end(), [](const swMissingPart& MissingPart) { return MissingPart.PartData->mState == swAssetState::Failed; }));
	const int MovedCount = PartCount - FailedCount;
	QStringList Messages;

	if (FailedCount)
		Messages.append(FailedCount == 1 ? tr("1 part could not be loaded. It shows as a red-outlined placeholder brick until replaced.") : tr("%1 parts could not be loaded. They show as red-outlined placeholder bricks until replaced.").arg(FailedCount));

	if (MovedCount)
		Messages.append(MovedCount == 1 ? tr("1 part is marked as moved or as an alias in the parts library.") : tr("%1 parts are marked as moved or as an alias in the parts library.").arg(MovedCount));

	QLabel* MessageLabel = new QLabel(Messages.join(QLatin1Char('\n')), this);
	MessageLabel->setWordWrap(true);
	MainLayout->addWidget(MessageLabel);

	constexpr int ThumbnailSize = 64;
	mPartsTable = new QTableWidget(PartCount, SW_MISSING_PARTS_COUNT, this);
	mPartsTable->setHorizontalHeaderLabels({ tr("Part"), tr("Replacement") });
	mPartsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	mPartsTable->verticalHeader()->hide();
	mPartsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	mPartsTable->setSelectionMode(QAbstractItemView::NoSelection);
	mPartsTable->setWordWrap(true);
	mPartsTable->setIconSize(QSize(ThumbnailSize, ThumbnailSize));
	mPartsTable->setAccessibleName(tr("Missing and Moved Parts"));
	mPartsTable->setToolTip(tr("Check the parts to replace with the part found in the library."));
	mPartsTable->verticalHeader()->setMinimumSectionSize(ThumbnailSize + 8);
	mPartsTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	MainLayout->addWidget(mPartsTable, 1);

	swThumbnailManager* ThumbnailManager = swGetPartsLibrary()->GetThumbnailManager();
	connect(ThumbnailManager, &swThumbnailManager::ThumbnailReady, this, &swMissingPartsDialog::ThumbnailReady);

	for (int Row = 0; Row < PartCount; Row++)
	{
		const swMissingPart& MissingPart = mMissingParts[Row];
		QTableWidgetItem* PartItem = new QTableWidgetItem(QStringLiteral("%1\n%2\n%3").arg(QString::fromLatin1(MissingPart.PartData->mFileName), MissingPart.Error, GetUsageText(MissingPart)));
		QTableWidgetItem* ReplacementItem = new QTableWidgetItem();

		if (MissingPart.Replacement && !MissingPart.Models.empty())
		{
			PartItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
			PartItem->setCheckState(Qt::Checked);
			ReplacementItem->setText(QStringLiteral("%1\n%2\n%3").arg(QString::fromLatin1(MissingPart.Replacement->mFileName), QString::fromLatin1(MissingPart.Replacement->m_strDescription), MissingPart.Origin));

			auto [ThumbnailId, Thumbnail] = ThumbnailManager->RequestThumbnail(MissingPart.Replacement, MissingPart.ColorIndex, ThumbnailSize, devicePixelRatioF());
			mThumbnails.emplace_back(ThumbnailId, ReplacementItem);

			if (!Thumbnail.isNull())
				ReplacementItem->setData(Qt::DecorationRole, Thumbnail);
		}
		else
		{
			PartItem->setFlags(Qt::ItemIsEnabled);
			ReplacementItem->setText(MissingPart.Models.empty() ? tr("Used inside a library part, it can not be replaced here") : tr("No replacement found"));
		}

		mPartsTable->setItem(Row, SW_MISSING_PARTS_PART, PartItem);
		mPartsTable->setItem(Row, SW_MISSING_PARTS_REPLACEMENT, ReplacementItem);
	}

	mDontWarnCheck = new QCheckBox(tr("Don't Warn Again"), this);
	mDontWarnCheck->setChecked(!swGetPreferences().mWarnPartsLoadFailure);
	mDontWarnCheck->setToolTip(tr("Stop showing this list when a model opens, the status bar icon still opens it."));
	MainLayout->addWidget(mDontWarnCheck);

	QDialogButtonBox* ButtonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
	QPushButton* CopyButton = ButtonBox->addButton(tr("Copy List"), QDialogButtonBox::ActionRole);
	CopyButton->setToolTip(tr("Copy the list of parts not loaded to the clipboard."));
	mReplaceButton = ButtonBox->addButton(tr("Replace Checked"), QDialogButtonBox::AcceptRole);
	mReplaceButton->setToolTip(tr("Replace the checked parts in every submodel and save the file, it can be undone."));
	MainLayout->addWidget(ButtonBox);

	connect(ButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(CopyButton, &QPushButton::clicked, this, &swMissingPartsDialog::CopyList);
	connect(mReplaceButton, &QPushButton::clicked, this, &swMissingPartsDialog::ReplaceChecked);
	connect(mPartsTable, &QTableWidget::itemChanged, this, &swMissingPartsDialog::UpdateReplaceButton);
	connect(this, &QDialog::finished, this, [this]()
	{
		swGetPreferences().mWarnPartsLoadFailure = !mDontWarnCheck->isChecked();
		swSetProfileInteger(SW_PROFILE_WARN_PARTS_LOAD_FAILURE, swGetPreferences().mWarnPartsLoadFailure);
	});

	// focus close, not the table

	ButtonBox->button(QDialogButtonBox::Close)->setFocus();

	UpdateReplaceButton();
	resize(640, 400);
}

swMissingPartsDialog::~swMissingPartsDialog()
{
	swThumbnailManager* ThumbnailManager = swGetPartsLibrary()->GetThumbnailManager();

	for (const auto& [ThumbnailId, Item] : mThumbnails)
		ThumbnailManager->ReleaseThumbnail(ThumbnailId);
}

QString swMissingPartsDialog::GetUsageText(const swMissingPart& MissingPart) const
{
	QStringList Usage;

	for (const auto& [FileName, Count] : MissingPart.Models)
		Usage.append(tr("%1× in %2").arg(Count).arg(FileName));

	return Usage.isEmpty() ? tr("Used inside a library part") : tr("Used %1").arg(Usage.join(QLatin1String(", ")));
}

void swMissingPartsDialog::ThumbnailReady(swPartThumbnailId ThumbnailId, QPixmap Pixmap)
{
	for (const auto& [Id, Item] : mThumbnails)
		if (Id == ThumbnailId)
			Item->setData(Qt::DecorationRole, Pixmap);
}

void swMissingPartsDialog::UpdateReplaceButton()
{
	bool Checked = false;

	for (int Row = 0; Row < mPartsTable->rowCount() && !Checked; Row++)
		Checked = mPartsTable->item(Row, SW_MISSING_PARTS_PART)->checkState() == Qt::Checked && (mPartsTable->item(Row, SW_MISSING_PARTS_PART)->flags() & Qt::ItemIsUserCheckable);

	mReplaceButton->setEnabled(Checked);
}

void swMissingPartsDialog::ReplaceChecked()
{
	Project* ActiveProject = swGetActiveProject();
	swModel* MainModel = ActiveProject ? ActiveProject->GetMainModel() : nullptr;
	std::map<swPartData*, swPartData*> Replacements;

	for (int Row = 0; Row < mPartsTable->rowCount(); Row++)
	{
		const QTableWidgetItem* PartItem = mPartsTable->item(Row, SW_MISSING_PARTS_PART);

		if ((PartItem->flags() & Qt::ItemIsUserCheckable) && PartItem->checkState() == Qt::Checked)
			Replacements.emplace(mMissingParts[Row].PartData, mMissingParts[Row].Replacement);
	}

	if (MainModel && !Replacements.empty())
		MainModel->ReplacePartData(Replacements);

	accept();

	// replacing is the approval to save

	if (MainModel && !Replacements.empty())
		gMainWindow->mActions[SW_FILE_SAVE]->trigger();
}

void swMissingPartsDialog::CopyList()
{
	QStringList Lines;

	for (const swMissingPart& MissingPart : mMissingParts)
	{
		QString Line = QStringLiteral("%1: %2, %3").arg(QString::fromLatin1(MissingPart.PartData->mFileName), MissingPart.Error, GetUsageText(MissingPart));

		if (MissingPart.Replacement)
			Line += tr(", replacement %1").arg(QString::fromLatin1(MissingPart.Replacement->mFileName));

		Lines.append(Line);
	}

	QApplication::clipboard()->setText(Lines.join(QLatin1Char('\n')));
}
