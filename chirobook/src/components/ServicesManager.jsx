import { useState, useEffect } from 'react';
import { collection, getDocs, addDoc, updateDoc, deleteDoc, doc } from 'firebase/firestore';
import { db } from '../firebase';

const CATEGORIES = ['adjustment', 'consultation', 'therapy', 'x-ray', 'other'];

const DEFAULT_SERVICES = [
  { name: 'Initial Consultation', duration: 60, price: 150, category: 'consultation' },
  { name: 'Chiropractic Adjustment', duration: 20, price: 65, category: 'adjustment' },
  { name: 'Adjustment + Soft Tissue', duration: 30, price: 85, category: 'adjustment' },
  { name: 'Spinal Decompression', duration: 45, price: 90, category: 'therapy' },
  { name: 'X-Ray Series', duration: 15, price: 110, category: 'x-ray' },
];

const EMPTY = { name: '', duration: 20, price: '', category: 'adjustment' };

export default function ServicesManager({ userId }) {
  const [services, setServices] = useState([]);
  const [loading, setLoading] = useState(true);
  const [form, setForm] = useState(EMPTY);
  const [editing, setEditing] = useState(null);
  const [saving, setSaving] = useState(false);
  const [seeding, setSeeding] = useState(false);

  const colRef = collection(db, 'practices', userId, 'services');

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
      category: form.category,
    };
    if (editing) {
      await updateDoc(doc(db, 'practices', userId, 'services', editing), data);
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
    await deleteDoc(doc(db, 'practices', userId, 'services', id));
    await load();
  }

  async function seedDefaults() {
    setSeeding(true);
    for (const svc of DEFAULT_SERVICES) await addDoc(colRef, svc);
    await load();
    setSeeding(false);
  }

  function startEdit(svc) {
    setEditing(svc.id);
    setForm({ name: svc.name, duration: svc.duration, price: svc.price ?? '', category: svc.category || 'adjustment' });
  }

  if (loading) return <div className="animate-pulse h-64 bg-slate-100 rounded-xl" />;

  return (
    <div className="space-y-6 max-w-2xl">
      <div className="bg-white rounded-xl p-6 shadow-sm">
        <h2 className="text-lg font-semibold text-slate-900 mb-4">
          {editing ? 'Edit Service' : 'Add Service'}
        </h2>
        <form onSubmit={handleSubmit} className="space-y-4">
          <div>
            <label className="block text-sm font-medium text-slate-700 mb-1">Service Name *</label>
            <input
              required
              value={form.name}
              onChange={(e) => setForm((f) => ({ ...f, name: e.target.value }))}
              placeholder="e.g. Chiropractic Adjustment"
              className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500"
            />
          </div>
          <div className="grid grid-cols-3 gap-4">
            <div>
              <label className="block text-sm font-medium text-slate-700 mb-1">Duration (min) *</label>
              <input
                required type="number" min="5" step="5"
                value={form.duration}
                onChange={(e) => setForm((f) => ({ ...f, duration: e.target.value }))}
                className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500"
              />
            </div>
            <div>
              <label className="block text-sm font-medium text-slate-700 mb-1">Price ($)</label>
              <input
                type="number" min="0" step="0.01"
                value={form.price}
                onChange={(e) => setForm((f) => ({ ...f, price: e.target.value }))}
                placeholder="Optional"
                className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500"
              />
            </div>
            <div>
              <label className="block text-sm font-medium text-slate-700 mb-1">Category</label>
              <select
                value={form.category}
                onChange={(e) => setForm((f) => ({ ...f, category: e.target.value }))}
                className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500 capitalize"
              >
                {CATEGORIES.map((c) => <option key={c} value={c} className="capitalize">{c}</option>)}
              </select>
            </div>
          </div>
          <div className="flex gap-3">
            <button
              type="submit"
              disabled={saving}
              className="bg-teal-600 text-white px-5 py-2.5 rounded-lg text-sm font-medium hover:bg-teal-700 disabled:opacity-50"
            >
              {saving ? 'Saving…' : editing ? 'Update Service' : 'Add Service'}
            </button>
            {editing && (
              <button
                type="button"
                onClick={() => { setEditing(null); setForm(EMPTY); }}
                className="px-5 py-2.5 rounded-lg text-sm font-medium border hover:bg-slate-50"
              >
                Cancel
              </button>
            )}
          </div>
        </form>
      </div>

      {/* Services list */}
      <div className="bg-white rounded-xl shadow-sm overflow-hidden">
        <div className="px-6 py-4 border-b flex items-center justify-between">
          <h3 className="font-medium text-slate-900">Your Services ({services.length})</h3>
          {services.length === 0 && (
            <button
              onClick={seedDefaults}
              disabled={seeding}
              className="text-sm text-teal-600 hover:underline disabled:opacity-50"
            >
              {seeding ? 'Adding…' : '+ Add chiro defaults'}
            </button>
          )}
        </div>
        {services.length === 0 ? (
          <p className="px-6 py-10 text-slate-400 text-center text-sm">
            No services yet. Add your first one above or load chiro defaults.
          </p>
        ) : (
          <ul className="divide-y">
            {services.map((svc) => (
              <li key={svc.id} className="px-6 py-4 flex items-center justify-between">
                <div>
                  <div className="flex items-center gap-2">
                    <p className="font-medium text-slate-900">{svc.name}</p>
                    {svc.category && (
                      <span className="text-xs bg-slate-100 text-slate-500 px-2 py-0.5 rounded capitalize">
                        {svc.category}
                      </span>
                    )}
                  </div>
                  <p className="text-sm text-slate-500 mt-0.5">
                    {svc.duration} min{svc.price != null ? ` · $${svc.price}` : ''}
                  </p>
                </div>
                <div className="flex gap-3">
                  <button onClick={() => startEdit(svc)} className="text-sm text-teal-600 hover:underline">Edit</button>
                  <button onClick={() => handleDelete(svc.id)} className="text-sm text-red-500 hover:underline">Delete</button>
                </div>
              </li>
            ))}
          </ul>
        )}
      </div>
    </div>
  );
}
