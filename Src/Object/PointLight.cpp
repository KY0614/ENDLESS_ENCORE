#include "../Manager/Generic/ResourceManager.h"
#include "../Manager/Generic/Camera.h"
#include "../Manager/Generic/SceneManager.h"
#include "../Renderer/ModelMaterial.h"
#include "../Renderer/ModelRenderer.h"
#include "PointLight.h"

PointLight::PointLight(void)
{
}

PointLight::~PointLight(void)
{
}

void PointLight::Init(void)
{

	// ƒ‚ƒfƒ‹‚ÌŠî–{î•ñ
	transform_.SetModel(
		ResourceManager::GetInstance().LoadModelDuplicate(
			ResourceManager::SRC::COIN)
	);
	transform_.pos = { 1.0f,-30.0f,1.0f };
	transform_.scl = { 1.0f,1.0f,1.0f };
	transform_.Update();

	material_ = std::make_unique<ModelMaterial>(
		"PointLightVS.cso", 0,
		"PointLightPS.cso", 0
	);
	material_->AddConstBufPS({ 1.0f, 1.0f, 1.0f, 1.0f });

	renderer_ = std::make_unique<ModelRenderer>(
		transform_.modelId, *material_
	);
}

void PointLight::Update(void)
{
	transform_.Update();
}

void PointLight::Draw(void)
{
	renderer_->Draw();
}
