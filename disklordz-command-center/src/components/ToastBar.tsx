import { useCommandCenter } from "../context/CommandCenterContext";

export function ToastBar() {
  const { toast, clearToast } = useCommandCenter();
  if (!toast) return null;

  return (
    <div className="toast-bar" role="status">
      <span>{toast}</span>
      <button type="button" className="btn btn--tiny" onClick={clearToast}>
        Dismiss
      </button>
    </div>
  );
}