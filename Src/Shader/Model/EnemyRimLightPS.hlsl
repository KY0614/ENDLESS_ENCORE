// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

//PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
    float4 g_color;         //定数バッファの色
    
    float3 g_light_dir;     //ライトの方向
    float dummy;
    
    float4 g_ambient_color; // 環境光の色
    
    float4 g_fog_color;     //霧の色
    
    float3 g_camera_pos;    //カメラの位置
    float dummy2;          //ダミー
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float4 color;
    color = diffuseMapTexture.Sample(diffuseMapSampler, PSInput.uv);
    if (color.a < 0.01f) discard; // アルファ値が0のピクセルは破棄)
    
    color *= g_color; //定数バッファの色を乗算
    
    float3 normal = PSInput.normal;
    //ライト
    float lihgt = dot(normal, -g_light_dir);
    //霧
    float fogFactor = PSInput.fogFactor;
    float3 fogCol = g_fog_color.rgb;
    
    //リムライト
    float3 toCamera = normalize(g_camera_pos - PSInput.worldPos);
    float cDot = abs(dot(PSInput.normal, toCamera));
    cDot = 1.0f - cDot;
    float rimLightPow = 3.0f; //リムライトの強さを調整するためのパワー値
    float rimDot = pow(cDot, rimLightPow); //強めのリムライトを設定
    float4 rimColor = float4(1.0f, 0.0f, 0.0f, 1.0f) * rimDot; // 赤リムライト
    
    float3 rgb = (color.rgb * g_color.rgb) + rimColor.rgb;
    float3 finalColor = lerp(fogCol, rgb, fogFactor);
    return float4(rgb, color.a);
}