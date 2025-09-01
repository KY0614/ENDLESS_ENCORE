// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

//PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
    float4 g_color;
    
    float3 g_light_dir;
    float dummy; // float4のサイズに合わせるためのダミー変数
    
    float4 g_ambient_color; // 環境光の色
    
    float3 g_camera_pos; // カメラの位置
    float dummy2; // float4のサイズに合わせるためのダミー変数
    
    float4 g_specular_color; // スペキュラの色
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
    float lihgt = dot(PSInput.normal, -g_light_dir);

    float3 toCamera = normalize(g_camera_pos - PSInput.worldPos);
    float cDot = abs(dot(PSInput.normal, toCamera));
    cDot = 1.0f - cDot;
    float rimDot = pow(cDot, 2.0f); // 強弱を強め
    float4 rimColor = float4(0.0f, 1.0f, 0.0f, 1.0f) * rimDot; // 緑リムライト
    
    // 色の合成
    float3 rgb = (color.rgb * g_color.rgb * lihgt) + rimColor.rgb
     + g_ambient_color.rgb;
    
    return float4(rgb, color.a);

}