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
	QString ExceptionsPath;
};

// library parts and the bricklink numbers that replace their mapping
using swBrickLinkExceptions = std::vector<std::pair<QString, QString>>;

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

QString swGetBrickLinkExceptionsPath(const QString& FileName);
swResult<swBrickLinkExceptions> swLoadBrickLinkExceptions(const QString& FileName);
swResult<void> swSaveBrickLinkExceptions(const QString& FileName, const swBrickLinkExceptions& Exceptions);

std::optional<double> swGetBrickLinkWeight(const swPartData* PartData);
QString swGetBrickLinkWeightText(const swPartData* PartData);

swResult<int> swExportBrickLink(const QString& FileName, const swPartsList& PartsList, const swBrickLinkOptions& Options);
swResult<int> swUpdateBrickLinkMapping(QWidget* Parent, const QString& FileName, const QString& ApiKey);
