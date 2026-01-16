#pragma once
#include <DxLib.h>
#include "../../Manager/Generic/ResourceManager.h"

class UIBase
{
public:
	//コンストラクタ
    UIBase(void);
    //デストラクタ
    virtual ~UIBase() = default;

    /// <summary>
    /// 初期化処理
    /// </summary>
    virtual void Init(void) = 0;

    /// <summary>
    /// 更新処理
    /// </summary>  
    virtual void Update(void) = 0;

    /// <summary>
    /// 描画処理
    /// </summary>  
    virtual void Draw(void) = 0;

	/// <summary>
	/// アルファ値を設定する
	/// </summary>
	/// <param name="alpha">アルファ値（0.0f～1.0f)</param>
	void SetAlpha(const float alpha) { alpha_ = alpha; }

    /// <summary>
	/// 表示、非表示を設定する
    /// </summary>
    /// <param name="isActive">true:表示、false:非表示</param>
    void SetActive(const bool isActive) { isActive_ = isActive; }

protected:
    //UI画像のリソースID
	ResourceManager::SRC uiSrc_; 
	ResourceManager::SRC uiBackSrc_; 

    //UI画像ハンドル
	int uiImg_;     //メイン画像
	int uiBackImg_; //背景画像

    //UI画像の透明度
    float alpha_;

	//表示、非表示用のフラグ
    bool isActive_;
};

