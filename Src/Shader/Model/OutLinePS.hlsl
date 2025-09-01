// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

//PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
    float4 g_outline_color; //定数バッファの色
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    return float4(g_outline_color); // 黒アウトライン
}