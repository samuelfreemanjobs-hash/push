import { useState, useEffect } from 'react';
import { doc, getDoc, setDoc } from 'firebase/firestore';
import { db } from '../firebase';

export default function BusinessSetup({ userId }) {
  const [form, setForm] = useState({ name: '', description: '', address: '', logoUrl: '' });
  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [saved, setSaved] = useState(false);

  useEffect(() => {
    async function load() {
      const snap = await getDoc(doc(db, 'businesses', userId));
      if (snap.exists()) {
        const d = snap.data();
        setForm({
          name: d.name || '',
          description: d.description || '',
          address: d.address || '',
          logoUrl: d.logoUrl || '',
        });
      }
      setLoading(false);
    }
    load();
  }, [userId]);

  async function handleSave(e) {
    e.preventDefault();
    setSaving(true);
    await setDoc(
      doc(db, 'businesses', userId),
      { ...form, ownerId: userId, updatedAt: new Date().toISOString() },
      { merge: true }
    );
    setSaving(false);
    setSaved(true);
    setTimeout(() => setSaved(false), 2000);
  }

  if (loading) return <div className="animate-pulse h-64 bg-gray-100 rounded-xl" />;

  return (
    <div className="bg-white rounded-xl p-6 shadow-sm max-w-2xl">
      <h2 className="text-lg font-semibold text-gray-900 mb-6">Business Information</h2>
      <form onSubmit={handleSave} className="space-y-5">
        <div>
          <label className="block text-sm font-medium text-gray-700 mb-1">Business Name *</label>
          <input
            required
            value={form.name}
            onChange={(e) => setForm((f) => ({ ...f, name: e.target.value }))}
            placeholder="e.g. Sarah's Hair Studio"
            className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-indigo-500"
          />
        </div>
        <div>
          <label className="block text-sm font-medium text-gray-700 mb-1">Description</label>
          <textarea
            value={form.description}
            onChange={(e) => setForm((f) => ({ ...f, description: e.target.value }))}
            placeholder="Tell customers a bit about your business"
            rows={3}
            className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-indigo-500"
          />
        </div>
        <div>
          <label className="block text-sm font-medium text-gray-700 mb-1">Address</label>
          <input
            value={form.address}
            onChange={(e) => setForm((f) => ({ ...f, address: e.target.value }))}
            placeholder="123 Main St, City, State"
            className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-indigo-500"
          />
        </div>
        <div>
          <label className="block text-sm font-medium text-gray-700 mb-1">Logo URL</label>
          <input
            type="url"
            value={form.logoUrl}
            onChange={(e) => setForm((f) => ({ ...f, logoUrl: e.target.value }))}
            placeholder="https://..."
            className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-indigo-500"
          />
          {form.logoUrl && (
            <img
              src={form.logoUrl}
              alt="Logo preview"
              className="mt-2 h-16 w-16 rounded-full object-cover border"
              onError={(e) => { e.target.style.display = 'none'; }}
            />
          )}
        </div>
        <button
          type="submit"
          disabled={saving}
          className="bg-indigo-600 text-white px-6 py-2.5 rounded-lg text-sm font-medium hover:bg-indigo-700 disabled:opacity-50"
        >
          {saving ? 'Saving...' : saved ? '✓ Saved' : 'Save Changes'}
        </button>
      </form>
    </div>
  );
}
