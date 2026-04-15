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
    float dummy;   
    
    float4 g_ambient_color; // 環境光の色
    
    float4 g_fog_color;
    
    float3 g_pointlight_pos;
    float g_pointlight_range; //光の最大距離
    
    //スポットライト
    float3 g_spotlight_pos;
    float g_spotlight_range; //光の最大距離
    
    float3 g_spotlight_dir; //ライトの方向
    float g_spotlight_attenuation;

}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float4 color;
    color = diffuseMapTexture.Sample(diffuseMapSampler, PSInput.uv);
    if (color.a < 0.01f)
        discard; // アルファ値が0のピクセルは破棄)
    
    color *= g_color; // 定数バッファの色を乗算
    
    float3 normal = PSInput.normal;
    //ライト
    float lihgt = dot(normal, -g_light_dir);
    //ポイントライト
    //距離
    float pointDis = length(PSInput.worldPos - g_pointlight_pos);
    //ライトの色
    float4 pointLightCol = float4(1.0f, 1.0f, 1.0f, 0.5f);
    //影響力を計算
    float pointLightAtten = 1.0f - 1.0f / g_pointlight_range * pointDis;
    //影響力がマイナスなら0にする
    if (pointLightAtten < 0.0f)
    {
        pointLightAtten = 0.0f;
    }
    //2乗して減衰を表現
    pointLightAtten = pow(pointLightAtten, 2.0f);
    //ライトの色に影響力をかける
    pointLightCol.rgb = (pointLightCol.rgb * saturate(pointLightAtten));
    
    //スポットライト---------------------------------------------------------

    float fogFactor = PSInput.fogFactor;
    float3 fogCol = g_fog_color.rgb;
    float3 rgb = (color.rgb * g_color.rgb * lihgt) + g_ambient_color.rgb + pointLightCol.rgb;
    float3 finalColor = lerp(fogCol, rgb, fogFactor);
    return float4(finalColor, color.a);
    
    
}