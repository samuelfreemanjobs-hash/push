#include "CinematicDSP.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <cstdint>

static void little(std::ofstream& f,uint32_t v,int bytes) {
    for(int k=0;k<bytes;++k) f.put(char((v>>(8*k))&0xff));
}
static void writeWav24(const std::filesystem::path& path,const std::vector<float>& x,uint32_t sr) {
    std::ofstream f(path,std::ios::binary);if(!f) throw std::runtime_error("Could not create WAV");
    const uint32_t data=uint32_t(x.size()*3);
    f.write("RIFF",4);little(f,36+data,4);f.write("WAVEfmt ",8);
    little(f,16,4);little(f,1,2);little(f,1,2);little(f,sr,4);little(f,sr*3,4);
    little(f,3,2);little(f,24,2);f.write("data",4);little(f,data,4);
    for(float s:x) {
        const int32_t z=int32_t(std::lround(std::clamp(s,-1.0f,1.0f)*8388607.0f));
        little(f,uint32_t(z),3);
    }
}
int main(int argc,char** argv) {
    namespace fs=std::filesystem;
    fs::path dir=argc>1?fs::path(argv[1]):fs::path("rendered");
    fs::create_directories(dir);
    const char* names[]={"808","Kick","Snare","Clap","ClosedHat","OpenHat","Perc"};
    const char* moods[]={"Dark","Luxury","Haunted","Aggressive","Atmospheric"};
    int n=0;
    for(int mood=0;mood<5;++mood) for(int type=0;type<7;++type) {
        auto p=dl::defaultRecipe(static_cast<dl::Voice>(type),static_cast<dl::Mood>(mood),261115+unsigned(mood*100+type));
        auto samples=dl::render(p);
        writeWav24(dir/(std::string("DL_CINE_")+moods[mood]+"_"+names[type]+".wav"),samples,48000);
        ++n;
    }
    std::cout<<"Wrote "<<n<<" original 24-bit 48kHz WAV samples to "<<dir<<"\n";
}
