#pragma once
#include <DxLib.h>
#include <memory>
#include <chrono>
#include <list>
#include "../../Common/Vector2.h"

// 推奨しませんが、どうしても使いたい方は
#define mainCamera SceneManager::GetInstance().GetCamera().lock()

class SceneBase;
class Fader;
class Camera;

class SceneManager
{
public:

	static constexpr VECTOR LIGHT_DIR = { 0.0f, -0.5f, 0.3f }; // デフォルトのライト方向

	// シーン管理用
	enum class SCENE_ID
	{
		NONE,
		TITLE,
		DEBUG,
		ADVERTISE,
		MOVIE,
		SELECT,
		TUTORIAL,
		GAME,
		PAUSE,
		INVENTORY,
		CHARACTER,
		OPTION,
		RESULT,
	};
	
	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static SceneManager& GetInstance(void);

	void Init(void);
	void Init3D(void);
	void Update(void);
	void Draw(void);

	// リソースの破棄
	void Destroy(void);

	// 状態遷移
	void ChangeScene(SCENE_ID nextId);
	void ChangeScene(std::unique_ptr<SceneBase> _scene);

	// シーンIDの取得
	SCENE_ID GetSceneID(void);

	// デルタタイムの取得
	float GetDeltaTime(void) const;

	// カメラの取得
	std::weak_ptr<Camera> GetCamera(void) const;

	/// <summary>
	/// シーンを新しく積む
	/// </summary>
	/// <param name="_scene">シーン情報</param>
	void PushScene(std::unique_ptr<SceneBase>_scene);
	void PushScene(SCENE_ID _scene);

	/// <summary>
	/// 最後に追加したシーンを削除する。
	/// </summary>
	/// <param name=""></param>
	void PopScene(void);

	/// <summary>
	/// 強制的に特定のシーンに飛ぶ。つんでてもリセット
	/// </summary>
	/// <param name="scene">ジャンプ先シーン</param>
	void JumpScene(std::unique_ptr<SceneBase> scene);

	void SetShakeScreen(bool isShake);

	void SceneID2Game(void) { sceneId_ = SCENE_ID::GAME; }

	int GetMainScreen(void) const { return mainScreen_; }

private:

	// 静的インスタンス
	static SceneManager* instance_;

	//シーンID
	SCENE_ID sceneId_;		//現在のシーンID
	SCENE_ID waitSceneId_;	//待ち（次の）シーンID

	// 各種
	std::unique_ptr<SceneBase> scene_;
	std::list<std::unique_ptr<SceneBase>>scenes_;

	// フェード
	std::unique_ptr<Fader> fader_;

	// カメラ
	std::shared_ptr<Camera> camera_;

	// シーン遷移中判定
	bool isSceneChanging_;

	// デルタタイム
	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;

	//フォグ
	float fogStart_;	//開始位置
	float fogEnd_;	//終了位置

	//ライトの方向
	VECTOR lightDir_;

	//シーンのレンダーターゲット
	int mainScreen_;

	//画面揺らし用
	int shakeFrame_;	//
	float shakeRate_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	SceneManager(void);
	// コピーコンストラクタも同様
	SceneManager(const SceneManager& manager) = default;
	// デストラクタも同様
	~SceneManager(void) = default;

	// デルタタイムをリセットする
	void ResetDeltaTime(void);

	// シーン遷移
	void DoChangeScene(SCENE_ID sceneId);

	// フェード
	void Fade(void);

	void ShakeScreen(void);

	//SCENE_IDからシーンを生成する
	template<typename T = SceneBase>
	std::unique_ptr<T> CreateScene(SCENE_ID sceneId);

	void UpdateDebugImGui(void);
};
