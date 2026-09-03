import { useState, useEffect } from 'react';
import { useParams } from 'react-router-dom';
import {
  doc, getDoc, collection, getDocs, addDoc, updateDoc, increment,
} from 'firebase/firestore';
import { db } from '../firebase';
import { generateTimeSlots, getDayName, formatTime } from '../utils/slots';

const MONTH_NAMES = [
  'January','February','March','April','May','June',
  'July','August','September','October','November','December',
];

function fmtLongDate(ds) {
  return new Date(ds + 'T00:00:00').toLocaleDateString('en-US', {
    weekday: 'long', month: 'long', day: 'numeric',
  });
}

function dateKey(y, m, d) {
  return `${y}-${String(m + 1).padStart(2, '0')}-${String(d).padStart(2, '0')}`;
}

export default function BookingPage() {
  const { practiceId } = useParams();

  const [practice, setPractice]       = useState(null);
  const [services, setServices]       = useState([]);
  const [blockedDates, setBlockedDates] = useState(new Set());
  const [loading, setLoading]         = useState(true);
  const [notFound, setNotFound]       = useState(false);

  const [step, setStep]                 = useState(1);
  const [selectedService, setSelectedService] = useState(null);
  const [selectedDate, setSelectedDate]       = useState('');
  const [selectedTime, setSelectedTime]       = useState('');

  // Patient info
  const [name, setName]           = useState('');
  const [email, setEmail]         = useState('');
  const [phone, setPhone]         = useState('');
  const [payType, setPayType]     = useState('cash'); // 'cash' | 'insurance'
  const [carrier, setCarrier]     = useState('');
  const [memberId, setMemberId]   = useState('');
  const [pkgCode, setPkgCode]     = useState('');
  const [pkgInfo, setPkgInfo]     = useState(null); // fetched package record
  const [pkgError, setPkgError]   = useState('');
  const [notes, setNotes]         = useState('');

  const [submitting, setSubmitting] = useState(false);
  const [confirmed, setConfirmed]   = useState(false);

  const [calMonth, setCalMonth] = useState(() => {
    const now = new Date();
    return { year: now.getFullYear(), month: now.getMonth() };
  });
  const [availableSlots, setAvailableSlots] = useState([]);
  const [slotsLoading, setSlotsLoading]     = useState(false);

  useEffect(() => {
    async function load() {
      const pSnap = await getDoc(doc(db, 'practices', practiceId));
      if (!pSnap.exists()) { setNotFound(true); setLoading(false); return; }
      setPractice(pSnap.data());
      const [sSnap, bSnap] = await Promise.all([
        getDocs(collection(db, 'practices', practiceId, 'services')),
        getDocs(collection(db, 'practices', practiceId, 'blockedDates')),
      ]);
      setServices(sSnap.docs.map((d) => ({ id: d.id, ...d.data() })));
      setBlockedDates(new Set(bSnap.docs.map((d) => d.id)));
      setLoading(false);
    }
    load();
  }, [practiceId]);

  useEffect(() => {
    if (!selectedDate || !selectedService || !practice?.hours) {
      setAvailableSlots([]);
      return;
    }
    async function loadSlots() {
      setSlotsLoading(true);
      const dayName = getDayName(selectedDate);
      const dayHours = practice.hours?.[dayName];
      if (!dayHours?.enabled) { setAvailableSlots([]); setSlotsLoading(false); return; }
      const snap = await getDocs(collection(db, 'practices', practiceId, 'bookings'));
      const bookedTimes = snap.docs
        .map((d) => d.data())
        .filter((b) => b.date === selectedDate && b.status !== 'cancelled')
        .map((b) => b.time);
      setAvailableSlots(
        generateTimeSlots(dayHours.open, dayHours.close, selectedService.duration, bookedTimes)
      );
      setSlotsLoading(false);
    }
    loadSlots();
  }, [selectedDate, selectedService, practice, practiceId]);

  async function lookupPackage() {
    setPkgError('');
    setPkgInfo(null);
    const code = pkgCode.trim();
    if (!code) return;
    try {
      const snap = await getDoc(doc(db, 'practices', practiceId, 'patientPackages', code));
      if (!snap.exists()) { setPkgError('Package code not found.'); return; }
      const data = snap.data();
      const remaining = data.totalVisits - data.usedVisits;
      if (remaining <= 0) { setPkgError('This package has no remaining visits.'); return; }
      if (data.status === 'expired') { setPkgError('This package has expired.'); return; }
      setPkgInfo({ id: snap.id, ...data, remaining });
    } catch {
      setPkgError('Could not look up package code.');
    }
  }

  async function handleConfirm(e) {
    e.preventDefault();
    setSubmitting(true);

    const bookingData = {
      patientName: name.trim(),
      patientEmail: email.trim(),
      patientPhone: phone.trim(),
      serviceId: selectedService.id,
      serviceName: selectedService.name,
      serviceDuration: selectedService.duration,
      servicePrice: selectedService.price ?? null,
      date: selectedDate,
      time: selectedTime,
      status: 'confirmed',
      paymentType: payType,
      insuranceCarrier: payType === 'insurance' ? carrier.trim() : '',
      memberId: payType === 'insurance' ? memberId.trim() : '',
      packageCode: pkgInfo ? pkgInfo.id : '',
      packageName: pkgInfo ? pkgInfo.packageName : '',
      intakeNotes: notes.trim(),
      createdAt: new Date().toISOString(),
    };

    await addDoc(collection(db, 'practices', practiceId, 'bookings'), bookingData);

    if (pkgInfo) {
      await updateDoc(doc(db, 'practices', practiceId, 'patientPackages', pkgInfo.id), {
        usedVisits: increment(1),
      });
    }

    setSubmitting(false);
    setConfirmed(true);
  }

  const today = new Date().toISOString().slice(0, 10);
  const daysInMonth = new Date(calMonth.year, calMonth.month + 1, 0).getDate();
  const firstDay = new Date(calMonth.year, calMonth.month, 1).getDay();

  function isAvailable(ds) {
    if (ds < today) return false;
    if (blockedDates.has(ds)) return false;
    if (!practice?.hours) return false;
    return practice.hours[getDayName(ds)]?.enabled === true;
  }

  if (loading) return (
    <div className="min-h-screen flex items-center justify-center">
      <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-teal-600" />
    </div>
  );

  if (notFound) return (
    <div className="min-h-screen flex items-center justify-center px-4">
      <div className="text-center">
        <p className="text-5xl mb-4">🦴</p>
        <h1 className="text-2xl font-bold text-slate-900">Booking page not found</h1>
        <p className="text-slate-500 mt-2">This practice hasn't set up their booking page yet.</p>
      </div>
    </div>
  );

  if (confirmed) return (
    <div className="min-h-screen bg-slate-50 flex items-center justify-center px-4">
      <div className="bg-white rounded-2xl shadow-sm p-8 max-w-md w-full text-center">
        <div className="w-16 h-16 bg-teal-100 rounded-full flex items-center justify-center mx-auto mb-4 text-3xl">
          ✓
        </div>
        <h1 className="text-2xl font-bold text-slate-900 mb-2">Appointment Confirmed!</h1>
        <p className="text-slate-500 mb-6">See you soon, {name.split(' ')[0]}.</p>
        <div className="bg-slate-50 rounded-xl p-4 text-left space-y-2.5 mb-6 text-sm">
          {[
            ['Practice', practice.name],
            ['Service', selectedService.name],
            ['Date', fmtLongDate(selectedDate)],
            ['Time', formatTime(selectedTime)],
            ['Name', name],
            ['Phone', phone],
            ['Payment', payType === 'insurance' ? `Insurance — ${carrier}` : 'Cash pay'],
          ].filter(([, v]) => v).map(([label, value]) => (
            <div key={label} className="flex justify-between gap-4">
              <span className="text-slate-500">{label}</span>
              <span className="font-medium text-right">{value}</span>
            </div>
          ))}
          {pkgInfo && (
            <div className="flex justify-between gap-4">
              <span className="text-slate-500">Package</span>
              <span className="font-medium text-teal-700">
                {pkgInfo.packageName} · {pkgInfo.remaining - 1} visits remaining
              </span>
            </div>
          )}
        </div>
        <p className="text-sm text-slate-400">
          The practice will confirm or reach out if anything changes.
        </p>
      </div>
    </div>
  );

  const STEP_LABELS = ['Service', 'Date', 'Time', 'Your Info'];

  return (
    <div className="min-h-screen bg-slate-50">
      {/* Practice header */}
      <div className="bg-white border-b px-4 py-5">
        <div className="max-w-lg mx-auto flex items-center gap-4">
          {practice.logoUrl && (
            <img
              src={practice.logoUrl}
              alt={practice.name}
              className="w-14 h-14 rounded-full object-cover flex-shrink-0 border"
              onError={(e) => { e.target.style.display = 'none'; }}
            />
          )}
          <div>
            <h1 className="text-xl font-bold text-slate-900">{practice.name}</h1>
            {practice.description && (
              <p className="text-sm text-slate-600 mt-0.5">{practice.description}</p>
            )}
            {practice.address && (
              <p className="text-xs text-slate-400 mt-0.5">📍 {practice.address}</p>
            )}
            {practice.phone && (
              <p className="text-xs text-slate-400">📞 {practice.phone}</p>
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
                  step > i + 1 ? 'bg-teal-600 text-white' :
                  step === i + 1 ? 'bg-teal-600 text-white' :
                  'bg-slate-200 text-slate-500'
                }`}>
                  {step > i + 1 ? '✓' : i + 1}
                </div>
                <span className={`text-xs hidden sm:block ${step === i + 1 ? 'font-medium text-slate-900' : 'text-slate-400'}`}>
                  {label}
                </span>
                {i < 3 && <div className="flex-1 h-px bg-slate-200 mx-1" />}
              </div>
            ))}
          </div>
        </div>
      </div>

      <div className="max-w-lg mx-auto px-4 py-8">

        {/* Step 1 — Service */}
        {step === 1 && (
          <div>
            <h2 className="text-lg font-semibold text-slate-900 mb-4">Select a service</h2>
            {services.length === 0 ? (
              <div className="bg-white rounded-xl p-8 text-center shadow-sm">
                <p className="text-slate-400">No services available yet.</p>
              </div>
            ) : (
              <div className="space-y-3">
                {services.map((svc) => (
                  <button
                    key={svc.id}
                    onClick={() => { setSelectedService(svc); setStep(2); }}
                    className="w-full bg-white rounded-xl border-2 border-transparent hover:border-teal-500 p-4 text-left transition-all shadow-sm group"
                  >
                    <div className="flex justify-between items-center">
                      <div>
                        <p className="font-medium text-slate-900 group-hover:text-teal-700">{svc.name}</p>
                        <p className="text-sm text-slate-500 mt-0.5">{svc.duration} min</p>
                        {svc.category && (
                          <span className="inline-block text-xs bg-slate-100 text-slate-500 px-2 py-0.5 rounded mt-1 capitalize">
                            {svc.category}
                          </span>
                        )}
                      </div>
                      {svc.price != null && (
                        <span className="text-teal-700 font-semibold text-lg">${svc.price}</span>
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
            <button onClick={() => setStep(1)} className="text-sm text-teal-600 hover:underline mb-4 flex items-center gap-1">← Back</button>
            <h2 className="text-lg font-semibold text-slate-900 mb-4">Select a date</h2>
            <div className="bg-white rounded-xl shadow-sm p-5">
              <div className="flex items-center justify-between mb-4">
                <button
                  onClick={() => setCalMonth((m) => { const d = new Date(m.year, m.month - 1); return { year: d.getFullYear(), month: d.getMonth() }; })}
                  className="p-1.5 hover:bg-slate-100 rounded-lg text-slate-600"
                >‹</button>
                <span className="font-semibold text-slate-900">{MONTH_NAMES[calMonth.month]} {calMonth.year}</span>
                <button
                  onClick={() => setCalMonth((m) => { const d = new Date(m.year, m.month + 1); return { year: d.getFullYear(), month: d.getMonth() }; })}
                  className="p-1.5 hover:bg-slate-100 rounded-lg text-slate-600"
                >›</button>
              </div>
              <div className="grid grid-cols-7 text-center text-xs text-slate-400 mb-2">
                {['Su', 'Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa'].map((d) => <div key={d} className="py-1">{d}</div>)}
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
                        !avail ? 'text-slate-200 cursor-default' :
                        isSel ? 'bg-teal-600 text-white' :
                        'hover:bg-teal-50 text-slate-700'
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
            <button onClick={() => setStep(2)} className="text-sm text-teal-600 hover:underline mb-4 flex items-center gap-1">← Back</button>
            <h2 className="text-lg font-semibold text-slate-900 mb-1">Select a time</h2>
            <p className="text-sm text-slate-500 mb-5">{fmtLongDate(selectedDate)}</p>
            {slotsLoading ? (
              <div className="grid grid-cols-3 gap-2">
                {Array.from({ length: 9 }).map((_, i) => (
                  <div key={i} className="h-11 bg-slate-100 rounded-xl animate-pulse" />
                ))}
              </div>
            ) : availableSlots.length === 0 ? (
              <div className="bg-white rounded-xl p-8 text-center shadow-sm">
                <p className="text-slate-400">No available times on this date.</p>
                <button onClick={() => setStep(2)} className="mt-3 text-teal-600 text-sm hover:underline">Choose a different date</button>
              </div>
            ) : (
              <div className="grid grid-cols-3 gap-2">
                {availableSlots.map((slot) => (
                  <button
                    key={slot}
                    onClick={() => { setSelectedTime(slot); setStep(4); }}
                    className="bg-white border-2 border-transparent hover:border-teal-500 rounded-xl py-3 text-sm font-medium text-slate-700 shadow-sm transition-all"
                  >
                    {formatTime(slot)}
                  </button>
                ))}
              </div>
            )}
          </div>
        )}

        {/* Step 4 — Patient info */}
        {step === 4 && (
          <div>
            <button onClick={() => setStep(3)} className="text-sm text-teal-600 hover:underline mb-4 flex items-center gap-1">← Back</button>
            <h2 className="text-lg font-semibold text-slate-900 mb-5">Your information</h2>

            {/* Booking summary */}
            <div className="bg-teal-50 rounded-xl p-4 mb-6 text-sm space-y-1">
              <p className="font-semibold text-teal-900">{selectedService.name}</p>
              <p className="text-teal-700">{fmtLongDate(selectedDate)} at {formatTime(selectedTime)}</p>
              {selectedService.price != null && (
                <p className="text-teal-600 font-medium">${selectedService.price}</p>
              )}
            </div>

            <form onSubmit={handleConfirm} className="space-y-4">
              {/* Name */}
              <div>
                <label className="block text-sm font-medium text-slate-700 mb-1">Full Name *</label>
                <input required value={name} onChange={(e) => setName(e.target.value)}
                  placeholder="Jane Smith"
                  className="w-full border rounded-xl px-3 py-3 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500" />
              </div>

              {/* Email */}
              <div>
                <label className="block text-sm font-medium text-slate-700 mb-1">Email *</label>
                <input required type="email" value={email} onChange={(e) => setEmail(e.target.value)}
                  placeholder="jane@example.com"
                  className="w-full border rounded-xl px-3 py-3 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500" />
              </div>

              {/* Phone */}
              <div>
                <label className="block text-sm font-medium text-slate-700 mb-1">Phone *</label>
                <input required type="tel" value={phone} onChange={(e) => setPhone(e.target.value)}
                  placeholder="(555) 000-0000"
                  className="w-full border rounded-xl px-3 py-3 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500" />
              </div>

              {/* Payment type */}
              <div>
                <label className="block text-sm font-medium text-slate-700 mb-2">Payment Type *</label>
                <div className="grid grid-cols-2 gap-3">
                  {[['cash', 'Cash Pay'], ['insurance', 'Insurance']].map(([val, label]) => (
                    <button
                      key={val}
                      type="button"
                      onClick={() => setPayType(val)}
                      className={`py-3 rounded-xl border-2 text-sm font-medium transition-colors ${
                        payType === val
                          ? 'border-teal-600 bg-teal-50 text-teal-800'
                          : 'border-slate-200 text-slate-600 hover:border-slate-300'
                      }`}
                    >
                      {label}
                    </button>
                  ))}
                </div>
              </div>

              {/* Insurance fields */}
              {payType === 'insurance' && (
                <div className="space-y-3 bg-slate-50 rounded-xl p-4">
                  <div>
                    <label className="block text-sm font-medium text-slate-700 mb-1">Insurance Carrier *</label>
                    <input required={payType === 'insurance'} value={carrier} onChange={(e) => setCarrier(e.target.value)}
                      placeholder="e.g. Blue Cross Blue Shield"
                      className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500 bg-white" />
                  </div>
                  <div>
                    <label className="block text-sm font-medium text-slate-700 mb-1">Member ID *</label>
                    <input required={payType === 'insurance'} value={memberId} onChange={(e) => setMemberId(e.target.value)}
                      placeholder="Your member ID"
                      className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500 bg-white" />
                  </div>
                </div>
              )}

              {/* Care package code */}
              <div>
                <label className="block text-sm font-medium text-slate-700 mb-1">
                  Care Package Code <span className="text-slate-400 font-normal">(optional)</span>
                </label>
                <div className="flex gap-2">
                  <input value={pkgCode} onChange={(e) => { setPkgCode(e.target.value); setPkgInfo(null); setPkgError(''); }}
                    placeholder="Enter your package code"
                    className="flex-1 border rounded-xl px-3 py-3 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500" />
                  <button type="button" onClick={lookupPackage}
                    className="px-4 py-3 bg-slate-100 hover:bg-slate-200 rounded-xl text-sm font-medium text-slate-700 transition-colors">
                    Apply
                  </button>
                </div>
                {pkgError && <p className="text-xs text-red-500 mt-1.5">{pkgError}</p>}
                {pkgInfo && (
                  <div className="mt-2 flex items-center gap-2 text-sm bg-teal-50 text-teal-800 px-3 py-2 rounded-lg">
                    <span>✓</span>
                    <span className="font-medium">{pkgInfo.packageName}</span>
                    <span className="text-teal-600">· {pkgInfo.remaining} visit{pkgInfo.remaining !== 1 ? 's' : ''} remaining</span>
                  </div>
                )}
              </div>

              {/* Intake notes */}
              <div>
                <label className="block text-sm font-medium text-slate-700 mb-1">
                  What brings you in? <span className="text-slate-400 font-normal">(optional)</span>
                </label>
                <textarea value={notes} onChange={(e) => setNotes(e.target.value)}
                  rows={3}
                  placeholder="Chief complaint, areas of pain, any relevant history…"
                  className="w-full border rounded-xl px-3 py-3 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500 resize-none" />
              </div>

              <button type="submit" disabled={submitting}
                className="w-full bg-teal-600 text-white py-3.5 rounded-xl font-semibold hover:bg-teal-700 disabled:opacity-50 transition-colors text-sm">
                {submitting ? 'Confirming…' : 'Confirm Appointment'}
              </button>
            </form>
          </div>
        )}

      </div>
    </div>
  );
}
