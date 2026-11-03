#include "sw_global.h"
#include "sw_dialog_export_bricklink.h"
#include "sw_window_utilities.h"
#include "sw_profile.h"

// mapping files are json text
static const char* swBrickLinkMappingFilter = QT_TRANSLATE_NOOP("swBrickLinkExportDialog", "Mapping Files (*.txt *.json);;All Files (*.*)");

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
	setWhatsThis(tr("<p><b>Export BrickLink</b></p><p>Saves the part list as a BrickLink XML wanted list. Part numbers and colors are translated through the BrickLink mapping file set under Preferences &gt; General &gt; Reference Paths.</p><p>The wanted list takes the address of its BrickLink page or its number. After saving, Copy XML and Open BrickLink puts the list on the clipboard and opens the BrickLink upload page to paste it into.</p><p>The choices are remembered for the next export.</p>"));

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

	// keep the rows together at the top of the minimum dialog size
	MainLayout->addStretch(1);

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

	return Options;
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
