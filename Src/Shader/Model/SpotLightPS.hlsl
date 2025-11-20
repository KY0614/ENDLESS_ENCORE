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
        // 描画しない(アルファテスト)
        discard;
    }
    //白色スポットライト
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}