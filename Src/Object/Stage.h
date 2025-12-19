#pragma once
#include "Common/Transform.h"

class Cube;
class Box;

class ModelRenderer;
class ModelMaterial;

class Stage
{
public:

	Stage(void);
	//デストラクタ
	~Stage(void);

	/// <summary>
	///	初期化
	/// </summary>
	void Init(const VECTOR pos = {0.0f,-10000.0f,0.0f}, VECTOR sPos = { 0.0f,0.0f,0.0f });

	/// <summary>
	///	更新処理
	/// </summary>
	void Update(void);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void);

	/// <summary>
	/// デバッグ用ImGuiの更新(ウィンドウを表示し、各種変数を操作可能にする)
	/// </summary>
	void UpdateDebugImGui(void);

	/// <summary>
	/// ステージのモデル情報を取得する
	/// </summary>
	/// <returns>現在のステージモデル情報</returns>
	const Transform& GetTransform(void){ return transform_; }

	const Transform& GetMistWallTransform(void) { return mistWallTransform_; }

	void IsBattle(void) { isBattle_ = true; }

	void SetMistWall(const bool isVisible) { isBattle_ = isVisible; }

private:
	//ステージのマテリアルとレンダー
	std::unique_ptr<ModelMaterial> stageMaterial_;
	std::unique_ptr<ModelRenderer> stageRenderer_;
	
	//霧の壁のマテリアルとレンダー
	std::unique_ptr<ModelMaterial> mistWallMaterial_;
	std::unique_ptr<ModelRenderer> mistWallRenderer_;

	int mistWallNoiseTex_;

	//ステージ本体のモデル情報
	Transform transform_;

	//戦闘中に出す霧の壁のモデル情報
	Transform mistWallTransform_;
	int noiseTextureId_;
	float mistScrollSpeed_;
	float dissolveAlphaLine_;

	//std::unique_ptr<Cube> cube_;
	std::unique_ptr<Box> cube_;

	//戦闘中かどうか
	bool isBattle_;		//true:戦闘中 false:戦闘中ではない

	/// <summary>
	/// 3Dモデル初期化
	/// </summary>
	void Init3DModel(void);

	/// <summary>
	/// マテリアル初期化
	/// </summary>
	void InitMaterial(const VECTOR pos = { 0.0f,0.0f,0.0f }, VECTOR sPos = { 0.0f,0.0f,0.0f });

	/// <summary>
	/// ステージマテリアルの定数バッファ更新
	/// </summary>
	void UpdateStageMaterialConstBuf(void);

	/// <summary>
	/// 霧の壁マテリアルの定数バッファ更新
	/// </summary>
	void UpdateMistWallMaterialConstBuf(void);
};

