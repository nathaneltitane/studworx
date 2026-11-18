#include "sw_global.h"
#include "sw_window_utilities.h"
#include "sw_application.h"
#include "sw_library.h"
#include "sw_model.h"
#include "sw_data_part.h"
#include "sw_string.h"
#include "sw_profile.h"
#include "sw_color_configuration.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <TlHelp32.h>
#endif

// use to read the rebrickable key, asked for once
QString swRequestRebrickableApiKey(QWidget* Parent, const QString& Purpose)
{
	const QString StoredKey = swGetProfileString(SW_PROFILE_REBRICKABLE_API_KEY);

	if (!StoredKey.isEmpty())
		return StoredKey;

	bool Ok = false;
	const QString Key = QInputDialog::getText(Parent, QCoreApplication::translate("swRebrickable", "Rebrickable API Key"), QCoreApplication::translate("swRebrickable", "%1\nEnter your Rebrickable API key, free from your account settings at rebrickable.com.\nIt can be modified under Preferences > General > Online Services.").arg(Purpose), QLineEdit::Normal, QString(), &Ok).trimmed();

	if (!Ok || Key.isEmpty())
		return QString();

	swSetProfileString(SW_PROFILE_REBRICKABLE_API_KEY, Key);

	return Key;
}

// use to pick an existing or a new color configuration file
QString swChooseColorConfigurationFile(QWidget* Parent, swColorSource Source, const QString& FileName, bool Existing)
{
	QString StartPath = FileName;

	// suggest the official name beside the parts library
	if (StartPath.isEmpty())
	{
		const QString LibraryPath = swGetPartsLibrary() ? swGetPartsLibrary()->GetLibraryPath() : swGetProfileString(SW_PROFILE_PARTS_LIBRARY);
		const QFileInfo LibraryInfo(LibraryPath);
		const QString LibraryFolder = LibraryInfo.isFile() ? LibraryInfo.absolutePath() : LibraryPath;

		StartPath = QDir(LibraryFolder).absoluteFilePath(swGetColorConfigurationOfficialName(Source));
	}

	const QString Title = Existing ? QCoreApplication::translate("swColorConfiguration", "Choose %1").arg(swGetColorConfigurationName(Source)) : QCoreApplication::translate("swColorConfiguration", "Save %1").arg(swGetColorConfigurationName(Source));
	const QString Filter = QCoreApplication::translate("swColorConfiguration", "LDraw Files (*.ldr);;All Files (*)");
	const QString Result = Existing ? QFileDialog::getOpenFileName(Parent, Title, StartPath, Filter) : QFileDialog::getSaveFileName(Parent, Title, StartPath, Filter);

	return Result.isEmpty() ? QString() : QDir::toNativeSeparators(Result);
}

// use to write the latest official file to a chosen name
bool swUpdateColorConfigurationFile(QWidget* Parent, swColorSource Source, const QString& FileName, bool ShowSummary)
{
	const QString Title = QCoreApplication::translate("swColorConfiguration", "Update %1").arg(swGetColorConfigurationName(Source));
	const swResult<void> Result = swUpdateColorConfiguration(Parent, Source, FileName);

	if (!Result)
	{
		QMessageBox::warning(Parent, Title, Result.error());
		return false;
	}

	if (ShowSummary)
		QMessageBox::information(Parent, Title, QCoreApplication::translate("swColorConfiguration", "Saved the latest %1 to '%2'.").arg(swGetColorConfigurationOfficialName(Source), FileName));

	return true;
}

// use to register and write a missing color configuration
QString swEnsureColorConfigurationFile(QWidget* Parent, swColorSource Source, const QString& FileName)
{
	if (swColorConfigurationExists(FileName))
		return FileName;

	const QString Name = swGetColorConfigurationName(Source);
	const QString Question = FileName.isEmpty() ?
		QCoreApplication::translate("swColorConfiguration", "No %1 file is set. It defines the LDraw colors and their edge lines.\n\nChoose where to save it and download the latest %2 now?").arg(Name, swGetColorConfigurationOfficialName(Source)) :
		QCoreApplication::translate("swColorConfiguration", "The %1 file '%2' was not found. It defines the LDraw colors and their edge lines.\n\nChoose where to save it and download the latest %3 now?").arg(Name, FileName, swGetColorConfigurationOfficialName(Source));

	if (QMessageBox::question(Parent, Name, Question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes) != QMessageBox::Yes)
		return QString();

	const QString ChosenName = swChooseColorConfigurationFile(Parent, Source, FileName, false);

	if (ChosenName.isEmpty() || !swUpdateColorConfigurationFile(Parent, Source, ChosenName, false))
		return QString();

	return ChosenName;
}

QPoint swGetPopupPosition(const QPoint& Position, const QSize& Size, const QPoint& ScreenPosition)
{
	QScreen* Screen = QGuiApplication::screenAt(ScreenPosition);

	if (!Screen)
		Screen = QGuiApplication::primaryScreen();

	if (!Screen)
		return Position;

	const QRect AvailableGeometry = Screen->availableGeometry();
	const int X = qBound(AvailableGeometry.left(), Position.x(), qMax(AvailableGeometry.left(), AvailableGeometry.right() - Size.width() + 1));
	const int Y = qBound(AvailableGeometry.top(), Position.y(), qMax(AvailableGeometry.top(), AvailableGeometry.bottom() - Size.height() + 1));

	return QPoint(X, Y);
}

// use to start form labels at the left edge and grow fields to the right edge
static void swAlignFormLayouts(QWidget* Window)
{
	for (QFormLayout* FormLayout : Window->findChildren<QFormLayout*>())
	{
		FormLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		FormLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	}
}

bool swWindowGeometryFilter::eventFilter(QObject* Object, QEvent* Event)
{
	if (Event->type() != QEvent::Show || !Object->isWidgetType())
		return false;

	QWidget* Window = static_cast<QWidget*>(Object);

	if (!Window->isWindow() || Window->property("swInitialGeometry").toBool())
		return false;

	// text editor prompts stay resizable
	const QInputDialog* InputDialog = qobject_cast<QInputDialog*>(Window);

	if (Window->property("swInputPrompt").toBool() || (InputDialog && !InputDialog->testOption(QInputDialog::UsePlainTextEditForTextInput)))
	{
		Window->setProperty("swInitialGeometry", true);
		swAlignFormLayouts(Window);

		// four by one up to 64 characters
		const QSize ContentSize = Window->sizeHint().expandedTo(Window->minimumSizeHint());
		const int ProportionalWidth = qMin(4 * ContentSize.height(), 64 * Window->fontMetrics().averageCharWidth());

		// widen until every field shows its whole text
		int TextOverflow = 0;

		for (const QLineEdit* LineEdit : Window->findChildren<QLineEdit*>())
		{
			const int TextWidth = LineEdit->fontMetrics().horizontalAdvance(LineEdit->text()) + 4 * LineEdit->fontMetrics().averageCharWidth();
			TextOverflow = qMax(TextOverflow, TextWidth - LineEdit->sizeHint().width());
		}

		int Width = qMax(ContentSize.width() + TextOverflow, ProportionalWidth);

		if (const QScreen* Screen = Window->screen())
			Width = qMin(Width, qMax(ContentSize.width(), Screen->availableGeometry().width() / 2));

		Window->setFixedSize(Width, ContentSize.height());

		return false;
	}

	// qt dialogs size themselves
	const bool IsDialog = qobject_cast<QDialog*>(Window) && !qobject_cast<QMessageBox*>(Window) && !qobject_cast<QInputDialog*>(Window) && !qobject_cast<QProgressDialog*>(Window) &&
		!qobject_cast<QFileDialog*>(Window) && !qobject_cast<QColorDialog*>(Window) && !qobject_cast<QFontDialog*>(Window) && !qobject_cast<QPrintDialog*>(Window);
	const bool IsMainWindow = qobject_cast<QMainWindow*>(Window);
	const bool IsSecondaryWindow = IsMainWindow && Window->parentWidget();

	if (!IsDialog && !IsMainWindow)
		return false;

	Window->setProperty("swInitialGeometry", true);
	swAlignFormLayouts(Window);

	QWidget* Parent = Window->parentWidget() ? Window->parentWidget()->window() : nullptr;
	QScreen* Screen = Parent ? Parent->screen() : Window->screen();

	if (!Screen)
		return false;

	// title bar and borders, from the parent frame

	const QSize ParentFrame = Parent ? Parent->frameGeometry().size() - Parent->size() : QSize();
	const QSize FrameSize = ParentFrame.height() > 0 ? ParentFrame : QSize(0, Window->style()->pixelMetric(QStyle::PM_TitleBarHeight));

	// a quarter of the screen, frame included

	const QRect ScreenArea = Screen->availableGeometry();
	const QRect Available(ScreenArea.topLeft(), (ScreenArea.size() - FrameSize).expandedTo(QSize(1, 1)));
	const QSize MinimumSize = QSize(Available.width() / 4, Available.height() / 4).expandedTo(Window->minimumSizeHint()).boundedTo(Available.size());

	// keep at least a quarter
	Window->setMinimumSize(MinimumSize);

	if (!IsDialog && !IsSecondaryWindow)
		return false;

	// set only the size
	QSize Size = MinimumSize.expandedTo(Window->sizeHint());
	Size = Size.boundedTo(Available.size());

	Window->resize(Size);

	return false;
}

void swSetInputPrompt(QDialog* Dialog)
{
	Dialog->setProperty("swInputPrompt", true);
}

swBlockingProgress* swBlockingProgress::mActive = nullptr;

swBlockingProgress::swBlockingProgress(const QString& Title, const QString& Label, bool Cancelable)
	: mTitle(Title), mLabel(Label), mCancelable(Cancelable)
{
	// nested waits share one task
	mNested = mActive != nullptr;

	if (!mNested)
		mActive = this;

	mTimer.start();
}

swBlockingProgress::~swBlockingProgress()
{
	delete mDialog;

	if (mActive == this)
		mActive = nullptr;
}

void swBlockingProgress::SetProgress(int Value, int Maximum)
{
	if (mNested)
		return;

	if (!mDialog)
	{
		// no dialog for short tasks
		if (Value >= Maximum || mTimer.elapsed() < 400)
			return;

		mDialog = new QProgressDialog(QApplication::activeWindow());
		mDialog->setWindowTitle(mTitle);
		mDialog->setLabelText(mLabel);
		mDialog->setWindowModality(Qt::ApplicationModal);
		mDialog->setMinimumDuration(0);
		mDialog->setAutoClose(false);
		mDialog->setAutoReset(false);

		QProgressBar* Bar = new QProgressBar(mDialog);
		Bar->setFormat(tr("%v of %m"));
		mDialog->setBar(Bar);

		if (!mCancelable)
		{
			mDialog->setCancelButton(nullptr);
			mDialog->setWindowFlags(mDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
		}

		mDialog->setRange(0, Maximum);
		mDialog->show();
		mUpdateTimer.invalidate();
	}

	// throttle the repaint updates
	if (mUpdateTimer.isValid() && mUpdateTimer.elapsed() < 50 && Value < Maximum)
		return;

	mUpdateTimer.start();
	mDialog->setMaximum(Maximum);
	mDialog->setValue(Value);
}

bool swBlockingProgress::IsCanceled() const
{
	return mDialog && mDialog->wasCanceled();
}

QColor swGetWarningTextColor()
{
	return QColor(208, 0, 0);
}

// use to keep warning text red while the row is selected
void swWarningItemDelegate::initStyleOption(QStyleOptionViewItem* Option, const QModelIndex& Index) const
{
	QStyledItemDelegate::initStyleOption(Option, Index);

	const QVariant Foreground = Index.data(Qt::ForegroundRole);

	if (Foreground.canConvert<QBrush>() && Foreground.value<QBrush>().color() == swGetWarningTextColor())
		Option->palette.setColor(QPalette::HighlightedText, swGetWarningTextColor());
}

QString swFormatValue(float Value, int Precision)
{
	QString String = QString::number(Value, 'f', Precision);
	const qsizetype Dot = String.indexOf('.');

	if (Dot != -1)
	{
		while (String.endsWith('0'))
			String.chop(1);

		if (String.endsWith('.'))
			String.chop(1);
	}

	return String;
}

QString swFormatFloatLocalized(float Value, QLocale Locale)
{
	Locale.setNumberOptions(Locale.numberOptions() | QLocale::OmitGroupSeparator);
	QString DecimalPoint = Locale.decimalPoint();
	QString String = Locale.toString(Value, 'f', 4);

	if (String.indexOf(DecimalPoint) != -1)
	{
		while (String.endsWith(Locale.zeroDigit()))
			String.chop(1);

		if (String.endsWith(DecimalPoint))
			String.chop(1);
	}

	return String;
}

QString swFormatDoubleLocalized(double Value, QLocale Locale, char Format, int Precision)
{
	Locale.setNumberOptions(Locale.numberOptions() | QLocale::OmitGroupSeparator);
	return Locale.toString(Value, Format, Precision);
}

QString swFormatIntegerLocalized(qlonglong Value, QLocale Locale)
{
	Locale.setNumberOptions(Locale.numberOptions() | QLocale::OmitGroupSeparator);
	return Locale.toString(Value);
}

std::optional<float> swParseFloatLocalized(const QString& Value, QLocale Locale)
{
	bool Ok;
	const float Number = Locale.toFloat(Value, &Ok);

	if (!Ok || !std::isfinite(Number))
		return std::nullopt;

	return Number;
}

#ifdef Q_OS_WIN

int swTerminateChildProcess(QWidget* Parent, const qint64 Pid, const qint64 Ppid)
{
	DWORD PID        = DWORD(Pid);
	DWORD PPID       = DWORD(Ppid);
	HANDLE hSnapshot = INVALID_HANDLE_VALUE, hProcess = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 PE32;

	if ((hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, PPID)) == INVALID_HANDLE_VALUE)
	{
		QMessageBox::warning(Parent, QObject::tr("Stop Blender Process"), QObject::tr("%1 failed: %2").arg("CreateToolhelp32Snapshot").arg(GetLastError()));
		return -1;
	}
	PE32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hSnapshot, &PE32) == FALSE)
	{
		QMessageBox::warning(Parent, QObject::tr("Stop Blender Process"), QObject::tr("%1 failed: %2").arg("Process32First").arg(GetLastError()));
		CloseHandle(hSnapshot);
		return -2;
	}
	do
	{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
		QRegularExpression ExeExpression("^(?:cmd\\.exe|conhost\\.exe|blender\\.exe)$", QRegularExpression::CaseInsensitiveOption);
#else
		QRegExp ExeExpression("^(?:cmd\\.exe|conhost\\.exe|blender\\.exe)$", Qt::CaseInsensitive);
#endif

		if (QString::fromWCharArray(PE32.szExeFile).contains(ExeExpression))
		{
			if ((PE32.th32ProcessID == PID && PE32.th32ParentProcessID == PPID) || PE32.th32ParentProcessID == PID)
			{
				if ((hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, PE32.th32ProcessID)) == INVALID_HANDLE_VALUE)
				{
					QMessageBox::warning(Parent, QObject::tr("Stop Blender Process"), QObject::tr("%1 failed: %2").arg("OpenProcess").arg(GetLastError()));
					return -3;
				}
				else
				{
					TerminateProcess(hProcess, 9);
					CloseHandle(hProcess);
				}
			}
		}
	}
	while (Process32Next(hSnapshot, &PE32));

	CloseHandle(hSnapshot);

	return 0;
}

bool swRunElevatedProcess(const LPCWSTR ExeName, const LPCWSTR Arguments, const LPCWSTR WorkingFolder)
{
	SHELLEXECUTEINFO ShellExecuteInfo = {};

	ShellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShellExecuteInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShellExecuteInfo.hwnd = NULL;
	ShellExecuteInfo.lpVerb = L"runas";
	ShellExecuteInfo.lpFile = ExeName;
	ShellExecuteInfo.lpParameters = Arguments;
	ShellExecuteInfo.lpDirectory = WorkingFolder;
	ShellExecuteInfo.nShow = SW_SHOW;
	ShellExecuteInfo.hInstApp = NULL;

	if (!ShellExecuteEx(&ShellExecuteInfo) || !ShellExecuteInfo.hProcess)
		return false;

	if (WaitForSingleObject(ShellExecuteInfo.hProcess, INFINITE))
		return false;

	DWORD ExitCode;

	if (!GetExitCodeProcess(ShellExecuteInfo.hProcess, &ExitCode))
		return false;

	CloseHandle(ShellExecuteInfo.hProcess);

	return ExitCode == 0;
}

#endif

// resize columns except the stretching one
swTreeWidgetColumnStretcher::swTreeWidgetColumnStretcher(QTreeWidget* TreeWidget, int ColumnToStretch)
	: QObject(TreeWidget->header()), mColumnToStretch(ColumnToStretch), mInteractiveResize(false), mStretchWidth(0)
{
	parent()->installEventFilter(this);
	connect(TreeWidget->header(), &QHeaderView::sectionResized, this, &swTreeWidgetColumnStretcher::SectionResized);

	QHideEvent Fake;
	swTreeWidgetColumnStretcher::eventFilter(parent(), &Fake);
}

void swTreeWidgetColumnStretcher::SectionResized(int LogicalIndex, int OldSize, int NewSize)
{
	Q_UNUSED(OldSize)

	if (LogicalIndex == mColumnToStretch)
	{
		QHeaderView* HeaderView = qobject_cast<QHeaderView*>(parent());

		if (HeaderView->isVisible())
			mInteractiveResize = true;

		mStretchWidth = NewSize;
	}
}

bool swTreeWidgetColumnStretcher::eventFilter(QObject* Object, QEvent* Event)
{
	if (Object == parent())
	{
		QHeaderView* HeaderView = qobject_cast<QHeaderView*>(Object);

		if (Event->type() == QEvent::Show)
		{
			for (int i = 0; i < HeaderView->count(); ++i)
				HeaderView->setSectionResizeMode(i, QHeaderView::Interactive);

			mStretchWidth = HeaderView->sectionSize(mColumnToStretch);

		}
		else if (Event->type() == QEvent::Hide)
		{
			if (!mInteractiveResize)
				for (int i = 0; i < HeaderView->count(); ++i)
					HeaderView->setSectionResizeMode(i, i == mColumnToStretch ? QHeaderView::Stretch : QHeaderView::ResizeToContents);
		}
		else if (Event->type() == QEvent::Resize)
		{
			if (HeaderView->sectionResizeMode(mColumnToStretch) == QHeaderView::Interactive) {

				const int StretchWidth = HeaderView->isVisible() ? mStretchWidth : 32;

				HeaderView->resizeSection(mColumnToStretch, StretchWidth);
			}
		}
	}

	return false;
}

swPartIdStringModel::swPartIdStringModel(swModel* Model, QObject* Parent)
	: QAbstractListModel(Parent)
{
	swPartsLibrary* Library = swGetPartsLibrary();
	mSortedParts = Library->GetParts();

	mSortedParts.erase(std::remove_if(mSortedParts.begin(), mSortedParts.end(), [Model](swPartData* PartData)
	{
		return PartData->IsModel() && PartData->GetModel()->IncludesModel(Model);
	}), mSortedParts.end());

	auto PartCompare = [](swPartData* PartData1, swPartData* PartData2)
	{
		return strcmp(PartData1->m_strDescription, PartData2->m_strDescription) < 0;
	};

	std::sort(mSortedParts.begin(), mSortedParts.end(), PartCompare);
}

QModelIndex swPartIdStringModel::Index(swPartData* PartData) const
{
	for (size_t PartDataIndex = 0; PartDataIndex < mSortedParts.size(); PartDataIndex++)
		if (mSortedParts[PartDataIndex] == PartData)
			return index(static_cast<int>(PartDataIndex), 0);

	return QModelIndex();
}

int swPartIdStringModel::rowCount(const QModelIndex& Parent) const
{
	Q_UNUSED(Parent);

	return static_cast<int>(mSortedParts.size());
}

QVariant swPartIdStringModel::data(const QModelIndex& Index, int Role) const
{
	if (Index.row() < static_cast<int>(mSortedParts.size()))
	{
		if (Role == Qt::DisplayRole)
			return QString::fromLatin1(mSortedParts[Index.row()]->m_strDescription);
		else if (Role == Qt::UserRole)
			return QVariant::fromValue(reinterpret_cast<void*>(mSortedParts[Index.row()]));
	}

	return QVariant();
}

swColorDialogPopup::swColorDialogPopup(const QColor& InitialColor, bool ShowAlphaChannel, QWidget* Parent)
	: QWidget(Parent)
{
	setAccessibleName(tr("Choose Color"));
	setWhatsThis(tr("<p><b>Choose Color</b></p><p>Pick a color using the swatches or enter its component values. Choose OK to use the selected color, or Cancel to keep the original color. Editing the controls alone does not apply the color.</p>"));

	QVBoxLayout* Layout = new QVBoxLayout(this);
	Layout->setContentsMargins(0, 0, 0, 0);

	mColorDialog = new QColorDialog(InitialColor, this);
	mColorDialog->setWindowFlags(Qt::Widget);
	mColorDialog->setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::NoButtons);
	mColorDialog->setOption(QColorDialog::ShowAlphaChannel, ShowAlphaChannel);

	// transparent start picks opaque
	mTransparentStart = ShowAlphaChannel && InitialColor.alpha() == 0;

	QObject::connect(mColorDialog, &QColorDialog::currentColorChanged, this, [this](const QColor& Color)
	{
		if (!mTransparentStart || Color.alpha() != 0)
			return;

		mTransparentStart = false;

		QColor OpaqueColor(Color);
		OpaqueColor.setAlpha(255);
		mColorDialog->setCurrentColor(OpaqueColor);
	});

	Layout->addWidget(mColorDialog);
	mColorDialog->setAccessibleName(tr("Custom Color"));
	mColorDialog->setToolTip(tr("Choose a color or enter its component values."));

	QDialogButtonBox* ButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
	Layout->addWidget(ButtonBox);
	swSetupWidgetHelp(this);

	QObject::connect(ButtonBox, &QDialogButtonBox::accepted, this, &swColorDialogPopup::Accept);
	QObject::connect(ButtonBox, &QDialogButtonBox::rejected, this, &swColorDialogPopup::Reject);
}

void swColorDialogPopup::Accept()
{
	QColor Color = mColorDialog->currentColor();

	emit ColorSelected(Color);

	Close();
}

void swColorDialogPopup::Reject()
{
	Close();
}

void swColorDialogPopup::Close()
{
	QMenu* Menu = qobject_cast<QMenu*>(parent());

	if (Menu)
		Menu->close();
}
