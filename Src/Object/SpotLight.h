#pragma once
#include <memory>
#include "ActorBase.h"

class ModelRenderer;
class ModelMaterial;

class SpotLight :  public ActorBase
{
public:

	static constexpr float SPEC_POW = 50.0f;
	// コンストラクタ
	SpotLight(void);
	// デストラクタ
	~SpotLight(void);
	// 初期化・更新・描画
	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

	void SetPos(VECTOR pos);
private:
	std::unique_ptr<ModelMaterial> material_;
	std::unique_ptr<ModelRenderer> renderer_;
};

