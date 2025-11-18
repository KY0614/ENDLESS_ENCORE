#pragma once
#include <map>
#include "Common/Transform.h"

class Cube;
class Box;

class ModelRenderer;
class ModelMaterial;

class Stage
{
public:

	enum class TYPE
	{
		EXPLORE,	//探索ステージ
		BATTLE		//戦闘ステージ
	};

	Stage(void);
	//デストラクタ
	~Stage(void);

	/// <summary>
	///	初期化
	/// </summary>
	void Init(void);

	/// <summary>
	///	更新処理
	/// </summary>
	void Update(void);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void);

	//Cube& GetStageCube(void) { return *cube_; }

	/// <summary>
	/// ステージを変更する
	/// </summary>
	/// <param name="type">指定するステージ</param>
	void ChangeType(const TYPE& type) { type_ = type; }

	/// <summary>
	/// デバッグ用ImGuiの更新(ウィンドウを表示し、各種変数を操作可能にする)
	/// </summary>
	void UpdateDebugImGui(void);

	/// <summary>
	/// ステージのモデル情報を取得する
	/// </summary>
	/// <returns>現在のステージモデル情報</returns>
	const Transform& GetTransform(void){ return stageTransform_[type_]; }
private:
	std::unique_ptr<ModelMaterial> material_;
	std::unique_ptr<ModelRenderer> renderer_;

	TYPE type_;

	std::unordered_map<TYPE, Transform> stageTransform_;

	//std::unique_ptr<Cube> cube_;
	std::unique_ptr<Box> cube_;

	/// <summary>
	/// 3Dモデル初期化
	/// </summary>
	void Init3DModel(void);
};

