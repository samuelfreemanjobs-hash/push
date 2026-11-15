#include "DlzOfflineLicense.h"

namespace dlz::license
{

namespace
{

juce::String fieldString(const juce::var& obj, const char* key)
{
    if (!obj.isObject())
        return {};
    const auto* o = obj.getDynamicObject();
    if (o == nullptr)
        return {};
    return o->getProperty(key).toString();
}

std::int64_t fieldInt64(const juce::var& obj, const char* key)
{
    if (!obj.isObject())
        return 0;
    const auto* o = obj.getDynamicObject();
    if (o == nullptr)
        return 0;
    return static_cast<std::int64_t>(static_cast<juce::int64>(o->getProperty(key)));
}

juce::String canonicalSignPayload(const SignedLicensePayload& p)
{
    // Must match AudioStorePortal backend canonical JSON signing order.
    return "serial_key=" + p.serialKey + "\n"
           + "machine_id=" + p.machineId + "\n"
           + "issued_at=" + juce::String(p.issuedAtUnix) + "\n"
           + "expires_at=" + juce::String(p.expiresAtUnix);
}

} // namespace

std::optional<SignedLicensePayload> parseActivateResponse(const juce::String& jsonUtf8)
{
    const auto parsed = juce::JSON::parse(jsonUtf8);
    if (parsed.isVoid())
        return std::nullopt;

    SignedLicensePayload out;
    out.serialKey = fieldString(parsed, "serial_key");
    if (out.serialKey.isEmpty())
        out.serialKey = fieldString(parsed, "serialKey");
    out.machineId = fieldString(parsed, "machine_id");
    if (out.machineId.isEmpty())
        out.machineId = fieldString(parsed, "machineId");
    out.issuedAtUnix = fieldInt64(parsed, "issued_at");
    if (out.issuedAtUnix == 0)
        out.issuedAtUnix = fieldInt64(parsed, "issuedAt");
    out.expiresAtUnix = fieldInt64(parsed, "expires_at");
    if (out.expiresAtUnix == 0)
        out.expiresAtUnix = fieldInt64(parsed, "expiresAt");
    out.signatureBase64 = fieldString(parsed, "signature");
    if (out.signatureBase64.isEmpty())
        out.signatureBase64 = fieldString(parsed, "signature_base64");

    if (out.serialKey.isEmpty() || out.signatureBase64.isEmpty())
        return std::nullopt;

    return out;
}

bool verifyEd25519Offline(const SignedLicensePayload& payload, const std::array<std::uint8_t, 32>& publicKey32)
{
    juce::ignoreUnused(publicKey32);

#if defined(DLZ_LICENSE_ED25519) && DLZ_LICENSE_ED25519
    const auto message = canonicalSignPayload(payload);
    const auto sigBytes = juce::Base64::convertFromBase64(payload.signatureBase64);
    if (sigBytes.getSize() != 64)
        return false;

    // Link libsodium and replace this block — see juce-ed25519-offline-license.md
    juce::ignoreUnused(message);
    return false;
#else
    juce::ignoreUnused(payload);
    return false;
#endif
}

bool isLicenseCurrentlyValid(const SignedLicensePayload& payload, const juce::String& localMachineId)
{
    if (payload.machineId != localMachineId)
        return false;

    const auto now = juce::Time::getCurrentTime().toMilliseconds() / 1000;
    if (payload.expiresAtUnix > 0 && now > payload.expiresAtUnix)
        return false;

    return true;
}

} // namespace dlz::license