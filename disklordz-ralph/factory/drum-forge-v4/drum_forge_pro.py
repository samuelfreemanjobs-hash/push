#!/usr/bin/env python3
"""DiskLordz Drum Forge PRO 3.0 (offline synthesis-only batch kit factory).
Requires numpy, scipy, soundfile and forge_core_v2.py in the same directory.

Example: python drum_forge_pro.py --genres memphis drift_phonk --variations 4 --out Kit
"""
from __future__ import annotations
import argparse, csv, hashlib, json, math
from dataclasses import asdict, dataclass
from pathlib import Path
import numpy as np
import soundfile as sf
from scipy.signal import resample_poly
import forge_core_v2 as core

@dataclass
class Quality:
    finite: bool
    non_silent: bool
    true_peak_dbtp: float
    dc: float
    duration_s: float
    crest_db: float
    rms_dbfs: float
    pitch_verified: bool | None
    spectral_centroid_hz: float
    transient_ratio: float
    accepted: bool
    reason: str

def mutate_preset(p: dict, rng, family: str, strength: float) -> tuple[dict, dict]:
    """Musical variations: restrict the range of relevant parameters per instrument."""
    p=p.copy()
    adjustments={}
    ranges={'808': {'808_decay':(.65,1.5),'808_drive':(.64,1.55),'knock':(.65,1.45)},
            'kick': {'knock':(.60,1.65)},
            'snare': {'snare_air':(.60,1.55),'verb':(.50,1.8)},
            'clap': {'verb':(.35,2.0)},
            'closed_hat': {'hat_bright':(.65,1.55)},
            'open_hat': {'hat_bright':(.65,1.55)},
            'cowbell': {'lofi':(.65,1.35)},
            'perc': {'lofi':(.65,1.35)}}
    for param,(lo,hi) in ranges[family].items():
        # strength=0 gives the original genre voice; strength=1 full spread.
        factor=1+strength*(float(rng.uniform(lo,hi))-1)
        p[param]=max(0,p[param]*factor)
        adjustments[param]=round(factor,4)
    return p,adjustments

def descriptors(x,sr):
    mono=np.mean(x,axis=1) if x.ndim==2 else x
    # compare attack and body spectra, plus duration; scale-independent.
    def bands(y):
        if not len(y): return np.zeros(10)
        n=min(len(y),int(.35*sr))
        z=np.abs(np.fft.rfft(y[:n]*np.hanning(n),n=max(2048,2**int(np.ceil(np.log2(n))))))**2
        frequencies=np.fft.rfftfreq(max(2048,2**int(np.ceil(np.log2(n)))),1/sr)
        edges=np.geomspace(25,min(18000,sr*.46),11)
        energies=np.array([np.sum(z[(frequencies>=edges[i]) & (frequencies<edges[i+1])]) for i in range(10)])
        return np.log1p(energies)
    spectrum=bands(mono)
    spectrum=(spectrum-spectrum.mean())/(np.linalg.norm(spectrum-spectrum.mean())+1e-8)
    attack=mono[:max(1,int(.035*sr))]
    sustain=mono[max(1,int(.035*sr)):max(2,int(.2*sr))]
    attack_rms=np.sqrt(np.mean(attack**2)+1e-12)
    sustain_rms=np.sqrt(np.mean(sustain**2)+1e-12) if len(sustain) else 1e-6
    transient_ratio=float(np.clip(attack_rms/sustain_rms,0,30))
    mag=np.abs(np.fft.rfft(mono[:min(len(mono),sr)]))
    fr=np.fft.rfftfreq(min(len(mono),sr),1/sr)
    centroid=float(np.sum(fr*mag)/(np.sum(mag)+1e-12))
    feature=np.r_[spectrum, np.log1p(transient_ratio)/5, np.log1p(centroid)/10, np.log1p(len(mono)/sr)/5]
    feature=feature/(np.linalg.norm(feature)+1e-10)
    return feature,centroid,transient_ratio

def assess(x,sr,ceiling,root_hz,centroid,transient):
    finite=bool(np.all(np.isfinite(x)))
    if not finite:
        return Quality(False,False,float('nan'),float('nan'),len(x)/sr,0,0,None,centroid,transient,False,'nonfinite')
    peak=float(np.max(np.abs(x)))
    rms=float(np.sqrt(np.mean(x*x)))
    dc=float(np.max(np.abs(np.mean(x,axis=0))))
    tp=core.true_peak_db(x)
    crest=20*np.log10(max(peak,1e-12)/max(rms,1e-12))
    detected,conf=core.estimate_808_pitch(x,sr,root_hz) if root_hz else (None,0)
    pitch_ok=(detected is not None and abs(12*np.log2(detected/root_hz))<.5 and conf>=.48) if root_hz else None
    problems=[]
    if peak<.01 or rms<.0008:problems.append('silent_or_too_quiet')
    if tp>ceiling+.04:problems.append('true_peak')
    if dc>.01:problems.append('dc_offset')
    if pitch_ok is False:problems.append('pitch_unverified')
    return Quality(finite,peak>=.01,tp,dc,len(x)/sr,crest,20*np.log10(max(rms,1e-12)),pitch_ok,centroid,transient,not problems,','.join(problems))

def generate(sr,genre,family,seed,strength,ceiling):
    rng=np.random.default_rng(seed)
    p,changes=mutate_preset(core.PRESETS[genre],rng,family,strength)
    # A fixed target note family is deliberate, as random note mutation harms likeness comparisons.
    midi=int(rng.choice([24,26,28,29,31,33,35,36,38,40,41,43]))
    x,root=(core.gen_808(sr,rng,p,midi) if family=='808' else core.GENERATORS[family](sr,rng,p))
    if x.ndim==1 and family not in ('808','kick'):
        x=core.quantize_character(x,sr,rng,p['lofi'])
    x=core.master(x,sr,ceiling)
    vec,centroid,transient=descriptors(x,sr)
    q=assess(x,sr,ceiling,root,centroid,transient)
    return x,root,vec,q,changes

def main():
    a=argparse.ArgumentParser(description=__doc__)
    a.add_argument('--genres',nargs='+',choices=list(core.PRESETS),default=list(core.PRESETS))
    a.add_argument('--families',nargs='+',choices=list(core.FAMILIES),default=list(core.FAMILIES))
    a.add_argument('--variations',type=int,default=4)
    a.add_argument('--strength',type=float,default=.85)
    a.add_argument('--max-retries',type=int,default=10)
    a.add_argument('--similarity',type=float,default=.9995,help='Reject within-family near-duplicates when descriptor cosine exceeds threshold')
    a.add_argument('--seed',type=int,default=660808)
    a.add_argument('--sr',type=int,choices=(44100,48000,96000),default=48000)
    a.add_argument('--ceiling',type=float,default=-1.0)
    a.add_argument('--out',default='DiskLordz_Forge_PRO_Kit')
    args=a.parse_args()
    if not 1<=args.variations<=100:a.error('variations must be 1..100')
    if not 0<=args.strength<=1:a.error('strength must be 0..1')
    if not .8<=args.similarity<=1:a.error('similarity must be .8..1')
    if not -12<=args.ceiling<=-.1:a.error('ceiling must be -12..-.1')
    if not 1<=args.max_retries<=100:a.error('max-retries must be 1..100')
    root=Path(args.out);root.mkdir(parents=True,exist_ok=True)
    accepted=[];rejected=[]
    for gi,genre in enumerate(args.genres):
      for fi,family in enumerate(args.families):
        seen=[]
        for variation in range(args.variations):
          winner=None
          for attempt in range(args.max_retries):
            seed=args.seed+gi*10000000+fi*100000+variation*1000+attempt*37
            x,pitch,vec,q,changes=generate(args.sr,genre,family,seed,args.strength,args.ceiling)
            similarity=max((float(np.dot(vec,prior)) for prior in seen),default=-1.0)
            if q.accepted and similarity<args.similarity:
                winner=(x,pitch,vec,q,changes,seed,similarity)
                break
            rejected.append({'genre':genre,'family':family,'variation':variation+1,'attempt':attempt+1,'seed':seed,'reason':q.reason if not q.accepted else 'near_duplicate','similarity':round(similarity,6)})
          if winner is None:
            print(f'[SKIP] {genre}/{family} V{variation+1:03d}: quality/diversity target not reached')
            continue
          x,pitch,vec,q,changes,seed,sim=winner
          note=f'_{core.hz_to_note(pitch).replace("#","s")}' if pitch else ''
          relative=Path(genre)/family/f'DiskLordz_{genre}_{family}{note}_V{variation+1:03d}.wav'
          dest=root/relative;dest.parent.mkdir(parents=True,exist_ok=True)
          sf.write(dest,x,args.sr,subtype='PCM_24')
          digest=hashlib.sha256(dest.read_bytes()).hexdigest()
          record={'file':str(relative).replace('\\','/'),'genre':genre,'family':family,'variation':variation+1,
            'seed':seed,'root_note':core.hz_to_note(pitch) if pitch else '',
            'root_hz':round(pitch,5) if pitch else '', 'sample_rate':args.sr,
            'channels':1 if x.ndim==1 else x.shape[1], 'frames':len(x),
            'max_similarity':round(sim,6) if sim>=0 else '', 'dna_changes':json.dumps(changes),
            **asdict(q),'sha256':digest}
          accepted.append(record)
          print(f'[PASS] {relative}  TP={q.true_peak_dbtp:.2f} dBTP  DNA={changes}')
    with (root/'manifest.csv').open('w',encoding='utf8',newline='') as f:
        if accepted:
            w=csv.DictWriter(f,fieldnames=list(accepted[0]));w.writeheader();w.writerows(accepted)
    (root/'manifest.json').write_text(json.dumps({'engine':'DiskLordz Drum Forge PRO 3.0','settings':vars(args),'accepted':accepted,'rejected':rejected},indent=2),encoding='utf8')
    with (root/'rejected.csv').open('w',encoding='utf8',newline='') as f:
        if rejected:
            w=csv.DictWriter(f,fieldnames=list(rejected[0]));w.writeheader();w.writerows(rejected)
    (root/'README.txt').write_text('DiskLordz Drum Forge PRO 3.0 — all sounds procedurally synthesized.\n'
      '24-bit WAV; 808 key is taken from oscillator root and independently verified.\n'
      'Samples pass per-file QC for peak, pitch (808), DC and silence. Similarity is a heuristic, not perceptual proof.\n'
      'True-peak control uses oversampled measurement + static gain reduction, NOT dynamic limiting.\n'
      'Review/DAW audition, device translation and rights/trademark checks required before retail launch.\n',encoding='utf8')
    print(f'Completed {len(accepted)} samples; {len(rejected)} rejected attempts. Output: {root}')

if __name__=='__main__':main()
