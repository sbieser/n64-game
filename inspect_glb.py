import struct, json, sys

path = sys.argv[1]
with open(path, 'rb') as f:
    magic, ver, length = struct.unpack('<III', f.read(12))
    chunk_len, chunk_type = struct.unpack('<II', f.read(8))
    json_data = json.loads(f.read(chunk_len))

meshes = json_data.get('meshes', [])
nodes  = json_data.get('nodes', [])
print(f'Meshes: {len(meshes)}, Nodes: {len(nodes)}')
for i, m in enumerate(meshes[:5]):
    print(f'  mesh[{i}] name={m.get("name","?")}')
    for p in m.get('primitives', []):
        attrs = list(p.get('attributes', {}).keys())
        mat   = p.get('material', None)
        print(f'    primitive attrs={attrs} material={mat}')
