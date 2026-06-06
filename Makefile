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

# Exclude intermediate patched GLBs from the wildcard — those are build
# artifacts produced by the explicit patch rules below, not source assets.
ASSET_GLBS  = $(filter-out assets/%.patched.glb assets/cockpit_patched.glb, $(wildcard assets/*.glb))
ASSET_T3DMS = $(ASSET_GLBS:assets/%.glb=filesystem/%.t3dm)

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

AUDIO_ASSETS = filesystem/signal.wav64

$(BUILD_DIR)/$(ROM_NAME).dfs: $(ASSET_T3DMS) $(AUDIO_ASSETS)

filesystem/signal.wav64: assets/signal.wav
	cd $(CURDIR) && $(N64_INST)/bin/audioconv64 --wav-loop true $< && mv signal.wav64 filesystem/

ARES = /c/ares-v147/ares.exe

# Build and launch in Ares. cygpath converts the MSYS2 path to a Windows path
# that Ares (a native Windows app) can understand.
run: $(ROM_NAME).z64
	$(ARES) --system "Nintendo 64" "$(shell cygpath -w $(CURDIR)/$(ROM_NAME).z64)" &

clean:
	rm -rf $(BUILD_DIR) $(ROM_NAME).z64

.PHONY: all clean run

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

filesystem/cockpit.t3dm: assets/cockpit.glb
	$(PYTHON) patch_glb_material.py $< assets/cockpit_patched.glb
	$(T3D_GLTF_TO_3D) assets/cockpit_patched.glb $@ --base-scale=1 --ignore-materials

filesystem/ice_moon.t3dm: assets/ice_moon.glb
	$(PYTHON) patch_glb_material.py $< assets/ice_moon.patched.glb
	$(T3D_GLTF_TO_3D) assets/ice_moon.patched.glb $@ --base-scale=1 --ignore-materials

filesystem/young_star_field.t3dm: assets/young_star_field.glb
	$(PYTHON) patch_glb_material.py $< assets/young_star_field.patched.glb
	$(T3D_GLTF_TO_3D) assets/young_star_field.patched.glb $@ --base-scale=1 --ignore-materials

# Debris meshes (asteroid, shard, wreck) are small objects authored in Blender
# with export_materials='NONE' to preserve COLOR_0 vertex colors. Like the stage
# geometry they need patch_glb_material.py to inject a material index (or
# gltf_to_t3d silently emits an empty 65-byte file), but unlike the stage they
# are small, so they keep the default --base-scale=64 for sub-unit precision.
filesystem/asteroid.t3dm: assets/asteroid.glb
	$(PYTHON) patch_glb_material.py $< assets/asteroid.patched.glb
	$(T3D_GLTF_TO_3D) assets/asteroid.patched.glb $@ --base-scale=64 --ignore-materials

filesystem/shard.t3dm: assets/shard.glb
	$(PYTHON) patch_glb_material.py $< assets/shard.patched.glb
	$(T3D_GLTF_TO_3D) assets/shard.patched.glb $@ --base-scale=64 --ignore-materials

filesystem/wreck.t3dm: assets/wreck.glb
	$(PYTHON) patch_glb_material.py $< assets/wreck.patched.glb
	$(T3D_GLTF_TO_3D) assets/wreck.patched.glb $@ --base-scale=64 --ignore-materials

filesystem/%.t3dm: assets/%.glb
	$(T3D_GLTF_TO_3D) $< $@ --base-scale=64 --ignore-materials
