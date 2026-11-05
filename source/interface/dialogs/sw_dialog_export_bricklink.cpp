#include "sw_global.h"
#include "sw_dialog_export_bricklink.h"
#include "sw_window_utilities.h"
#include "sw_profile.h"

// mapping files are json text
static const char* swBrickLinkMappingFilter = QT_TRANSLATE_NOOP("swBrickLinkExportDialog", "Mapping Files (*.txt *.json);;All Files (*)");

// exceptions files are comma or tab separated text
static const char* swBrickLinkExceptionsFilter = QT_TRANSLATE_NOOP("swBrickLinkExportDialog", "Exception Files (*.csv *.tsv *.txt);;All Files (*)");

// exceptions table columns
enum swBrickLinkExceptionsColumn
{
	SW_BRICKLINK_EXCEPTIONS_PART,
	SW_BRICKLINK_EXCEPTIONS_BRICKLINK,
	SW_BRICKLINK_EXCEPTIONS_COUNT
};

// use to turn a pasted wanted list address into its number
class swBrickLinkWantedListValidator : public QValidator
{
public:
	using QValidator::QValidator;

	State validate(QString& Input, int& Position) const override
	{
		const std::optional<QString> WantedListId = swParseBrickLinkWantedListId(Input);

		if (!WantedListId)
			return Invalid;

		if (*WantedListId != Input)
		{
			Input = *WantedListId;
			Position = Input.length();
		}

		return Acceptable;
	}
};

swBrickLinkExportDialog::swBrickLinkExportDialog(QWidget* Parent)
	: QDialog(Parent)
{
	const swBrickLinkOptions Options = swGetBrickLinkOptions();

	setWindowTitle(tr("Export BrickLink"));
	setWhatsThis(tr("<p><b>Export BrickLink</b></p><p>Saves the part list as a BrickLink XML wanted list. Part numbers and colors are translated through the BrickLink mapping file set under Preferences &gt; General &gt; Reference Paths, parts the library moved to a new number take that number.</p><p>The wanted list takes the address of its BrickLink page or its number. After saving, Copy XML and Open BrickLink puts the list on the clipboard and opens the BrickLink upload page to paste it into.</p><p>Part Exceptions replace the mapping for the parts BrickLink does not take, they are saved to their file and used by every export.</p><p>The choices are remembered for the next export.</p>"));

	QVBoxLayout* MainLayout = new QVBoxLayout(this);

	QGroupBox* WantedListGroup = new QGroupBox(tr("Wanted List"), this);
	QGridLayout* WantedListLayout = new QGridLayout(WantedListGroup);
	MainLayout->addWidget(WantedListGroup);

	const QString ConditionHelp = tr("Choose the condition wanted for every part: any condition, new only, or used only.");
	QLabel* ConditionLabel = new QLabel(tr("Condition"), WantedListGroup);
	mConditionCombo = new QComboBox(WantedListGroup);
	mConditionCombo->addItem(tr("Any"));
	mConditionCombo->addItem(tr("New"));
	mConditionCombo->addItem(tr("Used"));
	mConditionCombo->setCurrentIndex(static_cast<int>(Options.Condition));
	mConditionCombo->setAccessibleName(tr("Condition"));
	mConditionCombo->setToolTip(ConditionHelp);
	ConditionLabel->setToolTip(ConditionHelp);
	ConditionLabel->setBuddy(mConditionCombo);

	const QString WantedListHelp = tr("Paste the address of the BrickLink wanted list page or enter its number to add the parts to it, or leave empty to use the main wanted list.");
	QLabel* WantedListLabel = new QLabel(tr("Wanted List"), WantedListGroup);
	mWantedListEdit = new QLineEdit(swParseBrickLinkWantedListId(Options.WantedListId).value_or(QString()), WantedListGroup);
	mWantedListEdit->setValidator(new swBrickLinkWantedListValidator(mWantedListEdit));
	mWantedListEdit->setPlaceholderText(tr("Main wanted list"));
	mWantedListEdit->setAccessibleName(tr("Wanted List"));
	mWantedListEdit->setToolTip(WantedListHelp);
	WantedListLabel->setToolTip(WantedListHelp);
	WantedListLabel->setBuddy(mWantedListEdit);

	mNotifyCheck = new QCheckBox(tr("Notify New Listings"), WantedListGroup);
	mNotifyCheck->setChecked(Options.Notify);
	mNotifyCheck->setToolTip(tr("Ask BrickLink to send an email when new lots matching these parts are listed."));

	WantedListLayout->addWidget(ConditionLabel, 0, 0);
	WantedListLayout->addWidget(mConditionCombo, 0, 1);
	WantedListLayout->addWidget(WantedListLabel, 1, 0);
	WantedListLayout->addWidget(mWantedListEdit, 1, 1);
	WantedListLayout->addWidget(mNotifyCheck, 2, 1);
	WantedListLayout->setColumnStretch(1, 1);

	QGroupBox* ExceptionsGroup = new QGroupBox(tr("Part Exceptions"), this);
	QGridLayout* ExceptionsLayout = new QGridLayout(ExceptionsGroup);
	MainLayout->addWidget(ExceptionsGroup, 1);

	ExceptionsGroup->setWhatsThis(tr("<p><b>Part Exceptions</b></p><p>Each row replaces the BrickLink number of a library part, for parts the mapping gets wrong or BrickLink does not know. Exceptions come before the mapping and the moved part numbers.</p><p>The rows are saved to the exceptions file when exporting, one part and its BrickLink number a line. The file can also be edited as comma or tab separated text.</p>"));

	const QString ExceptionsHelp = tr("The comma or tab separated file the part exceptions are kept in, the StudWorx data folder keeps it when empty.");
	QLabel* ExceptionsLabel = new QLabel(tr("Exceptions"), ExceptionsGroup);
	mExceptionsEdit = new QLineEdit(Options.ExceptionsPath, ExceptionsGroup);
	mExceptionsEdit->setPlaceholderText(swGetBrickLinkExceptionsPath(QString()));
	mExceptionsEdit->setAccessibleName(tr("Exceptions"));
	mExceptionsEdit->setToolTip(ExceptionsHelp);
	ExceptionsLabel->setToolTip(ExceptionsHelp);
	ExceptionsLabel->setBuddy(mExceptionsEdit);

	QToolButton* BrowseButton = new QToolButton(ExceptionsGroup);
	BrowseButton->setText(tr("..."));
	BrowseButton->setToolTip(tr("Choose the exceptions file, the exceptions of an existing one are loaded."));
	BrowseButton->setAccessibleName(tr("Browse"));

	QHBoxLayout* ExceptionsFileLayout = new QHBoxLayout();
	ExceptionsFileLayout->addWidget(mExceptionsEdit, 1);
	ExceptionsFileLayout->addWidget(BrowseButton);

	mExceptionsTable = new QTableWidget(0, SW_BRICKLINK_EXCEPTIONS_COUNT, ExceptionsGroup);
	mExceptionsTable->setHorizontalHeaderLabels({ tr("Part"), tr("BrickLink Part") });
	mExceptionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	mExceptionsTable->verticalHeader()->hide();
	mExceptionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	mExceptionsTable->setAccessibleName(tr("Part Exceptions"));
	mExceptionsTable->setToolTip(tr("The library part number, such as u8200, and the BrickLink number it is exported as."));

	QPushButton* AddButton = new QPushButton(tr("Add"), ExceptionsGroup);
	AddButton->setToolTip(tr("Add an exception row and start typing its library part number."));
	mRemoveButton = new QPushButton(tr("Remove"), ExceptionsGroup);
	mRemoveButton->setToolTip(tr("Remove the selected exception rows."));

	QHBoxLayout* ExceptionsButtonLayout = new QHBoxLayout();
	ExceptionsButtonLayout->addWidget(AddButton);
	ExceptionsButtonLayout->addWidget(mRemoveButton);
	ExceptionsButtonLayout->addStretch(1);

	ExceptionsLayout->addWidget(ExceptionsLabel, 0, 0);
	ExceptionsLayout->addLayout(ExceptionsFileLayout, 0, 1);
	ExceptionsLayout->addWidget(mExceptionsTable, 1, 1);
	ExceptionsLayout->addLayout(ExceptionsButtonLayout, 2, 1);
	ExceptionsLayout->setColumnStretch(1, 1);
	ExceptionsLayout->setRowStretch(1, 1);

	// line the controls of both groups up on one label column
	const int LabelWidth = std::max({ ConditionLabel->sizeHint().width(), WantedListLabel->sizeHint().width(), ExceptionsLabel->sizeHint().width() });
	WantedListLayout->setColumnMinimumWidth(0, LabelWidth);
	ExceptionsLayout->setColumnMinimumWidth(0, LabelWidth);

	connect(BrowseButton, &QToolButton::clicked, this, &swBrickLinkExportDialog::BrowseExceptions);
	connect(mExceptionsEdit, &QLineEdit::editingFinished, this, &swBrickLinkExportDialog::LoadExceptions);
	connect(AddButton, &QPushButton::clicked, this, &swBrickLinkExportDialog::AddException);
	connect(mRemoveButton, &QPushButton::clicked, this, &swBrickLinkExportDialog::RemoveExceptions);
	connect(mExceptionsTable, &QTableWidget::itemSelectionChanged, this, &swBrickLinkExportDialog::UpdateExceptionButtons);

	LoadExceptions();

	QDialogButtonBox* ButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	ButtonBox->button(QDialogButtonBox::Ok)->setText(tr("Export..."));
	MainLayout->addWidget(ButtonBox);

	connect(ButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(ButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	swSetupDialogHelp(this);
}

swBrickLinkOptions swBrickLinkExportDialog::GetOptions() const
{
	swBrickLinkOptions Options;

	Options.Condition = static_cast<swBrickLinkCondition>(mConditionCombo->currentIndex());
	Options.WantedListId = swParseBrickLinkWantedListId(mWantedListEdit->text()).value_or(QString());
	Options.Notify = mNotifyCheck->isChecked();
	Options.ExceptionsPath = mExceptionsEdit->text().trimmed();

	return Options;
}

// use to save the exceptions before closing, a failed save keeps the dialog open
void swBrickLinkExportDialog::accept()
{
	const swResult<void> Result = swSaveBrickLinkExceptions(swGetBrickLinkExceptionsPath(mExceptionsEdit->text()), GetExceptions());

	if (!Result)
	{
		QMessageBox::warning(this, windowTitle(), Result.error());
		return;
	}

	QDialog::accept();
}

// use to choose the exceptions file
void swBrickLinkExportDialog::BrowseExceptions()
{
	const QString FileName = QFileDialog::getSaveFileName(this, tr("BrickLink Exceptions"), swGetBrickLinkExceptionsPath(mExceptionsEdit->text()), tr(swBrickLinkExceptionsFilter), nullptr, QFileDialog::DontConfirmOverwrite);

	if (FileName.isEmpty())
		return;

	mExceptionsEdit->setText(QDir::toNativeSeparators(FileName));
	LoadExceptions();
}

// use to show the exceptions of an existing file, a new file keeps the rows
void swBrickLinkExportDialog::LoadExceptions()
{
	const QString FileName = swGetBrickLinkExceptionsPath(mExceptionsEdit->text());

	if (QFileInfo::exists(FileName))
	{
		const swResult<swBrickLinkExceptions> Exceptions = swLoadBrickLinkExceptions(FileName);

		if (Exceptions)
			SetExceptions(Exceptions.value());
		else
			QMessageBox::warning(this, windowTitle(), Exceptions.error());
	}

	UpdateExceptionButtons();
}

// use to add a row and edit its part
void swBrickLinkExportDialog::AddException()
{
	const int Row = mExceptionsTable->rowCount();

	mExceptionsTable->insertRow(Row);

	for (int Column = 0; Column < SW_BRICKLINK_EXCEPTIONS_COUNT; Column++)
		mExceptionsTable->setItem(Row, Column, new QTableWidgetItem());

	mExceptionsTable->setCurrentCell(Row, SW_BRICKLINK_EXCEPTIONS_PART);
	mExceptionsTable->editItem(mExceptionsTable->item(Row, SW_BRICKLINK_EXCEPTIONS_PART));
}

// use to remove the selected rows
void swBrickLinkExportDialog::RemoveExceptions()
{
	std::set<int> Rows;

	for (const QModelIndex& Index : mExceptionsTable->selectionModel()->selectedIndexes())
		Rows.insert(Index.row());

	for (auto RowIt = Rows.rbegin(); RowIt != Rows.rend(); ++RowIt)
		mExceptionsTable->removeRow(*RowIt);

	UpdateExceptionButtons();
}

void swBrickLinkExportDialog::UpdateExceptionButtons()
{
	mRemoveButton->setEnabled(!mExceptionsTable->selectionModel()->selectedIndexes().isEmpty());
}

// use to read the filled rows, a part number loses its extension like in the file
swBrickLinkExceptions swBrickLinkExportDialog::GetExceptions() const
{
	swBrickLinkExceptions Exceptions;

	for (int Row = 0; Row < mExceptionsTable->rowCount(); Row++)
	{
		const QTableWidgetItem* PartItem = mExceptionsTable->item(Row, SW_BRICKLINK_EXCEPTIONS_PART);
		const QTableWidgetItem* BrickLinkItem = mExceptionsTable->item(Row, SW_BRICKLINK_EXCEPTIONS_BRICKLINK);
		const QString Part = PartItem ? PartItem->text().trimmed() : QString();
		const QString BrickLinkPart = BrickLinkItem ? BrickLinkItem->text().trimmed() : QString();

		if (!Part.isEmpty() && !BrickLinkPart.isEmpty())
			Exceptions.emplace_back(Part, BrickLinkPart);
	}

	return Exceptions;
}

void swBrickLinkExportDialog::SetExceptions(const swBrickLinkExceptions& Exceptions)
{
	mExceptionsTable->setRowCount(0);

	for (const auto& [Part, BrickLinkPart] : Exceptions)
	{
		const int Row = mExceptionsTable->rowCount();

		mExceptionsTable->insertRow(Row);
		mExceptionsTable->setItem(Row, SW_BRICKLINK_EXCEPTIONS_PART, new QTableWidgetItem(Part));
		mExceptionsTable->setItem(Row, SW_BRICKLINK_EXCEPTIONS_BRICKLINK, new QTableWidgetItem(BrickLinkPart));
	}
}

// use to choose where the mapping file is saved
QString swChooseBrickLinkMappingFile(QWidget* Parent, const QString& FileName)
{
	const QString StartPath = !FileName.isEmpty() ? FileName : swGetProfileString(SW_PROFILE_PROJECTS_PATH);
	const QString Result = QFileDialog::getSaveFileName(Parent, QCoreApplication::translate("swBrickLinkExportDialog", "Save BrickLink Mapping"), StartPath, QCoreApplication::translate("swBrickLinkExportDialog", swBrickLinkMappingFilter));

	return Result.isEmpty() ? QString() : QDir::toNativeSeparators(Result);
}

// use to build or refresh a mapping file
bool swUpdateBrickLinkMappingFile(QWidget* Parent, const QString& FileName, bool ShowSummary, const QString& ApiKey)
{
	const QString Title = QCoreApplication::translate("swBrickLinkExportDialog", "Update BrickLink Mapping");
	const QString Key = !ApiKey.isEmpty() ? ApiKey : swRequestRebrickableApiKey(Parent, QCoreApplication::translate("swBrickLinkExportDialog", "BrickLink part numbers and colors are downloaded from Rebrickable."));

	if (Key.isEmpty())
		return false;

	const swResult<int> Result = swUpdateBrickLinkMapping(Parent, FileName, Key);

	if (!Result)
	{
		if (!Result.error().isEmpty())
			QMessageBox::warning(Parent, Title, Result.error());

		return false;
	}

	if (ShowSummary)
		QMessageBox::information(Parent, Title, QCoreApplication::translate("swBrickLinkExportDialog", "Saved %1 part numbers to '%2'.").arg(Result.value()).arg(FileName));

	return true;
}

// use to rebuild a set mapping file gone missing
static bool swEnsureBrickLinkMapping(QWidget* Parent)
{
	if (swBrickLinkMappingExists())
		return true;

	const QString Title = QCoreApplication::translate("swBrickLinkExportDialog", "Export BrickLink");
	const QString MappingPath = swGetBrickLinkMappingPath();
	const QString Question = QCoreApplication::translate("swBrickLinkExportDialog", "The BrickLink mapping file '%1' was not found. It translates LDraw part numbers and colors to BrickLink.\n\nChoose where to save it and build it now?").arg(MappingPath);

	if (QMessageBox::question(Parent, Title, Question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes) != QMessageBox::Yes)
		return false;

	const QString FileName = swChooseBrickLinkMappingFile(Parent, MappingPath);

	if (FileName.isEmpty())
		return false;

	swSetProfileString(SW_PROFILE_BRICKLINK_MAPPING, FileName);

	return swUpdateBrickLinkMappingFile(Parent, FileName, false);
}

std::optional<swBrickLinkOptions> swRequestBrickLinkExport(QWidget* Parent)
{
	swBrickLinkExportDialog Dialog(Parent);

	if (Dialog.exec() != QDialog::Accepted)
		return std::nullopt;

	const swBrickLinkOptions Options = Dialog.GetOptions();
	swSetBrickLinkOptions(Options);

	if (!swEnsureBrickLinkMapping(Parent))
		return std::nullopt;

	return Options;
}

void swShowBrickLinkExportSummary(QWidget* Parent, const swBrickLinkExport& Export)
{
	const QString Title = QCoreApplication::translate("swBrickLinkExportDialog", "Export BrickLink");
	const QString FileName = QDir::toNativeSeparators(Export.FileName);
	const QString Summary = Export.Lots == 1 ? QCoreApplication::translate("swBrickLinkExportDialog", "Saved 1 lot to '%1'.").arg(FileName) : QCoreApplication::translate("swBrickLinkExportDialog", "Saved %1 lots to '%2'.").arg(Export.Lots).arg(FileName);
	QMessageBox MessageBox(QMessageBox::Information, Title, Summary, QMessageBox::Close, Parent);
	QPushButton* UploadButton = MessageBox.addButton(QCoreApplication::translate("swBrickLinkExportDialog", "Copy XML and Open BrickLink"), QMessageBox::ActionRole);
	UploadButton->setToolTip(QCoreApplication::translate("swBrickLinkExportDialog", "Copy the saved XML to the clipboard and open the BrickLink wanted list upload page to paste it into."));
	MessageBox.setDefaultButton(UploadButton);
	MessageBox.exec();

	if (MessageBox.clickedButton() != UploadButton)
		return;

	QFile File(Export.FileName);

	if (!File.open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(Parent, Title, QCoreApplication::translate("swBrickLinkExportDialog", "Could not read file '%1'.").arg(FileName));
		return;
	}

	QApplication::clipboard()->setText(QString::fromUtf8(File.readAll()));
	QDesktopServices::openUrl(QUrl(swGetBrickLinkUploadUrl()));
}
