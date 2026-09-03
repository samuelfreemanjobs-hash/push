import { useState, useEffect } from 'react';
import {
  collection, getDocs, addDoc, updateDoc, deleteDoc, doc, query, orderBy,
} from 'firebase/firestore';
import { db } from '../firebase';

const EMPTY_OFFERING = { name: '', visitCount: 12, price: '' };
const EMPTY_SOLD = { patientName: '', patientEmail: '', patientPhone: '', offeringId: '' };

export default function PackagesManager({ userId }) {
  const [offerings, setOfferings] = useState([]);
  const [soldPkgs, setSoldPkgs] = useState([]);
  const [loadingO, setLoadingO] = useState(true);
  const [loadingS, setLoadingS] = useState(true);

  const [offeringForm, setOfferingForm] = useState(EMPTY_OFFERING);
  const [editingOffering, setEditingOffering] = useState(null);
  const [savingO, setSavingO] = useState(false);

  const [soldForm, setSoldForm] = useState(EMPTY_SOLD);
  const [sellTab, setSellTab] = useState(false);
  const [savingS, setSavingS] = useState(false);

  const offeringsRef = collection(db, 'practices', userId, 'packages');
  const soldRef = collection(db, 'practices', userId, 'patientPackages');

  async function loadOfferings() {
    const snap = await getDocs(offeringsRef);
    const list = snap.docs.map((d) => ({ id: d.id, ...d.data() }));
    setOfferings(list);
    setLoadingO(false);
    if (!soldForm.offeringId && list.length > 0) {
      setSoldForm((f) => ({ ...f, offeringId: list[0].id }));
    }
  }

  async function loadSold() {
    const q = query(soldRef, orderBy('purchasedAt', 'desc'));
    const snap = await getDocs(q);
    setSoldPkgs(snap.docs.map((d) => ({ id: d.id, ...d.data() })));
    setLoadingS(false);
  }

  useEffect(() => { loadOfferings(); loadSold(); }, [userId]);

  // ── Offerings CRUD ──
  async function handleOfferingSubmit(e) {
    e.preventDefault();
    setSavingO(true);
    const data = {
      name: offeringForm.name.trim(),
      visitCount: Number(offeringForm.visitCount),
      price: offeringForm.price !== '' ? Number(offeringForm.price) : null,
    };
    if (editingOffering) {
      await updateDoc(doc(db, 'practices', userId, 'packages', editingOffering), data);
    } else {
      await addDoc(offeringsRef, data);
    }
    setOfferingForm(EMPTY_OFFERING);
    setEditingOffering(null);
    await loadOfferings();
    setSavingO(false);
  }

  async function deleteOffering(id) {
    if (!window.confirm('Delete this package offering?')) return;
    await deleteDoc(doc(db, 'practices', userId, 'packages', id));
    await loadOfferings();
  }

  function startEditOffering(o) {
    setEditingOffering(o.id);
    setOfferingForm({ name: o.name, visitCount: o.visitCount, price: o.price ?? '' });
  }

  // ── Sell a package ──
  async function handleSellSubmit(e) {
    e.preventDefault();
    setSavingS(true);
    const offering = offerings.find((o) => o.id === soldForm.offeringId);
    if (!offering) { setSavingS(false); return; }
    await addDoc(soldRef, {
      patientName: soldForm.patientName.trim(),
      patientEmail: soldForm.patientEmail.trim(),
      patientPhone: soldForm.patientPhone.trim(),
      offeringId: offering.id,
      packageName: offering.name,
      totalVisits: offering.visitCount,
      usedVisits: 0,
      price: offering.price,
      purchasedAt: new Date().toISOString(),
      status: 'active',
    });
    setSoldForm((f) => ({ ...f, patientName: '', patientEmail: '', patientPhone: '' }));
    setSellTab(false);
    await loadSold();
    setSavingS(false);
  }

  async function markExpired(id) {
    await updateDoc(doc(db, 'practices', userId, 'patientPackages', id), { status: 'expired' });
    await loadSold();
  }

  function remaining(pkg) { return pkg.totalVisits - pkg.usedVisits; }

  const activePkgs = soldPkgs.filter((p) => p.status === 'active' && remaining(p) > 0);
  const inactivePkgs = soldPkgs.filter((p) => p.status !== 'active' || remaining(p) <= 0);

  return (
    <div className="space-y-8 max-w-3xl">

      {/* Package Offerings */}
      <div className="space-y-5">
        <h2 className="text-lg font-semibold text-slate-900">Package Offerings</h2>
        <div className="bg-white rounded-xl p-6 shadow-sm">
          <h3 className="font-medium text-slate-800 mb-4">{editingOffering ? 'Edit Offering' : 'New Offering'}</h3>
          <form onSubmit={handleOfferingSubmit} className="space-y-4">
            <div>
              <label className="block text-sm font-medium text-slate-700 mb-1">Name *</label>
              <input required value={offeringForm.name}
                onChange={(e) => setOfferingForm((f) => ({ ...f, name: e.target.value }))}
                placeholder="e.g. 12-Visit Adjustment Package"
                className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500" />
            </div>
            <div className="grid grid-cols-2 gap-4">
              <div>
                <label className="block text-sm font-medium text-slate-700 mb-1">Visit Count *</label>
                <input required type="number" min="1" value={offeringForm.visitCount}
                  onChange={(e) => setOfferingForm((f) => ({ ...f, visitCount: e.target.value }))}
                  className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500" />
              </div>
              <div>
                <label className="block text-sm font-medium text-slate-700 mb-1">Price ($)</label>
                <input type="number" min="0" value={offeringForm.price}
                  onChange={(e) => setOfferingForm((f) => ({ ...f, price: e.target.value }))}
                  placeholder="Optional"
                  className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500" />
              </div>
            </div>
            <div className="flex gap-3">
              <button type="submit" disabled={savingO}
                className="bg-teal-600 text-white px-5 py-2.5 rounded-lg text-sm font-medium hover:bg-teal-700 disabled:opacity-50">
                {savingO ? 'Saving…' : editingOffering ? 'Update' : 'Add Offering'}
              </button>
              {editingOffering && (
                <button type="button"
                  onClick={() => { setEditingOffering(null); setOfferingForm(EMPTY_OFFERING); }}
                  className="px-5 py-2.5 rounded-lg text-sm font-medium border hover:bg-slate-50">
                  Cancel
                </button>
              )}
            </div>
          </form>
        </div>

        {!loadingO && offerings.length > 0 && (
          <div className="bg-white rounded-xl shadow-sm overflow-hidden">
            <ul className="divide-y">
              {offerings.map((o) => (
                <li key={o.id} className="px-6 py-4 flex items-center justify-between">
                  <div>
                    <p className="font-medium text-slate-900">{o.name}</p>
                    <p className="text-sm text-slate-500 mt-0.5">
                      {o.visitCount} visits{o.price != null ? ` · $${o.price}` : ''}
                      {o.price != null ? ` ($${(o.price / o.visitCount).toFixed(0)}/visit)` : ''}
                    </p>
                  </div>
                  <div className="flex gap-3">
                    <button onClick={() => startEditOffering(o)} className="text-sm text-teal-600 hover:underline">Edit</button>
                    <button onClick={() => deleteOffering(o.id)} className="text-sm text-red-500 hover:underline">Delete</button>
                  </div>
                </li>
              ))}
            </ul>
          </div>
        )}
      </div>

      {/* Sold Packages */}
      <div className="space-y-5">
        <div className="flex items-center justify-between">
          <div>
            <h2 className="text-lg font-semibold text-slate-900">Patient Packages</h2>
            {activePkgs.length > 0 && (
              <p className="text-sm text-slate-500 mt-0.5">{activePkgs.length} active</p>
            )}
          </div>
          {offerings.length > 0 && (
            <button
              onClick={() => setSellTab((v) => !v)}
              className="bg-teal-600 text-white px-4 py-2 rounded-lg text-sm font-medium hover:bg-teal-700"
            >
              + Sell Package
            </button>
          )}
        </div>

        {sellTab && offerings.length > 0 && (
          <div className="bg-teal-50 border border-teal-200 rounded-xl p-6">
            <h3 className="font-medium text-teal-900 mb-4">Sell a Package to a Patient</h3>
            <form onSubmit={handleSellSubmit} className="space-y-4">
              <div>
                <label className="block text-sm font-medium text-slate-700 mb-1">Package *</label>
                <select required value={soldForm.offeringId}
                  onChange={(e) => setSoldForm((f) => ({ ...f, offeringId: e.target.value }))}
                  className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500 bg-white">
                  {offerings.map((o) => (
                    <option key={o.id} value={o.id}>{o.name} ({o.visitCount} visits{o.price != null ? `, $${o.price}` : ''})</option>
                  ))}
                </select>
              </div>
              <div>
                <label className="block text-sm font-medium text-slate-700 mb-1">Patient Name *</label>
                <input required value={soldForm.patientName}
                  onChange={(e) => setSoldForm((f) => ({ ...f, patientName: e.target.value }))}
                  placeholder="Jane Smith"
                  className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500 bg-white" />
              </div>
              <div className="grid grid-cols-2 gap-4">
                <div>
                  <label className="block text-sm font-medium text-slate-700 mb-1">Email *</label>
                  <input required type="email" value={soldForm.patientEmail}
                    onChange={(e) => setSoldForm((f) => ({ ...f, patientEmail: e.target.value }))}
                    placeholder="jane@example.com"
                    className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500 bg-white" />
                </div>
                <div>
                  <label className="block text-sm font-medium text-slate-700 mb-1">Phone</label>
                  <input type="tel" value={soldForm.patientPhone}
                    onChange={(e) => setSoldForm((f) => ({ ...f, patientPhone: e.target.value }))}
                    placeholder="(555) 000-0000"
                    className="w-full border rounded-lg px-3 py-2.5 text-sm focus:outline-none focus:ring-2 focus:ring-teal-500 bg-white" />
                </div>
              </div>
              <p className="text-xs text-teal-700 bg-teal-100 px-3 py-2 rounded-lg">
                A package code will be generated. Share it with the patient so they can apply it when booking online.
              </p>
              <div className="flex gap-3">
                <button type="submit" disabled={savingS}
                  className="bg-teal-600 text-white px-5 py-2.5 rounded-lg text-sm font-medium hover:bg-teal-700 disabled:opacity-50">
                  {savingS ? 'Creating…' : 'Create Package'}
                </button>
                <button type="button" onClick={() => setSellTab(false)}
                  className="px-5 py-2.5 rounded-lg text-sm font-medium border bg-white hover:bg-slate-50">
                  Cancel
                </button>
              </div>
            </form>
          </div>
        )}

        {loadingS ? (
          <div className="animate-pulse h-32 bg-slate-100 rounded-xl" />
        ) : soldPkgs.length === 0 ? (
          <div className="bg-white rounded-xl p-8 text-center shadow-sm text-slate-400 text-sm">
            No patient packages yet. Sell a package above to get started.
          </div>
        ) : (
          <div className="space-y-3">
            {/* Active */}
            {activePkgs.length > 0 && (
              <div className="bg-white rounded-xl shadow-sm overflow-hidden">
                <div className="px-5 py-3 border-b bg-slate-50">
                  <p className="text-xs font-semibold text-slate-500 uppercase tracking-wide">Active</p>
                </div>
                <ul className="divide-y">
                  {activePkgs.map((pkg) => {
                    const rem = remaining(pkg);
                    const pct = Math.round((pkg.usedVisits / pkg.totalVisits) * 100);
                    return (
                      <li key={pkg.id} className="px-5 py-4">
                        <div className="flex items-start justify-between gap-4 mb-2">
                          <div>
                            <p className="font-medium text-slate-900">{pkg.patientName}</p>
                            <p className="text-sm text-slate-500">{pkg.packageName}</p>
                            <p className="text-xs text-slate-400 mt-0.5">{pkg.patientEmail}</p>
                          </div>
                          <div className="text-right flex-shrink-0">
                            <p className="text-lg font-bold text-teal-700">{rem}</p>
                            <p className="text-xs text-slate-500">visits left</p>
                          </div>
                        </div>
                        {/* Progress bar */}
                        <div className="h-1.5 bg-slate-100 rounded-full overflow-hidden mb-2">
                          <div
                            className="h-full bg-teal-500 rounded-full"
                            style={{ width: `${pct}%` }}
                          />
                        </div>
                        <div className="flex items-center justify-between">
                          <p className="text-xs text-slate-400">
                            {pkg.usedVisits} of {pkg.totalVisits} used · Code: <code className="bg-slate-100 px-1 rounded">{pkg.id}</code>
                          </p>
                          <button onClick={() => markExpired(pkg.id)} className="text-xs text-slate-400 hover:text-red-500">
                            Expire
                          </button>
                        </div>
                      </li>
                    );
                  })}
                </ul>
              </div>
            )}

            {/* Inactive */}
            {inactivePkgs.length > 0 && (
              <div className="bg-white rounded-xl shadow-sm overflow-hidden opacity-60">
                <div className="px-5 py-3 border-b bg-slate-50">
                  <p className="text-xs font-semibold text-slate-500 uppercase tracking-wide">Completed / Expired</p>
                </div>
                <ul className="divide-y">
                  {inactivePkgs.map((pkg) => (
                    <li key={pkg.id} className="px-5 py-3 flex items-center justify-between">
                      <div>
                        <p className="text-sm font-medium text-slate-700">{pkg.patientName}</p>
                        <p className="text-xs text-slate-400">{pkg.packageName} · {pkg.usedVisits}/{pkg.totalVisits} used</p>
                      </div>
                      <span className="text-xs text-slate-400 capitalize">{pkg.status}</span>
                    </li>
                  ))}
                </ul>
              </div>
            )}
          </div>
        )}
      </div>
    </div>
  );
}
