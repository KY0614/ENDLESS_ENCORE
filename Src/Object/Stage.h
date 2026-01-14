#pragma once
#include "Common/Transform.h"

class ModelRenderer;
class ModelMaterial;

class Stage
{
public:
	//コンストラクタ
	Stage(void);
	//デストラクタ
	~Stage(void);

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="pos">ポイントライトの座標</param>
	/// <param name="sPos">スポットライトの座標</param>
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
	/// ステージのモデル情報を取得する
	/// </summary>
	/// <returns>ステージモデル情報</returns>
	const Transform& GetTransform(void){ return transform_; }

	/// <summary>
	/// 霧の壁のモデル情報を取得する
	/// </summary>
	/// <returns>霧の壁のモデル情報</returns>
	const Transform& GetMistWallTransform(void) { return mistWallTransform_; }

	/// <summary>
	/// バトル中にする(バトル用の霧の壁を表示する)
	/// </summary>
	void IsBattle(void) { isBattle_ = true; }

private:
	//ステージのマテリアルとレンダー
	std::unique_ptr<ModelMaterial> stageMaterial_;
	std::unique_ptr<ModelRenderer> stageRenderer_;
	
	//霧の壁のマテリアルとレンダー
	std::unique_ptr<ModelMaterial> mistWallMaterial_;
	std::unique_ptr<ModelRenderer> mistWallRenderer_;

	//ステージ本体のモデル情報
	Transform transform_;

	//戦闘中に出す霧の壁のモデル情報
	Transform mistWallTransform_;
	//霧の壁のノイズテクスチャID
	int noiseTextureId_;
	//霧の壁のスクロール速度
	float mistScrollSpeed_;
	//霧の壁の溶解ラインのアルファ値
	float dissolveAlphaLine_;

	//戦闘中かどうか
	bool isBattle_;		//true:戦闘中 false:戦闘中ではない

	/// <summary>
	/// 3Dモデル初期化
	/// </summary>
	void Init3DModel(void);

	/// <summary>
	/// マテリアル初期化
	/// </summary>
	/// <param name="pos">ポイントライトの座標</param>
	/// <param name="sPos">スポットライトの座標</param>
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