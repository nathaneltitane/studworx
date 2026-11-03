# -*- coding: utf-8 -*-
"""
Trevor SANDY
Last Update September 01, 2025
Copyright (c) 2024 by Toby Nelson
Copyright (c) 2020 - 2025 by Trevor SANDY

LPub3D Import LDraw GPLv2 license.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.


LPub3D Import LDraw

This file defines the importer for Blender.
It stores and recalls preferences for the importer.
The execute() function kicks off the import process.
The python module load_ldraw does the actual work.

Adapted from Import LDraw by Toby Nelson - tobymnelson@gmail.com
"""

# Import From Files - See PR https://github.com/TobyLobster/ImportLDraw/pull/49/
if "bpy" in locals():
    import importlib
    importlib.reload(load_ldraw)
else:
    from . import load_ldraw

import os
import bpy
from bpy.props import (StringProperty,
                       FloatProperty,
                       EnumProperty,
                       BoolProperty
                       )
from bpy_extras.io_utils import ImportHelper

from io_scene_render_ldraw.preferences import Preferences

"""
Default preferences file:

[DEFAULT]

[import_ldraw]
blendFile                     =
customLDConfigFile            =
additionalSearchPaths   =
ldrawDirectory                =
lsynthDirectory               =
studLogoDirectory             =
addEnvironment                = True
bevelEdges                    = True
bevelWidth                    = 0.5
blendfileTrusted              = False
cameraBorderPercentage        = 0.5
curvedWalls                   = True
defaultColour                 = 4
flattenHierarchy              = False
gaps                          = False
realGapWidth                  = 0.0002
importCameras                 = True
instanceStuds                 = False
linkParts                     = True
logoStudVersion               = 4
numberNodes                   = True
overwriteExistingMaterials    = False
overwriteExistingMeshes       = False
overwriteImage                = True
positionCamera                = True
positionObjectOnGroundAtOrigin= True
removeDoubles                 = False
renderWindow                  = False
resolution                    = Standard
resolveNormals                = guess
realScale                     = 1.0    # revert realScale: (rollback 0.02)
scaleStrategy                 = Mesh
smoothShading                 = True
transparentBackground         = True
useColourScheme               = lego
useLogoStuds                  = False
useLook                       = normal
useLSynthParts                = True
useUnofficialParts            = True
useArchiveLibrary             = False
verbose                       = 0
"""

class ImportLDrawOps(bpy.types.Operator, ImportHelper):
    """Import LDraw - Import Operator."""

    bl_idname = "import_scene.lpub3d_import_ldraw"
    bl_description = "Import LDraw model (.io/.mpd/.ldr/.l3b/.dat)"
    bl_label = "Import LDraw Model"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_options = {'REGISTER', 'UNDO', 'PRESET'}

    # Declarations
    ldraw_model_file_loaded = False

    # Initialize the preferences system
    preferencesFile = os.path.join(os.path.dirname(__file__), "configuration", "import_preferences.ini")
    prefs = Preferences(preferencesFile)

    # Properties - specified from preferences function
    ldrawPath: StringProperty(
        name="",
        description="Full filepath to the LDraw Parts Library (download from http://www.ldraw.org)",
        default=prefs.get("ldrawDirectory", load_ldraw.Configure.findDefaultLDrawDirectory())
    )

    realScale: FloatProperty(
        name="Scale",
        description="Sets a scale for the model (range: .01 and 1.0, 0.04 is LeoCAD scale, 1.0 = real LEOG scale)",
        default=prefs.get("realScale", 1.0)     # revert realScale: (rollback: 0.01)
    )

    resPrims: EnumProperty(
        name="Resolution of part primitives",
        description="Resolution of part primitives, ie. how much geometry they have",
        default=prefs.get("resolution", "Standard"),
        items=(
            ("Standard", "Standard primitives", "Import using standard resolution primitives."),
            ("High", "High resolution primitives", "Import using high resolution primitives."),
            ("Low", "Low resolution primitives", "Import using low resolution primitives.")
        )
    )

    look: EnumProperty(
        name="Overall Look",
        description="Realism or Schematic look",
        default=prefs.get("useLook", "normal"),
        items=(
            ("normal", "Realistic Look", "Render to look realistic."),
            ("instructions", "Lego Instructions Look", "Render to look like the instruction book pictures."),
        )
    )

    useColourScheme: EnumProperty(
        name="Colour scheme options",
        description="Colour scheme options",
        default=prefs.get("useColourScheme", "lgeo"),
        items=[
            ("lgeo", "Realistic colours", "Uses the LGEO colour scheme for realistic colours."),
            ("ldraw", "Original LDraw colours", "Uses the standard LDraw colour scheme (LDConfig.ldr). Good with the Instructions Look."),
            ("alt", "Alternate LDraw colours", "Uses the alternate LDraw colour scheme (LDCfgalt.ldr). Good with the Instructions Look."),
            ("custom", "Custom LDraw colours", "Uses a user specified LDraw colour file.")
        ],
    )

    logoStudVersion: EnumProperty(
        name="Stud Logo Version",
        description="Which version of the stud logo to use",
        default=prefs.get("logoStudVersion", "4"),
        items=(
            ("4", "Rounded (4)", "Rounded logo lettering geometry"),
            ("3", "Flat (3)", "Flat logo lettering geometry"),
            ("5", "Subtle Rounded (5)", "Subtle rounded logo lettering geometry"),
        )
    )

    smoothParts: BoolProperty(
        name="Smooth faces and edge-split",
        description="Smooth faces and add an edge-split modifier",
        default=prefs.get("smoothShading", True)
    )

    addGaps: BoolProperty(
        name="Add space between each part:",
        description="Add a small space between each part",
        default=prefs.get("gaps", False)
    )

    gapWidthMM: FloatProperty(
        name="Space",
        description="Amount of space between each part (default 0.2mm)",
        default=1000 * prefs.get("realGapWidth", 0.0002)
    )

    curvedWalls: BoolProperty(
        name="Use curved wall normals",
        description="Makes surfaces look slightly concave, for interesting reflections",
        default=prefs.get("curvedWalls", True)
    )

    addSubsurface: BoolProperty(
        name="Add subsurface",
        description="Adds subsurface to principled shader",
        default=prefs.get("addSubsurface", True)
    )

    importCameras: BoolProperty(
        name="Import cameras",
        description="Import camera definitions (from models authored in LPub3D or LeoCAD)",
        default=prefs.get("importCameras", True)
    )

    importLights: BoolProperty(
        name="Import lights",
        description="Import Light definitions (from models authored in LPub3D or LeoCAD)",
        default=prefs.get("importLights", True)
    )

    linkParts: BoolProperty(
        name="Link identical parts",
        description="Identical parts (of the same type and colour) share the same mesh",
        default=prefs.get("linkParts", True)
    )

    numberNodes: BoolProperty(
        name="Number each object",
        description="Each object has a five digit prefix eg. 00001_car. This keeps the list in it's proper order",
        default=prefs.get("numberNodes", True)
    )

    positionOnGround: BoolProperty(
        name="Put model on ground at origin",
        description="The object is centred at the origin, and on the ground plane",
        default=prefs.get("positionObjectOnGroundAtOrigin", True)
    )

    flatten: BoolProperty(
        name="Flatten tree",
        description="In Scene Outliner, all parts are placed directly below the root - there's no tree of submodels",
        default=prefs.get("flattenHierarchy", False)
    )

    minifigHierarchy: BoolProperty(
        name="Parent Minifigs",
        description="Add a parent/child hierarchy (tree) for Minifigs",
        default=prefs.get("minifigHierarchy", True)
    )

    useUnofficialParts: BoolProperty(
        name="Include unofficial parts",
        description="Additionally searches for parts in the <ldraw-dir>/unofficial/ directory",
        default=prefs.get("useUnofficialParts", True)
    )

    useLogoStuds: BoolProperty(
        name="Show 'LEGO' logo on studs",
        description="Shows the LEGO logo on each stud (at the expense of some extra geometry and import time)",
        default=prefs.get("useLogoStuds", True)
    )

    instanceStuds: BoolProperty(
        name="Make individual studs",
        description="Creates a Blender Object for each and every stud (WARNING: can be slow to import and edit in "
                    "Blender if there are lots of studs)",
        default=prefs.get("instanceStuds", False)
    )

    resolveNormals: EnumProperty(
        name="Resolve ambiguous normals option",
        description="Some older LDraw parts have faces with ambiguous normals, this specifies what do do with them",
        default=prefs.get("resolveNormals", "guess"),
        items=(
            ("guess", "Recalculate Normals", "Uses Blender's Recalculate Normals to get a consistent set of normals."),
            ("double", "Two faces back to back",
             "Two faces are added with their normals pointing in opposite directions."),
        )
    )

    bevelEdges: BoolProperty(
        name="Bevel edges",
        description="Adds a Bevel modifier for rounding off sharp edges",
        default=prefs.get("bevelEdges", True)
    )

    bevelWidth: FloatProperty(
        name="Bevel Width",
        description="Width of the bevelled edges",
        default=prefs.get("bevelWidth", 0.5)
    )

    addEnvironment: BoolProperty(
        name="Add Environment",
        description="Adds a ground plane and environment texture (for realistic look only)",
        default=prefs.get("addEnvironment", True)
    )

    positionCamera: BoolProperty(
        name="Position the camera",
        description="Position the camera to show the whole model",
        default=prefs.get("positionCamera", True)
    )

    cameraBorderPercentage: FloatProperty(
        name="Camera Border %",
        description="When positioning the camera, include a (percentage) border leeway around the model in the rendered image",
        default=prefs.get("cameraBorderPercentage", 5.0)
    )

    customLDConfigFile: StringProperty(
        name="",
        description="Full directory path to specified custom LDraw colours (LDConfig) file",
        default=prefs.get("customLDConfigFile", r"")
    )

    additionalSearchPaths: StringProperty(
        name="",
        description="Full directory paths, comma delimited, to additional LDraw search paths",
        default=prefs.get("additionalSearchPaths", "")
    )

    studLogoPath: StringProperty(
        name="",
        description="Full directory path to LDraw stud logo primitives - specify if unofficial parts not used",
        default=prefs.get("studLogoDirectory", r"")
    )

    lsynthPath: StringProperty(
        name="",
        description="Full directory path to LSynth primitives - specify if not using blender module default",
        default=prefs.get("lsynthDirectory", r"")
    )

    environmentFile: StringProperty(
        name="",
        description="Full file path to .exr environment texture file - specify if not using addon default",
        default=prefs.get("environmentFile", r"")
    )

    defaultColour: StringProperty(
        name="Default Colour",
        description="Sets the default part colour",
        default=prefs.get("defaultColour", "4")
    )

    useLSynthParts: BoolProperty(
        name="Use LSynth for Flex Parts",
        description="Source used to create flexible parts",
        default=prefs.get("useLSynthParts", True)
    )

    overwriteExistingMaterials: BoolProperty(
        name="Use Existing Material",
        description="Overwrite existing material with the same name",
        default=prefs.get("overwriteExistingMaterials", True)
    )

    overwriteExistingMeshes: BoolProperty(
        name="Use Existing Mesh",
        description="Overwrite existing mesh with the same name",
        default=prefs.get("overwriteExistingMeshes", False)
    )

    removeDoubles: BoolProperty(
        name="No Duplicate Verticies",
        description="Remove duplicate vertices (recommended)",
        default=prefs.get("removeDoubles", True)
    )

    useArchiveLibrary: BoolProperty(
        name="Use Archive Libraries",
        description="Add any archive (zip) libraries in the LDraw file path to the library search list",
        default=False
    )

    searchAdditionalPaths: BoolProperty(
        name="Search Additional Paths",
        description="Search additional LDraw paths (automatically set for fade previous steps and highlight step)",
        default=False
    )

    verbose: BoolProperty(
        name="Verbose Output",
        description="Output all messages while working, else only show warnings and errors",
        default=prefs.get("verbose", True)
    )

    modelFile: StringProperty(
        default=r"",
        options={'HIDDEN'}
    )

    renderLDraw:  BoolProperty(
        default=False,
        options={"HIDDEN"}
    )

    # File type filter in file browser
    filename_ext = ".ldr"
    filter_glob: StringProperty(
        default="*.io;*.mpd;*.ldr;*.l3b;*.dat",
        options={'HIDDEN'}
    )
    # End Hidden properties

    def draw(self, context):
        """Display import options."""

        space_factor = 0.3
        layout = self.layout
        layout.use_property_split = True  # Active single-column layout

        box = layout.box()
        box.label(text="LDraw Import Options", icon='PREFERENCES')
        box.label(text="LDraw Parts Folder:", icon='FILEBROWSER')
        box.prop(self, "ldrawPath")
        box.label(text="Custom LDConfig:", icon='FILEBROWSER')
        box.prop(self, "customLDConfigFile")
        if not self.ldraw_model_file_loaded:
            box.label(text="Environment File:", icon='FILEBROWSER')
            box.prop(self, "environmentFile")
        box.prop(self, "searchAdditionalPaths")

        layout.separator(factor=space_factor)
        box.label(text="Import Options")
        box.prop(self, "addEnvironment")
        box.prop(self, "importCameras")
        box.prop(self, "importLights")
        box.prop(self, "realScale")
        box.prop(self, "look", expand=True)
        box.prop(self, "useColourScheme", expand=True)
        box.prop(self, "defaultColour")
        box.prop(self, "positionCamera")
        box.prop(self, "cameraBorderPercentage")
        box.prop(self, "positionOnGround")
        box.prop(self, "useLogoStuds")
        box.prop(self, "logoStudVersion", expand=True)
        box.prop(self, "numberNodes")
        box.prop(self, "minifigHierarchy")

        layout.separator(factor=space_factor)
        box.label(text="Cleanup Options")
        box.prop(self, "flatten")
        box.prop(self, "instanceStuds")
        box.prop(self, "resPrims", expand=True)
        box.prop(self, "smoothParts")
        box.prop(self, "addSubsurface")
        box.prop(self, "bevelEdges")
        box.prop(self, "bevelWidth")
        box.prop(self, "addGaps")
        box.prop(self, "gapWidthMM")
        box.prop(self, "curvedWalls")
        box.prop(self, "linkParts")

        layout.separator(factor=space_factor)
        box.label(text="Resolve Ambiguous Normals:", icon='ORIENTATION_NORMAL')
        box.prop(self, "resolveNormals", expand=True)

        layout.separator(factor=space_factor)
        box.label(text="Extras")
        box.prop(self, "useUnofficialParts")
        box.prop(self, "useArchiveLibrary")
        box.prop(self, "verbose")

    def execute(self, context):
        """Start the import process."""

        # Confirm minimum Blender version
        if bpy.app.version < (2, 82, 0):
            self.report({'ERROR'}, 'The ImportLDraw addon requires Blender 2.82 or greater.')
            return {'FINISHED'}

        # Initialize model globals
        from io_scene_render_ldraw import model_globals
        model_globals.init()

        ImportLDrawOps.prefs = Preferences(self.preferencesFile)
        load_ldraw.Options.verbose = self.verbose
        if self.renderLDraw:
            load_ldraw.debugPrint("-----Import Settings-----")
            # Update properties with the reinitialized preferences
            self.addEnvironment          = ImportLDrawOps.prefs.get("addEnvironment",       self.addEnvironment)
            self.addGaps                 = ImportLDrawOps.prefs.get("gaps",                 self.addGaps)
            self.additionalSearchPaths   = ImportLDrawOps.prefs.get("additionalSearchPaths", self.additionalSearchPaths)
            self.addSubsurface           = ImportLDrawOps.prefs.get("addSubsurface",        self.addSubsurface)
            self.bevelEdges              = ImportLDrawOps.prefs.get("bevelEdges",           self.bevelEdges)
            self.bevelWidth              = ImportLDrawOps.prefs.get("bevelWidth",           self.bevelWidth)
            self.cameraBorderPercentage  = ImportLDrawOps.prefs.get("cameraBorderPercentage", self.cameraBorderPercentage)
            self.useColourScheme         = ImportLDrawOps.prefs.get("useColourScheme",      self.useColourScheme)
            self.curvedWalls             = ImportLDrawOps.prefs.get("curvedWalls",          self.curvedWalls)
            self.customLDConfigFile      = ImportLDrawOps.prefs.get("customLDConfigFile",   self.customLDConfigFile)
            self.defaultColour           = ImportLDrawOps.prefs.get("defaultColour",        self.defaultColour)
            self.environmentFile         = ImportLDrawOps.prefs.get("environmentFile",      self.environmentFile)
            self.flatten                 = ImportLDrawOps.prefs.get("flattenHierarchy",     self.flatten)
            self.minifigHierarchy        = ImportLDrawOps.prefs.get("minifigHierarchy",     self.minifigHierarchy)
            self.gapWidthMM              = ImportLDrawOps.prefs.get("realGapWidth",         self.gapWidthMM / 1000)
            self.importCameras           = ImportLDrawOps.prefs.get("importCameras",        self.importCameras)
            self.importLights            = ImportLDrawOps.prefs.get("importLights",         self.importLights)
            self.realScale               = ImportLDrawOps.prefs.get("realScale",            self.realScale)
            self.instanceStuds           = ImportLDrawOps.prefs.get("instanceStuds",        self.instanceStuds)
            self.ldrawPath               = ImportLDrawOps.prefs.get("ldrawDirectory",       self.ldrawPath)
            self.linkParts               = ImportLDrawOps.prefs.get("linkParts",            self.linkParts)
            self.logoStudVersion         = ImportLDrawOps.prefs.get("logoStudVersion",      self.logoStudVersion)
            self.look                    = ImportLDrawOps.prefs.get("useLook",              self.look)
            self.lsynthPath              = ImportLDrawOps.prefs.get("lsynthDirectory",      self.lsynthPath)
            self.numberNodes             = ImportLDrawOps.prefs.get("numberNodes",          self.numberNodes)
            self.overwriteExistingMaterials = ImportLDrawOps.prefs.get("overwriteExistingMaterials", self.overwriteExistingMaterials)
            self.overwriteExistingMeshes = ImportLDrawOps.prefs.get("overwriteExistingMeshes", self.overwriteExistingMeshes)
            self.positionCamera          = ImportLDrawOps.prefs.get("positionCamera",       self.positionCamera)
            self.positionOnGround        = ImportLDrawOps.prefs.get("positionObjectOnGroundAtOrigin", self.positionOnGround)
            self.removeDoubles           = ImportLDrawOps.prefs.get("removeDoubles",        self.removeDoubles)
            self.resolveNormals          = ImportLDrawOps.prefs.get("resolveNormals",       self.resolveNormals)
            self.resPrims                = ImportLDrawOps.prefs.get("resolution",           self.resPrims)
            self.useArchiveLibrary       = ImportLDrawOps.prefs.get("useArchiveLibrary",    self.useArchiveLibrary)
            self.searchAdditionalPaths   = ImportLDrawOps.prefs.get("searchAdditionalPaths", self.searchAdditionalPaths)
            self.smoothParts             = ImportLDrawOps.prefs.get("smoothShading",        self.smoothParts)
            self.studLogoPath            = ImportLDrawOps.prefs.get("studLogoDirectory",    self.studLogoPath)
            self.useLogoStuds            = ImportLDrawOps.prefs.get("useLogoStuds",         self.useLogoStuds)
            self.useLSynthParts          = ImportLDrawOps.prefs.get("useLSynthParts",       self.useLSynthParts)
            self.useUnofficialParts      = ImportLDrawOps.prefs.get("useUnofficialParts",   self.useUnofficialParts)
            self.verbose                 = ImportLDrawOps.prefs.get("verbose",              self.verbose)
        else:
            load_ldraw.debugPrint("------Import LDraw-------")
            # Read current preferences from the UI and save them
            ImportLDrawOps.prefs.set("customLDConfigFile",     self.customLDConfigFile)
            ImportLDrawOps.prefs.set("environmentFile",        self.environmentFile)
            ImportLDrawOps.prefs.set("addEnvironment",         self.addEnvironment)
            ImportLDrawOps.prefs.set("addSubsurface",          self.addSubsurface)
            ImportLDrawOps.prefs.set("bevelEdges",             self.bevelEdges)
            ImportLDrawOps.prefs.set("bevelWidth",             self.bevelWidth)
            ImportLDrawOps.prefs.set("cameraBorderPercentage", self.cameraBorderPercentage)
            ImportLDrawOps.prefs.set("curvedWalls",            self.curvedWalls)
            ImportLDrawOps.prefs.set("defaultColour",          self.defaultColour)
            ImportLDrawOps.prefs.set("flattenHierarchy",       self.flatten)
            ImportLDrawOps.prefs.set("minifigHierarchy",       self.minifigHierarchy)
            ImportLDrawOps.prefs.set("gaps",                   self.addGaps)
            ImportLDrawOps.prefs.set("realGapWidth",           self.gapWidthMM / 1000)
            ImportLDrawOps.prefs.set("importCameras",          self.importCameras)
            ImportLDrawOps.prefs.set("importLights",           self.importLights)
            ImportLDrawOps.prefs.set("instanceStuds",          self.instanceStuds)
            ImportLDrawOps.prefs.set("ldrawDirectory",         self.ldrawPath)
            ImportLDrawOps.prefs.set("linkParts",              self.linkParts)
            ImportLDrawOps.prefs.set("logoStudVersion",        self.logoStudVersion)
            ImportLDrawOps.prefs.set("lsynthDirectory",        self.lsynthPath)
            ImportLDrawOps.prefs.set("numberNodes",            self.numberNodes)
            ImportLDrawOps.prefs.set("positionCamera",         self.positionCamera)
            ImportLDrawOps.prefs.set("positionObjectOnGroundAtOrigin", self.positionOnGround)
            ImportLDrawOps.prefs.set("resolution",             self.resPrims)
            ImportLDrawOps.prefs.set("resolveNormals",         self.resolveNormals)
            ImportLDrawOps.prefs.set("realScale",              self.realScale)
            ImportLDrawOps.prefs.set("useArchiveLibrary",      self.useArchiveLibrary)
            ImportLDrawOps.prefs.set("searchAdditionalPaths",  self.searchAdditionalPaths)
            ImportLDrawOps.prefs.set("smoothShading",          self.smoothParts)
            ImportLDrawOps.prefs.set("studLogoDirectory",      self.studLogoPath)
            ImportLDrawOps.prefs.set("useColourScheme",        self.useColourScheme)
            ImportLDrawOps.prefs.set("useLogoStuds",           self.useLogoStuds)
            ImportLDrawOps.prefs.set("useLook",                self.look)
            ImportLDrawOps.prefs.set("useUnofficialParts",     self.useUnofficialParts)
            ImportLDrawOps.prefs.set("verbose",                self.verbose)

        ImportLDrawOps.prefs.save()

        if self.useColourScheme == "custom":
            assert self.customLDConfigFile != "", "Custom LDraw colour (LDConfig) file path not specified."

        # Set bpy related variables here since it isn't available immediately on Blender startup
        load_ldraw.hasCollections = hasattr(bpy.data, "collections")

        # Set import options and import
        load_ldraw.Options.addBevelModifier            = self.bevelEdges and not load_ldraw.Options.instructionsLook
        load_ldraw.Options.addGroundPlane              = self.addEnvironment
        load_ldraw.Options.addSubsurface               = self.addSubsurface
        load_ldraw.Options.addWorldEnvironmentTexture  = self.addEnvironment
        load_ldraw.Options.bevelWidth                  = self.bevelWidth
        load_ldraw.Options.cameraBorderPercent         = self.cameraBorderPercentage / 100.0
        load_ldraw.Options.createInstances             = self.linkParts
        load_ldraw.Options.curvedWalls                 = self.curvedWalls
        load_ldraw.Options.defaultColour               = self.defaultColour
        load_ldraw.Options.edgeSplit                   = self.smoothParts  # Edge split is appropriate only if we are smoothing
        load_ldraw.Options.flattenHierarchy            = self.flatten
        load_ldraw.Options.minifigHierarchy            = self.minifigHierarchy
        load_ldraw.Options.gaps                        = self.addGaps
        load_ldraw.Options.realGapWidth                = self.gapWidthMM / 1000
        load_ldraw.Options.importCameras               = self.importCameras
        load_ldraw.Options.importLights                = self.importLights
        load_ldraw.Options.instanceStuds               = self.instanceStuds
        load_ldraw.Options.instructionsLook            = self.look == "instructions"
        load_ldraw.Options.logoStudVersion             = self.logoStudVersion
        load_ldraw.Options.numberNodes                 = self.numberNodes
        load_ldraw.Options.overwriteExistingMaterials  = self.overwriteExistingMaterials
        load_ldraw.Options.overwriteExistingMeshes     = self.overwriteExistingMeshes
        load_ldraw.Options.positionCamera              = self.positionCamera
        load_ldraw.Options.positionObjectOnGroundAtOrigin = self.positionOnGround
        load_ldraw.Options.removeDoubles               = self.removeDoubles
        load_ldraw.Options.resolution                  = self.resPrims
        load_ldraw.Options.resolveAmbiguousNormals     = self.resolveNormals
        load_ldraw.Options.realScale                   = self.realScale
        load_ldraw.Options.useArchiveLibrary           = self.useArchiveLibrary
        load_ldraw.Options.searchAdditionalPaths       = self.searchAdditionalPaths
        load_ldraw.Options.smoothShading               = self.smoothParts
        load_ldraw.Options.useColourScheme             = self.useColourScheme
        load_ldraw.Options.useLogoStuds                = self.useLogoStuds
        load_ldraw.Options.useLSynthParts              = self.useLSynthParts
        load_ldraw.Options.useUnofficialParts          = self.useUnofficialParts
        load_ldraw.Options.verbose                     = self.verbose

        load_ldraw.Options.additionalSearchPaths       = self.additionalSearchPaths
        load_ldraw.Options.customLDConfigFile          = self.customLDConfigFile

        #assert self.ldrawPath, "LDraw library path not specified."
        load_ldraw.Options.ldrawDirectory              = self.ldrawPath

        if self.environmentFile == "":
            load_ldraw.Options.environmentFile         = ImportLDrawOps.prefs.getEnvironmentFile()
        else:
            load_ldraw.Options.environmentFile         = self.environmentFile
        if self.lsynthPath == "":
            load_ldraw.Options.LSynthDirectory         = ImportLDrawOps.prefs.getLSynthPath()
        else:
            load_ldraw.Options.LSynthDirectory         = self.lsynthPath
        if self.studLogoPath == "":
            load_ldraw.Options.studLogoDirectory       = ImportLDrawOps.prefs.getLStudsPath()
        else:
            load_ldraw.Options.studLogoDirectory       = self.studLogoPath

        if self.filepath != "":
            self.modelFile                            = self.filepath

        model_globals.LDRAW_MODEL_FILE = self.modelFile

        load_result = load_ldraw.loadFromFile(self, self.modelFile)

        model_globals.LDRAW_MODEL_LOADED = True

        load_ldraw.debugPrint("-----Import Complete-----")
        load_ldraw.debugPrint(f"Model file: {model_globals.LDRAW_MODEL_FILE}")
        load_ldraw.debugPrint(f"Part count: {load_ldraw.globalBrickCount}")
        load_ldraw.debugPrint(f"Elapsed time: {load_ldraw.formatElapsed(load_ldraw.ldrawLoadElapsed)}")
        load_ldraw.debugPrint("-------------------------")
        load_ldraw.debugPrint("")

        return {"FINISHED"}
