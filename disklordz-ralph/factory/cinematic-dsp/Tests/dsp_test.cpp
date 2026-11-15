#include "CinematicDSP.h"
#include <stdexcept>
static void require(bool b) { if (!b) throw std::runtime_error("DSP smoke test failed"); }
#include <algorithm>
#include <cmath>
#include <iostream>
int main(){
    for(int mood=0;mood<5;++mood) for(int type=0;type<7;++type){
        auto p=dl::defaultRecipe(static_cast<dl::Voice>(type),static_cast<dl::Mood>(mood));
        auto a=dl::render(p);auto b=dl::render(p);
        require(a==b);require(!a.empty());
        float peak=0;double rms=0;
        for(float v:a){require(std::isfinite(v));peak=std::max(peak,std::abs(v));rms+=double(v)*v;}
        require(peak<1.0f);require(rms>1e-10);
    }
    std::cout<<"PASS: all 35 recipes are deterministic, finite, non-silent, peak-safe\n";
}
