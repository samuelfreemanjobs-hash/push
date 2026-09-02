import { useState, useEffect } from 'react';
import { useParams } from 'react-router-dom';
import { doc, getDoc, collection, getDocs, addDoc } from 'firebase/firestore';
import { db } from '../firebase';
import { generateTimeSlots, getDayName, formatTime } from '../utils/slots';

const MONTH_NAMES = [
  'January','February','March','April','May','June',
  'July','August','September','October','November','December',
];

function formatLongDate(dateStr) {
  return new Date(dateStr + 'T00:00:00').toLocaleDateString('en-US', {
    weekday: 'long', month: 'long', day: 'numeric',
  });
}

export default function BookingPage() {
  const { businessId } = useParams();

  const [business, setBusiness] = useState(null);
  const [services, setServices] = useState([]);
  const [blockedDates, setBlockedDates] = useState(new Set());
  const [loading, setLoading] = useState(true);
  const [notFound, setNotFound] = useState(false);

  // Booking flow state
  const [step, setStep] = useState(1);
  const [selectedService, setSelectedService] = useState(null);
  const [selectedDate, setSelectedDate] = useState('');
  const [selectedTime, setSelectedTime] = useState('');
  const [customerName, setCustomerName] = useState('');
  const [customerEmail, setCustomerEmail] = useState('');
  const [submitting, setSubmitting] = useState(false);
  const [confirmed, setConfirmed] = useState(false);

  // Calendar state
  const [calMonth, setCalMonth] = useState(() => {
    const now = new Date();
    return { year: now.getFullYear(), month: now.getMonth() };
  });

  // Slots
  const [availableSlots, setAvailableSlots] = useState([]);
  const [slotsLoading, setSlotsLoading] = useState(false);

  useEffect(() => {
    async function load() {
      const bizSnap = await getDoc(doc(db, 'businesses', businessId));
      if (!bizSnap.exists()) { setNotFound(true); setLoading(false); return; }
      setBusiness(bizSnap.data());

      const [svcSnap, blockedSnap] = await Promise.all([
        getDocs(collection(db, 'businesses', businessId, 'services')),
        getDocs(collection(db, 'businesses', businessId, 'blockedDates')),
      ]);
      setServices(svcSnap.docs.map((d) => ({ id: d.id, ...d.data() })));
      setBlockedDates(new Set(blockedSnap.docs.map((d) => d.id)));
      setLoading(false);
    }
    load();
  }, [businessId]);

  useEffect(() => {
    if (!selectedDate || !selectedService || !business?.hours) {
      setAvailableSlots([]);
      return;
    }
    async function loadSlots() {
      setSlotsLoading(true);
      const dayName = getDayName(selectedDate);
      const dayHours = business.hours?.[dayName];
      if (!dayHours?.enabled) { setAvailableSlots([]); setSlotsLoading(false); return; }

      const snap = await getDocs(collection(db, 'businesses', businessId, 'bookings'));
      const bookedTimes = snap.docs
        .map((d) => d.data())
        .filter((b) => b.date === selectedDate && b.status !== 'cancelled')
        .map((b) => b.time);

      setAvailableSlots(generateTimeSlots(dayHours.open, dayHours.close, selectedService.duration, bookedTimes));
      setSlotsLoading(false);
    }
    loadSlots();
  }, [selectedDate, selectedService, business, businessId]);

  async function handleConfirm(e) {
    e.preventDefault();
    setSubmitting(true);
    await addDoc(collection(db, 'businesses', businessId, 'bookings'), {
      customerName: customerName.trim(),
      customerEmail: customerEmail.trim(),
      serviceId: selectedService.id,
      serviceName: selectedService.name,
      serviceDuration: selectedService.duration,
      servicePrice: selectedService.price ?? null,
      date: selectedDate,
      time: selectedTime,
      status: 'confirmed',
      createdAt: new Date().toISOString(),
    });
    setSubmitting(false);
    setConfirmed(true);
  }

  const today = new Date().toISOString().slice(0, 10);
  const daysInMonth = new Date(calMonth.year, calMonth.month + 1, 0).getDate();
  const firstDay = new Date(calMonth.year, calMonth.month, 1).getDay();

  function dateKey(year, month, day) {
    return `${year}-${String(month + 1).padStart(2, '0')}-${String(day).padStart(2, '0')}`;
  }

  function isAvailable(dateStr) {
    if (dateStr < today) return false;
    if (blockedDates.has(dateStr)) return false;
    if (!business?.hours) return false;
    return business.hours[getDayName(dateStr)]?.enabled === true;
  }

  // ── Loading ──
  if (loading) return (
    <div className="min-h-screen flex items-center justify-center">
      <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-indigo-600" />
    </div>
  );

  // ── Not found ──
  if (notFound) return (
    <div className="min-h-screen flex items-center justify-center px-4">
      <div className="text-center">
        <p className="text-5xl mb-4">📅</p>
        <h1 className="text-2xl font-bold text-gray-900">Booking page not found</h1>
        <p className="text-gray-500 mt-2">This business hasn't set up their booking page yet.</p>
      </div>
    </div>
  );

  // ── Confirmed ──
  if (confirmed) return (
    <div className="min-h-screen bg-gray-50 flex items-center justify-center px-4">
      <div className="bg-white rounded-2xl shadow-sm p-8 max-w-md w-full text-center">
        <div className="w-16 h-16 bg-green-100 rounded-full flex items-center justify-center mx-auto mb-4 text-3xl">
          ✓
        </div>
        <h1 className="text-2xl font-bold text-gray-900 mb-2">Booking Confirmed!</h1>
        <p className="text-gray-500 mb-6">Here's your booking summary:</p>
        <div className="bg-gray-50 rounded-xl p-4 text-left space-y-2.5 mb-6 text-sm">
          {[
            ['Business', business.name],
            ['Service', selectedService.name],
            ['Date', formatLongDate(selectedDate)],
            ['Time', formatTime(selectedTime)],
            ['Name', customerName],
            ['Email', customerEmail],
          ].map(([label, value]) => (
            <div key={label} className="flex justify-between gap-4">
              <span className="text-gray-500">{label}</span>
              <span className="font-medium text-right">{value}</span>
            </div>
          ))}
        </div>
        <p className="text-sm text-gray-400">
          We've recorded your appointment. The business will be in touch if anything changes.
        </p>
      </div>
    </div>
  );

  // ── Booking flow ──
  const STEP_LABELS = ['Service', 'Date', 'Time', 'Your Info'];

  return (
    <div className="min-h-screen bg-gray-50">
      {/* Business header */}
      <div className="bg-white border-b px-4 py-5">
        <div className="max-w-lg mx-auto flex items-center gap-4">
          {business.logoUrl && (
            <img
              src={business.logoUrl}
              alt={business.name}
              className="w-14 h-14 rounded-full object-cover flex-shrink-0 border"
              onError={(e) => { e.target.style.display = 'none'; }}
            />
          )}
          <div>
            <h1 className="text-xl font-bold text-gray-900">{business.name}</h1>
            {business.description && (
              <p className="text-sm text-gray-600 mt-0.5">{business.description}</p>
            )}
            {business.address && (
              <p className="text-xs text-gray-400 mt-0.5">📍 {business.address}</p>
            )}
          </div>
        </div>
      </div>

      {/* Step progress */}
      <div className="bg-white border-b">
        <div className="max-w-lg mx-auto px-4 py-3">
          <div className="flex items-center gap-1">
            {STEP_LABELS.map((label, i) => (
              <div key={label} className="flex items-center gap-1 flex-1">
                <div className={`w-6 h-6 rounded-full text-xs font-bold flex items-center justify-center flex-shrink-0 ${
                  step > i + 1 ? 'bg-indigo-600 text-white' :
                  step === i + 1 ? 'bg-indigo-600 text-white' :
                  'bg-gray-200 text-gray-500'
                }`}>
                  {step > i + 1 ? '✓' : i + 1}
                </div>
                <span className={`text-xs hidden sm:block ${step === i + 1 ? 'font-medium text-gray-900' : 'text-gray-400'}`}>
                  {label}
                </span>
                {i < 3 && <div className="flex-1 h-px bg-gray-200 mx-1" />}
              </div>
            ))}
          </div>
        </div>
      </div>

      <div className="max-w-lg mx-auto px-4 py-8">

        {/* Step 1 — Service */}
        {step === 1 && (
          <div>
            <h2 className="text-lg font-semibold text-gray-900 mb-4">Select a service</h2>
            {services.length === 0 ? (
              <div className="bg-white rounded-xl p-8 text-center shadow-sm">
                <p className="text-gray-400">No services available yet.</p>
              </div>
            ) : (
              <div className="space-y-3">
                {services.map((svc) => (
                  <button
                    key={svc.id}
                    onClick={() => { setSelectedService(svc); setStep(2); }}
                    className="w-full bg-white rounded-xl border-2 border-transparent hover:border-indigo-500 p-4 text-left transition-all shadow-sm group"
                  >
                    <div className="flex justify-between items-center">
                      <div>
                        <p className="font-medium text-gray-900 group-hover:text-indigo-700">{svc.name}</p>
                        <p className="text-sm text-gray-500 mt-0.5">{svc.duration} minutes</p>
                      </div>
                      {svc.price != null && (
                        <span className="text-indigo-600 font-semibold text-lg">${svc.price}</span>
                      )}
                    </div>
                  </button>
                ))}
              </div>
            )}
          </div>
        )}

        {/* Step 2 — Date */}
        {step === 2 && (
          <div>
            <button onClick={() => setStep(1)} className="text-sm text-indigo-600 hover:underline mb-4 flex items-center gap-1">
              ← Back
            </button>
            <h2 className="text-lg font-semibold text-gray-900 mb-4">Select a date</h2>
            <div className="bg-white rounded-xl shadow-sm p-5">
              <div className="flex items-center justify-between mb-4">
                <button
                  onClick={() => setCalMonth((m) => { const d = new Date(m.year, m.month - 1); return { year: d.getFullYear(), month: d.getMonth() }; })}
                  className="p-1.5 hover:bg-gray-100 rounded-lg text-gray-600"
                >‹</button>
                <span className="font-semibold text-gray-900">{MONTH_NAMES[calMonth.month]} {calMonth.year}</span>
                <button
                  onClick={() => setCalMonth((m) => { const d = new Date(m.year, m.month + 1); return { year: d.getFullYear(), month: d.getMonth() }; })}
                  className="p-1.5 hover:bg-gray-100 rounded-lg text-gray-600"
                >›</button>
              </div>
              <div className="grid grid-cols-7 text-center text-xs text-gray-400 mb-2">
                {['Su', 'Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa'].map((d) => (
                  <div key={d} className="py-1">{d}</div>
                ))}
              </div>
              <div className="grid grid-cols-7 gap-1">
                {Array.from({ length: firstDay }).map((_, i) => <div key={`e-${i}`} />)}
                {Array.from({ length: daysInMonth }, (_, i) => i + 1).map((day) => {
                  const ds = dateKey(calMonth.year, calMonth.month, day);
                  const avail = isAvailable(ds);
                  const isSel = ds === selectedDate;
                  return (
                    <button
                      key={day}
                      disabled={!avail}
                      onClick={() => { setSelectedDate(ds); setSelectedTime(''); setStep(3); }}
                      className={`aspect-square rounded-lg text-sm font-medium transition-colors ${
                        !avail ? 'text-gray-200 cursor-default' :
                        isSel ? 'bg-indigo-600 text-white' :
                        'hover:bg-indigo-50 text-gray-700'
                      }`}
                    >
                      {day}
                    </button>
                  );
                })}
              </div>
            </div>
          </div>
        )}

        {/* Step 3 — Time */}
        {step === 3 && (
          <div>
            <button onClick={() => setStep(2)} className="text-sm text-indigo-600 hover:underline mb-4 flex items-center gap-1">
              ← Back
            </button>
            <h2 className="text-lg font-semibold text-gray-900 mb-1">Select a time</h2>
            <p className="text-sm text-gray-500 mb-5">{formatLongDate(selectedDate)}</p>
            {slotsLoading ? (
              <div className="grid grid-cols-3 gap-2">
                {Array.from({ length: 9 }).map((_, i) => (
                  <div key={i} className="h-11 bg-gray-100 rounded-xl animate-pulse" />
                ))}
              </div>
            ) : availableSlots.length === 0 ? (
              <div className="bg-white rounded-xl p-8 text-center shadow-sm">
                <p className="text-gray-400">No available times on this date.</p>
                <button onClick={() => setStep(2)} className="mt-3 text-indigo-600 text-sm hover:underline">
                  Choose a different date
                </button>
              </div>
            ) : (
              <div className="grid grid-cols-3 gap-2">
                {availableSlots.map((slot) => (
                  <button
                    key={slot}
                    onClick={() => { setSelectedTime(slot); setStep(4); }}
                    className="bg-white border-2 border-transparent hover:border-indigo-500 rounded-xl py-3 text-sm font-medium text-gray-700 shadow-sm transition-all"
                  >
                    {formatTime(slot)}
                  </button>
                ))}
              </div>
            )}
          </div>
        )}

        {/* Step 4 — Customer info */}
        {step === 4 && (
          <div>
            <button onClick={() => setStep(3)} className="text-sm text-indigo-600 hover:underline mb-4 flex items-center gap-1">
              ← Back
            </button>
            <h2 className="text-lg font-semibold text-gray-900 mb-5">Your information</h2>

            {/* Booking summary */}
            <div className="bg-indigo-50 rounded-xl p-4 mb-6 text-sm space-y-1">
              <p className="font-semibold text-indigo-900">{selectedService.name}</p>
              <p className="text-indigo-700">{formatLongDate(selectedDate)} at {formatTime(selectedTime)}</p>
              {selectedService.price != null && (
                <p className="text-indigo-600 font-medium">${selectedService.price}</p>
              )}
            </div>

            <form onSubmit={handleConfirm} className="space-y-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-1">Your Name *</label>
                <input
                  required
                  value={customerName}
                  onChange={(e) => setCustomerName(e.target.value)}
                  placeholder="Jane Smith"
                  className="w-full border rounded-xl px-3 py-3 text-sm focus:outline-none focus:ring-2 focus:ring-indigo-500"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-1">Email Address *</label>
                <input
                  required
                  type="email"
                  value={customerEmail}
                  onChange={(e) => setCustomerEmail(e.target.value)}
                  placeholder="jane@example.com"
                  className="w-full border rounded-xl px-3 py-3 text-sm focus:outline-none focus:ring-2 focus:ring-indigo-500"
                />
              </div>
              <button
                type="submit"
                disabled={submitting}
                className="w-full bg-indigo-600 text-white py-3.5 rounded-xl font-semibold hover:bg-indigo-700 disabled:opacity-50 transition-colors text-sm"
              >
                {submitting ? 'Confirming...' : 'Confirm Booking'}
              </button>
            </form>
          </div>
        )}

      </div>
    </div>
  );
}
