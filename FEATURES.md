# Features

Every StudWorx feature, by area. `tools/studworx-site` builds the feature sections of [studworx.app](https://studworx.app) from this file. Each `##` section starts with a comment that sets its page id, icon and heading, and its first paragraph is the lead. A `<!-- gallery class -->` comment turns the images and video links below it into one gallery, and the title of a single image sets its figure class. A `<!-- video -->` comment turns the link below it into a looping video, with the link text as its caption.

## Interface

<!-- interface | console | INTERFACE // MENUS, TOOLBARS AND HELP -->

StudWorx is built from standard Qt widgets: panels, dialogs and windows follow your system style and light or dark theme, and only the model views draw with OpenGL. Menus are grouped by task, every icon and cursor is drawn in code so it stays sharp at any size, and every dialog and panel explains itself.

- **Menus by task.** File, Edit, View, Part, Steps, Models and Help, with clean labels and a unique mnemonic in each menu. Edit holds the Selection Mode, Transform and Tools submenus, View the Toolbars, Viewpoints, Cutaway, Cameras, Projection and Shading submenus, and Part the Joint, Group and Visibility submenus.
- **Options menus.** A gear button opens the settings that belong to its place: grids, steps and highlighting and view overlays on the model view, list display and filtering on Parts, the insert color on Colors, overlays and selection on Preview, shading and lines on the Minifigure Builder, the model folder on Terminal, editing on the Editor and page setup on the instructions Properties.
- **Toolbars.** Standard, Tools and Time toolbars. Move and rotate cursors show the axis they work on.
- **Transform toolbar.** Type a position or rotation for the selection and apply it, as absolute coordinates or relative to where it is now. It has no entry of its own: its fields sit at the bottom of the Properties panel, shown with View > Toolbars > Properties.
- **Tabs, splits and status bar.** Open several models in tabs, close one with Close Tab (Ctrl+W) or the others from the tab's right-click menu, split the view and go Full Screen. The status bar shows the selection, the view orientation and projection, the focus position, the snap and transform settings and the current step. The selection reads the name, ID and weight of the selected part, a submodel reading ID N/A and the total weight of its parts when all of them have one, and the total weight of two or more selected parts. Right click the status bar to show or hide each segment.
- **Split views.** Split the current view horizontally or vertically, as often as you like, remove one view, or Reset Views back to one. Each view keeps its own camera and projection, a border marks the active one, and the view cube and axis icon stay on the top left view.
- **Toolbars and panels on demand.** View > Toolbars shows or hides each toolbar and panel: Standard, Tools, Time, Parts, Colors, Properties, Timeline, Preview, Terminal, Minifigure Builder, Explode and Joints.
- **Undo and clipboard.** Undo and Redo every change, and Cut, Copy and Paste parts between models and tabs.
- **Contextual help.** Choose Help > "What's This?" or click the question mark in a title bar, then click any control to read what it does. F1 explains the focused control, Shift+F1 starts "What's This?" inside a dialog, and every entry is listed in the Help index below.
- **Help menu.** Home Page opens the StudWorx repository on GitHub, Report a Bug its issues, and About shows the version and system information.
- **Ten languages.** English, French, German, Spanish, Portuguese, Czech, Ukrainian, Russian, Simplified Chinese and Traditional Chinese.
- **Color theme.** Follow the system, or force the light or dark theme, from Preferences.

<!-- gallery scroll -->

![File](resources/images/interface-menu-01.png)
![Edit](resources/images/interface-menu-02.png)
![View](resources/images/interface-menu-03.png)
![Part](resources/images/interface-menu-04.png)
![Steps](resources/images/interface-menu-05.png)
![Models](resources/images/interface-menu-06.png)
![Help](resources/images/interface-menu-07.png)
![Viewpoints](resources/images/interface-menu-08.png)
![Cutaway](resources/images/interface-menu-09.png)
![Projection](resources/images/interface-menu-10.png)
![Shading](resources/images/interface-menu-11.png)
![View options](resources/images/interface-menu-12.png)
![Preview options](resources/images/interface-menu-13.png)
![Parts options](resources/images/interface-menu-14.png)
![Colors options](resources/images/interface-menu-15.png)
![Minifigure Builder options](resources/images/interface-menu-16.png)
![The main window with the model view, Parts, Colors, Properties and Timeline panels](resources/images/interface-window-01.png)
![Standard, Tools and Time toolbars](resources/images/interface-window-02.png)
![Status bar with the selected part](resources/images/interface-window-03.png)
![Split views](resources/images/interface-window-04.png)

## Files

<!-- files | grey | FILES // OPEN, IMPORT, EXPORT AND PRINT -->

Models are plain LDraw text files in LeoCAD's format, so they open in either program. StudWorx reads and writes them, imports from other tools, exports to 3D and parts list formats, and prints the current view.

<!-- gallery one -->

![Model thumbnails in Dolphin](resources/images/files-example-01.png)

- **LDraw models.** .ldr, .mpd and .dat files, with New, Open, Merge, Save, Save As, Reload and the four most recent files. Save As follows the LDraw file specification and adds the extension when it is left out: .ldr for a single model, .mpd for a model with submodels or embedded data. Files are written with Unix line endings, and a model loaded with Windows line endings keeps them when saved. Watch for File Changes reloads a model edited in another program. Sample models, a minifigure, cube, sphere and cylinder cutaways of it, an exploded model, a flexible part, a posable minifigure with joints, lights and cameras, and a scene with POV-Ray lights, floor and camera, install to /usr/share/studworx/samples, the samples folder beside the program on Windows, or the application bundle resources on macOS.
- **Startup.** Open Most Recent File on Startup reopens the last model, Restore Tab Layout brings back its tabs and split views, and Load All Parts Before Showing waits until every part is loaded before the model appears.
- **LeoCAD models.** Models saved by LeoCAD open as they are, and their LeoCAD meta commands are migrated to StudWorx ones, automatically when you choose so in Preferences. Older LeoCAD projects (.lcd) open and merge too, and save as LDraw.
- **Import.** LEGO® Digital Designer models (.lxf and .lxfml), also opened with Open, recent files, drag and drop and the command line as an untitled project, with a bundled part and color mapping, and official set inventories. Set inventories use your own free Rebrickable API key, asked for once or set under Online Services in the preferences, and are searched by set name or number.
- **Export.** 3D Studio, COLLADA, Wavefront, KMZ, POV-Ray, HTML pages from the Instructions window, BrickLink wanted lists and CSV parts lists. BrickLink exports pick the condition, the wanted list by its number or the address of its page, and new listing notices, copy the XML and open the BrickLink upload page in one step, and translate part numbers and colors through a bundled mapping of over 55,000 parts or a file built from the library and Rebrickable, with Rebrickable numbers taking the lead over stale part keywords. BrickLink and CSV exports list parts the library moved to a new number under that number, and Part Exceptions in the BrickLink export keeps a comma or tab separated list of parts whose BrickLink number you set by hand.
- **KMZ.** Google Earth and SketchUp models, with the visible surfaces only or every part's geometry, set in Preferences.
- **COLLADA.** Translucent colors can be marked as transparent materials for programs that read them.
- **HTML.** Export HTML in the Instructions window writes styled pages in a folder named after the model, opening at index.html, with light and dark colors that follow the browser. One scrolling page of the Instructions window pages that stops at each page, with #page-N and #step-N links and First, Previous, Next and Last Page arrows on the right, left or bottom edge, laid out for the Instructions Page, a Landscape Display, Portrait Display or Tablet shape in pixels and saved at twice that size for sharp high density screens, or one step per page with an index, step images of any width and height on a background color you choose, transparent with its alpha, framed in rounded tiles of the same color, and a parts list after each step or at the end, which Override Instructions Settings also applies to the scrolling page with Settings Overrides for the page, cover, Bill of Materials, step, text, divider, pointer, fade, highlight and element box colors, without changing the model, for the current model, its submodels or all models. Edit Style Sheet changes the CSS of the pages in an editor with the options and find and replace of the Editor panel, its comments naming the feature of each block.
- **Print.** File > Print prints the current model view through the system print dialog, with a Preview button where the dialog allows it.
- **Thumbnails.** Model thumbnails in file managers, from a KDE thumbnail plugin or the desktop thumbnailer.
- **Parts library.** Downloaded on the first start as the official LDraw complete.zip, or chosen from an archive you already have, and reloaded from the File menu. An unofficial parts folder can be added beside it.
- **Command line.** Render images, transparent images with --transparent and POV-Ray renders, and import or export settings, shortcuts and categories without opening a window. See man studworx. [L²CU](https://l2cu.app), the LDraw command line utility for Linux based systems, drives these renders and exports for whole folders of models.
- **Exit.** File > Exit asks to save each changed model before StudWorx closes.

### Command line

studworx model.ldr opens a model. The options below work without opening a window, and man studworx describes each one.

| Options | What they do |
| --- | --- |
| -l, --library, --unofficial-library, --shadow-library | Choose the parts, unofficial parts and LDCad shadow libraries |
| -i, --image, -w, --width, --height | Save an image of the model at a size |
| -f, --from <step>, -t, --to <step>, -s, --submodel | Save a range of steps, or one submodel |
| -c, --camera, --viewpoint, --camera-angles, --camera-position, --camera-position-ldraw | Choose a camera, a viewpoint or an exact camera position |
| --projection, --field-of-view, --clip-planes, --transparent | Projection, field of view, clipping planes and a transparent background |
| --fade-steps, --no-fade-steps, --fade-color, --highlight, --no-highlight, --highlight-color | Fade previous steps and highlight new parts |
| --shading, --lighting, --line-width, --antialiasing-samples | Shading mode, lighting style, edge line width and anti-aliasing |
| --stud-style <id> | One of the eight stud styles, 0 to 7 |
| --edge-color, --edge-color-black, --edge-color-dark, --stud-cylinder-color, their --no- forms, --edge-color-automatic, --edge-contrast, --edge-light-dark | Edge, black edge, dark edge and stud cylinder colors, automated edge colors, contrast and light to dark value |
| --bricklink-condition, --bricklink-wanted-list, --bricklink-notify | Set the condition wanted, the wanted list to add to, and ask BrickLink to notify new listings |
| --bricklink-exceptions | Use a comma or tab separated file of parts and their BrickLink numbers in BrickLink exports |
| --flatten | Replace every submodel at all depths with its parts and save the model, before any other export |
| --undo | Undo every command line edit and save the model, to check that undo restores it |
| --export-wavefront, --export-3ds, --export-collada, --export-kmz, --export-csv, --export-bricklink, --export-html, --export-povray | Export Wavefront, 3D Studio, COLLADA, KMZ, CSV, BrickLink, HTML and POV-Ray files, each with an optional file name |
| --kmz-geometry | Keep every part or only the visible surfaces in KMZ exports |
| --render-povray | Render with POV-Ray, with an optional file name |
| --import-settings, --export-settings | Settings as an ini file |
| --import-keyboard-shortcuts, --export-keyboard-shortcuts, --import-mouse-shortcuts, --export-mouse-shortcuts | Keyboard and mouse shortcuts |
| --import-categories, --export-categories | Part categories |
| -v, --version, -h, --help | Version and usage |

<!-- gallery scroll -->

![Import menu](resources/images/files-menu-01.png)
![Export menu](resources/images/files-menu-02.png)
![LeoCAD meta command migration](resources/images/files-window-01.png)
![Print](resources/images/files-window-02.png)
![Rebrickable API key](resources/images/files-window-03.png)
![Set inventory import](resources/images/files-window-04.png)
![BrickLink export](resources/images/files-window-05.png)
![HTML export](resources/images/files-window-06.png)
![HTML style sheet](resources/images/files-window-07.png)

## Image

<!-- image | camera | SAVE MODEL IMAGE // STILLS AND ANIMATED GIFS -->

File > Image saves still images and animated GIFs of the active model. The preview on the left shows exactly what will be saved, and the settings on the right are grouped by what they change. Every setting is kept for the next time.

<!-- gallery three -->

![Turntable](resources/images/image-example-01.gif)
![Steps](resources/images/image-example-02.gif)
![Next Step, six steps, one turn each](resources/images/image-example-03.gif)

- **Background.** View Color saves the view background opaque. Transparent leaves it empty so the model can be placed over other images. Only PNG and GIF files keep transparency.
- **Steps.** Current Step, All Steps or a Range. Saving several still steps writes one numbered file per step: model.png becomes model01.png, model02.png and so on. The arrows under the preview look through the steps first.
- **Framing.** Current View saves what the model view shows. Fit Model moves the camera back until every part fits, at every turntable angle and every pose, so a Joints animation holds still. Fit Each Step fits every step on its own parts. Margin keeps 0 to 50 percent of empty space around the model.
- **Center and Bounds.** Turntables turn around the World Origin, the Model Origin or the Model Center. Bounds measures the fit on the Whole Model, so every step lines up, or on the Shown Steps only.
- **Size.** Ratio presets from 1:1 Square through 5:4, 4:3, 3:2 and 16:10 to 16:9 and 21:9, Free, or the shape of the View. Width and Height go up to 32768 pixels, in Landscape or Portrait.
- **File.** The path runs along the bottom of the dialog, type it or choose it with the ... button. The extension sets the format: .png, .jpg, .bmp, or .gif for animations. Turning Animated GIF on or off switches the extension for you.

### Animated GIF

Check Animated GIF to save one looping GIF. The preview plays it live with a play and pause button, and Motion chooses what moves.

| Setting | Turntable | Steps | Joints |
| --- | --- | --- | --- |
| Axis, Full Turn, Angle | used | off | off |
| Next Step | Off, Turns or Seconds | off | off |
| Turns | revolutions or swings per loop | off | off |
| Frames | per turn, 2 to 360 | one per step, read only | between two poses, 2 to 360 |
| Frame Rate | 0.5 to 50 fps | 0.5 to 50 fps | 0.5 to 50 fps |
| Hold Last | with Next Step on | used | used |
| Duration | read only | read only | read only |

- **Turntable.** The model turns around the X, Y or Z axis through the Framing center. Full Turn makes seamless revolutions, Angle swings between two angles, measured from the current view, and back. Turns sets 1 to 10 revolutions per loop, Frames the images in one turn, Frame Rate how many are shown each second. Fit Model keeps one steady zoom through the whole turn.
- **Steps.** Each chosen step becomes one frame, without turning, for short animated instructions. At 2 fps every step shows for half a second.
- **Next Step.** The model keeps turning while the build moves on every 1 to 10 Turns or every 0.1 to 60 Seconds, through the chosen steps, and the loop ends on the last one. Turns then shows the total, steps × turns per step. Frames stay per turn, so the turning speed is the same however many steps there are.
- **Joints.** Blends through the poses of the main model and back to the first, see Joints mode.
- **Hold Last.** Keeps the finished model on screen for 0 to 10 seconds before a loop through the steps or poses restarts.
- **Duration.** Shows how long one loop plays: turns × frames ÷ frame rate for a turntable, steps ÷ frame rate + hold for steps, the frames of every step ÷ frame rate + hold with Next Step, and poses × frames ÷ frame rate + hold for joints. Six steps, one turn each, at 24 frames per turn and 12 fps with a one second hold make 144 frames playing for 13 seconds.

### Settings and command line

The dialog keeps its settings in the [Settings] group of ~/.config/StudWorx/StudWorx.conf, saved when you click Save. Still images can also be saved without opening a window with --image and the options in man studworx, including --transparent; animations are saved from the dialog.

| Key | Setting |
| --- | --- |
| ImageWidth, ImageHeight, ImageRatio | Size |
| ImageExtension | File format |
| ImageSteps, ImageTransparent | Steps and Background |
| ImageMotion, ImageAxis, ImageFullTurn, ImageAngleStart, ImageAngleEnd | Motion and turn |
| ImageTurns, ImageFrames, ImageFrameRate | Turns, Frames, Frame Rate |
| ImageNextStep, ImageNextStepTurns, ImageNextStepSeconds | Next Step |
| ImageHoldLast | Hold Last |
| ImageFit, ImageCenter, ImageMargin, ImageFitScope | Framing |

<!-- gallery scroll -->

![Still image: background, steps, framing, size and file](resources/images/image-window-01.png)
![Animated GIF turntable](resources/images/image-window-02.png)
![Steps: one step per frame](resources/images/image-window-03.png)
![Next Step: steps while turning](resources/images/image-window-04.png)

## Rendering

<!-- rendering | details | RENDERING // LIGHTING, SHADING AND RENDERERS -->

The model view draws fast with OpenGL in four lighting styles, and File > Render hands the model to POV-Ray or Blender for photo quality images.

<!-- gallery four -->

![A POV-Ray render](resources/images/rendering-example-01.png)
![Standard](resources/images/rendering-example-02.png)
![Studio](resources/images/rendering-example-03.png)
![Matte](resources/images/rendering-example-04.png)
![Toon](resources/images/rendering-example-05.png)
![Outline selection](resources/images/rendering-example-06.png)
![Tint selection](resources/images/rendering-example-07.png)

- **POV-Ray.** Found on its own in the PATH, Flatpak or Snap and checked for version 3.7. LGEO high detail parts are found beside the parts library or downloaded on request, with a Parts choice between LDraw and LGEO.
- **Render window.** Choose the output file, follow the progress, and watch the image appear in the preview as it renders.
- **Blender.** Render with the bundled Blender LDraw add-ons, with the output, image, scene, parts and editing settings in panels of the render window, or Open in Blender to keep working on the scene there. The Blender executable is set in Preferences.
- **Lighting styles.** Standard, Studio, Matte and Toon, from the View menu or the command line with --lighting. Standard uses one light with soft highlights, Studio key, fill and rim lights, Matte no highlights, and Toon banded cel shading.
- **Shading.** Wireframe, flat shading, default lights, edge lines, conditional lines and mesh level of detail.
- **Mesh level of detail.** Distant parts draw with simpler meshes, from a distance set in Preferences, so large models stay smooth.
- **Studs.** Eight stud styles: Plain, Thin-Line Logo, Outline Logo, Sharp Top Logo, Rounded Top Logo, Flattened Logo, High Contrast and High Contrast Logo.
- **Color configurations.** Colors come from the parts library, the LDraw color configuration or the alternate configuration with more realistic edge line colors for rendering, chosen under Rendering > Colors.
- **Edge colors.** Automate Edge Color computes edge colors from each part color, with edge contrast, saturation and light to dark settings, or set the stud cylinder, edge, black edge and dark edge colors yourself.
- **Separate settings.** The model view, the preview, the parts list and the Minifigure Builder each keep their own rendering settings. Each can follow the model view instead, or set its own wireframe, flat shading, default lights, edge lines, conditional lines and line width.
- **Selection highlight.** Outline or tint and edges, with an adjustable outline width and transparency.
- **Anti-aliasing.** 2x, 4x or 8x samples, and an adjustable line width.

<!-- gallery scroll -->

![Render menu](resources/images/rendering-menu-01.png)
![Render window](resources/images/rendering-window-01.png)
![Blender render window](resources/images/rendering-window-02.png)
![Edge colors](resources/images/rendering-window-03.png)
![Stud styles](resources/images/rendering-window-04.png)

## Modeling

<!-- modeling | color | MODELING // PARTS, SELECTION AND TRANSFORMS -->

Pick a part in the Parts panel, drop it in the model view, then move, rotate, paint and group it. Snapping keeps everything on the LDraw grid, and selection tools find the parts you need in large models.

<!-- gallery three -->

![Flex Path control points](resources/images/modeling-example-01.png)
![Train track](resources/images/modeling-example-02.png)
![Explode](resources/images/modeling-example-03.png)

<!-- gallery three -->

[A Flex Path hose follows its end point as it moves, then seats back on its brick](resources/videos/studworx-flex-01.mp4)
[A hose end seated on a newly placed brick, the path bending to reach it](resources/videos/studworx-flex-02.mp4)
[A hose seated on the side of a brick, following it as the bricks move](resources/videos/studworx-flex-03.mp4)

- **Parts.** Insert, delete, duplicate, paint, hide and group parts, with the first part placed at the origin as an option.
- **Insert colors.** Insert with the current color, the selected part's color or the last inserted color, or lock the Parts panel to one color.
- **Snapping.** Move in studs, flats and bricks, rotate in degrees with a custom angle, with relative or absolute transforms and separate or shared rotation.
- **Move snap.** Horizontal steps of 1/20 stud, 1/4 stud, 1 flat, 1/2 stud and 1, 2, 3, 4 or 8 studs, vertical steps of 1/20 stud, 1/4 stud, 1 flat, 1/2 stud, 1 stud and 1, 2, 4 or 8 bricks, or None.
- **Rotation snap.** 1, 5, 15, 22.5, 30, 45, 60, 90 or 180 degrees, a custom angle, or None. Camera Snap orbits the view in the same steps. Turns in any step land exactly on square angles, so later moves keep whole coordinates.
- **Transforms.** Relative Transforms move and rotate the selection around the part that has focus, Absolute Transforms in model coordinates. Rotate Separately turns each part around its own pivot, Rotate Together turns the selection as one. Each pair has a toggle shortcut.
- **Keyboard moves.** Move and rotate the selection along +X, -X, +Y, -Y, +Z and -Z by the snap steps. Fixed Direction Keys keeps the arrow keys on the model axes instead of following the view.
- **Selection.** Select All, Select None, Invert Selection, Adjacent Parts, by Name, and by part, color or both. Select Connected Parts follows the connection shapes of the LDCad shadow library.
- **Selection modes.** Single Selection picks one part, Part Selection every part of the same type, Color Selection every part of the same color, and Part and Color Selection both.
- **Select by Name.** A checklist of every object in the model, with All, None and Invert.
- **Part connections.** Use Part Connections turns connected part selection on and off, with the shadow library folder chosen in Preferences.
- **Find and Replace.** A bar to find parts by part and color, step through the matches or Find All, and replace one or all, submodels included on request.
- **Array.** Repeat the selection along one, two or three axes, with a count, an X, Y and Z offset and an X, Y and Z rotation for each.
- **Explode.** The Explode panel, shown from the panel context menu or View > Toolbars, spreads every other visible part, or only the selected ones with each group moving as one part, away from an anchor, the focused part or the first or last selected one, by an X, Y and Z percentage of its distance, or one value for all three with Uniform following the chosen axis. Each part slides straight out along the stud, pin or socket that holds it, a part without one along the nearest X, Y or Z axis from the part beside it, and with Show Lines a line in the Explode Lines color from Preferences runs through the connection, from the socket on the part it came off to the moved part. Submodels in the spread, and the submodels inside them, flatten into their parts first, so everything spreads, and the flatten and spread are one undo step. Changing a setting spreads the parts again, Collapse or Undo puts the parts and the submodels back, and any other edit only puts the parts back. Save Copy saves the whole exploded project to a separate file named with -exploded before the extension, and the open file is not saved.
- **Flex Path.** Hoses, cables and other flexible parts bend along control points that stop at the part length.
- **Control points.** Insert and remove control points, clamp a point along the path or release it to rotate freely, align it to the path once, or turn it along the X, Y or Z axis. New points can start clamped, and the spread handle shapes the curve.
- **Groups.** Group the selection under a unique name, Ungroup, add the focused part to a group or remove it, and Edit Groups in a tree of parts and groups.
- **Hide and show.** Hide Selected or Hide Unselected, then Unhide Selected or Unhide All.
- **Show Earlier and Later.** Move the selected parts one step earlier or later.
- **Train track.** Rotate Train Track turns a track part to its next connection, and Focus Next and Previous Connection choose where the next part joins.
- **Pivot points and keyframes.** Reset Pivot Point puts the pivot back at each part's origin, and Remove Keyframes clears the animation of the selected parts.
- **Lights and cameras.** Lights and cameras are placed in the model and saved with it, see Tools.

<!-- gallery scroll -->

![Move snap](resources/images/modeling-menu-01.png)
![Rotation snap](resources/images/modeling-menu-02.png)
![Control point menu](resources/images/modeling-menu-03.png)
![Part connections](resources/images/interface-menu-12.png)
![Group](resources/images/modeling-window-01.png)
![Select by Name](resources/images/modeling-window-02.png)
![Edit Groups](resources/images/modeling-window-03.png)
![The Find and Replace bar](resources/images/modeling-window-04.png)
![The Array dialog](resources/images/modeling-window-05.png)

## Tools

<!-- tools | teal | TOOLS // MOUSE, LIGHTS AND CAMERAS -->

The Tools toolbar sets what the mouse does in the model view. Every tool has a keyboard shortcut, and mouse buttons with modifier keys can start any tool for one drag.

<!-- gallery two -->

![Lights and cameras](resources/images/tools-example-01.png)
![Joints animation of a minifigure](resources/images/tools-example-02.gif)

- **Edit tools.** Insert adds the part chosen in the Parts panel, Select picks objects, with Ctrl or a drag for several, Move and Rotate change the selection, Erase removes what you click, Paint colors a part, and Color Picker takes a part's color.
- **View tools.** Zoom, Pan, Rotate View and Roll change the camera, and Zoom Region zooms into a rectangle you drag.
- **Keyboard and gamepad view.** Zoom In and Zoom Out, and Move Forward, Backward, Left, Right, Up and Down. In builds with gamepad support, the left stick of a gamepad moves the camera.
- **Lights.** Point Light, Spotlight, Directional Light and Area Light tools place lights in the model, with named properties and units, saved in the model file.
- **Cameras.** The Camera tool places a camera. View > Cameras switches between the first sixteen cameras, or None for the free view.
- **Cancel Action.** Esc cancels the current mouse action, a face or cutaway pick or a cutaway resize, closes the Find and Replace bar, clears the selection, and with nothing selected removes the cutaway.
- **Mouse.** Mouse sensitivity and a shortcut for every tool and view action, recorded in Preferences.

### Joints mode

Give a placed submodel a joint, turn it with the mouse and save the angles as poses of each model, to recall them or play them as an animation.

- **Joints from the shadow library.** SNAP_CYL round shapes become hinges, and SNAP_CLP clips, SNAP_FGR fingers and SNAP_GEN spheres (`bounding=sph`) are read as well. Only SNAP_CYL shapes take part in part connections. Shapes in sub-files of a part, and their placement and mirroring, are followed the same way.
- **Set Joint.** Part > Joint > Set with one submodel selected, then click a connection of the submodel with the Joints tool, which shows the Set Joint pointer and outlines the part under it in red. The click sets the joint and goes back to the Select tool. A submodel can hold several joints: Set again at another connection adds one, and Set at a joint already there resets it. During the pick only the selected submodel takes clicks, so you can pick through other submodels in front of it. A round shape makes a hinge on its axis, a sphere makes a ball, and a part without shadow data gets a hinge on its origin. The Joints panel shows the joint, with an X, Y or Z axis choice, the angle and Clear Joint. Each joint is saved before the placed submodel, one line per joint, as `0 !SW JOINT PART <HINGE|BALL> <pivot x y z> <axis x y z> <rotation x y z>` in LDraw coordinates, and setting and clearing it is undoable.
- **Joints tool.** Next to Rotate in the Tools toolbar, and the Joint menu is also in the right-click menu of the model view. With one submodel selected, the parts holding its joints are outlined in red, and a right click goes back to the Select tool. Click down into submodels to take the innermost one with a joint and drag; on a submodel with several joints, the joint nearest the click turns. A hinge turns about its axis by the horizontal drag, a ball turns about the screen up and right directions. Rotation Snap applies and one drag is one undo step.
- **Joint Move and Rotate.** Part > Joint > Move or Rotate, also in the right-click menu of the model view, switches to the Move or Rotate tool with the gizmo on the pivot of the selected submodel's picked joint, lined up with a hinge axis in relative mode. The move gizmo gets a red hexagon like a cube outline and the rotate gizmo a red outer ring. The rotate keys and the Rotate tool then turn the joint around its pivot and axis instead of the pivot point, saving the turn as the joint angle; a hinge only turns about its axis. Selecting something else goes back to the pivot point. Use Joint Rotation under General > Editing in the preferences does this whenever a selected submodel has a joint.
- **Joints panel.** View > Toolbars > Joints shows a tree of every model with joints, under it each jointed submodel by name and position (`arm-right.ldr 006`), and under that each of its joints (`Joint 001`) with type and angle. Clicking a joint selects its submodel, in its own model, and makes it the joint the panel and the Move and Rotate gizmos work on. Below the tree are the poses of the picked model, which hold that model's joints only. Save stores them under a name, saving a name again replaces it, Recall puts the angles back in one undo step, and Rename and Delete edit the list. Poses are saved in their own model's file as `0 !SW JOINT MODEL "<name>" "<model file>" <part index> <rx ry rz> [joint number]`, one line per joint, the joint number counting from 0 and left out for the first joint. Older project-wide sets and `0 !SW POSE` lines still load, split into the models they belong to.
- **Joints motion.** File > Image > Animated GIF with Motion Joints blends through the poses of the main model in order and back to the first, with Frames between two poses, Frame Rate and Hold Last. Hinges blend their angle, balls blend along the shortest turn.

<!-- gallery scroll -->

![Tools menu](resources/images/tools-menu-01.png)
![Part > Joint](resources/images/interface-menu-04.png)
![Tools toolbar](resources/images/tools-window-01.png)
![Light properties](resources/images/tools-window-02.png)
![Save Image with Motion Joints](resources/images/tools-window-03.png)

## Views

<!-- views | plane | VIEWS // CUBE, CAMERAS, CUTAWAYS AND GRIDS -->

Look at a model from any side, see inside it without changing it, and measure it against the grid.

<!-- gallery four -->

![View cube small labels](resources/images/views-example-01.png)
![View cube large abbreviations](resources/images/views-example-02.png)
![Axis icon](resources/images/views-example-03.png)
![Camera bar](resources/images/views-example-04.png)
![Perspective](resources/images/views-example-05.png)
![Orthographic](resources/images/views-example-06.png)
![Cube cutaway](resources/images/views-example-07.png)
![Sphere cutaway](resources/images/views-example-08.png)
![Cylinder cutaway](resources/images/views-example-09.png)
![XZ and YZ height grids](resources/images/views-example-10.png)
![Idle turntable](resources/images/views-example-11.gif)

- **View cube.** Chamfered edges and corners: click any of its 6 faces, 12 edges or 8 corners for that view.
- **View cube settings.** Shown in any corner of the view, in Extra Small to Extra Large, with its own face, highlight and text colors, and small full names or large abbreviations (TP, BT, LT, RT, FT, BK) on its faces.
- **Axis icon.** X, Y and Z labels with optional negative axes. The view cube, axis icon and part thumbnails come in Extra Small to Extra Large.
- **Axis icon settings.** Shown in any corner, with or without labels, with negative axes and a cube outline of adjustable transparency, and a color for each axis and its negative.
- **Preview overlays.** The preview shows its own view cube and axis icon, each turned on and off from its options.
- **Viewpoints.** Front, Back, Top, Bottom, Left, Right and Home, Zoom Extents, Look At and Normal to Face.
- **Projection.** Perspective or orthographic, for the active view or all views at once.
- **Cameras.** A floating camera bar, camera orbit snapping and cameras numbered Camera 001.
- **Cutaways.** Cube, sphere and cylinder cutaways with solid cut surfaces, a shaded cut and resize knobs, pushed in or pulled out by one plate. The cutaway is saved with the model and comes back when the file opens, and the parts themselves are never changed.
- **Cutaway controls.** Grow and Shrink resize the cutaway, Push In and Pull Out move it one plate, Shade Color sets the light shade on the cut faces, and Remove shows the whole model again.
- **Cut edges.** With edge lines on, the cut draws edge lines in each part's edge color where it meets the outer faces, and thinner lines along the corners of the cut faces. Cut Edges in Preferences > Views > Cutaway turns them off.
- **Grids.** A base grid with studs, lines and origin lines, vertical XZ and YZ height grids sized around the model, each grid with its own toggle and All Grids to turn them on or off together, and a toggle for each axis line.
- **Grid settings.** Line spacing, stud and line colors, and Height Grids Behind Model to keep the height grids behind the parts.
- **Idle turntable.** The model turns full screen around the X, Y or Z axis when you leave StudWorx idle. Any input ends it. The idle time and the time for one turn are set in Preferences.

<!-- gallery scroll -->

![Grids with the base grid off](resources/images/views-menu-01.png)
![Viewpoints](resources/images/interface-menu-08.png)
![Grids](resources/images/interface-menu-12.png)

## Steps

<!-- steps | yellow | STEPS // BUILD ORDER AND TIMELINE -->

Every part belongs to a building step. Steps drive the model view, the instructions and the animated GIFs.

<!-- gallery one -->

![Fade and highlight](resources/images/steps-example-01.png)

- **Steps menu.** Go to the first, previous, next or last step, and insert or remove steps.
- **Timeline.** Shows the step each part is added in and removed in. An empty removal step reads Never. Drag parts between steps or reorder them.
- **Move and paste.** Move the selection to the current, previous or next step, and paste with its steps.
- **Step views.** MLCad ROTSTEP step views are shown, edited and followed in the Timeline.
- **Step view commands.** Step View sets the view rotation of the current step, Remove Step View clears it, and Follow Step Views turns the view to each step's rotation as you change steps.
- **Emphasis.** Highlight New Parts and Fade Previous Steps in the model view and the instructions, with a color for each, and an empty step fades nothing. Fade on Edit tints the rest of the model with the edit fade color while you edit a submodel in place. When it is off, the rest of the model keeps its own colors, slightly translucent. Fade on Joint Set tints every other submodel while you pick a joint, and Fade on Joint Move and Rotate keeps it on while you move or rotate in that mode. All five are under Steps and Highlighting in the view gear menu.
- **Animation keys.** Add Keys records new animation keys for parts, lights and cameras as you change them.

<!-- gallery scroll -->

![Steps and Highlighting](resources/images/steps-menu-01.png)
![Steps menu](resources/images/interface-menu-05.png)
![Timeline panel](resources/images/steps-window-01.png)
![Step view](resources/images/steps-window-02.png)

## Panels

<!-- panels | wireframe | PANELS // PARTS, COLORS, PROPERTIES AND MORE -->

Panels dock side by side in any area of the main window, with uniform headers, buttons and text boxes sized from their options button.

- **Parts.** Categories and palettes with a filter. The panel snaps to whole thumbnail tiles and rows, docked or floating, and the tooltip of each part shows its name, ID and weight.
- **Part palettes.** Create, rename, reorder and delete palettes, import them, or build one from an online set inventory.
- **Colors.** Collapsible solid, translucent and special color groups with square color tiles.
- **Properties.** Compact position, rotation and transform rows and a spanning color swatch. Lights and cameras get named rows with units.
- **Minifigure Builder.** Build a minifigure from templates and add several variations to the model.
- **Preview.** Parts and models, in a Square Viewport when you choose, with Keep Last Selection.
- **Joints.** The picked joint of the selected submodel with its axis and Clear Joint, the tree of models, submodels and their joints, and the poses of the picked model with Save, Recall, Rename and Delete.
- **Terminal.** An embedded terminal sized to whole rows and columns, with Open Model Folder.
- **Terminal folders.** Follow Model Folder moves the terminal to the folder of each model you open, and Go to Model Folder moves it there once.

<!-- gallery scroll -->

![Terminal folders](resources/images/panels-menu-01.png)
![A selected part with its Properties](resources/images/panels-window-01.png)
![Preview](resources/images/panels-window-02.png)
![Parts](resources/images/panels-window-03.png)
![Part tooltip](resources/images/panels-window-04.png)
![Minifigure Builder](resources/images/panels-window-05.png)
![Colors](resources/images/panels-window-06.png)
![Properties](resources/images/panels-window-07.png)
![Joints](resources/images/panels-window-08.png)
![Terminal](resources/images/panels-window-09.png)

## Instructions

<!-- instructions | blue | INSTRUCTIONS // PAGES READY TO PRINT -->

File > Instructions lays the steps out on pages with a cover, parts lists and a bill of materials. Every page element can be styled and placed, and everything is saved in the model file as !SW INSTRUCTION meta commands.

<!-- gallery three -->

![Cover page](resources/images/instructions-example-01.png)
![Step page](resources/images/instructions-example-02.png)
![Bill of materials](resources/images/instructions-example-03.png)

- **Page setup.** Paper and screen presets, metric or imperial units, margins and orientation.
- **Page bar.** First, Previous, Next and Last buttons under the page, Cover and Bill of Materials jumps that grey out while those pages are off, Step Page and Blank Page to insert an empty step or a blank page before the current page, Remove Page for blank pages and pages of empty steps, and a page box to type the page to go to. Every button has its own mnemonic, and the timeline First, Previous, Next and Last shortcuts turn the pages.
- **Pages.** A cover page with the model title and author in their own fonts and colors, a bill of materials under a Parts List title in its own font and color, blank page breaks and the steps per page, set for each page, with page backgrounds of their own for submodel steps and an Apply to All Pages for the page background and thumbnails.
- **Style.** Step dividers, the document font, the model projection, parts list counts, sort order and border, and full page backgrounds.
- **Step images.** Parts new to each step are outlined and the earlier ones fade.
- **Placement.** Place, drag and lock every page element within the page margins, on a snapping page grid. Snap to Anchors snaps the corners, edge centers and center of a dragged element to those of the page area and the other elements, and turning it on turns Snap to Grid off, and the other way around.
- **Elements.** Page numbers, colors, fonts, padding, margin, border, corner radius and background for every page element, plus notices and pointers to emphasize parts and steps.
- **Rulers.** Horizontal and vertical rulers measure the page in the page units, or the document units when they are in use, with the page and its margins shaded and an arrow on each ruler that follows the pointer, filled with its own color and alpha. Show Rulers turns them on and off.
- **Element inspector.** Like the box model of a web browser inspector, Show Element Margins and Show Element Padding fill the margin around and the padding inside every page element, and Show Element Anchors marks its corners, edge centers and center. Each overlay has its own color and alpha in the Colors preferences, and none of them is printed.
- **Editor, Export HTML and Print.** An Editor panel to read and edit the model file text, whose height snaps to whole lines so no line is ever cut off, Export HTML for the instruction pages, and a built-in Print panel.

<!-- gallery scroll -->

![Properties options](resources/images/instructions-menu-01.png)
![The Instructions window](resources/images/instructions-window-01.png)
![Element margins, padding and anchors](resources/images/instructions-window-02.png)
![Properties](resources/images/instructions-window-03.png)
![Grid](resources/images/instructions-window-04.png)
![Placement](resources/images/instructions-window-05.png)
![Print](resources/images/instructions-window-06.png)
![Editor](resources/images/instructions-window-07.png)

## Models

<!-- models | green | MODELS // SUBMODELS AND PROPERTIES -->

Large builds split into submodels that you can edit on their own or in place, inside the model that uses them.

<!-- gallery one -->

![Editing a submodel in place](resources/images/models-example-01.png)

- **Properties.** Part IDs, thumbnails and counts, with BrickLink or CSV export, and the size and weight of the model in metric and imperial units. A Weight column gives each part's weight in grams or ounces, chosen with the units box below the table, from the BrickLink part weights, and parts without a known weight show N/A.
- **Submodels.** New Submodel, the Submodels list, Open Selected Submodel, Inline Selected Submodels, Flatten Submodels, which replaces every submodel at all depths with its parts in one undo step, also run from the command line with --flatten, and Move to New Submodel.
- **Submodels list.** Create, rename, duplicate and delete submodels, set the active one, and export them as separate LDraw files.
- **Models menu.** Switch between up to forty models of the file from the Models menu.
- **In-place editing.** Edit a submodel inside its parent. A path bar shows where you are, and its buttons go back to each model.
- **Editing commands.** Edit Selected Submodel enters the selected submodel in place, and End Submodel Editing goes back to its parent.
- **Names.** Consistent default names and numbered duplicates: Submodel 001.ldr, and Car.ldr becomes Car 001.ldr.

<!-- gallery scroll -->

![Model properties](resources/images/models-window-01.png)
![Parts](resources/images/models-window-02.png)
![Submodels](resources/images/models-window-03.png)

## Preferences

<!-- preferences | grid | PREFERENCES // EVERY SETTING IN ONE PLACE -->

Nine pages with lined up label and control columns, checkboxes included, each label greyed out with its control. Value boxes show their maximum and bring any typed number into range, and every slider shows its value.

- **General.** Reference paths, where the color configurations, minifigure builder settings and BrickLink mapping update from ldraw.org and Rebrickable with one button and bundled copies are used until a file is set, online services, where the Rebrickable API key is kept and Get Key opens its page, a rejected key being cleared so it is asked for again, the author of new models built from a name, email address and comment, such as All rights reserved, joined by the author line splitter and previewed as the author line, and the parts library, Blender, POV-Ray and LGEO locations.
- **Startup and files.** Open the most recent file, restore the tab layout, load all parts before showing a model, watch for file changes and ask, migrate or keep LeoCAD models when opened.
- **Rendering and Views.** Performance, colors, steps and highlighting, and lines with edge contrast on Rendering, and the Model, View Cube, Axis Icon, Cutaway, Turntable, Base Grid, Parts, Preview and Minifigure Builder groups on Views.
- **Interface and Instructions.** Interface and view colors, the default instruction page settings, and the rulers and element inspector overlays of the instructions page.
- **View colors.** Background, active and inactive view borders, selected, focused and new parts, the marquee, overlays, text and its halo, lights, cameras, control points, Flex Paths, the view cube, every axis, and the element margin, padding and anchor overlays and the ruler pointer arrows of the instructions page, each with an alpha slider.
- **Categories.** Part categories turned on and off with a checkbox, edited, imported and exported.
- **Context Menu.** The right-click menu of the model view is yours to arrange, in its own tab before Keyboard. Add commands and menus from the list on the left with the arrow, a double click or a drag, reorder them with Up, Down or a drag, and add separators and named submenus, rename or remove entries, or reset to the default layout. Control point commands only show on flexible parts, and empty submenus are left out.
- **Keyboard and Mouse.** Shortcuts recorded by clicking, importable and exportable. Commands that share a shortcut show in red, and assigning a taken shortcut asks before moving it.
- **Shortcut filters.** Filter commands and tools by name or shortcut, give each two shortcuts, and reset them to the defaults.
- **From LeoCAD.** Settings, part palettes and minifigure templates are copied from LeoCAD on the first start.

<!-- gallery scroll -->

![General](resources/images/preferences-window-01.png)
![Rendering](resources/images/preferences-window-02.png)
![Views](resources/images/preferences-window-03.png)
![Interface](resources/images/preferences-window-04.png)
![Instructions](resources/images/preferences-window-05.png)
![Categories](resources/images/preferences-window-06.png)
![Context Menu](resources/images/preferences-window-07.png)
![Keyboard](resources/images/preferences-window-08.png)
![Mouse](resources/images/preferences-window-09.png)
![About](resources/images/preferences-window-10.png)
