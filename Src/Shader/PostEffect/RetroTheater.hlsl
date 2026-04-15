#include "../Common/Pixel/PixelShader2DHeader.hlsli"

// 定数バッファ：スロット4番目(b4と書く)
cbuffer cbParam : register(b4)
{
    float4 g_color;         //モデルカラー
    
    float g_line_uv_x;      //黒線ノイズのUV座標
    float2 g_noise_uv;      //シミノイズのUV座標
    float g_sepia_white_pow;//セピア調に加算する白味
    
    float g_vine_pow;       //ビネットの強度
    float g_film_scroll;    //フィルムスクロール
    float2 dummy;           //16バイトアライメント用ダミー    
}

float4 main(PS_INPUT PSInput) : SV_TARGET
{
	// UV座標とテクスチャを参照して、最適な色を取得する
    float4 srcCol = tex.Sample(texSampler, PSInput.uv);
    if (srcCol.a < 0.01f)
    {
		// 描画しない(アルファテスト)
        discard;
    }

    float4 dstCol = srcCol;
   
    //黒線ノイズ----------------------------------------------------------
    float diffNoiseLine = abs(PSInput.uv.x - g_line_uv_x);
    if (diffNoiseLine < 0.001f)
    {
        dstCol.rgb = float3(0.0f, 0.0f, 0.0f);
    }
    
    //シミノイズ-----------------------------------------------------------
    float noiseScale = 0.03f;   //ノイズの拡大率
    float uvCenter = 0.5f;      //ノイズの中心位置
    //ノイズ用UVを計算
    float2 localUV = (PSInput.uv - g_noise_uv) / noiseScale + uvCenter;
    //localUVが0.0～1.0fの範囲内かチェックし、
    //範囲内の場合のみノイズを合成する
    if (localUV.x >= 0.0f && localUV.x <= 1.0f && localUV.y >= 0.0f && localUV.y <= 1.0f)
    {
        //ノイズ用の画像
        float4 noiseCol = noiseTexture.Sample(texSampler, localUV);
        //シミノイズを合成
        dstCol.rgb = lerp(dstCol.rgb, noiseCol.rgb, noiseCol.a);
    }
   
    //セピア----------------------------------------------------------
    dstCol.rgb *= g_color.rgb;
    float3 sepia = dstCol.rgb;
    sepia.r = dot(dstCol.rgb, float3(0.393f, 0.769f, 0.189f));
    sepia.g = dot(dstCol.rgb, float3(0.349f, 0.686f, 0.168f));
    sepia.b = dot(dstCol.rgb, float3(0.272f, 0.534f, 0.131f));
    //白の強さ加算
    sepia.rgb += float3(g_sepia_white_pow, g_sepia_white_pow, g_sepia_white_pow);
    dstCol.rgb = lerp(dstCol.rgb, sepia, 1.0f);
    
    //フィルムのパーフォレーション（送り穴、画面端の白黒のやつ）----------
    float filmWidth = 0.12f;    //画面端のフィルムの幅（黒いやつ)
    float holeWidth = 0.08f;    //穴の横幅
    float holeHeight = 0.45f;   //穴の縦幅
    float holeMarginX = 0.02f;  //穴の左右マージン
    float holeMarginY = 0.005f; //穴の上下マージン
    float holePitch = 0.18f;    //穴の縦方向ピッチ
    //左側の範囲
    float sideFilm = step(PSInput.uv.x, filmWidth) + step(1.0f - filmWidth, PSInput.uv.x);
    //最大値を1.0に固定
    sideFilm = saturate(sideFilm);
    //穴のY座標を周期的に配置
    float uvY = frac((PSInput.uv.y + g_film_scroll) / holePitch);
    float holeY = step(holeMarginY, uvY) * step(uvY, holeMarginY + holeHeight);
    //左右の穴範囲
    float leftHole = step(holeMarginX, PSInput.uv.x) * step(PSInput.uv.x, holeMarginX + holeWidth);
    float rightHole = step(1.0f - holeMarginX - holeWidth, PSInput.uv.x) * step(PSInput.uv.x, 1.0f - holeMarginX);
    //穴の合成
    float holeMask = (leftHole + rightHole) * holeY;
    holeMask = saturate(holeMask);
    //黒帯→白穴の合成
    float3 filmColor = lerp(float3(0.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 1.0f), holeMask);
    dstCol.rgb = lerp(dstCol.rgb, filmColor, sideFilm);
    
    //ビネット----------------------------------------------------------
    float2 uv = PSInput.uv;
    float x = abs(uv.x - 0.5f); // 0.0～1.0f → -0.5～0.5 → 0.0～0.5
    float y = abs(uv.y - 0.5f); // 0.0～1.0f → -0.5～0.5 → 0.0～0.5
    float dis = (x * x + y * y);

	//ビネットの強度を計算
    float vignette = 1.0f - saturate(dis * g_vine_pow);

    //色にビネットを乗算
    dstCol.rgb *= vignette;
    
    return dstCol;
}