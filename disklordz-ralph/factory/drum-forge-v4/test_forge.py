"""Simple checks for deterministic rendering and generated WAV consistency."""
import numpy as np
import forge_core_v2 as core
import drum_forge_v4 as forge

def test_notes():
    assert forge.parse_midi('C1')==24
    assert forge.parse_midi('Bb0')==22
    assert forge.parse_midi('F#1')==30

def test_reproducibility():
    p=core.PRESETS['memphis']
    a,_=core.gen_808(44100,np.random.default_rng(123),p,24)
    b,_=core.gen_808(44100,np.random.default_rng(123),p,24)
    assert np.array_equal(a,b)

def test_audio():
    x,r=core.gen_808(44100,np.random.default_rng(42),core.PRESETS['drift_phonk'],24)
    assert np.all(np.isfinite(x))
    y=core.master(forge.final_trim(x,44100),44100,-1.12)
    assert core.true_peak_db(y)<=-1.10
    detected,confidence=core.estimate_808_pitch(y,44100,r)
    assert detected is not None and abs(12*np.log2(detected/r))<.5 and confidence>.48

if __name__=='__main__':
    test_notes();test_reproducibility();test_audio();print('All tests passed')
