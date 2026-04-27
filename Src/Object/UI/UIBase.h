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
	/// 表示、非表示を設定する
    /// </summary>
    /// <param name="isActive">true:表示、false:非表示</param>
    void SetActive(const bool isActive) { isActive_ = isActive; }

protected:

    //UI画像ハンドル
	int uiImg_;

	//表示、非表示用のフラグ
    bool isActive_;
};

