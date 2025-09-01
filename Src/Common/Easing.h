#pragma once

class Easing 
{
public:
static float QuadIn(float time, float totalTime, float start, float end);
static float QuadOut(float time, float totalTime, float start, float end);
static float QuadInOut(float time, float totaltime, float start, float end);
static float CubicIn(float time, float totaltime, float start, float end);
static float CubicOut(float time, float totaltime, float start, float end);
static float CubicInOut(float time, float totaltime, float start, float end);
static float QuartIn(float timet, float totaltime, float start, float end);
static float QuartOut(float time, float totaltime, float start, float end);
static float QuartInOut(float time, float totaltime, float start, float end);
static float QuintIn(float time, float totaltime, float start, float end);
static float QuintOut(float time, float totaltime, float start, float end);
static float QuintInOut(float time, float totaltime, float start, float end);
static float SineIn(float time, float totaltime, float start, float end);
static float SineOut(float time, float totaltime, float start, float end);
static float SineInOut(float time, float totaltime, float start, float end);
static float ExpIn(float time, float totaltime, float start, float end);
static float ExpOut(float time, float totaltime, float start, float end);
static float ExpInOut(float time, float totaltime, float start, float end);
static float CircIn(float time, float totaltime, float start, float end);
static float CircOut(float time, float totaltime, float start, float end);
static float CircInOut(float time, float totaltime, float start, float end);
static float ElasticIn(float time, float totaltime, float start, float end);
static float ElasticOut(float time, float totaltime, float start, float end);
static float ElasticInOut(float time, float totaltime, float start, float end);
static float BackIn(float time, float totaltime, float start, float end, float s);
static float BackOut(float time, float totaltime, float start, float end, float s);
static float BackInOut(float time, float totaltime, float start, float end, float s);
static float BounceIn(float time, float totaltime, float start, float end);
static float BounceOut(float time, float totaltime, float start, float end);
static float BounceInOut(float time, float totaltime, float start, float end);
static float Linear(float time, float totaltime, float start, float end);
};