### NOTICE

  <details open>
  <summary>2026-11-25</summary>

  - Properties: the Summary tab has a Publication group with title, author, product ID, version, publication date, URL, contact, copyright and notice, saved in the model file
  - Preferences: the Author group is gone, the author of a model lives in Model Properties > Summary
  - Instructions: the front and back cover each have their own Title, Author, Product ID, Version, Publication Date, URL, Contact, Copyright, Notice, Comments, Measurements, Weight and Pieces, and Pieces is the part count of the whole model
  - Instructions: texts fill in %title, %author, %id, %version, %date, %url, %contact, %copyright, %notice, %size, %weight and %comments from the model
  - Instructions: placed cover texts stay put when others are shown or hidden, and the placement tool places every element, pointer tips too
  - Instructions: texts are selected with one click anywhere on them, and pages no longer stall while clicking
  - Instructions: templates keep the notices of the covers, bands and watermark
  - Instructions: the file properties of every instructions setting start with their scope, like COVER FRONT TITLE_FONT, the old names are no longer read
  - Instructions: Relative To places the step number against the parts list and the parts list against the number, choosing one frees the other
  - Instructions: the parts list sits centered beside the step number, and the pair keeps the same spacing in every step
  - Instructions: Copy, Paste and Duplicate for notices, images and pointers, with the main window keys and in the right-click menu, each copy one grid step away, a paste on another page lands in place
  - Instructions: Paste works from the page list too, and locked pages grey out Paste and Duplicate
  - Instructions: saved pages keep their layout, placements are always written so new defaults never move them, and older files keep their cover cells
  - Instructions: pages redraw at once after moving or editing texts, step pictures and parts lists are kept in memory until the model changes
  - Instructions: each cover text starts on its own cell, the title above the author, so turning texts on never stacks them

  </details>

  <details>
  <summary>2026-11-24</summary>

  - Command line: --export-pages saves each instruction page as a PNG picture at the page setup size
  - Command line: the pictures are named after the model and page number, and saved beside the model or in a folder you name
  - Properties: the Summary tab has a Publication group with title, author, product ID, version, publication date, URL, contact, copyright and notice, saved in the model file
  - Instructions: the front and back cover each have their own Title, Author, Product ID, Version, Publication Date, URL, Contact, Copyright, Notice, Comments, Measurements, Weight and Pieces, the Publication group picks the cover to edit, and Pieces is the part count of the whole model
  - Preferences: the Author group is gone, the author of a model lives in Model Properties > Summary
  - Instructions: placed cover texts stay put when others are shown or hidden
  - Instructions: the placement tool places every element, pointer tips too
  - Instructions: texts are selected with one click anywhere on them, and pages no longer stall while clicking
  - Instructions: the Author shows only the Summary author, and the file properties of every instructions setting start with their scope, like COVER FRONT TITLE_FONT or STEP NUMBER_FONT, the old names are no longer read
  - Instructions: templates keep the notices of the covers, bands and watermark, and a model applying one shows them
  - Instructions: texts fill in %title, %author, %id, %version, %date, %url, %contact, %copyright, %notice, %size, %weight and %comments from the model, and the Publication URL shows the model URL by default

  </details>

  <details>
  <summary>2026-11-23</summary>

  - Instructions: the final model, step areas, bands and watermark are boxes with margin, padding, border, corner radius and background like every other element
  - Instructions: bands now start at the paper edge, not inside the page margins
  - Instructions: Reserve Space only moves the page content when a band reaches past the margin
  - Instructions: the model keeps its own page size, margins and units, opening it no longer brings back the default paper
  - Instructions: Header and Footer, Left Band and Right Band each have their own properties, clicking a band opens its group
  - Instructions: On This Page Only keeps a band on the current page and clears it from the other pages
  - Instructions: Delete hides a selected title, cover image, part count, step number, parts list or page number and resets its position
  - Instructions: Show Title and Show Model Image turn the cover title and the finished model on and off
  - Instructions: the Publication group shows the title, author, product ID, version, publication date, URL, contact, copyright, notice, measurements, weight and comments on the front or back cover, each with its own font, color, placement and box, empty texts show the model properties
  - Instructions: images grow to 500 percent of the page width and can run past the paper edge
  - Instructions: pointers can be a line, an L, Z or U shape with rounded bends, or a callout wedge, end in an arrow, a dot or a hollow dot, and their bend and reach are dragged on the page
  - Instructions: titles, part counts, step and page numbers and notices line up left, centered or right and top, middle or bottom in a box sized with Edit Size
  - Instructions: Show Final Model Page closes the main model steps with a page showing the finished model, on by default
  - Instructions: the Pages panel names each page after its submodel and marks the assembly pages
  - Instructions: arrow keys move the selection by one grid division, Shift by a whole grid unit, and Reset Position puts a dragged element back on its placement
  - Instructions: a click only selects, an element moves and the pointer changes only once the mouse moves with the button held
  - Instructions: the Properties panel keeps one width, selecting an element no longer shifts or resizes the page
  - Instructions: boxes with a background color no longer show a faint line around the parts list or where they overlap
  - Preferences: units like px, s, studs and plates sit in a label beside their field, like the page units
  - Instructions: Blank Page Background colors every blank page, the inserted ones and those after the cover and before the bill of materials
  - Instructions: the parts list quantities have their own box, margin, padding, border, corners and background, and stay selected when placed
  - Instructions: band settings grey out while the band is hidden
  - Instructions: the parts list thumbnail fill is one shared fill out to the border, following rounded corners
  - Instructions: box backgrounds fill inside the border with the margin left clear outside, the same for every element
  - Instructions: Model Size is a length in page units, greyed while Fit Model to Box is on
  - Instructions: every element takes clicks and selection only on what it draws, and clicking a divider, a step area or the final model frame opens its properties
  - Instructions: dividers, pointer lines and the final model frame snap to whole screen pixels like box borders, so equal widths draw equal
  - Steps: Add Step View saves the view turned with the view cube into the current step as a ROTSTEP line, absolute, relative or default as picked beside it, absolute by default
  - Steps panel: a step view row always shows its angles and mode, and an end line reads View: 0, 0, 0 relative [default]
  - Steps: step views taken from the model view are no longer mirrored, the saved angles turn back to exactly the view shown, relative ones included
  - Preferences: End Step Views, on by default, saves an end line in the next step whenever a step view is saved, so a view turns only its own step, and removing the step view removes that end line too
  - Step view dialog: Absolute, Relative or Default, absolute angles from the world zero (the front view) or relative angles from the home view, filled in from the model view and free to change, Default ends a carried view like MLCAD ROTSTEP END; older added step views still load and stay as they are
  - Instructions: Lock to Home View draws every model from the home view, a step view still wins, and pages redraw as soon as the model view turns
  - Instructions: clicking a blank spot of a page opens the properties of that page, the cover, step, bill of materials or blank page settings
  - Instructions: a scaled up cover image no longer covers the cover texts
  - Command line: --export-pdf prints the instruction pages to a PDF file on the page setup paper
  - Command line: --export-pages saves each instruction page as a PNG picture at the page setup size
  - Instructions: the bill of materials has its own title, parts list, quantity and part count settings and boxes, so changing it no longer changes the step pages; older files start from the step values
  - Instructions: Show Final Model leaves out the separate final model page and adds no page after a full last page, the final model background takes the Page Background color, submodel pages included, until you pick your own
  - Instructions: two buttons beside the page number show two pages at once, side by side or one above the other, pages 1 and 2, 3 and 4 and so on, and clicking the other page edits it
  - Instructions: Assembly Background in the Page section colors the pages where two or more models come together, clear follows the page as before
  - Instructions: a main model step placing only a just finished submodel gets no page, its final model already shows it
  - Instructions: At Final Model in the Divider section leaves out the lines along the final model box
  - Instructions: the page of the finished main model is listed as Completion, and Completion Background in the Page section colors its own page, or only its box on a steps page, the Completion box in the Box section has its own margin, padding, border and background
  - Instructions: the mouse wheel over the page runs through the pages, two at a time when two pages show
  - Instructions: Reserve Parts Area in the Parts List section keeps a strip of each step for the parts list and step number, the model fits in the rest
  - Instructions: Show Boundaries in the gear menu outlines the outer edge, margin and padding, or every edge of each element in thin red lines, and Layout Areas outlines the content and step areas, parts strips and dividers in blue, the edge, border and layout colors follow the axis colors and change in Preferences, on both pages of a spread, on screen only
  - Window titles: a space before the unsaved changes asterisk, like KDE apps
  - Command line: --resave <outfile.ldr> loads the instruction settings and saves the model, to check how older files load

  </details>

  <details>
  <summary>2026-11-22</summary>

  - Instructions: header, footer, left and right bands repeat their text and images on every step page, and %page and %title fill in the page number and the model title
  - Instructions: the Bands properties set each band's size and color, and Reserve Space keeps the page content clear of a band
  - Instructions: right click the page to add watermark text or a watermark image behind the page content, with its own opacity
  - Instructions: Show On in a notice's right click menu moves it between its page, a band and the watermark
  - Instructions: a page can have its own bands or hide the watermark, and Apply to All Pages copies its bands to every page

  </details>

  <details>
  <summary>2026-11-21</summary>

  - Site: the search fields have a ✕ button that clears the text and shows every group again
  - Instructions: a back cover closes the instructions, shown and blank by default, and the cover is now the Front Cover
  - Instructions: right click a cover to add a linked image, like a logo, on the front cover, the back cover or both, with its own placement, size and lock
  - Instructions: Save Template and Apply Template in the Properties options carry the look, the steps per page and the cover images to any model, and Preferences > General > Default Instructions Template applies one under every model
  - Instructions: Use Document Font and Use Document Font Size work on their own, one gives all text the document typeface, the other its size, and older files keep both
  - Page Setup: Portrait or Landscape follows the width and height while they are typed
  - Instructions: the Placement buttons and Page Area now work on notices too, outer cells put a notice on the paper edge
  - Instructions: the font window opens at a quarter of the screen, centered
  - Instructions: the save key saves the model from the Instructions window too, and its title shows * while there are unsaved changes
  - Instructions: Apply to All Pages on a submodel page keeps the submodel background instead of going back to the default
  - Instructions: header, footer and side bands repeat their text and images on every step page, a watermark sits behind the page content, %page and %title fill in the page number and title
  - Instructions: the cover, the bill of materials and the step pages can show how many parts they take, as "119 PIECES" or your own text
  - Instructions: the cover title, author and part count start at the same size as the Parts List title, and the parts list count sits at its top right
  - Instructions: Show Submodel Preview puts the finished submodel and its part count on the first page of each submodel
  - Instructions: the outer Placement buttons put cover texts, part counts, the submodel preview and page numbers against the paper edge
  - Instructions: the page grid and margin colors start dark red, lilac and navy
  - Instructions: the submodel preview count and the parts list quantities are their own elements, placed anywhere around their picture

  </details>

  <details>
  <summary>2026-11-19</summary>

  - Joints: Joint Mode with nothing selected, or from a right click on empty space, opens an overview of every joint, outlined and ringed; click an outlined part to open its joint
  - Preferences: the joint options have their own Joints group, with Joint Mode Start last
  - Preferences: Select Joints by Submodel, off by default, opens the nearest joint when you click anywhere on a jointed submodel in the joint overview
  - Mouse: the right button orbits only once the mouse moves, so a plain right click opens the right-click menu
  - Shading: menus list Default Lights, Flat and Wireframe, then Studio, Standard, Matte and Toon; Studio is the new default lighting style
  - Save Model Image: the preview plays the Joints motion from pose to pose, framed once so the model holds still
  - Train tracks: no crash when a track part is picked before the view draws it
  - Site: every image retaken from the installed build, with the joint overview, the Joints group in Preferences and Studio lighting

  </details>

  <details>
  <summary>2026-11-18</summary>

  - Keyboard preferences: a Built-in Keys group at the end lists the keys that can't be changed, greyed out
  - Shortcuts: Delete removes only what is selected where you work: the joint in the Joints panel, the step in the Steps panel, the picked keys in the Keyframes panel, the pointers, notices or blank page in the instructions, or the selected objects in the view; Del always deletes, listed under Built-in Keys in Keyboard preferences and can't be given to another command
  - Steps panel: the Timeline panel is now the Steps panel, saved shortcuts and panel places carry over
  - Panels: the Poses panel keeps its place on every start, and a panel layout filled with Joints copies resets to the default once
  - Site: Preferences lists every built-in key and what it does
  - Site: Steps, Keyboard preferences, main window, lighting styles, projections, cutaways, wireframe, height grids, split views and Selected Part images retaken
  - Keyframes panel: click a diamond to pick it, Ctrl+click to add or take it out, Shift+click to take it out; picked keys light up and Delete removes them
  - Shortcuts: Ctrl+Shift+W, Exit Mode in the menus and Keyboard preferences, exits one mode per press, innermost first: a pick, joint mode, the cutaway, then in-place submodel editing; it keeps the selection
  - Shortcuts: Esc steps back the same way, clears the selection before leaving a mode, and now also steps out of in-place submodel editing
  - Shortcuts: Ctrl+Shift+J only goes into joint mode; Ctrl+Shift+W or Esc leaves it
  - Joints: Ctrl+Shift+J goes into joint mode on the selected submodel, or the jointed submodel around the selected part, and Ctrl+Shift+W or Esc leaves it; Joint Mode and End Joint Mode are in Part > Joint and the right-click menu
  - Joints: right-clicking in joint mode keeps the joint, so End Joint Mode works from the right-click menu
  - Joints panel: a Piece column shows where each submodel sits in its file, taken out of the submodel's name
  - Joints panel: a Lock column freezes a joint, saved in the file; a locked joint shows in the Deactivated grey and can't be moved, turned, edited or deleted
  - Preferences: Joint Mode Start opens the first joint, the last used joint of that submodel, or an overview with every joint shown as a ring to click
  - Preferences: Select Joints in View shows the other joints as faded rings while a joint is open, and clicking one opens it
  - Preferences: joint options have their own Joints group, with Joint Mode Start last
  - Preferences: Select Joints by Submodel, off by default, opens the nearest joint from a click anywhere on a jointed submodel in the joint overview
  - Joints: Joint Mode with nothing selected, or from a right click on empty space, opens the overview with every joint outlined and ringed; clicking an outlined part opens its joint
  - Mouse: the right button orbits only once the mouse moves, so a plain right click opens the right-click menu
  - Shading: menus list Default Lights, Flat and Wireframe, then Studio, Standard, Matte and Toon; Studio is the new default lighting style
  - Save Model Image: the preview plays the Joints motion from pose to pose, framed once so the model holds still
  - Train tracks: no crash when a track part is picked before the view draws it
  - Site: every image retaken from the installed build, with the joint overview, the Joints group in Preferences and Studio lighting

  </details>

  <details>
  <summary>2026-11-17</summary>

  - Preferences: a Deactivated color with alpha under Interface > Objects, always a grey shade, for locked or unavailable items
  - Joints: in joint mode, the hinge's locked rotate rings show in the Deactivated color, and holding Ctrl lights them and lets them turn the whole submodel around the joint
  - Joints: parts grouped with a jointed submodel turn along with its joint, from the gizmo, the keys and the Joints tool
  - Shortcuts: Shift+I switches to Default Lights shading by default
  - Joints: in joint mode the Move and Rotate tools and their shortcuts act on the joint, like Move Joint and Rotate Joint
  - Views: the rotate gizmo draws its axis circles all the way around, no longer cut in half
  - Joints: editing a submodel in place no longer starts joint mode or joint move and rotate, joint mode only starts from a joint
  - Joints: a jointed submodel inside a group keeps its joint editing, moving and turning
  - Joints: Escape or a click away from the joints leaves joint mode and goes back to selecting parts
  - Instructions: the page view smooths lines, borders and round ends
  - Instructions: rounded divider ends are full half circles at every width, thin dividers no longer look pointy
  - Instructions: Highlight Width sets how thick the new part outline is, from 1 to 72, and wide outlines stay solid
  - Instructions: the highlight color keeps the opacity you pick, no longer held between 85 and 95 percent
  - Instructions: Final Border, Final Border Color and Final Margin in Steps per Page frame the final model area and keep it away from the steps and page margins, none by default
  - Instructions: the Properties groups follow the page from largest to smallest, Page before Bill of Materials and Step Number, Parts List then Divider after Steps per Page
  - Instructions: Page Number is part of the Page group
  - Instructions: every Properties group shows on every page and starts folded, selecting an element opens its group and folds the others
  - Instructions: Show Part IDs in the Bill of Materials group prints the part number under each part of the bill of materials, on by default
  - Instructions: selecting the bill of materials list opens the Parts List group too
  - Instructions: Thumbnail Size goes up to 2048 pixels for large pages
  - Instructions: a full parts list column no longer spills past its area
  - Instructions: a lock beside each page in the Pages panel keeps its elements where they are, and a locked page can't be removed
  - Instructions: a snowflake beside each page freezes it, so it keeps its own settings while changes for all pages skip it; unfreezing follows the other pages again
  - Translations: the merged Page and Properties help, Show Part IDs and the page lock and freeze read right in all nine languages
  - Site: the Properties image is retaken to show the new group order, folded
  - Site: the bill of materials image is retaken to show the part IDs
  - Translations: the new highlight and final model settings read right in all nine languages
  - Site: the Instructions, Rendering, Views and Interface images are retaken to show the panel fold arrows and the new instructions settings, and the Instructions features name Step Flow, the final model, dividers, Model Size, Highlight Width, text files and Undo and Redo
  - Site: the POV-Ray and Blender Render images show the full output path the windows fill in
  - Site: the page notes in SITE.md use plain Markdown comments, and the README says TL:DR

  </details>

  <details>
  <summary>2026-11-16</summary>

  - Site: every footer button has its own logo, with tools, files and steps logos for Toolbars, Files and Steps, GitHub and PayPal logos for GitHub and Donate, and L²CU in green like on MechaBlocks
  - Site: the page styles live in their own files under resources/css, and resources/css/logo.css picks each section's logo, so a new or swapped logo shows without rebuilding the page
  - Translations: Traditional Chinese has about 1,900 more and better translations, from yoyomaimai
  - Translations: Traditional Chinese reads right for Startup, Origin, Author Line, Hip Accessory and Filter Parts
  - Site: Interface covers the window and Panels the panels, with split views told once and showing or hiding panels moved to Panels
  - Translations: every line in all nine languages was reviewed, the missing ones translated, and each language uses one word per meaning, the same words as its menus
  - Translations: snap sizes read as studs, plates and bricks in every language, and each property row letter matches its row name
  - Translations: Yes, No and the other dialog buttons follow the language picked in StudWorx
  - Instructions: Undo and Redo at the start of the page bar, and the main window Undo and Redo shortcuts, take back and put back changes to the instructions, quick changes to one setting counting as one
  - Instructions: Model Size in the Box of the step and cover models sizes them from 10 to 200 percent, on all pages or this page only
  - Instructions: new part outlines follow only what can be seen, no more lines from parts hidden inside or behind others
  - Instructions: Page Background shows its own color on submodel pages instead of the Submodel Background
  - Instructions: Parts List and Notice backgrounds start transparent, like the other boxes and images
  - Preferences: the Get Key button widens to show its whole label in longer languages
  - Translations: every machine-translated line in all nine languages was checked, and about 2,900 wrong ones now read right, like Close that read as near, Light as pale and Stud as earring
  - Translations: Traditional Chinese uses Taiwan words, like 匯入 and 匯出 for import and export
  - Instructions: Step Flow in Steps per Page fills pages row by row or column by column, picked by you instead of by the page shape
  - Instructions: on pages with empty cells the last step stretches to fill them, and the step dividers follow the cells
  - Instructions: Show Final Model puts the finished model in the room left after a model's last step, with the final model box background to color it
  - Instructions: when the last page is full, the finished model goes on a new page, and Show Final Model is greyed out on pages without a model's last step
  - Instructions: Apply to All Pages on a submodel page keeps the main Page Background instead of resetting it
  - Instructions: Cover Title, Cover Author and BOM Title take your own text, or a text file that wins over it, and notices take a text file too
  - Instructions: the Editor panel stays folded when collapsed or dragged down, instead of growing back by a line
  - Instructions: dividers get their own Divider group with margin, padding, rounded ends and Horizontal or Vertical to hide lines where page colors already split the page, and reach the page margins by default without poking past them
  - Instructions: the Steps per Page buttons show only their divider lines, the button is the page, one step per page shows a circle
  - Instructions: new part outlines on large step images no longer show seam lines
  - Instructions: Show Author sits on its own row

  </details>

  <details>
  <summary>2026-11-15</summary>

  - Site: the page styles and scripts carry short notes that say what each part does
  - Site: the link preview card shows the current app window

  </details>

  <details>
  <summary>2026-11-14</summary>

  - Preferences: a new Show Connection Points setting under Use Part Connections turns the green connection marks on or off
  - Preferences: the Connection Points color moved to Steps and Highlighting, and it can now be made see-through
  - Translations: the new Show Connection Points setting and its tooltip are added to every language, waiting to be translated
  - Site: the General and Interface Preferences images are retaken to show the moved settings
  - Colors: docked at the top or bottom, the swatches line up in one row across the whole window, scrolled with the wheel or the arrows at its ends
  - Colors: docked at the top or bottom, the panel no longer squeezes the panels sharing its row
  - Colors: the strip tiles are as big as the buttons, with no blank space under the row, and the color button and gear sit at its right end
  - Colors: the strip holds one row of swatches at most, and can be squeezed down to its title like the other panels
  - Colors: double-clicking an arrow jumps to the start or the end of the strip
  - Colors: undocked, every group starts folded, and the panel grows and shrinks with the groups you open, without shaking or a flickering scroll bar
  - Panels: a panel dragged out by its title follows its contents again, even when the window manager first ignores its new size
  - Colors: putting the panel back no longer makes the Terminal and Keyframes panels taller
  - Colors: docked, the strip opens back up when you drag the divider above it, like the other panels
  - Panels: dragging the divider next to any docked panel, on any side, opens only that panel, even when the panels around it are folded; the panels in between keep their size and move along, and the space comes from the view, then the largest stretching panel; panels that fit their contents, like Properties, give space last
  - Properties: docked, it no longer jumps back to full height while you drag its divider
  - Panels: a new arrow button on each docked panel title folds the panel down to its title, and opens it back to its old size; folding hands the space to the view or the open panel above, opening takes it from the view or the largest stretching panel, so Properties keeps its full height
  - Translations: the new Colors tooltips and help text are added to every language, waiting to be translated
  - Translations: the panel arrow tooltips, the array step numbers and the snap labels that held placeholder guesses are now written out in every language; snap labels follow each language's own words for stud, brick and plate

  </details>

  <details>
  <summary>2026-11-13</summary>

  - Joints: an open Joints panel no longer turns off Highlight New Parts in the model view until a joint is picked
  - Site: every image retaken from the current build, with a new Connection Points image in Modeling
  - Translations: about a hundred strings per language that only changed their mnemonic, punctuation or wording get their old translation back, marked unfinished for review
  - Translations: the Array dialog counts and a status line no longer show broken guesses like 8x {1?} in every language
  - Tests: catalogs are checked to compile with every translation, keep their placeholders and carry no merge guesses
  - Site: screenshots never show larger than their real size, so menus and panels stay sharp
  - Site: every gallery follows the order of the program, window first, then its menus and dialogs, with no image shown twice
  - Site: a Toolbars section explains the Standard, Tools, Time and Transform toolbars
  - Site: a Where Is It section lists the menu, toolbar or right click menu of every command, with its shortcut and a search, read from the program itself
  - Site: Help entries name where to find their command, and the menu screenshots give way to the command list
  - Site: the frame around a screenshot ends where the image ends
  - Minifigure Builder: the preview no longer outlines every part in the highlight color when edge lines show, Highlight New Parts now only marks the model view
  - Tests: the site's feature descriptions are kept short, and the command list is checked to name every command; Linux builds now run the tests that start the program
  - Command line: a hidden --menus option prints every menu path, used to build the site
  - Connections: a part grabbed by one of its studs or holes seats by that one, so a brick with studs on every side can go into a side hole by its top stud
  - Connections: the green connection marks follow the parts as they move, and hide while a part is dragged
  - Insert: a part dropped into empty space lands under the pointer, level with the model, instead of far away toward the horizon
  - Preferences: Show Connection Points, under Use Part Connections, turns the connection marks on or off, and the Connection Points color moved to Steps and Highlighting and takes transparency
  - Model view gear menu: Connection Points moved from Steps and Highlighting to sit under Part Connections, next to Insert First Part at Origin, matching the Editing preferences
  - Windows: dialogs taller than the screen now open with their title bar and buttons on screen, and their pages scroll
  - Missing parts: the placeholder brick for a part the library can't find gets a bold red outline, in the timeline's red, in every step and inside submodels, and the timeline shows the submodel holding it in the same red; selecting it shows the selection outline instead
  - Missing and Moved Parts: Replace Checked saves the file right after swapping the parts, unless Save After Replacing is unchecked in the list or under Preferences, General, Files
  - Preferences: Flexible Parts lists Add Middle Control Point first and Handle Spread last
  - Samples: every sample has a title, and its name line matches its file name, so instructions cover pages show the title instead of New Model.ldr
  - File: Missing and Moved Parts reopens the list of missing and renumbered parts after its warning was closed, without reopening the model
  - Site: feature descriptions rewritten in plain words, saying what each feature does instead of listing its commands, now that the command index names them

  </details>

  <details>
  <summary>2026-11-12</summary>

  - Views: the X, Y and Z axis lines draw on their own, the Grid Origin Lines setting is gone, and the negative Z line shows only once parts go below the base grid
  - Views: the base grid starts at 2 × 2 studs and the height grids one brick high, and they grow past the parts by a Grid Margin in studs and a Height Margin in plates
  - Views: Split Horizontal, Split Vertical and Remove View no longer crash when the model view doesn't have keyboard focus
  - Preferences: Rendering, Views and Interface sort their settings into clearer groups, the colors move to Interface, and color alpha is set in the color picker
  - Explode: the X, Y and Z spreads are multiples from 1.0 to 10.0 instead of percentages, and Explode is greyed out while every spread is 1.0
  - Keyframes: panel buttons are painted in the color theme and run the same commands as the Time toolbar, which gains Set Keys, Remove Keys, Play and Stop
  - Timeline: steps with animation keys list a Keys row per keyed object, and clicking an object in Keyframes or Timeline selects it in the model view
  - Samples: sample-train-tracks.ldr, an S-curve of 9V curved track joined at its train track connections
  - POV-Ray: the POV-Ray path shows in Preferences on Windows and macOS too, so any POV-Ray 3.7 console can replace the bundled one, which stays the fallback, and starts at the bundled console there in place of the Linux path
  - POV-Ray: Windows builds bundle a 64 bit POV-Ray console in place of the 32 bit one
  - POV-Ray: Focal Blur in the Image panel turns the depth of field on or off at any quality, off by default, where High quality always used it, and render presets keep it
  - POV-Ray: Region shows a faded view of the model from the render camera and step until the first render, so the region can be drawn on the model
  - POV-Ray: a Region render keeps only the region, over the faded view of the model, where it was placed into an older image of the same file
  - POV-Ray: Region draws a square with Ctrl held, resizes from its edges and corners, moves when dragged from inside, and is drawn anew when dragged from outside
  - POV-Ray: Editor beside Log shows the scene file with syntax colors, renders your edits when Render using this configuration instead of the model configuration is ticked, greys out the settings it ignores, writes the scene again with Reset, opens a scene file with Open and saves a copy with Save As
  - POV-Ray: the render window is a normal window with minimize and maximize like the Instructions window, and its buttons read Editor, Render, Log
  - Render and Instructions windows: panels use the main window title bar with Float and Close, follow Lock Interface and reopen from the right-click panel list, where the Render panel stays greyed out as the one that never closes
  - Preferences: Download beside Part Connections and LGEO Library fetches the LDCad shadow library and the LGEO library from GitHub into the folder named there or the StudWorx data folder, with a download icon that turns into Update with the refresh icon once the library is there, and Use Part Connections moves to the second row of the Editing group of General, greying out the Part Connections path while off, and the window is a normal window with the window manager buttons like Instructions
  - Connection Points: with Part Connections on, the part under the pointer shows its connection points within a 2x2 stud area, as stud sized cylinders standing out of each opening, filled for male and hollow for female, only where they face the camera, and clips, hinge fingers and balls filled at their own size, all drawn over the parts, toggled under Steps and Highlighting in the view gear menu, with its color in Preferences
  - Connection Points: no longer crash the program with NVIDIA graphics on Windows
  - Snap to Connections: with Part Connections on, a part dragged or inserted over another part seats on the matching stud, anti stud, hole, axle, bar, clip, ball socket or hinge finger nearest the pointer, turning onto side studs and other sideways connections, turned in quarter turns to fit, balls keeping the part's turn and hinge fingers meshing with the gaps of the other side, exactly on the seat even when the part keeps its turn, Alt moves freely, and Joint on Snap sets a hinge joint where a dragged submodel snaps on a single round axis, both in the Movement Snap menu of the toolbar and the Edit menu
  - Flex Path: dragged hose and cable ends seat on the connection nearest the pointer like loose parts, without waiting for the grid, and end connectors show their connection points and take parts snapped onto them
  - Flex Path: the control point cube at a free end rests on the base of its end connector like a seated one, only its color telling free from seated
  - Blender Render: opens as a normal window like POV-Ray Render and Instructions, maximized, with the window manager buttons, and stays open beside the model
  - Edit menu: Movement Snap and Rotation Snap follow Transform, the same menus as the toolbar buttons, so snapping is set without the Standard toolbar shown
  - Train track: the end gizmo is half again as big, and the add marker past an open end is a solid ghost track piece sized on the connection cube, a cube's width away from it
  - Views: inserting parts into a new model no longer zooms the camera onto them, while views still fit an opened model as its parts load
  - Render: the Log panel shows renderer progress on one line updated in place, as in a terminal, instead of running every update together

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

