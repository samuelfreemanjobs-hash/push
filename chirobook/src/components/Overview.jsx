import { useState, useEffect } from 'react';
import { collection, getDocs, query, orderBy } from 'firebase/firestore';
import { db } from '../firebase';
import { formatTime } from '../utils/slots';

function fmtDate(ds) {
  return new Date(ds + 'T00:00:00').toLocaleDateString('en-US', {
    weekday: 'short', month: 'short', day: 'numeric',
  });
}

export default function Overview({ userId }) {
  const [bookings, setBookings] = useState([]);
  const [packages, setPackages] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    async function load() {
      const [bSnap, pSnap] = await Promise.all([
        getDocs(query(collection(db, 'practices', userId, 'bookings'), orderBy('date'), orderBy('time'))),
        getDocs(collection(db, 'practices', userId, 'patientPackages')),
      ]);
      setBookings(bSnap.docs.map((d) => ({ id: d.id, ...d.data() })));
      setPackages(pSnap.docs.map((d) => ({ id: d.id, ...d.data() })));
      setLoading(false);
    }
    load();
  }, [userId]);

  if (loading) return (
    <div className="space-y-4">
      {Array.from({ length: 3 }).map((_, i) => (
        <div key={i} className="animate-pulse h-24 bg-slate-100 rounded-xl" />
      ))}
    </div>
  );

  const today = new Date().toISOString().slice(0, 10);
  const weekEnd = new Date();
  weekEnd.setDate(weekEnd.getDate() + 7);
  const weekEndStr = weekEnd.toISOString().slice(0, 10);

  const active = bookings.filter((b) => b.status !== 'cancelled');
  const todayAppts = active.filter((b) => b.date === today);
  const weekAppts = active.filter((b) => b.date >= today && b.date <= weekEndStr);
  const upcoming = active.filter((b) => b.date >= today);

  const insuranceCount = upcoming.filter((b) => b.paymentType === 'insurance').length;
  const cashCount = upcoming.filter((b) => b.paymentType === 'cash' || !b.paymentType).length;

  const activePkgs = packages.filter((p) => p.status === 'active' && (p.totalVisits - p.usedVisits) > 0);
  const totalRemainingVisits = activePkgs.reduce((sum, p) => sum + (p.totalVisits - p.usedVisits), 0);

  const STATS = [
    { label: "Today's Appointments", value: todayAppts.length, sub: today === new Date().toISOString().slice(0, 10) ? fmtDate(today) : '', color: 'text-slate-900' },
    { label: 'Next 7 Days', value: weekAppts.length, sub: 'appointments', color: 'text-slate-900' },
    { label: 'Active Packages', value: activePkgs.length, sub: `${totalRemainingVisits} visits remaining`, color: 'text-teal-700' },
    { label: 'Insurance / Cash', value: `${insuranceCount} / ${cashCount}`, sub: 'upcoming split', color: 'text-slate-900' },
  ];

  return (
    <div className="space-y-8 max-w-4xl">
      {/* Stat tiles */}
      <div className="grid grid-cols-2 sm:grid-cols-4 gap-4">
        {STATS.map((s) => (
          <div key={s.label} className="bg-white rounded-xl p-5 shadow-sm">
            <p className={`text-2xl font-bold ${s.color}`}>{s.value}</p>
            <p className="text-sm font-medium text-slate-700 mt-0.5">{s.label}</p>
            {s.sub && <p className="text-xs text-slate-400 mt-0.5">{s.sub}</p>}
          </div>
        ))}
      </div>

      {/* Today's schedule */}
      <div>
        <h2 className="text-base font-semibold text-slate-900 mb-3">Today's Schedule</h2>
        {todayAppts.length === 0 ? (
          <div className="bg-white rounded-xl p-6 text-center shadow-sm text-slate-400 text-sm">
            No appointments scheduled for today.
          </div>
        ) : (
          <div className="bg-white rounded-xl shadow-sm overflow-hidden">
            <ul className="divide-y">
              {todayAppts.map((b) => (
                <li key={b.id} className="px-5 py-3.5 flex items-center gap-4">
                  <div className="text-right w-16 flex-shrink-0">
                    <p className="text-sm font-semibold text-slate-900">{formatTime(b.time)}</p>
                  </div>
                  <div className="flex-1 min-w-0">
                    <p className="font-medium text-slate-900 truncate">{b.patientName}</p>
                    <p className="text-sm text-slate-500">{b.serviceName} · {b.serviceDuration} min</p>
                  </div>
                  <div className="flex-shrink-0 flex gap-1.5">
                    {b.paymentType === 'insurance' ? (
                      <span className="text-xs bg-blue-100 text-blue-700 px-2 py-0.5 rounded-full">Insurance</span>
                    ) : (
                      <span className="text-xs bg-emerald-100 text-emerald-700 px-2 py-0.5 rounded-full">Cash</span>
                    )}
                    {b.packageCode && (
                      <span className="text-xs bg-teal-100 text-teal-700 px-2 py-0.5 rounded-full">Pkg</span>
                    )}
                  </div>
                </li>
              ))}
            </ul>
          </div>
        )}
      </div>

      {/* Active packages snapshot */}
      {activePkgs.length > 0 && (
        <div>
          <h2 className="text-base font-semibold text-slate-900 mb-3">Active Care Packages</h2>
          <div className="bg-white rounded-xl shadow-sm overflow-hidden">
            <ul className="divide-y">
              {activePkgs.slice(0, 8).map((pkg) => {
                const rem = pkg.totalVisits - pkg.usedVisits;
                const pct = Math.round((pkg.usedVisits / pkg.totalVisits) * 100);
                const isLow = rem <= 2;
                return (
                  <li key={pkg.id} className="px-5 py-3.5 flex items-center gap-4">
                    <div className="flex-1 min-w-0">
                      <p className="font-medium text-slate-900 truncate">{pkg.patientName}</p>
                      <p className="text-xs text-slate-500">{pkg.packageName}</p>
                      <div className="h-1 bg-slate-100 rounded-full mt-1.5 w-32 overflow-hidden">
                        <div
                          className={`h-full rounded-full ${isLow ? 'bg-amber-400' : 'bg-teal-500'}`}
                          style={{ width: `${pct}%` }}
                        />
                      </div>
                    </div>
                    <div className="text-right flex-shrink-0">
                      <p className={`text-lg font-bold ${isLow ? 'text-amber-600' : 'text-teal-700'}`}>{rem}</p>
                      <p className="text-xs text-slate-400">left</p>
                    </div>
                  </li>
                );
              })}
              {activePkgs.length > 8 && (
                <li className="px-5 py-3 text-xs text-slate-400">
                  +{activePkgs.length - 8} more active packages
                </li>
              )}
            </ul>
          </div>
        </div>
      )}

      {/* Upcoming (next 7 days, not today) */}
      {weekAppts.filter((b) => b.date !== today).length > 0 && (
        <div>
          <h2 className="text-base font-semibold text-slate-900 mb-3">Upcoming This Week</h2>
          <div className="bg-white rounded-xl shadow-sm overflow-hidden">
            <ul className="divide-y">
              {weekAppts.filter((b) => b.date !== today).slice(0, 10).map((b) => (
                <li key={b.id} className="px-5 py-3 flex items-center gap-4">
                  <div className="w-24 flex-shrink-0">
                    <p className="text-xs font-medium text-slate-500">{fmtDate(b.date)}</p>
                    <p className="text-sm font-semibold text-slate-800">{formatTime(b.time)}</p>
                  </div>
                  <div className="flex-1 min-w-0">
                    <p className="text-sm font-medium text-slate-900 truncate">{b.patientName}</p>
                    <p className="text-xs text-slate-500">{b.serviceName}</p>
                  </div>
                  {b.paymentType === 'insurance' ? (
                    <span className="text-xs bg-blue-100 text-blue-700 px-2 py-0.5 rounded-full flex-shrink-0">Ins.</span>
                  ) : (
                    <span className="text-xs bg-emerald-100 text-emerald-700 px-2 py-0.5 rounded-full flex-shrink-0">Cash</span>
                  )}
                </li>
              ))}
            </ul>
          </div>
        </div>
      )}
    </div>
  );
}
