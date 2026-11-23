# Meta Commands

Meta commands are line type 0 lines of an LDraw file, written as `0 <COMMAND> <parameters>` with the command in capitals ([LDraw File Format](https://www.ldraw.org/article/218.html)). New official commands start with `!`, unofficial ones should start with `!` too and be based on the name of the program that defines them, so StudWorx writes its own commands under the `!SW` marker.

This list holds every meta command StudWorx handles, sorted by tool or context, official ones first. The handling column reads:

| Handling | Meaning |
| --- | --- |
| Read | interpreted when a model or the parts library is loaded |
| Write | written when a model is saved |
| Keep | not interpreted, saved back unchanged at its place in the model |
| Ignore | skipped when the parts library is loaded |

## LDraw Official

### File Format

From the [LDraw File Format](https://www.ldraw.org/article/218.html).

| Command | Handling | Use |
| --- | --- | --- |
| `0 // <comment>` | Keep | comment, the bare `0 <comment>` form is deprecated |
| `0 STEP` | Read, Write | ends a building step |
| `0 WRITE <message>` | Keep | message shown when the line is reached |
| `0 PRINT <message>` | Keep | same as `WRITE` |
| `0 CLEAR` | Keep | clears the screen in viewers |
| `0 PAUSE` | Keep | pauses viewers until a key press |
| `0 SAVE` | Keep | saves the screen in viewers |

### Header

From the [File Header Specification](https://www.ldraw.org/article/398.html).

| Command | Handling | Use |
| --- | --- | --- |
| `0 <description>` | Read, Write | first line of a model, its description |
| `0 Name: <file name>` | Read, Write | model file name |
| `0 Author: <name>` | Read, Write | model author |
| `0 !LDRAW_ORG <type> <update>` | Keep | official library part type |
| `0 !LICENSE <license>` | Keep | file license |
| `0 !HELP <text>` | Keep | usage notes |
| `0 BFC CERTIFY <CW\|CCW>` | Read | see [Back Face Culling](#back-face-culling) |
| `0 !CATEGORY <category>` | Keep | part category |
| `0 !KEYWORDS <words>` | Keep | part search words |
| `0 !CMDLINE <options>` | Keep | viewer command line options |
| `0 !PREVIEW <color> <matrix>` | Keep | part preview orientation |
| `0 !HISTORY <date> <author> <text>` | Keep | file change history |
| `0 ~Moved to <file>` | Keep | moved part description |

### Multi-Part Documents

From the [MPD Language Extension](https://www.ldraw.org/article/47.html).

| Command | Handling | Use |
| --- | --- | --- |
| `0 FILE <file name>` | Read, Write | starts a submodel |
| `0 NOFILE` | Read | ends a submodel |
| `0 !DATA <file name>` | Read, Write | starts an embedded file, such as a texture image |
| `0 !: <data>` | Read, Write | base64 line of an embedded file |

### Colours

From the [Colour Definition Language Extension](https://www.ldraw.org/article/299.html), read from `LDConfig.ldr` and models.

| Command | Handling | Use |
| --- | --- | --- |
| `0 !COLOUR <name> CODE <code> VALUE <#RRGGBB> EDGE <#RRGGBB\|code>` | Read | color definition |
| `ALPHA <0-255>` | Read | transparency |
| `LUMINANCE <0-255>` | Read | glow |
| `CHROME`, `PEARLESCENT`, `RUBBER`, `MATTE_METALLIC`, `METAL` | Read | finish |
| `MATERIAL <GLITTER\|SPECKLE> ...` | Read | material parameters |

### Texture Mapping

From the [Texture Mapping Language Extension](https://www.ldraw.org/texmap-spec.html), read from parts.

| Command | Handling | Use |
| --- | --- | --- |
| `0 !TEXMAP START <method> <parameters> <image> [GLOSSMAP <image>]` | Read | maps a texture on the following lines |
| `0 !TEXMAP NEXT <method> <parameters> <image> [GLOSSMAP <image>]` | Read | maps a texture on the next line only |
| `PLANAR`, `CYLINDRICAL`, `SPHERICAL` | Read | projection method |
| `0 !TEXMAP FALLBACK` | Read | starts the geometry used without texture support |
| `0 !TEXMAP END` | Read | ends the mapping |
| `0 !: <line>` | Read | textured geometry line inside a mapping |

### Back Face Culling

From the [BFC Language Extension](https://www.ldraw.org/article/415.html), read from parts.

| Command | Handling | Use |
| --- | --- | --- |
| `0 BFC CW`, `0 BFC CCW` | Read | winding of the following polygons |
| `0 BFC INVERTNEXT` | Read | inverts the winding of the next subfile |
| `0 BFC CERTIFY <CW\|CCW>` | Read | certifies the file, with its winding |
| `0 BFC NOCERTIFY`, `0 BFC CLIP`, `0 BFC NOCLIP` | Ignore | culling is not switched per file |

### Avatars

| Command | Handling | Use |
| --- | --- | --- |
| `0 !AVATAR CATEGORY "<category>"` | Keep | avatar parts, see [Avatar Library](https://www.ldraw.org/article/346.html) |

## StudWorx

StudWorx commands take the form `0 !SW <PROPERTY> <OBJECT> <arguments>`. An animated property writes one `<PROPERTY>_KEY` line per key, with the step before the values, as in `0 !SW POSITION_KEY CAMERA 3 0 -80 40`. Matrices are written as 9 numbers of a 3 by 3 rotation, positions and vectors as 3 numbers.

### Part

Written before the type 1 line of the part they apply to.

| Command | Handling | Use |
| --- | --- | --- |
| `0 !SW STEP_HIDE PART <step>` | Read, Write | step that hides the part |
| `0 !SW HIDDEN PART` | Read, Write | part hidden in the editor |
| `0 !SW PIVOT PART <x> <y> <z> <matrix>` | Read, Write | custom pivot point |
| `0 !SW POSITION_KEY PART <step> <x> <y> <z>` | Read, Write | animated position |
| `0 !SW ORIENTATION_KEY PART <step> <matrix>` | Read, Write | animated orientation |
| `0 !SW JOINT PART <HINGE\|BALL> <pivot x y z> <axis x y z> <rotation x y z>` | Read, Write | joint of a placed submodel, rotation as a vector in degrees, one line per joint |

### Camera

| Command | Handling | Use |
| --- | --- | --- |
| `0 !SW FOV CAMERA <degrees> ZNEAR <near> ZFAR <far>` | Read, Write | field of view and clipping planes |
| `0 !SW POSITION CAMERA <x> <y> <z>` | Read, Write | camera position, `POSITION_KEY` when animated |
| `0 !SW TARGET_POSITION CAMERA <x> <y> <z>` | Read, Write | target position, `TARGET_POSITION_KEY` when animated |
| `0 !SW UP_VECTOR CAMERA <x> <y> <z>` | Read, Write | up direction, `UP_VECTOR_KEY` when animated |
| `0 !SW HIDDEN CAMERA` | Read, Write | camera hidden in the editor |
| `0 !SW ORTHOGRAPHIC CAMERA` | Read, Write | orthographic projection |
| `0 !SW LABEL CAMERA <name>` | Read, Write | camera name, ends the camera |

### Light

| Command | Handling | Use |
| --- | --- | --- |
| `0 !SW SHADOWLESS LIGHT` | Read, Write | light casts no shadows |
| `0 !SW POSITION LIGHT <x> <y> <z>` | Read, Write | light position |
| `0 !SW ORIENTATION LIGHT <matrix>` | Read, Write | light direction |
| `0 !SW COLOR_RGB LIGHT <red> <green> <blue>` | Read, Write | light color, 0 to 1 |
| `0 !SW BLENDER_RADIUS LIGHT <size>` | Read, Write | Blender point and spot radius |
| `0 !SW BLENDER_ANGLE LIGHT <degrees>` | Read, Write | Blender sun angle |
| `0 !SW BLENDER_POWER LIGHT <watts>` | Read, Write | Blender power |
| `0 !SW AREA_SIZE_X LIGHT <size>` | Read, Write | area width |
| `0 !SW AREA_SIZE_Y LIGHT <size>` | Read, Write | area height |
| `0 !SW POVRAY_POWER LIGHT <power>` | Read, Write | POV-Ray power |
| `0 !SW POVRAY_FADE_DISTANCE LIGHT <distance>` | Read, Write | POV-Ray fade distance |
| `0 !SW POVRAY_FADE_POWER LIGHT <power>` | Read, Write | POV-Ray fade power |
| `0 !SW SPOT_CONE_ANGLE LIGHT <degrees>` | Read, Write | spot cone |
| `0 !SW SPOT_PENUMBRA_ANGLE LIGHT <degrees>` | Read, Write | spot penumbra |
| `0 !SW POVRAY_SPOT_TIGHTNESS LIGHT <tightness>` | Read, Write | POV-Ray spot tightness |
| `0 !SW AREA_SHAPE LIGHT <RECTANGLE\|SQUARE\|DISK\|ELLIPSE>` | Read, Write | area shape |
| `0 !SW POVRAY_AREA_GRID_X LIGHT <count>` | Read, Write | POV-Ray area grid columns |
| `0 !SW POVRAY_AREA_GRID_Y LIGHT <count>` | Read, Write | POV-Ray area grid rows |
| `0 !SW TYPE LIGHT <POINT\|SPOT\|DIRECTIONAL\|AREA>` | Read, Write | light type |
| `0 !SW LABEL LIGHT <name>` | Read, Write | light name, ends the light |

Each light property takes the `_KEY` form when animated.

### Model

| Command | Handling | Use |
| --- | --- | --- |
| `0 !SW COMMENT MODEL <text>` | Read, Write | model comment, one line each |
| `0 !SW AUTHOR MODEL <name>` | Read | author, written as `0 Author:` |
| `0 !SW DESCRIPTION MODEL <text>` | Read | description, written as the first line |
| `0 !SW ASSET_SOURCE MODEL <base64 path>` | Read, Write | folder searched for embedded file sources |
| `0 !SW ASSET_SOURCE_PRIORITY MODEL SOURCE` | Read, Write | search the asset sources before the model folder |
| `0 !SW CUTAWAY MODEL <CUBE\|SPHERE\|CYLINDER> <x> <y> <z> <normal x> <normal y> <normal z> <size>` | Read, Write | cutaway kept with the model, center and size in StudWorx units |
| `0 !SW JOINT MODEL "<name>" "<model file>" <part index> <rx> <ry> <rz> [<joint>]` | Read, Write | joint angles of a saved pose, one line per joint, the joint number counting from 0 and left out for the first, in the file of the model the joints belong to |
| `0 !SW POSE "<name>" "<model file>" <part index> <rx> <ry> <rz>` | Read | older name of a pose line, saved as `0 !SW JOINT MODEL` once the model is edited |

### POV-Ray

| Command | Handling | Use |
| --- | --- | --- |
| `0 !SW HEADER_FILE_INCLUDE POV_RAY <path>` | Read, Write | file included before the scene |
| `0 !SW FOOTER_FILE_INCLUDE POV_RAY <path>` | Read, Write | file included after the scene |
| `0 !SW FLOOR_AXIS POV_RAY <0\|1\|2>` | Read, Write | floor axis, x, y or z |
| `0 !SW FLOOR_COLOR_RGB POV_RAY <red> <green> <blue>` | Read, Write | floor color |
| `0 !SW FLOOR_AMBIENT POV_RAY <value>` | Read, Write | floor ambient |
| `0 !SW FLOOR_DIFFUSE POV_RAY <value>` | Read, Write | floor diffuse |
| `0 !SW EXCLUDE_FLOOR POV_RAY` | Read, Write | no floor |
| `0 !SW EXCLUDE_BACKGROUND POV_RAY` | Read, Write | no background |
| `0 !SW NO_REFLECTION POV_RAY` | Read, Write | no reflections |
| `0 !SW NO_SHADOWS POV_RAY` | Read, Write | no shadows |
| `0 !SW USE_LGEO POV_RAY` | Read, Write | use the LGEO library |

### Instruction

Instructions settings, written in the main model header. Models are quoted file names, `-` for none. Steps are `STEP <model> <step>`, pages `<STEPS|COVER|BILL_OF_MATERIALS|BLANK|BACK_COVER> <model> <step> <rank>` and page elements `<element> <model> <step>`, with the element `NONE`, `STEP_NUMBER`, `PARTS_LIST`, `COVER_TITLE`, `COVER_AUTHOR`, `BILL_OF_MATERIALS`, `STEP_IMAGE`, `COVER_IMAGE`, `PAGE_NUMBER`, `NOTICE`, `POINTER`, `COVER_PART_COUNT`, `BILL_OF_MATERIALS_PART_COUNT`, `PAGE_PART_COUNT`, `SUBMODEL_PREVIEW`, `SUBMODEL_PREVIEW_PART_COUNT`, `PARTS_LIST_QUANTITY` or `IMAGE`. Values are `TRUE`/`FALSE`, numbers, `#RRGGBBAA` colors or text to the end of the line, with `\\`, `\"` and `\n` escaped.

| Command | Handling | Use |
| --- | --- | --- |
| Template | Read | a model file holding `PROPERTY`, `STEPS_PER_PAGE` and `IMAGE` lines, written by Save Template; Apply Template copies them into the model, and the Default Template of the preferences applies them under every model, whose own lines win |
| `0 !SW PROPERTY INSTRUCTION <name> <value>` | Read, Write | document property, such as `STEP_NUMBER_BORDER_WIDTH 3` or `COVER_TITLE_FILE title.txt`; `COVER_TITLE`, `COVER_AUTHOR` and `BILL_OF_MATERIALS_TITLE` take `_TEXT` and `_FILE`, the file wins over the text, and files are relative to the model file |
| `0 !SW PROPERTY INSTRUCTION USE_DOCUMENT_FONT <TRUE\|FALSE>` | Read, Write | give all text the typeface of `DOCUMENT_FONT`, each text keeps its own size, default `FALSE`; `USE_DOCUMENT_FONT_SIZE` gives all text the size of `DOCUMENT_FONT` and each text keeps its typeface. A file with `USE_DOCUMENT_FONT TRUE` and no `USE_DOCUMENT_FONT_SIZE` line uses both |
| `0 !SW PROPERTY INSTRUCTION SHOW_COVER_PART_COUNT <TRUE\|FALSE>` | Read, Write | show the part count of the whole model on the cover, default `FALSE`; `SHOW_BILL_OF_MATERIALS_PART_COUNT` does the same under the bill of materials title and `SHOW_PAGE_PART_COUNT` shows the new parts of the steps on each step page |
| `0 !SW PROPERTY INSTRUCTION PART_COUNT_TEXT <text>` | Read, Write | text of all part counts and submodel previews with `%1` for the number, default `%1 PIECES`; `PART_COUNT_FILE` takes a text file relative to the model file that wins over the text |
| `0 !SW PROPERTY INSTRUCTION PART_COUNT_FONT <font>` | Read, Write | font of the cover and bill of materials part count, the bill of materials one in the size of the `BILL_OF_MATERIALS_TITLE_FONT`, `PART_COUNT_COLOR` is its color; `PAGE_PART_COUNT_FONT` and `PAGE_PART_COUNT_COLOR` style the step page count and the submodel preview count |
| `0 !SW PROPERTY INSTRUCTION PART_COUNT_PLACEMENT <0-24>` | Read, Write | placement of the cover part count, `BILL_OF_MATERIALS_PART_COUNT_PLACEMENT` (top right, beside the title, or past the title when they meet) and `PAGE_PART_COUNT_PLACEMENT` place the others, and `SUBMODEL_PREVIEW_PLACEMENT` the preview; on the page, the outer cells 0-4, 5, 9, 10, 14, 15, 19 and 20-24 put these, the cover texts and the page number against the paper edge; `SUBMODEL_PREVIEW_PART_COUNT_PLACEMENT` places the preview count around the preview, default `22`, and `PARTS_LIST_QUANTITY_PLACEMENT` each parts list quantity around its part, default `21`, inner cells inside, outer cells outside |
| `0 !SW PROPERTY INSTRUCTION PART_COUNT_<MARGIN\|PADDING\|BORDER_WIDTH\|BORDER_COLOR\|BACKGROUND_COLOR\|CORNER_RADIUS> <value>` | Read, Write | box of the part counts, `SUBMODEL_PREVIEW_` takes the same suffixes for the box of the submodel preview |
| `0 !SW PROPERTY INSTRUCTION SHOW_SUBMODEL_PREVIEW <TRUE\|FALSE>` | Read, Write | show the finished submodel with its part count on the first page of each submodel, default `FALSE`; `SUBMODEL_PREVIEW_MODEL_SIZE` is the longest side of its model in points, default `0` fits it to its box |
| `0 !SW PROPERTY INSTRUCTION FINAL_MODEL_MARGIN <value>` | Read, Write | box of the final model area: `FINAL_MODEL_MARGIN`, `FINAL_MODEL_PADDING`, `FINAL_MODEL_BORDER_WIDTH`, `FINAL_MODEL_BORDER_COLOR`, `FINAL_MODEL_CORNER_RADIUS` and `FINAL_MODEL_BACKGROUND_COLOR`, and a page can have its own background; default `0`, black and transparent |
| `0 !SW PROPERTY INSTRUCTION STEP_MARGIN <value>` | Read, Write | box of each step area: `STEP_MARGIN`, `STEP_PADDING`, `STEP_BORDER_WIDTH`, `STEP_BORDER_COLOR`, `STEP_CORNER_RADIUS`, and `STEP_BACKGROUND_COLOR` as its background color, default `0`, black and transparent |
| `0 !SW PROPERTY INSTRUCTION SHOW_FINAL_MODEL_PAGE <TRUE\|FALSE>` | Read, Write | close the main model steps with a page showing the finished model, default `TRUE` |
| `0 !SW PROPERTY INSTRUCTION SHOW_BACK_COVER <TRUE\|FALSE>` | Read, Write | end the instructions with a back cover after the bill of materials, default `TRUE`, blank until it gets images |
| `0 !SW PAGE_PROPERTY INSTRUCTION STEP <model> <step> <name> <value>` | Read, Write | property of the page starting at the step |
| `0 !SW STEPS_PER_PAGE INSTRUCTION <rows> <columns> [VERTICAL] [FINAL_MODEL]` | Read, Write | steps per page, filled down instead of across, with the finished model in the room left; old files may still have `FINAL_MODEL_COLOR`, read as the page color for `FINAL_MODEL_BACKGROUND_COLOR` |
| `0 !SW PAGE_STEPS_PER_PAGE INSTRUCTION STEP <model> <step> <rows> <columns> [VERTICAL] [FINAL_MODEL]` | Read, Write | steps per page from the step on, same words as above |
| `0 !SW BLANK_PAGE INSTRUCTION STEP <model> <step>` | Read, Write | blank page before the step |
| `0 !SW OFFSET INSTRUCTION <element> <model> <step> <x> <y>` | Read, Write | element moved from its place |
| `0 !SW LOCK INSTRUCTION <element> <model> <step> <x> <y>` | Read, Write | element locked at a position |
| `0 !SW NOTICE INSTRUCTION <id> PAGE <page> POSITION <x> <y> [FILE "<file>"] TEXT <text>` | Read, Write | notice on a page, the text file wins over the text |
| `0 !SW IMAGE INSTRUCTION <id> <NONE\|FRONT\|BACK\|BOTH> PLACEMENT <0-24> SIZE <1-100> FILE "<file>"` | Read, Write | linked image on the front cover, the back cover or both, like a logo, placed on the page grid with the outer cells against the paper edge, its width in percent of the page width, the file relative to the model file; `NONE` hides an image of the default template |
| `0 !SW PROPERTY INSTRUCTION SHOW_HEADER_BAND <TRUE\|FALSE>` | Read, Write | show the header band on the step and blank pages, default `FALSE`; `SHOW_FOOTER_BAND`, `SHOW_LEFT_BAND` and `SHOW_RIGHT_BAND` do the same for the other edges, `HEADER_BAND_SIZE` is the band thickness from the paper edge, default `72`, `HEADER_BAND_COLOR` its `#RRGGBBAA` background color, default transparent, and `HEADER_BAND_RESERVE` keeps the page content clear of it, default `TRUE`; the other edges have `FOOTER_BAND_SIZE`, `LEFT_BAND_SIZE`, `RIGHT_BAND_SIZE`, `FOOTER_BAND_COLOR`, `LEFT_BAND_COLOR`, `RIGHT_BAND_COLOR`, `FOOTER_BAND_RESERVE`, `LEFT_BAND_RESERVE` and `RIGHT_BAND_RESERVE`; every band is a box, with `HEADER_BAND_MARGIN`, `HEADER_BAND_PADDING`, `HEADER_BAND_BORDER_WIDTH`, `HEADER_BAND_BORDER_COLOR`, `HEADER_BAND_CORNER_RADIUS`, and the same five for the other bands: `FOOTER_BAND_MARGIN`, `FOOTER_BAND_PADDING`, `FOOTER_BAND_BORDER_WIDTH`, `FOOTER_BAND_BORDER_COLOR`, `FOOTER_BAND_CORNER_RADIUS`, `LEFT_BAND_MARGIN`, `LEFT_BAND_PADDING`, `LEFT_BAND_BORDER_WIDTH`, `LEFT_BAND_BORDER_COLOR`, `LEFT_BAND_CORNER_RADIUS`, `RIGHT_BAND_MARGIN`, `RIGHT_BAND_PADDING`, `RIGHT_BAND_BORDER_WIDTH`, `RIGHT_BAND_BORDER_COLOR` and `RIGHT_BAND_CORNER_RADIUS`, all default `0` and black; a page can have its own values |
| `0 !SW PROPERTY INSTRUCTION SHOW_WATERMARK <TRUE\|FALSE>` | Read, Write | show the watermark on the page, default `TRUE`, a page can hide it; `WATERMARK_OPACITY` is the watermark opacity in percent, default `25`; the watermark is a box with `WATERMARK_MARGIN`, `WATERMARK_PADDING`, `WATERMARK_BORDER_WIDTH`, `WATERMARK_BORDER_COLOR`, `WATERMARK_CORNER_RADIUS` and `WATERMARK_BACKGROUND_COLOR`, default `0`, black and transparent |
| `0 !SW PROPERTY INSTRUCTION LOCK_HOME_VIEW <TRUE\|FALSE>` | Read, Write | draw every model from the home view instead of the model view, default `FALSE`, a step with its own step view keeps it |
| `0 !SW NOTICE INSTRUCTION <id> SHOW <HEADER\|FOOTER\|LEFT\|RIGHT\|WATERMARK> POSITION <x> <y> [FILE "<file>"] TEXT <text>` | Read, Write | notice repeated on every step page in a band or as the watermark, the position is relative to the band, `%page` and `%title` in the text fill in the page number and the model title; images also take `HEADER`, `FOOTER`, `LEFT`, `RIGHT` and `WATERMARK` as their pages |
| `0 !SW POINTER INSTRUCTION <id> PAGE <page> SOURCE <element> ANCHOR <AUTO\|0-7> STYLE <DEFAULT\|LINE\|WEDGE\|L_SHAPE\|Z_SHAPE\|U_SHAPE> TIP <DEFAULT\|ARROW\|DOT\|HOLLOW_DOT> TARGET <POINT\|ELEMENT\|PART> <element> PART <index> POSITION <x> <y> [AT <fx> <fy>] [BEND <0-1>] [REACH <length>]` | Read, Write | pointer from an element border to a point, element or part; `AT` is where the tip sits on its target element as a fraction of its box, `BEND` where the Z shape bends from base to tip, `REACH` how far the U shape reaches out |

### Group

| Command | Handling | Use |
| --- | --- | --- |
| `0 !SW BEGIN GROUP <name>` | Read, Write | starts a group, groups nest |
| `0 !SW END GROUP` | Read, Write | ends the innermost group |

### Synthesized Path

Flexible parts such as hoses and cables, built from control points.

| Command | Handling | Use |
| --- | --- | --- |
| `0 !SW BEGIN SYNTH_PATH` | Read, Write | starts the control points of the next part |
| `0 !SW CONTROL_POINT SYNTH_PATH <x> <y> <z> <matrix> <scale>` | Read, Write | control point |
| `0 !SW END SYNTH_PATH` | Read, Write | ends the control points |

### Clipboard

| Command | Handling | Use |
| --- | --- | --- |
| `0 !SW PROJECT CLIPBOARD <uuid>` | Read, Write | source project of copied parts, clipboard only |

## LeoCAD

LeoCAD commands take the form `0 !LEOCAD <OBJECT> <PROPERTY> <arguments>`. They are read as the matching StudWorx commands. A model opened with them asks to migrate, a migrated model is saved with StudWorx commands and a kept model is saved with LeoCAD commands.

| LeoCAD | StudWorx |
| --- | --- |
| `0 !LEOCAD PIECE <PROPERTY>` | `0 !SW <PROPERTY> PART` |
| `0 !LEOCAD <OBJECT> ROTATION`, `ROTATION_KEY` | `0 !SW ORIENTATION <OBJECT>`, `ORIENTATION_KEY` |
| `0 !LEOCAD LIGHT COLOR`, `COLOR_KEY` | `0 !SW COLOR_RGB LIGHT`, `COLOR_RGB_KEY` |
| `0 !LEOCAD <OBJECT> NAME <name>` | `0 !SW LABEL <OBJECT> <name>` |
| `0 !LEOCAD CAMERA HIDDEN ORTHOGRAPHIC NAME <name>` | `0 !SW HIDDEN CAMERA`, `0 !SW ORTHOGRAPHIC CAMERA`, `0 !SW LABEL CAMERA <name>` |
| `0 !LEOCAD LIGHT TYPE <type> NAME <name>` | `0 !SW TYPE LIGHT <type>`, `0 !SW LABEL LIGHT <name>` |
| `0 !LEOCAD POV_RAY HEADER_INCLUDE_FILE`, `FOOTER_INCLUDE_FILE` | `0 !SW HEADER_FILE_INCLUDE POV_RAY`, `FOOTER_FILE_INCLUDE` |
| `0 !LEOCAD SYNTH BEGIN`, `CONTROL_POINT`, `END` | `0 !SW BEGIN SYNTH_PATH`, `CONTROL_POINT`, `END` |
| `0 !LEOCAD GROUP BEGIN <name>`, `END` | `0 !SW BEGIN GROUP <name>`, `END` |

Every other LeoCAD property keeps its name, as in `0 !LEOCAD PIECE PIVOT` read as `0 !SW PIVOT PART`.

## MLCad

From the [list of known unofficial meta commands](https://wiki.ldraw.org/wiki/List_of_Known_Unofficial_META_Commands).

| Command | Handling | Use |
| --- | --- | --- |
| `0 ROTSTEP <x> <y> <z> [REL\|ABS\|ADD]` | Read, Write | ends a step and sets its view rotation |
| `0 ROTSTEP END` | Read, Write | ends a step and resets the view rotation |
| `0 BACKGROUND <image>` | Keep | step background |
| `0 BUFEXCHG <A-H> <STORE\|RETRIEVE>` | Keep | buffer exchange |
| `0 GHOST <line>` | Keep | ghosted line |
| `0 GROUP <count> <name>` | Keep | MLCad group |
| `0 MLCAD BTG <name>` | Keep | belongs to group |
| `0 MLCAD HIDE <line>` | Keep | hidden line |
| `0 MLCAD SKIP_BEGIN`, `0 MLCAD SKIP_END` | Keep | skipped lines |
| `0 MLCAD ARROW_BEGIN`, `0 MLCAD ARROW_END` | Keep | arrows |
| `0 ROTATION CENTER`, `CONFIG`, `AXLE` | Keep | rotation centers |
| `0 MLCAD SPRING`, `FLEXHOSE`, `RUBBER_BELT` | Keep | generated flexible parts |

## LPub

| Command | Handling | Use |
| --- | --- | --- |
| `0 !LPUB <command>`, `0 LPUB <command>` | Keep | instruction layout, parts lists, callouts and placement |

## LSynth

| Command | Handling | Use |
| --- | --- | --- |
| `0 SYNTH BEGIN <type> <color>` | Keep | starts a synthesized part |
| `0 SYNTH END` | Keep | ends a synthesized part |
| `0 SYNTH INSIDE`, `OUTSIDE`, `CROSS` | Keep | constraint side |
| `0 SYNTH SHOW`, `HIDE` | Keep | constraint visibility |
| `0 SYNTHESIZED BEGIN`, `0 SYNTHESIZED END` | Keep | generated lines |

The StudWorx `SYNTH_PATH` object is named apart from these commands, which describe a different format.

## L3P

| Command | Handling | Use |
| --- | --- | --- |
| `0 L3P IFPOV`, `IFNOTPOV`, `ELSEPOV`, `ENDPOV` | Keep | POV-Ray only and non POV-Ray lines |

## LDLite

| Command | Handling | Use |
| --- | --- | --- |
| `0 COLOR`, `0 COLOUR`, `0 COLORNAME`, `0 COLOURNAME` | Keep | color definitions |
| `0 TRANSLATE`, `0 ROTATE`, `0 SCALE`, `0 TRANSFORM`, `0 MATRIX` | Keep | transformations |
| `0 POINT` | Keep | point |
| `0 CMDLINE` | Keep | command line options |

## Blender

The Blender LDraw import add-on handles these commands when Import Cameras and Import Lights in the Scene panel of the Blender Render window ask for it. The step StudWorx hands to Blender is written with LeoCAD meta commands, the form the add-on reads cameras, lights and groups from.

| Command | Option | Use |
| --- | --- | --- |
| `0 BFC` | BFC | back face culling |
| `0 CLEAR` | CLEAR Command | hides earlier parts from that step on |
| `0 !LEOCAD GROUP`, `0 GROUP` | GROUP Command | collections from groups |
| `0 !LEOCAD CAMERA` | Import Cameras | cameras |
| `0 !LEOCAD LIGHT` | Import Lights | lights |
| `0 PRINT`, `0 WRITE` | PRINT/WRITE Command | messages printed to the console |
| `0 STEP` | STEP Command, STEP Groups | step keys and step collections |
| `0 !TEXMAP`, `0 !DATA` | TEXMAP and DATA Command | textures |
| `0 PAUSE`, `0 SAVE` | PAUSE Command, SAVE Command | not implemented |
