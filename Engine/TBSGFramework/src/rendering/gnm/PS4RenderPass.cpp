#ifdef PLATFORM_PS
#include "rendering/gnm/PS4RenderPass.h"
#include "core/Assertion.h"
#include "rendering/gnm/PS4Shaders.h"
#include <common/allocator.h>
#include "rendering/Mesh.h"

gfx::PS4RenderPass::PS4RenderPass(uint8_t a_attributeBitMask, ptl::vector<IShader*> a_shaders, RenderPassId a_id): PS4Resource(a_id), m_shaders(a_shaders)
{
	SetAttributeBitmask(a_attributeBitMask);
	SetShaderBitmask();

	// create and init the depth stencil desc
	// Enable z-writes using a less comparison function
	m_depthStencilDesc.init();
	m_depthStencilDesc.setDepthControl(sce::Gnm::kDepthControlZWriteEnable, sce::Gnm::kCompareFuncLess);
	m_depthStencilDesc.setDepthEnable(true);

	// Cull clock-wise backfaces
	m_primitiveDesc.init();
	m_primitiveDesc.setCullFace(sce::Gnm::kPrimitiveSetupCullFaceBack);
	m_primitiveDesc.setFrontFace(sce::Gnm::kPrimitiveSetupFrontFaceCcw);

	// Setup an additive blending mode
	m_blendDesc.init();
	m_blendDesc.setBlendEnable(true);
	m_blendDesc.setColorEquation(
		sce::Gnm::kBlendMultiplierSrcAlpha,
		sce::Gnm::kBlendFuncAdd,
		sce::Gnm::kBlendMultiplierOneMinusSrcAlpha);

	
}

void gfx::PS4RenderPass::PrepareForRendering(sce::Gnmx::GnmxGfxContext& a_gfxc) const
{
	a_gfxc.setDepthStencilControl(m_depthStencilDesc);
	a_gfxc.setPrimitiveSetup(m_primitiveDesc);
	a_gfxc.setBlendControl(0, m_blendDesc);

/*
	sce::Gnm::AlphaToMaskControl control{};
	control.init();
	control.setEnabled(sce::Gnm::kAlphaToMaskEnable);
	a_gfxc.setAlphaToMaskControl(control);
*/

	a_gfxc.setActiveShaderStages(m_activeShaderStages);

	for (auto s : m_shaders)
	{
		switch (s->type)
		{
		// guaranteed to be typesafe
		case VS: { auto x = static_cast<PS4VShader*>(s);  auto& y = x->GetShader();  a_gfxc.setVsShader(&y, x->GetFetchShader().shaderModifier, x->GetFetchShader().memory, &x->GetData().inputOffsetsCache); } break;
		case PS: { auto x = static_cast<PS4PShader*>(s);  auto& y = x->GetShader();  a_gfxc.setPsShader(&y, &x->GetData().inputOffsetsCache); } break;
		case CS: { auto x = static_cast<PS4CShader*>(s);  auto& y = x->GetShader();  a_gfxc.setCsShader(&y, &x->GetData().inputOffsetsCache); } break;
		default: ASSERT(0);
		}
	}
}

uint8_t gfx::PS4RenderPass::GetAttributeBitMask() const
{
	return m_attributeBitMask;
}

void gfx::PS4RenderPass::SetAttributeBitmask(uint8_t a_bitmask)
{
	m_attributeBitMask = a_bitmask;
	// 8 bits in uint8_t
	for (int i = 0; i < 8; ++i)
	{
		uint8_t bitValue = 1 << i;

		// check if the end has been reached
		if (bitValue > ATTRIBUTE_END_BIT)
			break;

		if ((m_attributeBitMask) & (1 << i))
		{
			++m_attributeCount;
		}
	}
}

void gfx::PS4RenderPass::SetShaderBitmask()
{
	for(auto s : m_shaders)
	{
		ASSERT_MSG(s->type <= SHADER_END_BIT, "BIT EXCEEDS MAXIMUM OF 8");
		if(m_shaderBitMask & s->type)
		{
			ASSERT_MSG(false, "RENDER PASS CAN'T HAVE TWO SHADERS OF THE SAME TYPE!")
		}
		// flip the bit
		m_shaderBitMask += s->type;
	}

	if (m_shaderBitMask & VS && m_shaderBitMask & PS) m_activeShaderStages = sce::Gnm::kActiveShaderStagesVsPs;
	else if (CS) return;
	else ASSERT_MSG(false, "THE CURRENT SHADER COMBINATION HAS NOT BEEN DEFINED YET.....");
}
#endif