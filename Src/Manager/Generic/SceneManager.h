#pragma once
#include <DxLib.h>
#include <memory>
#include <chrono>
#include <list>
#include "../../Common/Vector2.h"

//カメラ取得
#define mainCamera SceneManager::GetInstance().GetCamera().lock()

class SceneBase;
class Fader;
class Camera;

class SceneManager
{
public:
	// ライトの方向のデフォルト値
	static constexpr VECTOR LIGHT_DIR = { 0.0f, -0.5f, 0.3f }; // デフォルトのライト方向

	// シーン管理用
	enum class SCENE_ID
	{
		NONE,
		TITLE,
		GAME,
	};
	
	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static SceneManager& GetInstance(void);

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Init(void);

	/// <summary>
	/// 3D用の設定初期化処理
	/// </summary>
	/// <param name=""></param>
	void Init3D(void);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(void);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(void);

	// リソースの破棄
	void Destroy(void);

	/// <summary>
	/// 状態遷移
	/// </summary>
	/// <param name="nextId">遷移先のシーンID</param>
	void ChangeScene(SCENE_ID nextId);

	// シーンIDの取得
	SCENE_ID GetSceneID(void);

	// デルタタイムの取得
	float GetDeltaTime(void) const;

	// カメラの取得
	std::weak_ptr<Camera> GetCamera(void) const;

	// フェーダーの取得
	std::weak_ptr<Fader> GetFader(void) const;

	/// <summary>
	/// シーンを新しく積む
	/// </summary>
	/// <param name="_scene">積むシーンID</param>
	void PushScene(SCENE_ID _scene);

	// 最後に追加したシーンを削除する
	void PopScene(void);

	/// <summary>
	/// 強制的に特定のシーンに飛ぶ。つんでてもリセット
	/// </summary>
	/// <param name="scene">ジャンプ先シーン</param>
	void JumpScene(std::unique_ptr<SceneBase> scene);
	
	/// <summary>
	/// フォグの開始位置と終了位置を設定
	/// </summary>
	/// <param name="fogStart">フォグの開始位置</param>
	/// <param name="fogEnd">フォグの終了位置</param>
	void SetFog(const float fogStart,const float fogEnd);

	/// <summary>
	/// フォグの設定をリセット
	/// </summary>
	void ResetFog(void);

	/// <summary>
	/// 画面揺らし開始
	/// </summary>
	void StartShakeScreen(void);

	/// <summary>
	/// メインスクリーン取得
	/// </summary>
	/// <returns>メインスクリーンのハンドル</returns>
	int GetMainScreen(void) const { return mainScreen_; }

	/// <summary>
	/// フェードアウトが終わったかどうか
	/// </summary>
	/// <returns>true:終了 false:フェードアウト中</returns>
	bool IsFadeOutEnd(void);

	/// <summary>
	/// フェードイン終わったかどうか
	/// </summary>
	/// <returns>true:終了 false:フェードイン中</returns>
	bool IsFadeInEnd(void);

	/// <summary>
	/// 画面比率を取得する
	/// </summary>
	/// <returns>画面の比率</returns>
	const float GetScreenAspectRatio(void) const;

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
	std::shared_ptr<Fader> fader_;

	// カメラ
	std::shared_ptr<Camera> camera_;

	// シーン遷移中判定
	bool isSceneChanging_;

	// デルタタイム
	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;

	//フォグ
	float fogStart_;	//開始位置
	float fogEnd_;		//終了位置

	//ライトの方向
	VECTOR lightDir_;

	//シーンのレンダーターゲット
	int mainScreen_;

	//画面揺らし用
	int shakeFrame_;	//揺らしフレーム数
	float shakeRate_;	//揺らし強度

	std::string sceneName_;

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

	/// <summary>
	/// 画面揺らしの処理
	/// </summary>
	void ShakeScreen(void);

	//SCENE_IDからシーンを生成する
	template<typename T = SceneBase>
	std::unique_ptr<T> CreateScene(SCENE_ID sceneId);

	/// <summary>
	/// SceneごとのImGui更新処理
	/// </summary>
	void SceneUpdateImGui(void);
};
