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
    float g_scroll_time;
    
    //float4 g_ambient_color; // 環境光の色
    
    //float3 g_camera_pos; // カメラの位置
    //float dummy; // float4のサイズに合わせるためのダミー変数
    
    float g_alpha_line; //透明化する線
    float g_alpha_range; //輪郭線を光らせる範囲
    float2 dummy;
    
    float4 g_alphaline_color; //輪郭線の色
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float4 color;
    
    //波
    float2 uv = PSInput.uv;
    float g_speed = 0.7f; //波の速さ
    float g_wave_height = 10.0f; //波の高さ
    float g_wave_frequency = 0.05f; //波の周波数
    uv.y += sin(PSInput.uv.x * g_wave_height + g_scroll_time * g_speed) * g_wave_frequency;
    uv.y += cos(PSInput.uv.x * g_wave_height + g_scroll_time * g_speed) * g_wave_frequency;
    
    //テクスチャ取得
    color = diffuseMapTexture.Sample(diffuseMapSampler, uv);
    if (color.a < 0.01f)
    {
        discard; // アルファ値が0のピクセルは破棄)
    }
    
    float noiseCol = noiseMapTexture.Sample(diffuseMapSampler, uv).r;
    // ディゾルブ処理
    if (noiseCol.x >= g_alpha_line - g_alpha_range && noiseCol.x <= g_alpha_line + g_alpha_range)
    {
        return g_alphaline_color;
    }
    else if (noiseCol.x > g_alpha_line)
    {
        discard;
    }
    
    float3 retCol = saturate((color.xyz * g_color.xyz) * (1.0 - g_alpha_line)) + float3(0.5f, 0.5f, 0.5f);
    
    return float4(retCol, saturate(color.a * g_color.a));
    //// ディゾルブの閾値計算
    //float threshold = g_dissolve + noiseCol * 0.25;
    
    //// ディゾルブとエッジグロー
    //float edge_width = 0.1f; // エッジの太さを調整する値
    //float4 edge_color = float4(1.0f, 1.0f, 0.0f, 1.0f); // オレンジ色のグロー、自由に変更可能

    //if (noiseCol < threshold)
    //{
    //    // ノイズが閾値より低いが、edge_widthの範囲内であれば、エッジの色を適用
    //    if (noiseCol > threshold - edge_width)
    //    {
    //        // edge_width内から閾値に近づくにつれて、基本色とエッジ色の間で線形補間する
    //        float edge_intensity = saturate((noiseCol - (threshold - edge_width)) / edge_width);
    //        color.rgb = lerp(edge_color.rgb, color.rgb, edge_intensity);
    //        color.a = lerp(edge_color.a, color.a, edge_intensity); // エッジのアルファも必要に応じて調整
    //    }
    //    else
    //    {
    //        discard; // エッジの外側なら完全に破棄
    //    }
    //}
    
    //color.rgb *= g_color.rgb; // 定数バッファの色を乗算
    
    //float3 normal = PSInput.normal;
    //float lihgt = dot(normal, -g_light_dir);
    
    //float3 rgb = (color.rgb * lihgt) + g_ambient_color.rgb;
    
    //return float4(rgb, color.a);
}