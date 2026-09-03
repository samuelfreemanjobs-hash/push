import { useState, useEffect } from 'react';
import { collection, getDocs, doc, updateDoc, query, orderBy } from 'firebase/firestore';
import { db } from '../firebase';
import { formatTime } from '../utils/slots';

function fmtDate(ds) {
  return new Date(ds + 'T00:00:00').toLocaleDateString('en-US', {
    weekday: 'short', month: 'short', day: 'numeric',
  });
}

function PayBadge({ type }) {
  if (type === 'insurance') return (
    <span className="inline-flex items-center text-xs bg-blue-100 text-blue-700 px-2 py-0.5 rounded-full font-medium">
      Insurance
    </span>
  );
  return (
    <span className="inline-flex items-center text-xs bg-emerald-100 text-emerald-700 px-2 py-0.5 rounded-full font-medium">
      Cash
    </span>
  );
}

export default function AppointmentsView({ userId }) {
  const [bookings, setBookings] = useState([]);
  const [loading, setLoading] = useState(true);
  const [filter, setFilter] = useState('upcoming');
  const [expanded, setExpanded] = useState(null);

  async function load() {
    const q = query(
      collection(db, 'practices', userId, 'bookings'),
      orderBy('date'),
      orderBy('time')
    );
    const snap = await getDocs(q);
    setBookings(snap.docs.map((d) => ({ id: d.id, ...d.data() })));
    setLoading(false);
  }

  useEffect(() => { load(); }, [userId]);

  async function handleCancel(id) {
    if (!window.confirm('Cancel this appointment? The patient will not be automatically notified.')) return;
    await updateDoc(doc(db, 'practices', userId, 'bookings', id), { status: 'cancelled' });
    await load();
  }

  async function handleComplete(id) {
    await updateDoc(doc(db, 'practices', userId, 'bookings', id), { status: 'completed' });
    await load();
  }

  const today = new Date().toISOString().slice(0, 10);

  const filtered = bookings.filter((b) => {
    if (filter === 'upcoming') return b.status !== 'cancelled' && b.date >= today;
    if (filter === 'insurance') return b.paymentType === 'insurance' && b.status !== 'cancelled';
    if (filter === 'cash') return (b.paymentType === 'cash' || !b.paymentType) && b.status !== 'cancelled';
    return true;
  });

  const upcoming = bookings.filter((b) => b.status !== 'cancelled' && b.date >= today);
  const insuranceCount = upcoming.filter((b) => b.paymentType === 'insurance').length;
  const cashCount = upcoming.filter((b) => b.paymentType === 'cash' || !b.paymentType).length;

  if (loading) return <div className="animate-pulse h-64 bg-slate-100 rounded-xl" />;

  return (
    <div className="space-y-5 max-w-3xl">
      {/* Stats row */}
      {upcoming.length > 0 && (
        <div className="grid grid-cols-3 gap-4">
          {[
            { label: 'Upcoming', value: upcoming.length, color: 'text-slate-900' },
            { label: 'Insurance', value: insuranceCount, color: 'text-blue-700' },
            { label: 'Cash Pay', value: cashCount, color: 'text-emerald-700' },
          ].map((s) => (
            <div key={s.label} className="bg-white rounded-xl p-4 shadow-sm text-center">
              <p className={`text-2xl font-bold ${s.color}`}>{s.value}</p>
              <p className="text-xs text-slate-500 mt-0.5">{s.label}</p>
            </div>
          ))}
        </div>
      )}

      {/* Filter bar */}
      <div className="flex items-center gap-3">
        <h2 className="text-lg font-semibold text-slate-900 flex-1">Appointments</h2>
        <div className="flex rounded-lg border overflow-hidden text-sm">
          {[
            { id: 'upcoming', label: 'Upcoming' },
            { id: 'insurance', label: 'Insurance' },
            { id: 'cash', label: 'Cash' },
            { id: 'all', label: 'All' },
          ].map((f) => (
            <button key={f.id} onClick={() => setFilter(f.id)}
              className={`px-3 py-1.5 ${filter === f.id ? 'bg-teal-600 text-white' : 'text-slate-600 hover:bg-slate-50'}`}>
              {f.label}
            </button>
          ))}
        </div>
      </div>

      <div className="bg-white rounded-xl shadow-sm overflow-hidden">
        {filtered.length === 0 ? (
          <p className="p-10 text-center text-slate-400 text-sm">
            No appointments to show.
          </p>
        ) : (
          <ul className="divide-y">
            {filtered.map((b) => {
              const isExpanded = expanded === b.id;
              return (
                <li key={b.id} className={`${b.status === 'cancelled' ? 'opacity-50' : b.status === 'completed' ? 'opacity-70' : ''}`}>
                  <div
                    className="px-5 py-4 flex items-start justify-between gap-4 cursor-pointer hover:bg-slate-50"
                    onClick={() => setExpanded(isExpanded ? null : b.id)}
                  >
                    <div className="space-y-1 min-w-0">
                      <div className="flex items-center gap-2 flex-wrap">
                        <p className="font-medium text-slate-900">{b.patientName}</p>
                        {b.status === 'cancelled' && (
                          <span className="text-xs bg-red-100 text-red-600 px-2 py-0.5 rounded-full">Cancelled</span>
                        )}
                        {b.status === 'completed' && (
                          <span className="text-xs bg-slate-100 text-slate-500 px-2 py-0.5 rounded-full">Completed</span>
                        )}
                        <PayBadge type={b.paymentType} />
                        {b.packageCode && (
                          <span className="text-xs bg-teal-100 text-teal-700 px-2 py-0.5 rounded-full">Package</span>
                        )}
                      </div>
                      <p className="text-sm text-slate-600">
                        {b.serviceName} · {b.serviceDuration} min
                        {b.servicePrice != null ? ` · $${b.servicePrice}` : ''}
                      </p>
                      <p className="text-sm text-slate-500">{fmtDate(b.date)} at {formatTime(b.time)}</p>
                    </div>
                    <span className="text-slate-400 text-sm flex-shrink-0">{isExpanded ? '▲' : '▼'}</span>
                  </div>

                  {isExpanded && (
                    <div className="px-5 pb-4 bg-slate-50 border-t space-y-3">
                      <div className="grid grid-cols-2 gap-x-4 gap-y-1.5 text-sm pt-3">
                        {[
                          ['Email', b.patientEmail],
                          ['Phone', b.patientPhone],
                          b.paymentType === 'insurance' && ['Carrier', b.insuranceCarrier],
                          b.paymentType === 'insurance' && ['Member ID', b.memberId],
                          b.packageCode && ['Package Code', b.packageCode],
                          b.packageName && ['Package', b.packageName],
                        ].filter(Boolean).map(([label, value]) => value ? (
                          <div key={label}>
                            <span className="text-slate-500">{label}: </span>
                            <span className="font-medium text-slate-800">{value}</span>
                          </div>
                        ) : null)}
                      </div>
                      {b.intakeNotes && (
                        <div className="text-sm">
                          <p className="text-slate-500 mb-0.5">Intake notes:</p>
                          <p className="text-slate-800 bg-white border rounded-lg px-3 py-2">{b.intakeNotes}</p>
                        </div>
                      )}
                      {b.status === 'confirmed' && (
                        <div className="flex gap-3 pt-1">
                          <button onClick={() => handleComplete(b.id)}
                            className="text-sm bg-teal-600 text-white px-4 py-1.5 rounded-lg hover:bg-teal-700">
                            Mark Complete
                          </button>
                          <button onClick={() => handleCancel(b.id)}
                            className="text-sm text-red-500 hover:underline">
                            Cancel
                          </button>
                        </div>
                      )}
                    </div>
                  )}
                </li>
              );
            })}
          </ul>
        )}
      </div>
    </div>
  );
}
