#pragma once

#include "sw_application.h"
#include "sw_shortcuts.h"
#include "sw_category.h"
#include "sw_instructions.h"
#include "sw_color_configuration.h"

// The instructions preferences.
struct swInstructionsPreferences
{
	swInstructionsPageSetup PageSetup;
	int PageUnits;
	int MarginUnits;
	bool ShowGrid;
	bool SnapToGrid;
	float GridSize;
	int GridDivisions;
	bool GridWithinMargins;
	bool NewModelDefaults;
	bool ShowMargins;
	bool ShowRulers;
	bool ShowElementMargins;
	bool ShowElementPadding;
	bool ShowElementAnchors;
	quint32 ElementMarginColor;
	quint32 ElementPaddingColor;
	quint32 ElementAnchorColor;
	quint32 RulerPointerColor;
	int ElementMarginAlpha;
	int ElementPaddingAlpha;
	int ElementAnchorAlpha;
	int RulerPointerAlpha;
	QColor GridPrimaryColor;
	QColor GridSecondaryColor;
	QColor MarginColor;
	bool EditorHighlight;
	bool EditorNumbers;
	bool EditorWrap;
	bool EditorMarkers;

	void Load();
	void Save() const;
};

struct swPreferencesDialogOptions
{
	swPreferences Preferences;

	QString LibraryPath;
	QString UnofficialLibraryPath;
	QString ShadowLibraryPath;
	QString ColorConfigPath;
	QString ColorConfigAlternatePath;
	swColorSource ColorSource;
	QString MinifigureBuilderSettingsPath;
	QString BrickLinkMappingPath;
	QString POVRayPath;
	QString BlenderPath;
	QString BlenderVersion;
	QString LGEOPath;
	QString RebrickableApiKey;
	QString DefaultAuthorName;
	QString DefaultAuthorEmail;
	QString DefaultAuthorComment;
	QString DefaultAuthorSplitter;
	QString Language;

	swInstructionsPreferences Instructions;

	int AASamples;
	swStudStyle StudStyle;

	std::vector<swLibraryCategory> Categories;
	bool CategoriesModified;
	bool CategoriesDefault;

	swKeyboardShortcuts KeyboardShortcuts;
	bool KeyboardShortcutsModified;
	bool KeyboardShortcutsDefault;

	swMouseShortcuts MouseShortcuts;
	bool MouseShortcutsModified;
	bool MouseShortcutsDefault;
};

namespace Ui
{
class swPreferencesDialog;
}

class swPreferencesDialog : public QDialog
{
	Q_OBJECT

public:
	swPreferencesDialog(QWidget* Parent, swPreferencesDialogOptions* Options);
	~swPreferencesDialog();

	swPreferencesDialogOptions* mOptions;

	enum
	{
		CategoryRole = Qt::UserRole
	};

	bool eventFilter(QObject* Object, QEvent* Event) override;

protected:
	void showEvent(QShowEvent* Event) override;

public slots:
	void accept() override;

private slots:
	void PartsLibraryBrowseClicked();
	void UnofficialLibraryBrowseButtonClicked();
	void ShadowLibraryBrowseButtonClicked();
	void ShadowLibraryEnabledToggled(bool Checked);
	void ColorConfigBrowseButtonClicked();
	void ColorConfigUpdateButtonClicked();
	void ColorConfigAlternateBrowseButtonClicked();
	void ColorConfigAlternateUpdateButtonClicked();
	void MinifigureBuilderSettingsBrowseButtonClicked();
	void MinifigureBuilderSettingsUpdateButtonClicked();
	void BrickLinkMappingBrowseButtonClicked();
	void BrickLinkMappingUpdateButtonClicked();
	void RebrickableApiKeyButtonClicked();
	void PovrayExecutableBrowseClicked();
	void LgeoPathBrowseClicked();
	void AuthorChanged();
	void ColorThemeCurrentIndexChanged(int Index);
	void ColorButtonClicked();
	void AutomateEdgeColor();
	void AutomateEdgeColorToggled();
	void EdgeColorSliderValueChanged();
	void BlenderExecutableBrowseClicked();
	void StudStyleComboCurrentIndexChanged(int Index);
	void AntiAliasingToggled();
	void EdgeLinesToggled();
	void ConditionalLinesCheckBoxToggled();
	void LineWidthSliderValueChanged();
	void AxisIconCubeOutlineAlphaChanged();
	void CutawayShadeAlphaChanged();
	void ElementAlphaChanged();
	void RenderTargetChanged();
	void MeshLODSliderValueChanged();
	void FadeStepsToggled();
	void FadeInlineToggled();
	void FadeJointToggled();
	void HighlightNewPartsToggled();
	void GridStudsToggled();
	void GridLinesToggled();
	void ViewCubeSizeComboCurrentIndexChanged(int Index);
	void PreviewViewCubeSizeComboCurrentIndexChanged(int Index);
	void AxisIconSizeComboCurrentIndexChanged(int Index);
	void UpdateParts();
	void CategoriesDropped(const QModelIndex& Parent, int First, int Last);
	void CategoryItemChanged(QTreeWidgetItem* CategoryItem, int Column);
	void NewCategoryClicked();
	void EditCategoryClicked();
	void DeleteCategoryClicked();
	void ImportCategoriesClicked();
	void ExportCategoriesClicked();
	void ResetCategoriesClicked();
	void ShortcutAssignClicked();
	void ShortcutRemoveClicked();
	void ShortcutsImportClicked();
	void ShortcutsExportClicked();
	void ShortcutsResetClicked();
	void CommandChanged(QTreeWidgetItem* Current);
	void KeyboardFilterEditTextEdited(const QString& Text);
	void MouseAssignClicked();
	void MouseRemoveClicked();
	void MouseImportButtonClicked();
	void MouseExportButtonClicked();
	void MouseResetClicked();
	void MouseTreeItemChanged(QTreeWidgetItem* Current);
	void MouseFilterEditTextEdited(const QString& Text);

private:
	Ui::swPreferencesDialog* ui;

	QLineEdit* GetColorConfigurationEdit(swColorSource Source) const;
	QString GetMinifigureBuilderSettingsStartPath() const;
	void BrowseColorConfiguration(swColorSource Source);
	void UpdateColorConfiguration(swColorSource Source);
	bool EnsureColorConfiguration();
	void UpdateCategories();
	void SnapCategoriesTree();
	void UpdateCommandList();
	void UpdateMouseTree();
	void UpdateMouseTreeItem(int ItemIndex);
	QTreeWidgetItem* FindMouseTreeItem(int ItemIndex) const;
	static void UpdateStudSuffix(QSpinBox* SpinBox);
	void SetShortcutModified(QTreeWidgetItem* TreeItem, bool Modified);
	void UpdateShortcutConflicts();
	void CreateInstructionsTab();
	void CreateContextMenuTab();
	void SetContextMenuTree(const QStringList& Layout);
	QStringList GetContextMenuLayout() const;
	void UpdateInstructionsPage();

	QPushButton* mInstructionsPageButton = nullptr;
	QCheckBox* mInstructionsShowGrid = nullptr;
	QCheckBox* mInstructionsSnapToGrid = nullptr;
	QDoubleSpinBox* mInstructionsGridSize = nullptr;
	QComboBox* mInstructionsGridScope = nullptr;
	QSpinBox* mInstructionsGridDivisions = nullptr;
	QToolButton* mInstructionsGridPrimaryColor = nullptr;
	QToolButton* mInstructionsGridSecondaryColor = nullptr;
	QToolButton* mInstructionsMarginColor = nullptr;
	QCheckBox* mInstructionsNewModelDefaults = nullptr;
	QCheckBox* mInstructionsShowMargins = nullptr;
	QCheckBox* mInstructionsShowRulers = nullptr;
	QCheckBox* mInstructionsShowElementMargins = nullptr;
	QCheckBox* mInstructionsShowElementPadding = nullptr;
	QCheckBox* mInstructionsShowElementAnchors = nullptr;
	QCheckBox* mInstructionsEditorHighlight = nullptr;
	QCheckBox* mInstructionsEditorNumbers = nullptr;
	QCheckBox* mInstructionsEditorWrap = nullptr;
	QCheckBox* mInstructionsEditorMarkers = nullptr;
	QTreeWidget* mContextCommandTree = nullptr;
	QTreeWidget* mContextMenuTree = nullptr;

	struct swRenderTargetWidgets
	{
		QCheckBox* SyncRendering;
		QLabel* ShadingModeLabel;
		QComboBox* ShadingMode;
		QCheckBox* EdgeLines;
		QCheckBox* ConditionalLines;
		QLabel* LineWidthTitle;
		QSlider* LineWidthSlider;
		QLabel* LineWidthLabel;
	};

	swRenderTargetWidgets GetRenderTargetWidgets(swRenderTarget Target) const;

	Qt::MouseButton mMouseShortcutButton = Qt::NoButton;
	Qt::KeyboardModifiers mMouseShortcutModifiers = Qt::NoModifier;
	float mLineWidthRange[2];
	float mLineWidthGranularity;
	static constexpr float mMeshLODMultiplier = 25.0f;
};
