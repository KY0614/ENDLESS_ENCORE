#pragma once

class FpsControl
{
public:
    static constexpr float FRAME_RATE = (1000 / 60);
    static constexpr int ONE_FRAME_TIME = 1000;

    FpsControl();

    void Init(void);

    bool UpdateFrameRate(void);

    void CalcFrameRate(void);

    void DrawFrameRate(void);

private:
    int currentTime_;   //現在の時間
    int prevFrmTime_;   //前回のフレーム実行時の時間

    int frmCnt_;      //フレームカウント用
    int updatreFrmRateTime_;    //フレームレートを更新した時間

    float frmRate_;     //フレームレート

};