#pragma once

#include "sw_result.h"

class QWidget;

enum class swBrickLinkCondition
{
	Any,
	New,
	Used
};

struct swBrickLinkOptions
{
	swBrickLinkCondition Condition = swBrickLinkCondition::Any;
	QString WantedListId;
	bool Notify = false;
};

struct swBrickLinkExport
{
	QString FileName;
	int Lots = 0;
};

swBrickLinkOptions swGetBrickLinkOptions();
std::optional<QString> swParseBrickLinkWantedListId(const QString& Text);
QString swGetBrickLinkUploadUrl();
QString swGetRebrickableApiKeyUrl();
QString swGetRebrickableKeyRejectedMessage();
void swForgetRebrickableApiKey(const QString& ApiKey);
void swSetBrickLinkOptions(const swBrickLinkOptions& Options);

QString swGetBrickLinkMappingPath();
bool swBrickLinkMappingExists();

swResult<int> swExportBrickLink(const QString& FileName, const swPartsList& PartsList, const swBrickLinkOptions& Options);
swResult<int> swUpdateBrickLinkMapping(QWidget* Parent, const QString& FileName, const QString& ApiKey);
