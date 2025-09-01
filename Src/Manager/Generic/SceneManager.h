#pragma once
#include <DxLib.h>
#include <memory>
#include <chrono>
#include <list>

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
		MOVIE,
		SELECT,
		TUTORIAL,
		GAME,
		PAUSE,
		RESULT
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

private:

	// 静的インスタンス
	static SceneManager* instance_;

	SCENE_ID sceneId_;
	SCENE_ID waitSceneId_;

	// フェード
	std::unique_ptr<SceneBase> scene_;
	std::list<std::unique_ptr<SceneBase>>scenes_;

	// 各種シーン
	std::unique_ptr<Fader> fader_;

	// カメラ
	std::shared_ptr<Camera> camera_;

	// シーン遷移中判定
	bool isSceneChanging_;

	// デルタタイム
	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;

	VECTOR lightDir_;
	
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

	void MakeScene(SCENE_ID sceneId);

	void UpdateDebugImGui(void);
};
