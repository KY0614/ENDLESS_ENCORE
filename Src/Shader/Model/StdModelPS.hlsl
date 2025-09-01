// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

//PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
    float4 g_color;     //定数バッファの色
    
    float3 g_light_dir; //ライトの方向
    float dummy; // float4のサイズに合わせるためのダミー変数
    
    float4 g_ambient_color; // 環境光の色
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float4 color;
    color = diffuseMapTexture.Sample(diffuseMapSampler, PSInput.uv);
    if (color.a < 0.01f)
    {
        discard; // アルファ値が0のピクセルは破棄)
    }
    
    color *= g_color; // 定数バッファの色を乗算
    
    float3 normal = PSInput.normal;
    float lihgt = dot(normal, -g_light_dir);
    float3 rgb = (color.rgb * g_color.rgb * lihgt) + g_ambient_color.rgb;

    return float4(rgb, color.a);
    
}