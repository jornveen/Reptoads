#pragma once
#include "memory/freelist_resources.h"
#ifdef PLATFORM_PS
#include "memory/Containers.h"
#include "PS4RenderPass.h"
#include "rendering/Mesh.h"
#include "PS4Resource.h"
#include "ClassID.h"
#include <functional>

namespace ptl
{
	class MonotonicResource;
}

namespace gfx
{
	class PS4Mesh : public Mesh, public PS4Resource, public ClassID<PS4Mesh>
	{
	public:
		//#TODO add material ID to the constructor
		PS4Mesh(const Vertex* a_vertices, 
				const size_t a_vtxCount, 
				const uint32_t* a_indices, 
				const size_t a_idxCount, 
				const MeshId a_id, 
				ptl::FreeListResource& memResource);
		~PS4Mesh();
		void UploadBuffers(const Vertex* a_vertices, size_t a_vtxCount, const uint32_t* a_indices, size_t a_idxCount) override;
		float* GetDataPtr() const { return (float*)m_vtxData; }
		Vertex& GetVertex(uint32_t index) { return *(m_vtxData + index); }
		const uint32_t* GetIdxBuffer() const { return m_idxData; }
		size_t AddOnMeshUpdated(std::function<void()> a_func);
		void RemoveOnMeshUpdated(size_t);
		sce::Gnm::Buffer* GetAttributeBuffers(const PS4RenderPass& a_pass);
		void RegisterPass(const PS4RenderPass& a_pass);

	private:
		sce::Gnm::DataFormat GetDataFormat(uint8_t a_bit) const;
		uint32_t GetAttributeSizeInBytes(uint8_t a_bit) const;
		ptl::vector<sce::Gnm::Buffer> attributeBuffers;
		//#TODO, make sure it never runs out of options
		size_t m_count = -1;
		ptl::map<size_t,std::function<void()>> m_funcs;
		Vertex* m_vtxData;
		uint32_t* m_idxData;
		const ptl::unordered_map<uint8_t, sce::Gnm::DataFormat> m_attribFormats;
		ptl::unordered_map<const PS4RenderPass*, sce::Gnm::Buffer*> m_registeredPasses;
		ptl::FreeListResource* alloc;
	};
}
#endif	
