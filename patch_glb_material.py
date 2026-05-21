"""
Inject a dummy material entry into a GLB so gltf_to_t3d will process every
primitive. gltf_to_t3d skips primitives with no material index; --ignore-materials
tells it to discard the material content but still process the geometry.

Usage: python patch_glb_material.py input.glb output.glb
"""
import struct, json, sys

MAGIC       = 0x46546C67
CHUNK_JSON  = 0x4E4F534A
CHUNK_BIN   = 0x004E4942

def patch(in_path, out_path):
    with open(in_path, 'rb') as f:
        raw = f.read()

    # Parse header
    magic, version, total_len = struct.unpack_from('<III', raw, 0)
    assert magic == MAGIC, "Not a GLB file"

    # Parse JSON chunk
    json_len, json_type = struct.unpack_from('<II', raw, 12)
    assert json_type == CHUNK_JSON
    json_bytes = raw[20:20 + json_len]
    gltf = json.loads(json_bytes)

    # Rest of file after JSON chunk (binary chunk, if any)
    rest = raw[20 + json_len:]

    # Add one dummy material if none exist
    if 'materials' not in gltf:
        gltf['materials'] = []
    if len(gltf['materials']) == 0:
        gltf['materials'].append({
            "name": "Dummy",
            "pbrMetallicRoughness": {"baseColorFactor": [1, 1, 1, 1]}
        })

    # Point every primitive at material index 0
    patched = 0
    for mesh in gltf.get('meshes', []):
        for prim in mesh.get('primitives', []):
            if prim.get('material') is None:
                prim['material'] = 0
                patched += 1

    print(f"Patched {patched} primitives -> material 0")

    # Re-encode JSON, pad to 4-byte alignment with spaces
    new_json_bytes = json.dumps(gltf, separators=(',', ':')).encode('utf-8')
    pad = (4 - len(new_json_bytes) % 4) % 4
    new_json_bytes += b' ' * pad

    # Rebuild file
    new_json_len = len(new_json_bytes)
    new_total    = 12 + 8 + new_json_len + len(rest)

    with open(out_path, 'wb') as f:
        f.write(struct.pack('<III', MAGIC, version, new_total))
        f.write(struct.pack('<II',  new_json_len, CHUNK_JSON))
        f.write(new_json_bytes)
        f.write(rest)

    print(f"Written: {out_path} ({new_total} bytes)")

if __name__ == '__main__':
    patch(sys.argv[1], sys.argv[2])
