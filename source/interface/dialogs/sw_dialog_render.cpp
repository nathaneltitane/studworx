#include "sw_global.h"
#include "sw_dialog_render.h"
#include "ui_sw_dialog_render.h"
#include "sw_project.h"
#include "sw_application.h"
#include "sw_profile.h"
#include "sw_model.h"
#include "sw_object_camera.h"
#include "sw_window_utilities.h"
#include "sw_renderer_povray.h"
#include "sw_renderer_blender.h"

#ifndef SW_DISABLE_RENDER_DIALOG

#define SW_POVRAY_PREVIEW_WIDTH 768
#define SW_POVRAY_PREVIEW_HEIGHT 432

// raise to drop old panel layouts
#define SW_RENDER_WINDOW_STATE_VERSION 3
#define SW_RENDER_PANEL_WIDTH 380
#define SW_RENDER_LOG_HEIGHT 240

void swRenderPreviewWidget::resizeEvent(QResizeEvent* Event)
{
	mScaledImage = QImage();

	QWidget::resizeEvent(Event);
}

// use to find the canvas in the preview
QRect swRenderPreviewWidget::GetCanvasRect() const
{
	const QSize ImageSize = !mImage.isNull() ? mImage.size() : mCanvasSize;

	if (ImageSize.isEmpty())
		return QRect();

	const int Margin = style()->pixelMetric(QStyle::PM_LayoutLeftMargin);
	const QRect AreaRect = rect().adjusted(Margin, Margin, -Margin, -Margin);
	QRect CanvasRect(QPoint(0, 0), ImageSize.scaled(AreaRect.size(), Qt::KeepAspectRatio));
	CanvasRect.moveCenter(AreaRect.center());

	return CanvasRect;
}

// use to get a canvas fraction
QPointF swRenderPreviewWidget::GetCanvasPoint(const QPoint& Position) const
{
	const QRect CanvasRect = GetCanvasRect();

	if (CanvasRect.isEmpty())
		return QPointF();

	return QPointF(std::clamp(qreal(Position.x() - CanvasRect.left()) / CanvasRect.width(), 0.0, 1.0), std::clamp(qreal(Position.y() - CanvasRect.top()) / CanvasRect.height(), 0.0, 1.0));
}

void swRenderPreviewWidget::mousePressEvent(QMouseEvent* MouseEvent)
{
	if (!mRegionMode || MouseEvent->button() != Qt::LeftButton || GetCanvasRect().isEmpty())
	{
		QWidget::mousePressEvent(MouseEvent);
		return;
	}

	mRegionDragging = true;
	mRegionStart = GetCanvasPoint(MouseEvent->pos());
	mRegion = QRectF(mRegionStart, QSizeF());
	update();
}

void swRenderPreviewWidget::mouseMoveEvent(QMouseEvent* MouseEvent)
{
	if (!mRegionDragging)
	{
		QWidget::mouseMoveEvent(MouseEvent);
		return;
	}

	mRegion = QRectF(mRegionStart, GetCanvasPoint(MouseEvent->pos())).normalized();
	update();
}

void swRenderPreviewWidget::mouseReleaseEvent(QMouseEvent* MouseEvent)
{
	if (!mRegionDragging || MouseEvent->button() != Qt::LeftButton)
	{
		QWidget::mouseReleaseEvent(MouseEvent);
		return;
	}

	mRegionDragging = false;
	mRegion = QRectF(mRegionStart, GetCanvasPoint(MouseEvent->pos())).normalized();

	// a click keeps the whole image
	if (mRegion.width() < 0.01 || mRegion.height() < 0.01)
		mRegion = QRectF(0.0, 0.0, 1.0, 1.0);

	update();

	emit RegionChanged(mRegion);
}

void swRenderPreviewWidget::paintEvent(QPaintEvent* PaintEvent)
{
	Q_UNUSED(PaintEvent);

	QPainter Painter(this);

	// canvas on the backdrop
	Painter.fillRect(rect(), palette().color(QPalette::Mid));

	const QRect CanvasRect = GetCanvasRect();

	if (CanvasRect.isEmpty())
		return;

	// empty canvas is a blank sheet
	Painter.fillRect(CanvasRect, Qt::white);

	if (!mImage.isNull())
	{
		if (mScaledImage.size() != CanvasRect.size())
			mScaledImage = mImage.scaled(CanvasRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		Painter.drawImage(CanvasRect.topLeft(), mScaledImage);
	}
	else if (mPlaceholderSize > 0)
	{
		// paint the placeholder at its size
		const int IconSize = qMin(mPlaceholderSize, qMin(CanvasRect.width(), CanvasRect.height()) / 2);
		const qreal PixelRatio = devicePixelRatioF();
		QImage IconImage = swCreatePaintedIconImage(swPaintedIcon::RenderPlaceholder, qRound(IconSize * PixelRatio), palette().color(QPalette::Mid));
		IconImage.setDevicePixelRatio(PixelRatio);
		QRect IconRect(0, 0, IconSize, IconSize);
		IconRect.moveCenter(CanvasRect.center());
		Painter.drawImage(IconRect, IconImage);
	}

	if (mRegionMode)
	{
		// dim outside the region
		const QRectF RegionRect(CanvasRect.left() + mRegion.left() * CanvasRect.width(), CanvasRect.top() + mRegion.top() * CanvasRect.height(), mRegion.width() * CanvasRect.width(), mRegion.height() * CanvasRect.height());
		QPainterPath OutsidePath;
		OutsidePath.addRect(CanvasRect);
		OutsidePath.addRect(RegionRect);
		Painter.fillPath(OutsidePath, QColor(0, 0, 0, 96));

		QPen Pen(palette().color(QPalette::Highlight), 2);
		Pen.setCosmetic(true);
		Painter.setPen(Pen);
		Painter.setBrush(Qt::NoBrush);
		Painter.drawRect(RegionRect);
	}
}

// use to warn with the full output behind show details
static void swShowDetailedWarning(QWidget* Parent, const QString& Title, const QString& Text, const QString& Details)
{
	QMessageBox MessageBox(QMessageBox::Warning, Title, Text, QMessageBox::Ok, Parent);
	MessageBox.setDetailedText(Details);
	MessageBox.exec();
}

// use to find the bounds of the visible pixels
QRect swGetOpaqueRect(const QImage& Image)
{
	const QImage ArgbImage = Image.convertToFormat(QImage::Format_ARGB32);
	int Left = ArgbImage.width(), Top = ArgbImage.height(), Right = -1, Bottom = -1;

	for (int y = 0; y < ArgbImage.height(); y++)
	{
		const QRgb* Line = reinterpret_cast<const QRgb*>(ArgbImage.constScanLine(y));

		for (int x = 0; x < ArgbImage.width(); x++)
		{
			if (!qAlpha(Line[x]))
				continue;

			Left = std::min(Left, x);
			Right = std::max(Right, x);
			Top = std::min(Top, y);
			Bottom = std::max(Bottom, y);
		}
	}

	return Right < 0 ? QRect() : QRect(QPoint(Left, Top), QPoint(Right, Bottom));
}

swRenderDialog::swRenderDialog(QWidget* Parent, swRenderDialogMode RenderDialogMode)
	: QDialog(Parent), mDialogMode(RenderDialogMode), ui(new Ui::swRenderDialog)
{
	ui->setupUi(this);
	swSetupDialogHelp(this);

	mWidth = swGetProfileInteger(SW_PROFILE_RENDER_WIDTH);
	mHeight = swGetProfileInteger(SW_PROFILE_RENDER_HEIGHT);
	mQuality = swGetProfileRenderQuality();

	ui->OutputEdit->setText(swGetActiveProject()->GetImageFileName(false));

	// buttons in the render panel
	mRenderButton = new QToolButton(this);
	mRenderButton->setText(tr("Render"));
	mLogButton = new QToolButton(this);
	mLogButton->setText(tr("Log"));

	mRenderButton->setWhatsThis(tr("<p><b>Render</b></p><p>Start the render using the current settings. While rendering, this button becomes Cancel so you can stop the operation.</p>"));

	connect(mRenderButton, &QAbstractButton::clicked, this, &swRenderDialog::RenderButtonClicked);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &swRenderDialog::reject);
	connect(ui->OutputBrowseButton, &QPushButton::clicked, this, &swRenderDialog::OutputBrowseButtonClicked);

	ui->buttonBox->hide();

	if (mDialogMode == swRenderDialogMode::RenderPOVRay)
	{
		setWindowTitle(tr("POV-Ray Render"));
		setWhatsThis(tr("<p><b>POV-Ray Render</b></p><p>Render the current model step using POV-Ray. Choose the output image, its size and quality and the scene in the panels beside the preview, then select Render to start.</p><p>The preview is scaled to fit the window. Log shows the renderer output below the preview.</p>"));
		mRenderButton->setToolTip(tr("Render the current model step with POV-Ray."));
	}
	else
	{
		setWindowTitle(tr("Blender Render"));
		setWhatsThis(tr("<p><b>Blender Render</b></p><p>Render the current model step with Blender and the bundled LDraw add-ons. Choose the output image, its size and the scene, parts and editing settings in the panels beside the preview, then select Render to start, or Open in Blender to edit the model there.</p><p>Set the Blender executable in Preferences, General first. Log shows the Blender output below the preview.</p>"));
	}

	CreatePanels();

	// render icon in the empty preview
	ui->preview->SetPlaceholderSize(SW_PART_ICON_SIZES.back());
	ui->preview->SetCanvasSize(QSize(mWidth, mHeight));

	connect(&mUpdateTimer, &QTimer::timeout, this, &swRenderDialog::Update);

	mUpdateTimer.start(500);
}

// use to name the saved panel layout of the mode
QString swRenderDialog::GetPanelStateGroup() const
{
	return mDialogMode == swRenderDialogMode::RenderBlender ? QStringLiteral("BlenderRenderWindow") : QStringLiteral("RenderWindow");
}
// use to lay out the render window
void swRenderDialog::CreatePanels()
{
	const bool Blender = mDialogMode == swRenderDialogMode::RenderBlender;

	QMainWindow* PanelWindow = new QMainWindow(this);
	PanelWindow->setWindowFlags(Qt::Widget);
	PanelWindow->setDockOptions(QMainWindow::AnimatedDocks | QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

	mPanelWindow = PanelWindow;

	// preview fills the middle
	PanelWindow->setCentralWidget(ui->preview);
	ui->MainLayout->addWidget(PanelWindow, 1);
	ui->MainLayout->setContentsMargins(0, 0, 0, 0);
	layout()->setContentsMargins(0, 0, 0, 0);
	ui->outputLabel->hide();

	QList<QLabel*> Labels;

	// panels that end at their last row
	QList<QWidget*> CompactPanels;

	auto CreatePanel = [PanelWindow](const QString& Title, const QString& Name, const QString& WhatsThis)
	{
		QDockWidget* Panel = new QDockWidget(Title, PanelWindow);
		Panel->setObjectName(Name);
		Panel->setWhatsThis(WhatsThis);
		Panel->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

		QWidget* Widget = new QWidget(Panel);
		Panel->setWidget(Widget);
		new QVBoxLayout(Widget);

		// keep rows under the title
		Panel->layout()->setAlignment(Widget, Qt::AlignTop);

		// top and bottom panels span the window
		Panel->setProperty("swFloatingSize", QSize(SW_RENDER_PANEL_WIDTH, 0));
		PanelWindow->addDockWidget(Qt::RightDockWidgetArea, Panel);
		swSetupWidgetHelp(Panel);

		return Widget;
	};

	auto CreateRow = [&Labels](QWidget* Panel, const QString& Name, QWidget* Control, const QString& ToolTip, const QString& WhatsThis)
	{
		QWidget* Row = swCreatePanelRow(Panel);
		Panel->layout()->addWidget(Row);

		QLabel* Label = new QLabel(Name, Row);
		Label->setBuddy(Control);
		Label->setToolTip(ToolTip);
		Label->setWhatsThis(WhatsThis);
		Labels.append(Label);
		Row->layout()->addWidget(Label);

		Control->setParent(Row);
		Control->setToolTip(ToolTip);
		Control->setWhatsThis(WhatsThis);
		Control->setAccessibleName(Name);
		Control->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		swSetupPanelEdit(Control);
		static_cast<QHBoxLayout*>(Row->layout())->addWidget(Control, 1);

		return Row;
	};

	// check boxes sit right of the label column
	auto CreateCheckBox = [&Labels](QWidget* Panel, const QString& Name, bool Checked, const QString& ToolTip, const QString& WhatsThis)
	{
		QWidget* Row = swCreatePanelRow(Panel);
		Panel->layout()->addWidget(Row);

		QLabel* Label = new QLabel(Row);
		Labels.append(Label);
		Row->layout()->addWidget(Label);

		QCheckBox* CheckBox = new QCheckBox(Name, Row);
		CheckBox->setChecked(Checked);
		CheckBox->setToolTip(ToolTip);
		CheckBox->setWhatsThis(WhatsThis);
		swSetupPanelEdit(CheckBox);
		static_cast<QHBoxLayout*>(Row->layout())->addWidget(CheckBox, 1);

		return CheckBox;
	};

	auto CreateComboBox = [](const QStringList& Items, int Index)
	{
		QComboBox* ComboBox = new QComboBox();
		ComboBox->addItems(Items);
		ComboBox->setCurrentIndex(std::clamp(Index, 0, static_cast<int>(Items.size()) - 1));

		return ComboBox;
	};

	auto CreateSpinBox = [](int Value)
	{
		QSpinBox* SpinBox = new QSpinBox();
		SpinBox->setRange(1, SW_MAXIMUM_IMAGE_DIMENSION);
		SpinBox->setValue(Value);
		SpinBox->setSuffix(tr(" px"));
		swSetupPanelSpinBox(SpinBox);

		return SpinBox;
	};

	QWidget* PresetsPanel = nullptr;
	QToolButton* PresetSaveButton = nullptr;

	if (!Blender)
	{
		PresetsPanel = CreatePanel(tr("Presets"), "RenderPresets", tr("<p><b>Presets</b></p><p>Keep the image size, quality and scene settings under a name and bring them back with one choice. Save stores the current settings as a preset and Delete removes the chosen one.</p>"));
		CompactPanels.append(PresetsPanel);

		mPresetComboBox = new QComboBox();
		CreateRow(PresetsPanel, tr("Preset"), mPresetComboBox, tr("Choose a preset to apply its image size, quality and scene settings."), QString());

		// preset buttons line up with the preset list
		QWidget* PresetButtonRow = swCreatePanelRow(PresetsPanel);
		PresetsPanel->layout()->addWidget(PresetButtonRow);

		QLabel* PresetButtonLabel = new QLabel(PresetButtonRow);
		Labels.append(PresetButtonLabel);
		PresetButtonRow->layout()->addWidget(PresetButtonLabel);

		auto CreatePresetButton = [PresetButtonRow](const QString& Text, const QString& ToolTip)
		{
			QToolButton* Button = new QToolButton(PresetButtonRow);
			Button->setText(Text);
			Button->setToolTip(ToolTip);
			Button->setWhatsThis(ToolTip);
			Button->setToolButtonStyle(Qt::ToolButtonTextOnly);
			swSetupPanelButton(Button);
			Button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			Button->setMinimumWidth(0);
			static_cast<QHBoxLayout*>(PresetButtonRow->layout())->addWidget(Button, 1);

			return Button;
		};

		PresetSaveButton = CreatePresetButton(tr("Save..."), tr("Save the current image size, quality and scene settings as a preset."));
		mPresetDeleteButton = CreatePresetButton(tr("Delete..."), tr("Delete the chosen preset."));
	}

	QWidget* OutputPanel = CreatePanel(tr("Output"), "RenderOutput", tr("<p><b>Output</b></p><p>Choose where the rendered image is saved and its format, such as PNG, JPEG, or BMP.</p>"));
	CompactPanels.append(OutputPanel);

	QWidget* FileRow = CreateRow(OutputPanel, tr("File"), ui->OutputEdit, ui->OutputEdit->toolTip(), ui->OutputEdit->whatsThis());
	ui->OutputBrowseButton->setParent(FileRow);
	swSetupPanelButton(ui->OutputBrowseButton);
	FileRow->layout()->addWidget(ui->OutputBrowseButton);

	QWidget* ImagePanel = CreatePanel(tr("Image"), "RenderImage", tr("<p><b>Image</b></p><p>Set the size of the rendered image in pixels and the rendering quality. Larger images and higher quality take longer to render.</p>"));
	CompactPanels.append(ImagePanel);

	mWidthSpinBox = CreateSpinBox(mWidth);
	QSpinBox* WidthSpinBox = mWidthSpinBox;
	CreateRow(ImagePanel, tr("Width"), WidthSpinBox, tr("Set the rendered image width in pixels (1 to 32768)."), QString());

	mHeightSpinBox = CreateSpinBox(mHeight);
	QSpinBox* HeightSpinBox = mHeightSpinBox;
	CreateRow(ImagePanel, tr("Height"), HeightSpinBox, tr("Set the rendered image height in pixels (1 to 32768)."), QString());

	// keep changes for the next render
	connect(WidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int Value)
	{
		mWidth = Value;
		swSetProfileInteger(SW_PROFILE_RENDER_WIDTH, Value);
		ui->preview->SetCanvasSize(QSize(mWidth, mHeight));
	});

	connect(HeightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int Value)
	{
		mHeight = Value;
		swSetProfileInteger(SW_PROFILE_RENDER_HEIGHT, Value);
		ui->preview->SetCanvasSize(QSize(mWidth, mHeight));
	});

	QWidget* RenderPanel = nullptr;
	QWidget* StretchPanel = nullptr;

	if (Blender)
	{
		auto IsSettingOn = [](const QString& Key)
		{
			return swGetBlenderSetting(Key).compare(QLatin1String("True"), Qt::CaseInsensitive) == 0;
		};

		auto GetSettingText = [](bool On)
		{
			return On ? QStringLiteral("True") : QStringLiteral("False");
		};

		auto CreateSettingCheckBox = [this, &CreateCheckBox, IsSettingOn, GetSettingText](QWidget* Panel, const QString& Name, const QString& Key, const QString& ToolTip, const QString& WhatsThis)
		{
			QCheckBox* CheckBox = CreateCheckBox(Panel, Name, IsSettingOn(Key), ToolTip, WhatsThis);

			connect(CheckBox, &QCheckBox::toggled, this, [Key, GetSettingText](bool Checked)
			{
				swSetBlenderSetting(Key, GetSettingText(Checked));
			});

			return CheckBox;
		};

		auto CreateSettingComboBox = [this, &CreateRow, &CreateComboBox](QWidget* Panel, const QString& Name, const QStringList& Items, const QStringList& Values, const QString& Key, const QString& ToolTip, const QString& WhatsThis)
		{
			QComboBox* ComboBox = CreateComboBox(Items, Values.indexOf(swGetBlenderSetting(Key)));
			CreateRow(Panel, Name, ComboBox, ToolTip, WhatsThis);

			connect(ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [Key, Values](int Index)
			{
				swSetBlenderSetting(Key, Values[Index]);
			});
		};

		auto CreateDoubleSpinBox = [](double Maximum, double Value, const QString& Suffix)
		{
			QDoubleSpinBox* SpinBox = new QDoubleSpinBox();
			SpinBox->setRange(0.0, Maximum);
			SpinBox->setSingleStep(0.1);
			SpinBox->setValue(Value);
			SpinBox->setSuffix(Suffix);
			swSetupPanelSpinBox(SpinBox);

			return SpinBox;
		};

		// zero turns the setting off
		auto CreateSettingSpinBox = [this, &CreateRow, &CreateDoubleSpinBox, IsSettingOn, GetSettingText](QWidget* Panel, const QString& Name, const QString& Key, const QString& ValueKey, double Maximum, double Factor, const QString& Suffix, const QString& ToolTip, const QString& WhatsThis)
		{
			QDoubleSpinBox* SpinBox = CreateDoubleSpinBox(Maximum, IsSettingOn(Key) ? swGetBlenderSetting(ValueKey).toDouble() * Factor : 0.0, Suffix);
			SpinBox->setSpecialValueText(tr("Off"));
			CreateRow(Panel, Name, SpinBox, ToolTip, WhatsThis);

			connect(SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [Key, ValueKey, Factor, GetSettingText](double Value)
			{
				swSetBlenderSetting(Key, GetSettingText(Value > 0.0));

				if (Value > 0.0)
					swSetBlenderSetting(ValueKey, QString::number(Value / Factor));
			});
		};

		CreateSettingCheckBox(OutputPanel, tr("Overwrite Image"), "overwriteimage", tr("Replace an existing image file of the same name."), QString());
		QCheckBox* TransparentCheckBox = CreateSettingCheckBox(OutputPanel, tr("Transparent Background"), "transparentbackground", tr("Leave the background as transparent pixels for compositing."), tr("<p><b>Transparent Background</b></p><p>Renders the background as transparent pixels for compositing. The ground plane added by Add Environment stays visible.</p>"));

		QCheckBox* CropCheckBox = CreateCheckBox(OutputPanel, tr("Crop Image"), swGetProfileInteger(SW_PROFILE_BLENDER_CROP_IMAGE), tr("Crop the image to the visible model, needs Transparent Background."), tr("<p><b>Crop Image</b></p><p>Cuts the transparent border off the rendered image so it ends at the visible model. Needs Transparent Background.</p>"));
		CropCheckBox->setEnabled(TransparentCheckBox->isChecked());

		connect(TransparentCheckBox, &QCheckBox::toggled, CropCheckBox, &QCheckBox::setEnabled);
		connect(CropCheckBox, &QCheckBox::toggled, this, [](bool Checked)
		{
			swSetProfileInteger(SW_PROFILE_BLENDER_CROP_IMAGE, Checked);
		});

		QSpinBox* ScaleSpinBox = new QSpinBox();
		ScaleSpinBox->setRange(1, 100);
		ScaleSpinBox->setValue(std::clamp(qRound(swGetBlenderSetting("renderpercentage").toDouble()), 1, 100));
		ScaleSpinBox->setSuffix(tr(" %"));
		swSetupPanelSpinBox(ScaleSpinBox);
		CreateRow(ImagePanel, tr("Scale"), ScaleSpinBox, tr("Render at a percentage of the image size for quick test renders."), QString());

		connect(ScaleSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [](int Value)
		{
			swSetBlenderSetting("renderpercentage", QString::number(Value));
		});

		QWidget* ScenePanel = CreatePanel(tr("Scene"), "RenderScene", tr("<p><b>Scene</b></p><p>Set the look, colors, camera and lights of the scene Blender renders.</p>"));

		CreateSettingComboBox(ScenePanel, tr("Look"), { tr("Photorealistic"), tr("LEGO® Instructions") }, { "normal", "instructions" }, "uselook", tr("Choose a photorealistic render or the flat look of LEGO® building instructions."), QString());
		CreateSettingComboBox(ScenePanel, tr("Colors"), { tr("Realistic"), tr("Original LDraw"), tr("Alternate LDraw") }, { "lgeo", "ldraw", "alt" }, "usecolourscheme", tr("Choose the color scheme of the parts."), tr("<p><b>Colors</b></p><p>Realistic uses the colors of the real parts. Original LDraw uses the colors of the parts library. Alternate LDraw uses its alternate colors with lighter edges.</p>"));
		CreateSettingCheckBox(ScenePanel, tr("Add Environment"), "addenvironment", tr("Add a ground plane and a lighting environment, photorealistic look only."), QString());
		QCheckBox* PositionCheckBox = CreateSettingCheckBox(ScenePanel, tr("Position Camera"), "positioncamera", tr("Place the camera so the whole model is in view."), QString());

		QDoubleSpinBox* BorderSpinBox = CreateDoubleSpinBox(50.0, swGetBlenderSetting("cameraborderpercentage").toDouble(), tr(" %"));
		CreateRow(ScenePanel, tr("Camera Border"), BorderSpinBox, tr("Leave this border around the model when the camera is placed."), QString());
		BorderSpinBox->setEnabled(PositionCheckBox->isChecked());

		connect(PositionCheckBox, &QCheckBox::toggled, BorderSpinBox, &QWidget::setEnabled);
		connect(BorderSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [](double Value)
		{
			swSetBlenderSetting("cameraborderpercentage", QString::number(Value));
		});

		CreateSettingCheckBox(ScenePanel, tr("Import Cameras"), "importcameras", tr("Use the cameras of the model."), QString());
		CreateSettingCheckBox(ScenePanel, tr("Import Lights"), "importlights", tr("Use the lights of the model."), QString());

		QWidget* PartsPanel = CreatePanel(tr("Parts"), "RenderParts", tr("<p><b>Parts</b></p><p>Set how much detail Blender gives the parts. More detail looks better and takes longer to import and render.</p>"));

		CreateSettingComboBox(PartsPanel, tr("Primitives"), { tr("Standard"), tr("High"), tr("Low") }, { "Standard", "High", "Low" }, "resolution", tr("Choose the resolution of the part primitives, how round curved surfaces are."), QString());

		const QStringList LogoValues = { QString(), "4", "3", "5" };
		QComboBox* LogoComboBox = CreateComboBox({ tr("Off"), tr("Rounded"), tr("Flattened"), tr("Subtle Rounded") }, IsSettingOn("uselogostuds") ? std::max(static_cast<int>(LogoValues.indexOf(swGetBlenderSetting("logostudversion"))), 1) : 0);
		CreateRow(PartsPanel, tr("Logo Studs"), LogoComboBox, tr("Show the LEGO® logo on the studs, at the cost of extra geometry and import time."), QString());

		connect(LogoComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [LogoValues, GetSettingText](int Index)
		{
			swSetBlenderSetting("uselogostuds", GetSettingText(Index > 0));

			if (Index > 0)
				swSetBlenderSetting("logostudversion", LogoValues[Index]);
		});

		CreateSettingSpinBox(PartsPanel, tr("Bevel Edges"), "beveledges", "bevelwidth", 5.0, 1.0, QString(), tr("Round off sharp edges by this width, Off keeps them sharp."), QString());
		CreateSettingSpinBox(PartsPanel, tr("Part Gaps"), "gaps", "realgapwidth", 2.0, 1000.0, tr(" mm"), tr("Leave this gap between the parts, Off places them touching."), QString());
		CreateSettingComboBox(PartsPanel, tr("Flex Parts"), { tr("LSynth"), tr("LDCad") }, { "True", "False" }, "uselsynthparts", tr("Choose the source that shapes flexible parts such as hoses and string."), QString());
		CreateSettingCheckBox(PartsPanel, tr("Curved Walls"), "curvedwalls", tr("Make surfaces slightly concave for livelier reflections."), QString());
		CreateSettingCheckBox(PartsPanel, tr("Smooth Shading"), "smoothshading", tr("Smooth curved faces and keep sharp edges."), QString());

		QWidget* EditingPanel = CreatePanel(tr("Editing"), "RenderEditing", tr("<p><b>Editing</b></p><p>Set how the model is built up in Blender, which matters most when you edit it after Open in Blender.</p>"));

		CreateSettingCheckBox(EditingPanel, tr("Link Like Parts"), "linkparts", tr("Let parts of the same shape and color share one mesh."), QString());
		CreateSettingCheckBox(EditingPanel, tr("Instance Studs"), "instancestuds", tr("Make every stud its own object, slow for models with many studs."), QString());
		CreateSettingCheckBox(EditingPanel, tr("Flatten Hierarchy"), "flattenhierarchy", tr("Place all parts directly under the root instead of a tree of submodels."), QString());
		CreateSettingCheckBox(EditingPanel, tr("Parent Minifigures"), "minifighierarchy", tr("Parent the parts of each minifigure to each other."), QString());

		// scene, parts and editing share the space below
		for (QWidget* Panel : { ScenePanel, PartsPanel, EditingPanel })
			static_cast<QVBoxLayout*>(Panel->layout())->addStretch(1);

		PanelWindow->tabifyDockWidget(qobject_cast<QDockWidget*>(ScenePanel->parentWidget()), qobject_cast<QDockWidget*>(PartsPanel->parentWidget()));
		PanelWindow->tabifyDockWidget(qobject_cast<QDockWidget*>(PartsPanel->parentWidget()), qobject_cast<QDockWidget*>(EditingPanel->parentWidget()));
		ScenePanel->parentWidget()->raise();

		RenderPanel = CreatePanel(tr("Render"), "RenderRender", tr("<p><b>Render</b></p><p>Render the image or open the model in Blender, then follow the progress here. Log shows the Blender output below the preview.</p>"));
	}
	else
	{
		mQualityComboBox = CreateComboBox({ tr("Low"), tr("Medium"), tr("High") }, static_cast<int>(mQuality));
		QComboBox* QualityComboBox = mQualityComboBox;
		CreateRow(ImagePanel, tr("Quality"), QualityComboBox, tr("Choose the rendering quality; higher quality can take longer."), tr("<p><b>Quality</b></p><p>Low uses POV-Ray's default quality without additional antialiasing options. Medium uses quality level 5 with antialiasing and fast radiosity. High uses quality level 11 with additional antialiasing settings and full radiosity.</p>"));

		mAreaComboBox = CreateComboBox({ tr("Full Image"), tr("Region") }, swGetProfileInteger(SW_PROFILE_RENDER_AREA));
		CreateRow(ImagePanel, tr("Area"), mAreaComboBox, tr("Choose whether the whole image is rendered or only a region drawn on the preview."), tr("<p><b>Area</b></p><p>Full Image renders the whole image. Region renders only the part outlined on the preview, drag on the preview to draw it. A region rendered at the size of the last image is placed into that image so a detail can be refined without rendering everything again.</p>"));

		QWidget* BatchPanel = CreatePanel(tr("Batch"), "RenderBatch", tr("<p><b>Batch</b></p><p>Render several images in one go, one for each building step or each camera of the model. Each image is saved next to the output file with the step number or camera name added to its name.</p>"));
		CompactPanels.append(BatchPanel);

		mStepsComboBox = CreateComboBox({ tr("Model"), tr("Current Step"), tr("All Steps") }, swGetProfileInteger(SW_PROFILE_RENDER_STEPS));
		CreateRow(BatchPanel, tr("Steps"), mStepsComboBox, tr("Choose whether the finished model, the current step or every step is rendered."), tr("<p><b>Steps</b></p><p>Model renders the finished model. Current Step renders the model as built up to the step shown in the main window. All Steps renders one image for every step, numbered after the step.</p>"));

		mCamerasComboBox = new QComboBox();
		CreateRow(BatchPanel, tr("Cameras"), mCamerasComboBox, tr("Choose the view or a camera of the model to render from, or every camera."), tr("<p><b>Cameras</b></p><p>View renders from the view of the main window. A camera renders from that camera of the model. All Cameras renders one image for every camera, named after the camera.</p>"));
		UpdateCameras();

		QWidget* ScenePanel = CreatePanel(tr("Scene"), "RenderScene", tr("<p><b>Scene</b></p><p>Set the lights, floor and surroundings POV-Ray renders the model in.</p>"));
		StretchPanel = ScenePanel;

		QComboBox* PartsComboBox = CreateComboBox({ tr("LDraw"), tr("LGEO") }, swGetProfileInteger(SW_PROFILE_RENDER_PARTS));
		CreateRow(ScenePanel, tr("Parts"), PartsComboBox, tr("Choose whether the model is rendered with LDraw parts or the modeled LGEO parts."), tr("<p><b>Parts</b></p><p>LDraw renders the parts of the parts library. LGEO replaces the parts it has with the smooth modeled parts of the LGEO library by Lutz Uhlmann, StudWorx offers to download it the first time. Models that ask for LGEO in their file always use it.</p>"));

		mLightRigComboBox = CreateComboBox({ tr("Standard"), tr("Studio"), tr("Matte") }, swGetProfileInteger(SW_PROFILE_RENDER_LIGHT_RIG));
		QComboBox* LightRigComboBox = mLightRigComboBox;
		CreateRow(ScenePanel, tr("Lights"), LightRigComboBox, tr("Choose the lights placed around models that have no lights of their own."), tr("<p><b>Lights</b></p><p>Standard uses four point lights with crisp shadows. Studio uses a large soft key light, a fill light and a rim light behind the model. Matte uses a wide overhead light and a shadowless light from the camera for even, soft lighting. Lights placed in the model replace these.</p>"));

		mFloorStyleComboBox = CreateComboBox({ tr("Plain"), tr("Shadow Catcher"), tr("Reflective"), tr("Cyclorama") }, swGetProfileInteger(SW_PROFILE_RENDER_FLOOR_STYLE));
		QComboBox* FloorStyleComboBox = mFloorStyleComboBox;
		CreateRow(ScenePanel, tr("Floor"), FloorStyleComboBox, tr("Choose how the floor under the model looks."), tr("<p><b>Floor</b></p><p>Plain uses the floor color of the model. Shadow Catcher takes the background color so only the shadows show. Reflective adds a glossy mirror finish. Cyclorama fades the floor into the background with distance for a seamless backdrop.</p>"));

		mEnvironmentEdit = new QLineEdit();
		QLineEdit* EnvironmentEdit = mEnvironmentEdit;
		EnvironmentEdit->setText(QDir::toNativeSeparators(swGetProfileString(SW_PROFILE_RENDER_ENVIRONMENT_IMAGE)));
		EnvironmentEdit->setPlaceholderText(tr("Background Color"));
		EnvironmentEdit->setClearButtonEnabled(true);
		QWidget* EnvironmentRow = CreateRow(ScenePanel, tr("Environment"), EnvironmentEdit, tr("Choose a panoramic image that surrounds and lights the scene, or leave empty to use the background color."), tr("<p><b>Environment</b></p><p>Choose an equirectangular panorama, ideally an HDR or EXR image, that wraps around the scene as its sky and lights it through radiosity at Medium and High quality. Leave empty to use the background color.</p>"));

		QToolButton* EnvironmentButton = new QToolButton(EnvironmentRow);
		EnvironmentButton->setText(tr("..."));
		EnvironmentButton->setToolTip(tr("Choose an environment image."));
		EnvironmentButton->setWhatsThis(EnvironmentButton->toolTip());
		EnvironmentButton->setAccessibleName(tr("Browse"));
		swSetupPanelButton(EnvironmentButton);
		EnvironmentRow->layout()->addWidget(EnvironmentButton);

		mBackgroundComboBox = CreateComboBox({ tr("Visible"), tr("Transparent") }, swGetProfileInteger(SW_PROFILE_RENDER_TRANSPARENT) ? 1 : 0);
		QComboBox* BackgroundComboBox = mBackgroundComboBox;
		CreateRow(ScenePanel, tr("Background"), BackgroundComboBox, tr("Choose whether the background and floor are rendered or left as transparent pixels for compositing."), QString());

		RenderPanel = CreatePanel(tr("Render"), "RenderRender", tr("<p><b>Render</b></p><p>Start the POV-Ray render, then follow its progress here. Log shows the POV-Ray output below the preview.</p>"));

		connect(QualityComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int Index)
		{
			mQuality = static_cast<swPOVRayRenderQuality>(Index);
			swSetProfileRenderQuality(mQuality);
		});

		connect(PartsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [](int Index)
		{
			swSetProfileInteger(SW_PROFILE_RENDER_PARTS, Index);
		});

		connect(LightRigComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [](int Index)
		{
			swSetProfileInteger(SW_PROFILE_RENDER_LIGHT_RIG, Index);
		});

		connect(FloorStyleComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [](int Index)
		{
			swSetProfileInteger(SW_PROFILE_RENDER_FLOOR_STYLE, Index);
		});

		connect(BackgroundComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [](int Index)
		{
			swSetProfileInteger(SW_PROFILE_RENDER_TRANSPARENT, Index);
		});

		connect(EnvironmentEdit, &QLineEdit::textChanged, this, [](const QString& Text)
		{
			swSetProfileString(SW_PROFILE_RENDER_ENVIRONMENT_IMAGE, QDir::fromNativeSeparators(Text.trimmed()));
		});

		// show custom on a mismatch
		for (QSpinBox* SpinBox : { WidthSpinBox, HeightSpinBox })
			connect(SpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &swRenderDialog::UpdatePresets);

		for (QComboBox* ComboBox : { QualityComboBox, LightRigComboBox, FloorStyleComboBox, BackgroundComboBox })
			connect(ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &swRenderDialog::UpdatePresets);

		connect(EnvironmentEdit, &QLineEdit::textChanged, this, &swRenderDialog::UpdatePresets);

		connect(mPresetComboBox, QOverload<int>::of(&QComboBox::activated), this, [this](int Index)
		{
			ApplyPreset(mPresetComboBox->itemData(Index).toString());
		});

		connect(PresetSaveButton, &QToolButton::clicked, this, &swRenderDialog::SavePresetClicked);
		connect(mPresetDeleteButton, &QToolButton::clicked, this, &swRenderDialog::DeletePresetClicked);

		UpdatePresets();

		// keep the drawn region
		const QStringList RegionValues = swGetProfileString(SW_PROFILE_RENDER_REGION).split(' ', Qt::SkipEmptyParts);

		if (RegionValues.size() == 4)
			ui->preview->SetRegion(QRectF(RegionValues[0].toDouble(), RegionValues[1].toDouble(), RegionValues[2].toDouble(), RegionValues[3].toDouble()) & QRectF(0.0, 0.0, 1.0, 1.0));

		ui->preview->SetRegionMode(mAreaComboBox->currentIndex() == 1);

		connect(mAreaComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int Index)
		{
			swSetProfileInteger(SW_PROFILE_RENDER_AREA, Index);
			ui->preview->SetRegionMode(Index == 1);
		});

		connect(ui->preview, &swRenderPreviewWidget::RegionChanged, this, [](const QRectF& Region)
		{
			swSetProfileString(SW_PROFILE_RENDER_REGION, QString("%1 %2 %3 %4").arg(Region.x()).arg(Region.y()).arg(Region.width()).arg(Region.height()));
		});

		connect(mStepsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [](int Index)
		{
			swSetProfileInteger(SW_PROFILE_RENDER_STEPS, Index);
		});

		connect(mCamerasComboBox, QOverload<int>::of(&QComboBox::activated), this, [this](int Index)
		{
			swSetProfileString(SW_PROFILE_RENDER_CAMERAS, mCamerasComboBox->itemData(Index).toString());
		});

		connect(EnvironmentButton, &QToolButton::clicked, this, [this, EnvironmentEdit]()
		{
			const QString FileName = QFileDialog::getOpenFileName(this, tr("Select Environment Image"), EnvironmentEdit->text(), tr("Environment Images (*.hdr *.exr *.png *.jpg *.jpeg *.tif *.tiff);;All Files (*)"));

			if (!FileName.isEmpty())
				EnvironmentEdit->setText(QDir::toNativeSeparators(FileName));
		});
	}

	CompactPanels.append(RenderPanel);

	QWidget* ProgressRow = swCreatePanelRow(RenderPanel);
	RenderPanel->layout()->addWidget(ProgressRow);
	ui->RenderProgress->setParent(ProgressRow);
	ui->RenderProgress->setFixedHeight(swGetPanelButtonSize().height());
	ProgressRow->layout()->addWidget(ui->RenderProgress);

	// render button spans the panel
	QWidget* ButtonRow = swCreatePanelRow(RenderPanel);
	RenderPanel->layout()->addWidget(ButtonRow);

	QList<QToolButton*> RenderButtons;

	if (Blender)
	{
		QToolButton* OpenButton = new QToolButton(ButtonRow);
		OpenButton->setText(tr("Open in Blender"));
		OpenButton->setWhatsThis(tr("<p><b>Open in Blender</b></p><p>Import the current model step into Blender with the Scene, Parts and Editing settings and open it there for editing. No image is saved.</p>"));
		RenderButtons.append(OpenButton);

		connect(OpenButton, &QToolButton::clicked, this, [this]()
		{
			if (!mProcess)
				RenderBlender(true);
		});
	}

	RenderButtons.append(static_cast<QToolButton*>(mRenderButton));
	RenderButtons.append(static_cast<QToolButton*>(mLogButton));

	for (QToolButton* Button : RenderButtons)
	{
		Button->setParent(ButtonRow);
		Button->setToolButtonStyle(Qt::ToolButtonTextOnly);
		Button->setAccessibleName(Button->text());
		swSetupPanelButton(Button);
		ButtonRow->layout()->addWidget(Button);
	}

	mRenderButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	static_cast<QHBoxLayout*>(ButtonRow->layout())->setStretchFactor(mRenderButton, 1);

	if (Blender)
	{
		// the add-ons are bundled, so a tested blender is all that is needed
		const bool BlenderConfigured = !swGetProfileString(SW_PROFILE_BLENDER_VERSION).isEmpty() && QFileInfo(swGetProfileString(SW_PROFILE_BLENDER_PATH)).isExecutable();

		RenderButtons[0]->setToolTip(BlenderConfigured ? tr("Import the current model step and open it in Blender.") : tr("Set the Blender executable in Preferences, General to use Blender."));
		mRenderButton->setToolTip(BlenderConfigured ? tr("Render the current model step with Blender.") : RenderButtons[0]->toolTip());

		for (QToolButton* Button : { RenderButtons[0], static_cast<QToolButton*>(mRenderButton) })
			Button->setEnabled(BlenderConfigured);
	}
	else
		static_cast<QVBoxLayout*>(StretchPanel->layout())->addStretch(1);

	// labels share one width
	int LabelWidth = 0;

	for (QLabel* Label : Labels)
		LabelWidth = std::max(LabelWidth, Label->sizeHint().width());

	for (QLabel* Label : Labels)
		Label->setFixedWidth(LabelWidth);

	// log below the preview
	mLogPanel = new QDockWidget(tr("Log"), PanelWindow);
	mLogPanel->setObjectName("RenderLog");
	mLogPanel->setProperty("swFloatingSize", QSize(2 * SW_RENDER_PANEL_WIDTH, SW_RENDER_LOG_HEIGHT));
	mLogPanel->setWhatsThis(tr("<p><b>Log</b></p><p>The output of the renderer, updated while it renders. Read it to investigate warnings or a failed render, select text and copy it with the standard keyboard shortcut when reporting a problem.</p>"));

	mLogEdit = new QPlainTextEdit(mLogPanel);
	mLogEdit->setReadOnly(true);
	mLogEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
	mLogEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	mLogEdit->setPlaceholderText(tr("The renderer output shows here once a render starts."));
	mLogEdit->setAccessibleName(tr("Renderer Output"));
	mLogEdit->setToolTip(tr("Read and copy renderer output and diagnostic messages."));
	mLogEdit->setMinimumHeight(6 * QFontMetrics(mLogEdit->font()).height());
	mLogPanel->setWidget(mLogEdit);

	swSetupWidgetHelp(mLogPanel);

	// log and render panels span the edges
	QDockWidget* RenderDock = qobject_cast<QDockWidget*>(RenderPanel->parentWidget());
	PanelWindow->addDockWidget(Qt::BottomDockWidgetArea, RenderDock);
	PanelWindow->addDockWidget(Qt::BottomDockWidgetArea, mLogPanel);
	PanelWindow->splitDockWidget(mLogPanel, RenderDock, Qt::Vertical);

	for (QDockWidget* EdgePanel : { RenderDock, mLogPanel })
		EdgePanel->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);

	mLogPanel->hide();

	// log button toggles the log panel
	mLogButton->setCheckable(true);
	mLogButton->setToolTip(tr("Show or hide the Log panel with the renderer output."));
	mLogButton->setWhatsThis(tr("<p><b>Log</b></p><p>Show or hide the Log panel below the preview with the renderer output and diagnostic messages.</p>"));

	connect(mLogButton, &QAbstractButton::toggled, mLogPanel, &QDockWidget::setVisible);
	connect(mLogPanel, &QDockWidget::visibilityChanged, this, [this](bool Visible)
	{
		if (mLogPanel->isHidden())
			mLogButton->setChecked(false);
		else if (Visible)
		{
			mLogButton->setChecked(true);
			UpdateLogPanel();
		}
	});

	for (QWidget* Panel : CompactPanels)
		Panel->setFixedHeight(Panel->sizeHint().height());

	// open moved panels at a usable size
	QList<QDockWidget*> CompactDocks;

	for (QWidget* Panel : CompactPanels)
		CompactDocks.append(qobject_cast<QDockWidget*>(Panel->parentWidget()));

	auto FitPanels = [PanelWindow, CompactDocks]()
	{
		// compact panels end at their last row
		for (QDockWidget* CompactPanel : CompactDocks)
			CompactPanel->setMaximumHeight(CompactPanel->isFloating() ? QWIDGETSIZE_MAX : CompactPanel->sizeHint().height());

		// restore squeezed panel sizes
		QList<QDockWidget*> SidePanels, EdgePanels;

		for (QDockWidget* AreaPanel : PanelWindow->findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly))
		{
			if (AreaPanel->isFloating() || AreaPanel->isHidden())
				continue;

			const Qt::DockWidgetArea Area = PanelWindow->dockWidgetArea(AreaPanel);

			if ((Area == Qt::LeftDockWidgetArea || Area == Qt::RightDockWidgetArea) && AreaPanel->width() < SW_RENDER_PANEL_WIDTH)
				SidePanels.append(AreaPanel);
			else if ((Area == Qt::TopDockWidgetArea || Area == Qt::BottomDockWidgetArea) && AreaPanel->height() < SW_RENDER_LOG_HEIGHT && !CompactDocks.contains(AreaPanel))
				EdgePanels.append(AreaPanel);
		}

		if (!SidePanels.isEmpty())
			PanelWindow->resizeDocks(SidePanels, QVector<int>(SidePanels.size(), SW_RENDER_PANEL_WIDTH).toList(), Qt::Horizontal);

		if (!EdgePanels.isEmpty())
			PanelWindow->resizeDocks(EdgePanels, QVector<int>(EdgePanels.size(), SW_RENDER_LOG_HEIGHT).toList(), Qt::Vertical);
	};

	for (QDockWidget* Panel : PanelWindow->findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly))
	{
		// size panels after the dock fitting
		auto FitLater = [Panel, FitPanels]()
		{
			QTimer::singleShot(0, Panel, [Panel, FitPanels]()
			{
				QTimer::singleShot(0, Panel, FitPanels);
			});
		};

		connect(Panel, &QDockWidget::dockLocationChanged, Panel, FitLater);
		connect(Panel, &QDockWidget::visibilityChanged, Panel, FitLater);
		connect(Panel, &QDockWidget::topLevelChanged, Panel, FitLater);
	}

	// fit a readable output path
	PanelWindow->resizeDocks({ qobject_cast<QDockWidget*>(OutputPanel->parentWidget()) }, { SW_RENDER_PANEL_WIDTH }, Qt::Horizontal);

	// restore the panel layout
	QSettings Settings;
	Settings.beginGroup(GetPanelStateGroup());
	PanelWindow->restoreState(Settings.value("State").toByteArray(), SW_RENDER_WINDOW_STATE_VERSION);
	Settings.endGroup();

	// keep untoggled panels visible
	for (QDockWidget* Panel : PanelWindow->findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly))
		if (Panel != mLogPanel)
			Panel->show();

	mLogButton->setChecked(!mLogPanel->isHidden());

	// restore squeezed saved layouts
	QTimer::singleShot(0, PanelWindow, FitPanels);
}

// use to list preset settings
QStringList swRenderDialog::GetPresetValues() const
{
	return { QString::number(mWidthSpinBox->value()), QString::number(mHeightSpinBox->value()), QString::number(mQualityComboBox->currentIndex()), QString::number(mLightRigComboBox->currentIndex()), QString::number(mFloorStyleComboBox->currentIndex()), QString::number(mBackgroundComboBox->currentIndex()), QDir::fromNativeSeparators(mEnvironmentEdit->text().trimmed()) };
}

// use to apply a stored preset
void swRenderDialog::ApplyPreset(const QString& Preset)
{
	const QStringList Values = Preset.split('|');

	if (Values.size() < 8)
		return;

	mPresetName = Values[0];

	mWidthSpinBox->setValue(Values[1].toInt());
	mHeightSpinBox->setValue(Values[2].toInt());
	mQualityComboBox->setCurrentIndex(std::clamp(Values[3].toInt(), 0, mQualityComboBox->count() - 1));
	mLightRigComboBox->setCurrentIndex(std::clamp(Values[4].toInt(), 0, mLightRigComboBox->count() - 1));
	mFloorStyleComboBox->setCurrentIndex(std::clamp(Values[5].toInt(), 0, mFloorStyleComboBox->count() - 1));
	mBackgroundComboBox->setCurrentIndex(std::clamp(Values[6].toInt(), 0, mBackgroundComboBox->count() - 1));
	mEnvironmentEdit->setText(QDir::toNativeSeparators(Values.mid(7).join('|')));

	UpdatePresets();
}

// use to fill the preset list
void swRenderDialog::UpdatePresets()
{
	if (!mPresetComboBox)
		return;

	const QStringList Presets = swGetProfileStringList(SW_PROFILE_RENDER_PRESETS);
	const QStringList Values = GetPresetValues();
	int CurrentIndex = -1;

	QSignalBlocker Blocker(mPresetComboBox);
	mPresetComboBox->clear();

	for (const QString& Preset : Presets)
	{
		const QStringList PresetValues = Preset.split('|');

		if (PresetValues.size() < 8)
			continue;

		// show the last chosen duplicate
		if ((CurrentIndex == -1 || PresetValues[0] == mPresetName) && PresetValues.mid(1, 6) == Values.mid(0, 6) && PresetValues.mid(7).join('|') == Values[6])
			CurrentIndex = mPresetComboBox->count();

		mPresetComboBox->addItem(PresetValues[0], Preset);
	}

	// unmatched settings show as custom
	if (CurrentIndex == -1)
	{
		mPresetComboBox->insertItem(0, tr("Custom"));
		CurrentIndex = 0;
	}

	mPresetComboBox->setCurrentIndex(CurrentIndex);
	mPresetDeleteButton->setEnabled(!mPresetComboBox->itemData(CurrentIndex).toString().isEmpty());
}

// use to save a named preset
void swRenderDialog::SavePresetClicked()
{
	const QString CurrentName = mPresetComboBox->currentData().toString().isEmpty() ? QString() : mPresetComboBox->currentText();
	bool Ok = false;
	const QString Name = QInputDialog::getText(this, tr("Save Preset"), tr("Preset Name"), QLineEdit::Normal, CurrentName, &Ok).trimmed().remove('|');

	if (!Ok)
		return;

	if (Name.isEmpty())
	{
		QMessageBox::information(this, tr("Save Preset"), tr("Preset name cannot be empty."));
		return;
	}

	QStringList Presets = swGetProfileStringList(SW_PROFILE_RENDER_PRESETS);
	const QString Preset = QStringList({ Name }).join('|') + '|' + GetPresetValues().join('|');
	int PresetIndex = -1;

	for (int Index = 0; Index < Presets.size(); Index++)
		if (Presets[Index].section('|', 0, 0) == Name)
			PresetIndex = Index;

	if (PresetIndex != -1)
	{
		if (QMessageBox::question(this, tr("Save Preset"), tr("A preset named '%1' already exists. Do you want to overwrite it?").arg(Name), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		Presets[PresetIndex] = Preset;
	}
	else
		Presets.append(Preset);

	swSetProfileStringList(SW_PROFILE_RENDER_PRESETS, Presets);
	mPresetName = Name;
	UpdatePresets();
}

// use to remove the chosen preset
void swRenderDialog::DeletePresetClicked()
{
	const QString Preset = mPresetComboBox->currentData().toString();

	if (Preset.isEmpty())
		return;

	if (QMessageBox::question(this, tr("Delete Preset"), tr("Are you sure you want to delete the preset '%1'?").arg(mPresetComboBox->currentText()), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return;

	QStringList Presets = swGetProfileStringList(SW_PROFILE_RENDER_PRESETS);
	Presets.removeAll(Preset);
	swSetProfileStringList(SW_PROFILE_RENDER_PRESETS, Presets);
	UpdatePresets();
}

// use to list views and cameras
void swRenderDialog::UpdateCameras()
{
	const QString CameraName = swGetProfileString(SW_PROFILE_RENDER_CAMERAS);
	swModel* Model = swGetActiveModel();

	mCamerasComboBox->clear();
	mCamerasComboBox->addItem(tr("View"), QString());

	if (Model && !Model->GetCameras().empty())
	{
		for (const std::unique_ptr<swCamera>& Camera : Model->GetCameras())
			mCamerasComboBox->addItem(Camera->GetName(), Camera->GetName());

		mCamerasComboBox->addItem(tr("All Cameras"), QString("*"));
	}

	mCamerasComboBox->setCurrentIndex(std::max(mCamerasComboBox->findData(CameraName), 0));
}

// use to get the region in pixels
QRect swRenderDialog::GetRegionRect() const
{
	if (!mAreaComboBox || mAreaComboBox->currentIndex() != 1)
		return QRect();

	const QRectF Region = ui->preview->GetRegion();
	const int Left = std::clamp(qRound(Region.left() * mWidth), 0, mWidth - 2);
	const int Top = std::clamp(qRound(Region.top() * mHeight), 0, mHeight - 2);
	const int Right = std::clamp(qRound(Region.right() * mWidth), Left + 2, mWidth);
	const int Bottom = std::clamp(qRound(Region.bottom() * mHeight), Top + 2, mHeight);
	const QRect RegionRect(QPoint(Left, Top), QPoint(Right - 1, Bottom - 1));

	// a full region is a full render
	return RegionRect == QRect(0, 0, mWidth, mHeight) ? QRect() : RegionRect;
}

// use to run the next batch image
void swRenderDialog::StartNextJob()
{
	if (mJobIndex >= mJobs.size())
	{
		mJobs.clear();
		mJobIndex = 0;
		ui->RenderProgress->setFormat("%p%");
		return;
	}

	mRenderTime.start();
	RenderPOVRay();
}

// use to fill the log panel
void swRenderDialog::UpdateLogPanel()
{
	if (!mLogPanel || mLogPanel->isHidden())
		return;

	QFile File(GetStdErrFileName());
	QString Text = File.open(QFile::ReadOnly | QFile::Text) ? QString::fromLocal8Bit(File.readAll()) : QString();

	if (mDialogMode == swRenderDialogMode::RenderBlender)
		Text.prepend(mStdOutList.join(QString()));

	if (Text == mLogEdit->toPlainText())
		return;

	mLogEdit->setPlainText(Text);
	mLogEdit->moveCursor(QTextCursor::End);
}

swRenderDialog::~swRenderDialog()
{
	// save the panel layout
	if (mPanelWindow)
	{
		QSettings Settings;
		Settings.beginGroup(GetPanelStateGroup());
		Settings.setValue("State", mPanelWindow->saveState(SW_RENDER_WINDOW_STATE_VERSION));
		Settings.endGroup();
	}

	delete ui;
}

QString swRenderDialog::GetStdOutFileName() const
{
	QString LogFile = mDialogMode == swRenderDialogMode::RenderPOVRay ? QLatin1String("studworx-povray-render.out") : QLatin1String("studworx-blender-render.out");

	return QDir(QDir::tempPath()).absoluteFilePath(LogFile);
}

QString swRenderDialog::GetStdErrFileName() const
{
	QString LogFile = mDialogMode == swRenderDialogMode::RenderPOVRay ? QLatin1String("studworx-povray-render.err") : QLatin1String("studworx-blender-render.err");

	return QDir(QDir::tempPath()).absoluteFilePath(LogFile);
}

QString swRenderDialog::GetPOVFileName() const
{
	return QDir(QDir::tempPath()).absoluteFilePath("studworx-render.pov");
}

void swRenderDialog::CloseProcess()
{
	delete mProcess;
	mProcess = nullptr;

	if (mDialogMode == swRenderDialogMode::RenderPOVRay)
	{
		if (mPOVRayProgram.Bundled)
		{
			mOutputFile.unmap((uchar*)mOutputBuffer);
			mOutputBuffer = nullptr;
			mOutputFile.close();

			QFile::remove(GetStdOutFileName());
		}

		QFile::remove(GetPOVFileName());
	}

	mRenderButton->setText(tr("Render"));
}

bool swRenderDialog::PromptCancel()
{
	if (!mProcess)
		return true;

	if (QMessageBox::question(this, tr("Cancel Render"), tr("Are you sure you want to cancel the current render?"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		return false;

	if (mProcess)
	{
#ifdef Q_OS_WIN
		swTerminateChildProcess(this, mProcess->processId(), QCoreApplication::applicationPid());
#endif
		mProcess->kill();
	}

	// cancel waiting batch images
	mJobs.clear();
	mJobIndex = 0;
	ui->RenderProgress->setFormat("%p%");

	CloseProcess();

	if (mStdOutList.size())
		WriteStdOut();

	return true;
}

void swRenderDialog::RenderButtonClicked()
{
	if (mProcess)
	{
		PromptCancel();
		return;
	}

	if (mLogEdit)
		mLogEdit->clear();

	mPreviewWidth  = ui->preview->width();
	mPreviewHeight = ui->preview->height();

	mRenderTime.start();

	if (mDialogMode != swRenderDialogMode::RenderPOVRay)
	{
		RenderBlender(false);
		return;
	}

	// set up pov-ray and lgeo first
	mPOVRayProgram = swFindPOVRay();

	if (!mPOVRayProgram.Supported)
	{
		mPOVRayProgram = swSetupPOVRay(this, mPOVRayProgram);

		if (!mPOVRayProgram.Supported)
			return;
	}

	const bool UseLGEO = swGetActiveProject()->GetModels()[0]->GetPOVRayOptions().UseLGEO || swGetProfileInteger(SW_PROFILE_RENDER_PARTS) == 1;

	if (UseLGEO && swFindLGEOPath().isEmpty())
		swSetupLGEO(this);

	if (UseLGEO && !swFindLGEOPath().isEmpty() && !swPOVRayHasIncludeFiles(mPOVRayProgram))
	{
		QMessageBox::warning(this, windowTitle(), tr("LGEO parts need the standard POV-Ray include files such as colors.inc, which POV-Ray %1 could not find.\n\nInstall the povray-includes package or the include files of your POV-Ray version, or choose LDraw parts in the Scene panel.").arg(mPOVRayProgram.Version));
		return;
	}

	// one image per step and camera
	const swModel* Model = swGetActiveProject()->GetModels()[0].get();
	std::vector<swStep> Steps;

	switch (mStepsComboBox->currentIndex())
	{
	case 1:
		Steps.push_back(Model->GetCurrentStep());
		break;

	case 2:
		for (swStep Step = 1; Step <= Model->GetLastStep(); Step++)
			Steps.push_back(Step);
		break;

	default:
		Steps.push_back(SW_STEP_MAXIMUM);
		break;
	}

	QStringList CameraNames;
	const QString CameraData = mCamerasComboBox->currentData().toString();

	if (CameraData == QLatin1String("*"))
	{
		for (int Index = 1; Index < mCamerasComboBox->count() - 1; Index++)
			CameraNames.append(mCamerasComboBox->itemData(Index).toString());
	}
	else
		CameraNames.append(CameraData);

	const QString OutputName = ui->OutputEdit->text();
	const QFileInfo OutputInfo(OutputName);
	const int StepDigits = std::max(2, static_cast<int>(QString::number(Model->GetLastStep()).size()));

	mJobs.clear();
	mJobIndex = 0;
	mJobRegion = GetRegionRect();

	for (swStep Step : Steps)
	{
		for (const QString& CameraName : CameraNames)
		{
			QString Suffix;

			if (Steps.size() > 1)
				Suffix += QString("-%1").arg(Step, StepDigits, 10, QLatin1Char('0'));

			if (CameraNames.size() > 1)
				Suffix += '-' + QString(CameraName).replace(QRegularExpression("[^\\w.-]+"), "_");

			const QString FileName = OutputName.isEmpty() || Suffix.isEmpty() ? OutputName : QDir(OutputInfo.path()).filePath(OutputInfo.completeBaseName() + Suffix + (OutputInfo.suffix().isEmpty() ? QString() : '.' + OutputInfo.suffix()));
			mJobs.push_back({ Step, CameraName, QDir::toNativeSeparators(FileName) });
		}
	}

	StartNextJob();
}

void swRenderDialog::reject()
{
	if (PromptCancel())
		QDialog::reject();
}

void swRenderDialog::RenderPOVRay()
{
	QString FileName = GetPOVFileName();

	ui->preview->SetImage(QImage());
	ui->RenderProgress->setValue(ui->RenderProgress->minimum());
	const swRenderJob& Job = mJobs[mJobIndex];
	const QString JobFormat = mJobs.size() > 1 ? tr("Image %1 of %2").arg(mJobIndex + 1).arg(mJobs.size()) : QString();
	ui->RenderProgress->setFormat(JobFormat.isEmpty() ? tr("Exporting Model") : tr("%1, Exporting Model").arg(JobFormat));

	const swCamera* Camera = nullptr;
	swModel* ActiveModel = swGetActiveModel();

	if (!Job.CameraName.isEmpty() && ActiveModel)
		for (const std::unique_ptr<swCamera>& ModelCamera : ActiveModel->GetCameras())
			if (ModelCamera->GetName() == Job.CameraName)
				Camera = ModelCamera.get();

	QApplication::setOverrideCursor(Qt::WaitCursor);

	swResult<void> ExportResult = swGetActiveProject()->ExportPOVRay(FileName, Camera, Job.Step);

	QGuiApplication::restoreOverrideCursor();

	ui->RenderProgress->setFormat(JobFormat.isEmpty() ? QString("%p%") : tr("%1, %p%").arg(JobFormat));

	if (!ExportResult)
	{
		mJobs.clear();
		mJobIndex = 0;

		if (!ExportResult.error().isEmpty())
			QMessageBox::warning(this, windowTitle(), ExportResult.error());

		return;
	}

	QStringList Arguments;

	Arguments.append(QString::fromLatin1("+I\"%1\"").arg(FileName));
	Arguments.append(QString::fromLatin1("+W%1").arg(QString::number(mWidth)));
	Arguments.append(QString::fromLatin1("+H%1").arg(QString::number(mHeight)));
	Arguments.append("-O-");

	if (swGetProfileInteger(SW_PROFILE_RENDER_TRANSPARENT))
		Arguments.append("+UA");

	// region in one based pixels
	if (mJobRegion.isValid())
	{
		auto RegionStart = [](int Pixel)
		{
			return Pixel > 0 ? QString::number(Pixel + 1) : QString("0.0");
		};

		Arguments.append("+SC" + RegionStart(mJobRegion.left()));
		Arguments.append("+EC" + QString::number(mJobRegion.right() + 1));
		Arguments.append("+SR" + RegionStart(mJobRegion.top()));
		Arguments.append("+ER" + QString::number(mJobRegion.bottom() + 1));
	}

	if (mPOVRayProgram.Bundled)
		Arguments.append(QString::fromLatin1("+SM\"%1\"").arg(GetStdOutFileName()));

	const bool UseLGEO = swGetActiveProject()->GetModels()[0]->GetPOVRayOptions().UseLGEO || swGetProfileInteger(SW_PROFILE_RENDER_PARTS) == 1;
	Arguments.append(swGetPOVRayArguments(mPOVRayProgram, mQuality, UseLGEO));

	swRenderProcess* Process = new swRenderProcess(this);
	if (!mPOVRayProgram.Bundled)
		connect(Process, &QProcess::readyReadStandardError, this, [this]() { bool Error; ReadStdErr(Error); });
	Process->setEnvironment(swGetPOVRayEnvironment());
	Process->setStandardErrorFile(GetStdErrFileName());
	Process->start(mPOVRayProgram.Path, Arguments);

	mImage = QImage(mWidth, mHeight, QImage::Format_ARGB32);
	mImage.fill(Qt::transparent);
	ui->preview->SetImage(mImage);

	if (Process->waitForStarted())
	{
		mProcess = Process;
		mRenderButton->setText(tr("Cancel"));
		ui->RenderProgress->setValue(ui->RenderProgress->minimum());
		mStdErrList.clear();
	}
	else
	{
		delete Process;
		QMessageBox::warning(this, windowTitle(), tr("Could not start POV-Ray."));
		CloseProcess();
	}
}

void swRenderDialog::RenderBlender(bool ImportOnly)
{
	QString Error;

	if (!swSetupBlenderAddon(Error))
	{
		QMessageBox::warning(this, windowTitle(), Error);
		return;
	}

	ui->RenderProgress->setFormat(tr("Saving Blender Model"));
	QApplication::processEvents();

	mBlendProgValue = 0;
	mBlendProgMax = 0;
	mStdOutList.clear();

	swModel* Model = swGetActiveProject()->GetActiveModel();
	const QString ModelFileName = QFileInfo(QDir(swGetProfileString(SW_PROFILE_PROJECTS_PATH)), QString("%1_Step_%2.ldr").arg(QFileInfo(Model->GetProperties().mFileName).baseName()).arg(Model->GetCurrentStep())).absoluteFilePath();

	swResult<void> ExportResult = swGetActiveProject()->ExportCurrentStep(ModelFileName);

	if (!ExportResult)
	{
		QMessageBox::warning(this, windowTitle(), ExportResult.error());
		return;
	}

	ui->RenderProgress->setFormat("%p%");
	QApplication::processEvents();

	if (!QFileInfo(ModelFileName).isReadable())
		return;

	QString PythonExpression = QString("import bpy; bpy.ops.render_scene.lpub3d_render_ldraw('EXEC_DEFAULT', resolution_width=%1, resolution_height=%2, render_percentage=%3, model_file=%4, image_file=%5, search_additional_paths=True")
		.arg(mWidth).arg(mHeight).arg(swGetBlenderSetting("renderpercentage").toDouble())
		.arg(swGetBlenderPythonString(QDir::toNativeSeparators(ModelFileName)), swGetBlenderPythonString(QDir::toNativeSeparators(ui->OutputEdit->text())));

	QStringList Arguments;

	if (ImportOnly)
	{
		PythonExpression.append(", import_only=True");
		Arguments << QLatin1String("--window-geometry") << QLatin1String("200") << QLatin1String("100") << QLatin1String("1440") << QLatin1String("900");
	}
	else
		Arguments << QLatin1String("--background");

	PythonExpression.append(", cli_render=True)");
	Arguments << swGetBlenderArguments(PythonExpression);

	QProcessEnvironment Environment = QProcessEnvironment::systemEnvironment();
	Environment.insert(QLatin1String("LDRAW_DIRECTORY"), swApplication::GetLibraryDirectory());

	const QString BlenderPath = swGetProfileString(SW_PROFILE_BLENDER_PATH);

	if (ImportOnly)
	{
		const QString StdOutFileName = GetStdOutFileName();
		QFile::remove(StdOutFileName);

		QProcess Launcher;
		Launcher.setProgram(BlenderPath);
		Launcher.setArguments(Arguments);
		Launcher.setProcessEnvironment(Environment);
		Launcher.setWorkingDirectory(swGetBlenderDataPath());
		Launcher.setStandardOutputFile(StdOutFileName);
		Launcher.setStandardErrorFile(GetStdErrFileName());

		if (!Launcher.startDetached())
		{
			QMessageBox::warning(this, windowTitle(), tr("Could not start Blender."));
			return;
		}

		// blender keeps running, so only the import messages of its first seconds are shown
		QFile Log(StdOutFileName);
		QFile ErrorLog(GetStdErrFileName());
		const QTime Wait = QTime::currentTime().addSecs(3);

		while (!Log.size() && !ErrorLog.size() && QTime::currentTime() < Wait)
			QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

		if (Log.open(QFile::ReadOnly | QFile::Text) && ErrorLog.open(QFile::ReadOnly | QFile::Text))
		{
			const QString Output = QString::fromLocal8Bit(Log.readAll() + ErrorLog.readAll());
			const bool HasError = Output.contains(QLatin1String("ERROR: "), Qt::CaseInsensitive);
			const bool HasWarning = Output.contains(QLatin1String("WARNING: "), Qt::CaseInsensitive);

			if (HasError || HasWarning)
			{
				const QString Items = HasError ? tr("errors%1").arg(HasWarning ? tr(" and warnings") : "") : tr("warnings");
				const QString Body = tr("Open in Blender encountered %1. See Show Details...").arg(Items);
				swShowDetailedWarning(this, tr("Open in Blender"), Body, Output);
			}
		}

		return;
	}

	mProcess = new swRenderProcess(this);

	connect(mProcess, &QProcess::readyReadStandardOutput, this, &swRenderDialog::ReadStdOut);

	mProcess->setProcessEnvironment(Environment);
	mProcess->setWorkingDirectory(swGetBlenderDataPath());
	mProcess->setStandardErrorFile(GetStdErrFileName());

	ui->RenderProgress->setRange(mBlendProgValue, mBlendProgMax);
	ui->RenderProgress->setValue(1);

	mProcess->start(BlenderPath, Arguments);

	if (mProcess->waitForStarted())
	{
		mRenderButton->setText(tr("Cancel"));
		ui->RenderProgress->setValue(ui->RenderProgress->minimum());
		QApplication::processEvents();
	}
	else
	{
		QMessageBox::warning(this, windowTitle(), tr("Could not start the Blender render process."));
		CloseProcess();
	}
}

void swRenderDialog::ReadStdOut()
{
	if (mDialogMode == swRenderDialogMode::RenderPOVRay)
		return;

	QString StdOut = QString(mProcess->readAllStandardOutput());
	mStdOutList.append(StdOut);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	QRegularExpression RxRenderProgress;
	RxRenderProgress.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
#else
	QRegExp RxRenderProgress;
	RxRenderProgress.setCaseSensitivity(Qt::CaseInsensitive);
#endif
	int BlenderVersionNum = swGetProfileString(SW_PROFILE_BLENDER_VERSION).section(QLatin1Char('.'), 0, 0).toInt();
	bool BlenderVersion3OrGreater = BlenderVersionNum > 2;

	if (BlenderVersion3OrGreater)
		RxRenderProgress.setPattern("Sample (\\d+)\\/(\\d+)");
	else
		RxRenderProgress.setPattern("(\\d+)\\/(\\d+) Tiles");

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	QRegularExpressionMatch Match = RxRenderProgress.match(StdOut);
	if (Match.hasMatch())
	{
		mBlendProgValue = Match.captured(1).toInt();
		mBlendProgMax   = Match.captured(2).toInt();
		ui->RenderProgress->setMaximum(mBlendProgMax);
		ui->RenderProgress->setValue(mBlendProgValue);
	}
#else
	if (StdOut.contains(RxRenderProgress))
	{
		mBlendProgValue = RxRenderProgress.cap(1).toInt();
		mBlendProgMax   = RxRenderProgress.cap(2).toInt();
		ui->RenderProgress->setMaximum(mBlendProgMax);
		ui->RenderProgress->setValue(mBlendProgValue);
	}
#endif
}

QString swRenderDialog::ReadStdErr(bool& HasError) const
{
	HasError = mDialogMode == swRenderDialogMode::RenderBlender ? false : true;

	QFile File;
	QStringList ReturnLines;

	File.setFileName(GetStdErrFileName());

	if (!File.open(QFile::ReadOnly | QFile::Text))
	{
		const QString Message = tr("Failed to open log file: '%1':\n%2").arg(File.fileName(), File.errorString());
		return Message;
	}

	QTextStream In(&File);
	while (!In.atEnd())
	{
		QString Line = In.readLine(0);
		ReturnLines << Line.trimmed();

		if (mDialogMode == swRenderDialogMode::RenderPOVRay)
		{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
			if (Line.contains(QRegularExpression("^POV-Ray finished$", QRegularExpression::CaseInsensitiveOption)))
				HasError = false;
#else
			if (Line.contains(QRegExp("^POV-Ray finished$", Qt::CaseSensitive)))
				HasError = false;
#endif
		}
		else if (mDialogMode == swRenderDialogMode::RenderBlender)
		{
			if (!HasError && !Line.isEmpty())
				HasError = true;
		}
	}

	return ReturnLines.join('\n');
}

void swRenderDialog::WriteStdOut()
{
	QFile File(GetStdOutFileName());

	if (File.open(QFile::WriteOnly | QIODevice::Truncate | QFile::Text))
	{
		QTextStream Out(&File);

		for (const QString& Line : std::as_const(mStdOutList))
			Out << Line;

		File.close();
	}
}

void swRenderDialog::Update()
{
	if (!mProcess)
		return;

	UpdateLogPanel();

	const bool ProcessFinished = mProcess->state() == QProcess::NotRunning;

	if (mDialogMode == swRenderDialogMode::RenderPOVRay && mPOVRayProgram.Bundled)
	{
		if (!mOutputBuffer)
		{
			mOutputFile.setFileName(GetStdOutFileName());

			if (!mOutputFile.open(QFile::ReadWrite))
			{
				if (ProcessFinished)
				{
					ShowResult();
					CloseProcess();
				}

				return;
			}

			mOutputBuffer = mOutputFile.map(0, mOutputFile.size());

			if (!mOutputBuffer)
			{
				mOutputFile.close();

				if (ProcessFinished)
				{
					ShowResult();
					CloseProcess();
				}

				return;
			}
		}

		struct swSharedMemoryHeader
		{
			quint32 Version;
			quint32 Width;
			quint32 Height;
			quint32 PixelsWritten;
			quint32 PixelsRead;
		};

		swSharedMemoryHeader* Header = (swSharedMemoryHeader*)mOutputBuffer;

		if (Header->PixelsWritten == Header->PixelsRead && !ProcessFinished)
			return;

		int Width = Header->Width;
		int Height = Header->Height;
		int PixelsWritten = Header->PixelsWritten;

		if (!Header->PixelsRead)
			mImage = QImage(Width, Height, QImage::Format_ARGB32);

		quint8* Pixels = (quint8*)(Header + 1);

		for (int y = 0; y < Height; y++)
		{
			for (int x = 0; x < Width; x++)
			{
				mImage.setPixel(x, y, qRgba(Pixels[0], Pixels[1], Pixels[2], Pixels[3]));
				Pixels += 4;
			}
		}

		Header->PixelsRead = PixelsWritten;

		ui->RenderProgress->setMaximum(mImage.width() * mImage.height());
		ui->RenderProgress->setValue(int(Header->PixelsRead));

		if (PixelsWritten == Width * Height)
			ui->RenderProgress->setValue(ui->RenderProgress->maximum());

		ui->preview->SetImage(mImage.scaled(mPreviewWidth, mPreviewHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}

	if (ProcessFinished)
	{
		if (mDialogMode == swRenderDialogMode::RenderPOVRay && !mPOVRayProgram.Bundled)
			mImage = QImage::fromData(mProcess->readAllStandardOutput());

		ShowResult();
		CloseProcess();

		if (!mJobs.empty())
		{
			mJobIndex++;
			StartNextJob();
		}
	}
}

void swRenderDialog::ShowResult()
{
	bool Error;
	const QString StdErrLog = ReadStdErr(Error);

	UpdateLogPanel();

	if (mProcess->exitStatus() != QProcess::NormalExit || mProcess->exitCode() != 0 || Error)
	{
		ui->RenderProgress->setRange(0, 1);
		ui->RenderProgress->setValue(0);
		mJobs.clear();
		mJobIndex = 0;
		ui->RenderProgress->setFormat("%p%");

		const QString Body  = tr("An error occurred while rendering.");
		swShowDetailedWarning(this, windowTitle(), Body, StdErrLog);
		return;
	}
	else
	{
		ui->RenderProgress->setValue(ui->RenderProgress->maximum());
	}

	bool Success = false;
	QString FileName = mDialogMode == swRenderDialogMode::RenderPOVRay && mJobIndex < mJobs.size() ? mJobs[mJobIndex].FileName : ui->OutputEdit->text();

	if (mDialogMode == swRenderDialogMode::RenderPOVRay)
	{
		if (mJobRegion.isValid() && !mImage.isNull())
		{
			// place regions into the saved image
			const QSize ImageSize(mWidth, mHeight);
			QImage BaseImage = mResultFileName == FileName && mResultImage.size() == ImageSize ? mResultImage : QImage();

			if (BaseImage.isNull() && !FileName.isEmpty())
			{
				QImageReader Reader(FileName);

				if (Reader.size() == ImageSize)
					BaseImage = Reader.read();
			}

			if (BaseImage.size() == ImageSize)
			{
				BaseImage = BaseImage.convertToFormat(QImage::Format_ARGB32);
				QPainter Painter(&BaseImage);
				Painter.setCompositionMode(QPainter::CompositionMode_Source);

				if (mImage.size() == ImageSize)
					Painter.drawImage(mJobRegion, mImage, mJobRegion);
				else if (mImage.size() == mJobRegion.size())
					Painter.drawImage(mJobRegion.topLeft(), mImage);

				Painter.end();
				mImage = BaseImage;
			}
		}

		mResultImage = mImage;
		mResultFileName = FileName;
		ui->preview->SetImage(mImage.scaled(mPreviewWidth, mPreviewHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));

		if (!FileName.isEmpty())
		{
			QImageWriter Writer(FileName);

			Success = Writer.write(mImage);

			if (!Success)
				QMessageBox::warning(this, windowTitle(), tr("Could not write file '%1':\n%2").arg(FileName, Writer.errorString()));
		}

	}
	else if (mDialogMode == swRenderDialogMode::RenderBlender)
	{
		Success = QFileInfo::exists(FileName);
		if (Success)
		{
			setMinimumSize(100, 100);
			QImageReader Reader(FileName);
			mImage = Reader.read();

			if (swGetProfileInteger(SW_PROFILE_BLENDER_CROP_IMAGE) && mImage.hasAlphaChannel())
			{
				const QRect CropRect = swGetOpaqueRect(mImage);

				if (CropRect.isValid() && CropRect != mImage.rect())
				{
					mImage = mImage.copy(CropRect);
					QImageWriter Writer(FileName);

					if (!Writer.write(mImage))
						QMessageBox::warning(this, windowTitle(), tr("Could not write file '%1':\n%2").arg(FileName, Writer.errorString()));
				}
			}

			mImage = mImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);

			mPreviewWidth  = mImage.width();
			mPreviewHeight = mImage.height();

			ui->preview->SetImage(mImage.scaled(mPreviewWidth, mPreviewHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}
	}

	if (!Success)
	{
		mJobs.clear();
		mJobIndex = 0;
		QMessageBox::warning(this, windowTitle(), tr("Could not render the image."));
	}

	WriteStdOut();
}

void swRenderDialog::OutputBrowseButtonClicked()
{
	const QString Result = QFileDialog::getSaveFileName(this, tr("Save Image File"), ui->OutputEdit->text(), tr("Supported Image Files (*.bmp *.png *.jpg);;BMP Files (*.bmp);;PNG Files (*.png);;JPEG Files (*.jpg);;All Files (*)"));

	if (!Result.isEmpty())
		ui->OutputEdit->setText(QDir::toNativeSeparators(Result));
}

swRenderProcess::~swRenderProcess()
{
	if (state() == QProcess::Running || state() == QProcess::Starting)
	{
		terminate();
		waitForFinished();
	}
}

#endif // SW_DISABLE_RENDER_DIALOG
