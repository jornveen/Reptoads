#pragma once
#ifdef PLATFORM_PS
#include <glm/mat4x2.hpp>
#include <vectormath/scalar_cpp/vectormath_aos.h>
#include "rendering/ResourceIds.h"
namespace gfx
{
	class PS4Resource
	{
	public:
		PS4Resource(Identifier a_id);
		virtual ~PS4Resource() = default;
		PS4Resource(const PS4Resource&) = delete; // you can't copy resources!!!
		sce::Vectormath::Scalar::Aos::Matrix4 ConvertMatrix(const glm::mat4& a_mat) const;
		sce::Vectormath::Scalar::Aos::Vector4 ConvertVector(const glm::vec4& a_mat) const;
		Identifier GetId() const { return m_id; }
	private:
		Identifier m_id;
	};

	inline PS4Resource::PS4Resource(Identifier a_id) : m_id(a_id)
	{
	}

	inline sce::Vectormath::Scalar::Aos::Matrix4 PS4Resource::ConvertMatrix(const glm::mat4& a_mat) const
	{
		return sce::Vectormath::Scalar::Aos::Matrix4(ConvertVector(a_mat[0]),ConvertVector(a_mat[1]),ConvertVector(a_mat[2]),ConvertVector(a_mat[3]));
	}

	inline sce::Vectormath::Scalar::Aos::Vector4 PS4Resource::ConvertVector(const glm::vec4& a_vec) const
	{
		return sce::Vectormath::Scalar::Aos::Vector4(a_vec.x, a_vec.y, a_vec.z, a_vec.w);
	}
}
#endif