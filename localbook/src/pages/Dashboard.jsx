import { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { signOut } from 'firebase/auth';
import { auth } from '../firebase';
import { useAuth } from '../hooks/useAuth';
import BusinessSetup from '../components/BusinessSetup';
import ServicesManager from '../components/ServicesManager';
import HoursManager from '../components/HoursManager';
import BlockedDates from '../components/BlockedDates';
import BookingsCalendar from '../components/BookingsCalendar';

const TABS = [
  { id: 'setup', label: 'Business Setup' },
  { id: 'services', label: 'Services' },
  { id: 'hours', label: 'Hours' },
  { id: 'blocked', label: 'Blocked Dates' },
  { id: 'bookings', label: 'Bookings' },
];

export default function Dashboard() {
  const { user } = useAuth();
  const navigate = useNavigate();
  const [activeTab, setActiveTab] = useState('setup');
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
    <div className="min-h-screen bg-gray-50">
      {/* Header */}
      <header className="bg-white border-b px-4 py-4">
        <div className="max-w-5xl mx-auto flex flex-col sm:flex-row sm:items-center justify-between gap-3">
          <span className="text-xl font-bold text-indigo-600">LocalBook</span>
          <div className="flex flex-col sm:flex-row items-start sm:items-center gap-3">
            <div className="flex items-center gap-2 text-sm">
              <span className="text-gray-500">Your link:</span>
              <a
                href={bookingUrl}
                target="_blank"
                rel="noopener noreferrer"
                className="text-indigo-600 hover:underline max-w-xs truncate"
              >
                {bookingUrl}
              </a>
              <button
                onClick={copyLink}
                className="text-xs border rounded px-2 py-0.5 text-gray-600 hover:bg-gray-50"
              >
                {copied ? '✓ Copied' : 'Copy'}
              </button>
            </div>
            <button
              onClick={handleSignOut}
              className="text-sm text-gray-500 hover:text-gray-800"
            >
              Sign out
            </button>
          </div>
        </div>
      </header>

      {/* Tabs */}
      <div className="bg-white border-b">
        <div className="max-w-5xl mx-auto px-4">
          <div className="flex overflow-x-auto">
            {TABS.map((tab) => (
              <button
                key={tab.id}
                onClick={() => setActiveTab(tab.id)}
                className={`px-4 py-3 text-sm font-medium whitespace-nowrap border-b-2 transition-colors ${
                  activeTab === tab.id
                    ? 'border-indigo-600 text-indigo-600'
                    : 'border-transparent text-gray-500 hover:text-gray-900'
                }`}
              >
                {tab.label}
              </button>
            ))}
          </div>
        </div>
      </div>

      {/* Content */}
      <main className="max-w-5xl mx-auto px-4 py-8">
        {activeTab === 'setup' && <BusinessSetup userId={user.uid} />}
        {activeTab === 'services' && <ServicesManager userId={user.uid} />}
        {activeTab === 'hours' && <HoursManager userId={user.uid} />}
        {activeTab === 'blocked' && <BlockedDates userId={user.uid} />}
        {activeTab === 'bookings' && <BookingsCalendar userId={user.uid} />}
      </main>
    </div>
  );
}
