#pragma once

#include <JuceHeader.h>
#include <array>
#include <cstdint>

#include "../Modules/License/DlzEd25519PublicKey.h"
#include "../Modules/License/crypto/monocypher.h"

/** Ed25519 offline license verification — message thread only (Monocypher, no extra DLLs). */
class LicenseVerifier
{
public:
    struct LicenseResult
    {
        bool isValid = false;
        juce::String errorMessage;
        juce::String serialKey;
        juce::int64 expiresAtUnix = 0;
    };

    static juce::File defaultLicenseFile()
    {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("DiskLordz")
            .getChildFile("license.json");
    }

    static juce::String getLocalMachineId()
    {
        juce::StringArray stats = juce::SystemStats::getDeviceIdentifiers();
        const juce::String raw = stats.joinIntoString("|") + "|" + juce::SystemStats::getComputerName();

        std::uint8_t hash[32] = {};
        crypto_blake2b(hash,
                       sizeof(hash),
                       reinterpret_cast<const std::uint8_t*>(raw.toRawUTF8()),
                       static_cast<std::size_t>(raw.getNumBytesAsUTF8()));

        return juce::String::toHexString(hash, static_cast<int>(sizeof(hash)), 0);
    }

    /**
     * Expected on-disk JSON from AudioStorePortal:
     * { "payload": "{\"serial_key\":\"DLZ-...\",\"machine_id\":\"...\",\"expires_at\":0}",
     *   "signature_hex": "<128 hex chars>" }
     */
    static LicenseResult verifyLicenseJson(const juce::String& licenseFileContent);

private:
    static bool decodeHex64(const juce::String& hex, std::array<std::uint8_t, 64>& out);
};