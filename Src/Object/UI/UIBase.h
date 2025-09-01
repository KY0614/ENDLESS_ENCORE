#pragma once
#include <DxLib.h>

class UIBase
{
public:

    UIBase(void);
    virtual ~UIBase() = default;

    virtual void Init(void) = 0;
    virtual void Update(void) = 0;
    virtual void Draw(void) = 0;

    void SetPos(const VECTOR pos) { pos_ = pos; }
	void SetAlpha(const float alpha) { alpha_ = alpha; }
    void SetActive(const bool isActive) { isActive_ = isActive; }

protected:

    //UI画像ハンドル
    int uiImg_;

    //UI座標
    VECTOR pos_;

    //UI画像の透明度
    float alpha_;

	//表示、非表示用のフラグ
    bool isActive_;
};

