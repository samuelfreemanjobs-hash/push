import { useState, useEffect } from 'react';
import { collection, getDocs, query, orderBy } from 'firebase/firestore';
import { db } from '../firebase';
import { formatTime } from '../utils/slots';

function fmtDate(ds) {
  return new Date(ds + 'T00:00:00').toLocaleDateString('en-US', {
    month: 'short', day: 'numeric', year: 'numeric',
  });
}

export default function PatientsView({ userId }) {
  const [patients, setPatients] = useState([]);
  const [loading, setLoading] = useState(true);
  const [search, setSearch] = useState('');
  const [selected, setSelected] = useState(null);

  useEffect(() => {
    async function load() {
      const [bookSnap, pkgSnap] = await Promise.all([
        getDocs(query(collection(db, 'practices', userId, 'bookings'), orderBy('date'), orderBy('time'))),
        getDocs(collection(db, 'practices', userId, 'patientPackages')),
      ]);

      const bookings = bookSnap.docs.map((d) => ({ id: d.id, ...d.data() }));
      const pkgMap = {};
      pkgSnap.docs.forEach((d) => {
        const data = d.data();
        const key = data.patientEmail?.toLowerCase();
        if (key) pkgMap[key] = { id: d.id, ...data };
      });

      // Group bookings by email
      const map = {};
      for (const b of bookings) {
        const key = b.patientEmail?.toLowerCase() || b.patientName;
        if (!map[key]) {
          map[key] = {
            key,
            name: b.patientName,
            email: b.patientEmail,
            phone: b.patientPhone || '',
            paymentType: b.paymentType || 'cash',
            bookings: [],
          };
        }
        map[key].bookings.push(b);
      }

      const today = new Date().toISOString().slice(0, 10);

      const list = Object.values(map).map((p) => {
        const activeBookings = p.bookings.filter((b) => b.status !== 'cancelled');
        const upcoming = activeBookings.filter((b) => b.date >= today).sort((a, b) => a.date.localeCompare(b.date));
        const past = activeBookings.filter((b) => b.date < today).sort((a, b) => b.date.localeCompare(a.date));
        const pkg = pkgMap[p.email?.toLowerCase()];
        return {
          ...p,
          totalVisits: activeBookings.length,
          upcoming,
          past,
          lastVisit: past[0] || null,
          nextVisit: upcoming[0] || null,
          package: pkg || null,
        };
      });

      list.sort((a, b) => {
        if (a.nextVisit && !b.nextVisit) return -1;
        if (!a.nextVisit && b.nextVisit) return 1;
        if (a.nextVisit && b.nextVisit) return a.nextVisit.date.localeCompare(b.nextVisit.date);
        return a.name.localeCompare(b.name);
      });

      setPatients(list);
      setLoading(false);
    }
    load();
  }, [userId]);

  const filtered = patients.filter((p) => {
    const q = search.toLowerCase();
    return !q || p.name.toLowerCase().includes(q) || p.email?.toLowerCase().includes(q) || p.phone?.includes(q);
  });

  const sel = selected ? patients.find((p) => p.key === selected) : null;

  if (loading) return <div className="animate-pulse h-64 bg-slate-100 rounded-xl" />;

  return (
    <div className="max-w-4xl">
      <div className="flex items-center justify-between mb-5">
        <div>
          <h2 className="text-lg font-semibold text-slate-900">Patients</h2>
          <p className="text-sm text-slate-500 mt-0.5">{patients.length} total patients</p>
        </div>
        <input
          value={search}
          onChange={(e) => setSearch(e.target.value)}
          placeholder="Search by name, email, or phone…"
          className="border rounded-lg px-3 py-2 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500 w-64"
        />
      </div>

      <div className="grid sm:grid-cols-2 gap-6">
        {/* Patient list */}
        <div className="space-y-2">
          {filtered.length === 0 ? (
            <div className="bg-white rounded-xl p-8 text-center shadow-sm text-slate-400 text-sm">
              {search ? 'No patients match that search.' : 'No patients yet. Patients appear here after their first booking.'}
            </div>
          ) : filtered.map((p) => {
            const rem = p.package ? p.package.totalVisits - p.package.usedVisits : null;
            const isSelected = selected === p.key;
            return (
              <button
                key={p.key}
                onClick={() => setSelected(isSelected ? null : p.key)}
                className={`w-full text-left bg-white rounded-xl p-4 shadow-sm border-2 transition-colors ${
                  isSelected ? 'border-teal-500' : 'border-transparent hover:border-slate-200'
                }`}
              >
                <div className="flex items-start justify-between gap-3">
                  <div className="min-w-0">
                    <p className="font-medium text-slate-900 truncate">{p.name}</p>
                    <p className="text-xs text-slate-500 truncate">{p.email}</p>
                    {p.phone && <p className="text-xs text-slate-400">{p.phone}</p>}
                  </div>
                  <div className="text-right flex-shrink-0">
                    <p className="text-lg font-bold text-slate-800">{p.totalVisits}</p>
                    <p className="text-xs text-slate-400">visit{p.totalVisits !== 1 ? 's' : ''}</p>
                  </div>
                </div>
                <div className="flex items-center gap-2 mt-2 flex-wrap">
                  {p.paymentType === 'insurance' ? (
                    <span className="text-xs bg-blue-100 text-blue-700 px-2 py-0.5 rounded-full">Insurance</span>
                  ) : (
                    <span className="text-xs bg-emerald-100 text-emerald-700 px-2 py-0.5 rounded-full">Cash</span>
                  )}
                  {p.package && rem !== null && rem > 0 && (
                    <span className="text-xs bg-teal-100 text-teal-700 px-2 py-0.5 rounded-full">
                      {rem} pkg visit{rem !== 1 ? 's' : ''} left
                    </span>
                  )}
                  {p.nextVisit && (
                    <span className="text-xs text-slate-500">Next: {fmtDate(p.nextVisit.date)}</span>
                  )}
                </div>
              </button>
            );
          })}
        </div>

        {/* Patient detail */}
        <div>
          {sel ? (
            <div className="bg-white rounded-xl shadow-sm overflow-hidden sticky top-4">
              <div className="px-5 py-4 border-b bg-slate-50">
                <p className="font-semibold text-slate-900">{sel.name}</p>
                <p className="text-sm text-slate-500">{sel.email}</p>
                {sel.phone && <p className="text-sm text-slate-500">{sel.phone}</p>}
              </div>

              {/* Package info */}
              {sel.package && (
                <div className="px-5 py-4 border-b bg-teal-50">
                  <p className="text-xs font-semibold text-teal-700 uppercase tracking-wide mb-1">Care Package</p>
                  <p className="font-medium text-teal-900">{sel.package.packageName}</p>
                  <div className="flex items-center justify-between mt-1">
                    <p className="text-sm text-teal-700">
                      {sel.package.usedVisits} of {sel.package.totalVisits} used
                    </p>
                    <p className="text-sm font-bold text-teal-800">
                      {sel.package.totalVisits - sel.package.usedVisits} remaining
                    </p>
                  </div>
                  <div className="h-1.5 bg-teal-200 rounded-full mt-2 overflow-hidden">
                    <div
                      className="h-full bg-teal-500 rounded-full"
                      style={{ width: `${Math.round((sel.package.usedVisits / sel.package.totalVisits) * 100)}%` }}
                    />
                  </div>
                </div>
              )}

              {/* Upcoming */}
              {sel.upcoming.length > 0 && (
                <div className="px-5 py-4 border-b">
                  <p className="text-xs font-semibold text-slate-500 uppercase tracking-wide mb-2">Upcoming</p>
                  <ul className="space-y-2">
                    {sel.upcoming.slice(0, 3).map((b) => (
                      <li key={b.id} className="text-sm text-slate-700">
                        {fmtDate(b.date)} at {formatTime(b.time)} — {b.serviceName}
                      </li>
                    ))}
                  </ul>
                </div>
              )}

              {/* Visit history */}
              {sel.past.length > 0 && (
                <div className="px-5 py-4">
                  <p className="text-xs font-semibold text-slate-500 uppercase tracking-wide mb-2">Recent Visits</p>
                  <ul className="space-y-2">
                    {sel.past.slice(0, 5).map((b) => (
                      <li key={b.id} className="text-sm">
                        <span className="text-slate-700">{fmtDate(b.date)}</span>
                        <span className="text-slate-400"> — {b.serviceName}</span>
                      </li>
                    ))}
                    {sel.past.length > 5 && (
                      <li className="text-xs text-slate-400">{sel.past.length - 5} more visits</li>
                    )}
                  </ul>
                </div>
              )}
            </div>
          ) : (
            <div className="bg-slate-100 rounded-xl h-48 flex items-center justify-center text-slate-400 text-sm">
              Select a patient to view details
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
