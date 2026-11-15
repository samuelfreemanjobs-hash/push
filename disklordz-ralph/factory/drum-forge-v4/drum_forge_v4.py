#!/usr/bin/env python3
"""DiskLordz DRUM FORGE 4: reproducible, curated synthesis-only sample factory.

Run: python drum_forge_v4.py --genres memphis drift_phonk --variations 4 --notes C1 D1 F1
Requires: numpy, scipy, soundfile, forge_core_v2.py, drum_forge_pro.py
"""
from __future__ import annotations
import argparse, csv, hashlib, html, json, math
from dataclasses import asdict
from pathlib import Path
import numpy as np
import soundfile as sf
import forge_core_v2 as core
import drum_forge_pro as pro

# Extended genre voices: inspired by production aesthetics, not recordings or third-party samples.
EXTRA = {
    'southern_trap': {'808_decay':1.35,'808_drive':4.3,'knock':1.17,'snare_air':.80,'hat_bright':.92,'lofi':.06,'verb':.12},
    'cloud_trap': {'808_decay':1.67,'808_drive':2.35,'knock':.55,'snare_air':.58,'hat_bright':.66,'lofi':.025,'verb':.25},
    'raw_underground': {'808_decay':.90,'808_drive':5.7,'knock':1.06,'snare_air':.70,'hat_bright':.61,'lofi':.25,'verb':.04},
}
core.PRESETS.update(EXTRA)

NOTE_TO_PC={name:i for i,name in enumerate(core.NOTES)}
def parse_midi(note: str) -> int:
    token=note.strip().upper().replace('♯','#').replace('♭','B')
    import re
    m=re.fullmatch(r'([A-G])([#B]?)(-?\d+)',token)
    if not m: raise ValueError('expected notes like C1, F#1, Bb0')
    key,acc,octv=m.groups();pc=NOTE_TO_PC[key]
    pc+=(1 if acc=='#' else -1 if acc=='B' else 0)
    midi=(int(octv)+1)*12+pc
    if not 12<=midi<=96:raise ValueError('note outside supported range')
    return midi

def final_trim(x,sr,threshold_db=-73,release_ms=18):
    """Trim excess silence, preserving at least 18 ms after last audible energy."""
    y=np.asarray(x)
    envelope=np.max(np.abs(y),axis=1) if y.ndim==2 else np.abs(y)
    idx=np.flatnonzero(envelope > 10**(threshold_db/20))
    if not len(idx): return y
    stop=min(len(y),int(idx[-1])+1+int(sr*release_ms/1000))
    out=y[:stop].copy()
    # terminal fade only, never silence a meaningful transient
    out=core.fade(out,sr,fadein_ms=.1,fadeout_ms=min(5.,1000*len(out)/sr/10))
    return out

def spectral_balance(x,sr):
    v=np.mean(x,axis=1) if x.ndim==2 else x
    n=min(len(v),max(2048,int(.24*sr)))
    if n<32:return {'sub_pct':0.,'lowmid_pct':0.,'presence_pct':0.,'air_pct':0.}
    z=np.abs(np.fft.rfft(v[:n]*np.hanning(n)))**2
    fr=np.fft.rfftfreq(n,1/sr)
    total=float(sum(z))+1e-12
    bands={'sub_pct':(20,100),'lowmid_pct':(100,1000),'presence_pct':(1000,6000),'air_pct':(6000,min(19000,sr*.49))}
    return {label:round(100*float(sum(z[(fr>=lo)&(fr<hi)]))/total,2) if hi>lo else 0. for label,(lo,hi) in bands.items()}

def make_catalog(records,settings):
    rows='\n'.join('<tr>'+''.join(f'<td>{html.escape(str(r.get(k,"")))}</td>' for k in ('genre','family','root_note','variation','duration_s','true_peak_dbtp','rms_dbfs','file'))+'</tr>' for r in records)
    return f'''<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>DiskLordz Forge 4 — QC Catalog</title>
<style>body{{font:15px system-ui;background:#101014;color:#ececf0;max-width:1200px;margin:36px auto;padding:0 20px}} h1{{color:#f4b35b}}p{{color:#aeb0c0}} table{{border-collapse:collapse;width:100%;font-size:12px}}td,th{{padding:10px;border-bottom:1px solid #32323b;text-align:left}}th{{color:#f4b35b;position:sticky;top:0;background:#101014}}tr:hover{{background:#202029}}input{{padding:12px;width:100%;margin:10px 0 22px;background:#22222a;color:white;border:1px solid #555;border-radius:8px}}</style></head><body>
<h1>DiskLordz / DRUM FORGE 4</h1><p>Quality-control catalog · {len(records)} accepted samples · Synthesis only</p><input id="search" placeholder="Filter by genre, family, key, filename" oninput="for(const r of document.querySelectorAll('tbody tr'))r.hidden=!r.innerText.toLowerCase().includes(this.value.toLowerCase())"><table><thead><tr>{''.join('<th>'+s+'</th>' for s in ['Genre','Family','Key','Variation','Duration (s)','True peak (dBTP)','RMS (dBFS)','File'])}</tr></thead><tbody>{rows}</tbody></table><p>True-peak measurement is estimated via 4× oversampling; mastering uses static attenuation. Quality checks do not replace auditioning in a DAW.</p></body></html>'''

def main():
    ap=argparse.ArgumentParser(description='DiskLordz Drum Forge 4: synthesis-only, QC-curated commercial pack factory')
    ap.add_argument('--genres',nargs='+',choices=sorted(core.PRESETS),default=list(core.PRESETS))
    ap.add_argument('--families',nargs='+',choices=list(core.FAMILIES),default=list(core.FAMILIES))
    ap.add_argument('--variations',type=int,default=4)
    ap.add_argument('--notes',nargs='+',default=['C1','D1','F1','G1'],help='808 roots; other drum families not pitched to these notes')
    ap.add_argument('--strength',type=float,default=.90)
    ap.add_argument('--max-retries',type=int,default=12)
    ap.add_argument('--similarity',type=float,default=.9995)
    ap.add_argument('--seed',type=int,default=660808)
    ap.add_argument('--sr',type=int,choices=(44100,48000,96000),default=48000)
    ap.add_argument('--bits',type=int,choices=(16,24),default=24)
    ap.add_argument('--ceiling',type=float,default=-1.0)
    ap.add_argument('--out',default='DiskLordz_Forge_4_Kit')
    args=ap.parse_args()
    if not 1<=args.variations<=100:ap.error('variations must be 1..100')
    if not 0<=args.strength<=1:ap.error('strength must be 0..1')
    if not .8<=args.similarity<=1:ap.error('similarity must be .8..1')
    if not -12<=args.ceiling<=-.1:ap.error('ceiling must be -12..-.1')
    if not 1<=args.max_retries<=100:ap.error('max retries must be 1..100')
    try: midis=list(dict.fromkeys(parse_midi(n) for n in args.notes))
    except ValueError as exc:ap.error(str(exc))
    if not midis:ap.error('at least one note is required')
    root=Path(args.out);root.mkdir(parents=True,exist_ok=True)
    accepted=[];rejected=[];completed=0
    for gi,genre in enumerate(args.genres):
      for fi,family in enumerate(args.families):
        targets=midis if family=='808' else [None]
        for ni,midi in enumerate(targets):
          seen=[]
          for vi in range(args.variations):
            selected=None
            for attempt in range(args.max_retries):
              seed=args.seed+gi*100000000+fi*1000000+ni*10000+vi*100+attempt
              rng=np.random.default_rng(seed)
              p,changes=pro.mutate_preset(core.PRESETS[genre],rng,family,args.strength)
              if family=='808':x,root_hz=core.gen_808(args.sr,rng,p,midi)
              else:x,root_hz=core.GENERATORS[family](args.sr,rng,p)
              if x.ndim==1 and family not in ('808','kick'):
                  x=core.quantize_character(x,args.sr,rng,p['lofi'])
              x=final_trim(x,args.sr)
              x=core.master(x,args.sr,args.ceiling-.12)  # .12 dB extra margin for PCM output
              vec,centroid,transient=pro.descriptors(x,args.sr)
              qc=pro.assess(x,args.sr,args.ceiling,root_hz,centroid,transient)
              sim=max((float(np.dot(vec,past)) for past in seen),default=-1.)
              if qc.accepted and sim<args.similarity:
                selected=(x,root_hz,vec,qc,changes,seed,sim)
                break
              rejected.append({'genre':genre,'family':family,'root_note':core.hz_to_note(root_hz) if root_hz else '', 'variation':vi+1,'attempt':attempt+1,'seed':seed,'reason':qc.reason if not qc.accepted else 'near_duplicate','similarity':round(sim,6)})
            if selected is None:
                print(f'[SKIP] {genre}/{family}/{core.hz_to_note(core.note_to_hz(midi)) if midi else ""} V{vi+1:03d}')
                continue
            x,root_hz,vec,qc,changes,seed,sim=selected
            note=core.hz_to_note(root_hz).replace('#','s') if root_hz else None
            name=f'DiskLordz_{genre}_{family}_{note+"_" if note else ""}V{vi+1:03d}.wav'
            rel=Path(genre)/family/name
            dest=root/rel;dest.parent.mkdir(parents=True,exist_ok=True)
            sf.write(dest,x,args.sr,subtype='PCM_24' if args.bits==24 else 'PCM_16')
            rendered,sr_read=sf.read(dest,always_2d=False,dtype='float64')
            actual_tp=core.true_peak_db(rendered)
            if sr_read!=args.sr or not np.isfinite(actual_tp) or actual_tp>args.ceiling+.03:
                dest.unlink(missing_ok=True)
                rejected.append({'genre':genre,'family':family,'root_note':note or '', 'variation':vi+1,'attempt':'postexport','seed':seed,'reason':'rendered_peak_or_sample_rate','similarity':round(sim,6)})
                continue
            qcdict=asdict(qc)
            qcdict['true_peak_dbtp']=round(actual_tp,3)
            rec={'file':rel.as_posix(),'genre':genre,'family':family,'root_note':core.hz_to_note(root_hz) if root_hz else '', 'root_hz':round(root_hz,5) if root_hz else '', 'variation':vi+1,'seed':seed,'sample_rate':args.sr,'bit_depth':args.bits,'channels':1 if x.ndim==1 else x.shape[1],'frames':len(x),'dna_changes':json.dumps(changes),'similarity':round(sim,6) if sim>=0 else '',**qcdict,**spectral_balance(rendered,args.sr),'sha256':hashlib.sha256(dest.read_bytes()).hexdigest()}
            accepted.append(rec);seen.append(vec);completed+=1
            print(f'[PASS] {rel.as_posix()} TP {actual_tp:.2f} dBTP')
    if accepted:
        with (root/'manifest.csv').open('w',encoding='utf-8',newline='') as f:
            w=csv.DictWriter(f,fieldnames=list(accepted[0]));w.writeheader();w.writerows(accepted)
    if rejected:
        with (root/'rejected.csv').open('w',encoding='utf-8',newline='') as f:
            w=csv.DictWriter(f,fieldnames=list(rejected[0]));w.writeheader();w.writerows(rejected)
    (root/'manifest.json').write_text(json.dumps({'engine':'DiskLordz DRUM FORGE 4','settings':vars(args),'count':completed,'accepted':accepted,'rejected':rejected},indent=2),encoding='utf-8')
    (root/'catalog.html').write_text(make_catalog(accepted,vars(args)),encoding='utf-8')
    (root/'README.txt').write_text('DiskLordz DRUM FORGE 4 | 100% synthetic sounds, no recordings used.\n'
      'Folders are organized by genre and family. 808 root labels are known from synthesis and independently tested by tail autocorrelation.\n'
      'All successful WAVs are reopened and peak-checked after encoding.\n'
      'IMPORTANT: True-peak check is 4x approximation; static gain reduction, not lookahead limiting.\n'
      'Similarity screening is approximate. Human audition and DAW compatibility testing are required before sale.\n',encoding='utf-8')
    print(f'Finished: {completed} accepted, {len(rejected)} rejected, catalog at {root/"catalog.html"}')

if __name__=='__main__':main()
