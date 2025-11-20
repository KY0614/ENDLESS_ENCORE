#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Renderer/ModelMaterial.h"
#include "../Renderer/ModelRenderer.h"
#include "SpotLight.h"

SpotLight::SpotLight(void)
{
}

SpotLight::~SpotLight(void)
{
}

void SpotLight::Init(void)
{

	// ƒ‚ƒfƒ‹‚ÌŠî–{î•ñ
	transform_.SetModel(
		ResourceManager::GetInstance().LoadModelDuplicate(
			ResourceManager::SRC::COIN)
	);
	transform_.pos = { 1.0f,-30.0f,1830.0f };
	transform_.scl = { 1.0f,1.0f,1.0f };
	transform_.Update();

	material_ = std::make_unique<ModelMaterial>(
		"SpotLightVS.cso", 0,
		"SpotLightPS.cso", 0
	);

	renderer_ = std::make_unique<ModelRenderer>(
		transform_.modelId, *material_
	);
}

void SpotLight::Update(void)
{
	transform_.Update();
}

void SpotLight::Draw(void)
{
	renderer_->Draw();
}