#pragma once
#include <IApplication.h>
#include "rendering/Camera.h"
#include <chrono>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include "rendering/IParticleRenderer.h"
#include "memory/smart_ptr.h"
#include "core/Input.h"
#include "audio/AudioSystem.h"
#include "resource_loading/ArgumentParsing.h"


//TMP
#include "ui/UIText.h"

namespace ui
{
    class UISystem;
}


class ViewerApplication final : public tbsg::IApplication
{
public:
	~ViewerApplication() override = default;
	void Initialize(int argc, char* argv[]) override;
	void Run() override;
protected:
	void Shutdown() override;

private:
	bool ReloadParticleEmitter(tbsg::InputAction action);

private:
	gfx::Camera camera;
	ptl::string modelPathToLoad;
	ptl::string particlePathToLoad;
    tbsg::ParsedArguments arguments{};
	gfx::ParticleEmitterId previewParticleEmitterId{0};

	//TMP
	bool ScaleUpHor(tbsg::InputAction ia);
	bool ScaleDownHor(tbsg::InputAction ia);
	bool ScaleUpVert(tbsg::InputAction ia);
	bool ScaleDownVert(tbsg::InputAction ia);
	//END TMP

	std::chrono::high_resolution_clock::time_point startTime;
	std::chrono::high_resolution_clock::time_point lastTime;
#ifdef PLATFORM_PS
	bool Vert(float val);
	bool Hori(float val);
	bool RotHori(float v) { yaw -= v * 0.05;  camera.set_Rotation(glm::toQuat(glm::yawPitchRoll<float>(yaw, roll, 0))); return false; }
	bool RotVert(float v) { roll += v * 0.05;  camera.set_Rotation(glm::toQuat(glm::yawPitchRoll<float>(yaw, roll, 0))); return false; }
	float yaw{}, roll{};
#endif
	ptl::unique_ptr<gfx::IParticleRenderer> particleRenderer;
    ui::UISystem* uiSystem;
    audio::AudioSystem audioSystem{};
	ui::UIText TMP_TXT;
	float width = 256;
	float height = 512;
	ptl::string skyboxFile;
};
