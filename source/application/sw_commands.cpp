#include "sw_global.h"
#include "sw_commands.h"

const swCommand gCommands[] =
{
	// SW_FILE_NEW
	{
		QT_TRANSLATE_NOOP("Action", "File.New"),
		QT_TRANSLATE_NOOP("Menu", "&New"),
		QT_TRANSLATE_NOOP("Status", "Create a new model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>New Model</b></p><p>Create a new, empty model. If the current model has unsaved changes, StudWorx asks whether to save them before replacing it.</p>"),
		""
	},

	// SW_FILE_OPEN
	{
		QT_TRANSLATE_NOOP("Action", "File.Open"),
		QT_TRANSLATE_NOOP("Menu", "&Open..."),
		QT_TRANSLATE_NOOP("Status", "Open an existing model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Open Model</b></p><p>Choose an existing model file to work on. If the current model has unsaved changes, save or discard those changes before opening the chosen file.</p>"),
		"Ctrl+O"
	},

	// SW_FILE_MERGE
	{
		QT_TRANSLATE_NOOP("Action", "File.Merge"),
		QT_TRANSLATE_NOOP("Menu", "&Merge..."),
		QT_TRANSLATE_NOOP("Status", "Merge the contents of another file with the current one"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Merge</b></p><p>Choose one or more model files and add their models to the current file as submodels, renamed when a name is already taken.</p>"),
		"Ctrl+M"
	},

	// SW_FILE_RELOAD
	{
		QT_TRANSLATE_NOOP("Action", "File.Reload"),
		QT_TRANSLATE_NOOP("Menu", "&Reload"),
		QT_TRANSLATE_NOOP("Status", "Reload the current model from disk"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Reload Model</b></p><p>Read the current model file again from disk, replacing the model in StudWorx. If the model has unsaved changes, StudWorx asks before discarding them.</p>"),
		""
	},

	// SW_FILE_RELOAD_LIBRARY
	{
		QT_TRANSLATE_NOOP("Action", "File.ReloadLibrary"),
		QT_TRANSLATE_NOOP("Menu", "Reload &Library"),
		QT_TRANSLATE_NOOP("Status", "Reload the parts library from disk"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Reload Library</b></p><p>Read the parts library and the unofficial library archive again from disk without restarting. Open models keep their parts, parts that no longer exist become missing parts, and new parts appear in the Parts panel.</p>"),
		""
	},

	// SW_FILE_TOGGLE_WATCH_FILE_CHANGES
	{
		QT_TRANSLATE_NOOP("Action", "File.ToggleWatchFileChanges"),
		QT_TRANSLATE_NOOP("Menu", "Watch for File Changes"),
		QT_TRANSLATE_NOOP("Status", "Toggle asking to reload the model when another application changes its file"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Watch for File Changes</b></p><p>When another program changes the file of an open model, StudWorx asks whether to reload it. Turn this off to keep working without being asked.</p>"),
		""
	},

	// SW_FILE_SAVE
	{
		QT_TRANSLATE_NOOP("Action", "File.Save"),
		QT_TRANSLATE_NOOP("Menu", "&Save"),
		QT_TRANSLATE_NOOP("Status", "Save the current model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Save Model</b></p><p>Save the model and its submodels to disk. The first save asks for a filename. Use Save As to save a separate copy under another name.</p>"),
		"Ctrl+S"
	},

	// SW_FILE_SAVE_AS
	{
		QT_TRANSLATE_NOOP("Action", "File.SaveAs"),
		QT_TRANSLATE_NOOP("Menu", "Save &As..."),
		QT_TRANSLATE_NOOP("Status", "Save the current model with a new name"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Save As</b></p><p>Save the current model under a new name or in a new folder. The extension is added when left out: .ldr for a single model, .mpd for a model with submodels or embedded data.</p>"),
		"Alt+S"
	},

	// SW_FILE_SAVE_IMAGE
	{
		QT_TRANSLATE_NOOP("Action", "File.SaveImage"),
		QT_TRANSLATE_NOOP("Menu", "Ima&ge..."),
		QT_TRANSLATE_NOOP("Status", "Export an image of the current view"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Image</b></p><p>Save the model as a still image, a series of step images or an animated GIF, with the background, steps, framing, size and file chosen in the Image dialog.</p>"),
		"Alt+Shift+I"
	},

	// SW_FILE_IMPORT_LDD
	{
		QT_TRANSLATE_NOOP("Action", "File.Import.LDD"),
		QT_TRANSLATE_NOOP("Menu", "&LEGO® Digital Designer..."),
		QT_TRANSLATE_NOOP("Status", "Import a file in LEGO® Digital Designer LXF or LXFML format"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>LEGO® Digital Designer</b></p><p>Open a LEGO® Digital Designer model (.lxf or .lxfml) as a new untitled project. Parts and colors are translated through the bundled mapping.</p>"),
		""
	},

	// SW_FILE_IMPORT_INVENTORY
	{
		QT_TRANSLATE_NOOP("Action", "File.Import.Inventory"),
		QT_TRANSLATE_NOOP("Menu", "Set &Inventory..."),
		QT_TRANSLATE_NOOP("Status", "Import all parts from an official set"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Set Inventory</b></p><p>Search an official set by name or number and open all of its parts as a new model, after saving the current one. It uses your own free Rebrickable API key, asked for once or set under Online Services in Preferences.</p>"),
		""
	},

	// SW_FILE_EXPORT_3DS
	{
		QT_TRANSLATE_NOOP("Action", "File.Export.3DS"),
		QT_TRANSLATE_NOOP("Menu", "3D &Studio..."),
		QT_TRANSLATE_NOOP("Status", "Export the current model in 3D Studio 3DS format"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>3D Studio</b></p><p>Export the current model as a 3D Studio .3ds file for other 3D programs.</p>"),
		""
	},

	// SW_FILE_EXPORT_COLLADA
	{
		QT_TRANSLATE_NOOP("Action", "File.Export.COLLADA"),
		QT_TRANSLATE_NOOP("Menu", "&COLLADA..."),
		QT_TRANSLATE_NOOP("Status", "Export the current model in COLLADA DAE format"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>COLLADA</b></p><p>Export the current model as a COLLADA .dae file. Translucent colors can be marked as transparent materials.</p>"),
		""
	},

	// SW_FILE_EXPORT_HTML
	{
		QT_TRANSLATE_NOOP("Action", "File.Export.HTML"),
		QT_TRANSLATE_NOOP("Menu", "&HTML..."),
		QT_TRANSLATE_NOOP("Status", "Create an HTML page for the current model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>HTML</b></p><p>Open the Instructions window to export the building instructions as styled HTML pages in a folder named after the model.</p>"),
		""
	},

	// SW_FILE_EXPORT_KMZ
	{
		QT_TRANSLATE_NOOP("Action", "File.Export.KMZ"),
		QT_TRANSLATE_NOOP("Menu", "&KMZ..."),
		QT_TRANSLATE_NOOP("Status", "Export the current model in KMZ format"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>KMZ</b></p><p>Export the current model as a KMZ file for Google Earth and SketchUp, with the visible surfaces only or every part's geometry.</p>"),
		""
	},

	// SW_FILE_EXPORT_BRICKLINK
	{
		QT_TRANSLATE_NOOP("Action", "File.Export.BrickLink"),
		QT_TRANSLATE_NOOP("Menu", "&BrickLink..."),
		QT_TRANSLATE_NOOP("Status", "Export a list of parts used in BrickLink XML format"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>BrickLink</b></p><p>Export the parts of the model as a BrickLink wanted list in XML, with part numbers and colors translated to BrickLink numbers, ready to copy and upload.</p>"),
		"Ctrl+Shift+B"
	},

	// SW_FILE_EXPORT_CSV
	{
		QT_TRANSLATE_NOOP("Action", "File.Export.CSV"),
		QT_TRANSLATE_NOOP("Menu", "CS&V..."),
		QT_TRANSLATE_NOOP("Status", "Export a list of parts used in comma delimited file format"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>CSV</b></p><p>Export the parts of the model as a comma separated list, with part numbers, colors and counts, for a spreadsheet.</p>"),
		""
	},

	// SW_FILE_EXPORT_POVRAY
	{
		QT_TRANSLATE_NOOP("Action", "File.Export.POVRay"),
		QT_TRANSLATE_NOOP("Menu", "&POV-Ray..."),
		QT_TRANSLATE_NOOP("Status", "Export the current model in POV-Ray format"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>POV-Ray</b></p><p>Export the current model as a POV-Ray scene file, with LGEO high detail parts when they are set up.</p>"),
		""
	},

	// SW_FILE_EXPORT_WAVEFRONT
	{
		QT_TRANSLATE_NOOP("Action", "File.Export.Wavefront"),
		QT_TRANSLATE_NOOP("Menu", "&Wavefront..."),
		QT_TRANSLATE_NOOP("Status", "Export the current model in Wavefront OBJ format"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Wavefront</b></p><p>Export the current model as a Wavefront .obj file with its materials for other 3D programs.</p>"),
		""
	},

	// SW_FILE_RENDER_POVRAY
	{
		QT_TRANSLATE_NOOP("Action", "File.Render.POVRay"),
		QT_TRANSLATE_NOOP("Menu", "&POV-Ray Render..."),
		QT_TRANSLATE_NOOP("Status", "Render the current model using POV-Ray"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>POV-Ray Render</b></p><p>Render the current model with POV-Ray in the Render window: choose the output file, follow the progress and watch the image appear as it renders.</p>"),
		"Alt+Shift+P"
	},

	// SW_FILE_RENDER_BLENDER
	{
		QT_TRANSLATE_NOOP("Action", "File.Render.Blender"),
		QT_TRANSLATE_NOOP("Menu", "&Blender Render..."),
		QT_TRANSLATE_NOOP("Status", "Render the current model using Blender"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Blender Render</b></p><p>Render the current model with Blender and the bundled LDraw add-ons, or open the scene in Blender. The Blender executable is set in Preferences.</p>"),
		"Alt+Shift+B"
	},

	// SW_FILE_INSTRUCTIONS
	{
		QT_TRANSLATE_NOOP("Action", "File.Instructions"),
		QT_TRANSLATE_NOOP("Menu", "Instr&uctions..."),
		QT_TRANSLATE_NOOP("Status", "Configure instructions layout"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Instructions</b></p><p>Open the Instructions window to lay out, style, export and print the building instructions of the model.</p>"),
		"Alt+Shift+U"
	},

	// SW_FILE_PRINT
	{
		QT_TRANSLATE_NOOP("Action", "File.Print"),
		QT_TRANSLATE_NOOP("Menu", "&Print..."),
		QT_TRANSLATE_NOOP("Status", "Print the current model view"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Print</b></p><p>Print the current model view through the system print dialog, with a preview where the dialog allows it.</p>"),
		"Ctrl+P"
	},

	// SW_FILE_RECENT1
	{
		QT_TRANSLATE_NOOP("Action", "File.Recent1"),
		QT_TRANSLATE_NOOP("Menu", "&Recent 1"),
		QT_TRANSLATE_NOOP("Status", "Open this model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Recent File</b></p><p>Open this recently used model again. The File menu keeps the four most recent files.</p>"),
		""
	},

	// SW_FILE_RECENT2
	{
		QT_TRANSLATE_NOOP("Action", "File.Recent2"),
		QT_TRANSLATE_NOOP("Menu", "&Recent 2"),
		QT_TRANSLATE_NOOP("Status", "Open this model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Recent File</b></p><p>Open this recently used model again. The File menu keeps the four most recent files.</p>"),
		""
	},

	// SW_FILE_RECENT3
	{
		QT_TRANSLATE_NOOP("Action", "File.Recent3"),
		QT_TRANSLATE_NOOP("Menu", "&Recent 3"),
		QT_TRANSLATE_NOOP("Status", "Open this model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Recent File</b></p><p>Open this recently used model again. The File menu keeps the four most recent files.</p>"),
		""
	},

	// SW_FILE_RECENT4
	{
		QT_TRANSLATE_NOOP("Action", "File.Recent4"),
		QT_TRANSLATE_NOOP("Menu", "&Recent 4"),
		QT_TRANSLATE_NOOP("Status", "Open this model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Recent File</b></p><p>Open this recently used model again. The File menu keeps the four most recent files.</p>"),
		""
	},

	// SW_FILE_EXIT
	{
		QT_TRANSLATE_NOOP("Action", "File.Exit"),
		QT_TRANSLATE_NOOP("Menu", "E&xit"),
		QT_TRANSLATE_NOOP("Status", "Quit the application; prompts to save model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Exit</b></p><p>Close StudWorx. Each model with unsaved changes asks to be saved first.</p>"),
		"Ctrl+Q"
	},

	// SW_EDIT_UNDO
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Undo"),
		QT_TRANSLATE_NOOP("Menu", "&Undo"),
		QT_TRANSLATE_NOOP("Status", "Undo the last action"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Undo</b></p><p>Reverse the most recent model edit, such as inserting, moving, painting, or deleting an object. Repeat Undo to move backward through the edit history.</p>"),
		"Ctrl+Z"
	},

	// SW_EDIT_REDO
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Redo"),
		QT_TRANSLATE_NOOP("Menu", "&Redo"),
		QT_TRANSLATE_NOOP("Status", "Redo the previously undone action"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Redo</b></p><p>Reapply an edit that was reversed with Undo. Making a new edit after Undo replaces the redo history with the new sequence of edits.</p>"),
		"Ctrl+Shift+Z"
	},

	// SW_EDIT_CUT
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Cut"),
		QT_TRANSLATE_NOOP("Menu", "Cu&t"),
		QT_TRANSLATE_NOOP("Status", "Cut the selection and put it on the Clipboard"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Cut</b></p><p>Remove the selected parts from the model and put them on the clipboard, to paste in this or another model.</p>"),
		"Ctrl+X"
	},

	// SW_EDIT_COPY
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Copy"),
		QT_TRANSLATE_NOOP("Menu", "&Copy"),
		QT_TRANSLATE_NOOP("Status", "Copy the selection and put it on the Clipboard"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Copy</b></p><p>Put a copy of the selected parts on the clipboard, to paste in this or another model.</p>"),
		"Ctrl+C"
	},

	// SW_EDIT_PASTE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Paste"),
		QT_TRANSLATE_NOOP("Menu", "&Paste"),
		QT_TRANSLATE_NOOP("Status", "Insert Clipboard contents in the current step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Paste</b></p><p>Insert the parts on the clipboard in the current step and select them.</p>"),
		"Ctrl+V"
	},

	// SW_EDIT_PASTE_STEPS
	{
		QT_TRANSLATE_NOOP("Action", "Edit.PasteSteps"),
		QT_TRANSLATE_NOOP("Menu", "Paste with Steps"),
		QT_TRANSLATE_NOOP("Status", "Insert Clipboard contents in their original steps"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Paste with Steps</b></p><p>Insert the parts on the clipboard in the steps they were copied from, and select the ones shown at the current step.</p>"),
		"Ctrl+Shift+V"
	},

	// SW_EDIT_FIND
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Find"),
		QT_TRANSLATE_NOOP("Menu", "&Find"),
		QT_TRANSLATE_NOOP("Status", "Find part"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Find</b></p><p>Show the Find and Replace bar over the view to find parts by part, color or both.</p>"),
		"Ctrl+F"
	},

	// SW_EDIT_FIND_NEXT
	{
		QT_TRANSLATE_NOOP("Action", "Edit.FindNext"),
		QT_TRANSLATE_NOOP("Menu", "Find Ne&xt"),
		QT_TRANSLATE_NOOP("Status", "Find next part"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Find Next</b></p><p>Select the next part that matches the Find and Replace bar.</p>"),
		""
	},

	// SW_EDIT_FIND_PREVIOUS
	{
		QT_TRANSLATE_NOOP("Action", "Edit.FindPrevious"),
		QT_TRANSLATE_NOOP("Menu", "Find Pre&vious"),
		QT_TRANSLATE_NOOP("Status", "Find previous part"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Find Previous</b></p><p>Select the previous part that matches the Find and Replace bar.</p>"),
		""
	},

	// SW_EDIT_FIND_ALL
	{
		QT_TRANSLATE_NOOP("Action", "Edit.FindAll"),
		QT_TRANSLATE_NOOP("Menu", "Find All"),
		QT_TRANSLATE_NOOP("Status", "Find all parts that match the search criteria"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Find All</b></p><p>Select every part that matches the Find and Replace bar.</p>"),
		"Ctrl+Alt+F"
	},

	// SW_EDIT_REPLACE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Replace"),
		QT_TRANSLATE_NOOP("Menu", "Rep&lace"),
		QT_TRANSLATE_NOOP("Status", "Replace part"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Replace</b></p><p>Show the Find and Replace bar with its replace fields, to change the part or color of matching parts.</p>"),
		""
	},

	// SW_EDIT_REPLACE_NEXT
	{
		QT_TRANSLATE_NOOP("Action", "Edit.ReplaceNext"),
		QT_TRANSLATE_NOOP("Menu", "Replace Next"),
		QT_TRANSLATE_NOOP("Status", "Replace next part"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Replace Next</b></p><p>Replace the part or color of the next matching part.</p>"),
		""
	},

	// SW_EDIT_REPLACE_ALL
	{
		QT_TRANSLATE_NOOP("Action", "Edit.ReplaceAll"),
		QT_TRANSLATE_NOOP("Menu", "Replace All"),
		QT_TRANSLATE_NOOP("Status", "Replace all parts that match the search criteria"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Replace All</b></p><p>Replace the part or color of every matching part. With Replace in Submodels it reaches every submodel the model uses.</p>"),
		""
	},

	// SW_EDIT_SELECT_ALL
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SelectAll"),
		QT_TRANSLATE_NOOP("Menu", "Select &All"),
		QT_TRANSLATE_NOOP("Status", "Select all parts in the model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Select All</b></p><p>Select every visible part of the current model.</p>"),
		"Ctrl+A"
	},

	// SW_EDIT_SELECT_NONE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SelectNone"),
		QT_TRANSLATE_NOOP("Menu", "Select &None"),
		QT_TRANSLATE_NOOP("Status", "Deselect everything"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Select None</b></p><p>Clear the selection.</p>"),
		""
	},

	// SW_EDIT_SELECT_INVERT
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SelectInvert"),
		QT_TRANSLATE_NOOP("Menu", "&Invert Selection"),
		QT_TRANSLATE_NOOP("Status", "Invert the current selection set"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Invert Selection</b></p><p>Select the parts that are not selected and clear the ones that are.</p>"),
		"Ctrl+I"
	},

	// SW_EDIT_SELECT_ADJACENT
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SelectAdjacent"),
		QT_TRANSLATE_NOOP("Menu", "Select A&djacent Parts"),
		QT_TRANSLATE_NOOP("Status", "Add the parts connected directly to the selected parts to the selection"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Select Adjacent Parts</b></p><p>Add every visible part connected directly to a selected part by a stud, anti stud, hole, axle or bar. Repeat to grow the selection one part further each time. Connections come from the LDCad shadow library set as Part Connections in Preferences, parts without shadow data never connect.</p>"),
		"Ctrl+Alt+Shift+A"
	},

	// SW_EDIT_SELECT_CONNECTED
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SelectConnected"),
		QT_TRANSLATE_NOOP("Menu", "Select Conn&ected Parts"),
		QT_TRANSLATE_NOOP("Status", "Add all parts connected to the selected parts to the selection"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Select Connected Parts</b></p><p>Add every visible part connected to a selected part, directly or through other parts, by a stud, anti stud, hole, axle or bar. Use it to pick up a whole assembly from one of its parts. Connections come from the LDCad shadow library set as Part Connections in Preferences, parts without shadow data never connect.</p>"),
		"Ctrl+Alt+Shift+S"
	},

	// SW_EDIT_TOGGLE_SHADOW_LIBRARY
	{
		QT_TRANSLATE_NOOP("Action", "Edit.ToggleShadowLibrary"),
		QT_TRANSLATE_NOOP("Menu", "Part Connections"),
		QT_TRANSLATE_NOOP("Status", "Toggle finding connected parts with the LDCad shadow library"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Part Connections</b></p><p>Use the connection shapes of the LDCad shadow library to find connected parts, for Select Connected Parts, joints and Explode. The shadow library folder is chosen in Preferences.</p>"),
		"Ctrl+Alt+Shift+C"
	},

	// SW_EDIT_SELECT_BY_NAME
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SelectByName"),
		QT_TRANSLATE_NOOP("Menu", "Select by Na&me..."),
		QT_TRANSLATE_NOOP("Status", "Select objects by name"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Select by Name</b></p><p>Open a checklist of every object in the model and select them by name, with All, None and Invert.</p>"),
		""
	},

	// SW_EDIT_SELECT_SINGLE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SelectSingle"),
		QT_TRANSLATE_NOOP("Menu", "Single Selection"),
		QT_TRANSLATE_NOOP("Status", "Select one part at a time"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Single Selection</b></p><p>A click selects only the part under the pointer.</p>"),
		"Ctrl+Shift+S"
	},

	// SW_EDIT_SELECT_PART
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SelectPart"),
		QT_TRANSLATE_NOOP("Menu", "Part Selection"),
		QT_TRANSLATE_NOOP("Status", "Select all parts of the same type"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Part Selection</b></p><p>A click selects every part of the same type as the one under the pointer.</p>"),
		"Ctrl+Shift+P"
	},

	// SW_EDIT_SELECT_COLOR
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SelectColor"),
		QT_TRANSLATE_NOOP("Menu", "Color Selection"),
		QT_TRANSLATE_NOOP("Status", "Select all parts of the same color"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Color Selection</b></p><p>A click selects every part of the same color as the one under the pointer.</p>"),
		"Ctrl+Shift+C"
	},

	// SW_EDIT_SELECT_PART_COLOR
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SelectPartColor"),
		QT_TRANSLATE_NOOP("Menu", "Part and Color Selection"),
		QT_TRANSLATE_NOOP("Status", "Select all parts of the same type and color"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Part and Color Selection</b></p><p>A click selects every part of the same type and color as the one under the pointer.</p>"),
		"Ctrl+Shift+A"
	},

	// SW_EDIT_TRANSFORM_RELATIVE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.TransformRelative"),
		QT_TRANSLATE_NOOP("Menu", "Relative Transforms"),
		QT_TRANSLATE_NOOP("Status", "Move and rotate objects relative to the one that has focus"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Relative Transforms</b></p><p>Move and rotate the selection along the axes of the part that has focus.</p>"),
		"-"
	},

	// SW_EDIT_TRANSFORM_ABSOLUTE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.TransformAbsolute"),
		QT_TRANSLATE_NOOP("Menu", "Absolute Transforms"),
		QT_TRANSLATE_NOOP("Status", "Move and rotate objects in absolute coordinates"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Absolute Transforms</b></p><p>Move and rotate the selection along the model axes.</p>"),
		"+"
	},

	// SW_EDIT_TRANSFORM_TOGGLE_RELATIVE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.TransformToggleRelative"),
		QT_TRANSLATE_NOOP("Menu", "Toggle Relative Transforms"),
		QT_TRANSLATE_NOOP("Status", "Toggle moving and rotating objects relative to the one that has focus"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Toggle Relative Transforms</b></p><p>Switch between relative and absolute transforms.</p>"),
		"/"
	},

	// SW_EDIT_TRANSFORM_SEPARATELY
	{
		QT_TRANSLATE_NOOP("Action", "Edit.TransformSeparately"),
		QT_TRANSLATE_NOOP("Menu", "Rotate Separately"),
		QT_TRANSLATE_NOOP("Status", "Rotate selected parts separately"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotate Separately</b></p><p>Turn each selected part around its own pivot point.</p>"),
		"Ctrl+-"
	},

	// SW_EDIT_TRANSFORM_TOGETHER
	{
		QT_TRANSLATE_NOOP("Action", "Edit.TransformTogether"),
		QT_TRANSLATE_NOOP("Menu", "Rotate Together"),
		QT_TRANSLATE_NOOP("Status", "Rotate selected parts together"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotate Together</b></p><p>Turn the selection as one around the pivot of the part that has focus.</p>"),
		"Ctrl+="
	},

	// SW_EDIT_TRANSFORM_TOGGLE_SEPARATE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.TransformToggleSeparate"),
		QT_TRANSLATE_NOOP("Menu", "Toggle Separate Transforms"),
		QT_TRANSLATE_NOOP("Status", "Toggle rotating selected parts separately"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Toggle Separate Transforms</b></p><p>Switch between rotating the selected parts separately and together.</p>"),
		"Ctrl+T"
	},

	// SW_EDIT_SNAP_MOVE_TOGGLE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Snap.Toggle"),
		QT_TRANSLATE_NOOP("Menu", "Move Snap"),
		QT_TRANSLATE_NOOP("Status", "Toggle movement snapping"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move Snap</b></p><p>Turn move snapping on or off. With it on, moves go in the horizontal and vertical steps chosen below.</p>"),
		"Ctrl+Shift+M"
	},

	// SW_EDIT_SNAP_MOVE_XY0
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.XY0"),
		QT_TRANSLATE_NOOP("Menu", "No XY Snap"),
		QT_TRANSLATE_NOOP("Status", "Do not snap movement along the XY plane"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Horizontal Move Snap</b></p><p>Move parts across the horizontal plane in this step, or freely with No XY Snap.</p>"),
		"0"
	},

	// SW_EDIT_SNAP_MOVE_XY1
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.XY1"),
		QT_TRANSLATE_NOOP("Menu", "XY 1/20 Stud"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the XY plane to 1/20 stud"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Horizontal Move Snap</b></p><p>Move parts across the horizontal plane in this step, or freely with No XY Snap.</p>"),
		"1"
	},

	// SW_EDIT_SNAP_MOVE_XY2
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.XY2"),
		QT_TRANSLATE_NOOP("Menu", "XY 1/4 Stud"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the XY plane to 1/4 stud"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Horizontal Move Snap</b></p><p>Move parts across the horizontal plane in this step, or freely with No XY Snap.</p>"),
		"2"
	},

	// SW_EDIT_SNAP_MOVE_XY3
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.XY3"),
		QT_TRANSLATE_NOOP("Menu", "XY 1 Flat"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the XY plane to 1 flat"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Horizontal Move Snap</b></p><p>Move parts across the horizontal plane in this step, or freely with No XY Snap.</p>"),
		"3"
	},

	// SW_EDIT_SNAP_MOVE_XY4
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.XY4"),
		QT_TRANSLATE_NOOP("Menu", "XY 1/2 Stud"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the XY plane to 1/2 stud"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Horizontal Move Snap</b></p><p>Move parts across the horizontal plane in this step, or freely with No XY Snap.</p>"),
		"4"
	},

	// SW_EDIT_SNAP_MOVE_XY5
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.XY5"),
		QT_TRANSLATE_NOOP("Menu", "XY 1 Stud"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the XY plane to 1 stud"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Horizontal Move Snap</b></p><p>Move parts across the horizontal plane in this step, or freely with No XY Snap.</p>"),
		"5"
	},

	// SW_EDIT_SNAP_MOVE_XY6
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.XY6"),
		QT_TRANSLATE_NOOP("Menu", "XY 2 Studs"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the XY plane to 2 studs"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Horizontal Move Snap</b></p><p>Move parts across the horizontal plane in this step, or freely with No XY Snap.</p>"),
		"6"
	},

	// SW_EDIT_SNAP_MOVE_XY7
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.XY7"),
		QT_TRANSLATE_NOOP("Menu", "XY 3 Studs"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the XY plane to 3 studs"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Horizontal Move Snap</b></p><p>Move parts across the horizontal plane in this step, or freely with No XY Snap.</p>"),
		"7"
	},

	// SW_EDIT_SNAP_MOVE_XY8
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.XY8"),
		QT_TRANSLATE_NOOP("Menu", "XY 4 Studs"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the XY plane to 4 studs"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Horizontal Move Snap</b></p><p>Move parts across the horizontal plane in this step, or freely with No XY Snap.</p>"),
		"8"
	},

	// SW_EDIT_SNAP_MOVE_XY9
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.XY9"),
		QT_TRANSLATE_NOOP("Menu", "XY 8 Studs"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the XY plane to 8 studs"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Horizontal Move Snap</b></p><p>Move parts across the horizontal plane in this step, or freely with No XY Snap.</p>"),
		"9"
	},

	// SW_EDIT_SNAP_MOVE_Z0
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.Z0"),
		QT_TRANSLATE_NOOP("Menu", "No Z Snap"),
		QT_TRANSLATE_NOOP("Status", "Do not snap movement along the Z axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Vertical Move Snap</b></p><p>Move parts up and down in this step, or freely with No Z Snap.</p>"),
		"Alt+#"
	},

	// SW_EDIT_SNAP_MOVE_Z1
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.Z1"),
		QT_TRANSLATE_NOOP("Menu", "Z 1/20 Stud"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the Z axis to 1/20 stud"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Vertical Move Snap</b></p><p>Move parts up and down in this step, or freely with No Z Snap.</p>"),
		"Alt+1"
	},

	// SW_EDIT_SNAP_MOVE_Z2
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.Z2"),
		QT_TRANSLATE_NOOP("Menu", "Z 1/4 Stud"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the Z axis to 1/4 stud"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Vertical Move Snap</b></p><p>Move parts up and down in this step, or freely with No Z Snap.</p>"),
		"Alt+2"
	},

	// SW_EDIT_SNAP_MOVE_Z3
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.Z3"),
		QT_TRANSLATE_NOOP("Menu", "Z 1 Flat"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the Z axis to 1 flat"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Vertical Move Snap</b></p><p>Move parts up and down in this step, or freely with No Z Snap.</p>"),
		"Alt+3"
	},

	// SW_EDIT_SNAP_MOVE_Z4
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.Z4"),
		QT_TRANSLATE_NOOP("Menu", "Z 1/2 Stud"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the Z axis to 1/2 stud"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Vertical Move Snap</b></p><p>Move parts up and down in this step, or freely with No Z Snap.</p>"),
		"Alt+4"
	},

	// SW_EDIT_SNAP_MOVE_Z5
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.Z5"),
		QT_TRANSLATE_NOOP("Menu", "Z 1 Stud"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the Z axis to 1 stud"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Vertical Move Snap</b></p><p>Move parts up and down in this step, or freely with No Z Snap.</p>"),
		"Alt+5"
	},

	// SW_EDIT_SNAP_MOVE_Z6
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.Z6"),
		QT_TRANSLATE_NOOP("Menu", "Z 1 Brick"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the Z axis to 1 brick"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Vertical Move Snap</b></p><p>Move parts up and down in this step, or freely with No Z Snap.</p>"),
		"Alt+6"
	},

	// SW_EDIT_SNAP_MOVE_Z7
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.Z7"),
		QT_TRANSLATE_NOOP("Menu", "Z 2 Bricks"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the Z axis to 2 bricks"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Vertical Move Snap</b></p><p>Move parts up and down in this step, or freely with No Z Snap.</p>"),
		"Alt+7"
	},

	// SW_EDIT_SNAP_MOVE_Z8
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.Z8"),
		QT_TRANSLATE_NOOP("Menu", "Z 4 Bricks"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the Z axis to 4 bricks"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Vertical Move Snap</b></p><p>Move parts up and down in this step, or freely with No Z Snap.</p>"),
		"Alt+8"
	},

	// SW_EDIT_SNAP_MOVE_Z9
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapMove.Z9"),
		QT_TRANSLATE_NOOP("Menu", "Z 8 Bricks"),
		QT_TRANSLATE_NOOP("Status", "Snap movement along the Z axis to 8 bricks"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Vertical Move Snap</b></p><p>Move parts up and down in this step, or freely with No Z Snap.</p>"),
		"Alt+9"
	},

	// SW_EDIT_SNAP_ANGLE_TOGGLE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapAngle.Toggle"),
		QT_TRANSLATE_NOOP("Menu", "Rotation Snap"),
		QT_TRANSLATE_NOOP("Status", "Snap rotations to fixed intervals"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotation Snap</b></p><p>Turn rotation snapping on or off. With it on, rotations go in the angle chosen below.</p>"),
		"Alt+R"
	},

	// SW_EDIT_SNAP_ANGLE0
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapAngle.Angle0"),
		QT_TRANSLATE_NOOP("Menu", "No Rotation Snap"),
		QT_TRANSLATE_NOOP("Status", "Do not snap rotations"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotation Snap Angle</b></p><p>Rotate parts in steps of this angle, or freely with No Rotation Snap. Turns land exactly on square angles, so later moves keep whole coordinates.</p>"),
		"Ctrl+0"
	},

	// SW_EDIT_SNAP_ANGLE1
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapAngle.Angle1"),
		QT_TRANSLATE_NOOP("Menu", "1 Degree"),
		QT_TRANSLATE_NOOP("Status", "Snap rotations to 1 degree"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotation Snap Angle</b></p><p>Rotate parts in steps of this angle, or freely with No Rotation Snap. Turns land exactly on square angles, so later moves keep whole coordinates.</p>"),
		"Ctrl+1"
	},

	// SW_EDIT_SNAP_ANGLE2
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapAngle.Angle2"),
		QT_TRANSLATE_NOOP("Menu", "5 Degrees"),
		QT_TRANSLATE_NOOP("Status", "Snap rotations to 5 degrees"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotation Snap Angle</b></p><p>Rotate parts in steps of this angle, or freely with No Rotation Snap. Turns land exactly on square angles, so later moves keep whole coordinates.</p>"),
		"Ctrl+2"
	},

	// SW_EDIT_SNAP_ANGLE3
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapAngle.Angle3"),
		QT_TRANSLATE_NOOP("Menu", "15 Degrees"),
		QT_TRANSLATE_NOOP("Status", "Snap rotations to 15 degrees"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotation Snap Angle</b></p><p>Rotate parts in steps of this angle, or freely with No Rotation Snap. Turns land exactly on square angles, so later moves keep whole coordinates.</p>"),
		"Ctrl+3"
	},

	// SW_EDIT_SNAP_ANGLE4
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapAngle.Angle4"),
		QT_TRANSLATE_NOOP("Menu", "22.5 Degrees"),
		QT_TRANSLATE_NOOP("Status", "Snap rotations to 22.5 degrees"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotation Snap Angle</b></p><p>Rotate parts in steps of this angle, or freely with No Rotation Snap. Turns land exactly on square angles, so later moves keep whole coordinates.</p>"),
		"Ctrl+4"
	},

	// SW_EDIT_SNAP_ANGLE5
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapAngle.Angle5"),
		QT_TRANSLATE_NOOP("Menu", "30 Degrees"),
		QT_TRANSLATE_NOOP("Status", "Snap rotations to 30 degrees"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotation Snap Angle</b></p><p>Rotate parts in steps of this angle, or freely with No Rotation Snap. Turns land exactly on square angles, so later moves keep whole coordinates.</p>"),
		"Ctrl+5"
	},

	// SW_EDIT_SNAP_ANGLE6
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapAngle.Angle6"),
		QT_TRANSLATE_NOOP("Menu", "45 Degrees"),
		QT_TRANSLATE_NOOP("Status", "Snap rotations to 45 degrees"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotation Snap Angle</b></p><p>Rotate parts in steps of this angle, or freely with No Rotation Snap. Turns land exactly on square angles, so later moves keep whole coordinates.</p>"),
		"Ctrl+6"
	},

	// SW_EDIT_SNAP_ANGLE7
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapAngle.Angle7"),
		QT_TRANSLATE_NOOP("Menu", "60 Degrees"),
		QT_TRANSLATE_NOOP("Status", "Snap rotations to 60 degrees"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotation Snap Angle</b></p><p>Rotate parts in steps of this angle, or freely with No Rotation Snap. Turns land exactly on square angles, so later moves keep whole coordinates.</p>"),
		"Ctrl+7"
	},

	// SW_EDIT_SNAP_ANGLE8
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapAngle.Angle8"),
		QT_TRANSLATE_NOOP("Menu", "90 Degrees"),
		QT_TRANSLATE_NOOP("Status", "Snap rotations to 90 degrees"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotation Snap Angle</b></p><p>Rotate parts in steps of this angle, or freely with No Rotation Snap. Turns land exactly on square angles, so later moves keep whole coordinates.</p>"),
		"Ctrl+8"
	},

	// SW_EDIT_SNAP_ANGLE9
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapAngle.Angle9"),
		QT_TRANSLATE_NOOP("Menu", "180 Degrees"),
		QT_TRANSLATE_NOOP("Status", "Snap rotations to 180 degrees"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotation Snap Angle</b></p><p>Rotate parts in steps of this angle, or freely with No Rotation Snap. Turns land exactly on square angles, so later moves keep whole coordinates.</p>"),
		"Ctrl+9"
	},

	// SW_EDIT_SNAP_ANGLE_CUSTOM
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapAngle.Custom"),
		QT_TRANSLATE_NOOP("Menu", "Custom..."),
		QT_TRANSLATE_NOOP("Status", "Snap rotations to a custom angle"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Custom Angle</b></p><p>Type your own rotation snap angle in degrees.</p>"),
		""
	},

	// SW_EDIT_SNAP_ORBIT_TOGGLE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.SnapAngle.ToggleOrbit"),
		QT_TRANSLATE_NOOP("Menu", "Camera Snap"),
		QT_TRANSLATE_NOOP("Status", "Toggle orbiting the camera in steps of the rotation snap angle"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera Snap</b></p><p>Orbit the view in steps of the rotation snap angle.</p>"),
		""
	},

	// SW_EDIT_TRANSFORM
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Transform"),
		QT_TRANSLATE_NOOP("Menu", "Transform"),
		QT_TRANSLATE_NOOP("Status", "Apply transform to selected objects"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Transform</b></p><p>Apply the position or rotation typed in the Transform toolbar to the selection.</p>"),
		""
	},

	// SW_EDIT_TRANSFORM_ABSOLUTE_TRANSLATION
	{
		QT_TRANSLATE_NOOP("Action", "Edit.TransformAbsoluteTranslation"),
		QT_TRANSLATE_NOOP("Menu", "Absolute Translation"),
		QT_TRANSLATE_NOOP("Status", "Switch to absolute translation mode when applying transforms"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Absolute Translation</b></p><p>The Transform toolbar moves the selection to the typed position in model coordinates.</p>"),
		"Alt+Shift+T"
	},

	// SW_EDIT_TRANSFORM_RELATIVE_TRANSLATION
	{
		QT_TRANSLATE_NOOP("Action", "Edit.TransformRelativeTranslation"),
		QT_TRANSLATE_NOOP("Menu", "Relative Translation"),
		QT_TRANSLATE_NOOP("Status", "Switch to relative translation mode when applying transforms"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Relative Translation</b></p><p>The Transform toolbar moves the selection by the typed distance.</p>"),
		"Ctrl+Alt+T"
	},

	// SW_EDIT_TRANSFORM_ABSOLUTE_ROTATION
	{
		QT_TRANSLATE_NOOP("Action", "Edit.TransformAbsoluteRotation"),
		QT_TRANSLATE_NOOP("Menu", "Absolute Rotation"),
		QT_TRANSLATE_NOOP("Status", "Switch to absolute rotation mode when applying transforms"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Absolute Rotation</b></p><p>The Transform toolbar turns the selection to the typed angles.</p>"),
		"Alt+Shift+R"
	},

	// SW_EDIT_TRANSFORM_RELATIVE_ROTATION
	{
		QT_TRANSLATE_NOOP("Action", "Edit.TransformRelativeRotation"),
		QT_TRANSLATE_NOOP("Menu", "Relative Rotation"),
		QT_TRANSLATE_NOOP("Status", "Switch to relative rotation mode when applying transforms"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Relative Rotation</b></p><p>The Transform toolbar turns the selection by the typed angles.</p>"),
		"Ctrl+Alt+R"
	},

	// SW_EDIT_ACTION_INSERT
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.Insert"),
		QT_TRANSLATE_NOOP("Menu", "Insert"),
		QT_TRANSLATE_NOOP("Status", "Add new parts to the model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Insert Part Tool</b></p><p>Choose a part in the Parts dock and a color in Colors, then use this tool in the model view to place the part. Movement snapping controls placement increments. Use the preview to check placement before inserting the part.</p>"),
		""
	},

	// SW_EDIT_ACTION_POINT_LIGHT
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.PointLight"),
		QT_TRANSLATE_NOOP("Menu", "Point Light"),
		QT_TRANSLATE_NOOP("Status", "Add new point light sources to the model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Point Light Tool</b></p><p>Use the model view to place a light that emits in all directions from a point. Select the light afterward to edit its color, shadow settings, and renderer-specific power in Properties.</p>"),
		""
	},

	// SW_EDIT_ACTION_SPOTLIGHT
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.SpotLight"),
		QT_TRANSLATE_NOOP("Menu", "Spotlight"),
		QT_TRANSLATE_NOOP("Status", "Add new spotlights to the model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Spotlight Tool</b></p><p>Use the model view to create a directed cone of light. In Properties, adjust its position and target, cone angle, penumbra, and renderer-specific power.</p>"),
		""
	},

	// SW_EDIT_ACTION_DIRECTIONAL_LIGHT
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.DirectionalLight"),
		QT_TRANSLATE_NOOP("Menu", "Directional Light"),
		QT_TRANSLATE_NOOP("Status", "Add new directional light sources to the model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Directional Light Tool</b></p><p>Create a light with parallel rays, such as sunlight. Its direction determines the illumination. Select it to adjust its transform, color, and renderer-specific settings in Properties.</p>"),
		""
	},

	// SW_EDIT_ACTION_AREA_LIGHT
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.AreaLight"),
		QT_TRANSLATE_NOOP("Menu", "Area Light"),
		QT_TRANSLATE_NOOP("Status", "Add new area light sources to the model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Area Light Tool</b></p><p>Create a light that emits over an area. Select it to set the area shape and size in Properties, along with color and renderer-specific settings.</p>"),
		""
	},

	// SW_EDIT_ACTION_CAMERA
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.Camera"),
		QT_TRANSLATE_NOOP("Menu", "Camera"),
		QT_TRANSLATE_NOOP("Status", "Create a new camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera Tool</b></p><p>Create a camera in the model view. Select the camera to edit its position, target, projection, and field of view in Properties. To look through the camera, choose it from View &gt; Cameras.</p>"),
		"Ctrl+Alt+C"
	},

	// SW_EDIT_ACTION_SELECT
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.Select"),
		QT_TRANSLATE_NOOP("Menu", "Select"),
		QT_TRANSLATE_NOOP("Status", "Select objects (hold the CTRL key down or drag the mouse to select multiple objects)"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Select Tool</b></p><p>Pick objects in the model view or drag a selection rectangle. Selection Mode controls whether matching part types or colors are included. The focused object provides the reference for relative transforms.</p>"),
		"Shift+S"
	},

	// SW_EDIT_ACTION_MOVE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.Move"),
		QT_TRANSLATE_NOOP("Menu", "Move"),
		QT_TRANSLATE_NOOP("Status", "Move selected objects"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move Tool</b></p><p>Select the objects to move, then drag in the model view. Use the manipulator to constrain movement to an axis or plane. Movement Snap sets the increments, and Transform Options selects the coordinate reference.</p>"),
		"Shift+M"
	},

	// SW_EDIT_ACTION_ROTATE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.Rotate"),
		QT_TRANSLATE_NOOP("Menu", "Rotate"),
		QT_TRANSLATE_NOOP("Status", "Rotate selected parts"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotate Tool</b></p><p>Select objects, then drag a rotation handle in the model view. Rotation Snap sets the angle increments. Transform Options chooses relative or model axes and whether parts rotate together or separately.</p>"),
		"Shift+R"
	},

	// SW_EDIT_ACTION_ERASE
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.Erase"),
		QT_TRANSLATE_NOOP("Menu", "Erase"),
		QT_TRANSLATE_NOOP("Status", "Erase objects"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Erase Tool</b></p><p>Use this tool to remove objects by clicking them in the model view. This is different from deleting the current selection as a single command. Undo restores an object removed by the tool.</p>"),
		"Shift+E"
	},

	// SW_EDIT_ACTION_PAINT
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.Paint"),
		QT_TRANSLATE_NOOP("Menu", "Paint"),
		QT_TRANSLATE_NOOP("Status", "Change part color"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Paint Tool</b></p><p>Choose the replacement color in the Colors dock, then click parts in the model view to recolor them. To recolor the current selection together, use the color button above the swatches.</p>"),
		"P"
	},

	// SW_EDIT_ACTION_JOINTS
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.Joints"),
		QT_TRANSLATE_NOOP("Menu", "Joints"),
		QT_TRANSLATE_NOOP("Status", "Turn a joint"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Joints Tool</b></p><p>Drag a submodel that has a joint to turn it. Hinges turn about their axis with the horizontal drag, balls turn about the screen up and right directions. Rotation Snap sets the steps and one drag is one undo step. Use Part &gt; Joint &gt; Set Joint to give a submodel a joint.</p>"),
		"Shift+J"
	},

	// SW_EDIT_ACTION_COLOR_PICKER
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.ColorPicker"),
		QT_TRANSLATE_NOOP("Menu", "Color Picker"),
		QT_TRANSLATE_NOOP("Status", "Get part color"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Color Picker Tool</b></p><p>Click a part in the model view to make its color the current color. You can then use that color for insertion or painting; sampling it does not recolor the part.</p>"),
		"C"
	},

	// SW_EDIT_ACTION_ZOOM
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.Zoom"),
		QT_TRANSLATE_NOOP("Menu", "Zoom"),
		QT_TRANSLATE_NOOP("Status", "Zoom in or out"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Zoom Tool</b></p><p>Drag in the model view to move the viewpoint closer to or farther from the model. This changes the view rather than resizing parts. Use Zoom Region to frame a particular area.</p>"),
		"Z"
	},

	// SW_EDIT_ACTION_PAN
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.Pan"),
		QT_TRANSLATE_NOOP("Menu", "Pan"),
		QT_TRANSLATE_NOOP("Status", "Pan the current view"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Pan Tool</b></p><p>Drag to shift the view sideways or vertically without moving the model. Use this to reposition the model on screen while keeping the viewing direction.</p>"),
		"Shift+P"
	},

	// SW_EDIT_ACTION_ROTATE_VIEW
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.RotateView"),
		QT_TRANSLATE_NOOP("Menu", "Rotate View"),
		QT_TRANSLATE_NOOP("Status", "Rotate the current view"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Orbit Tool</b></p><p>Drag to orbit the view around its center and inspect the model from another direction. This rotates the viewpoint rather than the selected parts; use Rotate for object edits.</p>"),
		"Shift+V"
	},

	// SW_EDIT_ACTION_ROLL
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.Roll"),
		QT_TRANSLATE_NOOP("Menu", "Roll"),
		QT_TRANSLATE_NOOP("Status", "Roll the current view"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Roll Tool</b></p><p>Drag to turn the view around its viewing axis, tilting what appears upright on screen. Model parts keep their positions and orientations.</p>"),
		"Shift+L"
	},

	// SW_EDIT_ACTION_ZOOM_REGION
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Tool.ZoomRegion"),
		QT_TRANSLATE_NOOP("Menu", "Zoom Region"),
		QT_TRANSLATE_NOOP("Status", "Zoom into a region of the screen"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Zoom Region Tool</b></p><p>Drag a rectangle around the area you want to inspect. The view zooms to frame that region without changing model geometry.</p>"),
		"Shift+Z"
	},

	// SW_EDIT_CANCEL
	{
		QT_TRANSLATE_NOOP("Action", "Edit.Cancel"),
		QT_TRANSLATE_NOOP("Menu", "Cancel Action"),
		QT_TRANSLATE_NOOP("Status", "Cancel current mouse action"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Cancel Action</b></p><p>Cancel the current mouse action, a face or cutaway pick or a cutaway resize, close the Find and Replace bar, or clear the selection. With nothing selected it removes the cutaway.</p>"),
		"Esc"
	},

	// SW_VIEW_PREFERENCES
	{
		QT_TRANSLATE_NOOP("Action", "View.Preferences"),
		QT_TRANSLATE_NOOP("Menu", "Preference&s..."),
		QT_TRANSLATE_NOOP("Status", "Change program settings"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Preferences</b></p><p>Open the program settings: General, Rendering, Views, Interface, Instructions, Categories, Context Menu, Keyboard and Mouse.</p>"),
		"Ctrl+Alt+Shift+P"
	},

	// SW_VIEW_ZOOM_IN
	{
		QT_TRANSLATE_NOOP("Action", "View.ZoomIn"),
		QT_TRANSLATE_NOOP("Menu", "Zoom In"),
		QT_TRANSLATE_NOOP("Status", "Zoom in"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Zoom In</b></p><p>Move the camera closer to the model.</p>"),
		""
	},

	// SW_VIEW_ZOOM_OUT
	{
		QT_TRANSLATE_NOOP("Action", "View.ZoomOut"),
		QT_TRANSLATE_NOOP("Menu", "Zoom Out"),
		QT_TRANSLATE_NOOP("Status", "Zoom out"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Zoom Out</b></p><p>Move the camera away from the model.</p>"),
		""
	},

	// SW_VIEW_ZOOM_EXTENTS
	{
		QT_TRANSLATE_NOOP("Action", "View.ZoomExtents"),
		QT_TRANSLATE_NOOP("Menu", "Zoom E&xtents"),
		QT_TRANSLATE_NOOP("Status", "Fit all parts in the current view"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Zoom Extents</b></p><p>Move the camera until every part fits in the view.</p>"),
		"#"
	},

	// SW_VIEW_LOOK_AT
	{
		QT_TRANSLATE_NOOP("Action", "View.LookAt"),
		QT_TRANSLATE_NOOP("Menu", "Look At"),
		QT_TRANSLATE_NOOP("Status", "Rotate view so selected parts are at center"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Look At</b></p><p>Turn the view so the selected parts are at its center.</p>"),
		""
	},

	// SW_VIEW_MOVE_FORWARD
	{
		QT_TRANSLATE_NOOP("Action", "View.MoveForward"),
		QT_TRANSLATE_NOOP("Menu", "Move Forward"),
		QT_TRANSLATE_NOOP("Status", "Move the current view forward"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move View</b></p><p>Move the camera of the current view, keeping its direction, from the keyboard or a gamepad.</p>"),
		""
	},

	// SW_VIEW_MOVE_BACKWARD
	{
		QT_TRANSLATE_NOOP("Action", "View.MoveBackward"),
		QT_TRANSLATE_NOOP("Menu", "Move Backward"),
		QT_TRANSLATE_NOOP("Status", "Move the current view backward"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move View</b></p><p>Move the camera of the current view, keeping its direction, from the keyboard or a gamepad.</p>"),
		""
	},

	// SW_VIEW_MOVE_LEFT
	{
		QT_TRANSLATE_NOOP("Action", "View.MoveLeft"),
		QT_TRANSLATE_NOOP("Menu", "Move Left"),
		QT_TRANSLATE_NOOP("Status", "Move the current view to the left"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move View</b></p><p>Move the camera of the current view, keeping its direction, from the keyboard or a gamepad.</p>"),
		"Ctrl+Alt+Left"
	},

	// SW_VIEW_MOVE_RIGHT
	{
		QT_TRANSLATE_NOOP("Action", "View.MoveRight"),
		QT_TRANSLATE_NOOP("Menu", "Move Right"),
		QT_TRANSLATE_NOOP("Status", "Move the current view to the right"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move View</b></p><p>Move the camera of the current view, keeping its direction, from the keyboard or a gamepad.</p>"),
		"Ctrl+Alt+Right"
	},

	// SW_VIEW_MOVE_UP
	{
		QT_TRANSLATE_NOOP("Action", "View.MoveUp"),
		QT_TRANSLATE_NOOP("Menu", "Move Up"),
		QT_TRANSLATE_NOOP("Status", "Move the current view up"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move View</b></p><p>Move the camera of the current view, keeping its direction, from the keyboard or a gamepad.</p>"),
		"Ctrl+Alt+Up"
	},

	// SW_VIEW_MOVE_DOWN
	{
		QT_TRANSLATE_NOOP("Action", "View.MoveDown"),
		QT_TRANSLATE_NOOP("Menu", "Move Down"),
		QT_TRANSLATE_NOOP("Status", "Move the current view down"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move View</b></p><p>Move the camera of the current view, keeping its direction, from the keyboard or a gamepad.</p>"),
		"Ctrl+Alt+Down"
	},

	// SW_VIEW_VIEWPOINT_FRONT
	{
		QT_TRANSLATE_NOOP("Action", "View.Viewpoint.Front"),
		QT_TRANSLATE_NOOP("Menu", "&Front"),
		QT_TRANSLATE_NOOP("Status", "View model from the front"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Viewpoint</b></p><p>Look at the model from this side, or from the default angle with Home.</p>"),
		"F"
	},

	// SW_VIEW_VIEWPOINT_BACK
	{
		QT_TRANSLATE_NOOP("Action", "View.Viewpoint.Back"),
		QT_TRANSLATE_NOOP("Menu", "&Back"),
		QT_TRANSLATE_NOOP("Status", "View model from the back"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Viewpoint</b></p><p>Look at the model from this side, or from the default angle with Home.</p>"),
		"B"
	},

	// SW_VIEW_VIEWPOINT_TOP
	{
		QT_TRANSLATE_NOOP("Action", "View.Viewpoint.Top"),
		QT_TRANSLATE_NOOP("Menu", "&Top"),
		QT_TRANSLATE_NOOP("Status", "View model from the top"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Viewpoint</b></p><p>Look at the model from this side, or from the default angle with Home.</p>"),
		"T"
	},

	// SW_VIEW_VIEWPOINT_BOTTOM
	{
		QT_TRANSLATE_NOOP("Action", "View.Viewpoint.Bottom"),
		QT_TRANSLATE_NOOP("Menu", "B&ottom"),
		QT_TRANSLATE_NOOP("Status", "View model from the bottom"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Viewpoint</b></p><p>Look at the model from this side, or from the default angle with Home.</p>"),
		"O"
	},

	// SW_VIEW_VIEWPOINT_LEFT
	{
		QT_TRANSLATE_NOOP("Action", "View.Viewpoint.Left"),
		QT_TRANSLATE_NOOP("Menu", "&Left"),
		QT_TRANSLATE_NOOP("Status", "View model from the left"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Viewpoint</b></p><p>Look at the model from this side, or from the default angle with Home.</p>"),
		"L"
	},

	// SW_VIEW_VIEWPOINT_RIGHT
	{
		QT_TRANSLATE_NOOP("Action", "View.Viewpoint.Right"),
		QT_TRANSLATE_NOOP("Menu", "&Right"),
		QT_TRANSLATE_NOOP("Status", "View model from the right"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Viewpoint</b></p><p>Look at the model from this side, or from the default angle with Home.</p>"),
		"R"
	},

	// SW_VIEW_VIEWPOINT_HOME
	{
		QT_TRANSLATE_NOOP("Action", "View.Viewpoint.Home"),
		QT_TRANSLATE_NOOP("Menu", "&Home"),
		QT_TRANSLATE_NOOP("Status", "View model from the default position"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Viewpoint</b></p><p>Look at the model from this side, or from the default angle with Home.</p>"),
		"H"
	},

	// SW_VIEW_CAMERA_NONE
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.None"),
		QT_TRANSLATE_NOOP("Menu", "No Camera"),
		QT_TRANSLATE_NOOP("Status", "Do not use a camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>No Camera</b></p><p>Leave the model's cameras and use the free view.</p>"),
		""
	},

	// SW_VIEW_CAMERA1
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera01"),
		QT_TRANSLATE_NOOP("Menu", "Camera 1"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA2
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera02"),
		QT_TRANSLATE_NOOP("Menu", "Camera 2"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA3
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera03"),
		QT_TRANSLATE_NOOP("Menu", "Camera 3"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA4
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera04"),
		QT_TRANSLATE_NOOP("Menu", "Camera 4"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA5
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera05"),
		QT_TRANSLATE_NOOP("Menu", "Camera 5"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA6
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera06"),
		QT_TRANSLATE_NOOP("Menu", "Camera 6"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA7
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera07"),
		QT_TRANSLATE_NOOP("Menu", "Camera 7"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA8
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera08"),
		QT_TRANSLATE_NOOP("Menu", "Camera 8"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA9
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera09"),
		QT_TRANSLATE_NOOP("Menu", "Camera 9"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA10
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera10"),
		QT_TRANSLATE_NOOP("Menu", "Camera 10"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA11
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera11"),
		QT_TRANSLATE_NOOP("Menu", "Camera 11"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA12
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera12"),
		QT_TRANSLATE_NOOP("Menu", "Camera 12"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA13
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera13"),
		QT_TRANSLATE_NOOP("Menu", "Camera 13"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA14
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera14"),
		QT_TRANSLATE_NOOP("Menu", "Camera 14"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA15
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera15"),
		QT_TRANSLATE_NOOP("Menu", "Camera 15"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_CAMERA16
	{
		QT_TRANSLATE_NOOP("Action", "View.Cameras.Camera16"),
		QT_TRANSLATE_NOOP("Menu", "Camera 16"),
		QT_TRANSLATE_NOOP("Status", "Use this camera"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Camera</b></p><p>Look through this camera of the model. The menu lists the first sixteen cameras.</p>"),
		""
	},

	// SW_VIEW_TIME_FIRST
	{
		QT_TRANSLATE_NOOP("Action", "View.Time.First"),
		QT_TRANSLATE_NOOP("Menu", "First"),
		QT_TRANSLATE_NOOP("Status", "Go to the first step of the model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>First Step</b></p><p>Display the model at its first building step. Step navigation changes which parts are visible; it does not move parts to a different step.</p>"),
		""
	},

	// SW_VIEW_TIME_PREVIOUS
	{
		QT_TRANSLATE_NOOP("Action", "View.Time.Previous"),
		QT_TRANSLATE_NOOP("Menu", "Previous"),
		QT_TRANSLATE_NOOP("Status", "Go to the previous step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Previous Step</b></p><p>Display the model one building step earlier. Parts introduced in later steps are hidden so you can inspect the earlier stage of construction.</p>"),
		"Ctrl+E"
	},

	// SW_VIEW_TIME_NEXT
	{
		QT_TRANSLATE_NOOP("Action", "View.Time.Next"),
		QT_TRANSLATE_NOOP("Menu", "Next"),
		QT_TRANSLATE_NOOP("Status", "Go to the next step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Next Step</b></p><p>Display the next building step to reveal the next stage of the model. Use the Timeline to change which step introduces a part.</p>"),
		"Ctrl+N"
	},

	// SW_VIEW_TIME_LAST
	{
		QT_TRANSLATE_NOOP("Action", "View.Time.Last"),
		QT_TRANSLATE_NOOP("Menu", "Last"),
		QT_TRANSLATE_NOOP("Status", "Go to the last step of the model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Last Step</b></p><p>Display the last building step of the current model. Use this to return to the completed model after inspecting earlier steps.</p>"),
		""
	},

	// SW_VIEW_TIME_INSERT_BEFORE
	{
		QT_TRANSLATE_NOOP("Action", "View.Time.InsertBefore"),
		QT_TRANSLATE_NOOP("Menu", "Insert Before"),
		QT_TRANSLATE_NOOP("Status", "Insert a new step before the current step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Insert Before</b></p><p>Add an empty step before the current step. The current step and the ones after it move one step later.</p>"),
		""
	},

	// SW_VIEW_TIME_INSERT_AFTER
	{
		QT_TRANSLATE_NOOP("Action", "View.Time.InsertAfter"),
		QT_TRANSLATE_NOOP("Menu", "Insert After"),
		QT_TRANSLATE_NOOP("Status", "Insert a new step after the current step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Insert After</b></p><p>Add an empty step after the current step. The steps after it move one step later.</p>"),
		""
	},

	// SW_VIEW_TIME_DELETE
	{
		QT_TRANSLATE_NOOP("Action", "View.Time.Delete"),
		QT_TRANSLATE_NOOP("Menu", "Delete Step"),
		QT_TRANSLATE_NOOP("Status", "Delete the current step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Delete Step</b></p><p>Remove the current step. Its parts join the step before it and later steps move one step earlier.</p>"),
		"Shift+Del"
	},

	// SW_VIEW_TIME_ADD_KEYS
	{
		QT_TRANSLATE_NOOP("Action", "View.Time.AddKeys"),
		QT_TRANSLATE_NOOP("Menu", "Add Keys"),
		QT_TRANSLATE_NOOP("Status", "Toggle adding new animation keys"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Add Keys</b></p><p>Enable this when an edit should create a new animation key at the current step. For example, moving a part at a later step can give it a different position there. The round key buttons in Properties show and toggle keys for individual properties.</p>"),
		"Alt+K"
	},

	// SW_VIEW_TIME_SET_KEYS
	{
		QT_TRANSLATE_NOOP("Action", "View.Time.SetKeys"),
		QT_TRANSLATE_NOOP("Menu", "Set Keys"),
		QT_TRANSLATE_NOOP("Status", "Add keys to the selected objects at the current step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Set Keys</b></p><p>Add position and rotation keys to the selected objects at the current step, holding their values at that step. The full diamond in the Keyframes panel does the same.</p>"),
		"K"
	},

	// SW_VIEW_TIME_REMOVE_KEYS
	{
		QT_TRANSLATE_NOOP("Action", "View.Time.RemoveKeys"),
		QT_TRANSLATE_NOOP("Menu", "Remove Keys"),
		QT_TRANSLATE_NOOP("Status", "Remove the keys of the selected objects at the current step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Remove Keys</b></p><p>Remove every key the selected objects have at the current step. The hollow diamond in the Keyframes panel does the same.</p>"),
		"Shift+K"
	},

	// SW_VIEW_TIME_PLAY
	{
		QT_TRANSLATE_NOOP("Action", "View.Time.Play"),
		QT_TRANSLATE_NOOP("Menu", "Play or Pause"),
		QT_TRANSLATE_NOOP("Status", "Play or pause the steps"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Play or Pause</b></p><p>Step through the model one step at a time to the last step, showing the keyed motion. Use it again to pause.</p>"),
		"Ctrl+K"
	},

	// SW_VIEW_TIME_STOP
	{
		QT_TRANSLATE_NOOP("Action", "View.Time.Stop"),
		QT_TRANSLATE_NOOP("Menu", "Stop"),
		QT_TRANSLATE_NOOP("Status", "Stop playing and go back to the step playing started from"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Stop</b></p><p>Stop playing through the steps and go back to the step playing started from.</p>"),
		"Ctrl+Shift+K"
	},

	// SW_VIEW_SPLIT_HORIZONTAL
	{
		QT_TRANSLATE_NOOP("Action", "View.SplitHorizontal"),
		QT_TRANSLATE_NOOP("Menu", "Split &Horizontal"),
		QT_TRANSLATE_NOOP("Status", "Split the current view horizontally"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Split Horizontal</b></p><p>Split the current view into two views stacked one above the other, each with its own camera. The view cube and axis icon stay on the top left view.</p>"),
		""
	},

	// SW_VIEW_SPLIT_VERTICAL
	{
		QT_TRANSLATE_NOOP("Action", "View.SplitVertical"),
		QT_TRANSLATE_NOOP("Menu", "Split &Vertical"),
		QT_TRANSLATE_NOOP("Status", "Split the current view vertically"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Split Vertical</b></p><p>Split the current view into two views side by side, each with its own camera. The view cube and axis icon stay on the top left view.</p>"),
		""
	},

	// SW_VIEW_REMOVE_VIEW
	{
		QT_TRANSLATE_NOOP("Action", "View.RemoveView"),
		QT_TRANSLATE_NOOP("Menu", "Re&move View"),
		QT_TRANSLATE_NOOP("Status", "Remove the current view"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Remove View</b></p><p>Close the active split view.</p>"),
		""
	},

	// SW_VIEW_RESET_VIEWS
	{
		QT_TRANSLATE_NOOP("Action", "View.ResetViews"),
		QT_TRANSLATE_NOOP("Menu", "Rese&t Views"),
		QT_TRANSLATE_NOOP("Status", "Reset all views"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Reset Views</b></p><p>Remove every split and go back to one view.</p>"),
		""
	},

	// SW_VIEW_TOOLBAR_STANDARD
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.Standard"),
		QT_TRANSLATE_NOOP("Menu", "Standard Toolbar"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Standard toolbar"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Show or Hide</b></p><p>Show or hide this toolbar or panel. A right click on any toolbar or panel title gives the same list.</p>"),
		"F4"
	},

	// SW_VIEW_TOOLBAR_TOOLS
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.Tools"),
		QT_TRANSLATE_NOOP("Menu", "Tools Toolbar"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Tools toolbar"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Show or Hide</b></p><p>Show or hide this toolbar or panel. A right click on any toolbar or panel title gives the same list.</p>"),
		"F5"
	},

	// SW_VIEW_TOOLBAR_TIME
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.Time"),
		QT_TRANSLATE_NOOP("Menu", "Time Toolbar"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Time toolbar"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Show or Hide</b></p><p>Show or hide this toolbar or panel. A right click on any toolbar or panel title gives the same list.</p>"),
		"F6"
	},

	// SW_VIEW_TOOLBAR_PARTS
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.Parts"),
		QT_TRANSLATE_NOOP("Menu", "Parts Panel"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Parts panel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Show or Hide</b></p><p>Show or hide this toolbar or panel. A right click on any toolbar or panel title gives the same list.</p>"),
		"F8"
	},

	// SW_VIEW_TOOLBAR_COLORS
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.Colors"),
		QT_TRANSLATE_NOOP("Menu", "Colors Panel"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Colors panel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Show or Hide</b></p><p>Show or hide this toolbar or panel. A right click on any toolbar or panel title gives the same list.</p>"),
		"F7"
	},

	// SW_VIEW_TOOLBAR_PROPERTIES
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.Properties"),
		QT_TRANSLATE_NOOP("Menu", "Properties Panel"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Properties panel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Show or Hide</b></p><p>Show or hide this toolbar or panel. A right click on any toolbar or panel title gives the same list.</p>"),
		"F9"
	},

	// SW_VIEW_TOOLBAR_TIMELINE
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.Timeline"),
		QT_TRANSLATE_NOOP("Menu", "Timeline Panel"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Timeline panel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Show or Hide</b></p><p>Show or hide this toolbar or panel. A right click on any toolbar or panel title gives the same list.</p>"),
		"F10"
	},

	// SW_VIEW_TOOLBAR_PREVIEW
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.Preview"),
		QT_TRANSLATE_NOOP("Menu", "Preview Panel"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Preview panel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Show or Hide</b></p><p>Show or hide this toolbar or panel. A right click on any toolbar or panel title gives the same list.</p>"),
		"F11"
	},

	// SW_VIEW_TOOLBAR_TERMINAL
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.Terminal"),
		QT_TRANSLATE_NOOP("Menu", "Terminal Panel"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Terminal panel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Show or Hide</b></p><p>Show or hide this toolbar or panel. A right click on any toolbar or panel title gives the same list.</p>"),
		""
	},

	// SW_VIEW_TOOLBAR_MINIFIGURE_BUILDER
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.MinifigureBuilder"),
		QT_TRANSLATE_NOOP("Menu", "Minifigure Builder Panel"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Minifigure Builder panel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Show or Hide</b></p><p>Show or hide this toolbar or panel. A right click on any toolbar or panel title gives the same list.</p>"),
		""
	},

	// SW_VIEW_TOOLBAR_EXPLODE
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.Explode"),
		QT_TRANSLATE_NOOP("Menu", "Explode Panel"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Explode panel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Explode</b></p><p>Show or hide the Explode panel, which spreads the parts on screen away from an anchor to show how they connect.</p>"),
		""
	},

	// SW_VIEW_TOOLBAR_JOINTS
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.Joints"),
		QT_TRANSLATE_NOOP("Menu", "Joints Panel"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Joints panel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Joints</b></p><p>Show or hide the Joints panel, which lists the joints of every model and moves, rotates or deletes them.</p>"),
		""
	},

	// SW_VIEW_TOOLBAR_POSES
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.Poses"),
		QT_TRANSLATE_NOOP("Menu", "Poses Panel"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Poses panel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Poses</b></p><p>Show or hide the Poses panel, which saves and recalls named sets of joint angles.</p>"),
		""
	},
	// SW_VIEW_TOOLBAR_KEYFRAMES
	{
		QT_TRANSLATE_NOOP("Action", "View.Toolbars.Keyframes"),
		QT_TRANSLATE_NOOP("Menu", "Keyframes Panel"),
		QT_TRANSLATE_NOOP("Status", "Toggle the Keyframes panel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Keyframes</b></p><p>Show or hide the Keyframes panel, a timeline of the animation keys of the objects per step.</p>"),
		""
	},

	// SW_VIEW_LOCK_INTERFACE
	{
		QT_TRANSLATE_NOOP("Action", "View.LockInterface"),
		QT_TRANSLATE_NOOP("Menu", "Lock Interface"),
		QT_TRANSLATE_NOOP("Status", "Lock the panels and toolbars in place"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Lock Interface</b></p><p>Keep the panels and toolbars where they are: panel title bars are hidden to save space, and panels and toolbars can no longer be dragged, floated or docked elsewhere. Panels still resize, and they can still be shown or hidden from this menu. Right-click a toolbar, or use View, Toolbars, to lock or unlock.</p>"),
		""
	},

	// SW_VIEW_FULL_SCREEN
	{
		QT_TRANSLATE_NOOP("Action", "View.FullScreen"),
		QT_TRANSLATE_NOOP("Menu", "&Full Screen"),
		QT_TRANSLATE_NOOP("Status", "Toggle full screen mode"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Full Screen</b></p><p>Fill the whole screen with the StudWorx window, or go back to a normal window.</p>"),
		"Alt+F11"
	},

	// SW_VIEW_CLOSE_CURRENT_TAB
	{
		QT_TRANSLATE_NOOP("Action", "View.CloseCurrentTab"),
		QT_TRANSLATE_NOOP("Menu", "Close &Tab"),
		QT_TRANSLATE_NOOP("Status", "Close current tab"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Close Tab</b></p><p>Close the current model tab, asking to save its changes first.</p>"),
		"Ctrl+W"
	},

	// SW_VIEW_SHADE_WIREFRAME
	{
		QT_TRANSLATE_NOOP("Action", "View.Shade.Wireframe"),
		QT_TRANSLATE_NOOP("Menu", "&Wireframe"),
		QT_TRANSLATE_NOOP("Status", "Display the scene as wireframe"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Wireframe</b></p><p>Draw the model view as lines only, without faces.</p>"),
		"Shift+W"
	},

	// SW_VIEW_SHADE_FLAT
	{
		QT_TRANSLATE_NOOP("Action", "View.Shade.Flat"),
		QT_TRANSLATE_NOOP("Menu", "&Flat Shading"),
		QT_TRANSLATE_NOOP("Status", "Display the scene without any shading or lights"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Flat Shading</b></p><p>Draw the model view in plain colors, without shading or lights.</p>"),
		"Shift+F"
	},

	// SW_VIEW_SHADE_DEFAULT_LIGHTS
	{
		QT_TRANSLATE_NOOP("Action", "View.Shade.DefaultLights"),
		QT_TRANSLATE_NOOP("Menu", "&Default Lights"),
		QT_TRANSLATE_NOOP("Status", "Display the scene with the default lights"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Default Lights</b></p><p>Light the model view with the built-in lights instead of the lights placed in the model.</p>"),
		""
	},

	// SW_VIEW_LIGHTING_STANDARD
	{
		QT_TRANSLATE_NOOP("Action", "View.Lighting.Standard"),
		QT_TRANSLATE_NOOP("Menu", "&Standard Lighting"),
		QT_TRANSLATE_NOOP("Status", "Use a single light with soft highlights for the default lights"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Standard Lighting</b></p><p>Light parts drawn with the Default Lights shading mode using one light above the camera with a soft highlight. This is the classic look.</p>"),
		""
	},

	// SW_VIEW_LIGHTING_STUDIO
	{
		QT_TRANSLATE_NOOP("Action", "View.Lighting.Studio"),
		QT_TRANSLATE_NOOP("Menu", "St&udio Lighting"),
		QT_TRANSLATE_NOOP("Status", "Use key, fill and rim lights for the default lights"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Studio Lighting</b></p><p>Light parts drawn with the Default Lights shading mode using a key light, a fill light from the camera and a rim light that brightens silhouette edges, for a softer, more photographic look.</p>"),
		""
	},

	// SW_VIEW_LIGHTING_MATTE
	{
		QT_TRANSLATE_NOOP("Action", "View.Lighting.Matte"),
		QT_TRANSLATE_NOOP("Menu", "&Matte Lighting"),
		QT_TRANSLATE_NOOP("Status", "Use the default lights without highlights"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Matte Lighting</b></p><p>Light parts drawn with the Default Lights shading mode without specular highlights, so colors stay flat and true while surfaces keep their shading.</p>"),
		""
	},

	// SW_VIEW_LIGHTING_TOON
	{
		QT_TRANSLATE_NOOP("Action", "View.Lighting.Toon"),
		QT_TRANSLATE_NOOP("Menu", "&Toon Lighting"),
		QT_TRANSLATE_NOOP("Status", "Use banded cel shading for the default lights"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Toon Lighting</b></p><p>Light parts drawn with the Default Lights shading mode in a few flat brightness bands with a sharp highlight, for a cartoon look that pairs well with edge lines.</p>"),
		""
	},

	// SW_VIEW_PROJECTION_PERSPECTIVE
	{
		QT_TRANSLATE_NOOP("Action", "View.Projection.Perspective"),
		QT_TRANSLATE_NOOP("Menu", "&Perspective"),
		QT_TRANSLATE_NOOP("Status", "Set the current camera to use a perspective projection"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Perspective</b></p><p>Show the view with perspective, distant parts smaller.</p>"),
		"F3"
	},

	// SW_VIEW_PROJECTION_ORTHOGRAPHIC
	{
		QT_TRANSLATE_NOOP("Action", "View.Projection.Orthographic"),
		QT_TRANSLATE_NOOP("Menu", "&Orthographic"),
		QT_TRANSLATE_NOOP("Status", "Set the current camera to use an orthographic projection"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Orthographic</b></p><p>Show the view without perspective, parallel lines staying parallel.</p>"),
		"F2"
	},

	// SW_VIEW_TOGGLE_VIEW_CUBE
	{
		QT_TRANSLATE_NOOP("Action", "View.ToggleViewCube"),
		QT_TRANSLATE_NOOP("Menu", "View Cube"),
		QT_TRANSLATE_NOOP("Status", "Toggle the view cube"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>View Cube</b></p><p>Show or hide the view cube. Click its faces, edges and corners to turn the camera. With split views it shows on the top left view only.</p>"),
		"F12"
	},

	// SW_VIEW_TOGGLE_AXIS_ICON
	{
		QT_TRANSLATE_NOOP("Action", "View.ToggleAxisIcon"),
		QT_TRANSLATE_NOOP("Menu", "Axis Icon"),
		QT_TRANSLATE_NOOP("Status", "Toggle axis icon"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Axis Icon</b></p><p>Show or hide the X, Y and Z axis icon. With split views it shows on the top left view only.</p>"),
		"Alt+F12"
	},

	// SW_VIEW_TOGGLE_GRID
	{
		QT_TRANSLATE_NOOP("Action", "View.ToggleGrid"),
		QT_TRANSLATE_NOOP("Menu", "Base Grid"),
		QT_TRANSLATE_NOOP("Status", "Toggle grid"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Base Grid</b></p><p>Show or hide the base grid. The height grids have their own toggles.</p>"),
		""
	},

	// SW_VIEW_FADE_PREVIOUS_STEPS
	{
		QT_TRANSLATE_NOOP("Action", "View.FadePreviousSteps"),
		QT_TRANSLATE_NOOP("Menu", "Fade Previous Steps"),
		QT_TRANSLATE_NOOP("Status", "Toggle fading previous model steps"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Fade Previous Steps</b></p><p>Tint parts from previous building steps with the chosen fade color, so the parts of the current step stand out.</p>"),
		""
	},

	// SW_VIEW_STEP_VIEW_EDIT
	{
		QT_TRANSLATE_NOOP("Action", "View.StepView.Edit"),
		QT_TRANSLATE_NOOP("Menu", "Step View..."),
		QT_TRANSLATE_NOOP("Status", "Set the view rotation of the current step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Step View</b></p><p>Set the view rotation of the current step, saved as an MLCad ROTSTEP.</p>"),
		""
	},

	// SW_VIEW_STEP_VIEW_REMOVE
	{
		QT_TRANSLATE_NOOP("Action", "View.StepView.Remove"),
		QT_TRANSLATE_NOOP("Menu", "Remove Step View"),
		QT_TRANSLATE_NOOP("Status", "Remove the view rotation of the current step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Remove Step View</b></p><p>Clear the view rotation of the current step.</p>"),
		""
	},

	// SW_VIEW_STEP_VIEW_TOGGLE_FOLLOW
	{
		QT_TRANSLATE_NOOP("Action", "View.StepView.ToggleFollow"),
		QT_TRANSLATE_NOOP("Menu", "Follow Step Views"),
		QT_TRANSLATE_NOOP("Status", "Toggle turning the view to each step's view rotation when changing steps"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Follow Step Views</b></p><p>Turn the view to each step's view rotation when the step changes.</p>"),
		""
	},

	// SW_VIEW_NORMAL_TO_FACE
	{
		QT_TRANSLATE_NOOP("Action", "View.NormalToFace"),
		QT_TRANSLATE_NOOP("Menu", "Normal to Face"),
		QT_TRANSLATE_NOOP("Status", "Click a face to align the view straight on to it"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Normal to Face</b></p><p>Click a face of any part, including parts inside submodels, to point the camera straight at it. The model is not changed. Press Esc to cancel.</p>"),
		""
	},

	// SW_VIEW_CUTAWAY_CUBE
	{
		QT_TRANSLATE_NOOP("Action", "View.Cutaway.Cube"),
		QT_TRANSLATE_NOOP("Menu", "&Cube"),
		QT_TRANSLATE_NOOP("Status", "Click a face to cut away a cube of the model around it"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Cube Cutaway</b></p><p>Click a face of any part to hide everything inside a cube centered on it, so you can see inside the model. Clicking empty space centers the cube on the view target. The model is not changed. Press Esc to cancel.</p>"),
		""
	},

	// SW_VIEW_CUTAWAY_SPHERE
	{
		QT_TRANSLATE_NOOP("Action", "View.Cutaway.Sphere"),
		QT_TRANSLATE_NOOP("Menu", "&Sphere"),
		QT_TRANSLATE_NOOP("Status", "Click a face to cut away a sphere of the model around it"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Sphere Cutaway</b></p><p>Click a face of any part to hide everything inside a sphere centered on it, so you can see inside the model. Clicking empty space centers the sphere on the view target. The model is not changed. Press Esc to cancel.</p>"),
		""
	},

	// SW_VIEW_CUTAWAY_CYLINDER
	{
		QT_TRANSLATE_NOOP("Action", "View.Cutaway.Cylinder"),
		QT_TRANSLATE_NOOP("Menu", "C&ylinder"),
		QT_TRANSLATE_NOOP("Status", "Click a face to drill a cylinder into the model through it"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Cylinder Cutaway</b></p><p>Click a face of any part to hide everything inside a cylinder centered on it, its axis running straight through the face, so you can see inside the model. Clicking empty space centers the cylinder on the view target. The model is not changed. Press Esc to cancel.</p>"),
		""
	},

	// SW_VIEW_CUTAWAY_GROW
	{
		QT_TRANSLATE_NOOP("Action", "View.Cutaway.Grow"),
		QT_TRANSLATE_NOOP("Menu", "&Grow"),
		QT_TRANSLATE_NOOP("Status", "Make the cutaway larger"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Grow</b></p><p>Make the cutaway larger.</p>"),
		""
	},

	// SW_VIEW_CUTAWAY_SHRINK
	{
		QT_TRANSLATE_NOOP("Action", "View.Cutaway.Shrink"),
		QT_TRANSLATE_NOOP("Menu", "S&hrink"),
		QT_TRANSLATE_NOOP("Status", "Make the cutaway smaller"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Shrink</b></p><p>Make the cutaway smaller.</p>"),
		""
	},

	// SW_VIEW_CUTAWAY_PUSH_IN
	{
		QT_TRANSLATE_NOOP("Action", "View.Cutaway.PushIn"),
		QT_TRANSLATE_NOOP("Menu", "Push &In"),
		QT_TRANSLATE_NOOP("Status", "Move the cutaway one plate deeper into the model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Push In</b></p><p>Move the cutaway one plate deeper into the model.</p>"),
		""
	},

	// SW_VIEW_CUTAWAY_PULL_OUT
	{
		QT_TRANSLATE_NOOP("Action", "View.Cutaway.PullOut"),
		QT_TRANSLATE_NOOP("Menu", "Pull &Out"),
		QT_TRANSLATE_NOOP("Status", "Move the cutaway one plate back out of the model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Pull Out</b></p><p>Move the cutaway one plate back out of the model.</p>"),
		""
	},

	// SW_VIEW_CUTAWAY_SHADE_COLOR
	{
		QT_TRANSLATE_NOOP("Action", "View.Cutaway.ShadeColor"),
		QT_TRANSLATE_NOOP("Menu", "Sh&ade Color..."),
		QT_TRANSLATE_NOOP("Status", "Choose the light shade on the cutaway faces"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Shade Color</b></p><p>Choose the light shade drawn on the cut faces.</p>"),
		""
	},

	// SW_VIEW_CUTAWAY_REMOVE
	{
		QT_TRANSLATE_NOOP("Action", "View.Cutaway.Remove"),
		QT_TRANSLATE_NOOP("Menu", "&Remove"),
		QT_TRANSLATE_NOOP("Status", "Show the whole model again, or press Esc with nothing selected"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Remove</b></p><p>Remove the cutaway and show the whole model again. Esc with nothing selected does the same.</p>"),
		""
	},

	// SW_VIEW_GRID_TOGGLE_ALL
	{
		QT_TRANSLATE_NOOP("Action", "View.Grid.ToggleAll"),
		QT_TRANSLATE_NOOP("Menu", "All Grids"),
		QT_TRANSLATE_NOOP("Status", "Turn the base, XZ and YZ height grids on or off together"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>All Grids</b></p><p>Turn the base grid and both height grids on together, or off when they are all on. The axis lines are not changed.</p>"),
		""
	},

	// SW_VIEW_GRID_TOGGLE_HEIGHT_XZ
	{
		QT_TRANSLATE_NOOP("Action", "View.Grid.ToggleHeightXZ"),
		QT_TRANSLATE_NOOP("Menu", "XZ Height Grid"),
		QT_TRANSLATE_NOOP("Status", "Toggle the vertical XZ height grid"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>XZ Height Grid</b></p><p>Show or hide the vertical XZ height grid sized around the model.</p>"),
		""
	},

	// SW_VIEW_GRID_TOGGLE_HEIGHT_YZ
	{
		QT_TRANSLATE_NOOP("Action", "View.Grid.ToggleHeightYZ"),
		QT_TRANSLATE_NOOP("Menu", "YZ Height Grid"),
		QT_TRANSLATE_NOOP("Status", "Toggle the vertical YZ height grid"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>YZ Height Grid</b></p><p>Show or hide the vertical YZ height grid sized around the model.</p>"),
		""
	},

	// SW_VIEW_GRID_TOGGLE_HEIGHT_BEHIND
	{
		QT_TRANSLATE_NOOP("Action", "View.Grid.ToggleHeightBehindModel"),
		QT_TRANSLATE_NOOP("Menu", "Height Grids Behind Model"),
		QT_TRANSLATE_NOOP("Status", "Toggle drawing the height grids behind the model instead of through the origin axes"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Height Grids Behind Model</b></p><p>Draw the height grids behind the model instead of through the origin axes.</p>"),
		""
	},

	// SW_VIEW_GRID_TOGGLE_AXIS_X
	{
		QT_TRANSLATE_NOOP("Action", "View.Grid.ToggleAxisX"),
		QT_TRANSLATE_NOOP("Menu", "X Axis Line"),
		QT_TRANSLATE_NOOP("Status", "Toggle the x axis line on the grid origin"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>X Axis Line</b></p><p>Show or hide the X axis line through the grid origin.</p>"),
		""
	},

	// SW_VIEW_GRID_TOGGLE_AXIS_Y
	{
		QT_TRANSLATE_NOOP("Action", "View.Grid.ToggleAxisY"),
		QT_TRANSLATE_NOOP("Menu", "Y Axis Line"),
		QT_TRANSLATE_NOOP("Status", "Toggle the y axis line on the grid origin"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Y Axis Line</b></p><p>Show or hide the Y axis line through the grid origin.</p>"),
		""
	},

	// SW_VIEW_GRID_TOGGLE_AXIS_Z
	{
		QT_TRANSLATE_NOOP("Action", "View.Grid.ToggleAxisZ"),
		QT_TRANSLATE_NOOP("Menu", "Z Axis Line"),
		QT_TRANSLATE_NOOP("Status", "Toggle the z axis line on the grid origin and the height grids"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Z Axis Line</b></p><p>Show or hide the Z axis line through the grid origin and the height grids.</p>"),
		""
	},

	// SW_VIEW_TERMINAL_TOGGLE_FOLLOW_MODEL
	{
		QT_TRANSLATE_NOOP("Action", "View.Terminal.ToggleFollowModel"),
		QT_TRANSLATE_NOOP("Menu", "Follow Model Folder"),
		QT_TRANSLATE_NOOP("Status", "Toggle changing the terminal to the folder of each model that is opened"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Follow Model Folder</b></p><p>Move the terminal to the folder of each model you open.</p>"),
		""
	},

	// SW_VIEW_TERMINAL_GO_TO_MODEL_FOLDER
	{
		QT_TRANSLATE_NOOP("Action", "View.Terminal.GoToModelFolder"),
		QT_TRANSLATE_NOOP("Menu", "Go to Model Folder"),
		QT_TRANSLATE_NOOP("Status", "Change the terminal to the folder of the current model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Go to Model Folder</b></p><p>Move the terminal to the folder of the current model.</p>"),
		""
	},

	// SW_VIEW_TERMINAL_OPEN_MODEL_FOLDER
	{
		QT_TRANSLATE_NOOP("Action", "View.Terminal.OpenModelFolder"),
		QT_TRANSLATE_NOOP("Menu", "Open Model Folder"),
		QT_TRANSLATE_NOOP("Status", "Open the model folder"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Open Model Folder</b></p><p>Open the folder of the current model in the file manager. The button is unavailable until the model is saved to a file.</p>"),
		""
	},

	// SW_VIEW_COLORS_TOGGLE_SOLID
	{
		QT_TRANSLATE_NOOP("Action", "View.Colors.ToggleSolid"),
		QT_TRANSLATE_NOOP("Menu", "Solid Colors"),
		QT_TRANSLATE_NOOP("Status", "Expand or collapse the solid colors"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Solid Colors</b></p><p>Expand or collapse the solid colors in the Colors panel.</p>"),
		""
	},

	// SW_VIEW_COLORS_TOGGLE_TRANSLUCENT
	{
		QT_TRANSLATE_NOOP("Action", "View.Colors.ToggleTranslucent"),
		QT_TRANSLATE_NOOP("Menu", "Translucent Colors"),
		QT_TRANSLATE_NOOP("Status", "Expand or collapse the translucent colors"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Translucent Colors</b></p><p>Expand or collapse the translucent colors in the Colors panel.</p>"),
		""
	},

	// SW_VIEW_COLORS_TOGGLE_SPECIAL
	{
		QT_TRANSLATE_NOOP("Action", "View.Colors.ToggleSpecial"),
		QT_TRANSLATE_NOOP("Menu", "Special Colors"),
		QT_TRANSLATE_NOOP("Status", "Expand or collapse the special colors"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Special Colors</b></p><p>Expand or collapse the special colors in the Colors panel.</p>"),
		""
	},

	// SW_PART_INSERT_COLOR_CURRENT
	{
		QT_TRANSLATE_NOOP("Action", "Part.InsertColor.Current"),
		QT_TRANSLATE_NOOP("Menu", "Current Color"),
		QT_TRANSLATE_NOOP("Status", "Insert new parts with the current color"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Current Color</b></p><p>Insert new parts in the color chosen in the Colors panel.</p>"),
		""
	},

	// SW_PART_INSERT_COLOR_SELECTED
	{
		QT_TRANSLATE_NOOP("Action", "Part.InsertColor.Selected"),
		QT_TRANSLATE_NOOP("Menu", "Selected Part Color"),
		QT_TRANSLATE_NOOP("Status", "Insert new parts with the color of the selected part"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Selected Part Color</b></p><p>Insert new parts in the color of the selected part.</p>"),
		""
	},

	// SW_PART_INSERT_COLOR_LAST_INSERTED
	{
		QT_TRANSLATE_NOOP("Action", "Part.InsertColor.LastInserted"),
		QT_TRANSLATE_NOOP("Menu", "Last Inserted Part Color"),
		QT_TRANSLATE_NOOP("Status", "Insert new parts with the color of the last inserted part"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Last Inserted Part Color</b></p><p>Insert new parts in the color of the last part inserted.</p>"),
		""
	},

	// SW_VIEW_PARTS_TOGGLE_LOCK_COLOR
	{
		QT_TRANSLATE_NOOP("Action", "View.Parts.ToggleLockColor"),
		QT_TRANSLATE_NOOP("Menu", "Lock Parts Color"),
		QT_TRANSLATE_NOOP("Status", "Toggle keeping the parts list in a fixed color instead of following the insert color"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Lock Parts Color</b></p><p>Keep the Parts panel thumbnails in one color instead of following the insert color.</p>"),
		""
	},

	// SW_FILE_TOGGLE_OPEN_MOST_RECENT
	{
		QT_TRANSLATE_NOOP("Action", "File.ToggleOpenMostRecent"),
		QT_TRANSLATE_NOOP("Menu", "Open Most Recent File on Startup"),
		QT_TRANSLATE_NOOP("Status", "Toggle opening the most recently used file when StudWorx starts"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Open Most Recent File on Startup</b></p><p>Reopen the last model you worked on each time StudWorx starts.</p>"),
		""
	},

	// SW_FILE_TOGGLE_RESTORE_TAB_LAYOUT
	{
		QT_TRANSLATE_NOOP("Action", "File.ToggleRestoreTabLayout"),
		QT_TRANSLATE_NOOP("Menu", "Restore Tab Layout"),
		QT_TRANSLATE_NOOP("Status", "Toggle restoring saved model tabs and view layouts when opening a model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Restore Tab Layout</b></p><p>Bring back the saved model tabs and view splits of a model when it opens.</p>"),
		""
	},

	// SW_EDIT_TOGGLE_FIXED_DIRECTION_KEYS
	{
		QT_TRANSLATE_NOOP("Action", "Edit.ToggleFixedDirectionKeys"),
		QT_TRANSLATE_NOOP("Menu", "Fixed Direction Keys"),
		QT_TRANSLATE_NOOP("Status", "Toggle moving parts along fixed model axes instead of view-relative directions"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Fixed Direction Keys</b></p><p>Keep the arrow keys on the model axes instead of following the direction of the view.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_MESH_LOD
	{
		QT_TRANSLATE_NOOP("Action", "View.ToggleMeshLOD"),
		QT_TRANSLATE_NOOP("Menu", "Mesh LOD"),
		QT_TRANSLATE_NOOP("Status", "Toggle using simpler meshes for distant parts"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Mesh LOD</b></p><p>Draw distant parts with simpler meshes (level of detail), from a distance set in Preferences, so large models stay smooth.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_HIGHLIGHT_NEW_PARTS
	{
		QT_TRANSLATE_NOOP("Action", "View.ToggleHighlightNewParts"),
		QT_TRANSLATE_NOOP("Menu", "Highlight New Parts"),
		QT_TRANSLATE_NOOP("Status", "Toggle highlighting parts added in the current step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Highlight New Parts</b></p><p>Highlight the parts added in the current building step with the chosen highlight color.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_FADE_ON_EDIT
	{
		QT_TRANSLATE_NOOP("Action", "View.ToggleFadeOnEdit"),
		QT_TRANSLATE_NOOP("Menu", "Fade on Edit"),
		QT_TRANSLATE_NOOP("Status", "Toggle tinting the rest of the model while editing a submodel in place"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Fade on Edit</b></p><p>Tint the rest of the model with the chosen color while editing a submodel in place. When off, the rest of the model stays in its own colors, slightly translucent.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_FADE_ON_JOINT_SET
	{
		QT_TRANSLATE_NOOP("Action", "View.ToggleFadeOnJointSet"),
		QT_TRANSLATE_NOOP("Menu", "Fade on Joint Set"),
		QT_TRANSLATE_NOOP("Status", "Toggle tinting all but the selected submodel while setting a joint"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Fade on Joint Set</b></p><p>Tint all but the selected submodel with the chosen color while setting a joint or in joint mode.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_FADE_ON_JOINT_MOVE
	{
		QT_TRANSLATE_NOOP("Action", "View.ToggleFadeOnJointMove"),
		QT_TRANSLATE_NOOP("Menu", "Fade on Joint Move and Rotate"),
		QT_TRANSLATE_NOOP("Status", "Toggle tinting all but the selected submodel while moving or rotating a joint"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Fade on Joint Move and Rotate</b></p><p>Keep the joint fade while moving or rotating a joint. Leave it off to check parts for collisions in their own colors.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_HOVER_OUTLINE
	{
		QT_TRANSLATE_NOOP("Action", "View.ToggleHoverOutline"),
		QT_TRANSLATE_NOOP("Menu", "Submodel Hover Outline"),
		QT_TRANSLATE_NOOP("Status", "Toggle outlining the innermost submodel under the pointer"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel Hover Outline</b></p><p>Outline the innermost submodel under the pointer with the hover outline color, or the submodel a joint would go on while setting a joint.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_GRID_STUDS
	{
		QT_TRANSLATE_NOOP("Action", "View.Grid.ToggleStuds"),
		QT_TRANSLATE_NOOP("Menu", "Grid Studs"),
		QT_TRANSLATE_NOOP("Status", "Toggle drawing studs on the base grid"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Grid Studs</b></p><p>Show or hide the studs of the base grid.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_GRID_LINES
	{
		QT_TRANSLATE_NOOP("Action", "View.Grid.ToggleLines"),
		QT_TRANSLATE_NOOP("Menu", "Grid Lines"),
		QT_TRANSLATE_NOOP("Status", "Toggle drawing lines on the base grid"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Grid Lines</b></p><p>Show or hide the lines of the base grid.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_GRID_ORIGIN
	{
		QT_TRANSLATE_NOOP("Action", "View.Grid.ToggleOrigin"),
		QT_TRANSLATE_NOOP("Menu", "Grid Origin Lines"),
		QT_TRANSLATE_NOOP("Status", "Toggle drawing the base grid origin lines"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Grid Origin Lines</b></p><p>Show or hide the origin lines of the base grid.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_AUTOMATE_EDGE_COLOR
	{
		QT_TRANSLATE_NOOP("Action", "View.ToggleAutomateEdgeColor"),
		QT_TRANSLATE_NOOP("Menu", "Automate Edge Color"),
		QT_TRANSLATE_NOOP("Status", "Toggle automatically computing part edge colors"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Automate Edge Color</b></p><p>Compute edge colors from each part color, with the edge contrast, saturation and light to dark settings in Preferences.</p>"),
		""
	},

	// SW_VIEW_HIGHLIGHT_STYLE_OUTLINE
	{
		QT_TRANSLATE_NOOP("Action", "View.HighlightStyle.Outline"),
		QT_TRANSLATE_NOOP("Menu", "Outline Highlight"),
		QT_TRANSLATE_NOOP("Status", "Highlight selected and new parts with outlines"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Outline Highlight</b></p><p>Mark selected and new parts with an outline.</p>"),
		""
	},

	// SW_VIEW_HIGHLIGHT_STYLE_TINT
	{
		QT_TRANSLATE_NOOP("Action", "View.HighlightStyle.TintAndEdges"),
		QT_TRANSLATE_NOOP("Menu", "Tint and Edges Highlight"),
		QT_TRANSLATE_NOOP("Status", "Highlight selected and new parts with tinted faces and colored edges"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Tint and Edges Highlight</b></p><p>Mark selected and new parts with tinted faces and colored edges.</p>"),
		""
	},

	// SW_VIEW_SELECTION_OUTLINE_WIDTH_INCREASE
	{
		QT_TRANSLATE_NOOP("Action", "View.HighlightStyle.IncreaseOutlineWidth"),
		QT_TRANSLATE_NOOP("Menu", "Increase Outline Width"),
		QT_TRANSLATE_NOOP("Status", "Make the selection outline thicker"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Increase Outline Width</b></p><p>Make the selection outline thicker.</p>"),
		""
	},

	// SW_VIEW_SELECTION_OUTLINE_WIDTH_DECREASE
	{
		QT_TRANSLATE_NOOP("Action", "View.HighlightStyle.DecreaseOutlineWidth"),
		QT_TRANSLATE_NOOP("Menu", "Decrease Outline Width"),
		QT_TRANSLATE_NOOP("Status", "Make the selection outline thinner"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Decrease Outline Width</b></p><p>Make the selection outline thinner.</p>"),
		""
	},

	// SW_VIEW_STUD_STYLE_PLAIN
	{
		QT_TRANSLATE_NOOP("Action", "View.StudStyle.Plain"),
		QT_TRANSLATE_NOOP("Menu", "Plain Studs"),
		QT_TRANSLATE_NOOP("Status", "Draw studs with the plain style"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Stud Style</b></p><p>Draw studs in this style: plain, with one of the logo styles, or in high contrast.</p>"),
		""
	},

	// SW_VIEW_STUD_STYLE_THIN_LINES_LOGO
	{
		QT_TRANSLATE_NOOP("Action", "View.StudStyle.ThinLinesLogo"),
		QT_TRANSLATE_NOOP("Menu", "Thin-Line Logo Studs"),
		QT_TRANSLATE_NOOP("Status", "Draw studs with the thin-line logo style"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Stud Style</b></p><p>Draw studs in this style: plain, with one of the logo styles, or in high contrast.</p>"),
		""
	},

	// SW_VIEW_STUD_STYLE_OUTLINE_LOGO
	{
		QT_TRANSLATE_NOOP("Action", "View.StudStyle.OutlineLogo"),
		QT_TRANSLATE_NOOP("Menu", "Outline Logo Studs"),
		QT_TRANSLATE_NOOP("Status", "Draw studs with the outline logo style"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Stud Style</b></p><p>Draw studs in this style: plain, with one of the logo styles, or in high contrast.</p>"),
		""
	},

	// SW_VIEW_STUD_STYLE_SHARP_TOP_LOGO
	{
		QT_TRANSLATE_NOOP("Action", "View.StudStyle.SharpTopLogo"),
		QT_TRANSLATE_NOOP("Menu", "Sharp Top Logo Studs"),
		QT_TRANSLATE_NOOP("Status", "Draw studs with the sharp top logo style"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Stud Style</b></p><p>Draw studs in this style: plain, with one of the logo styles, or in high contrast.</p>"),
		""
	},

	// SW_VIEW_STUD_STYLE_ROUNDED_TOP_LOGO
	{
		QT_TRANSLATE_NOOP("Action", "View.StudStyle.RoundedTopLogo"),
		QT_TRANSLATE_NOOP("Menu", "Rounded Top Logo Studs"),
		QT_TRANSLATE_NOOP("Status", "Draw studs with the rounded top logo style"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Stud Style</b></p><p>Draw studs in this style: plain, with one of the logo styles, or in high contrast.</p>"),
		""
	},

	// SW_VIEW_STUD_STYLE_FLATTENED_LOGO
	{
		QT_TRANSLATE_NOOP("Action", "View.StudStyle.FlattenedLogo"),
		QT_TRANSLATE_NOOP("Menu", "Flattened Logo Studs"),
		QT_TRANSLATE_NOOP("Status", "Draw studs with the flattened logo style"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Stud Style</b></p><p>Draw studs in this style: plain, with one of the logo styles, or in high contrast.</p>"),
		""
	},

	// SW_VIEW_STUD_STYLE_HIGH_CONTRAST
	{
		QT_TRANSLATE_NOOP("Action", "View.StudStyle.HighContrast"),
		QT_TRANSLATE_NOOP("Menu", "High Contrast Studs"),
		QT_TRANSLATE_NOOP("Status", "Draw studs with the high contrast style"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Stud Style</b></p><p>Draw studs in this style: plain, with one of the logo styles, or in high contrast.</p>"),
		""
	},

	// SW_VIEW_STUD_STYLE_HIGH_CONTRAST_LOGO
	{
		QT_TRANSLATE_NOOP("Action", "View.StudStyle.HighContrastLogo"),
		QT_TRANSLATE_NOOP("Menu", "High Contrast Logo Studs"),
		QT_TRANSLATE_NOOP("Status", "Draw studs with the high contrast logo style"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Stud Style</b></p><p>Draw studs in this style: plain, with one of the logo styles, or in high contrast.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_EDGE_LINES
	{
		QT_TRANSLATE_NOOP("Action", "View.ToggleEdgeLines"),
		QT_TRANSLATE_NOOP("Menu", "Edge Lines"),
		QT_TRANSLATE_NOOP("Status", "Toggle edge lines in model views"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Edge Lines</b></p><p>Show or hide the edge lines of parts in the model view.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_CONDITIONAL_LINES
	{
		QT_TRANSLATE_NOOP("Action", "View.ToggleConditionalLines"),
		QT_TRANSLATE_NOOP("Menu", "Conditional Lines"),
		QT_TRANSLATE_NOOP("Status", "Toggle conditional lines in model views"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Conditional Lines</b></p><p>Show or hide the conditional lines that outline curved surfaces in the model view.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_PREVIEW_SYNCHRONIZED_RENDERING
	{
		QT_TRANSLATE_NOOP("Action", "View.Preview.ToggleSyncRendering"),
		QT_TRANSLATE_NOOP("Menu", "Preview Uses Model View Settings"),
		QT_TRANSLATE_NOOP("Status", "Toggle using the model view rendering settings for the preview"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Preview Uses Model View Settings</b></p><p>Draw the preview with the rendering settings of the model view instead of its own.</p>"),
		""
	},

	// SW_VIEW_PREVIEW_SHADING_WIREFRAME
	{
		QT_TRANSLATE_NOOP("Action", "View.Preview.Shade.Wireframe"),
		QT_TRANSLATE_NOOP("Menu", "Preview Wireframe"),
		QT_TRANSLATE_NOOP("Status", "Display the preview as wireframe"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Preview Wireframe</b></p><p>Draw the preview as lines only, without faces.</p>"),
		""
	},

	// SW_VIEW_PREVIEW_SHADING_FLAT
	{
		QT_TRANSLATE_NOOP("Action", "View.Preview.Shade.Flat"),
		QT_TRANSLATE_NOOP("Menu", "Preview Flat Shading"),
		QT_TRANSLATE_NOOP("Status", "Display the preview without any shading or lights"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Preview Flat Shading</b></p><p>Draw the preview in plain colors, without shading or lights.</p>"),
		""
	},

	// SW_VIEW_PREVIEW_SHADING_DEFAULT_LIGHTS
	{
		QT_TRANSLATE_NOOP("Action", "View.Preview.Shade.DefaultLights"),
		QT_TRANSLATE_NOOP("Menu", "Preview Default Lights"),
		QT_TRANSLATE_NOOP("Status", "Display the preview with the default lights"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Preview Default Lights</b></p><p>Light the preview with the built-in lights.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_PREVIEW_EDGE_LINES
	{
		QT_TRANSLATE_NOOP("Action", "View.Preview.ToggleEdgeLines"),
		QT_TRANSLATE_NOOP("Menu", "Preview Edge Lines"),
		QT_TRANSLATE_NOOP("Status", "Toggle edge lines in the preview"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Preview Edge Lines</b></p><p>Show or hide edge lines in the preview.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_PREVIEW_CONDITIONAL_LINES
	{
		QT_TRANSLATE_NOOP("Action", "View.Preview.ToggleConditionalLines"),
		QT_TRANSLATE_NOOP("Menu", "Preview Conditional Lines"),
		QT_TRANSLATE_NOOP("Status", "Toggle conditional lines in the preview"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Preview Conditional Lines</b></p><p>Show or hide conditional lines in the preview.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_PARTS_SYNCHRONIZED_RENDERING
	{
		QT_TRANSLATE_NOOP("Action", "View.Parts.ToggleSyncRendering"),
		QT_TRANSLATE_NOOP("Menu", "Parts Use Model View Settings"),
		QT_TRANSLATE_NOOP("Status", "Toggle using the model view rendering settings for the parts list"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Parts Use Model View Settings</b></p><p>Draw the parts list with the rendering settings of the model view instead of its own.</p>"),
		""
	},

	// SW_VIEW_PARTS_SHADING_WIREFRAME
	{
		QT_TRANSLATE_NOOP("Action", "View.Parts.Shade.Wireframe"),
		QT_TRANSLATE_NOOP("Menu", "Parts Wireframe"),
		QT_TRANSLATE_NOOP("Status", "Display the parts list as wireframe"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Parts Wireframe</b></p><p>Draw the parts list as lines only, without faces.</p>"),
		""
	},

	// SW_VIEW_PARTS_SHADING_FLAT
	{
		QT_TRANSLATE_NOOP("Action", "View.Parts.Shade.Flat"),
		QT_TRANSLATE_NOOP("Menu", "Parts Flat Shading"),
		QT_TRANSLATE_NOOP("Status", "Display the parts list without any shading or lights"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Parts Flat Shading</b></p><p>Draw the parts list in plain colors, without shading or lights.</p>"),
		""
	},

	// SW_VIEW_PARTS_SHADING_DEFAULT_LIGHTS
	{
		QT_TRANSLATE_NOOP("Action", "View.Parts.Shade.DefaultLights"),
		QT_TRANSLATE_NOOP("Menu", "Parts Default Lights"),
		QT_TRANSLATE_NOOP("Status", "Display the parts list with the default lights"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Parts Default Lights</b></p><p>Light the parts list with the built-in lights.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_PARTS_EDGE_LINES
	{
		QT_TRANSLATE_NOOP("Action", "View.Parts.ToggleEdgeLines"),
		QT_TRANSLATE_NOOP("Menu", "Parts Edge Lines"),
		QT_TRANSLATE_NOOP("Status", "Toggle edge lines in the parts list"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Parts Edge Lines</b></p><p>Show or hide edge lines in the parts list.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_PARTS_CONDITIONAL_LINES
	{
		QT_TRANSLATE_NOOP("Action", "View.Parts.ToggleConditionalLines"),
		QT_TRANSLATE_NOOP("Menu", "Parts Conditional Lines"),
		QT_TRANSLATE_NOOP("Status", "Toggle conditional lines in the parts list"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Parts Conditional Lines</b></p><p>Show or hide conditional lines in the parts list.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_MINIFIGURE_BUILDER_SYNCHRONIZED_RENDERING
	{
		QT_TRANSLATE_NOOP("Action", "View.MinifigureBuilder.ToggleSyncRendering"),
		QT_TRANSLATE_NOOP("Menu", "Minifigure Builder Uses Model View Settings"),
		QT_TRANSLATE_NOOP("Status", "Toggle using the model view rendering settings in the minifigure builder preview"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Minifigure Builder Uses Model View Settings</b></p><p>Draw the Minifigure Builder preview with the rendering settings of the model view instead of its own.</p>"),
		""
	},

	// SW_VIEW_MINIFIGURE_BUILDER_SHADING_WIREFRAME
	{
		QT_TRANSLATE_NOOP("Action", "View.MinifigureBuilder.Shade.Wireframe"),
		QT_TRANSLATE_NOOP("Menu", "Minifigure Builder Wireframe"),
		QT_TRANSLATE_NOOP("Status", "Display the minifigure builder preview as a wireframe"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Minifigure Builder Wireframe</b></p><p>Draw the Minifigure Builder preview as lines only, without faces.</p>"),
		""
	},

	// SW_VIEW_MINIFIGURE_BUILDER_SHADING_FLAT
	{
		QT_TRANSLATE_NOOP("Action", "View.MinifigureBuilder.Shade.Flat"),
		QT_TRANSLATE_NOOP("Menu", "Minifigure Builder Flat Shading"),
		QT_TRANSLATE_NOOP("Status", "Display the minifigure builder preview with flat shading"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Minifigure Builder Flat Shading</b></p><p>Draw the Minifigure Builder preview in plain colors, without shading or lights.</p>"),
		""
	},

	// SW_VIEW_MINIFIGURE_BUILDER_SHADING_DEFAULT_LIGHTS
	{
		QT_TRANSLATE_NOOP("Action", "View.MinifigureBuilder.Shade.DefaultLights"),
		QT_TRANSLATE_NOOP("Menu", "Minifigure Builder Default Lights"),
		QT_TRANSLATE_NOOP("Status", "Display the minifigure builder preview with the default lights"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Minifigure Builder Default Lights</b></p><p>Light the Minifigure Builder preview with the built-in lights.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_MINIFIGURE_BUILDER_EDGE_LINES
	{
		QT_TRANSLATE_NOOP("Action", "View.MinifigureBuilder.ToggleEdgeLines"),
		QT_TRANSLATE_NOOP("Menu", "Minifigure Builder Edge Lines"),
		QT_TRANSLATE_NOOP("Status", "Toggle edge lines in the minifigure builder preview"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Minifigure Builder Edge Lines</b></p><p>Show or hide edge lines in the Minifigure Builder preview.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_MINIFIGURE_BUILDER_CONDITIONAL_LINES
	{
		QT_TRANSLATE_NOOP("Action", "View.MinifigureBuilder.ToggleConditionalLines"),
		QT_TRANSLATE_NOOP("Menu", "Minifigure Builder Conditional Lines"),
		QT_TRANSLATE_NOOP("Status", "Toggle conditional lines in the minifigure builder preview"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Minifigure Builder Conditional Lines</b></p><p>Show or hide conditional lines in the Minifigure Builder preview.</p>"),
		""
	},

	// SW_VIEW_PARTS_TOGGLE_SNAP_TO_TILES
	{
		QT_TRANSLATE_NOOP("Action", "View.Parts.ToggleSnapToTiles"),
		QT_TRANSLATE_NOOP("Menu", "Snap Panel Width to Tiles"),
		QT_TRANSLATE_NOOP("Status", "Toggle snapping the parts panel width to whole thumbnail tiles and the categories to the longest name"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Snap Panel Width to Tiles</b></p><p>Snap the Parts panel width to whole thumbnail tiles, and the categories to the longest name.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_PREVIEW_VIEW_CUBE
	{
		QT_TRANSLATE_NOOP("Action", "View.Preview.ToggleViewCube"),
		QT_TRANSLATE_NOOP("Menu", "Preview View Cube"),
		QT_TRANSLATE_NOOP("Status", "Toggle the view cube in the preview"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Preview View Cube</b></p><p>Show or hide the view cube in the preview.</p>"),
		""
	},

	// SW_VIEW_TOGGLE_PREVIEW_AXIS_ICON
	{
		QT_TRANSLATE_NOOP("Action", "View.Preview.ToggleAxisIcon"),
		QT_TRANSLATE_NOOP("Menu", "Preview Axis Icon"),
		QT_TRANSLATE_NOOP("Status", "Toggle the axis icon in the preview"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Preview Axis Icon</b></p><p>Show or hide the axis icon in the preview.</p>"),
		""
	},

	// SW_FILE_TOGGLE_LOAD_PARTS_BEFORE_SHOWING
	{
		QT_TRANSLATE_NOOP("Action", "File.ToggleLoadPartsBeforeShowing"),
		QT_TRANSLATE_NOOP("Menu", "Load All Parts Before Showing"),
		QT_TRANSLATE_NOOP("Status", "Toggle waiting for every part to load before showing a model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Load All Parts Before Showing</b></p><p>Wait until every part of a model has loaded before showing it, instead of showing parts as they load.</p>"),
		""
	},

	// SW_VIEW_PREVIEW_TOGGLE_SQUARE
	{
		QT_TRANSLATE_NOOP("Action", "View.Preview.ToggleSquareViewport"),
		QT_TRANSLATE_NOOP("Menu", "Square Viewport"),
		QT_TRANSLATE_NOOP("Status", "Keep the preview at a 1:1 ratio"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Square Viewport</b></p><p>Keep the preview square, at a 1:1 ratio.</p>"),
		""
	},

	// SW_VIEW_PREVIEW_TOGGLE_KEEP_SELECTION
	{
		QT_TRANSLATE_NOOP("Action", "View.Preview.ToggleKeepSelection"),
		QT_TRANSLATE_NOOP("Menu", "Keep Last Selection"),
		QT_TRANSLATE_NOOP("Status", "Keep showing the last selected part after the selection is cleared"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Keep Last Selection</b></p><p>Keep showing the last selected part in the preview after the selection is cleared.</p>"),
		""
	},

	// SW_PART_INSERT
	{
		QT_TRANSLATE_NOOP("Action", "Part.Insert"),
		QT_TRANSLATE_NOOP("Menu", "&Insert Part"),
		QT_TRANSLATE_NOOP("Status", "Add a new part to the model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Insert Part</b></p><p>Add the part chosen in the Parts panel to the model, next to the part that has focus or the last part added.</p>"),
		"Insert"
	},

	// SW_PART_TOGGLE_FIRST_AT_ORIGIN
	{
		QT_TRANSLATE_NOOP("Action", "Part.ToggleFirstPartAtOrigin"),
		QT_TRANSLATE_NOOP("Menu", "Insert First Part at Origin"),
		QT_TRANSLATE_NOOP("Status", "Toggle placing the first part of an empty model at 0, 0, 0"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Insert First Part at Origin</b></p><p>Place the first part of an empty model at 0, 0, 0.</p>"),
		""
	},

	// SW_PART_DELETE
	{
		QT_TRANSLATE_NOOP("Action", "Part.Delete"),
		QT_TRANSLATE_NOOP("Menu", "&Delete"),
		QT_TRANSLATE_NOOP("Status", "Delete selected objects"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Delete</b></p><p>Remove the selected objects from the model.</p>"),
		"Delete"
	},

	// SW_PART_DUPLICATE
	{
		QT_TRANSLATE_NOOP("Action", "Part.Duplicate"),
		QT_TRANSLATE_NOOP("Menu", "&Duplicate"),
		QT_TRANSLATE_NOOP("Status", "Create a copy of the selected parts"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Duplicate</b></p><p>Add a copy of each selected part in the same place, with copies of their groups, and select the copies.</p>"),
		"Ctrl+D"
	},

	// SW_PART_PAINT_SELECTED
	{
		QT_TRANSLATE_NOOP("Action", "Part.PaintSelected"),
		QT_TRANSLATE_NOOP("Menu", "&Paint Selected"),
		QT_TRANSLATE_NOOP("Status", "Change the color of the selected parts"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Paint Selected</b></p><p>Change the selected parts to the color chosen in the Colors panel.</p>"),
		""
	},

	// SW_PART_SET_JOINT
	{
		QT_TRANSLATE_NOOP("Action", "Part.SetJoint"),
		QT_TRANSLATE_NOOP("Menu", "&Set Joint"),
		QT_TRANSLATE_NOOP("Status", "Click a connection of the selected submodel to make it a joint"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Set Joint</b></p><p>Select one submodel, choose Part &gt; Joint &gt; Set Joint, also in the right-click menu of the model view, then click one of its connections with the Set Joint pointer, which outlines the part under it in red. The click sets the joint and goes back to the Select tool in joint mode on the new joint. Set Joint again at another connection adds a joint, and Set Joint at a joint already there resets it. A round connection makes a hinge on its axis and a sphere makes a ball. A part without shadow data gets a hinge on its origin. The joint shows in the Joints panel and setting it can be undone.</p>"),
		""
	},

	// SW_PART_JOINT_MOVE
	{
		QT_TRANSLATE_NOOP("Action", "Part.JointMove"),
		QT_TRANSLATE_NOOP("Menu", "&Move Joint"),
		QT_TRANSLATE_NOOP("Status", "Move the selected submodel with the gizmo on its picked joint"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Joint Move</b></p><p>With one submodel with a joint selected, switches to the Move tool with the gizmo on the pivot of the picked joint, outlined by a red hexagon. Selecting something else goes back to the pivot point.</p>"),
		""
	},

	// SW_PART_JOINT_ROTATE
	{
		QT_TRANSLATE_NOOP("Action", "Part.JointRotate"),
		QT_TRANSLATE_NOOP("Menu", "&Rotate Joint"),
		QT_TRANSLATE_NOOP("Status", "Rotate the selected submodel around its picked joint instead of the pivot point"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Joint Rotate</b></p><p>With one submodel with a joint selected, switches to the Rotate tool with the gizmo on the pivot of the picked joint, outlined in red, and the rotate keys and the Rotate tool turn the joint around its own pivot and axis. A hinge only turns about its axis. Selecting something else goes back to the pivot point. Use Joint Rotation in the preferences does this whenever a selected submodel has a joint.</p>"),
		""
	},

	// SW_PART_RESET_PIVOT_POINT
	{
		QT_TRANSLATE_NOOP("Action", "Part.ResetPivotPoint"),
		QT_TRANSLATE_NOOP("Menu", "Reset &Pivot Point"),
		QT_TRANSLATE_NOOP("Status", "Reset the pivot point of the selected parts to their origin"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Reset Pivot Point</b></p><p>Put the pivot point of each selected part back at its origin.</p>"),
		"Ctrl+#"
	},

	// SW_PART_REMOVE_KEY_FRAMES
	{
		QT_TRANSLATE_NOOP("Action", "Part.RemoveKeyFrames"),
		QT_TRANSLATE_NOOP("Menu", "Remove &Keyframes"),
		QT_TRANSLATE_NOOP("Status", "Remove all keyframes from the selected parts"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Remove Keyframes</b></p><p>Remove every animation key from the selected parts, lights and cameras.</p>"),
		""
	},

	// SW_PART_CONTROL_POINT_INSERT
	{
		QT_TRANSLATE_NOOP("Action", "Part.ControlPoint.Insert"),
		QT_TRANSLATE_NOOP("Menu", "Insert Control Point"),
		QT_TRANSLATE_NOOP("Status", "Insert a new control point"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Insert Control Point</b></p><p>Add a control point to the selected flexible part, where it bends.</p>"),
		"Ctrl+["
	},

	// SW_PART_CONTROL_POINT_REMOVE
	{
		QT_TRANSLATE_NOOP("Action", "Part.ControlPoint.Remove"),
		QT_TRANSLATE_NOOP("Menu", "Remove Control Point"),
		QT_TRANSLATE_NOOP("Status", "Remove the selected control point"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Remove Control Point</b></p><p>Remove the selected control point of a flexible part.</p>"),
		"Ctrl+]"
	},

	// SW_PART_CONTROL_POINT_TANGENT
	{
		QT_TRANSLATE_NOOP("Action", "Part.ControlPoint.Tangent"),
		QT_TRANSLATE_NOOP("Menu", "Clamp Tangent"),
		QT_TRANSLATE_NOOP("Status", "Turn the selected control point along the path, or release it to rotate it freely"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Clamp Tangent</b></p><p>Hold the selected control point along the path, or release it to rotate freely.</p>"),
		""
	},

	// SW_PART_CONTROL_POINT_ALIGN_TANGENT
	{
		QT_TRANSLATE_NOOP("Action", "Part.ControlPoint.AlignTangent"),
		QT_TRANSLATE_NOOP("Menu", "Align to Tangent"),
		QT_TRANSLATE_NOOP("Status", "Turn the selected control point along the path once and keep it released"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Align to Tangent</b></p><p>Turn the selected control point along the path once and keep it released.</p>"),
		""
	},

	// SW_PART_CONTROL_POINT_ALIGN_X
	{
		QT_TRANSLATE_NOOP("Action", "Part.ControlPoint.AlignX"),
		QT_TRANSLATE_NOOP("Menu", "X Axis"),
		QT_TRANSLATE_NOOP("Status", "Turn the selected control point along the X axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>X Axis</b></p><p>Turn the selected control point along the X axis.</p>"),
		""
	},

	// SW_PART_CONTROL_POINT_ALIGN_Y
	{
		QT_TRANSLATE_NOOP("Action", "Part.ControlPoint.AlignY"),
		QT_TRANSLATE_NOOP("Menu", "Y Axis"),
		QT_TRANSLATE_NOOP("Status", "Turn the selected control point along the Y axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Y Axis</b></p><p>Turn the selected control point along the Y axis.</p>"),
		""
	},

	// SW_PART_CONTROL_POINT_ALIGN_Z
	{
		QT_TRANSLATE_NOOP("Action", "Part.ControlPoint.AlignZ"),
		QT_TRANSLATE_NOOP("Menu", "Z Axis"),
		QT_TRANSLATE_NOOP("Status", "Turn the selected control point along the Z axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Z Axis</b></p><p>Turn the selected control point along the Z axis.</p>"),
		""
	},

	// SW_PART_TRAIN_TRACK_FOCUS_NEXT
	{
		QT_TRANSLATE_NOOP("Action", "Part.TrainTrack.FocusNext"),
		QT_TRANSLATE_NOOP("Menu", "Focus Next Connection"),
		QT_TRANSLATE_NOOP("Status", "Focus the next connection of the currently selected track"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Focus Next Connection</b></p><p>Move the focus to the next connection of the selected track, where the next part joins.</p>"),
		""
	},

	// SW_PART_TRAIN_TRACK_FOCUS_PREVIOUS
	{
		QT_TRANSLATE_NOOP("Action", "Part.TrainTrack.FocusPrevious"),
		QT_TRANSLATE_NOOP("Menu", "Focus Previous Connection"),
		QT_TRANSLATE_NOOP("Status", "Focus the previous connection of the currently selected track"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Focus Previous Connection</b></p><p>Move the focus to the previous connection of the selected track, where the next part joins.</p>"),
		""
	},

	// SW_PART_TRAIN_TRACK_ROTATE
	{
		QT_TRANSLATE_NOOP("Action", "Part.TrainTrack.Rotate"),
		QT_TRANSLATE_NOOP("Menu", "Rotate Train Track"),
		QT_TRANSLATE_NOOP("Status", "Rotate the focused train track around its last connection"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotate Train Track</b></p><p>Turn the focused track part to its next connection.</p>"),
		""
	},

	// SW_PART_MOVE_PLUS_X
	{
		QT_TRANSLATE_NOOP("Action", "Part.Move.PlusX"),
		QT_TRANSLATE_NOOP("Menu", "Move +X"),
		QT_TRANSLATE_NOOP("Status", "Move selected objects along the X axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move Selection</b></p><p>Move the selected objects one move snap step along this axis.</p>"),
		"Right"
	},

	// SW_PART_MOVE_MINUS_X
	{
		QT_TRANSLATE_NOOP("Action", "Part.Move.MinusX"),
		QT_TRANSLATE_NOOP("Menu", "Move -X"),
		QT_TRANSLATE_NOOP("Status", "Move selected objects along the X axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move Selection</b></p><p>Move the selected objects one move snap step along this axis.</p>"),
		"Left"
	},

	// SW_PART_MOVE_PLUS_Y
	{
		QT_TRANSLATE_NOOP("Action", "Part.Move.PlusY"),
		QT_TRANSLATE_NOOP("Menu", "Move +Y"),
		QT_TRANSLATE_NOOP("Status", "Move selected objects along the Y axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move Selection</b></p><p>Move the selected objects one move snap step along this axis.</p>"),
		"Up"
	},

	// SW_PART_MOVE_MINUS_Y
	{
		QT_TRANSLATE_NOOP("Action", "Part.Move.MinusY"),
		QT_TRANSLATE_NOOP("Menu", "Move -Y"),
		QT_TRANSLATE_NOOP("Status", "Move selected objects along the Y axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move Selection</b></p><p>Move the selected objects one move snap step along this axis.</p>"),
		"Down"
	},

	// SW_PART_MOVE_PLUS_Z
	{
		QT_TRANSLATE_NOOP("Action", "Part.Move.PlusZ"),
		QT_TRANSLATE_NOOP("Menu", "Move +Z"),
		QT_TRANSLATE_NOOP("Status", "Move selected objects along the Z axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move Selection</b></p><p>Move the selected objects one move snap step along this axis.</p>"),
		"Alt+Up"
	},

	// SW_PART_MOVE_MINUS_Z
	{
		QT_TRANSLATE_NOOP("Action", "Part.Move.MinusZ"),
		QT_TRANSLATE_NOOP("Menu", "Move -Z"),
		QT_TRANSLATE_NOOP("Status", "Move selected objects along the Z axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move Selection</b></p><p>Move the selected objects one move snap step along this axis.</p>"),
		"Alt+Down"
	},

	// SW_PART_ROTATE_PLUS_X
	{
		QT_TRANSLATE_NOOP("Action", "Part.Rotate.PlusX"),
		QT_TRANSLATE_NOOP("Menu", "Rotate +X"),
		QT_TRANSLATE_NOOP("Status", "Rotate selected objects around the X axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotate Selection</b></p><p>Rotate the selected objects one rotation snap step around this axis.</p>"),
		"Ctrl+Down"
	},

	// SW_PART_ROTATE_MINUS_X
	{
		QT_TRANSLATE_NOOP("Action", "Part.Rotate.MinusX"),
		QT_TRANSLATE_NOOP("Menu", "Rotate -X"),
		QT_TRANSLATE_NOOP("Status", "Rotate selected objects around the X axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotate Selection</b></p><p>Rotate the selected objects one rotation snap step around this axis.</p>"),
		"Ctrl+Up"
	},

	// SW_PART_ROTATE_PLUS_Y
	{
		QT_TRANSLATE_NOOP("Action", "Part.Rotate.PlusY"),
		QT_TRANSLATE_NOOP("Menu", "Rotate +Y"),
		QT_TRANSLATE_NOOP("Status", "Rotate selected objects around the Y axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotate Selection</b></p><p>Rotate the selected objects one rotation snap step around this axis.</p>"),
		"Ctrl+Shift+Left"
	},

	// SW_PART_ROTATE_MINUS_Y
	{
		QT_TRANSLATE_NOOP("Action", "Part.Rotate.MinusY"),
		QT_TRANSLATE_NOOP("Menu", "Rotate -Y"),
		QT_TRANSLATE_NOOP("Status", "Rotate selected objects around the Y axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotate Selection</b></p><p>Rotate the selected objects one rotation snap step around this axis.</p>"),
		"Ctrl+Shift+Right"
	},

	// SW_PART_ROTATE_PLUS_Z
	{
		QT_TRANSLATE_NOOP("Action", "Part.Rotate.PlusZ"),
		QT_TRANSLATE_NOOP("Menu", "Rotate +Z"),
		QT_TRANSLATE_NOOP("Status", "Rotate selected objects around the Z axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotate Selection</b></p><p>Rotate the selected objects one rotation snap step around this axis.</p>"),
		"Ctrl+Left"
	},

	// SW_PART_ROTATE_MINUS_Z
	{
		QT_TRANSLATE_NOOP("Action", "Part.Rotate.MinusZ"),
		QT_TRANSLATE_NOOP("Menu", "Rotate -Z"),
		QT_TRANSLATE_NOOP("Status", "Rotate selected objects around the Z axis"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Rotate Selection</b></p><p>Rotate the selected objects one rotation snap step around this axis.</p>"),
		"Ctrl+Right"
	},

	// SW_PART_MINIFIGURE_BUILDER
	{
		QT_TRANSLATE_NOOP("Action", "Part.MinifigureBuilder"),
		QT_TRANSLATE_NOOP("Menu", "&Minifigure Builder"),
		QT_TRANSLATE_NOOP("Status", "Show the Minifigure Builder panel to build a minifigure and add it to the model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Minifigure Builder</b></p><p>Show the Minifigure Builder panel to build a minifigure from templates and add it to the model.</p>"),
		""
	},

	// SW_PART_ARRAY
	{
		QT_TRANSLATE_NOOP("Action", "Part.Array"),
		QT_TRANSLATE_NOOP("Menu", "A&rray..."),
		QT_TRANSLATE_NOOP("Status", "Make copies of the selected parts"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Array</b></p><p>Repeat the selection along one, two or three axes, with a count, offset and rotation for each.</p>"),
		""
	},

	// SW_PART_VIEW_SELECTED_MODEL
	{
		QT_TRANSLATE_NOOP("Action", "Part.ViewSelectedModel"),
		QT_TRANSLATE_NOOP("Menu", "Open Selected Submodel"),
		QT_TRANSLATE_NOOP("Status", "Open the submodel referenced by the selected part in a new tab"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Open Selected Submodel</b></p><p>Open the submodel used by the selected part in its own tab, to edit it on its own.</p>"),
		"Ctrl+Shift+O"
	},

	// SW_PART_MOVE_SELECTION_TO_MODEL
	{
		QT_TRANSLATE_NOOP("Action", "Part.MoveSelectionToModel"),
		QT_TRANSLATE_NOOP("Menu", "Move to New Submodel..."),
		QT_TRANSLATE_NOOP("Status", "Move the selected parts to a new submodel and replace them with a reference to it"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move to New Submodel</b></p><p>Move the selected parts into a new submodel and put one reference to it in their place.</p>"),
		"Ctrl+Alt+N"
	},

	// SW_PART_INLINE_SELECTED_MODELS
	{
		QT_TRANSLATE_NOOP("Action", "Part.InlineSelectedModels"),
		QT_TRANSLATE_NOOP("Menu", "Inline Selected Submodels"),
		QT_TRANSLATE_NOOP("Status", "Insert the contents of the selected submodels into the current model"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Inline Selected Submodels</b></p><p>Replace each selected submodel with its parts, in place.</p>"),
		"Ctrl+Shift+I"
	},

	// SW_PART_FLATTEN_MODEL
	{
		QT_TRANSLATE_NOOP("Action", "Part.FlattenModel"),
		QT_TRANSLATE_NOOP("Menu", "Flatten Submodels"),
		QT_TRANSLATE_NOOP("Status", "Replace every submodel, and the submodels inside it, with its parts in one step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Flatten Submodels</b></p><p>Replace every submodel in the current model, and every submodel inside those, with its parts in one undo step. Each part keeps its place and color, and the groups inside the submodels are not kept.</p>"),
		"Ctrl+Shift+F"
	},

	// SW_PART_EDIT_SELECTED_SUBMODEL
	{
		QT_TRANSLATE_NOOP("Action", "Part.EditSelectedSubmodel"),
		QT_TRANSLATE_NOOP("Menu", "Edit Selected Submodel"),
		QT_TRANSLATE_NOOP("Status", "Edit the currently selected submodel in-place"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Edit Selected Submodel</b></p><p>Edit the selected submodel in place, inside its parent. A path bar shows where you are.</p>"),
		"Ctrl+Shift+E"
	},

	// SW_PART_EDIT_END_SUBMODEL
	{
		QT_TRANSLATE_NOOP("Action", "Part.EditEndSubmodel"),
		QT_TRANSLATE_NOOP("Menu", "End Submodel Editing"),
		QT_TRANSLATE_NOOP("Status", "End in-place submodel editing"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>End Submodel Editing</b></p><p>Leave in-place editing and go back to the parent model.</p>"),
		"Ctrl+Shift+W"
	},

	// SW_PART_GROUP
	{
		QT_TRANSLATE_NOOP("Action", "Part.Group"),
		QT_TRANSLATE_NOOP("Menu", "&Group..."),
		QT_TRANSLATE_NOOP("Status", "Group selected parts together"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Group</b></p><p>Group the selected parts under a unique name, so they select and move as one.</p>"),
		"Ctrl+G"
	},

	// SW_PART_UNGROUP
	{
		QT_TRANSLATE_NOOP("Action", "Part.Ungroup"),
		QT_TRANSLATE_NOOP("Menu", "&Ungroup"),
		QT_TRANSLATE_NOOP("Status", "Ungroup selected group"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Ungroup</b></p><p>Remove the selected group, keeping its parts.</p>"),
		"Ctrl+Shift+G"
	},

	// SW_PART_GROUP_ADD
	{
		QT_TRANSLATE_NOOP("Action", "Part.GroupAdd"),
		QT_TRANSLATE_NOOP("Menu", "&Add to Group"),
		QT_TRANSLATE_NOOP("Status", "Add focused part to selected group"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Add to Group</b></p><p>Add the part that has focus to the selected group.</p>"),
		""
	},

	// SW_PART_GROUP_REMOVE
	{
		QT_TRANSLATE_NOOP("Action", "Part.GroupRemove"),
		QT_TRANSLATE_NOOP("Menu", "Re&move from Group"),
		QT_TRANSLATE_NOOP("Status", "Remove focused part from group"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Remove from Group</b></p><p>Take the part that has focus out of its group.</p>"),
		""
	},

	// SW_PART_GROUP_EDIT
	{
		QT_TRANSLATE_NOOP("Action", "Part.GroupEdit"),
		QT_TRANSLATE_NOOP("Menu", "&Edit Groups..."),
		QT_TRANSLATE_NOOP("Status", "Edit groups"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Edit Groups</b></p><p>Create, rename and nest groups and drag parts between them in a tree of parts and groups.</p>"),
		"Ctrl+Alt+Shift+G"
	},

	// SW_PART_HIDE_SELECTED
	{
		QT_TRANSLATE_NOOP("Action", "Part.HideSelected"),
		QT_TRANSLATE_NOOP("Menu", "&Hide Selected"),
		QT_TRANSLATE_NOOP("Status", "Hide selected objects"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Hide Selected</b></p><p>Hide the selected objects.</p>"),
		"Ctrl+H"
	},

	// SW_PART_HIDE_UNSELECTED
	{
		QT_TRANSLATE_NOOP("Action", "Part.HideUnselected"),
		QT_TRANSLATE_NOOP("Menu", "Hide &Unselected"),
		QT_TRANSLATE_NOOP("Status", "Hide objects that are not selected"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Hide Unselected</b></p><p>Hide every object that is not selected.</p>"),
		"Ctrl+Alt+Shift+H"
	},

	// SW_PART_UNHIDE_SELECTED
	{
		QT_TRANSLATE_NOOP("Action", "Part.UnhideSelected"),
		QT_TRANSLATE_NOOP("Menu", "&Unhide Selected"),
		QT_TRANSLATE_NOOP("Status", "Show hidden objects that are selected"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Unhide Selected</b></p><p>Show the selected hidden objects again.</p>"),
		"Ctrl+Shift+H"
	},

	// SW_PART_UNHIDE_ALL
	{
		QT_TRANSLATE_NOOP("Action", "Part.UnhideAll"),
		QT_TRANSLATE_NOOP("Menu", "U&nhide All"),
		QT_TRANSLATE_NOOP("Status", "Show all hidden objects"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Unhide All</b></p><p>Show every hidden object again.</p>"),
		"Ctrl+Alt+H"
	},

	// SW_PART_SHOW_EARLIER
	{
		QT_TRANSLATE_NOOP("Action", "Part.ShowEarlier"),
		QT_TRANSLATE_NOOP("Menu", "Show Earlier"),
		QT_TRANSLATE_NOOP("Status", "Show selected parts one step earlier"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Show Earlier</b></p><p>Move the selected parts one step earlier.</p>"),
		""
	},

	// SW_PART_SHOW_LATER
	{
		QT_TRANSLATE_NOOP("Action", "Part.ShowLater"),
		QT_TRANSLATE_NOOP("Menu", "Show Later"),
		QT_TRANSLATE_NOOP("Status", "Show selected parts one step later"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Show Later</b></p><p>Move the selected parts one step later.</p>"),
		""
	},

	// SW_MODEL_NEW
	{
		QT_TRANSLATE_NOOP("Action", "Model.New"),
		QT_TRANSLATE_NOOP("Menu", "New Submodel..."),
		QT_TRANSLATE_NOOP("Status", "Create a new submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>New Submodel</b></p><p>Add a new, empty submodel to the file and switch to it.</p>"),
		"Alt+Shift+N"
	},

	// SW_MODEL_PROPERTIES
	{
		QT_TRANSLATE_NOOP("Action", "Model.Properties"),
		QT_TRANSLATE_NOOP("Menu", "Proper&ties..."),
		QT_TRANSLATE_NOOP("Status", "Display the properties of the current submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Properties</b></p><p>Edit the name, author, description and comments of the current submodel, and see its parts, counts, size and weight, with BrickLink and CSV export.</p>"),
		""
	},

	// SW_MODEL_LIST
	{
		QT_TRANSLATE_NOOP("Action", "Model.List"),
		QT_TRANSLATE_NOOP("Menu", "Submodels..."),
		QT_TRANSLATE_NOOP("Status", "Show a list of all submodels"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodels</b></p><p>Create, rename, duplicate, delete, reorder and export the submodels of the file, and set the active one.</p>"),
		"Alt+Shift+M"
	},

	// SW_MODEL_01
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model01"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 1"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_02
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model02"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 2"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_03
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model03"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 3"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_04
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model04"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 4"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_05
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model05"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 5"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_06
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model06"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 6"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_07
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model07"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 7"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_08
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model08"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 8"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_09
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model09"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 9"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_10
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model10"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 10"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_11
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model11"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 11"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_12
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model12"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 12"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_13
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model13"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 13"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_14
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model14"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 14"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_15
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model15"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 15"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_16
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model16"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 16"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_17
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model17"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 17"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_18
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model18"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 18"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_19
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model19"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 19"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_20
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model20"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 20"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_21
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model21"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 21"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_22
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model22"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 22"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_23
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model23"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 23"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_24
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model24"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 24"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_25
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model25"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 25"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_26
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model26"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 26"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_27
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model27"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 27"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_28
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model28"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 28"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_29
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model29"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 29"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_30
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model30"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 30"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_31
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model31"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 31"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_32
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model32"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 32"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_33
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model33"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 33"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_34
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model34"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 34"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_35
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model35"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 35"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_36
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model36"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 36"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_37
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model37"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 37"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_38
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model38"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 38"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_39
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model39"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 39"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_MODEL_40
	{
		QT_TRANSLATE_NOOP("Action", "Model.Model40"),
		QT_TRANSLATE_NOOP("Menu", "Submodel 40"),
		QT_TRANSLATE_NOOP("Status", "Switch to this submodel"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Submodel</b></p><p>Switch to this submodel. The Models menu lists up to forty submodels of the file.</p>"),
		""
	},

	// SW_HELP_WHATS_THIS
	{
		QT_TRANSLATE_NOOP("Action", "Help.WhatsThis"),
		QT_TRANSLATE_NOOP("Menu", "&What's This?"),
		QT_TRANSLATE_NOOP("Status", "Click a control to read its contextual help"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>What's This?</b></p><p>Click any control after this to read what it does. F1 explains the focused control.</p>"),
		"Shift+F1"
	},

	// SW_HELP_HOME_PAGE
	{
		QT_TRANSLATE_NOOP("Action", "Help.HomePage"),
		QT_TRANSLATE_NOOP("Menu", "&Home Page"),
		QT_TRANSLATE_NOOP("Status", "Open the StudWorx home page in your default web browser"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Home Page</b></p><p>Open the StudWorx repository on GitHub in your web browser.</p>"),
		""
	},

	// SW_HELP_BUG_REPORT
	{
		QT_TRANSLATE_NOOP("Action", "Help.BugReport"),
		QT_TRANSLATE_NOOP("Menu", "Report a &Bug"),
		QT_TRANSLATE_NOOP("Status", "Open the StudWorx bug report form in your default web browser"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Report a Bug</b></p><p>Open the StudWorx issues on GitHub in your web browser to report a problem.</p>"),
		""
	},

	// SW_HELP_ABOUT
	{
		QT_TRANSLATE_NOOP("Action", "Help.About"),
		QT_TRANSLATE_NOOP("Menu", "&About..."),
		QT_TRANSLATE_NOOP("Status", "Display program version number and system information"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>About</b></p><p>Show the StudWorx version, a link to its home page, and system information to copy into a bug report.</p>"),
		""
	},

	// SW_TIMELINE_INSERT_BEFORE
	{
		"",
		QT_TRANSLATE_NOOP("Menu", "Insert Step Before"),
		QT_TRANSLATE_NOOP("Status", "Insert a new step before the current step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Insert Step Before</b></p><p>Add an empty step before this step in the Timeline.</p>"),
		""
	},

	// SW_TIMELINE_INSERT_AFTER
	{
		"",
		QT_TRANSLATE_NOOP("Menu", "Insert Step After"),
		QT_TRANSLATE_NOOP("Status", "Insert a new step after the current step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Insert Step After</b></p><p>Add an empty step after this step in the Timeline.</p>"),
		""
	},

	// SW_TIMELINE_DELETE
	{
		"",
		QT_TRANSLATE_NOOP("Menu", "Delete Step"),
		QT_TRANSLATE_NOOP("Status", "Delete the current step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Delete Step</b></p><p>Remove this step. Its parts join the step before it and later steps move one step earlier.</p>"),
		""
	},

	// SW_TIMELINE_MOVE_SELECTION
	{
		"",
		QT_TRANSLATE_NOOP("Menu", "Move Selection Here"),
		QT_TRANSLATE_NOOP("Status", "Move the selected parts into this step"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move Selection Here</b></p><p>Move the selected parts into this step.</p>"),
		""
	},

	// SW_TIMELINE_MOVE_SELECTION_BEFORE
	{
		"",
		QT_TRANSLATE_NOOP("Menu", "Move Selection Before"),
		QT_TRANSLATE_NOOP("Status", "Move the selected parts into a new step before this"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move Selection Before</b></p><p>Move the selected parts into a new step before this one.</p>"),
		""
	},

	// SW_TIMELINE_MOVE_SELECTION_AFTER
	{
		"",
		QT_TRANSLATE_NOOP("Menu", "Move Selection After"),
		QT_TRANSLATE_NOOP("Status", "Move the selected parts into a new step after this"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Move Selection After</b></p><p>Move the selected parts into a new step after this one.</p>"),
		""
	},

	// SW_TIMELINE_SET_CURRENT
	{
		"",
		QT_TRANSLATE_NOOP("Menu", "Set Current Step"),
		QT_TRANSLATE_NOOP("Status", "View the model at this point in the timeline"),
		QT_TRANSLATE_NOOP("WhatsThis", "<p><b>Set Current Step</b></p><p>Show the model at this step.</p>"),
		""
	}
};

SW_ARRAY_SIZE_CHECK(gCommands, SW_COMMAND_COUNT);

const char* gToolNames[] =
{
	QT_TRANSLATE_NOOP("Mouse", "NewPart"),            // swTool::Insert
	QT_TRANSLATE_NOOP("Mouse", "NewPointLight"),       // swTool::PointLight
	QT_TRANSLATE_NOOP("Mouse", "NewSpotLight"),        // swTool::SpotLight
	QT_TRANSLATE_NOOP("Mouse", "NewDirectionalLight"), // swTool::DirectionalLight
	QT_TRANSLATE_NOOP("Mouse", "NewAreaLight"),        // swTool::AreaLight
	QT_TRANSLATE_NOOP("Mouse", "NewCamera"),           // swTool::Camera
	QT_TRANSLATE_NOOP("Mouse", "Select"),              // swTool::Select
	QT_TRANSLATE_NOOP("Mouse", "Move"),                // swTool::Move
	QT_TRANSLATE_NOOP("Mouse", "Rotate"),              // swTool::Rotate
	QT_TRANSLATE_NOOP("Mouse", "Erase"),               // swTool::Erase
	QT_TRANSLATE_NOOP("Mouse", "Paint"),               // swTool::Paint
	QT_TRANSLATE_NOOP("Mouse", "Joints"),                // swTool::Joints
	QT_TRANSLATE_NOOP("Mouse", "ColorPicker"),         // swTool::ColorPicker
	QT_TRANSLATE_NOOP("Mouse", "Zoom"),                // swTool::Zoom
	QT_TRANSLATE_NOOP("Mouse", "Pan"),                 // swTool::Pan
	QT_TRANSLATE_NOOP("Mouse", "Orbit"),               // swTool::RotateView
	QT_TRANSLATE_NOOP("Mouse", "Roll"),                // swTool::Roll
	QT_TRANSLATE_NOOP("Mouse", "ZoomRegion")           // swTool::ZoomRegion
};

SW_ARRAY_SIZE_CHECK(gToolNames, swTool::Count);

QStringList swGetDefaultContextMenu()
{
	// names for the translation tools
	[[maybe_unused]] static const char* const TranslatedNames[] =
	{
		QT_TRANSLATE_NOOP("ContextMenu", "Submodel"),
		QT_TRANSLATE_NOOP("ContextMenu", "Control Points"),
		QT_TRANSLATE_NOOP("ContextMenu", "Align to Axis"),
		QT_TRANSLATE_NOOP("ContextMenu", "View"),
	};

	return QStringList(
	{
		"Edit.Cut", "Edit.Copy", "Edit.Paste", "Edit.PasteSteps",
		"-",
		"Part.Duplicate", "Part.Delete",
		"-",
		">Submodel",
		"\tPart.EditSelectedSubmodel", "\tPart.EditEndSubmodel", "\tPart.ViewSelectedModel", "\tPart.InlineSelectedModels", "\tPart.FlattenModel", "\tPart.MoveSelectionToModel",
		"@Joint",
		">Control Points",
		"\tPart.ControlPoint.Insert", "\tPart.ControlPoint.Remove", "\tPart.ControlPoint.AlignTangent",
		"\t>Align to Axis",
		"\t\tPart.ControlPoint.AlignX", "\t\tPart.ControlPoint.AlignY", "\t\tPart.ControlPoint.AlignZ",
		"\tPart.ControlPoint.Tangent",
		"-",
		"@Tools",
		"-",
		"View.NormalToFace", "@Viewpoint", "@Camera", "@Projection", "@Shading", "@Cutaway",
		"-",
		">View",
		"\tView.SplitHorizontal", "\tView.SplitVertical", "\tView.RemoveView", "\tView.ResetViews"
	});
}

const QStringList& swGetContextMenuBuiltins()
{
	// names for the translation tools
	[[maybe_unused]] static const char* const TranslatedNames[] =
	{
		QT_TRANSLATE_NOOP("ContextMenu", "Import"),
		QT_TRANSLATE_NOOP("ContextMenu", "Export"),
		QT_TRANSLATE_NOOP("ContextMenu", "Render"),
		QT_TRANSLATE_NOOP("ContextMenu", "Selection Mode"),
		QT_TRANSLATE_NOOP("ContextMenu", "Transform"),
		QT_TRANSLATE_NOOP("ContextMenu", "Tools"),
		QT_TRANSLATE_NOOP("ContextMenu", "Toolbars"),
		QT_TRANSLATE_NOOP("ContextMenu", "Viewpoint"),
		QT_TRANSLATE_NOOP("ContextMenu", "Cutaway"),
		QT_TRANSLATE_NOOP("ContextMenu", "Camera"),
		QT_TRANSLATE_NOOP("ContextMenu", "Projection"),
		QT_TRANSLATE_NOOP("ContextMenu", "Shading"),
		QT_TRANSLATE_NOOP("ContextMenu", "Joint"),
		QT_TRANSLATE_NOOP("ContextMenu", "Group"),
		QT_TRANSLATE_NOOP("ContextMenu", "Visibility"),
	};

	// window submenus by object name

	static const QStringList Builtins({ "Import", "Export", "Render", "Selection Mode", "Transform", "Tools", "Toolbars", "Viewpoint", "Cutaway", "Camera", "Projection", "Shading", "Joint", "Group", "Visibility" });
	return Builtins;
}

int swFindCommand(const QString& ID)
{
	for (int CommandIndex = 0; CommandIndex < SW_COMMAND_COUNT; CommandIndex++)
		if (gCommands[CommandIndex].ID[0] && ID == QLatin1String(gCommands[CommandIndex].ID))
			return CommandIndex;

	return -1;
}
