#pragma once

// The bundled LDraw import and render add-ons Blender loads from the data folder.
QString swGetBlenderDataPath();
QString swGetBlenderAddonPath();

// The render add-on configuration folder holding the settings StudWorx writes.
QString swGetBlenderConfigurationPath();

// Reads and writes the import settings the render add-on loads, keys as the add-on names them.
QString swGetBlenderSetting(const QString& Key);
void swSetBlenderSetting(const QString& Key, const QString& Value);

// Copies the bundled add-ons into the data folder, keeping the config files already there.
bool swSetupBlenderAddon(QString& Error);

// Runs Blender to read its version, empty with an error when it is not a working Blender.
QString swGetBlenderVersion(const QString& Path, QString& Error);

// Quotes a path or text for a Blender Python expression.
QString swGetBlenderPythonString(const QString& Text);

// Arguments that load the bundled add-ons into Blender and then run an expression.
QStringList swGetBlenderArguments(const QString& Expression);
