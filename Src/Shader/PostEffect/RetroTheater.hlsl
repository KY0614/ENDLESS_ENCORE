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
        dstCol = (0.0f, 0.0f, 0.0f);
    }
    
    //シミノイズ
    float noiseScale = 0.03f;
    float2 localUV = (PSInput.uv - g_noise_uv) / noiseScale + 0.5f;
    // 画像の範囲内だけ処理
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
    sepia.rgb += (g_sepia_white_pow, g_sepia_white_pow, g_sepia_white_pow);
    dstCol.rgb = lerp(dstCol.rbg, sepia, 1.0f);
    
    //フィルムのパーフォレーション（送り穴、画面端の白黒のやつ）----------
    //画面端のフィルムの幅（黒いやつ)
    float filmWidth = 0.12f;
    //左側の範囲
    float leftBand = step(PSInput.uv.x, filmWidth);
    //右側の範囲
    float rightBand = step(1.0f - filmWidth, PSInput.uv.x);
    //左右どちらかの範囲に入っているかどうか
    float bandMask = max(leftBand, rightBand);

    //白い四角形（フィルム穴）の範囲
    float holeWidth = 0.08f;
    //フィルムと穴と間
    float filmHoleSpace = 0.02f;
    float hole = step(holeWidth, 0.05f);
    //穴の範囲を決めるため範囲の(左側の)
    float leftRect = step(filmHoleSpace, PSInput.uv.x) * step(PSInput.uv.x, filmWidth - filmHoleSpace);
    float rightRect = step(0.9f, PSInput.uv.x) * step(PSInput.uv.x, 0.98f);
    float yRect = step(0.4f, PSInput.uv.y) * step(PSInput.uv.y, 0.5f);
    float rectMask = (leftRect + rightRect) * yRect;

    // 黒帯と白四角の合成
    float3 bandColor = float3(0.0f, 0.0f, 0.0f);
    float3 rectColor = float3(1.0f, 1.0f, 1.0f);
    float3 perfColor = lerp(bandColor, rectColor, rectMask);
    dstCol.rgb = lerp(dstCol.rgb, perfColor, bandMask);

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