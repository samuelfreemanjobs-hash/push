import { useState, useEffect } from 'react';
import { collection, getDocs, doc, updateDoc, query, orderBy } from 'firebase/firestore';
import { db } from '../firebase';
import { formatTime } from '../utils/slots';

export default function BookingsCalendar({ userId }) {
  const [bookings, setBookings] = useState([]);
  const [loading, setLoading] = useState(true);
  const [filter, setFilter] = useState('upcoming');

  async function load() {
    const q = query(
      collection(db, 'businesses', userId, 'bookings'),
      orderBy('date'),
      orderBy('time')
    );
    const snap = await getDocs(q);
    setBookings(snap.docs.map((d) => ({ id: d.id, ...d.data() })));
    setLoading(false);
  }

  useEffect(() => { load(); }, [userId]);

  async function handleCancel(id) {
    if (!window.confirm('Cancel this booking? The customer will not be automatically notified.')) return;
    await updateDoc(doc(db, 'businesses', userId, 'bookings', id), { status: 'cancelled' });
    await load();
  }

  function formatDate(dateStr) {
    return new Date(dateStr + 'T00:00:00').toLocaleDateString('en-US', {
      weekday: 'short', month: 'short', day: 'numeric',
    });
  }

  const today = new Date().toISOString().slice(0, 10);

  const filtered = bookings.filter((b) => {
    if (filter === 'upcoming') return b.status !== 'cancelled' && b.date >= today;
    return true;
  });

  const upcoming = bookings.filter((b) => b.status !== 'cancelled' && b.date >= today);

  if (loading) return <div className="animate-pulse h-64 bg-gray-100 rounded-xl" />;

  return (
    <div className="space-y-5 max-w-3xl">
      <div className="flex items-center justify-between">
        <div>
          <h2 className="text-lg font-semibold text-gray-900">Bookings</h2>
          {upcoming.length > 0 && (
            <p className="text-sm text-gray-500 mt-0.5">{upcoming.length} upcoming</p>
          )}
        </div>
        <div className="flex rounded-lg border overflow-hidden text-sm">
          {['upcoming', 'all'].map((f) => (
            <button
              key={f}
              onClick={() => setFilter(f)}
              className={`px-4 py-1.5 capitalize ${
                filter === f ? 'bg-indigo-600 text-white' : 'text-gray-600 hover:bg-gray-50'
              }`}
            >
              {f}
            </button>
          ))}
        </div>
      </div>

      <div className="bg-white rounded-xl shadow-sm overflow-hidden">
        {filtered.length === 0 ? (
          <p className="p-10 text-center text-gray-400 text-sm">
            {filter === 'upcoming' ? 'No upcoming bookings.' : 'No bookings yet.'}
          </p>
        ) : (
          <ul className="divide-y">
            {filtered.map((b) => (
              <li
                key={b.id}
                className={`px-6 py-4 flex items-start justify-between gap-4 ${
                  b.status === 'cancelled' ? 'opacity-50' : ''
                }`}
              >
                <div className="space-y-1 min-w-0">
                  <div className="flex items-center gap-2 flex-wrap">
                    <p className="font-medium text-gray-900">{b.customerName}</p>
                    {b.status === 'cancelled' && (
                      <span className="text-xs bg-red-100 text-red-600 px-2 py-0.5 rounded-full">
                        Cancelled
                      </span>
                    )}
                  </div>
                  <p className="text-sm text-gray-600">
                    {b.serviceName} · {b.serviceDuration} min
                    {b.servicePrice != null ? ` · $${b.servicePrice}` : ''}
                  </p>
                  <p className="text-sm text-gray-500">
                    {formatDate(b.date)} at {formatTime(b.time)}
                  </p>
                  <p className="text-xs text-gray-400">{b.customerEmail}</p>
                </div>
                {b.status !== 'cancelled' && (
                  <button
                    onClick={() => handleCancel(b.id)}
                    className="text-sm text-red-500 hover:underline flex-shrink-0"
                  >
                    Cancel
                  </button>
                )}
              </li>
            ))}
          </ul>
        )}
      </div>
    </div>
  );
}
