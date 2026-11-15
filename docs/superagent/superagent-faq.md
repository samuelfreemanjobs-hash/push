# DiskLordz – Official Customer Support & Licensing FAQ

> Upload to Base44 Knowledge Base. Update bracketed policy values when checkout goes live.

## 1. Downloads & File Extraction

- **Q: Where do I find my download links after purchasing?**
  - **A:** Download links appear on the order confirmation screen and are emailed to the address used at checkout. If you do not see the email, check Spam or Promotions for a message from **support@disklordz.com** (or your live support address).
- **Q: My download link expired or reached its download limit. How do I get a new one?**
  - **A:** Reply with your checkout **email** and **Order Number**; we will reset your link.
- **Q: My drum kit `.zip` says "corrupted" or "invalid" when I try to open it.**
  - **A:** Large packs can fail if the connection drops or the default unzipper struggles. Re-download on a stable connection and extract with **The Unarchiver** (macOS) or **7-Zip** (Windows).

## 2. VST Plugin Installation & DAW Troubleshooting

- **Q: What formats and operating systems do your plugins support?**
  - **A:** **macOS 11 Big Sur or newer (Intel & Apple Silicon native)** and **Windows 10 & 11 (64-bit)** in **VST3, AU, and AAX** where listed per product. We do not support 32-bit systems.
- **Q: I installed the plugin, but it is not showing up in my DAW (FL Studio, Ableton, Logic Pro, etc.).**
  - **A:**
    1. Confirm the installer placed files in the default folder:
       - **macOS VST3:** `/Library/Audio/Plug-Ins/VST3`
       - **macOS AU:** `/Library/Audio/Plug-Ins/Components`
       - **Windows VST3:** `C:\Program Files\Common Files\VST3`
    2. In the DAW Plugin Manager, run a **Full Rescan** (enable "Verify Plugins" if offered).
    3. Restart the DAW fully. On macOS, after a first AU install, a **Mac restart** may be required for Audio Unit registration.
- **Q: My license key isn't activating or says "Max Activations Reached."**
  - **A:** Each license allows up to **2** computers at once. After a drive format or new machine, email us your checkout email and license key to reset activations.

## 3. Licensing & Royalty-Free Terms

- **Q: Are your drum kits and sample packs royalty-free?**
  - **A:** Yes. Use drum one-shots, MIDI, and loops in commercial beats, independent streaming (Spotify, Apple Music, YouTube), and sync without paying DiskLordz royalties.
- **Q: What happens if I land a major label placement using your melody loops or plugins?**
  - **A:** **VST plugins:** no clearance or royalty splits regardless of release size. **Melody loops** in sample packs: independent releases are royalty-free; major-label placements (or releases exceeding **1,000,000 streams**) may require master/publishing clearance per the product EULA. **Drum one-shots** do not require clearance.
- **Q: Can I share or resell the sounds or plugins?**
  - **A:** No. Single-user licenses only. Reselling, redistributing, or uploading raw WAVs or installers to shared drives or sample platforms is prohibited.

## 4. Refunds & Billing

- **Q: What is your refund policy?**
  - **A:** Digital goods are generally final once downloaded. If you have a verified technical issue we cannot fix on a supported system, or you accidentally bought the same product twice, contact us within **14 days** and we will work with you.

## 5. Agent Instructions (Base44)

- Never paste presigned download URLs or license keys into chat unless verifying identity (email + order number).
- For order status, use the store’s read-only order API when integrated (see `INTEGRATION-NOTES.md`).
- Product names and presets must match `superagent-catalog.md` only.