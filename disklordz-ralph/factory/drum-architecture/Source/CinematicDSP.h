#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace dl {
constexpr double pi = 3.14159265358979323846;
enum class Voice { Bass808, Kick, Snare, Clap, ClosedHat, OpenHat, Perc };
enum class Mood { Dark, Luxury, Haunted, Aggressive, Atmospheric };

struct Recipe {
    Voice voice = Voice::Bass808;
    Mood mood = Mood::Dark;
    double rootHz = 49.0;
    double pitchStartHz = 155.0;
    double pitchTauMs = 28.0;
    double amplitudeTauMs = 320.0;
    double drive = 1.4;
    double noise = 0.2;
    double brightness = 0.5;
    double fmIndex = 1.8;
    double clapSpreadMs = 9.0;
    double outputGain = 0.65;
    uint32_t seed = 261115;
};
inline Recipe defaultRecipe(Voice voice, Mood mood, uint32_t seed=261115) {
    Recipe p; p.voice=voice;p.mood=mood;p.seed=seed;
    switch(voice) {
    case Voice::Bass808: p.rootHz=49;p.pitchStartHz=155;p.pitchTauMs=28;p.amplitudeTauMs=420;p.drive=1.3;p.outputGain=.72;break;
    case Voice::Kick: p.rootHz=57;p.pitchStartHz=185;p.pitchTauMs=18;p.amplitudeTauMs=70;p.drive=1.5;p.noise=.10;p.outputGain=.78;break;
    case Voice::Snare: p.rootHz=185;p.amplitudeTauMs=63;p.noise=.65;p.outputGain=.66;break;
    case Voice::Clap: p.amplitudeTauMs=44;p.clapSpreadMs=9;p.noise=.90;p.outputGain=.65;break;
    case Voice::ClosedHat: p.amplitudeTauMs=27;p.noise=.30;p.outputGain=.38;break;
    case Voice::OpenHat: p.amplitudeTauMs=175;p.noise=.30;p.outputGain=.34;break;
    case Voice::Perc: p.rootHz=660;p.amplitudeTauMs=74;p.fmIndex=1.8;p.outputGain=.52;break;
    }
    switch(mood) {
    case Mood::Dark:p.brightness=.35;break;
    case Mood::Luxury:p.drive=1.05;p.brightness=.62;p.outputGain*=.92;break;
    case Mood::Haunted:p.fmIndex*=1.7;p.amplitudeTauMs*=1.25;p.brightness=.22;break;
    case Mood::Aggressive:p.drive*=2.0;p.pitchTauMs*=.72;p.brightness=.77;break;
    case Mood::Atmospheric:p.amplitudeTauMs*=1.6;p.drive*=.85;p.brightness=.54;break;
    }
    return p;
}

// Audio-thread-friendly, owns its state; no allocations in next().
class VoiceDSP {
public:
    void prepare(double sampleRate) { fs=std::max(8000.0,sampleRate); reset(); }
    void start(const Recipe& recipe) {
        p=recipe;
        p.rootHz=std::clamp(p.rootHz,20.0,5000.0);
        p.pitchStartHz=std::clamp(p.pitchStartHz, p.rootHz, 8000.0);
        p.pitchTauMs=std::clamp(p.pitchTauMs,1.0,500.0);
        p.amplitudeTauMs=std::clamp(p.amplitudeTauMs,2.0,5000.0);
        p.clapSpreadMs=std::clamp(p.clapSpreadMs,2.0,35.0);
        p.drive=std::clamp(p.drive,.25,10.0);
        p.noise=std::clamp(p.noise,0.0,1.0);
        p.brightness=std::clamp(p.brightness,0.0,1.0);
        p.outputGain=std::clamp(p.outputGain,0.0,1.0);
        reset();
        rng = p.seed ? p.seed : 1;
        active=true;
    }
    bool isActive() const { return active; }
    void stop() { active=false; }
    float next() {
        if(!active) return 0;
        const double t=static_cast<double>(sampleIndex++)/fs;
        const double tau=p.amplitudeTauMs*.001;
        const double env=std::exp(-t/tau);
        double s=0;
        if(p.voice==Voice::Bass808 || p.voice==Voice::Kick) {
            const double sweep=p.pitchStartHz-p.rootHz;
            const double f=p.rootHz+sweep*std::exp(-t/(p.pitchTauMs*.001));
            phase=wrap(phase+f/fs);
            const double fundamental=std::sin(2*pi*phase);
            const double second=.11*std::sin(4*pi*phase);
            const double harmonic=std::tanh((fundamental+second)*p.drive)/std::tanh(p.drive);
            // Phase-coherent fundamental plus band-limited-ish mild harmonics.
            // Production upgrade: oversample nonlinear branch before shipping.
            s=((.80*fundamental+.20*harmonic)*env);
            if(p.voice==Voice::Kick) {
                s+=p.noise*highPass(noise(),2800.0)*std::exp(-t/.0025);
            } else {
                s+=p.noise*.09*highPass(noise(),1800.0)*std::exp(-t/.0025);
            }
            s*=1.0-std::exp(-t/.00055);
        } else if(p.voice==Voice::Snare) {
            const double body=(.65*std::sin(2*pi*185*t)+.22*std::sin(2*pi*334*t))*std::exp(-t/.038);
            const double snap=highPass(lowPass(noise(),7000),950)*std::exp(-t/tau);
            const double click=highPass(noise(),4000)*std::exp(-t/.0025);
            s=.47*body + .47*p.noise*snap + .11*click;
        } else if(p.voice==Voice::Clap) {
            const double spread=p.clapSpreadMs*.001;
            double bursts=0;
            for(int k=0;k<4;++k) {
                const double local=t-k*spread;
                if(local>=0) bursts+=std::exp(-local/(k==3 ? tau : .006));
            }
            s=.47*std::min(1.0,bursts*.6)*highPass(lowPass(noise(),8500),850);
        } else if(p.voice==Voice::ClosedHat || p.voice==Voice::OpenHat) {
            static constexpr std::array<double,6> f{205.3,304.4,369.6,522.7,540.0,800.0};
            double metal=0;
            for(size_t k=0;k<f.size();++k) {
                hatPhase[k]=wrap(hatPhase[k]+f[k]/fs);
                metal+=(hatPhase[k]<.5 ? 1.0 : -1.0)/6.0;
            }
            const double raw=(1.0-p.noise)*metal+p.noise*noise();
            const double cut=3500.0+4000.0*p.brightness;
            s=highPass(raw,cut)*env*.75;
        } else { // Tuned FM percussion / eerie modal effect.
            const double f=p.rootHz+700*std::exp(-t/.024);
            phase=wrap(phase+f/fs);
            modPhase=wrap(modPhase+f*2.5/fs);
            const double index=p.fmIndex*std::exp(-t/.035);
            s=std::sin(2*pi*phase+index*std::sin(2*pi*modPhase))*env;
            s+=.16*std::sin(2*pi*1.71*p.rootHz*t)*std::exp(-t/.05);
        }
        // Peak bounded to avoid clipping, input still needs mix gain management.
        s=std::tanh(s*p.drive*.70)*p.outputGain;
        if(t>std::max(.2, tau*11.0)+.12) active=false;
        return std::isfinite(s)?static_cast<float>(s):0.0f;
    }
private:
    static double wrap(double v){return v-std::floor(v);}
    void reset(){ sampleIndex=0;phase=modPhase=0;hatPhase.fill(0);lp=hpPrevX=hpPrevY=0;active=false; }
    double noise(){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return (double(rng)/4294967295.0)*2.0-1.0;}
    double lowPass(double x,double cutoff) {
        const double a=1-std::exp(-2*pi*std::min(cutoff,fs*.45)/fs);
        lp+=a*(x-lp);return lp;
    }
    double highPass(double x,double cutoff) {
        const double a=std::exp(-2*pi*std::min(cutoff,fs*.45)/fs);
        const double y=a*(hpPrevY+x-hpPrevX);hpPrevX=x;hpPrevY=y;return y;
    }
    double fs=48000;
    Recipe p{};
    bool active=false;
    uint64_t sampleIndex=0;
    uint32_t rng=1;
    double phase=0, modPhase=0, lp=0,hpPrevX=0,hpPrevY=0;
    std::array<double,6> hatPhase{};
};
inline std::vector<float> render(const Recipe& p,double sampleRate=48000.0) {
    VoiceDSP voice;voice.prepare(sampleRate);voice.start(p);
    const auto samples=static_cast<size_t>(sampleRate*std::min(6.0, p.amplitudeTauMs*.001*11.0+.14));
    std::vector<float> out;out.reserve(samples);
    for(size_t i=0;i<samples;++i) out.push_back(voice.next());
    const size_t fade=std::min<size_t>(128,out.size()/8);
    for(size_t i=0;i<fade;++i) out[out.size()-fade+i]*=float(fade-i)/float(fade);
    return out;
}
} // namespace dl
