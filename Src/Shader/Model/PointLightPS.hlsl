// VS/PS‹¤’Ê
#include "../Common/VertexToPixelHeader.hlsli"

// IN
#define PS_INPUT VertexToPixelLit

//PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

float4 main(PS_INPUT PSInput) : SV_TARGET
{
    return float4(1.0f, 1.0f, 0.1f, 1.0f);
}