#pragma once
#include <functional>
#include <map>
#include <vector>
#include <string>
#include "../Common/Vector2.h"

class Tutorial
{
public:
	//チュートリアルの種類
	enum class STATE
	{
		NONE,		//なし
		MOVE,		//移動
		CAMERA,		//カメラ操作
		DASH,		//ダッシュ
		JUMP,		//ジャンプ
		DODGE,		//回避
		PARRY,		//パリィ
	};

	//チュートリアルステップ構造体
	struct TutorialStep
	{
		STATE state_;			//チュートリアルの種類
		Vector2 pos_;			//チュートリアル表示位置
		std::string keyGuide_;	//キーガイドの文字列
		std::string controllerGuide_; //コントローラーガイドの文字列
		float requiredTime_;	//クリアに必要な時間
		int requiredNum_;		//クリアに必要な回数
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="firstStep">最初に表示するチュートリアル</param>
	Tutorial(const TutorialStep& firstStep);
	//デストラクタ
	~Tutorial(void);

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

	/// <summary>
	/// チュートリアルステップ追加
	/// </summary>
	/// <param name="tutorialStep">追加するチュートリアル(２個目以降に表示するもの)</param>
	void AddTutorialStep(const TutorialStep& tutorialStep);

	void UpdateImGui(void);
private:
	//現在のチュートリアルタイプ
	STATE state_; 

	//状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//チュートリアルステップリスト
	std::vector<TutorialStep> step_;

	std::string viewGuide_;

	/// <summary>
	/// 次のステップへ進む
	/// </summary>
	void NextStep(void);

	//状態遷移--------------------------------------------------------

	/// <summary>
	/// 状態変更
	/// </summary>
	/// <param name="state">遷移したい状態</param>
	void ChangeState(STATE state);

	/// <summary>
	/// 状態遷移：NONE
	/// </summary>
	void ChangeStateNone(void);

	/// <summary>
	/// 状態遷移：MOVE
	/// </summary>
	void ChangeStateMove(void);

	/// <summary>
	/// 状態遷移：CAMERA
	/// </summary>
	void ChangeStateCamera(void);

	/// <summary>
	/// 状態遷移：DASH
	/// </summary>
	void ChangeStateDash(void);

	/// <summary>
	/// 状態遷移：JUMP
	/// </summary>
	void ChangeStateJump(void);

	/// <summary>
	/// 状態遷移：DODGE
	/// </summary>
	void ChangeStateDodge(void);

	/// <summary>
	/// 状態遷移：PARRY
	/// </summary>
	void ChangeStateParry(void);

	//更新ステップ--------------------------------------------------------

	/// <summary>
	/// 更新：NONE
	/// </summary>
	void UpdateNone(void);

	/// <summary>
	/// 更新：MOVE
	/// </summary>
	void UpdateMove(void);

	/// <summary>
	/// 更新：CAMERA
	/// </summary>
	void UpdateCamera(void);

	/// <summary>
	/// 更新：DASH
	/// </summary>
	void UpdateDash(void);

	/// <summary>
	/// 更新：JUMP
	/// </summary>
	void UpdateJump(void);

	/// <summary>
	/// 更新：DODGE
	/// </summary>
	void UpdateDodge(void);

	/// <summary>
	/// 更新：PARRY
	/// </summary>
	void UpdateParry(void);

};