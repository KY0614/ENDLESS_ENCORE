#pragma once
#include <vector>
#include <memory>
#include "../Object/Common/Transform.h"
#include "SceneBase.h"

class PixelMaterial;
class PixelRenderer;
class Player;
class Enemy;
class Stage;
class EncountScene;
class PointLight;

class GameScene : public SceneBase
{
public:

	//コンストラクタ
	GameScene(void);

	//デストラクタ
	~GameScene(void);

	/// <summary>
	/// データ読込処理
	/// </summary>
	void LoadData(void) override;

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

	VECTOR GetPointLightPos();

private:
	// ポストエフェクト用スクリーン
	int postEffectScreen_;

	// ポストエフェクト用(ブラー)
	std::unique_ptr<PixelMaterial> blurMaterial_;
	std::unique_ptr<PixelRenderer> blurRenderer_;

	//関数ポインタ（カウントダウン、ゲーム中、タイムアップ）
	using UpdateFunc_t = void(GameScene::*)();
	using DrawFunc_t = void(GameScene::*)();

	UpdateFunc_t update_;
	DrawFunc_t draw_;

	//フェーズ管理
	bool isFaseChange_;

	//プレイヤー
	std::shared_ptr<Player> player_;
	//敵
	std::shared_ptr<Enemy> enemy_;
	//ステージ
	std::shared_ptr<Stage> stage_;
	//演出
	std::unique_ptr<EncountScene> encountScene_;
	// プレイヤー
	std::vector<std::unique_ptr<PointLight>> pointLight_;

	//メッセージクラスみたいなクラスを作って分けてもいいかも
	//選択肢文字列リスト
	//std::vector<std::wstring> selectList_;
	////選択肢関数テーブル
	//using SelectFunc_t = std::function<void()>;
	//std::map<std::wstring, SelectFunc_t> selectFuncTable_;
	////現在選択しているもの
	//int cursorIdx_;

	////触れているかどうか
	//bool isToutch_;

	/// <summary>
	/// 探索フェーズの更新処理
	/// </summary>
	void UpdateExplore(void);

	/// <summary>
	/// 探索フェーズの描画処理
	/// </summary>
	void DrawExplore(void);

	/// <summary>
	/// エンカウント中の更新処理
	/// </summary>
	void UpdateEncount(void);

	/// <summary>
	/// エンカウント中の描画処理
	/// </summary>
	void DrawEncount(void);

	/// <summary>
	/// ゲーム中の更新処理
	/// </summary>
	void UpdateGame(void);

	/// <summary>
	/// ゲーム中の描画
	/// </summary>
	void DrawGame(void);

	void DrawMessage(void);

	void UpdateDebugImGui(void);
};
