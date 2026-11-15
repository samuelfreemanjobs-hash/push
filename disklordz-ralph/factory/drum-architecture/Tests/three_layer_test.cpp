#include "ThreeLayerDrum.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
static void check(bool v){if(!v)throw std::runtime_error("Three layer DSP test failed");}
int main(){
 for(int mood=0;mood<5;++mood)for(int kind=0;kind<7;++kind){
  const auto d=dl::designFor(static_cast<dl::Voice>(kind),static_cast<dl::Mood>(mood),unsigned(20261115+kind+mood*7));
  const auto a=dl::renderDesign(d),b=dl::renderDesign(d);
  check(a==b);check(!a.empty());double rms=0.;
  for(const auto x:a){check(std::isfinite(x));rms+=double(x)*x;}
  check(rms>1.e-8);
  auto bodyOnly=d;bodyOnly.attack.enabled=false;bodyOnly.texture.enabled=false;
  check(dl::renderDesign(bodyOnly)!=a);
  auto attackOnly=d;attackOnly.body.enabled=false;attackOnly.texture.enabled=false;
  check(dl::renderDesign(attackOnly)!=a);
  auto textureOnly=d;textureOnly.body.enabled=false;textureOnly.attack.enabled=false;
  check(dl::renderDesign(textureOnly)!=a);
 }
 std::cout<<"PASS 35 designs: deterministic, finite, audible, independent layers\n";
}
