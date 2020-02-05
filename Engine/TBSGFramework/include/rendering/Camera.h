#pragma once

/*
 *  Copyright(c) 2018 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

 /**
  *  @file Camera.h
  *  @date October 24, 2018
  *  @author Jeremiah van Oosten
  *
  *  @brief DirectX 12 Camera class.
  */
#pragma warning( push )
#pragma warning( disable : 4127)
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#pragma warning( pop ) 
namespace gfx
{
	// When performing transformations on the camera, 
	// it is sometimes useful to express which space this 
	// transformation should be applied.
	enum class Space
	{
		Local,
		World,
	};

	class Camera
	{
	public:

		Camera();
		virtual ~Camera();

		void set_LookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);
		glm::mat4 get_ViewMatrix() const;
		glm::mat4 get_InverseViewMatrix() const;

		/**
		 * Set the camera to a perspective projection matrix.
		 * @param fovy The vertical field of view in degrees.
		 * @param aspect The aspect ratio of the screen.
		 * @param zNear The distance to the near clipping plane.
		 * @param zFar The distance to the far clipping plane.
		 */
		void set_Projection(float fovy, float aspect, float zNear, float zFar);
		glm::mat4 get_ProjectionMatrix() const;
		glm::mat4 get_InverseProjectionMatrix() const;

		/**
		 * Set the field of view in degrees.
		 */
		void set_FoV(float fovy);

		/**
		 * Get the field of view in degrees.
		 */
		float get_FoV() const;
		float get_ZNear() const { return m_zNear; }
		float get_ZFar() const { return m_zFar; }

		/**
		 * Set the camera's position in world-space.
		 */
		void set_Translation(glm::vec3 translation);
		glm::vec3 get_Translation() const;

		/**
		 * Set the camera's rotation in world-space.
		 * @param rotation The rotation quaternion.
		 */
		void set_Rotation(glm::quat rotation);
		/**
		 * Query the camera's rotation.
		 * @returns The camera's rotation quaternion.
		 */
		glm::quat get_Rotation() const;

		void Translate(glm::vec3 translation, Space space = Space::Local);
		void Rotate(glm::quat quaternion);

		// World-space position of the camera.
		glm::vec3 m_Translation;
		// World-space rotation of the camera.
		glm::quat m_Rotation;

	protected:
		virtual void UpdateViewMatrix() const;
		virtual void UpdateInverseViewMatrix() const;
		virtual void UpdateProjectionMatrix() const;
		virtual void UpdateInverseProjectionMatrix() const;

		// This data must be aligned otherwise the SSE intrinsics fail
		// and throw exceptions.
		// __declspec(align(16)) struct AlignedData
		// {
		// 	// World-space position of the camera.
		// 	glm::vec3 m_Translation;
		// 	// World-space rotation of the camera.
		// 	// THIS IS A QUATERNION!!!!
		// 	glm::quat m_Rotation; 
		//
		// 	glm::mat4 m_ViewMatrix, m_InverseViewMatrix;
		// 	glm::mat4 m_ProjectionMatrix, m_InverseProjectionMatrix;
		// };
		// AlignedData* pData;


		mutable glm::mat4 m_ViewMatrix, m_InverseViewMatrix;
		mutable glm::mat4 m_ProjectionMatrix, m_InverseProjectionMatrix;

		// projection parameters
		float m_vFoV;   // Vertical field of view.
		float m_AspectRatio; // Aspect ratio
		float m_zNear;      // Near clip distance
		float m_zFar;       // Far clip distance.

		// True if the view matrix needs to be updated.
		mutable bool m_ViewDirty, m_InverseViewDirty;
		// True if the projection matrix needs to be updated.
		mutable bool m_ProjectionDirty, m_InverseProjectionDirty;

	private:

	};
}