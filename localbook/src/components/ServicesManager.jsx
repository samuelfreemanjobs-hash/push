import { useState, useEffect } from 'react';
import { collection, getDocs, addDoc, updateDoc, deleteDoc, doc } from 'firebase/firestore';
import { db } from '../firebase';

const EMPTY = { name: '', duration: 30, price: '' };

export default function ServicesManager({ userId }) {
  const [services, setServices] = useState([]);
  const [loading, setLoading] = useState(true);
  const [form, setForm] = useState(EMPTY);
  const [editing, setEditing] = useState(null);
  const [saving, setSaving] = useState(false);

  const colRef = collection(db, 'businesses', userId, 'services');

  async function load() {
    const snap = await getDocs(colRef);
    setServices(snap.docs.map((d) => ({ id: d.id, ...d.data() })));
    setLoading(false);
  }

  useEffect(() => { load(); }, [userId]);

  async function handleSubmit(e) {
    e.preventDefault();
    setSaving(true);
    const data = {
      name: form.name.trim(),
      duration: Number(form.duration),
      price: form.price !== '' ? Number(form.price) : null,
    };
    if (editing) {
      await updateDoc(doc(db, 'businesses', userId, 'services', editing), data);
    } else {
      await addDoc(colRef, data);
    }
    setForm(EMPTY);
    setEditing(null);
    await load();
    setSaving(false);
  }

  async function handleDelete(id) {
    if (!window.confirm('Delete this service?')) return;
    await deleteDoc(doc(db, 'businesses', userId, 'services', id));
    await load();
  }

  function startEdit(svc) {
    setEditing(svc.id);
    setForm({ name: svc.name, duration: svc.duration, price: svc.price ?? '' });
  }

  if (loading) return <div className="animate-pulse h-64 bg-gray-100 rounded-xl" />;

  return (
    <div className="space-y-6 max-w-2xl">
      <div className="bg-white rounded-xl p-6 shadow-sm">
        <h2 className="text-lg font-semibold text-gray-900 mb-4">
          {editing ? 'Edit Service' : 'Add Service'}
        </h2>
        <form onSubmit={handleSubmit} className="space-y-4">
          <div>
            <label className="block text-sm font-medium text-gray-700 mb-1">Service Name *</label>
            <input
              required
              value={form.name}
              onChange={(e) => setForm((f) => ({ ...f, name: e.target.value }))}
              placeholder="e.g. Haircut & Style"
              className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-indigo-500"
            />
          </div>
          <div className="grid grid-cols-2 gap-4">
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-1">Duration (min) *</label>
              <input
                required
                type="number"
                min="5"
                step="5"
                value={form.duration}
                onChange={(e) => setForm((f) => ({ ...f, duration: e.target.value }))}
                className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-indigo-500"
              />
            </div>
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-1">Price ($)</label>
              <input
                type="number"
                min="0"
                step="0.01"
                value={form.price}
                onChange={(e) => setForm((f) => ({ ...f, price: e.target.value }))}
                placeholder="Optional"
                className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-indigo-500"
              />
            </div>
          </div>
          <div className="flex gap-3">
            <button
              type="submit"
              disabled={saving}
              className="bg-indigo-600 text-white px-5 py-2.5 rounded-lg text-sm font-medium hover:bg-indigo-700 disabled:opacity-50"
            >
              {saving ? 'Saving...' : editing ? 'Update Service' : 'Add Service'}
            </button>
            {editing && (
              <button
                type="button"
                onClick={() => { setEditing(null); setForm(EMPTY); }}
                className="px-5 py-2.5 rounded-lg text-sm font-medium border hover:bg-gray-50"
              >
                Cancel
              </button>
            )}
          </div>
        </form>
      </div>

      <div className="bg-white rounded-xl shadow-sm overflow-hidden">
        <div className="px-6 py-4 border-b">
          <h3 className="font-medium text-gray-900">Your Services ({services.length})</h3>
        </div>
        {services.length === 0 ? (
          <p className="px-6 py-10 text-gray-400 text-center text-sm">No services yet. Add your first one above.</p>
        ) : (
          <ul className="divide-y">
            {services.map((svc) => (
              <li key={svc.id} className="px-6 py-4 flex items-center justify-between">
                <div>
                  <p className="font-medium text-gray-900">{svc.name}</p>
                  <p className="text-sm text-gray-500 mt-0.5">
                    {svc.duration} min{svc.price != null ? ` · $${svc.price}` : ''}
                  </p>
                </div>
                <div className="flex gap-3">
                  <button onClick={() => startEdit(svc)} className="text-sm text-indigo-600 hover:underline">
                    Edit
                  </button>
                  <button onClick={() => handleDelete(svc.id)} className="text-sm text-red-500 hover:underline">
                    Delete
                  </button>
                </div>
              </li>
            ))}
          </ul>
        )}
      </div>
    </div>
  );
}
