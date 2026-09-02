# LocalBook

Appointment booking for local small businesses. Set up your booking page in minutes, share the link, and let customers book 24/7.

## What it does

- **Owner dashboard** — set your services, hours, and availability
- **Public booking page** — customers pick a service, date, and time, enter their name and email, and confirm
- **Bookings view** — see all upcoming appointments, cancel with one click
- **Shareable link** — `/book/:businessId` works on any device, no login required

## Tech Stack

- React 18 + Vite
- Tailwind CSS
- Firebase Authentication (Google Sign-In)
- Firebase Firestore
- Firebase Hosting

## Local Development

```bash
cd localbook
npm install
cp .env.example .env       # fill in your Firebase credentials
npm run dev
```

Visit `http://localhost:5173`

## Firebase Setup

1. Go to [console.firebase.google.com](https://console.firebase.google.com) → New Project
2. Enable **Authentication** → Sign-in method → Google
3. Enable **Firestore** → Create database → Start in production mode
4. Go to Project Settings → Your apps → Add web app → copy the config
5. Paste the config values into your `.env` file
6. Run the Firestore rules:
   ```bash
   firebase login
   firebase use --add   # select your project
   firebase deploy --only firestore:rules
   ```

## Deploy to Firebase Hosting

```bash
npm run deploy
# which runs: vite build && firebase deploy
```

Update `.firebaserc` with your real project ID first.

## Environment Variables

All required. Get them from Firebase Project Settings → Your apps.

| Variable | Description |
|----------|-------------|
| `VITE_FIREBASE_API_KEY` | API key |
| `VITE_FIREBASE_AUTH_DOMAIN` | Auth domain |
| `VITE_FIREBASE_PROJECT_ID` | Project ID |
| `VITE_FIREBASE_STORAGE_BUCKET` | Storage bucket |
| `VITE_FIREBASE_MESSAGING_SENDER_ID` | Messaging sender ID |
| `VITE_FIREBASE_APP_ID` | App ID |

## Firestore Data Structure

```
/businesses/{userId}
  name, description, address, logoUrl, ownerId
  hours: { monday: { enabled, open, close }, ... }

/businesses/{userId}/services/{id}
  name, duration (min), price (nullable)

/businesses/{userId}/bookings/{id}
  customerName, customerEmail, serviceId, serviceName
  serviceDuration, servicePrice, date, time, status

/businesses/{userId}/blockedDates/{YYYY-MM-DD}
  date, blockedAt
```

Note: `businessId` = the owner's Firebase Auth UID. The public booking URL is `/book/{uid}`.

## Pricing Model (when you're ready)

| Tier | Price | Limits |
|------|-------|--------|
| Free | $0 | 1 service, 20 bookings/month |
| Pro | $29/month | Unlimited services and bookings |
| Business | $59/month | Custom domain + SMS reminders |
