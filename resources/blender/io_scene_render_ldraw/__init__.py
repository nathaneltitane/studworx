# -*- coding: utf-8 -*-
"""
Trevor SANDY
Last Update May 23, 2023
Copyright (c) 2020 - 2025 by Trevor SANDY

LPub3D Render LDraw GPLv2 license.

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

"""

#############################################
# support reloading sub-modules
if "bpy" in locals():
    from importlib import reload
    reload(render_ldraw)
    reload(model_globals)
    del reload
else:
    from . import render_ldraw
    from . import model_globals
# support reloading sub-modules
#############################################

import bpy

bl_info = {
    "name": "StudWorx Render LDraw",
    "description": "Render LDraw model as .png file",
    "author": "Trevor SANDY <trevor.sandy@gmail.com>",
    "version": (1, 6, 8),
    "blender": (2, 82, 0),
    "location": "Render",
    "warning": "",
    "wiki_url": "https://github.com/trevorsandy/blenderldrawrender",
    "tracker_url": "https://github.com/trevorsandy/blenderldrawrender/issues",
    "category": "Render"
}


# Addon menu item
def menuRender(self, context):
    """Render menu listing label."""
    self.layout.operator(render_ldraw.RenderLDrawOps.bl_idname,
                         text="StudWorx Render LDraw (.png)",
                         icon='RENDER_RESULT')


# When enabling the addon.
def register():
    """Register Render LDraw."""
    bpy.utils.register_class(render_ldraw.RenderLDrawOps)
    bpy.types.TOPBAR_MT_render.prepend(menuRender)


# When disabling the addon.
def unregister():
    """Unregister Render LDraw."""
    bpy.utils.unregister_class(render_ldraw.RenderLDrawOps)
    bpy.types.TOPBAR_MT_render.remove(menuRender)


if __name__ == "__main__":
    register()

    # bpy.ops.render_scene.lpub3d_render_ldraw('INVOKE_DEFAULT') # Test call
