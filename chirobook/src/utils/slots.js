function timeToMinutes(time) {
  const [h, m] = time.split(':').map(Number);
  return h * 60 + m;
}

function minutesToTime(minutes) {
  const h = Math.floor(minutes / 60);
  const m = minutes % 60;
  return `${String(h).padStart(2, '0')}:${String(m).padStart(2, '0')}`;
}

export function formatTime(time24) {
  const [h, m] = time24.split(':').map(Number);
  const period = h >= 12 ? 'PM' : 'AM';
  const h12 = h % 12 || 12;
  return `${h12}:${String(m).padStart(2, '0')} ${period}`;
}

export function generateTimeSlots(openTime, closeTime, durationMinutes, bookedTimes = []) {
  const slots = [];
  let current = timeToMinutes(openTime);
  const close = timeToMinutes(closeTime);
  while (current + durationMinutes <= close) {
    const timeStr = minutesToTime(current);
    if (!bookedTimes.includes(timeStr)) slots.push(timeStr);
    current += durationMinutes;
  }
  return slots;
}

const DAYS = ['sunday', 'monday', 'tuesday', 'wednesday', 'thursday', 'friday', 'saturday'];

export function getDayName(dateStr) {
  return DAYS[new Date(dateStr + 'T00:00:00').getDay()];
}
