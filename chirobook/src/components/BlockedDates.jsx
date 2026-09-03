import { useState, useEffect } from 'react';
import { collection, getDocs, doc, setDoc, deleteDoc } from 'firebase/firestore';
import { db } from '../firebase';

const MONTHS = [
  'January','February','March','April','May','June',
  'July','August','September','October','November','December',
];

export default function BlockedDates({ userId }) {
  const [blocked, setBlocked] = useState(new Set());
  const [loading, setLoading] = useState(true);
  const [calMonth, setCalMonth] = useState(() => {
    const now = new Date();
    return { year: now.getFullYear(), month: now.getMonth() };
  });

  const colRef = collection(db, 'practices', userId, 'blockedDates');

  useEffect(() => {
    async function load() {
      const snap = await getDocs(colRef);
      setBlocked(new Set(snap.docs.map((d) => d.id)));
      setLoading(false);
    }
    load();
  }, [userId]);

  async function toggleDate(ds) {
    const ref = doc(db, 'practices', userId, 'blockedDates', ds);
    if (blocked.has(ds)) {
      await deleteDoc(ref);
      setBlocked((b) => { const n = new Set(b); n.delete(ds); return n; });
    } else {
      await setDoc(ref, { date: ds, blockedAt: new Date().toISOString() });
      setBlocked((b) => new Set(b).add(ds));
    }
  }

  function fmt(y, m, d) {
    return `${y}-${String(m + 1).padStart(2, '0')}-${String(d).padStart(2, '0')}`;
  }

  const today = new Date().toISOString().slice(0, 10);
  const daysInMonth = new Date(calMonth.year, calMonth.month + 1, 0).getDate();
  const firstDay = new Date(calMonth.year, calMonth.month, 1).getDay();

  if (loading) return <div className="animate-pulse h-64 bg-slate-100 rounded-xl" />;

  return (
    <div className="bg-white rounded-xl p-6 shadow-sm max-w-sm">
      <h2 className="text-lg font-semibold text-slate-900 mb-1">Blocked Dates</h2>
      <p className="text-xs text-slate-500 mb-5">Click a date to block it from patient bookings.</p>
      <div className="flex items-center justify-between mb-4">
        <button onClick={() => setCalMonth((m) => { const d = new Date(m.year, m.month - 1); return { year: d.getFullYear(), month: d.getMonth() }; })}
          className="p-1.5 hover:bg-slate-100 rounded-lg text-slate-600">‹</button>
        <span className="font-semibold text-slate-900 text-sm">{MONTHS[calMonth.month]} {calMonth.year}</span>
        <button onClick={() => setCalMonth((m) => { const d = new Date(m.year, m.month + 1); return { year: d.getFullYear(), month: d.getMonth() }; })}
          className="p-1.5 hover:bg-slate-100 rounded-lg text-slate-600">›</button>
      </div>
      <div className="grid grid-cols-7 gap-1 text-center text-xs text-slate-400 mb-2">
        {['Su', 'Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa'].map((d) => <div key={d} className="py-1">{d}</div>)}
      </div>
      <div className="grid grid-cols-7 gap-1">
        {Array.from({ length: firstDay }).map((_, i) => <div key={`e-${i}`} />)}
        {Array.from({ length: daysInMonth }, (_, i) => i + 1).map((day) => {
          const ds = fmt(calMonth.year, calMonth.month, day);
          const isPast = ds < today;
          const isBlocked = blocked.has(ds);
          return (
            <button key={day} onClick={() => !isPast && toggleDate(ds)} disabled={isPast}
              className={`aspect-square rounded-lg text-sm font-medium transition-colors ${
                isPast ? 'text-slate-200 cursor-default' :
                isBlocked ? 'bg-red-100 text-red-700 hover:bg-red-200' :
                'hover:bg-slate-100 text-slate-700'
              }`}
            >
              {day}
            </button>
          );
        })}
      </div>
      {blocked.size > 0 && (
        <p className="mt-4 text-xs text-slate-500">{blocked.size} date{blocked.size !== 1 ? 's' : ''} blocked</p>
      )}
    </div>
  );
}
