#pragma once

#include "sw_result.h"

class QWidget;

enum class swColorSource
{
	Library,
	Standard,
	Alternate
};

swColorSource swGetColorSource();
void swSetColorSource(swColorSource Source);

QString swGetColorConfigurationName(swColorSource Source);
QString swGetColorConfigurationOfficialName(swColorSource Source);
QString swGetColorConfigurationPath(swColorSource Source);
QString swGetColorConfigurationResource(swColorSource Source);
void swSetColorConfigurationPath(swColorSource Source, const QString& FileName);
bool swColorConfigurationExists(const QString& FileName);

swResult<void> swUpdateColorConfiguration(QWidget* Parent, swColorSource Source, const QString& FileName);
