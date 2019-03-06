
#include "triangle.h"
#include "utility/objreader.h"
#include "utility/error.h"

namespace ysl
{

	std::shared_ptr<TriangleMesh> CreateTriangleMeshFromFile(const ysl::Transform& objectToWorld, const std::string& fileName)
	{
		ObjReader reader(fileName);

		if(!reader.IsLoaded())
		{
			ysl::Warning("%s cannot be loaded\n", fileName.c_str());
		}

		return std::make_shared<TriangleMesh>(objectToWorld,
			reader.getVertices().data(),
			reader.getNormals().data(), 
			reader.getTextureCoord().data(), 
			reader.getVertexCount(), 
			reader.getFaceIndices().data(), 
			reader.getFaceIndices().size() / 3);
	}


}
