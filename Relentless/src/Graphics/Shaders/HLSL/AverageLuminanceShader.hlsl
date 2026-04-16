
//#include "Common.hlsli"
//
//#define HISTOGRAM_AVERAGE_THREADS_PER_DIMENSION 16
//#define NUM_HISTOGRAM_BINS 256

#if 0
struct PassParams
{
    uint PixelCount;
    float MinLogLuminance;
    float LogLuminanceRange;
    float TimeDelta;
    float SpeedUp;
    float SpeedDown;
    float ExposureCompensation;
    float MinEV100;
    float MaxEV100;
    uint LuminanceHistogramIndex;
    uint LuminanceOutputIndex;
    float Padding;
};

ConstantBuffer<PassParams> passParams : register(b0, space0);

groupshared float gHistogramShared[NUM_HISTOGRAM_BINS];

float EV100FromLuminance(float luminance)
{
	//https://google.github.io/filament/Filament.md.html#imagingpipeline/physicallybasedcamera/exposurevalue
    const float K = 12.5f; //Reflected-light meter constant
    const float ISO = 100.0f;
    return log2(luminance * (ISO / K));
}

float Exposure(float ev100)
{
	//https://google.github.io/filament/Filament.md.html#imagingpipeline/physicallybasedcamera/exposurevalue
    return 1.0f / (pow(2.0f, ev100) * 1.2f);
}

float Adaption(float current, float target, float dt, float speedUp, float speedDown)
{
    const float speed = target > current ? speedUp : speedDown;
    const float factor = 1.0f - exp2(-dt * speed);
    return current + (target - current) * factor;
}

[numthreads(HISTOGRAM_AVERAGE_THREADS_PER_DIMENSION, HISTOGRAM_AVERAGE_THREADS_PER_DIMENSION, 1)]
void cs_main(uint groupIndex : SV_GroupIndex)
{
    Buffer<uint> luminanceHistogram = ResourceDescriptorHeap[passParams.LuminanceHistogramIndex];
    
    const float countForThisBin = (float)luminanceHistogram[groupIndex];
    gHistogramShared[groupIndex] = countForThisBin * groupIndex;
    GroupMemoryBarrierWithGroupSync();

	[unroll]
    for (uint histogramSampleIndex = (NUM_HISTOGRAM_BINS >> 1); histogramSampleIndex > 0; histogramSampleIndex >>= 1)
    {
        if (groupIndex < histogramSampleIndex)
        {
            gHistogramShared[groupIndex] += gHistogramShared[groupIndex + histogramSampleIndex];
        }
        GroupMemoryBarrierWithGroupSync();
    }

    if (groupIndex == 0)
    {
        RWStructuredBuffer<float> output = ResourceDescriptorHeap[passParams.LuminanceOutputIndex];
        
        const float weightedLogAverage = (gHistogramShared[0] / max((float) passParams.PixelCount - countForThisBin, 1.0)) - 1.0;
        const float weightedAverageLuminance = exp2(((weightedLogAverage / (NUM_HISTOGRAM_BINS - 1)) * passParams.LogLuminanceRange) + passParams.MinLogLuminance);
        const float luminanceLastFrame = output[0];
        const float adaptedLuminance = Adaption(luminanceLastFrame, weightedAverageLuminance, passParams.TimeDelta, passParams.SpeedUp, passParams.SpeedDown);
        const float ev = clamp(EV100FromLuminance(adaptedLuminance), passParams.MinEV100, passParams.MaxEV100);
        const float exposure = Exposure(ev) * exp2(passParams.ExposureCompensation);
        
        output[0] = adaptedLuminance;
        output[1] = weightedAverageLuminance;
        output[2] = exposure;
    }
}

#endif

#include "Common.hlsli"

#define HISTOGRAM_AVERAGE_THREADS_PER_DIMENSION 16
#define NUM_HISTOGRAM_BINS 256

struct PassParams
{
    uint PixelCount;
    float MinLogLuminance;
    float LogLuminanceRange;
    float TimeDelta;

    float SpeedUp;
    float SpeedDown;

    float ExposureCompensation;
    float MinEV100;
    float MaxEV100;

    float LowPercent;
    float HighPercent;
    uint LuminanceHistogramIndex;
    uint LuminanceOutputIndex;
    
    float3 Padding;
};

ConstantBuffer<PassParams> passParams : register(b0, space0);

// Shared histogram counts (raw, not prefix summed)
groupshared uint gCountShared[NUM_HISTOGRAM_BINS];

float EV100FromLuminance(float luminance)
{
    // Filament: https://google.github.io/filament/Filament.md.html#imagingpipeline/physicallybasedcamera/exposurevalue
    const float K = 12.5f; // reflected-light meter constant
    const float ISO = 100.0f; // EV100 definition
    return log2(max(luminance, 1e-6f) * (ISO / K));
}

float ExposureFromEV100(float ev100)
{
    // Filament: exposure = 1 / (2^ev100 * 1.2)
    return 1.0f / (exp2(ev100) * 1.2f);
}

float Adapt(float current, float target, float dt, float speedUp, float speedDown)
{
    const float speed = (target > current) ? speedUp : speedDown;
    const float factor = 1.0f - exp2(-dt * speed);
    return current + (target - current) * factor;
}

[numthreads(HISTOGRAM_AVERAGE_THREADS_PER_DIMENSION, HISTOGRAM_AVERAGE_THREADS_PER_DIMENSION, 1)]
void cs_main(uint groupIndex : SV_GroupIndex)
{
    Buffer<uint> luminanceHistogram = ResourceDescriptorHeap[passParams.LuminanceHistogramIndex];

    // Load histogram to shared
    gCountShared[groupIndex] = luminanceHistogram[groupIndex];
    GroupMemoryBarrierWithGroupSync();

    // Single-threaded percentile trim + weighted average (256 bins => trivial cost)
    if (groupIndex == 0)
    {
        RWStructuredBuffer<float> output = ResourceDescriptorHeap[passParams.LuminanceOutputIndex];

        const uint bin0CountU = gCountShared[0];
        const uint totalU = (passParams.PixelCount > bin0CountU) ? (passParams.PixelCount - bin0CountU) : 0u;

        // If the frame is basically all "black bin", fall back safely
        if (totalU == 0u)
        {
            // Keep previous luminance but still compute exposure consistently
            const float lastLum = output[0];
            const float ev = clamp(EV100FromLuminance(lastLum), passParams.MinEV100, passParams.MaxEV100);
            output[2] = ExposureFromEV100(ev) * passParams.ExposureCompensation;
            output[1] = lastLum;
            return;
        }

        // Clamp percent inputs to sane ranges
        const float lowP = saturate(passParams.LowPercent);
        const float highP = saturate(passParams.HighPercent);

        // Ensure high > low; if not, disable trimming.
        const bool doTrim = (highP > lowP);

        const float lowThresholdF = lowP * (float) totalU;
        const float highThresholdF = highP * (float) totalU;

        // Find min/max bins (in [1..255]) based on cumulative counts excluding bin0
        uint minBin = 1;
        uint maxBin = NUM_HISTOGRAM_BINS - 1;

        if (doTrim)
        {
            uint cumulative = 0;
            bool foundMin = false;

            for (uint i = 1; i < NUM_HISTOGRAM_BINS; ++i)
            {
                cumulative += gCountShared[i];

                if (!foundMin && (float) cumulative >= lowThresholdF)
                {
                    minBin = i;
                    foundMin = true;
                }

                if ((float) cumulative > highThresholdF)
                {
                    // i is the first bin that exceeds high threshold => previous is last included
                    maxBin = (i > 1) ? (i - 1) : 1;
                    break;
                }
            }

            // Safety
            if (!foundMin)
                minBin = 1;
            if (minBin > maxBin)
            {
                minBin = 1;
                maxBin = NUM_HISTOGRAM_BINS - 1;
            }
        }

        // Compute trimmed weighted mean bin index in [1..255]
        double weightedSum = 0.0;
        uint countSum = 0;

        for (uint bin = minBin; bin <= maxBin; ++bin)
        {
            const uint c = gCountShared[bin];
            weightedSum += (double) c * (double) bin;
            countSum += c;
        }

        if (countSum == 0u)
        {
            // Fallback: use full range [1..255]
            weightedSum = 0.0;
            countSum = 0u;
            for (uint bin = 1; bin < NUM_HISTOGRAM_BINS; ++bin)
            {
                const uint c = gCountShared[bin];
                weightedSum += (double) c * (double) bin;
                countSum += c;
            }
            countSum = max(countSum, 1u);
        }

        // Your original mapping expects "-1" because bin 0 is reserved for "below epsilon"
        const float meanBin = (float) (weightedSum / (double) countSum);
        const float weightedLogAverage = meanBin - 1.0f;

        // Convert mean bin back to luminance using the same mapping as the histogram pass
        const float logL = ((weightedLogAverage / (NUM_HISTOGRAM_BINS - 1)) * passParams.LogLuminanceRange) + passParams.MinLogLuminance;
        const float targetLuminance = exp2(logL);

        // Temporal adaptation in luminance domain
        const float lastLuminance = output[0];
        const float adaptedLuminance = Adapt(lastLuminance, targetLuminance, passParams.TimeDelta, passParams.SpeedUp, passParams.SpeedDown);

        // Clamp in EV domain, then convert to exposure
        float ev = EV100FromLuminance(adaptedLuminance);

        // Exposure compensation in stops is additive in EV
        //ev += passParams.ExposureCompensation;

        ev = clamp(ev, passParams.MinEV100, passParams.MaxEV100);

        const float exposure = ExposureFromEV100(ev) * passParams.ExposureCompensation;

        output[0] = adaptedLuminance;
        output[1] = targetLuminance;
        output[2] = exposure;
    }
}
