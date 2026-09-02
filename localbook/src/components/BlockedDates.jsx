import { useState, useEffect } from 'react';
import { collection, getDocs, doc, setDoc, deleteDoc } from 'firebase/firestore';
import { db } from '../firebase';

const MONTH_NAMES = [
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

  const colRef = collection(db, 'businesses', userId, 'blockedDates');

  useEffect(() => {
    async function load() {
      const snap = await getDocs(colRef);
      setBlocked(new Set(snap.docs.map((d) => d.id)));
      setLoading(false);
    }
    load();
  }, [userId]);

  async function toggleDate(dateStr) {
    const ref = doc(db, 'businesses', userId, 'blockedDates', dateStr);
    if (blocked.has(dateStr)) {
      await deleteDoc(ref);
      setBlocked((b) => { const next = new Set(b); next.delete(dateStr); return next; });
    } else {
      await setDoc(ref, { date: dateStr, blockedAt: new Date().toISOString() });
      setBlocked((b) => new Set(b).add(dateStr));
    }
  }

  function prevMonth() {
    setCalMonth((m) => {
      const d = new Date(m.year, m.month - 1);
      return { year: d.getFullYear(), month: d.getMonth() };
    });
  }

  function nextMonth() {
    setCalMonth((m) => {
      const d = new Date(m.year, m.month + 1);
      return { year: d.getFullYear(), month: d.getMonth() };
    });
  }

  function formatDate(year, month, day) {
    return `${year}-${String(month + 1).padStart(2, '0')}-${String(day).padStart(2, '0')}`;
  }

  const today = new Date().toISOString().slice(0, 10);
  const daysInMonth = new Date(calMonth.year, calMonth.month + 1, 0).getDate();
  const firstDay = new Date(calMonth.year, calMonth.month, 1).getDay();

  if (loading) return <div className="animate-pulse h-64 bg-gray-100 rounded-xl" />;

  return (
    <div className="bg-white rounded-xl p-6 shadow-sm max-w-sm">
      <h2 className="text-lg font-semibold text-gray-900 mb-1">Blocked Dates</h2>
      <p className="text-xs text-gray-500 mb-5">
        Click any date to block it. Blocked dates won't appear on your booking page.
      </p>

      <div className="flex items-center justify-between mb-4">
        <button onClick={prevMonth} className="p-1.5 hover:bg-gray-100 rounded-lg text-gray-600">‹</button>
        <span className="font-semibold text-gray-900 text-sm">
          {MONTH_NAMES[calMonth.month]} {calMonth.year}
        </span>
        <button onClick={nextMonth} className="p-1.5 hover:bg-gray-100 rounded-lg text-gray-600">›</button>
      </div>

      <div className="grid grid-cols-7 gap-1 text-center text-xs text-gray-400 mb-2">
        {['Su', 'Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa'].map((d) => (
          <div key={d} className="py-1">{d}</div>
        ))}
      </div>

      <div className="grid grid-cols-7 gap-1">
        {Array.from({ length: firstDay }).map((_, i) => <div key={`e-${i}`} />)}
        {Array.from({ length: daysInMonth }, (_, i) => i + 1).map((day) => {
          const dateStr = formatDate(calMonth.year, calMonth.month, day);
          const isPast = dateStr < today;
          const isBlocked = blocked.has(dateStr);
          return (
            <button
              key={day}
              onClick={() => !isPast && toggleDate(dateStr)}
              disabled={isPast}
              title={isBlocked ? `${dateStr} — blocked` : dateStr}
              className={`aspect-square rounded-lg text-sm font-medium transition-colors ${
                isPast
                  ? 'text-gray-200 cursor-default'
                  : isBlocked
                  ? 'bg-red-100 text-red-700 hover:bg-red-200'
                  : 'hover:bg-gray-100 text-gray-700'
              }`}
            >
              {day}
            </button>
          );
        })}
      </div>

      {blocked.size > 0 && (
        <p className="mt-4 text-xs text-gray-500">
          {blocked.size} date{blocked.size !== 1 ? 's' : ''} currently blocked
        </p>
      )}
    </div>
  );
}
