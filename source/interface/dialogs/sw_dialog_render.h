#pragma once

#include "sw_renderer_povray.h"

enum class swPOVRayRenderQuality
{
	Low,
	Medium,
	High
};

enum class swPOVRayLightRig
{
	Standard,
	Studio,
	Matte,
	Count
};

enum class swPOVRayFloorStyle
{
	Plain,
	ShadowCatcher,
	Reflective,
	Cyclorama,
	Count
};

enum class swRenderDialogMode
{
	RenderPOVRay,
	RenderBlender
};

#ifndef SW_DISABLE_RENDER_DIALOG

namespace Ui {
class swRenderDialog;
}

class swEditorTextEdit;

class swRenderProcess : public QProcess
{
	Q_OBJECT

public:
	explicit swRenderProcess(QObject* Parent)
		: QProcess(Parent)
	{
	}
	~swRenderProcess();
};

class swRenderPreviewWidget : public QWidget
{
	Q_OBJECT

public:
	explicit swRenderPreviewWidget(QWidget* Parent)
		: QWidget(Parent)
	{
	}

	void SetImage(QImage Image)
	{
		mImage = Image;
		mScaledImage = QImage();
		update();
	}

	// Sets the faded image shown before a render.
	void SetGuideImage(const QImage& Image)
	{
		mGuideImage = Image;
		update();
	}

	// Sets the empty canvas icon size.
	void SetPlaceholderSize(int Size)
	{
		mPlaceholderSize = Size;
		update();
	}

	// Sets the empty canvas image size.
	void SetCanvasSize(const QSize& Size)
	{
		mCanvasSize = Size;
		update();
	}

	// Shows and draws the render region.
	void SetRegionMode(bool RegionMode)
	{
		mRegionMode = RegionMode;
		setMouseTracking(RegionMode);
		setCursor(RegionMode ? Qt::CrossCursor : Qt::ArrowCursor);
		update();
	}

	// Sets the render region fraction.
	void SetRegion(const QRectF& Region)
	{
		mRegion = Region;
		update();
	}

	QRectF GetRegion() const
	{
		return mRegion;
	}

signals:
	void RegionChanged(const QRectF& Region);

protected:
	void resizeEvent(QResizeEvent* Event) override;
	void paintEvent(QPaintEvent* PaintEvent) override;
	void mousePressEvent(QMouseEvent* MouseEvent) override;
	void mouseMoveEvent(QMouseEvent* MouseEvent) override;
	void mouseReleaseEvent(QMouseEvent* MouseEvent) override;
	QRect GetCanvasRect() const;
	QPointF GetCanvasPoint(const QPoint& Position) const;
	QRectF GetDrawnRegion(const QPoint& Position, Qt::KeyboardModifiers Modifiers) const;
	bool IsInsideRegion(const QPointF& Point) const;
	Qt::Edges GetRegionEdges(const QPoint& Position) const;

	QImage mImage;
	QImage mScaledImage;
	QImage mGuideImage;
	int mPlaceholderSize = 0;
	QSize mCanvasSize;
	bool mRegionMode = false;
	bool mRegionDragging = false;
	bool mRegionMoving = false;
	Qt::Edges mRegionEdges;
	QPointF mRegionStart;
	QRectF mRegion = QRectF(0.25, 0.25, 0.5, 0.5);
};

// Bounds of the pixels that are not fully transparent, empty when there are none.
QRect swGetOpaqueRect(const QImage& Image);

struct swRenderJob
{
	swStep Step;
	QString CameraName;
	QString FileName;
};

class swRenderDialog : public QDialog
{
	Q_OBJECT

public:
	explicit swRenderDialog(QWidget* Parent, swRenderDialogMode Mode);
	virtual ~swRenderDialog();

public slots:
	void reject() override;

private slots:
	void OutputBrowseButtonClicked();
	void Update();

protected slots:
	void RenderButtonClicked();
	void ReadStdOut();
	void WriteStdOut();

protected:
	QString GetStdOutFileName() const;
	QString GetStdErrFileName() const;
	QString GetPOVFileName() const;
	void CreatePanels();
	QString GetPanelStateGroup() const;
	void UpdateLogPanel();
	QString ReadStdErr(bool& Error) const;
	void RenderPOVRay();
	void RenderBlender(bool ImportOnly);
	void CloseProcess();
	bool PromptCancel();
	void ShowResult();
	void StartNextJob();
	QRect GetRegionRect() const;
	QStringList GetPresetValues() const;
	void ApplyPreset(const QString& Preset);
	void UpdatePresets();
	void SavePresetClicked();
	void DeletePresetClicked();
	void UpdateCameras();
	void UpdateGuideImage();
	swCamera* GetFirstCamera(swModel* Model) const;
	void LoadEditorScene();
	bool IsEditedScene() const;

	QTimer mUpdateTimer;
	QTimer mGuideTimer;
	QElapsedTimer mRenderTime;
	QFile mOutputFile;
	void* mOutputBuffer = nullptr;
	QImage mImage;
	QStringList mStdErrList;
	QStringList mStdOutList;
	QAbstractButton* mRenderButton = nullptr;
	QAbstractButton* mLogButton = nullptr;
	QAbstractButton* mEditorButton = nullptr;
	QMainWindow* mPanelWindow = nullptr;
	QDockWidget* mLogPanel = nullptr;
	QPlainTextEdit* mLogEdit = nullptr;
	QDockWidget* mEditorPanel = nullptr;
	swEditorTextEdit* mEditorEdit = nullptr;
	QCheckBox* mEditorCheckBox = nullptr;
	QList<QWidget*> mEditorDisabledWidgets;
	swRenderProcess* mProcess = nullptr;
	QComboBox* mPresetComboBox = nullptr;
	QAbstractButton* mPresetDeleteButton = nullptr;
	QSpinBox* mWidthSpinBox = nullptr;
	QSpinBox* mHeightSpinBox = nullptr;
	QComboBox* mQualityComboBox = nullptr;
	QComboBox* mFocalBlurComboBox = nullptr;
	QComboBox* mLightRigComboBox = nullptr;
	QComboBox* mFloorStyleComboBox = nullptr;
	QComboBox* mBackgroundComboBox = nullptr;
	QLineEdit* mEnvironmentEdit = nullptr;
	QComboBox* mAreaComboBox = nullptr;
	QComboBox* mStepsComboBox = nullptr;
	QComboBox* mCamerasComboBox = nullptr;
	std::vector<swRenderJob> mJobs;
	size_t mJobIndex = 0;
	QRect mJobRegion;
	QString mPresetName;

	int mWidth = 1280;
	int mHeight = 720;
	swPOVRayRenderQuality mQuality = swPOVRayRenderQuality::High;
	swPOVRayProgram mPOVRayProgram;
	int mPreviewWidth;
	int mPreviewHeight;
	swRenderDialogMode mDialogMode;
	int mBlendProgValue;
	int mBlendProgMax;

	Ui::swRenderDialog* ui;
};

#else

class swRenderPreviewWidget : public QWidget
{
public:
	explicit swRenderPreviewWidget(QWidget* Parent)
	    : QWidget(Parent)
	{
	}
};

#endif // SW_DISABLE_RENDER_DIALOG
