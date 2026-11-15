#pragma once

#include <JuceHeader.h>
#include <array>
#include <cstdint>
#include <optional>

namespace dlz::license
{

/** Mirrors POST /api/license/activate JSON + base64 Ed25519 signature (AudioStorePortal). */
struct SignedLicensePayload
{
    juce::String serialKey;
    juce::String machineId;
    std::int64_t issuedAtUnix = 0;
    std::int64_t expiresAtUnix = 0;
    juce::String signatureBase64;
};

/** Parse server JSON (UTF-8). Returns nullopt on malformed input — no exceptions across RT boundary callers. */
std::optional<SignedLicensePayload> parseActivateResponse(const juce::String& jsonUtf8);

/**
 * Verify Ed25519 signature over canonical payload bytes.
 * @param publicKey32 32-byte Ed25519 public key (from build-time DLZ_ED25519_PUBLIC_KEY_HEX).
 * Implementations: see docs/dev/juce-ed25519-offline-license.md (libsodium).
 * When DLZ_LICENSE_ED25519=0, always returns false (Lite / unsigned builds).
 */
bool verifyEd25519Offline(const SignedLicensePayload& payload, const std::array<std::uint8_t, 32>& publicKey32);

/** Non-RT: check expiry + machineId matches local fingerprint string from host. */
bool isLicenseCurrentlyValid(const SignedLicensePayload& payload, const juce::String& localMachineId);

} // namespace dlz::license