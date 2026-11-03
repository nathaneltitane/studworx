### NOTICE

  <details open>
  <summary>2026-11-03</summary>

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
  - Save Model Image: live preview of the saved image with step arrows, ratio presets from square to widescreen with Landscape and Portrait, a transparent background for PNG and GIF files, Save and Cancel in the File box and help from the title bar What's This button
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
  - Cutaway: faces lined with a light shade, its color and alpha set in Preferences, the alpha reads 0 to 1 over 95% to 85% transparency so the shade never disappears
  - Cutaway: cut parts show solid surfaces in their own color where the cut passes through them, like a sawn brick instead of a hollow shell
  - Projection: new model views and tabs open with the last chosen projection, set in Preferences to change only the active view or all model views at once
  - Instructions: Model Projection in the Document settings renders the step and cover models in perspective or orthographic, whatever the model view shows
  - Turntable: after an idle time the model turns full screen in perspective around the X, Y or Z axis set in Preferences on the background color, any input ends it
  - Preferences: Views page grouped into Model, Cutaway, Turntable, Base Grid, Parts, Preview and Minifigure Builder without repeating View or Preview, every slider shows its value on its left and reaches the right edge
  - Preferences: units inside their boxes and named by the count (1 stud, 6 studs), turntable idle and turn durations in seconds, grid line spacing in a spin box
  - Preferences: the Files group sets the Author of new models, "Author" until it is set
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
  - Axis icon: its own Preferences group with size, location, X, Y and Z labels, optional negative axes in their own duller colors set under Colors, and a chamfered view cube wireframe around the axis whose alpha reads 0 to 1 over the same range as the cutaway shade, the icon keeps the same clearance from the view edges as the view cube at each size with the labels kept inside it
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
  - Preview: optional square viewport that sizes the panel to fit
  - Models menu: 'Configure Minifigure...', Preferences and Instructions moved to the Edit menu

  </details>

