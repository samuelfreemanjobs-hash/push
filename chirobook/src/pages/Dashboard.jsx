import { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { signOut } from 'firebase/auth';
import { auth } from '../firebase';
import { useAuth } from '../hooks/useAuth';
import Overview from '../components/Overview';
import PatientsView from '../components/PatientsView';
import AppointmentsView from '../components/AppointmentsView';
import PackagesManager from '../components/PackagesManager';
import ServicesManager from '../components/ServicesManager';
import HoursManager from '../components/HoursManager';
import BlockedDates from '../components/BlockedDates';
import PracticeSetup from '../components/PracticeSetup';

const TABS = [
  { id: 'overview',      label: 'Overview' },
  { id: 'appointments',  label: 'Appointments' },
  { id: 'patients',      label: 'Patients' },
  { id: 'packages',      label: 'Care Packages' },
  { id: 'services',      label: 'Services' },
  { id: 'hours',         label: 'Hours' },
  { id: 'blocked',       label: 'Blocked Dates' },
  { id: 'settings',      label: 'Settings' },
];

export default function Dashboard() {
  const { user } = useAuth();
  const navigate = useNavigate();
  const [activeTab, setActiveTab] = useState('overview');
  const [copied, setCopied] = useState(false);

  const bookingUrl = `${window.location.origin}/book/${user?.uid}`;

  async function handleSignOut() {
    await signOut(auth);
    navigate('/');
  }

  function copyLink() {
    navigator.clipboard.writeText(bookingUrl);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  }

  return (
    <div className="min-h-screen bg-slate-50">
      {/* Header */}
      <header className="bg-white border-b px-4 py-4">
        <div className="max-w-6xl mx-auto flex flex-col sm:flex-row sm:items-center justify-between gap-3">
          <span className="text-xl font-bold text-teal-700 tracking-tight">ChiroBook</span>
          <div className="flex flex-col sm:flex-row items-start sm:items-center gap-3">
            <div className="flex items-center gap-2 text-sm">
              <span className="text-slate-400">Patient link:</span>
              <a
                href={bookingUrl}
                target="_blank"
                rel="noopener noreferrer"
                className="text-teal-600 hover:underline max-w-xs truncate"
              >
                {bookingUrl}
              </a>
              <button
                onClick={copyLink}
                className="text-xs border rounded px-2 py-0.5 text-slate-600 hover:bg-slate-50"
              >
                {copied ? '✓ Copied' : 'Copy'}
              </button>
            </div>
            <button
              onClick={handleSignOut}
              className="text-sm text-slate-400 hover:text-slate-700"
            >
              Sign out
            </button>
          </div>
        </div>
      </header>

      {/* Tabs */}
      <div className="bg-white border-b">
        <div className="max-w-6xl mx-auto px-4">
          <div className="flex overflow-x-auto">
            {TABS.map((tab) => (
              <button
                key={tab.id}
                onClick={() => setActiveTab(tab.id)}
                className={`px-4 py-3 text-sm font-medium whitespace-nowrap border-b-2 transition-colors ${
                  activeTab === tab.id
                    ? 'border-teal-600 text-teal-700'
                    : 'border-transparent text-slate-500 hover:text-slate-900'
                }`}
              >
                {tab.label}
              </button>
            ))}
          </div>
        </div>
      </div>

      {/* Content */}
      <main className="max-w-6xl mx-auto px-4 py-8">
        {activeTab === 'overview'     && <Overview userId={user.uid} />}
        {activeTab === 'appointments' && <AppointmentsView userId={user.uid} />}
        {activeTab === 'patients'     && <PatientsView userId={user.uid} />}
        {activeTab === 'packages'     && <PackagesManager userId={user.uid} />}
        {activeTab === 'services'     && <ServicesManager userId={user.uid} />}
        {activeTab === 'hours'        && <HoursManager userId={user.uid} />}
        {activeTab === 'blocked'      && <BlockedDates userId={user.uid} />}
        {activeTab === 'settings'     && <PracticeSetup userId={user.uid} />}
      </main>
    </div>
  );
}
