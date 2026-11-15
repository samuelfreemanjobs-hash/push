import { useOpsData } from "../context/OpsDataContext";

export function VaultPanel() {
  const { snapshot } = useOpsData();
  const vaultPrs = snapshot.vaultPrs;
  return (
    <section className="panel">
      <h2 className="panel__title">Vault PR merge plan</h2>
      <p className="panel__desc">
        DiskLordz-related open PRs on Revenue-Systems-Engineer-Master-Vault (9
        total). Merge #126 first for tripwire automation.
      </p>
      <table className="data-table">
        <thead>
          <tr>
            <th>PR</th>
            <th>Phase</th>
            <th>Merge</th>
            <th>Title</th>
          </tr>
        </thead>
        <tbody>
          {vaultPrs.map((pr) => (
            <tr key={pr.number}>
              <td>
                <a href={pr.url} target="_blank" rel="noreferrer">
                  #{pr.number}
                </a>
              </td>
              <td>{pr.phase}</td>
              <td>{pr.mergeable ?? "open"}</td>
              <td>{pr.title}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </section>
  );
}