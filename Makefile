BUILD_DIR  = build
SOURCE_DIR = src
ROM_NAME   = game

N64_ROM_TITLE  = "N64 Learn"
N64_MKDFS_ROOT = filesystem

# The pyrite64-sdk lives at /pyrite64-sdk inside MSYS2 (= C:\msys64\pyrite64-sdk on Windows).
# n64.mk defines all compiler flags, tool paths, and the .c -> .o -> .elf -> .z64 pattern rules.
# t3d.mk prepends -lt3d to the linker flags and sets T3D_GLTF_TO_3D.
export N64_INST = /pyrite64-sdk
include $(N64_INST)/include/n64.mk
include $(N64_INST)/include/t3d.mk

ASSET_GLBS  = $(wildcard assets/*.glb)
ASSET_T3DMS = $(ASSET_GLBS:assets/%.glb=filesystem/%.t3dm)

# Stage geometry spans thousands of world units. Default --base-scale=64 would
# multiply coordinates by 64 before int16 conversion, causing overflow
# (5000 * 64 = 320,000 > 32767). These specific rules override the wildcard below.
#
# Blender's glTF exporter omits the "material" field from primitives when
# export_materials='NONE'. gltf_to_t3d skips any primitive with no material
# index, so we run patch_glb_material.py to inject a dummy material=0 on
# every primitive before conversion. The patch is done on a copy so the
# source .glb is unchanged.
PYTHON = '/c/Users/Scott Bieser/AppData/Local/Microsoft/WindowsApps/python.exe'

filesystem/ice_moon.t3dm: assets/ice_moon.glb
	$(PYTHON) patch_glb_material.py $< assets/ice_moon.patched.glb
	$(T3D_GLTF_TO_3D) assets/ice_moon.patched.glb $@ --base-scale=1 --ignore-materials

filesystem/young_star_field.t3dm: assets/young_star_field.glb
	$(PYTHON) patch_glb_material.py $< assets/young_star_field.patched.glb
	$(T3D_GLTF_TO_3D) assets/young_star_field.patched.glb $@ --base-scale=1 --ignore-materials

filesystem/%.t3dm: assets/%.glb
	$(T3D_GLTF_TO_3D) $< $@ --base-scale=64 --ignore-materials

SRCS = $(wildcard $(SOURCE_DIR)/*.c)
OBJS = $(SRCS:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(ROM_NAME).z64

# Add our object files to the linker step (n64.mk provides the recipe).
$(BUILD_DIR)/$(ROM_NAME).elf: $(OBJS)

# Embed the DFS archive in the ROM (filesystem/ dir, even when empty).
# Target-specific variable sets EEPROM 4K save type in the ROM header so
# Ares and flashcarts auto-configure save hardware without a DB lookup.
$(ROM_NAME).z64: N64_ED64ROMCONFIGFLAGS = -w eeprom4k
$(ROM_NAME).z64: $(BUILD_DIR)/$(ROM_NAME).dfs

$(BUILD_DIR)/$(ROM_NAME).dfs: $(ASSET_T3DMS)

ARES = /c/ares-v147/ares.exe

# Build and launch in Ares. cygpath converts the MSYS2 path to a Windows path
# that Ares (a native Windows app) can understand.
run: $(ROM_NAME).z64
	$(ARES) --system "Nintendo 64" "$(shell cygpath -w $(CURDIR)/$(ROM_NAME).z64)" &

clean:
	rm -rf $(BUILD_DIR) $(ROM_NAME).z64

.PHONY: all clean run
