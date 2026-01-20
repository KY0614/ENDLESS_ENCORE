#pragma once
#include <functional>
#include <map>
#include <vector>
#include <string>

class Tutorial
{
public:
	//チュートリアルの種類
	enum class TUTORIAL_TYPE
	{
		MOVE,		//移動
		CAMERA,		//カメラ操作
		JUMP,		//ジャンプ
		DODGE,		//回避
		PARRY,		//パリィ
	};

	struct TutorialStep
	{
		TUTORIAL_TYPE type;		//チュートリアルの種類
		std::string keyGuide_;	//キーガイドの文字列
		std::string controllerGuide_; //コントローラーガイドの文字列
		float requiredTime_;	//クリアに必要な時間
		int requiredNum_;		//クリアに必要な回数
		bool isCompleted;		//クリアしたかどうか
	};

	//コンストラクタ
	Tutorial(void);
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

	void SetTutorialOrder(const std::vector<TUTORIAL_TYPE> order);

private:
	//現在のチュートリアルタイプ
	TUTORIAL_TYPE tutorialType_; 

	//状態管理(状態遷移時初期処理)
	std::map<TUTORIAL_TYPE, std::function<void(void)>> stateChanges_;

	//状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	//チュートリアル
	std::vector<TUTORIAL_TYPE> tutorialStep_;

	//現在のチュートリアルインデックス
	int currentTutorialIndex_; 

	void ChangeState(TUTORIAL_TYPE type);

	void ChangeStateMove(void);
	void ChangeStateCamera(void);
	void ChangeStateJump(void);
	void ChangeStateDodge(void);
	void ChangeStateParry(void);

	void UpdateMove(void);
	void UpdateCamera(void);
	void UpdateJump(void);
	void UpdateJump(void);
	void UpdateDodge(void);
	void UpdateParry(void);
};