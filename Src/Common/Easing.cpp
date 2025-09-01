#include <cmath>
#include "DxLib.h"
#include "Easing.h"

// time         = 進行度
// totalTime    = 目標時間
// start        = 開始値
// end          = 目標値

// ↓BackIn BackOut BackInOutのみ
// s            = 助走量

float Easing::QuadIn(float time, float totalTime, float start, float end)
{
    end -= start;
	time /= totalTime;
	
	return end * time * time + start;
}

float Easing::QuadOut(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime;
    return -end * time * (time - 2) + start;
}

float Easing::QuadInOut(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime / 2;
    if (time < 1) return end / 2 * time * time + start;

    time = time - 1;
    return -end / 2 * (time * (time - 2) - 1) + start;
}

float Easing::CubicIn(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime;
    return end * time * time * time + start;
}

float Easing::CubicOut(float time, float totaltime, float start, float end)
{
    end -= start;
    time = time / totaltime - 1;
    return end * (time * time * time + 1) + start;
}

float Easing::CubicInOut(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime / 2;
    if (time < 1) return end / 2 * time * time * time + start;

    time = time - 2;
    return end / 2 * (time * time * time + 2) + start;
}

float Easing::QuartIn(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime;
    return end * time * time * time * time + start;
}

float Easing::QuartOut(float time, float totaltime, float start, float end)
{
    end -= start;
    time = time / totaltime - 1;
    return -end * (time * time * time * time - 1) + start;
}

float Easing::QuartInOut(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime / 2;
    if (time < 1) return end / 2 * time * time * time * time + start;

    time = time - 2;
    return -end / 2 * (time * time * time * time - 2) + start;
}

float Easing::QuintIn(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime;
    return end * time * time * time * time * time + start;
}

float Easing::QuintOut(float time, float totaltime, float start, float end)
{
    end -= start;
    time = time / totaltime - 1;
    return end * (time * time * time * time * time + 1) + start;
}

float Easing::QuintInOut(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime / 2;
    if (time < 1) return end / 2 * time * time * time * time * time + start;

    time = time - 2;
    return end / 2 * (time * time * time * time * time + 2) + start;
}

float Easing::SineIn(float time, float totaltime, float start, float end)
{
    end -= start;
    return -end * cos(time * (DX_PI_F * 90 / 180) / totaltime) + end + start;
}

float Easing::SineOut(float time, float totaltime, float start, float end)
{
    end -= start;
    return end * sin(time * (DX_PI_F * 90 / 180) / totaltime) + start;
}

float Easing::SineInOut(float time, float totaltime, float start, float end)
{
    end -= start;
    return -end / 2 * (cos(time * DX_PI_F / totaltime) - 1) + start;
}

float Easing::ExpIn(float time, float totaltime, float start, float end)
{
    end -= start;
    return time == 0.0f ? start : end * powf(2, 10 * (time / totaltime - 1)) + start;
}

float Easing::ExpOut(float time, float totaltime, float start, float end)
{
    end -= start;
    return time == totaltime ? end + start : end * (-powf(2, -10 * time / totaltime) + 1) + start;
}

float Easing::ExpInOut(float time, float totaltime, float start, float end)
{
    if (time == 0.0f) return start;
    if (time == totaltime) return end;
    end -= start;
    time /= totaltime / 2;

    if (time < 1) return end / 2 * powf(2, 10 * (time - 1)) + start;

    time = time - 1;
    return end / 2 * (-powf(2, -10 * time) + 2) + start;

}

float Easing::CircIn(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime;
    return -end * (sqrt(1 - time * time) - 1) + start;
}

float Easing::CircOut(float time, float totaltime, float start, float end)
{
    end -= start;
    time = time / totaltime - 1;
    return end * sqrt(1 - time * time) + start;
}

float Easing::CircInOut(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime / 2;
    if (time < 1) return -end / 2 * (sqrt(1 - time * time) - 1) + start;

    time = time - 2;
    return end / 2 * (sqrt(1 - time * time) + 1) + start;
}

float Easing::ElasticIn(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime;

    float s = 1.70158f;
    float p = totaltime * 0.3f;
    float a = end;

    if (time == 0) return start;
    if (time == 1) return start + end;

    if (a < abs(end))
    {
        a = end;
        s = p / 4;
    }
    else
    {
        s = p / (2 * DX_PI_F) * asin(end / a);
    }

    time = time - 1;
    return -(a * powf(2, 10 * time) * sin((time * totaltime - s) * (2 * DX_PI_F) / p)) + start;
}

float Easing::ElasticOut(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime;

    float s = 1.70158f;
    float p = totaltime * 0.3f; ;
    float a = end;

    if (time == 0) return start;
    if (time == 1) return start + end;

    if (a < abs(end))
    {
        a = end;
        s = p / 4;
    }
    else
    {
        s = p / (2 * DX_PI_F) * asin(end / a);
    }

    return a * powf(2, -10 * time) * sin((time * totaltime - s) * (2 * DX_PI_F) / p) + end + start;
}

float Easing::ElasticInOut(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime / 2;

    float s = 1.70158f;
    float p = totaltime * (0.3f * 1.5f);
    float a = end;

    if (time == 0) return start;
    if (time == 2) return start + end;

    if (a < abs(end))
    {
        a = end;
        s = p / 4;
    }
    else
    {
        s = p / (2 * DX_PI_F) * asin(end / a);
    }

    if (time < 1)
    {
        return -0.5f * (a * powf(2, 10 * (time -= 1)) * sin((time * totaltime - s) * (2 * DX_PI_F) / p)) + start;
    }

    time = time - 1;
    return a * powf(2, -10 * time) * sin((time * totaltime - s) * (2 * DX_PI_F) / p) * 0.5f + end + start;
}

float Easing::BackIn(float time, float totaltime, float start, float end, float s)
{
    end -= start;
    time /= totaltime;
    return end * time * time * ((s + 1) * time - s) + start;
}

float Easing::BackOut(float time, float totaltime, float start, float end, float s)
{
    end -= start;
    time = time / totaltime - 1;
    return end * (time * time * ((s + 1) * time + s) + 1) + start;
}

float Easing::BackInOut(float time, float totaltime, float start, float end, float s)
{
    end -= start;
    s *= 1.525f;
    time /= totaltime / 2;
    if (time < 1) return end / 2 * (time * time * ((s + 1) * time - s)) + start;

    time = time - 2;
    return end / 2 * (time * time * ((s + 1) * time + s) + 2) + start;
}

float Easing::BounceIn(float time, float totaltime, float start, float end)
{
    end -= start;
    return end - BounceOut(totaltime - time, totaltime, 0, end) + start;
}

float Easing::BounceOut(float time, float totaltime, float start, float end)
{
    end -= start;
    time /= totaltime;

    if (time < 1.0f / 2.75f)
    {
        return end * (7.5625f * time * time) + start;
    }
    else if (time < 2.0f / 2.75f)
    {
        time -= 1.5f / 2.75f;
        return end * (7.5625f * time * time + 0.75f) + start;
    }
    else if (time < 2.5f / 2.75f)
    {
        time -= 2.25f / 2.75f;
        return end * (7.5625f * time * time + 0.9375f) + start;
    }
    else
    {
        time -= 2.625f / 2.75f;
        return end * (7.5625f * time * time + 0.984375f) + start;
    }
}

float Easing::BounceInOut(float time, float totaltime, float start, float end)
{
    if (time < totaltime / 2)
    {
        return BounceIn(time * 2, totaltime, 0, end - start) * 0.5f + start;
    }
    else
    {
        return BounceOut(time * 2 - totaltime, totaltime, 0, end - start) * 0.5f + start + (end - start) * 0.5f;
    }
}

float Easing::Linear(float time, float totaltime, float start, float end)
{
    return (end - start) * time / totaltime + start;
}
