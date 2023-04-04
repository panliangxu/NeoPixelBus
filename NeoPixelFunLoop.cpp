#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

const uint16_t PixelCount = 60; // 确保将其设置为条带中的像素数(灯珠数量)
const uint16_t PixelPin = 5;  // 确保将其设置为正确的引脚，Esp8266 忽略(数据接脚)
const uint16_t AnimCount = PixelCount; // 我们需要足够的动画

const uint16_t PixelFadeDuration = 300; //三分之一秒
const uint16_t NextPixelMoveDuration = 1000 / PixelCount; // how fast we move through the pixels

NeoGamma<NeoGammaTableMethod> colorGamma; // for any fade animations, best to correct gamma
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin); //初始化灯带

struct MyAnimationState
{
    RgbColor StartingColor;
    RgbColor EndingColor;
    uint16_t IndexPixel; // //这个动画影响哪个像素
};

NeoPixelAnimator animations(AnimCount); // 动画管理对象
MyAnimationState animationState[AnimCount];
uint16_t frontPixel = 0;  // 循环的前端
RgbColor frontColor;  // 循环前面的颜色

void SetRandomSeed()
{
    uint32_t seed;
    seed = analogRead(0);
    delay(1);

    for (int shifts = 3; shifts < 31; shifts += 3)
    {
        seed ^= analogRead(0) << shifts;
        delay(1);
    }

    randomSeed(seed);
}

void FadeOutAnimUpdate(const AnimationParam& param)
{
    strip.SetPixelColor(animationState[param.index].IndexPixel, 
        colorGamma.Correct(animationState[param.index].EndingColor));
}

void LoopAnimUpdate(const AnimationParam& param)
{
    if (param.state == AnimationState_Completed)
    {
        animations.RestartAnimation(param.index);

        frontPixel = (frontPixel + 1) % PixelCount; // 递增和换行
        if (frontPixel == 0)
        {
            frontColor = HslColor(random(360) / 360.0f, 1.0f, 0.25f);
        }

        uint16_t indexAnim;
        if (animations.NextAvailableAnimation(&indexAnim, 1))
        {
            animationState[indexAnim].StartingColor = frontColor;
            animationState[indexAnim].EndingColor = frontColor;
            animationState[indexAnim].IndexPixel = frontPixel;
            animations.StartAnimation(indexAnim, PixelFadeDuration, FadeOutAnimUpdate);
        }
    }
}

void setup()
{
    strip.Begin();
    strip.Show();
    SetRandomSeed();
    animations.StartAnimation(0, NextPixelMoveDuration, LoopAnimUpdate);
}


void loop()
{
    animations.UpdateAnimations();
    strip.Show();
}
