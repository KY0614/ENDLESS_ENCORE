// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

//PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    float4 color;
    color = diffuseMapTexture.Sample(diffuseMapSampler, PSInput.uv);
    if (color.a < 0.01f)
    {
        discard; // アルファ値が0のピクセルは破棄)
    }
    
    color *= float4(1.0f, 1.0f, 1.0f,1.0f); // 定数バッファの色を乗算
    
    float3 normal = PSInput.normal;
    float3 rgb = (color.rgb/* * g_color.rgb*/);

    return float4(rgb, color.a);
}