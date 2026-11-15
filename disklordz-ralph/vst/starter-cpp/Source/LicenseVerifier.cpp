#include "LicenseVerifier.h"

bool LicenseVerifier::decodeHex64(const juce::String& hex, std::array<std::uint8_t, 64>& out)
{
    const auto trimmed = hex.trim();
    if (trimmed.length() != 128)
        return false;

    for (int i = 0; i < 64; ++i)
    {
        const auto pair = trimmed.substring(i * 2, i * 2 + 2);
        out[static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(pair.getHexValue32());
    }
    return true;
}

LicenseVerifier::LicenseResult LicenseVerifier::verifyLicenseJson(const juce::String& licenseFileContent)
{
    LicenseResult result;

    const auto parsed = juce::JSON::parse(licenseFileContent);
    if (!parsed.isObject())
    {
        result.errorMessage = "Corrupted license file format.";
        return result;
    }

    const juce::String payloadStr = parsed.getProperty("payload", "").toString();
    const juce::String sigHex = parsed.getProperty("signature_hex", "").toString();

    if (payloadStr.isEmpty() || sigHex.length() != 128)
    {
        result.errorMessage = "Missing payload or invalid Ed25519 signature length.";
        return result;
    }

    std::array<std::uint8_t, 64> signature{};
    if (!decodeHex64(sigHex, signature))
    {
        result.errorMessage = "Malformed signature hex.";
        return result;
    }

    const auto& publicKey = dlz::license::kEd25519PublicKey;
    const auto* message = reinterpret_cast<const std::uint8_t*>(payloadStr.toRawUTF8());
    const std::size_t messageLen = static_cast<std::size_t>(payloadStr.getNumBytesAsUTF8());

    if (crypto_eddsa_check(signature.data(), publicKey.data(), message, messageLen) != 0)
    {
        result.errorMessage = "Cryptographic signature verification failed.";
        return result;
    }

    const auto claims = juce::JSON::parse(payloadStr);
    if (!claims.isObject())
    {
        result.errorMessage = "Invalid payload JSON.";
        return result;
    }

    const juce::String licensedMachine = claims.getProperty("machine_id", "").toString();
    const juce::int64 expiresAt = static_cast<juce::int64>(claims.getProperty("expires_at", 0));
    const juce::String serial = claims.getProperty("serial_key", "").toString();

    if (licensedMachine != getLocalMachineId())
    {
        result.errorMessage = "License belongs to a different machine ID.";
        return result;
    }

    const juce::int64 nowSec = juce::Time::getCurrentTime().toMilliseconds() / 1000;
    if (expiresAt > 0 && nowSec > expiresAt)
    {
        result.errorMessage = "Subscription or trial license has expired.";
        return result;
    }

    result.isValid = true;
    result.serialKey = serial;
    result.expiresAtUnix = expiresAt;
    return result;
}