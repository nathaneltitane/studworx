#pragma once

#include "sw_result.h"

class QWidget;

QString swGetMinifigureConfigurationName();
QString swGetMinifigureConfigurationResource();

swResult<void> swUpdateMinifigureConfiguration(QWidget* Parent, const QString& FileName);
