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
 
    float3 refDir = normalize(g_light_dir + (PSInput.normal * 2.0f));
    float3 refEye = g_camera_pos - PSInput.worldPos;
    float3 refEyeDir = normalize(refEye); // カメラ位置からのベクトルを正規化
    float refDot = abs(dot(refDir, refEyeDir));
    float g_specular_pow = 40.0f;
    float refDotEx = pow(refDot, g_specular_pow);
    
    // 色の合成
    float3 rgb = (color.rgb * g_color.rgb * lihgt)
    + (refDotEx * g_specular_color.rgb) + g_ambient_color.rgb;
    
    return float4(rgb, color.a);

}