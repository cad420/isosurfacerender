#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

#include "../mathematics/transformation.h"
#include <vector>
#include <unordered_map>


namespace ysl
{
	class TriangleMesh 
	{
		std::unique_ptr<Point3f[]> m_vertices;
		//std::unique_ptr<int[]> m_vertexIndices;
		std::unique_ptr<Vector3f[]> m_normals;
		std::unique_ptr<Point2f[]> m_textures;
		std::vector<int> m_vertexIndices;
		const int m_nTriangles;
		const int m_nVertex;
		ysl::Bound3f m_bound;

	public:
		TriangleMesh(const Transform & objectToWorld,
			const Point3f * vertices,
			const Vector3f * normals,
			const Point2f * textures,
			int nVertex,
			const int * vertexIndices,
			int nTriangles)noexcept:
			m_nVertex(nVertex),
			m_nTriangles(nTriangles),
			m_vertexIndices(vertexIndices,vertexIndices+3*nTriangles)
		{
			m_vertices.reset(new ysl::Point3f[nVertex]);
			for (int i = 0; i < nVertex; i++)
			{
				m_vertices[i] = objectToWorld * vertices[i];
				//objectToWorld += m_vertices[i];
			}
			//m_centroid /= nVertex;

			int normalBytes = 0;
			int textureBytes = 0;
			if (normals != nullptr) {
				//const auto nm = trans.normalMatrix();
				const auto nm = objectToWorld.Matrix().NormalMatrix();
				normalBytes = m_nVertex * sizeof(ysl::Vector3f);
				m_normals.reset(new ysl::Vector3f[nVertex]);
				for (int i = 0; i < nVertex; i++) {
					//m_normals[i] = trans * normals[i];
					const auto x = normals[i].x, y = normals[i].y, z = normals[i].z;
					m_normals[i] = ysl::Vector3f{ x*nm.m[0][0] + y * nm.m[0][1] + z * nm.m[0][2],x*nm.m[1][0] + y * nm.m[1][1] + z * nm.m[1][2],x*nm.m[2][0] + y * nm.m[2][1] + z * nm.m[2][2] }.Normalized();
				}
			}
			if (textures != nullptr) {
				textureBytes = m_nVertex * sizeof(ysl::Point2f);
				m_textures.reset(new ysl::Point2f[nVertex]);
				std::memcpy(m_textures.get(), textures, nVertex * sizeof(ysl::Point2f));
			}


		}
		const Point3f * Vertices()const
		{
			return m_vertices.get();
		}
		int VertexCount()const
		{
			return m_nVertex;
		}
		const int * Indices()const
		{
			return m_vertexIndices.data();
		}
		int TriangleCount()const
		{
			return m_nTriangles;
		}
		const Vector3f *Normals()const 
		{
			return m_normals.get();
		}

		const Point2f *Textures()const
		{
			return m_textures.get();
		}
		//void transform(const Transform & trans)
		//{
		//	for (int i = 0; i < m_nTriangles; i++)
		//	{
		//		m_vertices[i] = trans * m_vertices[i];
		//	}
		//}
		friend class Triangle;
	};

	std::shared_ptr<TriangleMesh> CreateTriangleMeshFromFile(const ysl::Transform& objectToWorld, const std::string & fileName);



}

#endif /*_TRIANGLE_H_*/
