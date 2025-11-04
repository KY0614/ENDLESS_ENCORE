#pragma once
#include <memory>
#include "SceneBase.h"
#include "../Object/Common/Transform.h"

class SceneManager;
class AnimationController;
class ModelRenderer;
class ModelMaterial;


class TitleScene : public SceneBase
{

public:
	static constexpr float HIGH_LIGHT_INTERVAL = 1.2f;

	//UVスケール
	static constexpr float TILLING_SIZE = 35.0f;

	static constexpr float AMBIENT_COLOR = 0.2f;

	static constexpr int LOGO_OFFSET_Y = 80;
	static constexpr int LOGO_HEIGHT = 1024;
	static constexpr int PUSHIMG_OFFSET_Y = 100;

	//コンストラクタ
	TitleScene(void);

	//デストラクタ
	~TitleScene(void);

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Init(void) override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(void) override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void) override;

private:
	//マテリアル
	std::unique_ptr<ModelMaterial> material_;
	//レンダラ
	std::unique_ptr<ModelRenderer> renderer_;

	//地面用
	Transform graoundTran_;

	//アニメーション
	std::unique_ptr<AnimationController> animationController_;

	int toAdvertiseLoopTimer_;

	/// <summary>
	/// マテリアル情報初期化
	/// </summary>
	/// <param name="">マテリアルの定数バッファ設定</param>
	void InitMaterial(void);

	void UpdateDebugImGui(void);
};
