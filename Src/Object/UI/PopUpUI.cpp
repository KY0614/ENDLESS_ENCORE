#include "../Manager/Generic/ResourceManager.h"
#include "PopUpUI.h"

PopUpUI::PopUpUI(int score, const VECTOR& startPos) : score_(score), animTime_(0.0f)
{
    pos_ = startPos;
    alpha_ = 255.0f; // 初期透明度を最大に設定
    isActive_ = true;
    numbersImgs_ = nullptr;
}

void PopUpUI::Init(void)
{
    numbersImgs_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::SCORE_NUMBER).handleIds_;
}

void PopUpUI::Update(void)
{
    if (!isActive_) return;

    animTime_ += 1.0f / 60.0f; // 60FPSを想定した経過時間更新
    if (animTime_ >= ANIM_DURATION)
    {
        isActive_ = false; // アニメーション終了
        return;
    }

    // イージング関数を使用して位置と透明度を計算
    float t = animTime_ / ANIM_DURATION;
    pos_.y = pos_.y - t * MOVE_DISTANCE; // 上に移動
    alpha_ = 255.0f * (1.0f - t); // フェードアウト

    SetAlpha(alpha_);
}

void PopUpUI::Draw(void)
{
    if (!isActive_) return;

    DrawScore(score_, pos_, alpha_);
}

void PopUpUI::DrawScore(int score, const VECTOR& pos, float alpha)
{
    std::string str = std::to_string(score);
    const int digitWidth = 35;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(alpha));

    for (int i = 0; i < str.size(); ++i)
    {
        char ch = str[i];
        if ('0' <= ch && ch <= '9')
        {
            int digit = ch - '0';
            DrawRotaGraph(
                pos.x - (str.size() * digitWidth / 2) + i * digitWidth,
                pos.y,
                0.4f, 0.0f,
                numbersImgs_[digit], true
            );
        }
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); // 描画モードを元に戻す
}
