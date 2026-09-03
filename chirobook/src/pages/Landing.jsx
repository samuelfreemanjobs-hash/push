import { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { signInWithPopup } from 'firebase/auth';
import { auth, googleProvider } from '../firebase';
import { useAuth } from '../hooks/useAuth';

const FEATURES = [
  {
    icon: '📦',
    title: 'Care package tracking',
    desc: 'Sell 6-visit or 12-visit bundles. Every dashboard view shows each patient\'s remaining visits at a glance.',
  },
  {
    icon: '🏷️',
    title: 'Insurance vs. cash-pay flags',
    desc: 'Patients select their payment type when booking. Your appointment list shows the split so you\'re never surprised at checkout.',
  },
  {
    icon: '👥',
    title: 'Patient roster',
    desc: 'See all your active patients, their visit history, and package status in one place — no spreadsheet required.',
  },
  {
    icon: '🔗',
    title: 'Simple booking link',
    desc: 'Share one URL. Patients pick their service, date, and time in under two minutes from any device.',
  },
];

export default function Landing() {
  const { user } = useAuth();
  const navigate = useNavigate();
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState('');

  if (user) { navigate('/dashboard'); return null; }

  async function handleSignIn() {
    setLoading(true);
    setError('');
    try {
      await signInWithPopup(auth, googleProvider);
      navigate('/dashboard');
    } catch (e) {
      setError('Sign-in failed. Please try again.');
      setLoading(false);
    }
  }

  return (
    <div className="min-h-screen bg-white">
      {/* Nav */}
      <nav className="border-b px-6 py-4 flex items-center justify-between max-w-5xl mx-auto">
        <span className="font-bold text-xl text-teal-700 tracking-tight">ChiroBook</span>
        <button
          onClick={handleSignIn}
          disabled={loading}
          className="bg-teal-600 text-white px-5 py-2 rounded-lg text-sm font-medium hover:bg-teal-700 disabled:opacity-50 transition-colors"
        >
          {loading ? 'Signing in…' : 'Sign in with Google'}
        </button>
      </nav>

      {/* Hero */}
      <div className="max-w-4xl mx-auto px-6 pt-20 pb-16 text-center">
        <div className="inline-flex items-center gap-2 bg-teal-50 text-teal-700 text-xs font-semibold px-3 py-1.5 rounded-full mb-6 tracking-wide uppercase">
          Built for solo & duo chiropractic practices
        </div>
        <h1 className="text-4xl sm:text-5xl font-bold text-slate-900 leading-tight mb-5">
          Booking that understands<br />how chiro offices actually work
        </h1>
        <p className="text-lg text-slate-500 max-w-2xl mx-auto mb-8 leading-relaxed">
          Package tracking, insurance vs. cash-pay flags, and a dead-simple patient booking link.
          No bloat, no monthly contracts for features you'll never use.
        </p>
        <button
          onClick={handleSignIn}
          disabled={loading}
          className="bg-teal-600 text-white px-8 py-4 rounded-xl text-base font-semibold hover:bg-teal-700 disabled:opacity-50 transition-colors shadow-sm"
        >
          {loading ? 'Signing in…' : 'Get started free'}
        </button>
        {error && <p className="mt-3 text-sm text-red-500">{error}</p>}
        <p className="mt-3 text-xs text-slate-400">Sign in with Google · No credit card required</p>
      </div>

      {/* Features */}
      <div className="max-w-5xl mx-auto px-6 pb-24">
        <div className="grid sm:grid-cols-2 gap-6">
          {FEATURES.map((f) => (
            <div key={f.title} className="bg-slate-50 rounded-2xl p-6">
              <div className="text-2xl mb-3">{f.icon}</div>
              <h3 className="font-semibold text-slate-900 mb-1.5">{f.title}</h3>
              <p className="text-sm text-slate-500 leading-relaxed">{f.desc}</p>
            </div>
          ))}
        </div>
      </div>

      {/* Footer */}
      <div className="border-t py-6 text-center text-xs text-slate-400">
        ChiroBook — built for independent chiropractic practices
      </div>
    </div>
  );
}
