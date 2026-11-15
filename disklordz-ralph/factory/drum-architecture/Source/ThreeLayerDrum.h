#pragma once
#include "CinematicDSP.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace dl {
// Pure C++20, no heap allocation during next(). Original sound = body,
// separately synthesized attack and textural resonances/noise.
struct LayerSettings {
    bool enabled = true;
    float level = 1.0f;
    float decayMs = 75.0f;
    float brightness = 0.5f;
};

struct DrumDesign {
    Recipe original = defaultRecipe(Voice::Bass808, Mood::Dark);
    LayerSettings body{true, 0.82f, 450.f, 0.5f};
    LayerSettings attack{true, 0.16f, 5.f, 0.8f};
    LayerSettings texture{true, 0.12f, 125.f, 0.55f};
    float attackToneHz = 2100.f;
    float textureToneHz = 770.f;
    float textureNoise = 0.5f;
    float outputGain = 0.72f;
    uint32_t seed = 261115u;
};

inline DrumDesign designFor(Voice v, Mood m, uint32_t seed=261115u) {
    DrumDesign d;
    d.original = defaultRecipe(v,m,seed);
    d.seed=seed;
    d.body.decayMs=static_cast<float>(d.original.amplitudeTauMs);
    switch(v) {
    case Voice::Bass808: d.attack={true,.11f,3.5f,.75f};d.texture={true,.07f,120.f,.28f};d.attackToneHz=1100;d.textureToneHz=95;break;
    case Voice::Kick: d.attack={true,.28f,2.5f,.80f};d.texture={true,.08f,35.f,.35f};d.attackToneHz=2200;d.textureToneHz=180;break;
    case Voice::Snare: d.attack={true,.38f,3.0f,.90f};d.texture={true,.24f,110.f,.70f};d.attackToneHz=3300;d.textureToneHz=850;break;
    case Voice::Clap: d.attack={true,.26f,5.f,.8f};d.texture={true,.30f,120.f,.65f};d.attackToneHz=2500;d.textureToneHz=1600;break;
    case Voice::ClosedHat: d.attack={true,.20f,1.5f,.95f};d.texture={true,.20f,30.f,.95f};d.attackToneHz=8500;d.textureToneHz=6500;break;
    case Voice::OpenHat: d.attack={true,.08f,2.0f,.95f};d.texture={true,.33f,290.f,.9f};d.attackToneHz=8500;d.textureToneHz=6000;break;
    case Voice::Perc: d.attack={true,.24f,4.f,.7f};d.texture={true,.35f,130.f,.6f};d.attackToneHz=2400;d.textureToneHz=1000;break;
    }
    return d;
}

class ThreeLayerDrum {
public:
    void prepare(double sampleRate) {
        fs=std::max(8000.0,sampleRate);
        body.prepare(fs);
        reset();
    }
    void start(DrumDesign design) {
        d=design;
        d.attack.decayMs=std::clamp(d.attack.decayMs,.25f,5000.f);
        d.texture.decayMs=std::clamp(d.texture.decayMs,.25f,5000.f);
        d.attackToneHz=std::clamp(d.attackToneHz,30.f,static_cast<float>(fs*.45));
        d.textureToneHz=std::clamp(d.textureToneHz,30.f,static_cast<float>(fs*.45));
        d.textureNoise=std::clamp(d.textureNoise,0.f,1.f);
        d.outputGain=std::clamp(d.outputGain,0.f,1.f);
        d.body.level=std::clamp(d.body.level,0.f,2.f);
        d.attack.level=std::clamp(d.attack.level,0.f,2.f);
        d.texture.level=std::clamp(d.texture.level,0.f,2.f);
        body.start(d.original);
        index=0;
        phaseAttack=phaseTexture=0;
        attackLP=textureLP=0;
        rng=d.seed?d.seed:1;
        active=true;
    }
    bool isActive() const noexcept { return active; }
    void stop() noexcept {active=false;body.stop();}
    float next() noexcept {
        if(!active)return 0.f;
        const double t=double(index++)/fs;
        double b=d.body.enabled?double(body.next())*d.body.level:0.;
        double a=0.,tx=0.;
        if(d.attack.enabled){
            const double e=std::exp(-t/(d.attack.decayMs*.001));
            // A high-passed synthetic noise impulse plus a fast decaying click oscillator.
            const double n=noise();
            const double cutoff=std::min(fs*.45,600.0+double(d.attack.brightness)*5500.0);
            const double alpha=1.-std::exp(-2.*pi*cutoff/fs);
            attackLP+=alpha*(n-attackLP);
            phaseAttack=wrap(phaseAttack+d.attackToneHz/fs);
            a=(.76*(n-attackLP)+.24*std::sin(2*pi*phaseAttack))*e*d.attack.level;
        }
        if(d.texture.enabled){
            const double e=std::exp(-t/(d.texture.decayMs*.001));
            const double tone=d.textureToneHz*(1.+.08*std::exp(-t/.016));
            phaseTexture=wrap(phaseTexture+tone/fs);
            const double modal=std::sin(2*pi*phaseTexture)+.28*std::sin(2*pi*1.63*phaseTexture);
            const double n=noise();
            const double cutoff=std::min(fs*.45,250.+double(d.texture.brightness)*13500.);
            const double alpha=1.-std::exp(-2.*pi*cutoff/fs);
            textureLP+=alpha*(n-textureLP);
            tx=(modal*(1.-d.textureNoise)+(n-textureLP)*d.textureNoise)*e*d.texture.level;
        }
        const double result=(b+a+tx)*d.outputGain;
        // No additional global waveshaping here: retain transient and phase identity.
        const double tail=std::max({.12,d.original.amplitudeTauMs*.001*11.,
                                    double(d.attack.decayMs)*.011,
                                    double(d.texture.decayMs)*.011});
        if(t>std::min(6.,tail+.02))stop();
        return std::isfinite(result)?static_cast<float>(result):0.f;
    }
private:
    static double wrap(double p) noexcept{return p-std::floor(p);}
    double noise() noexcept{rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return 2.*(double(rng)/4294967295.)-1.;}
    void reset(){body.stop();index=0;phaseAttack=phaseTexture=attackLP=textureLP=0;rng=1;active=false;}
    VoiceDSP body;
    DrumDesign d{};
    double fs=48000.,phaseAttack=0.,phaseTexture=0.,attackLP=0.,textureLP=0.;
    uint64_t index=0;
    uint32_t rng=1;
    bool active=false;
};
inline std::vector<float> renderDesign(const DrumDesign& design,double sampleRate=48000.){
    ThreeLayerDrum drum;drum.prepare(sampleRate);drum.start(design);
    const double tau=std::max({design.original.amplitudeTauMs,
             double(design.attack.decayMs),double(design.texture.decayMs)});
    const size_t samples=static_cast<size_t>(sampleRate*std::min(6.,std::max(.12,tau*.011+.04)));
    std::vector<float> output(samples);
    for(auto& s:output)s=drum.next();
    const size_t fade=std::min<size_t>(256,output.size()/8);
    for(size_t i=0;i<fade;++i)output[output.size()-fade+i]*=float(fade-i)/float(fade);
    return output;
}
}
