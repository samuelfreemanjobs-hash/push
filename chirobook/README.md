# ChiroBook

A booking and patient management tool built specifically for solo and duo chiropractic practices.

## What's different from a generic booking tool

- **Care package tracking** — define 6-visit / 12-visit bundles, sell them to patients, and track remaining visits with a progress bar on every active package
- **Insurance vs. cash-pay flags** — patients select their payment type at booking; the dashboard shows the split at a glance
- **Patient roster** — auto-built from booking history; click any patient to see visit count, package status, and upcoming appointments
- **Intake notes** — patients describe their chief complaint at booking; notes appear in the expanded appointment view
- **Package code system** — sell a package, get a code, share it with the patient; they enter it when booking online

## Tech stack

React + Firebase (Auth, Firestore) + Vite + Tailwind CSS

## Firestore schema

```
practices/{uid}
  name, description, address, phone, logoUrl, hours, ownerId

practices/{uid}/services/{id}
  name, duration, price, category

practices/{uid}/packages/{id}         ← package offerings (what you sell)
  name, visitCount, price

practices/{uid}/patientPackages/{id}  ← packages sold to patients
  patientName, patientEmail, patientPhone
  offeringId, packageName
  totalVisits, usedVisits, price
  purchasedAt, status

practices/{uid}/blockedDates/{dateStr}

practices/{uid}/bookings/{id}
  patientName, patientEmail, patientPhone
  serviceId, serviceName, serviceDuration, servicePrice
  date, time, status
  paymentType, insuranceCarrier, memberId
  packageCode, packageName
  intakeNotes, createdAt
```

## Setup

1. Create a Firebase project (Auth + Firestore)
2. Copy `.env.example` → `.env` and fill in your Firebase config
3. Deploy Firestore rules: `firebase deploy --only firestore:rules`
4. `npm install && npm run dev`

## Deploy

`npm run deploy` — builds and deploys to Firebase Hosting
