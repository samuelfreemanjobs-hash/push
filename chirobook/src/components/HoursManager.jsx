import { useState, useEffect } from 'react';
import { doc, getDoc, setDoc } from 'firebase/firestore';
import { db } from '../firebase';

const DAYS = ['monday', 'tuesday', 'wednesday', 'thursday', 'friday', 'saturday', 'sunday'];
const LABELS = {
  monday: 'Monday', tuesday: 'Tuesday', wednesday: 'Wednesday',
  thursday: 'Thursday', friday: 'Friday', saturday: 'Saturday', sunday: 'Sunday',
};
const DEFAULT_HOURS = {
  monday:    { enabled: true,  open: '08:00', close: '18:00' },
  tuesday:   { enabled: true,  open: '08:00', close: '18:00' },
  wednesday: { enabled: true,  open: '08:00', close: '18:00' },
  thursday:  { enabled: true,  open: '08:00', close: '18:00' },
  friday:    { enabled: true,  open: '08:00', close: '17:00' },
  saturday:  { enabled: false, open: '09:00', close: '13:00' },
  sunday:    { enabled: false, open: '09:00', close: '13:00' },
};

export default function HoursManager({ userId }) {
  const [hours, setHours] = useState(DEFAULT_HOURS);
  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [saved, setSaved] = useState(false);

  useEffect(() => {
    async function load() {
      const snap = await getDoc(doc(db, 'practices', userId));
      if (snap.exists() && snap.data().hours) {
        setHours({ ...DEFAULT_HOURS, ...snap.data().hours });
      }
      setLoading(false);
    }
    load();
  }, [userId]);

  function update(day, field, value) {
    setHours((h) => ({ ...h, [day]: { ...h[day], [field]: value } }));
  }

  async function handleSave() {
    setSaving(true);
    await setDoc(doc(db, 'practices', userId), { hours }, { merge: true });
    setSaving(false);
    setSaved(true);
    setTimeout(() => setSaved(false), 2000);
  }

  if (loading) return <div className="animate-pulse h-64 bg-slate-100 rounded-xl" />;

  return (
    <div className="bg-white rounded-xl p-6 shadow-sm max-w-lg">
      <h2 className="text-lg font-semibold text-slate-900 mb-6">Office Hours</h2>
      <div className="space-y-4">
        {DAYS.map((day) => (
          <div key={day} className="flex items-center gap-4">
            <label className="flex items-center gap-2 w-32 cursor-pointer">
              <input
                type="checkbox"
                checked={hours[day].enabled}
                onChange={(e) => update(day, 'enabled', e.target.checked)}
                className="rounded text-teal-600 focus:ring-teal-500"
              />
              <span className={`text-sm font-medium ${hours[day].enabled ? 'text-slate-900' : 'text-slate-400'}`}>
                {LABELS[day]}
              </span>
            </label>
            {hours[day].enabled ? (
              <div className="flex items-center gap-2 text-sm">
                <input type="time" value={hours[day].open}
                  onChange={(e) => update(day, 'open', e.target.value)}
                  className="border rounded-lg px-2 py-1.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500" />
                <span className="text-slate-400">–</span>
                <input type="time" value={hours[day].close}
                  onChange={(e) => update(day, 'close', e.target.value)}
                  className="border rounded-lg px-2 py-1.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500" />
              </div>
            ) : (
              <span className="text-sm text-slate-400">Closed</span>
            )}
          </div>
        ))}
      </div>
      <button
        onClick={handleSave}
        disabled={saving}
        className="mt-8 bg-teal-600 text-white px-6 py-2.5 rounded-lg text-sm font-medium hover:bg-teal-700 disabled:opacity-50"
      >
        {saving ? 'Saving…' : saved ? '✓ Saved' : 'Save Hours'}
      </button>
    </div>
  );
}
