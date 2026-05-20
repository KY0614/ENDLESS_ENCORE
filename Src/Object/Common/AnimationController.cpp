#include "../../Utility/StringUtility.h"
#include "../../Utility/CommonUtility.h"
#include "../../Manager/Generic/SceneManager.h"
#include "AnimationController.h"

AnimationController::AnimationController(int modelId)
{
	modelId_ = modelId;
	deltaTime_ = 0.0f;
	playType_ = -1;
	isLoop_ = false;

	isStop_ = false;
	switchLoopReverse_ = 0.0f;
	endLoopSpeed_ = 0.0f;
	stepEndLoopStart_ = 0.0f;
	stepEndLoopEnd_ = 0.0f;
	blendAnimRate_ = 0.0f;
	blendAnimTime_ = 0.0f;
	animations_.clear();
	playAnimations_.clear();
}

AnimationController::~AnimationController(void)
{
}

void AnimationController::Add(int type, const std::string& path, float speed)
{

	Animation anim;
	anim.model = MV1LoadModel(StringUtility::String2Wstring(path).c_str());
	anim.animIndex = type;
	anim.speed = speed;

	if (animations_.count(type) == 0)
	{
		//入れ替え
		animations_.emplace(type, anim);
	}
	else
	{
		//追加
		animations_[type].model = anim.model;
		animations_[type].animIndex = anim.animIndex;
		animations_[type].attachNo = anim.attachNo;
		animations_[type].totalTime = anim.totalTime;
	}
}

void AnimationController::Play(int type, bool isLoop,
	float startStep, float endStep, bool isStop, bool isForce)
{
	//同じ種類かつ強制再生を行わない場合
	if (type == playType_ && !isForce)
	{
		return;
	}

	if (playType_ != -1)
	{
		//モデルからアニメーションを外す
		playAnim_.attachNo = MV1DetachAnim(modelId_, playAnim_.attachNo);
	}

	//アニメーション種別を変更
	playType_ = type;
	playAnim_ = animations_[type];

	//初期化
	playAnim_.step = startStep;

	//モデルにアニメーションを付ける
	int animIdx = 0;
	if (MV1GetAnimNum(playAnim_.model) > 1)
	{
		//アニメーションが複数保存されていたら、番号1を指定
		animIdx = 1;
	}
	playAnim_.attachNo = MV1AttachAnim(modelId_, animIdx, playAnim_.model);

	//アニメーション総時間の取得
	if (endStep > 0.0f)
	{
		playAnim_.totalTime = endStep;
	}
	else
	{
		playAnim_.totalTime = MV1GetAttachAnimTotalTime(modelId_, playAnim_.attachNo);
	}

	//アニメーションループ
	isLoop_ = isLoop;

	//アニメーションしない
	isStop_ = isStop;

	stepEndLoopStart_ = -1.0f;
	stepEndLoopEnd_ = -1.0f;
	switchLoopReverse_ = 1.0f;
}

void AnimationController::Update(void)
{
	//経過時間の取得
	float deltaTime = SceneManager::GetInstance().GetDeltaTime();

	if (!isStop_)
	{
		//再生
		playAnim_.step += (deltaTime * playAnim_.speed * switchLoopReverse_);

		//アニメーション終了判定
		bool isEnd = false;
		if (switchLoopReverse_ > 0.0f)
		{
			//通常再生の場合
			if (playAnim_.step > playAnim_.totalTime)
			{
				isEnd = true;
			}
		}
		else
		{
			//逆再生の場合
			if (playAnim_.step < playAnim_.totalTime)
			{
				isEnd = true;
			}
		}

		if (isEnd)
		{
			//アニメーションが終了したら
			if (isLoop_)
			{
				//ループ再生
				if (stepEndLoopStart_ > 0.0f)
				{
					//アニメーション終了後の指定フレーム再生
					switchLoopReverse_ *= -1.0f;
					if (switchLoopReverse_ > 0.0f)
					{
						playAnim_.step = stepEndLoopStart_;
						playAnim_.totalTime = stepEndLoopEnd_;
					}
					else
					{
						playAnim_.step = stepEndLoopEnd_;
						playAnim_.totalTime = stepEndLoopStart_;
					}
					playAnim_.speed = endLoopSpeed_;
					
				}
				else
				{
					//通常のループ再生
					playAnim_.step = 0.0f;
				}
			}
			else
			{
				//ループしない
				playAnim_.step = playAnim_.totalTime;
			}
		}
	}

	//アニメーション設定
	MV1SetAttachAnimTime(modelId_, playAnim_.attachNo, playAnim_.step);

	//停止してる場合
	if (isStop_)
	{
		//処理は実行しない
		return;
	}

	deltaTime_ = SceneManager::GetInstance().GetDeltaTime();
	// メインアニメーション更新
	UpdateMainAnimation();

	// ブレンドアニメーション更新
	UpdateBlendAnimation();
}

void AnimationController::SetEndLoop(float startStep, float endStep, float speed)
{
	stepEndLoopStart_ = startStep;
	stepEndLoopEnd_ = endStep;
	endLoopSpeed_ = speed;
}

int AnimationController::GetPlayType(void) const
{
	return playType_;
}

bool AnimationController::IsEnd(void) const
{

	bool ret = false;

	if (isLoop_)
	{
		//ループ設定されているなら、
		//無条件で終了しないを返す
		return ret;
	}

	if (playAnim_.step >= playAnim_.totalTime)
	{
		//再生時間を過ぎたらtrue
		return true;
	}

	return ret;

}

void AnimationController::UpdateMainAnimation()
{
	//再生中のアニメーション情報を取得
	Animation& playAnim = playAnimations_[playType_];

	//再生
	playAnim.step += (deltaTime_ * playAnim.speed * switchLoopReverse_);

	// アニメーション終了判定
	bool isEnd = false;
	if (switchLoopReverse_ > 0.0f)
	{
		// 通常再生の場合
		if (playAnim.step > playAnim.totalTime)
		{
			isEnd = true;
		}
	}
	else
	{
		// 逆再生の場合
		if (playAnim.step < playAnim.totalTime)
		{
			isEnd = true;
		}
	}

	if (isEnd)
	{
		// アニメーションが終了したら
		if (isLoop_)
		{
			// ループ再生
			if (stepEndLoopStart_ > 0.0f)
			{
				// アニメーション終了後の指定フレーム再生
				switchLoopReverse_ *= -1.0f;
				if (switchLoopReverse_ > 0.0f)
				{
					playAnim.step = stepEndLoopStart_;
					playAnim.totalTime = stepEndLoopEnd_;
				}
				else
				{
					playAnim.step = stepEndLoopEnd_;
					playAnim.totalTime = stepEndLoopStart_;
				}
				playAnim.speed = endLoopSpeed_;
			}
			else
			{
				// 通常のループ再生
				playAnim.step = 0.0f;
			}
		}
		else
		{
			// ループしない
			playAnim.step = playAnim.totalTime;
		}
	}

	// 再生するアニメーション時間の設定
	MV1SetAttachAnimTime(modelId_, playAnim.attachNo, playAnim.step);
}

void AnimationController::UpdateBlendAnimation()
{
	// 再生中のアニメーションが一定以上登録されているなら
	if (static_cast<int>(playAnimations_.size()) <= 1)
	{
		return;
	}

	//ブレンドアニメーション率を増加
	blendAnimRate_ += deltaTime_;

	if (blendAnimRate_ >= blendAnimTime_)
	{
		blendAnimRate_ = blendAnimTime_;
	}

	//ブレンド進行率を計算
	float blendRate = blendAnimRate_ / blendAnimTime_;

	// 登録されているブレンドアニメーション率を更新
	for (auto it = playAnimations_.begin(); it != playAnimations_.end(); )
	{
		//変更後のアニメーションの場合
		if (it->first == playType_)
		{
			//ブレンドアニメーション率を増加
			it->second.blendRate += (1.0f - it->second.blendRate) * blendRate;

			//アニメーションのアタッチ
			MV1SetAttachAnimBlendRate(modelId_, it->second.attachNo, it->second.blendRate);
		}
		//変更前のアニメーションの場合
		else
		{
			//ブレンドアニメーション率を減少
			it->second.blendRate -= it->second.blendRate * blendRate;

			// ブレンドアニメーション率が0以下になったら
			if (it->second.blendRate <= 0.0f)
			{
				//アニメーションのデタッチ
				it->second.attachNo = MV1DetachAnim(modelId_, it->second.attachNo);

				// ブレンドアニメーション率が0以下になったら、リストから削除
				it = playAnimations_.erase(it);
				continue;
			}
			//アニメーションのアタッチ
			MV1SetAttachAnimBlendRate(modelId_, it->second.attachNo, it->second.blendRate);
		}
		++it;
	}
}
