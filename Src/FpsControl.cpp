#include <DxLib.h>
#include "../Application.h"
#include "FpsControl.h"

FpsControl::FpsControl()
    : currentTime_(0), prevFrmTime_(0),
    frmCnt_(0), updatreFrmRateTime_(0),
    frmRate_(0.0f)
{   
}

void FpsControl::Init(void)
{
    currentTime_ = 0;
    prevFrmTime_ = 0;  

    frmCnt_ = 0;  
    updatreFrmRateTime_ = 0;  

    frmRate_ = 0.0f; 
}

bool FpsControl::UpdateFrameRate(void)
{
    Sleep(1);   //システムに処理を返す

    currentTime_ = GetNowCount();

    if (currentTime_ - prevFrmTime_ >= FRAME_RATE)
    {
        prevFrmTime_ = currentTime_;

        frmCnt_++;

        return true;
    }

    return false;
}

void FpsControl::CalcFrameRate(void)
{
    //前回のフレームレート更新からの経過時間を求める
    int nDifTime = currentTime_ - updatreFrmRateTime_;

    //前回のフレームレートを更新から
    //1秒以上経過していたらフレームレートを更新する
    if (nDifTime > ONE_FRAME_TIME)
    {
        //フレームレート回数をミリ秒に合わせる
        //少数まで出したのでfloatにする
        float fFrameCnt = (float)(frmCnt_ * ONE_FRAME_TIME);

        //フレームレートを計算
        //理想通りなら6000 / 1000で60になる
        frmRate_ = fFrameCnt / nDifTime;

        //フレームレートをクリア
        frmCnt_ = 0;

        //フレームレート更新時間を更新
        updatreFrmRateTime_ = currentTime_;
    }
}

void FpsControl::DrawFrameRate(void)
{
    DrawFormatString(0, 0, 0xFF0000, L"FPS[%.2f]", frmRate_);
}
