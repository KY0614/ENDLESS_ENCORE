#pragma once
#include <string>
#include "UIBase.h"

class PopUpUI : public UIBase
{
public:
    static constexpr float ANIM_DURATION = 1.0f; // アニメーション時間（秒）
    static constexpr float MOVE_DISTANCE = 1.0f; // 上に移動する距離（ピクセル）

    PopUpUI(int score, const VECTOR& startPos);
    virtual ~PopUpUI() = default;

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

private:
    int score_;
    float animTime_;

    // スコアの数字画像ハンドル
    int* numbersImgs_;

    // 描画用のメソッド
    void DrawScore(int score, const VECTOR& pos, float alpha);
};

