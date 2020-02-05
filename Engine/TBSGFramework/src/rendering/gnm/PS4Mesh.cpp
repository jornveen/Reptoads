#ifdef PLATFORM_PS
#include <string>
#include "rendering/gnm/PS4Mesh.h"
#include "core/Assertion.h"
namespace gfx
{
	void MultiplyBuffer(const Vertex* buffer, size_t bufferCount, size_t stride, size_t floatCount, float value)
	{
		Vertex* thingy = const_cast<Vertex*>(buffer);
		float* floatBuf = reinterpret_cast<float*>(thingy);
		const size_t VertexFloatCount = sizeof(Vertex) / sizeof(float);
		for (auto i = 0; i < bufferCount; ++i)
		{
			for(auto j = 0; j < floatCount; ++j)
			{
				float& v = *(floatBuf + VertexFloatCount * i + stride + j);
				v *= value;
			}
		}
	}

	PS4Mesh::PS4Mesh(	const Vertex* a_vertices, 
						const size_t a_vtxCount, 
						const uint32_t* a_indices, 
						const size_t a_idxCount,
						const MeshId a_id, 
						ptl::FreeListResource& memResource) :
		Mesh{static_cast<uint32_t>(a_vtxCount), static_cast<uint32_t>(a_idxCount)},
		PS4Resource(a_id)
		, m_vtxData(nullptr)
		, m_idxData(nullptr)
		, m_attribFormats
		({
			{ vertexPosition, sce::Gnm::kDataFormatR32G32B32Float},
			{ vertexColor, sce::Gnm::kDataFormatR32G32B32Float},
			{ vertexNormal, sce::Gnm::kDataFormatR32G32B32Float},
			{ vertexUv, sce::Gnm::kDataFormatR32G32Float}
		})
		, alloc(&memResource)
	{
		// allocate data for the vtx buffer*/
		m_vtxData = static_cast<Vertex*>(memResource.allocate(
			sizeof(Vertex) * a_vtxCount, sce::Gnm::kAlignmentOfBufferInBytes));

		ASSERT_MSG(m_vtxData != nullptr, "Cannot allocate vertex data");

		// allocate data for the idx buffer
		// uint16_t = 2 bytes -> max value = 65535
			   /* */
		m_idxData = static_cast<uint32_t*>(memResource.allocate(
			a_idxCount * sizeof(uint32_t), sce::Gnm::kAlignmentOfBufferInBytes));

		ASSERT_MSG(m_idxData != nullptr, "Cannot allocate index data");

		std::cout << "VERTEX DATA IN RENDERER>>>... ";
		std::cout << ptl::to_string(sizeof(Vertex) * a_vtxCount) << std::endl;

		// copy the new data to the new buffer
		memcpy(m_vtxData, a_vertices, sizeof(Vertex) * a_vtxCount);
		memcpy(m_idxData, a_indices, sizeof(uint32_t) * a_idxCount);

		// register the passes again
		ptl::vector<const PS4RenderPass*> registeredPasses;
		for (auto passIt = m_registeredPasses.begin(); passIt != m_registeredPasses.end(); ++passIt)
		{
			registeredPasses.push_back(passIt->first);
		}

		m_registeredPasses.clear();

		for (auto p : registeredPasses)
			RegisterPass(*p);
	}

	PS4Mesh::~PS4Mesh()
	{
		for (auto& itr : m_registeredPasses)
		{
			alloc->deallocate(itr.second);
		}
	}

	void PS4Mesh::UploadBuffers(const Vertex* a_vertices, size_t a_vtxCount, const uint32_t* a_indices, size_t a_idxCount)
	{
		
	}

	size_t PS4Mesh::AddOnMeshUpdated(std::function<void()> a_func)
	{
		++m_count;
		m_funcs.emplace(m_count, a_func);
		return m_count;
	}

	void PS4Mesh::RemoveOnMeshUpdated(size_t a_id)
	{
		ASSERT_MSG(m_funcs.find(a_id) != m_funcs.end(), "ERROR, callback entry does not exist..");
		m_funcs.erase(m_funcs.find(a_id));
	}

	sce::Gnm::DataFormat gfx::PS4Mesh::GetDataFormat(uint8_t a_bit) const
	{
		auto v = m_attribFormats.find(a_bit);
		ASSERT_MSG(v != m_attribFormats.end(), "INVALID BIT PROVIDED...");
		return v->second;
	}

	uint32_t gfx::PS4Mesh::GetAttributeSizeInBytes(uint8_t a_bit) const
	{
		auto v = m_attribFormats.find(a_bit);
		ASSERT_MSG(v != m_attribFormats.end(), ptl::string(("INVALID BIT PROVIDED... :") + ptl::to_string(a_bit)).c_str());
		return v->second.getNumComponents();
	}

	sce::Gnm::Buffer* PS4Mesh::GetAttributeBuffers(const PS4RenderPass& a_pass)
	{
		auto v = m_registeredPasses.find(&a_pass);
		if (v == m_registeredPasses.end())
		{
			RegisterPass(a_pass);
			v = --m_registeredPasses.end();
		}
		ASSERT_MSG(v != m_registeredPasses.end(), "error, the pass has not been registered yet.")
			return v->second;
	}

	void PS4Mesh::RegisterPass(const PS4RenderPass& a_pass)
	{
		if (m_registeredPasses.find(&a_pass) != m_registeredPasses.end())
			return;

		// get the bitmask from the render pass
		const uint8_t bitmask = a_pass.GetAttributeBitMask();
		sce::Gnm::Buffer* buffers = reinterpret_cast<sce::Gnm::Buffer*>(ptl::DefaultMemoryResource::get_default_allocator()->allocate(sizeof(sce::Gnm::Buffer) * a_pass.GetAttributeCount(), 1));
		m_registeredPasses.emplace(&a_pass, buffers);

		// loop through all bits
		uint8_t offset = 0;
		uint8_t attribCount = 0;
		// max 8 bits in uint8_t
		for (auto i = 0; i < 8; ++i)
		{
			const uint8_t bitValue = 1 << i;
			// check if end has been reached
			if (bitValue >= ATTRIBUTE_END_BIT)
			{
				std::cout << std::to_string(ATTRIBUTE_END_BIT) << std::endl;
				ASSERT_MSG(a_pass.GetAttributeCount() == attribCount, "ATTRIB INITIALIZATION ERROR...");
				std::cout << "Renderable initialized with " + std::to_string(attribCount) + " attributes" << std::endl;
				break;
			}

			// check if the bit is enabled
			if ((bitmask)& bitValue)
			{
				// initialize the next buffer pointing to the next attribute with the vertex stride
				auto format = GetDataFormat(bitValue);
				buffers[attribCount].initAsVertexBuffer(reinterpret_cast<float*>(m_vtxData) + offset, format, sizeof(gfx::Vertex), m_vtxCount);
				// update the offset so the next stride will point to the next attribute
				offset += GetAttributeSizeInBytes(bitValue);
				++attribCount;
			}
			// add the offset when the bit is not set to assure unused attributes are skipped
			else offset += GetAttributeSizeInBytes(bitValue);
		}

		ASSERT_MSG(attribCount == a_pass.GetAttributeCount(),"BUFFER IS NOT INITIALIZED CORRECTLY!")
	}
#endif 
}