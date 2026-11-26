# StudWorx tests

All tests run in one Qt Test executable, `studworx_tests`. The current tests cover
object history, property-key comparisons, and the final animation key after
step insertion, removal, and key deletion, including 8,000 deterministic
randomized undo/redo and merge transitions, and how the connection shapes of
the LDCad shadow library are read and fit together and along which axis, on made-up parts.
A brick with studs on top and sides, dragged beside a side hole, must seat the
facing side stud, and its top stud when grabbed there.
Clip, finger and sphere shapes must be read as joints, a round shape in a placed
sub-file must keep its pivot and axis, and poses must blend a hinge along its axis
and a ball along the shortest turn. With `SW_TEST_BINARY` set, a model with a
`0 !SW JOINT PART` line and a `0 !SW JOINT MODEL` line must save both back unchanged.
With `SW_TEST_BINARY` set to a built `studworx`, a one brick model is exported
to POV-Ray and rendered at 64 x 64 (skipped without POV-Ray, `SW_TEST_LIBRARY`
sets an optional parts library). The same binary saves a Trans-Clear model on
transparent and opaque backgrounds, with and without multisampling, to check
that translucent pixels keep their color and opaque images keep the background
(rendered offscreen, or on the running display with `SW_TEST_NATIVE_DISPLAY=1`).
It also exports a box hiding a smaller box to KMZ, to check the archive entries,
the model named after the archive with spaces as underscores, fixed dates, and
that `--kmz-geometry visible` drops the hidden box while `--kmz-geometry all` keeps it.
A BrickLink export with no mapping file set must use the bundled mapping, so
parts and colors leave as their current BrickLink numbers.
The text case helper of the Properties dialog must give UPPERCASE, lowercase,
Sentence case and Title Case, leaving symbols, numbers and email dots alone.
A part whose description reads `~Moved to` must leave the BrickLink and CSV
exports as the part it moved to, merged into one lot with it.
Part exceptions given with `--bricklink-exceptions` must replace the mapping
and come before moved part numbers, read from comma, quoted and tab separated lines.
An HTML export into a folder that already holds a file must add only a folder
named after the model, with a styled `index.html` as its entry page that holds
the first instructions page image and a `step-1` anchor, and with a Landscape
Display the page image must be saved at twice the size `index.html` shows it. A style sheet in the profile must replace the built-in one, the built-in one must name the feature of each block in a comment set apart by blank lines,
and a step model box Corner Radius must round the page cards.
`--export-pdf` without a file name must save a PDF beside the model holding
one sheet per instructions page, and `--export-pages` into a missing folder must make it
and save one PNG per page at the page setup size, named after the model with its page number.
`--flatten` must replace a submodel inside a moved and turned submodel with its
parts at their world positions, with color 16 parts taking the color of the submodel above,
and `--undo` must then bring back the original main model.
LeoCAD meta commands must read as their StudWorx forms and other lines pass
unchanged, a model must convert to either format and back with its line endings
and indents kept, and with `SW_TEST_MODEL` set to a LeoCAD model, that model must
round trip with every meta command converted. A saved model must keep its `0 !SW CUTAWAY MODEL` line with its values, and drop
a cutaway line with no normal.
A LEGO® Digital Designer `.lxfml` model opened on the command line must load as an
untitled project with its part, checked by saving it with `--flatten`.
Rotation tests turn parts in 15, 30 and 90 degree steps around every axis, in
the part's own frame and in model coordinates, and check that each quarter turn
lands exactly on 0 and 1, that the turned part keeps its position and moves by
whole steps, and that real angles are never rounded.
The grid extent test grows the base grid in studs and the height grids in plates
around parts above and below the floor, and checks that the margins apply only
away from the origin, that a zero margin hugs the parts and that infinite bounds
leave the grid alone.
A step view test turns the camera to absolute and relative angles and reads
them back, so a saved step view never comes back mirrored, and checks that
absolute zero looks at the front and relative zero is the home view.
A profile test writes settings inside a batch, in a temporary settings folder,
and checks that they read back at once, that a nested batch and a removed key
behave, and that the file is written only when the batch ends. A first start
must give every setting a line in the settings file and keep a value already set.
Instructions part count tests read the source tree: the part count and submodel preview
properties and elements must be named in the instructions file reader and in
`commands-meta.md`, every part count toggle and the submodel preview must default
off, and the default text must be the translatable `%1 PIECES`. With `SW_TEST_BINARY`
set, a model with those properties set must save them back unchanged. The counting
itself (a submodel counted by its parts times its uses, hidden parts left out, a step
page summing the new parts of its steps) is not covered, it needs the application model.
The publication text test, with `SW_TEST_BINARY` set, saves a model with `COVER FRONT` and `COVER BACK` `SHOW_*` and `*_TEXT` lines for all thirteen elements (Title, Author, Product ID, Version, Publication Date, URL, Contact, Copyright, Notice, Comments, Measurements, Weight, Pieces) and expects each back, checks the template reader still takes any property line, and expects the old form lines to be dropped. The cover elements test exports models with a Title on both covers and expects each cover to draw its own text and not the other one. The placed element test exports a front cover with a moved Product ID alone and again with Comments shown, and expects every Product ID pixel to stay where it was.
The model publication test, with `SW_TEST_BINARY` set, saves a model with `PRODUCT_ID`, `VERSION`, `PUBLICATION_DATE`, `URL`, `CONTACT`, `NOTICE` and `!LICENSE` lines and expects each back once. The publication codes test exports the back cover of two models that differ only in `PRODUCT_ID`, with `%id` in the Publication URL text, and expects different pages. The publication pages test exports three models that differ only in one cover element, on the front or back cover, and expects only the first or only the last page to change, for the measurements on the front cover and the weight and comments on the back cover too. The summary codes test exports two back covers with `%comments`, `%size` or `%weight` that differ only in the comments, the part position or the part count and expects different pages.
The layout test, with `SW_TEST_BINARY` set, saves a model written before the cover cells (a Product ID shown and moved, the Author placed) and a model without settings. The old one must keep the earlier cells (headings on cell 7, Pieces on 16, the other texts on 22), its own Author cell and its offset; the new one must take the current cells; both must write every placement, anchor and alignment, so a later default change never moves a saved page.
The template notices test, with `SW_TEST_BINARY` set, sets a template with front cover, back cover and footer notices as the default template, checks that a fresh model saves all three back and draws them on its covers, and reads `sw_instructions_file.cpp` for the cover page mapping Save Template writes (the dialog itself is not reachable from the tests).
Documentation tests read the source tree: `index.html` must name every command
by its menu name (numbered families such as Viewpoint 1 to 9 are left out), every
command with a menu name must have `WhatsThis` help, and every image in
`resources/images` must be named `section-kind-subject` and linked from `SITE.md`,
while every image linked from `SITE.md`, `README.md` and the site template must exist.
Every feature bullet in `SITE.md` must stay under 450 characters, so it explains
instead of listing names, and `index.html` must hold no unfilled placeholder.
With `SW_TEST_BINARY` set, `studworx --menus` must start without a parts library,
print every command as a menu path, a tab and its shortcut, and show no file path.

The executable creates a `QCoreApplication`; apart from the `SW_TEST_BINARY`
checks it does not open windows or require an OpenGL context, the parts library, or network access. It uses the same Qt
headers as the application, plus Qt Test, and supports Qt 5 and Qt 6.

## Build and run

From the repository root, with the intended Qt installation's `qmake` on PATH:

```sh
mkdir -p build/testing
cd build/testing
qmake ../../source/testing/testing.pro CONFIG+=release CONFIG-=debug
make -j
make check
```

On Windows, use the make tool for the selected Qt kit, such as `nmake`.

To list test functions or run one function directly:

```sh
./studworx_tests -functions
./studworx_tests HistoryStoresOnlyChangedObjects
```

On Windows, the executable is `studworx_tests.exe` and may be in `release/`.
`make check` handles the configured executable location.

To write both console output and a JUnit report:

```sh
make check TESTARGS="-o test-results.xml,junitxml -o -,txt"
```

## Add tests

1. Declare a named `void` function under `private slots` in `sw_tests.h`.
2. Define it in the relevant test source file as `void swTests::FunctionName()`.
3. If adding a new source file, list it in `testing.pro`.

Use `QVERIFY`, `QCOMPARE`, or other Qt Test checks so assertions remain active in
release builds and failures identify the function and source line. Each test
must create its own state. Use temporary directories for writable fixtures and
fixed seeds for randomized tests; include the seed and iteration in failures.
Tests should call production code directly.

Application-level tests needing more source files should declare those sources
and dependencies in `testing.pro`; shared source lists can move into `.pri` files
when needed. Keep application startup and packaging out of the test target.

## GitHub CI

`.github/workflows/continuous.yml` builds and runs this target on Linux based systems (Qt 5),
macOS (Qt 6), and Windows x64 (Qt 6 with MSVC 2022), on pull requests to `main`,
pushes to `main`, and manual runs. JUnit reports are uploaded as artifacts,
including on test failure. Adding a test does not require changing the workflow
or `main.cpp`.

The Windows job uses `nmake` and a single release configuration. To reproduce its
commands in an x64 MSVC developer command prompt with Qt on PATH:

```bat
mkdir build\testing
cd build\testing
qmake ..\..\source\testing\testing.pro -spec win32-msvc CONFIG+=release CONFIG-=debug CONFIG-=debug_and_release
nmake /NOLOGO
nmake /NOLOGO check "TESTARGS=-o test-results.xml,junitxml -o -,txt"
```
