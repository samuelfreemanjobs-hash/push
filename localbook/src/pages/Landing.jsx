import { useEffect, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { signInWithPopup } from 'firebase/auth';
import { auth, googleProvider } from '../firebase';
import { useAuth } from '../hooks/useAuth';

export default function Landing() {
  const navigate = useNavigate();
  const { user, loading } = useAuth();
  const [signingIn, setSigningIn] = useState(false);
  const [error, setError] = useState('');

  useEffect(() => {
    if (!loading && user) navigate('/dashboard', { replace: true });
  }, [user, loading, navigate]);

  async function handleSignIn() {
    setSigningIn(true);
    setError('');
    try {
      await signInWithPopup(auth, googleProvider);
      navigate('/dashboard');
    } catch (err) {
      setError('Sign-in failed. Please try again.');
      console.error(err);
    } finally {
      setSigningIn(false);
    }
  }

  if (loading) return (
    <div className="min-h-screen flex items-center justify-center">
      <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-indigo-600" />
    </div>
  );

  return (
    <div className="min-h-screen bg-white">
      {/* Nav */}
      <nav className="border-b px-6 py-4 flex items-center justify-between">
        <span className="text-xl font-bold text-indigo-600">LocalBook</span>
        <button
          onClick={handleSignIn}
          disabled={signingIn}
          className="bg-indigo-600 text-white px-4 py-2 rounded-lg text-sm font-medium hover:bg-indigo-700 disabled:opacity-50"
        >
          {signingIn ? 'Signing in...' : 'Sign in with Google'}
        </button>
      </nav>

      {/* Hero */}
      <section className="max-w-4xl mx-auto px-6 py-24 text-center">
        <h1 className="text-5xl font-bold text-gray-900 mb-6 leading-tight">
          Appointment booking for<br />
          <span className="text-indigo-600">local businesses</span>
        </h1>
        <p className="text-xl text-gray-600 mb-10 max-w-2xl mx-auto">
          Set up your booking page in minutes. Share the link or embed it on your website.
          Your customers book, you show up.
        </p>
        {error && <p className="text-red-600 text-sm mb-4">{error}</p>}
        <button
          onClick={handleSignIn}
          disabled={signingIn}
          className="bg-indigo-600 text-white px-8 py-4 rounded-xl text-lg font-semibold hover:bg-indigo-700 disabled:opacity-50 shadow-lg"
        >
          {signingIn ? 'Signing in...' : 'Set up your booking page — it\'s free'}
        </button>
        <p className="mt-3 text-sm text-gray-500">No credit card. No setup fee. Takes 5 minutes.</p>
      </section>

      {/* Features */}
      <section className="bg-gray-50 py-20">
        <div className="max-w-5xl mx-auto px-6 grid md:grid-cols-3 gap-8">
          {[
            {
              icon: '📅',
              title: 'Easy setup',
              desc: 'Add your services, set your hours, and get a shareable booking link in under 5 minutes.',
            },
            {
              icon: '🔔',
              title: 'Bookings while you sleep',
              desc: 'Customers book 24/7. No more back-and-forth over text or phone.',
            },
            {
              icon: '📊',
              title: 'Simple dashboard',
              desc: 'See all upcoming appointments in one place. Cancel or reschedule with one click.',
            },
          ].map((f) => (
            <div key={f.title} className="bg-white rounded-xl p-6 shadow-sm">
              <div className="text-3xl mb-3">{f.icon}</div>
              <h3 className="font-semibold text-gray-900 mb-2">{f.title}</h3>
              <p className="text-gray-600 text-sm leading-relaxed">{f.desc}</p>
            </div>
          ))}
        </div>
      </section>

      {/* How it works */}
      <section className="max-w-3xl mx-auto px-6 py-20">
        <h2 className="text-3xl font-bold text-center text-gray-900 mb-12">How it works</h2>
        <div className="space-y-8">
          {[
            {
              step: '1',
              title: 'Sign in with Google',
              desc: "Create your account instantly — no forms, no credit card.",
            },
            {
              step: '2',
              title: 'Set up your services and hours',
              desc: "Add what you offer, how long each takes, and when you're available.",
            },
            {
              step: '3',
              title: 'Share your booking link',
              desc: "Send it via text, put it in your Instagram bio, or embed it on your website. Customers book directly.",
            },
          ].map((s) => (
            <div key={s.step} className="flex gap-5 items-start">
              <div className="w-10 h-10 rounded-full bg-indigo-600 text-white flex items-center justify-center font-bold text-sm flex-shrink-0">
                {s.step}
              </div>
              <div>
                <h3 className="font-semibold text-gray-900">{s.title}</h3>
                <p className="text-gray-600 mt-1 text-sm leading-relaxed">{s.desc}</p>
              </div>
            </div>
          ))}
        </div>
      </section>

      {/* CTA */}
      <section className="bg-indigo-600 py-16 text-center">
        <h2 className="text-3xl font-bold text-white mb-4">Ready to stop managing bookings over text?</h2>
        <p className="text-indigo-200 mb-8">Join thousands of local businesses saving hours every week.</p>
        <button
          onClick={handleSignIn}
          disabled={signingIn}
          className="bg-white text-indigo-600 px-8 py-4 rounded-xl font-semibold hover:bg-indigo-50 disabled:opacity-50 shadow"
        >
          {signingIn ? 'Signing in...' : 'Get started free'}
        </button>
      </section>

      <footer className="border-t px-6 py-8 text-center text-sm text-gray-500">
        © {new Date().getFullYear()} LocalBook
      </footer>
    </div>
  );
}
