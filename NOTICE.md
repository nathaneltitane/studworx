### NOTICE

  <details open>
  <summary>2026-11-12</summary>

  - Views: the X, Y and Z axis lines draw on their own, the Grid Origin Lines setting is gone, and the negative Z line shows only once parts go below the base grid
  - Views: the base grid starts at 2 × 2 studs and the height grids one brick high, and they grow past the parts by a Grid Margin in studs and a Height Margin in plates
  - Views: Split Horizontal, Split Vertical and Remove View no longer crash when the model view doesn't have keyboard focus
  - Preferences: Rendering, Views and Interface sort their settings into clearer groups, the colors move to Interface, and color alpha is set in the color picker
  - Explode: the X, Y and Z spreads are multiples from 1.0 to 10.0 instead of percentages, and Explode is greyed out while every spread is 1.0
  - Keyframes: panel buttons are painted in the color theme and run the same commands as the Time toolbar, which gains Set Keys, Remove Keys, Play and Stop
  - Timeline: steps with animation keys list a Keys row per keyed object, and clicking an object in Keyframes or Timeline selects it in the model view
  - Samples: sample-train-tracks.ldr, an S-curve of 9V curved track joined at its train track connections

  </details>

  <details>
  <summary>2026-11-11</summary>

  - Views: the X, Y and Z Axis Line toggles draw their lines on their own, and the Grid Origin Lines setting that silently hid them is gone
  - Views: the negative Z axis line follows the height grids when they stand behind the model and only shows once parts go below the base grid, like the X and Y lines growing with the model
  - Views: the base grid starts at 2 × 2 studs and the height grids one brick high, and the grids and axis lines grow stud by stud and plate by plate past the parts, with a Grid Margin in studs and a Height Margin in plates in Preferences, Views, Base Grid
  - Preferences: the Views page gathers the X, Y and Z axis line and negative axes settings in their own Axis Lines group above Base Grid, which keeps the grid settings
  - Views: Split Horizontal, Split Vertical and Remove View no longer crash when the model view doesn't have keyboard focus
  - Preferences: Rendering groups its settings into Display with Anti-aliasing, Mesh Detail Control and a Mesh Level of Detail slider with its value on the left, Studs with Contrast Settings at full width under Stud Style, Shading with the shading mode, lighting style and edge lines, Colors with the edge color settings, and Steps and Highlighting
  - Model view: Mesh Detail Control joins Edge Lines and Conditional Lines in the options menu of the model view
  - Preferences: Rendering, Steps and Highlighting lists its toggles first, then Fade on Joint Move and Rotate, Highlight Style and Outline Width, and its colors moved to Interface, Steps and Highlighting
  - Preferences: General shows Author right under Online Services
  - Preferences: the rows of Editing, Files, Base Grid, Parts, Preview and Minifigure Builder follow a steadier order, and Base Grid draws its lines with Draw Lines and Draw Lines Every, from 1 to 100 studs
  - Preferences: Views, Cutaway starts with Cut Edges, and the cutaway shade color moved to Interface
  - Preferences: the cutaway shade, axis icon outline and instructions page colors set their alpha in the color picker instead of a separate slider, and saved settings keep their look
  - Preferences: Interface groups the colors from the farthest back to the closest, Interface, Grid, Objects, Steps and Highlighting, Flexible Parts, Axes, View Cube and Instructions, and the background joins the Interface group, whose rows run from the background to the text
  - Preferences: the page margin and grid line colors of the Instructions page moved to Interface, Instructions as Page Margins, Page Grid Primary Lines and Page Grid Secondary Lines, and the help of the General, Rendering, Views and Interface pages names their new groups
  - Explode: the X, Y and Z spreads are multiples from 1.0 to 10.0 instead of percentages, 1.0 leaving the parts in place and 2.0 doubling their distance, and Explode is greyed out while every spread is 1.0
  - Site: the Panels section lists the Explode panel with its own image
  - Interface: group titles such as Base Grid are no longer cut short at the right with some fonts
  - Samples: sample-train-tracks.ldr, an S-curve of 9V curved track joined at its train track connections
  - Site: the Interface, Steps, Rendering, Panels, Views, Tools, Modeling (including Train Track) and Models galleries show the new grid and axis lines in fresh captures, and the Preferences Views page shows the Axis Lines group
  - Colors: the color picker popup opens wide enough for 16 color tiles a row instead of a narrow column
  - Interface: check boxes in lists such as Part Categories and Context Menu stand out from the list background on the dark theme
  - Instructions: the Add Notice and Edit Notice dialogs label their box Notice Text, and the Context Menu submenu dialogs label theirs Name, without a trailing colon
  - Site: every image retaken from the installed build, with new images for the Cameras, Status Bar, Find, Group, Selection Mode, Snap Z, Transform, Visibility, Submodel, Tab, Keyframes, Timeline, Colors, Joints, Properties, Parts and Instructions editor menus, the Light Color picker, Edit Palettes, Edit Category, Add Notice and Page Setup dialogs, and recent files shown from the installed samples
  - Keyframes: the rewind, play, pause, stop and forward buttons are painted in the color theme, so they no longer stay black on a dark interface
  - Interface: floating panels keep the StudWorx title bar with its painted float and close buttons on every system, instead of the small Windows tool window frame
  - Keyframes: Add Keys is a square icon button like its toolbar button, and every panel button runs the same command as the matching toolbar button
  - Time toolbar: Set Keys, Remove Keys, Play and Stop join Add Keys and the step buttons, so the toolbar does everything the Keyframes panel buttons do
  - Timeline: each step with animation keys lists a Keys row per keyed object, so steps made by keys are no longer empty, and a double click selects that object at that step and shows the Keyframes panel
  - Keyframes: clicking an object name selects it in the model view like the Timeline, Ctrl+click adds or removes it, the arrow still folds its properties, and selected objects are highlighted

  </details>

  <details>
  <summary>2026-11-10</summary>

  - Preferences: each tab scrolls when the window is taller than the screen, and the window opens wide enough to show every tab, so OK and Cancel stay in reach on low resolution screens
  - Interface: floating panels keep their own title bar, so a double click or a drag back onto the window docks them again
  - Interface: every floating panel opens at its smallest contents size and can be enlarged but never shrunk below it, forms like Properties, Explode, Minifigure Builder and Colors stay as tall as their contents as groups open and close, and the terminal counts as a resizable view
  - Interface: every docked panel can be squeezed down to its title bar, Preview included, which floats and docks with a square view and resizes like any other panel, and title-only panels keep their height when a panel beside them floats, closes or shrinks to its contents, Properties included as the selection changes, and squeezed panels stay squeezed after a restart or when the window is maximized and restored
  - Colors: squeezing the panel no longer crashes StudWorx, the floating panel follows its contents height as groups open and close, and the last group header and color row are no longer cut off
  - Site: the Views gallery shows the view cubes and axis icon on their own row, then Perspective, Orthographic and a new Wireframe view in matching captures, the cutaways, and the camera bar, height grids and idle turntable
  - Help: the Explode, Joints, Poses, Keyframes and Minifigure Builder panels each have one overview, shown on the panel and its title bar, the View menu entries only name the panel they show, the full diamond of the Keyframes panel is called Set Keys like its command, and the Render window panels name Blender or POV-Ray
  - Translations: the Log button of the Render window is translated, Render reads Renderizar in Portuguese and Rendu in French, Add Keys reads Adicionar chaves in Portuguese and 添加關鍵影格 in Traditional Chinese, and Set Keys reads Schlüssel setzen in German
  - Site: help search groups the panel controls under Explode Panel, Joints and Poses Panels and Keyframes Panel, and the Export BrickLink and Missing and Moved Parts help under their window names, instead of Main Window
  - Packaging: the publisher reads StudWorx.app in the Windows uninstall entry and the software store listing, and macOS keeps its settings under studworx.app
  - Windows: resources/packaging/windows/windows-portable.sh builds a portable self extractor that needs no admin rights, and StudWorx keeps its settings in an INI file in the settings folder beside StudWorx.exe and leaves the registry alone

  </details>

  <details>
  <summary>2026-11-09</summary>

  - Tabs: model tabs line up with the view gear button, each ending in a square close button behind a divider, and the tab scroll arrows are framed like the gear button
  - Poses: the poses move out of the Joints panel into their own Poses panel, View > Toolbars > Poses Panel, listed under a Pose header like the joints tree
  - Interface: Lock Interface, at the end of the toolbar and panel right-click menu and View > Toolbars, hides the panel title bars to save space and keeps panels and toolbars from being dragged, floated or redocked, while panels still resize and model tabs still move
  - Joints: an empty joints tree reads None: Part > Joint > Set Joint
  - Joints: with no joint picked the panel reads None: select a jointed submodel, and the axis list reads X Axis, Y Axis, Z Axis
  - Joints: the red joint outline runs inside the submodel edge like the hover line, so it shows next to the selection outline
  - Interface: the Colors, Minifigure Builder and Explode panels scroll when the window is too short instead of overlapping the panels below, so the window resizes down from its bottom edge and the status bar stays on screen
  - Keyframes: View > Toolbars > Keyframes Panel docks a full-width timeline at the bottom or top, with Selected or All objects expandable to their properties, a diamond per key joined by a line, click a step to go to it, double-click to add or remove a key, drag a key to another step, a right-click menu, full and hollow diamonds to add or remove the selection's keys, Add Keys, and rewind, play or pause, stop and forward buttons
  - Keyframes: Steps > Set Keys (K), Remove Keys (Shift+K), Add Keys (Alt+K), Play or Pause (Ctrl+K) and Stop (Ctrl+Shift+K) have keyboard shortcuts
  - Properties: the key buttons are small round dots again, filled when the property has a key at the current step
  - Help: every command, the Explode, Joints, Poses and Keyframes panels and the remaining buttons explain themselves in What's This? (Shift+F1) and F1

  </details>

  <details>
  <summary>2026-11-08</summary>

  - Steps: an opened model now starts on a new empty step after its last one, ready for new parts
  - Files: a Missing and Moved Parts list opens with a model whose parts could not be loaded or are marked as moved or as an alias, offering the part each one points to or its BrickLink mapping, replacing the checked ones in every submodel in one undo step, with Copy List, Don't Warn Again, a status bar icon and the Warn About Missing and Moved Parts preference
  - Find and Replace: the bar floats over the view, its part list shows the parts in the colors they are used in with a flag on parts used in more than one color, a gear button sets how the part lists are filtered and shown, and Replace in Submodels makes Replace All reach every submodel
  - Views: Submodel Hover Outline draws a pastel cyan outline over the innermost submodel under the pointer, on top of the selection outline, with its color in the preferences
  - Joints: the right-click menu of the Joints tree moves, rotates or deletes a joint, or edits or deletes all joints of a submodel, a joint inside a submodel opens it in place, and the joint bar over the view selects its submodels or ends joint mode
  - Menus: snap and joint entries use full names, such as No XY Snap, XY 1/20 Stud, No Rotation Snap, Set Joint, Delete Joint and Joints Panel
  - Thumbnails: part thumbnail icon badges sit in the bottom right corner

  </details>

  <details>
  <summary>2026-11-06</summary>

  - Joints: Part > Joint > Set Joint, also in the model view right-click menu, gives a submodel hinge or ball joints from the shadow library connections, several per submodel, with the picked part outlined in red, the Joints tool next to Rotate turns them with the mouse, and Part > Joint > Move Joint and Rotate Joint and the Use Joint Rotation preference turn them with the rotate keys and the Rotate tool
  - Joints: the Poses panel is now the Joints panel, listing every model, its submodels as `arm-right.ldr 006` and their joints as `Joint 001` in a tree with a left-aligned angle column, with named poses per model saved, recalled, renamed and deleted in what is now the Poses panel
  - Image: File > Image plays the main model poses as an animated GIF with Motion Joints, framed once around every pose so the model holds still
  - Samples: sample-joints.mpd, a minifigure with Stand, Wave, Walk and Work poses, and sample-pov-ray.ldr, a scene with key, fill and rim lights, a floor and a camera for POV-Ray renders
  - Preferences: the new Context Menu tab lets you arrange the model view right-click menu, adding commands, separators and named submenus, renaming, reordering or resetting them
  - Views: the base and height grids each have their own toggle, and All Grids turns them on or off together
  - Views: Fade on Edit and Fade on Joint Set tint the rest of the model with their own colors, set under Steps and Highlighting in the view gear menu and in the preferences, and an empty step no longer fades the model
  - Status bar: a selected submodel shows ID: N/A and the total weight of its parts when all of them have one, also in its Parts panel tooltip, and two or more selected parts show their total weight as `12.91 g Total`
  - Rendering: a model opened from the command line by a relative path renders with Blender next to the model, where the image was not found before
  - Settings: every setting has a line in StudWorx.conf from the first start

  </details>

  <details>
  <summary>2026-11-05</summary>

  - Open: LEGO® Digital Designer models (.lxf and .lxfml) open from File > Open, recent files, drag and drop, the command line and the file manager as a new untitled model
  - Import: File > Import takes .lxfml as well as .lxf
  - Submodels: the menu now reads Open Selected Submodel, Inline Selected Submodels, Flatten Submodels and Move to New Submodel
  - Menus: Select Invert is now Invert Selection, Remove Step is now Delete Step, and the snap toggles read Move Snap, Rotation Snap and Camera Snap
  - Studs: the stud styles are named Thin-Line Logo and High Contrast Logo in the menus, preferences and command line help
  - Interface: errors read "Could not …", file filters read "LDraw Models" and "All Files", menu, status bar, dialog and message wording is consistent, status bar text names toolbars and panels correctly, and the command line help, man page and SITE.md agree with the menus

  </details>

  <details>
  <summary>2026-11-04</summary>

  - Cutaway: the cutaway of a view is saved with the model and comes back when the file opens, the parts themselves are never changed
  - Samples: cube, sphere and cylinder cutaway samples of the minifigure join the explode, flexible part, lights and cameras and minifigure samples
  - Weights: part weights from BrickLink ship with StudWorx, the Properties dialog shows the model weight in grams and ounces and a Weight column with Metric or Imperial units, and the part tooltip and status bar show the weight of a part, or N/A when it is unknown
  - Properties: the measurements and weight are listed one unit per line
  - Import: LEGO® Digital Designer models use a bundled part, color and placement mapping converted from the ldraw.xml of slswww.free.fr, an ldraw.xml in the parts library is no longer read
  - Preferences: Migrate Files on Load chooses Ask, Migrate or Keep for models with LeoCAD meta commands, and Remember this choice in the question sets it
  - Preferences: every checkbox sits in the control column with its label on the left, and the label greys out with its checkbox
  - Status bar: hints such as the face pick and cutaway messages no longer overlap the selection text and stay shown after the menu closes
  - Views: the height grid entries of the view options are greyed out while the base grid is off
  - Views: with split views the view cube and axis icon show on the top left view only
  </details>

  <details>
  <summary>2026-11-03</summary>

  - Cutaway: edge lines in the part edge color outline where the cutaway cuts through parts, with thinner lines along the corners of the cut, and Cut Edges in Preferences > Views > Cutaway turns them off
  - Explode: the Explode panel, shown from the panel context menu or View > Toolbars, spreads the parts on screen away from an anchor part, straight out of their connections with lines back to where they connect, submodels at all depths flatten first in the same undo step and Collapse puts them back, and Save Copy saves the exploded model to a separate -exploded file
  - Submodels: Flatten Model in the Model menu and the Submodel context menu replaces every submodel at all depths with its parts in one undo step
  - Command line: --flatten <outfile.ldr> replaces every submodel at all depths with its parts and saves the model, before any other export
  - Command line: --undo <outfile.ldr> undoes every command line edit and saves the model, to check that undo restores it
  - Lights: point lights and light targets draw as wireframe spheres like the other lights, and spot, directional and area light outlines use more segments
  - Tools: the Delete tool is now Erase, so it no longer shares a name with Part > Delete
  - Defaults: new view colors and keyboard and mouse shortcuts, with Orbit on the right button and Pan on the middle button
  - Keyboard shortcuts: categories in Preferences show readable names such as Snap (Angle)
  - View cube: a Labels setting in Preferences shows Small Labels or Large Abbreviations (TP, BT, LT, RT, FT, BK) on its faces
  - Instructions: a bar under the page goes to the first, previous, cover, bill of materials, next and last pages, inserts a Step Page or Blank Page, removes blank or empty pages and has a page box to type the page, each button with its own mnemonic, and the timeline shortcuts turn the pages
  - Instructions: Submodel Background in the Page category colors the pages of submodel steps apart from the main model pages
  - Instructions: Apply to All Pages in the Page and Parts List categories gives every page the background or thumbnails of the current page, the Arrowhead pointer tip is now Arrow and Pointer Tip is greyed out for callout wedges
  - Instructions: the Bill of Materials page shows a Parts List title in place of the step number, with its own BOM Title Font and BOM Title Color, and the cover author has its own Cover Author Font and Cover Author Color
  - Export HTML: Transparent Background is gone, a Background Color with zero alpha leaves the step images clear
  - Instructions: every element box has a Corner Radius in the Box category of the Properties panel, 0 keeps its corners square
  - Export HTML: Single Page cards round their corners like the step model box, and Edit Style Sheet opens the CSS of the pages in an editor with the options and find and replace of the Editor panel, comments name the feature of each block, and Restore Defaults brings back the built-in one
  - Blender Render: StudWorx brings its own LDraw import and render add-ons, the output, image, scene, parts and editing settings sit in panels of the render window, and Open in Blender opens the model there to keep working on the scene
  - Render menu: Blender Render and POV-Ray Render are listed alphabetically
  - Preferences: the Blender executable is set on the General page with the other program and library locations
  - Export HTML: pages go into a folder named after the model inside the chosen folder, open at index.html and are styled with light and dark colors that follow the browser
  - Export HTML moved from File > Export to the Instructions window, between Editor and Print
  - Export HTML: Single Page shows the Instructions window pages on one scrolling index.html that stops at each page, with #page-N and #step-N links and First, Previous, Next and Last Page arrows on the right, left or bottom edge set by Arrows
  - Export HTML: Display lays the Single Page pages out for the Instructions Page or a Landscape Display (1920 x 1080), Portrait Display (1080 x 1920) or Tablet (2048 x 1536), filling Width and Height, and pages are saved at twice their shown size for high density screens
  - Export HTML: Background Color fills the step images and their rounded frames, and a transparent color leaves them clear, and Override Instructions Settings applies it, the Parts List choices and the Settings Overrides colors (page, cover, Bill of Materials, step, text, divider, pointer, fade, highlight and element boxes) to the Single Page export without changing the model
  - Export BrickLink: a mapping of over 55,000 parts and 236 colors now ships with StudWorx, so exports work without building one first, and Part Exceptions lets you replace the BrickLink number of any part, also as '--bricklink-exceptions' on the command line
  - Export BrickLink and Export CSV: parts the library moved to a new number, such as 60470 to 60470a, are listed and counted under the new number
  - Preferences: updating the BrickLink mapping starts from the bundled file, prefers Rebrickable numbers over outdated ones and keeps the old number of lettered variants BrickLink never took, the Rebrickable API key sits in a new Online Services group on the General page
  - Preferences and Timeline: commands that share a shortcut show in red, and assigning a taken shortcut asks before moving it, a new Highlight Style shows as soon as you press OK
  - Dialogs: the Help buttons are gone, the question mark in the title bar and F1 explain every control, labels sit in one column on the left and fields reach the right edge
  - Rotation: turning parts or pivots in angle steps lands exactly on square angles again, so later moves keep whole coordinates
  - Thumbnails: model thumbnails show again in file managers, and leftovers from the old StudWorks name are removed on install
  - Properties and Import Set Inventory: light settings scroll without a sideways scroll bar, and long set names show in full

  </details>

  <details>
  <summary>2026-11-02</summary>

  - Preferences: LDraw Color Configuration, LDraw Alternate Color Configuration, Minifigure Builder Settings and BrickLink Mapping each have an Update button that downloads the latest file from library.ldraw.org, ldraw.org or Rebrickable, the minifigure part list is saved as 'minifigure.ini'
  - Preferences: the bundled color configurations and minifigure part list are used until you set your own file, a set file that went missing is asked for again at startup
  - Preferences: a new Online Services group on the General page holds the Rebrickable API key with a Get Key button to its page, a key Rebrickable rejects is cleared so you are asked again
  - Dialogs: the Help buttons are gone, the question mark in the title bar and F1 explain every control
  - Rotation: turning parts or pivots in angle steps lands exactly on square angles again, so later moves keep whole coordinates
  - Preferences: commands that share a keyboard shortcut show in red, and assigning a taken shortcut asks before moving it, your shortcuts are left as they are
  - Preferences: a new Highlight Style shows on the selection as soon as you press OK, and closing the dialog no longer pauses while settings are saved
  - Array: the dialog has X, Y and Z column headers and numbered rows for each dimension
  - Edge Colors and Export HTML: the dialogs are laid out in clear groups with labels to the left and controls to the right
  - Properties: the light settings scroll without a sideways scroll bar and always keep one setting row in view
  - Thumbnails: model thumbnails show again in file managers that use the desktop thumbnailer, without step highlight or fading, and leftovers from the old StudWorks name are removed on install
  - Preferences: Colors From moved into a new Colors group on the Rendering page, the Colors page is now Interface
  - Export BrickLink: choose the condition, the wanted list and new listing notices, the choices are remembered, also as '--bricklink-condition', '--bricklink-wanted-list' and '--bricklink-notify' on the command line
  - Command line: '--kmz-geometry all' or 'visible' chooses the KMZ geometry and replaces '--kmz-all-parts'
  - Command line: '-h' now shows the help like '--help', the picture height is set with '--height'
  - Command line: options follow the usual style, one letter after a single dash and full words after two, so '-obj' is now '--export-wavefront', '--libpath' is '--library', '--fov' is '--field-of-view', '--zplanes' is '--clip-planes', '--orthographic' is '--projection orthographic', '--aa-samples' is '--antialiasing-samples', the edge colors are '--edge-color', '--edge-color-black', '--edge-color-dark', '--edge-color-automatic', '--edge-contrast' and '--edge-light-dark' with '--no-' forms, '--povray-render' is '--render-povray', and the help is grouped by topic
  - Export BrickLink: paste the address of a wanted list page instead of its number, after saving, Copy XML and Open BrickLink puts the list on the clipboard and opens the BrickLink upload page
  - Minifigure Builder: templates export as '.minifigure' files and '.minifig' files still import, settings, toolbar and category names carry over on the first start
  - Site: the author avatar in the footer is now a sharper 1024 pixel image
  - The license is now formatted as Markdown, so it is easier to read on GitHub
  - Preferences and Timeline: commands with a shared shortcut stay red while selected, until the conflict is resolved
  - Render: the preset buttons line up with the fields beside them
  - Save Model Image: the file path row now runs along the bottom of the dialog
  - Dialogs: labels sit in one column on the left without colons, fields reach the right edge instead of narrow inputs
  - Blender Settings: every group shares one label column
  - Array: the X, Y and Z columns are equally wide
  - Page Setup: one field per row with a shared label column
  - Prompts: name and value prompts widen to fit their text
  - Find and Replace: the bar labels read Find and Replace
  - Import Set Inventory: the Name column takes the free width, so long set names show in full
  - Step View: the dialog fits its fields, with labels on the left and fields to the right edge
  - Site: the Rendering, Views, Keyboard and Mouse preference screenshots show the shared slider value column and a selected shortcut conflict in red, and the Steps section shows the Step View dialog
  - Site: the Import Set Inventory screenshot shows search results
  - Export BrickLink: a current mapping of over 55,000 parts and 236 colors ships with StudWorx and is used until you set your own file, so exports work without building one first
  - Preferences: updating the BrickLink mapping now starts a new file from the bundled one and takes the Rebrickable numbers over outdated BrickLink numbers in the part files
  - Export BrickLink and Export CSV: parts the library has moved to a new number, such as 60470 to 60470a, are listed under the new number and counted together with it
  - Export BrickLink: Part Exceptions lists parts and the BrickLink numbers that replace their mapping, kept in a comma or tab separated file you can edit by hand, also as '--bricklink-exceptions' on the command line
  - Preferences: updating the BrickLink mapping keeps the old BrickLink number of parts moved to a lettered variant BrickLink never took, such as 3660a staying 3660

  </details>

  <details>
  <summary>2026-11-01</summary>

  - Site: the Modeling section has a looping video of a Flex Path hose seated between two bricks and reshaped by its control points, see [Modeling on the site](https://studworx.app/#modeling)
  - Site: the Array dialog screenshot is now shown full width

  </details>

  <details>
  <summary>2026-10-31</summary>

  - Interface: close, undock and filter clear buttons are drawn to match the theme, and the custom theme draws its own tree lines and expand arrows
  - Site: new Meta section covering the LDraw meta commands StudWorx reads and writes, see [Meta on the site](https://studworx.app/#meta)
  - Manual page: covers LeoCAD '.lcd' projects opening read only, step numbers in saved image names, the step and field of view limits, the LGEO library and studworx.app

  </details>

  <details>
  <summary>2026-10-30</summary>

  - Interface: built from standard Qt widgets throughout, panels, dialogs and windows follow the system style and theme, only the model views draw with OpenGL
  - Save Model Image: live preview of the saved image with step arrows, ratio presets from square to widescreen with Landscape and Portrait, a transparent background for PNG and GIF files, Save and Cancel in the File box and help from the title bar "What's This?" button
  - Animated GIF: turntable animations from Save Model Image, one or more full turns or back and forth swings around the X, Y or Z axis, with frames per turn, a frame rate and the duration shown, played live in the preview
  - Animated steps: Steps plays the build one step per frame, Next Step moves on every few turns or seconds while the model turns, and Hold Last keeps the finished model on screen before the loop restarts, see [Image on the site](https://studworx.app/#image)
  - Image framing: Fit Model and Fit Each Step keep every part inside the image with an adjustable margin, turntables keep one steady zoom through the whole turn around the World Origin, Model Origin or Model Center, and View Color backgrounds now save opaque
  - Print: File > Print prints the current model view through the system print dialog, with a Preview button where the dialog allows it
  - File menu: grouped by task, Print after Save As, Image beside Instructions and Render instead of under Export
  - Preferences: Custom Paths is now Reference Paths
  - Preferences: new Export group with Mark COLLADA Transparency, on by default, writes 'opaque="A_ONE"' on transparent colors so viewers that skip the COLLADA default show transparent parts translucent without an l2cu pass
  - Rotate tool: dragging empty space with a mouse shortcut orbits, pans or zooms the view again instead of being taken as a rotation handle (LeoCAD #1073)
  - Translucent colors: Parts list icons, transparent images and instruction steps keep translucent parts their true color instead of darkened by the background, and transparent decals blend over their part color (LeoCAD #938)
  - Status bar: shows the position of a part while it is dragged in or placed with the Insert tool, and returns to the focused object when the part is dragged back out (LeoCAD #939)
  - File > Export > KMZ: zipped COLLADA for web viewers, flattened to the visible surfaces with one mesh per color by default, or every part with the KMZ Geometry preference, also from the command line with -kmz and --kmz-all-parts
  - Command line: '-bl', '--export-bricklink' writes the BrickLink XML inventory like '--export-csv'
  - Images: saved in the format of their extension, '.jpg', '.bmp' and '.gif' are no longer written as PNG, backgrounds are opaque unless '--transparent' is used with PNG or GIF, and '.gif' steps save one still frame per step
  - Command line: renders and exports run without a display on Linux based systems, '--stud-style' help lists the current stud style names

  </details>

  <details>
  <summary>2026-10-29</summary>

  - File managers: LDraw model files show a thumbnail of the model, from a KDE thumbnail plugin or the desktop thumbnailer, rendered with the new '--transparent' command line option
  - Select Connected Parts: selects every part joined to the selection, with the connection shapes of the [LDCad shadow library](https://github.com/RolandMelkert/LDCadShadowLibrary) by Roland Melkert, set and turned on or off under Preferences › Part Connections
  - Parts library: no longer bundled, the first start offers to download the official LDraw 'complete.zip' into the data folder or to choose an existing archive
  - Install: 'tools/studworx-install' builds and installs StudWorx without a package, with thumbnails, file types and associations, '~/.local' needs no superuser rights, run it again after 'git pull' to update
  - Instructions: step images outline the parts new to each step and fade the earlier ones, set by Fade Previous Steps and Highlight New Parts in the Document settings, their colors kept 85% to 95% opaque
  - Instructions: every page element stays within the page margins, page numbers included, and Page Setup margins are limited to the room the page size leaves
  - Value boxes: the maximum shows after the value (6 / 32 studs), any number can be typed and is brought into range once applied
  - Find and Replace: one bar that starts with its close button, a chevron shows or hides the replace row, long part names no longer stretch it
  - Views: the camera name and the editing path float over the view as bars with a close button, each model in the path is a button that goes back to it, and the bars only move when the view cube or axis icon is in the way
  - POV-Ray: found on its own in the PATH, Flatpak or Snap and checked for version 3.7, LGEO parts found beside the parts library or downloaded on request, both asked for like the parts library when missing, a Parts choice between LDraw and LGEO, and '--povray-render' renders a model to PNG from the command line
  - Icons and cursors drawn in code and sharp at any size, the base grid toggle hides only the grids and leaves the axis lines alone, the terminal panel sizes to whole rows and columns

  </details>

  <details>
  <summary>2026-10-28</summary>

  - Cutaway: cut into the model around a clicked face to see inside it, with a cube, sphere or cylinder shaped cut - from View > Cutaway, the model is never changed
  - Cutaway: click for the default size or drag to size it, move and resize it with the gizmo knobs, push in and pull out by one plate, Esc leaves it
  - Cutaway: faces lined with a light shade, its color and alpha set in Preferences, the alpha spread over 95% to 85% transparency so the shade never disappears
  - Cutaway: cut parts show solid surfaces in their own color where the cut passes through them, like a sawn brick instead of a hollow shell
  - Projection: new model views and tabs open with the last chosen projection, set in Preferences to change only the active view or all model views at once
  - Instructions: Model Projection in the Document settings renders the step and cover models in perspective or orthographic, whatever the model view shows
  - Turntable: after an idle time the model turns full screen in perspective around the X, Y or Z axis set in Preferences on the background color, any input ends it
  - Preferences: Views page grouped into Model, Cutaway, Turntable, Base Grid, Parts, Preview and Minifigure Builder without repeating View or Preview, every slider shows its value on its left and reaches the right edge
  - Preferences: units inside their boxes and named by the count (1 stud, 6 studs), turntable idle and turn durations in seconds, grid line spacing in a spin box
  - Preferences: the Author of new models is set in Preferences, "Author" until it is set
  - Preview: the panel, its options and its preferences are named Preview, since it shows models as well as parts
  - Parts panel: a floating panel ends on whole thumbnail or list rows and scrolls by whole rows, the bottom row is never cut
  - Part categories: turn each category on or off with its checkbox, sub parts and aliases are no longer listed
  - Preview: 'Keep Last Selection' keeps showing the last selected part
  - Minifigure Builder: no longer crashes at startup when the parts library cannot be loaded
  - Panels: the right column shrinks again, value fields without limits no longer size for the largest possible number
  - Minifigure Builder: the template buttons sit in pairs with Add to Model below, so the panel can shrink
  - Views: a model opened from the Open dialog, recent files or a drop shows like one opened directly, sized and fitted to its loaded parts before it is first drawn, also when it is reopened, and stays fitted as the window resizes until the camera is moved
  - Instructions: padding, margin, border and background for every page element, the background fills the whole box
  - Instructions: notices, free text added to any page with a right-click
  - Instructions: pointers from an element border to a part, step, notice or other element - eight anchors or the nearest, line or callout wedge, arrowhead or dot tip
  - Instructions: settings, page layouts, notices and pointers saved in the model file as '!SW INSTRUCTION' meta commands
  - View cube: the view sphere is now a cube with chamfered edges and corners - click any of its 6 faces, 12 edges or 8 corners for that view, with sharper labels
  - Sizes: view cube, axis icon and part thumbnails each come in Extra Small to Extra Large on a 16px baseline - view cube and axis icon 48 to 192, thumbnails 32 to 128, with older saved sizes moved to the nearest one
  - Axis icon: its own Preferences group with size, location, X, Y and Z labels, optional negative axes in their own duller colors set in Preferences, and a chamfered view cube wireframe around the axis whose alpha spans the same range as the cutaway shade, the icon keeps the same clearance from the view edges as the view cube at each size with the labels kept inside it
  - Command line: import and export the settings, keyboard shortcuts, mouse shortcuts and part categories without opening a window - '--import-settings', '--export-settings', '--import-keyboard-shortcuts', '--export-keyboard-shortcuts', '--import-mouse-shortcuts', '--export-mouse-shortcuts', '--import-categories', '--export-categories'
  - Command line: '--help', '--version' and the import and export options run without a display
  - Code: views, preferences, the parts library and projects split into smaller files by task - camera, input, categories, shortcuts, cache, loading, file and import - with no change in behavior

  </details>

  <details>
  <summary>2026-10-27</summary>

  - Packages: .deb, .rpm and AppImage for x86_64 and arm64, Windows installer and universal macOS disk image, all with Qt and the parts library included
  - One release on GitHub, rebuilt from every change and tagged with its date version
  - Versions follow the release date everywhere: 'studworx-2026-10-27-x86_64.rpm', the About dialog and the Windows file version
  - Lighting styles for the default shading: Standard, Studio, Matte and Toon - also from the command line with '--lighting'
  - Selection outline width, set in Preferences or with 'Increase Outline Width' and 'Decrease Outline Width'
  - Cursors and toolbar icons drawn in code at any size, following the light or dark theme - move and rotate cursors show their axis
  - Instructions: place, drag and lock every page element, page grid with snapping, page numbers and colors and fonts for every element
  - Instructions: Print panel inside the instructions window replaces the print dialog and print preview, the window opens maximized
  - Preferences: label and control columns lined up on every page, mouse shortcuts recorded by clicking, mouse tools grouped like the keyboard commands
  - Menus: new Steps menu, Instructions and Properties in the File menu, Image under Export, Preferences at the bottom of the Edit menu
  - Parts panel snaps its categories to the longest name when docked on top or bottom
  - Terminal panel: 'Open Model Folder' button
  - Traditional Chinese, Russian and Ukrainian added to the language choices
  - About dialog shows the LEGO® trademark notice

  </details>

  <details>
  <summary>2026-10-26</summary>

  - LeoCAD+ is now StudWorx, its own project built on [LeoCAD](https://github.com/leozide/leocad)
  - Code split into modules: application, core, library, rendering, input_output and interface
  - Settings, part palettes and minifigure templates are copied from LeoCAD on the first start
  - Models keep LeoCAD's file format and open in either program
  - Set inventories use your own free Rebrickable API key, asked for once
  - Light and camera properties follow the part layout: named rows, values with their names and units, one keyframe per value

  </details>

  <details>
  <summary>2026-10-25</summary>

  - Default names use a three digit number and no number sign: 'Group 001', 'Minifigure 001', 'Submodel 001.ldr', 'Camera 001', 'Spot Light 001'
  - Duplicated submodels take the next free number instead of 'Copy of': 'Car.ldr' becomes 'Car 001.ldr'
  - Panels share one look: headers, buttons, text boxes and spacing follow the options (gear) button
  - Properties: part, color, position, rotation and transform rows below the list - color swatch spans the part and color rows
  - Timeline: steps a part is added and removed in, edited below the timeline - an empty removal step reads 'Never'
  - Model tabs fully outlined and lined up with the view options button

  </details>

  <details>
  <summary>2026-10-24</summary>

  - Instructions: paper and screen presets, metric or imperial units and margins, orientation-based step order
  - Instructions: cover page, bill of materials, blank page breaks, step dividers, document font and full page background
  - Instructions: steps per page set for each page, parts list counts, sort order and border
  - Minifigure Builder panel: build a minifigure, dock the builder and add several variations to the model
  - Preview: floats and docks with a square view
  - Models menu: 'Configure Minifigure...', Preferences and Instructions moved to the Edit menu

  </details>

