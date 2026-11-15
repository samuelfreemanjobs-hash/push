#!/usr/bin/env python3
"""DiskLordz DRUM FORGE v2 — deterministic, synthesis-only trap/phonk drum kit factory.

Dependencies: numpy, scipy, soundfile
Examples:
 python disklordz_drum_forge.py --preset all --variations 3 --out DiskLordz_Kit
 python disklordz_drum_forge.py --preset memphis --variations 8 --sr 48000
"""
from __future__ import annotations
import argparse, csv, json, math, re
from pathlib import Path
import numpy as np
import soundfile as sf
from scipy.signal import butter, sosfilt, resample_poly, lfilter

NOTES = ('C','C#','D','D#','E','F','F#','G','G#','A','A#','B')
PRESETS = {
    'atl_trap': {'808_decay':1.20, '808_drive':3.6, 'knock':0.85, 'snare_air':0.85, 'hat_bright':0.8, 'lofi':0.02, 'verb':0.11},
    'memphis':  {'808_decay':0.82, '808_drive':5.3, 'knock':0.95, 'snare_air':0.64, 'hat_bright':0.5, 'lofi':0.18, 'verb':0.06},
    'drift_phonk': {'808_decay':0.98, '808_drive':6.4, 'knock':1.15, 'snare_air':1.0, 'hat_bright':1.15, 'lofi':0.09, 'verb':0.10},
    'space_trap': {'808_decay':1.8, '808_drive':2.7, 'knock':0.65, 'snare_air':0.73, 'hat_bright':0.72, 'lofi':0.0, 'verb':0.22},
}
FAMILIES = ('808','kick','snare','clap','closed_hat','open_hat','cowbell','perc')

def hz_to_note(hz):
    midi = int(round(69 + 12 * np.log2(max(1e-10, float(hz))/440)))
    return f'{NOTES[midi%12]}{midi//12-1}'

def note_to_hz(midi): return 440 * 2**((midi-69)/12)

def envelope(t, attack=0.001, decay=0.2, curve=1.0):
    return (1-np.exp(-t/max(attack,1e-5))) * np.exp(-(t/max(decay,1e-4))**curve)

def filter_signal(x, cutoff, kind, sr, order=2):
    nyq = sr/2
    cutoff = float(np.clip(cutoff, 20, nyq*0.92))
    return sosfilt(butter(order, cutoff, btype=kind, fs=sr, output='sos'), x)

def osc(freq, sr, phase=0):
    """Frequency in Hz, scalar or vector. Integrated phase avoids glides that click."""
    return np.sin(phase + 2*np.pi*np.cumsum(np.asarray(freq))/sr)

def fade(x, sr, fadein_ms=0.1, fadeout_ms=8):
    x = np.array(x, dtype=np.float64, copy=True)
    n = len(x)
    ni = min(n, int(sr*fadein_ms/1000))
    no = min(n, int(sr*fadeout_ms/1000))
    if ni: x[:ni] *= np.linspace(0,1,ni)[:,None] if x.ndim==2 else np.linspace(0,1,ni)
    if no: x[-no:] *= np.linspace(1,0,no)[:,None] if x.ndim==2 else np.linspace(1,0,no)
    return x

def saturate_oversampled(x, drive=3, mix=1, factor=4):
    """Oversample nonlinear stage to reduce aliasing (not a perfect analog model)."""
    up = resample_poly(x, factor, 1, axis=0)
    y = np.tanh(drive*up)/max(np.tanh(drive), 1e-8)
    y = resample_poly(y, 1, factor, axis=0)[:len(x)]
    if len(y)<len(x): y = np.pad(y, (0,len(x)-len(y)))
    return (1-mix)*x + mix*y

def sub_translation(x, sr, amount=0.22, cutoff=115):
    """Harmonic enhancement from isolated low end; high-pass added harmonics."""
    sub = filter_signal(x, cutoff, 'lowpass', sr, 4)
    harmonic = saturate_oversampled(sub, 3.6, 1)
    harmonic = filter_signal(harmonic, cutoff*1.05, 'highpass', sr, 3)
    return x + amount * harmonic

def comb(x, delay, feedback, damping=0.28):
    """Lowpass feedback comb. One zero + denominator path with fixed coefficients."""
    # Feedback filter: y[n] = x[n] + f*(1-d)*y[n-D] + f*d*y[n-D-1]
    a = np.zeros(delay+2)
    a[0]=1
    a[delay] = -feedback*(1-damping)
    a[delay+1] = -feedback*damping
    return lfilter([1.], a, x)

def allpass(x, delay, gain=0.55):
    # H(z) = (-g + z^-D) / (1 - g*z^-D)
    b = np.zeros(delay+1); a=np.zeros(delay+1)
    b[0]=-gain; b[delay]=1
    a[0]=1; a[delay]=-gain
    return lfilter(b, a, x)

def room_reverb(x, sr, decay=0.36, wet=0.14):
    """Schroeder parallel comb + series allpass, stereo independent delay lines."""
    x=np.asarray(x)
    source = x if x.ndim==2 else np.column_stack([x,x])
    tail = int(sr*(decay+0.13))
    padded = np.pad(source,((0,tail),(0,0)))
    out=np.zeros_like(padded)
    for ch in range(2):
        comb_out=np.zeros(len(padded))
        for d_ms in ([29.7,37.1,41.1,43.7] if ch==0 else [30.7,35.9,40.3,44.9]):
            delay=max(1,int(sr*d_ms/1000))
            # -60 dB decay target: feedback^repetitions ~ 0.001
            feedback=min(0.90, 10**(-3*delay/(sr*max(decay,0.08))))
            comb_out += comb(padded[:,ch],delay,feedback)/4
        for d_ms in (5.0,1.7): comb_out=allpass(comb_out,max(1,int(sr*d_ms/1000)))
        out[:,ch]=comb_out
    out = (1-wet)*padded + wet*out
    return out if x.ndim==2 else out.mean(axis=1)

def quantize_character(x, sr, rng, amount):
    if amount<=0: return x
    # Blend slightly softened bandwidth and quiet, deterministic tape-like noise.
    dull = filter_signal(x, 12000 - 8500*amount, 'lowpass', sr, 2)
    noise = filter_signal(rng.standard_normal(len(x)), 200, 'highpass', sr) * 0.0018*amount
    return (1-0.55*amount)*x + 0.55*amount*dull + noise

def gen_808(sr,rng,p,midi):
    root=note_to_hz(midi)
    length=max(1.0,p['808_decay']*3.2)
    t=np.arange(int(sr*length))/sr
    pitch=root + rng.uniform(100,180)*np.exp(-t/rng.uniform(.009,.022)) + rng.uniform(2,7)*np.exp(-t/.14)
    core=osc(pitch,sr)
    amp=envelope(t,.0008,p['808_decay'],1.1)
    body=core*amp
    body += .075*np.sin(2*np.pi*np.cumsum(pitch*2)/sr)*amp
    click=filter_signal(rng.standard_normal(len(t)),1100,'highpass',sr)*np.exp(-t/.003)*.035*p['knock']
    x=body+click
    x=sub_translation(x,sr,amount=.32)
    x=saturate_oversampled(x,p['808_drive'],mix=.45)
    return x,root

def gen_kick(sr,rng,p):
    t=np.arange(int(sr*.72))/sr
    f=47+rng.uniform(95,155)*np.exp(-t/.017)
    x=osc(f,sr)*envelope(t,.00035,.16,.83)
    transient=filter_signal(rng.standard_normal(len(t)),1600,'highpass',sr)*np.exp(-t/.0025)*.09*p['knock']
    return saturate_oversampled(x+transient,2.4,.28),None

def gen_snare(sr,rng,p):
    t=np.arange(int(sr*.52))/sr
    pitch=175+105*np.exp(-t/.012)
    body=osc(pitch,sr)*np.exp(-t/.042)*.64
    noise=filter_signal(rng.standard_normal(len(t)),950,'highpass',sr)
    snap=filter_signal(noise,7000,'lowpass',sr)*np.exp(-t/.09)*p['snare_air']*.63
    x=body+snap
    return room_reverb(x,sr,decay=.25,wet=p['verb']),None

def gen_clap(sr,rng,p):
    t=np.arange(int(sr*.46))/sr
    n=filter_signal(rng.standard_normal(len(t)),1100,'highpass',sr)
    bursts=np.zeros_like(t)
    for ms in (0,13,26,39):
        start=ms/1000
        bursts += np.exp(-np.maximum(t-start,0)/(.005 if ms<39 else .065))*(t>=start)
    x=filter_signal(n,8500,'lowpass',sr)*bursts*.39
    return room_reverb(x,sr,.28,p['verb']*.85),None

def gen_hat(sr,rng,p,is_open=False):
    duration=.82 if is_open else .18
    t=np.arange(int(sr*duration))/sr
    noise=filter_signal(rng.standard_normal(len(t)),rng.uniform(5800,8500)*p['hat_bright'],'highpass',sr)
    metallic=(np.sign(np.sin(2*np.pi*4137*t))+np.sign(np.sin(2*np.pi*5579*t)))*.14
    decay=.28 if is_open else .035
    x=(noise*.58+metallic)*envelope(t,.0002,decay,1.07)
    return x,None

def gen_cowbell(sr,rng,p):
    t=np.arange(int(sr*.56))/sr
    # Inharmonic dual-pulse classic cowbell-inspired architecture, entirely synthesized.
    osc1=np.sign(np.sin(2*np.pi*(537+rng.uniform(-8,8))*t))
    osc2=np.sign(np.sin(2*np.pi*(803+rng.uniform(-9,9))*t))
    raw=(osc1+osc2)*.4
    x=filter_signal(filter_signal(raw,380,'highpass',sr),4800,'lowpass',sr)
    x*=envelope(t,.0004,.12,.91)
    return saturate_oversampled(x,2.0,.23),None

def gen_perc(sr,rng,p):
    t=np.arange(int(sr*.42))/sr
    f=rng.uniform(260,560)*np.exp(-t/.045)+rng.uniform(135,240)
    tone=osc(f,sr)*envelope(t,.0002,.065)
    noise=filter_signal(rng.standard_normal(len(t)),2300,'highpass',sr)*np.exp(-t/.012)*.18
    return tone*.75+noise,None

GENERATORS={'808':gen_808,'kick':gen_kick,'snare':gen_snare,'clap':gen_clap,
            'closed_hat':lambda sr,rng,p:gen_hat(sr,rng,p,False),
            'open_hat':lambda sr,rng,p:gen_hat(sr,rng,p,True),
            'cowbell':gen_cowbell,'perc':gen_perc}

def estimate_808_pitch(x,sr,expected_hz):
    """Stable-tail normalized autocorrelation plus agreement/confidence check."""
    s=int(sr*.24); e=min(len(x),s+int(sr*.34))
    v=np.asarray(x[s:e],dtype=float)
    if len(v)<2048 or np.max(np.abs(v))<1e-5: return None,0
    v=filter_signal(v,200,'lowpass',sr,3)
    v-=np.mean(v)
    # FFT-based linear autocorrelation avoids O(N^2) np.correlate.
    n=1 << (2*len(v)-1).bit_length()
    a=np.fft.irfft(abs(np.fft.rfft(v,n))**2,n)[:len(v)]
    lo=max(1,int(sr/(expected_hz*1.13)))
    hi=min(len(a),int(sr/(expected_hz*.87))+1)
    if hi<=lo:return None,0
    lag=lo+int(np.argmax(a[lo:hi])); confidence=float(a[lag]/max(a[0],1e-12))
    return sr/lag,confidence

def true_peak_db(x, factor=4):
    y=resample_poly(x,factor,1,axis=0)
    return float(20*np.log10(max(np.max(np.abs(y)),1e-12)))

def master(x,sr,ceiling_db=-1.0):
    """Apply DC removal and fades BEFORE oversampled true-peak-constrained gain."""
    x=np.asarray(x,dtype=np.float64)
    x=x-np.mean(x,axis=0)
    x=fade(x,sr)
    # Soft guard only for unexpected extreme spikes, keeping transient dynamics.
    peak=np.max(np.abs(x))
    if peak>8: x=x*(8/peak)
    tp=true_peak_db(x)
    ceiling=10**(ceiling_db/20)
    scale=min(1.,ceiling/(10**(tp/20)))
    x=x*scale
    return x

def main():
    ap=argparse.ArgumentParser(description='DiskLordz Drum Forge: original synthesis-only commercial-ready kit foundation')
    ap.add_argument('--preset',choices=(*PRESETS,'all'),default='all')
    ap.add_argument('--variations',type=int,default=3)
    ap.add_argument('--sr',type=int,choices=(44100,48000,96000),default=44100)
    ap.add_argument('--seed',type=int,default=808660)
    ap.add_argument('--out',default='DiskLordz_Drum_Forge')
    ap.add_argument('--ceiling',type=float,default=-1.0,help='true-peak ceiling in dBTP, default -1')
    args=ap.parse_args()
    if not 1<=args.variations<=100: ap.error('--variations must be between 1 and 100')
    if not -12<=args.ceiling<=-0.1: ap.error('--ceiling must be between -12 and -0.1 dBTP')
    root=Path(args.out)
    presets=list(PRESETS) if args.preset=='all' else [args.preset]
    rows=[]
    for pi,preset in enumerate(presets):
        p=PRESETS[preset]
        for fi,family in enumerate(FAMILIES):
            for i in range(args.variations):
                seed=args.seed+pi*100000+fi*1000+i*17
                rng=np.random.default_rng(seed)
                midi=int(rng.choice([24,26,28,29,31,33,35,36,38,40,41,43]))
                x,root_hz=(gen_808(args.sr,rng,p,midi) if family=='808' else GENERATORS[family](args.sr,rng,p))
                if x.ndim==1 and family not in ('808','kick'):
                    x=quantize_character(x,args.sr,rng,p['lofi'])
                # 808 root tags come from the oscillator, independently verify tail where possible.
                note=hz_to_note(root_hz) if root_hz else ''
                detected,confidence=estimate_808_pitch(x,args.sr,root_hz) if root_hz else (None,0)
                valid=detected is not None and abs(12*np.log2(detected/root_hz))<.45 and confidence>=.5
                x=master(x,args.sr,args.ceiling)
                suffix=f'_{note}' if note else ''
                name=f'DiskLordz_{preset}_{family}{suffix}_V{i+1:03d}.wav'
                target=root/preset/family/name
                target.parent.mkdir(parents=True,exist_ok=True)
                sf.write(target,x,args.sr,subtype='PCM_24')
                tp=true_peak_db(x)
                if tp>args.ceiling+.03: raise RuntimeError(f'True peak ceiling failed: {target}: {tp:.2f}')
                rows.append({'file':str(target.relative_to(root)),'preset':preset,'family':family,
                  'seed':seed,'sample_rate':args.sr,'channels':(1 if x.ndim==1 else 2),
                  'frames':len(x),'root_note':note,'root_hz':round(root_hz,3) if root_hz else '',
                  'detected_hz':round(detected,3) if detected else '',
                  'pitch_confidence':round(confidence,3),'pitch_verified':valid if root_hz else '',
                  'sample_peak_dbfs':round(20*np.log10(max(np.max(np.abs(x)),1e-12)),2),
                  'true_peak_dbtp':round(tp,2)})
                print(f'[{len(rows):03}] {target} | TP {tp:.2f} dBTP')
    with (root/'manifest.csv').open('w',newline='',encoding='utf-8') as f:
        writer=csv.DictWriter(f,fieldnames=rows[0].keys());writer.writeheader();writer.writerows(rows)
    with (root/'manifest.json').open('w',encoding='utf-8') as f:json.dump({'engine':'DiskLordz Drum Forge v2',
        'parameters':vars(args),'files':rows},f,indent=2)
    (root/'README.txt').write_text('DiskLordz Drum Forge v2 — original, procedural, synthesis-only samples.\n'
        '24-bit PCM WAV. 808s are root-tagged from known oscillator tuning; independent analysis in manifest.\n'
        'True-peak constrained gain with 4x interpolation, not a dynamic lookahead limiter.\n'
        'Test in your DAW and audition at different loudness levels before commercial release.\n',encoding='utf-8')
    print(f'Created {len(rows)} samples in {root.resolve()}')

if __name__=='__main__':main()
