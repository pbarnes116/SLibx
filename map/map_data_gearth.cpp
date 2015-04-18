#include "../../../inc/slibx/map/data_gearth.h"

#include "../../../inc/slib/core/io.h"

SLIB_MAP_NAMESPACE_START

class _GEarthTileFile
{
public:
	class GVertex
	{
	public:
		sl_int32 raw[3];
		sl_uint16 txi;
		sl_uint16 tyi;
		sl_int32 indexLayerOctant;

		GVertex()
		{
			txi = 0;
			tyi = 0;
			indexLayerOctant = 0;
		}
	};

	class GTexture
	{
	public:
		sl_uint32 format; // 1: normal,  6: crn
		sl_uint32 width;
		sl_uint32 height;
		Memory data;
	};

	class GMesh : public Referable
	{
	public:
		sl_uint32 id;
		Array<GVertex> vertices;
		Array<sl_uint8> alphas;
		Array<sl_uint16> indices;
		Array<sl_uint16> triangleIndices;
		sl_uint32 index_c;
		double uvOffsetX;
		double uvOffsetY;
		double uvScaleX;
		double uvScaleY;
		GTexture texture;
		sl_int32 layerBounds[10];
		Array<float> uvOffsetAndScale;
		Memory normalBuffer;
		Array<sl_uint32> indexLayerOctants;

		GMesh()
		{
			uvOffsetX = 0;
			uvOffsetY = 0;
			uvScaleX = 1;
			uvScaleY = 1;
		}
	};

	sl_bool m_flagOpen;

	double m_globeMatrix[16];
	List< Ref<GMesh> > m_listMesh;
	Ref<GMesh> m_waterMesh;
	List< Ref<GMesh> > m_listSurfaceMesh;

	sl_bool m_flagUseNormal;
	sl_uint32 m_nonEmptyOctants;
	List<sl_uint32> m_copyrightIds;


	_GEarthTileFile()
	{
		m_flagOpen = sl_false;

		m_flagUseNormal = sl_true;
		m_nonEmptyOctants = 0;

	}

	void close()
	{
		if (!m_flagOpen) {
			return;
		}
		m_listMesh.clear();
		m_listSurfaceMesh.clear();
		m_waterMesh.setNull();
		m_copyrightIds.clear();
		m_flagOpen = sl_false;
	}
	
	static sl_bool readType(MemoryReader& reader, sl_uint32& type, sl_uint32& opt)
	{
		sl_uint32 n = 0;
		if (reader.readUint32CVLI(&n)) {
			type = n >> 3;
			opt = n & 7;
			return sl_true;
		} else {
			type = 0;
			opt = 0;
			return sl_true;
		}
	}

	static sl_bool processTail(MemoryReader& input, sl_uint32 type)
	{
		switch (type)
		{
		case 0:
			{
				sl_uint8 c = 0;
				while (1) {
					if (!input.readUint8(&c)) {
						return sl_false;
					}
					if ((c & 128) == 0) {
						break;
					}
				}
			}
			return sl_true;
		case 1:
			if (!input.seek(8)) {
				return sl_false;
			}
			return sl_true;
		case 2:
			{
				sl_uint32 n = 0;
				if (!input.readUint32CVLI(&n)) {
					return sl_false;
				}
				if (!input.seek(n)) {
					return sl_false;
				}
			}
			return sl_true;
		case 5:
			if (!input.seek(4)) {
				return sl_false;
			}
			return sl_true;
		}
		return sl_false;
	}

	sl_bool loadTexture(GTexture& texture, Memory mem)
	{
		MemoryReader input(mem);
		texture.format = 1;
		texture.width = 256;
		texture.height = 256;
		while (1) {
			sl_uint32 type = 0, typeTail = 0;
			if (!readType(input, type, typeTail)) {
				return sl_false;
			}
			if (type == 0) {
				break;
			}
			switch (type) {
			case 1: // texture data
				texture.data = input.readSection();
				break;
			case 2:
				if (!input.readUint32CVLI(&(texture.format))) {
					return sl_false;
				}
				break;
			case 3:
				if (!input.readUint32CVLI(&(texture.width))) {
					return sl_false;
				}
				break;
			case 4:
				if (!input.readUint32CVLI(&(texture.height))) {
					return sl_false;
				}
				break;
			default:
				if (!processTail(input, typeTail)) {
					return sl_false;
				}
				break;
			}
		}
		if (texture.data.isEmpty()) {
			return sl_false;
		}
		return sl_true;
	}

	Ref<GMesh> loadMesh(Memory mem)
	{
		Ref<GMesh> _mesh = new GMesh();
		if (_mesh.isNull()) {
			return _mesh;
		}
		MemoryReader input(mem);

		sl_uint32 stack_bound[100];
		sl_uint32 stack_bound_index = 0;
	
		GMesh* mesh = _mesh;
		while (1) {
			sl_uint32 type = 0, typeTail = 0;
			if (!readType(input, type, typeTail)) {
				return sl_null;
			}
			if (type == 0) {
				break;
			}
			switch (type) {
			case 1: // vertices
				{
					sl_uint32 n = 0;
					if (!input.readUint32CVLI(&n)) {
						return sl_null;
					}
					if (n % 3 != 0) {
						return sl_null;
					}
					if (n == 0) {
						return sl_null;
					}
					n = n / 3;
					mesh->vertices = Array<GVertex>::create(n);
					GVertex* v = mesh->vertices.getBuf();
					if (!v) {
						return sl_null;
					}
					for (sl_uint32 k = 0; k < 3; k++) {
						sl_uint8 u = 0, m = 0;
						if (!input.readUint8(&u)) {
							return sl_null;
						}
						v[0].raw[k] = u;
						for (sl_uint32 i = 1; i < n; i++) {
							if (!input.readUint8(&m)) {
								return sl_null;
							}
							u = (u + m) & 255;
							v[i].raw[k] = u;
						}
					}
				}
				break;
			case 3: // indices
				{
					sl_uint32 n = 0;
					// skip
					if (!input.readUint32CVLI(&n)) {
						return sl_null;
					}
					if (!input.readUint32CVLI(&n)) {
						return sl_null;
					}
					mesh->indices = Array<sl_uint16>::create(n);
					sl_uint16* indices = mesh->indices.getBuf();
					if (!indices) {
						return sl_null;
					}
					List<sl_uint16> listTriangle;
					sl_uint32 maxIndex = 0;
					sl_uint32 ic = 0;
					sl_uint32 index = 0;
					sl_uint32 before2 = 0, before1 = 0;
					for (sl_uint32 i = 0; i < n; i++) {
						sl_uint32 offsetIndex = 0;
						if (!input.readUint32CVLI(&offsetIndex)) {
							return sl_null;
						}
						before2 = before1;
						before1 = index;
						index = (sl_uint32)(maxIndex - offsetIndex);
						indices[i] = index;
						if (before2 != before1 && before1 != index && before2 != index) {
							listTriangle.add(before2);
							listTriangle.add(before1);
							listTriangle.add(index);
							ic++;
						}
						if (offsetIndex == 0) {
							maxIndex++;
						}
					}
					mesh->index_c = ic;

					mesh->triangleIndices = listTriangle.toArray();
				}
				break;
			case 6: // texture
				{
					Memory buf = input.readSection();
					if (buf.isEmpty()) {
						return sl_null;
					}
					if (!loadTexture(mesh->texture, buf)) {
						return sl_null;
					}
				}
				break;
			case 7: // texture coordinate
				{
					sl_uint32 size = 0;
					if (!input.readUint32CVLI(&size)) {
						return sl_null;
					}
					sl_uint32 nTex = (size - 4) / 4;
					if (nTex != mesh->vertices.count()) {
						return sl_null;
					}
					sl_uint16 _maxTx = 0, _maxTy = 0;
					if (!input.readUint16(&_maxTx)) {
						return sl_null;
					}
					if (!input.readUint16(&_maxTy)) {
						return sl_null;
					}
					sl_uint32 maxTx = _maxTx;
					sl_uint32 maxTy = _maxTy;
					SLIB_SCOPED_ARRAY(sl_uint8, buf, 4 * nTex);
					if (!input.read(buf, 4 * nTex)) {
						return sl_null;
					}
					sl_int32 tx = 0, ty = 0;
					GVertex* vertices = mesh->vertices.getBuf();
					for (sl_uint32 i = 0; i < nTex; i++) {
						tx = (tx + ((sl_uint32)(buf[i]) | ((sl_uint32)(buf[2 * nTex + i]) << 8))) % (maxTx + 1);
						ty = (ty + ((sl_uint32)(buf[nTex + i]) | ((sl_uint32)(buf[3 * nTex + i]) << 8))) % (maxTy + 1);
						vertices[i].txi = (sl_uint16)(tx);
						vertices[i].tyi = (sl_uint16)(ty);
					}
					mesh->uvOffsetX = 0.5;
					mesh->uvOffsetY = 0.5;
					mesh->uvScaleX = 1.0 / (maxTx + 1);
					mesh->uvScaleY = 1.0 / (maxTy + 1);
				}
				break;
			case 8: // layer bounds
				{
					sl_uint32 num = 0;
					if (!input.readUint32CVLI(&num)) {
						return sl_null;
					}
					if (!input.readUint32CVLI(&num)) {
						return sl_null;
					}
					sl_uint32 indexBound = 0;
					sl_uint32 bound = 0;
					for (sl_uint32 i = 0; i < num; i++) {
						if (i % 8 == 0) {
							if (indexBound >= 10) {
								return sl_null;
							}
							mesh->layerBounds[indexBound] = bound;
							indexBound++;
						}
						sl_uint32 m = 0;
						if (!input.readUint32CVLI(&m)) {
							return sl_null;
						}
						stack_bound[stack_bound_index++] = m;
						bound += m;
					}
					for (; indexBound < 10; indexBound++) {
						mesh->layerBounds[indexBound] = bound;
					}
				}
				break;
			case 9: // vertex alpha
				{
					sl_uint32 num = 0;
					if (!input.readUint32CVLI(&num)) {
						return sl_null;
					}
					sl_uint32 a = 0;
					mesh->alphas = Array<sl_uint8>::create(num);
					sl_uint8* alphas = mesh->alphas.getBuf();
					if (!alphas) {
						return sl_null;
					}
					for (sl_uint32 i = 0; i < num; i++) {
						sl_uint8 m = 0;
						if (!input.readUint8(&m)) {
							return sl_null;
						}
						a = (a + m) & 255;
						alphas[i] = a;
					}
				}
				break;
			case 10: // uv offset and scale
				{
					sl_uint32 num = 0;
					if (!input.readUint32CVLI(&num)) {
						return sl_null;
					}
					if (num % 4 != 0) {
						return sl_null;
					}
					num = num / 4;
					mesh->uvOffsetAndScale = Array<float>::create(num);
					float* uvs = mesh->uvOffsetAndScale.getBuf();
					if (!uvs) {
						return sl_null;
					}
					for (sl_uint32 i = 0; i < num; i++) {
						float f = 0;
						if (!input.readFloat(&f)) {
							return sl_null;
						}
						uvs[i] = f;
					}
				}
				break;
			case 11: // normal buffer
				if (m_flagUseNormal) {
					mesh->normalBuffer = input.readSection();
					if (mesh->normalBuffer.isEmpty()) {
						return sl_null;
					}
				} else {
					if (!processTail(input, typeTail)) {
						return sl_null;
					}
				}
				break;
			case 12: // mesh id
				{
					sl_uint32 id = 0;
					if (!input.readUint32CVLI(&id)) {
						return sl_null;
					}
					mesh->id = id;
				}
				break;
			default:
				if (!processTail(input, typeTail)) {
					return sl_null;
				}
				break;
			}
		}

		// index layer octants
		{
			sl_uint32 n = (sl_uint32)(mesh->indices.count());
			mesh->indexLayerOctants = Array<sl_uint32>::create(n);
			sl_uint32* indexLayerOctants = mesh->indexLayerOctants.getBuf();
			if (!indexLayerOctants) {
				return sl_null;
			}
			sl_uint32 index = 0;
			for (sl_uint32 i = 0; i < stack_bound_index; i++)
			{
				sl_uint32 s = i & 7;
				sl_uint32 b = stack_bound[i];
				if (b > 0) {
					m_nonEmptyOctants |= (1 << s);
				}
				for (sl_uint32 k = 0; k < b; k++) {
					indexLayerOctants[index] = i;
					sl_uint32 vertexNumber = mesh->indices[index];
					mesh->vertices[vertexNumber].indexLayerOctant = s;
					index++;
				}
			}
		}

		return mesh;
	}

	sl_bool loadGlobeMatrix(MemoryReader& reader)
	{
		sl_uint32 n = 0;
		if (!reader.readUint32CVLI(&n)) {
			return sl_false;
		}
		if (n != 16 * 8) {
			return sl_false;
		}
		for (sl_uint32 i = 0; i < 16; i++) {
			double f = 0;
			if (!reader.readDouble(&f)) {
				return sl_false;
			}
			m_globeMatrix[i] = f;
		}
		return sl_true;
	}

	sl_bool open(const void* content, sl_size size)
	{
		if (m_flagOpen) {
			close();
		}

		MemoryReader input(content, size);

		while (1) {

			sl_uint32 type = 0, opt = 0;
			if (!readType(input, type, opt)) {
				return sl_false;
			}
			if (type == 0) {
				break;
			}
			switch (type) {
			case 1:
				if (!loadGlobeMatrix(input)) {
					return sl_false;
				}
				break;
			case 2:
				{
					Memory buf = input.readSection();
					if (buf.isEmpty()) {
						return sl_false;
					}
					Ref<GMesh> mesh = loadMesh(buf);
					if (mesh.isNotNull()) {
						m_listMesh.add(mesh);
					} else {
						return sl_false;
					}
				}
				break;
			case 3:
				{
					sl_uint32 n = 0;
					if (!input.readUint32CVLI(&n)) {
						return sl_false;
					}
					m_copyrightIds.add(n);
				}
				break;
			case 6:
				{
					Memory buf = input.readSection();
					if (buf.isEmpty()) {
						return sl_false;
					}
					Ref<GMesh> mesh = loadMesh(buf);
					if (mesh.isNotNull()) {
						m_waterMesh = mesh;
					} else {
						return sl_false;
					}
				}
				break;
			case 7:
				{
					Memory buf = input.readSection();
					if (buf.isEmpty()) {
						return sl_false;
					}
					Ref<GMesh> mesh = loadMesh(buf);
					if (mesh.isNotNull()) {
						m_listSurfaceMesh.add(mesh);
					} else {
						return sl_false;
					}
				}
				break;
			case 8:
				{
					if (m_flagUseNormal) {
						Memory buf = input.readSection();
						if (buf.isNull()) {
							return sl_false;
						}
						/*
						if (!u_ee(buf)) {
							return sl_false;
						}
						*/
					} else {
						if (!processTail(input, opt)) {
							return sl_false;
						}
					}
				}
				break;
			default:
				if (!processTail(input, opt)) {
					return sl_false;
				}
				break;
			}
		}
		m_flagOpen = sl_true;
		return sl_true;
	}

};


sl_bool GEarthTile::load(const void* data, sl_size size)
{
	meshes.clear();

	_GEarthTileFile file;
	if (file.open(data, size)) {
		double* _m = file.m_globeMatrix;
		float m00 = (float)_m[0];
		float m01 = (float)_m[1];
		float m02 = (float)_m[2];
		float m03 = (float)_m[3];
		float m10 = (float)_m[4];
		float m11 = (float)_m[5];
		float m12 = (float)_m[6];
		float m13 = (float)_m[7];
		float m20 = (float)_m[8];
		float m21 = (float)_m[9];
		float m22 = (float)_m[10];
		float m23 = (float)_m[11];
		float m30 = (float)_m[12];
		float m31 = (float)_m[13];
		float m32 = (float)_m[14];
		float m33 = (float)_m[15];
		List< Ref<_GEarthTileFile::GMesh> > gmeshes(file.m_listMesh);
		for (sl_size k = 0; k < gmeshes.count(); k++) {
			Mesh mesh;
			Ref<_GEarthTileFile::GMesh> gmesh = gmeshes[k];
			sl_uint32 nVertices = (sl_uint32)(gmesh->vertices.count());
			SLIB_SCOPED_ARRAY(float, vb, nVertices * 5);
			float* v = vb;
			_GEarthTileFile::GVertex* gv = gmesh->vertices.getBuf();
			
			sl_real roty = SLIB_PI / 2;
			sl_real roty_sin = Math::sin(roty);
			sl_real roty_cos = Math::cos(roty);
			for (sl_uint32 i = 0; i < nVertices; i++) {
				float x = (float)(gv->raw[0]);
				float y = (float)(gv->raw[1]);
				float z = (float)(gv->raw[2]);
				float _rx = x * m00 + y * m10 + z * m20 + m30;
				float _ry = x * m01 + y * m11 + z * m21 + m31;
				float _rz = x * m02 + y * m12 + z * m22 + m32;
				float rx = _rx * roty_cos + _ry * roty_sin;
				float ry = - _rx * roty_sin + _ry * roty_cos;
				float rz = _rz;
				v[0] = rx;
				v[1] = rz;
				v[2] = ry;
				v[3] = Math::clamp(-0.5f + (float)((gv->txi + gmesh->uvOffsetX) * gmesh->uvScaleX) * 2, 0.001f, 0.999f);
				v[4] = Math::clamp(-0.5f + (float)((gv->tyi + gmesh->uvOffsetY) * gmesh->uvScaleY) * 2, 0.001f, 0.999f);
				v += 5;
				gv++;
			}
			mesh.vertexBuffer = VertexBuffer::create(vb, nVertices * 20);
			mesh.indexBuffer = IndexBuffer::create(gmesh->triangleIndices.getBuf(), gmesh->triangleIndices.count() * 2);
			if (gmesh->texture.format == 1) {
				mesh.texture = Texture::loadFromMemory(gmesh->texture.data);
				if (k == 0) {
					texture = mesh.texture;
				}
			}
			if (mesh.vertexBuffer.isNotNull() && mesh.indexBuffer.isNotNull() && mesh.texture.isNotNull()) {
				meshes.add(mesh);
			}
		}
		return sl_true;
	}
	return sl_false;
}

SLIB_MAP_NAMESPACE_END
