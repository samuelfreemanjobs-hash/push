#include "RenderQueue.h"
#include "RecipeCard.h"

namespace forge
{
juce::String QueueItem::describe() const
{
    const auto& r = request;
    juce::String key = r.plan.fixedRoot >= 0 ? "16 Levels " + midiNoteName (r.plan.fixedRoot)
                     : r.plan.fixedRoot == PlanOptions::styleRoot ? juce::String ("16 Levels")
                     : r.plan.key == "random" ? juce::String ("random keys") : r.plan.key + " " + (r.plan.scale.isNotEmpty() ? r.plan.scale : r.style.scale);
    juce::String text = r.style.name + "  |  " + key + "  |  " + juce::String (r.plan.count) + " samples";
    if (r.plan.useCard)
    {
        auto cards = parseRecipeCards (r.plan.cardText, r.plan.cardMpcNotation);
        text = "Recipe card: " + (juce::isPositiveAndBelow (r.plan.cardIndex, (int) cards.size()) ? cards[(size_t) r.plan.cardIndex].name : juce::String ("?"));
    }
    if (r.plan.usePalette) text = r.style.name + "  |  your chord palette (" + juce::String ((int) r.plan.palette.size()) + " picks)";
    if (r.plan.octaveLow >= 0 && ! r.plan.usePalette)
        text << "  |  Oct " << r.plan.octaveLow << (r.plan.octaveHigh > r.plan.octaveLow ? "-" + juce::String (r.plan.octaveHigh) : juce::String());
    if (! r.plan.usePalette && (r.plan.scaleBank || r.style.scaleBankOnly)) text << " (banks" << (r.plan.bankType.isNotEmpty() ? ": " + r.plan.bankType : juce::String()) << ")";
    if (r.style.progressionLibrary.isNotEmpty() && r.plan.progressionNumber > 0) text << " #" << r.plan.progressionNumber;
    text << "  |  " << (pluginPath.isEmpty() ? "Built-in" + (r.plan.engine.isNotEmpty() ? " (" + r.plan.engine + ")" : juce::String())
                                             : r.instrumentName);
    if (smartPick) text << "  |  Smart Pick";
    if (r.roundRobins > 1) text << "  |  " << r.roundRobins << " takes";
    if (! r.registers.empty()) text << "  |  + registers";
    return text;
}

juce::var queueItemToVar (const QueueItem& item)
{
    const auto& r = item.request;
    auto* o = new juce::DynamicObject();
    o->setProperty ("status", item.status);
    o->setProperty ("result", item.result);
    o->setProperty ("doneFolder", item.doneFolder.getFullPathName());
    o->setProperty ("pluginPath", item.pluginPath);
    o->setProperty ("pluginState", item.pluginState.toBase64Encoding());
    o->setProperty ("instances", item.instances);
    o->setProperty ("smartPick", item.smartPick);
    o->setProperty ("instrumentName", r.instrumentName);
    o->setProperty ("outputFolder", r.outputFolder.getFullPathName());
    o->setProperty ("style", r.style.toVar());

    auto* p = new juce::DynamicObject();
    p->setProperty ("count", r.plan.count);             p->setProperty ("key", r.plan.key);
    p->setProperty ("scale", r.plan.scale);             p->setProperty ("seed", juce::String (r.plan.seed));
    p->setProperty ("prefix", r.plan.prefix);           p->setProperty ("fixedRoot", r.plan.fixedRoot);
    p->setProperty ("engine", r.plan.engine);           p->setProperty ("builtIn", r.plan.builtIn);
    p->setProperty ("scaleBank", r.plan.scaleBank);     p->setProperty ("bankType", r.plan.bankType);
    p->setProperty ("shortNames", r.plan.shortNames);   p->setProperty ("progressionNumber", r.plan.progressionNumber);
    p->setProperty ("octaveLow", r.plan.octaveLow);     p->setProperty ("octaveHigh", r.plan.octaveHigh);
    p->setProperty ("usePalette", r.plan.usePalette);
    p->setProperty ("useCard", r.plan.useCard);
    p->setProperty ("cardText", r.plan.cardText);
    p->setProperty ("cardIndex", r.plan.cardIndex);
    p->setProperty ("cardMpcNotation", r.plan.cardMpcNotation);
    p->setProperty ("cardUseRecipeFx", r.plan.cardUseRecipeFx);
    juce::Array<juce::var> pal;
    for (auto& c : r.plan.palette) pal.add (c.toString());
    p->setProperty ("palette", pal);
    o->setProperty ("plan", juce::var (p));

    auto* s = new juce::DynamicObject();
    s->setProperty ("sampleRate", r.render.sampleRate); s->setProperty ("bitDepth", r.render.bitDepth);
    s->setProperty ("normalise", r.render.normalise);   s->setProperty ("loudnessMatch", r.render.loudnessMatch);
    s->setProperty ("trimToSec", r.render.trimToSec);   s->setProperty ("maxTailSec", r.render.maxTailSec);
    s->setProperty ("warmUpSec", r.render.warmUpSec);
    o->setProperty ("render", juce::var (s));

    juce::Array<juce::var> regs;
    for (auto x : r.registers) regs.add (x);
    o->setProperty ("registers", regs);
    o->setProperty ("variety", r.variety);
    o->setProperty ("splitBanks", r.splitBanks);
    o->setProperty ("roundRobins", r.roundRobins);
    return juce::var (o);
}

bool queueItemFromVar (const juce::var& v, QueueItem& item, juce::String& error)
{
    if (! v.isObject()) { error = "not a queue item"; return false; }
    item = QueueItem();
    auto& r = item.request;
    if (! Style::fromVar (v["style"], r.style, error)) return false;

    item.status = v.getProperty ("status", "waiting").toString();
    item.result = v["result"].toString();
    item.doneFolder = juce::File (v["doneFolder"].toString());
    item.pluginPath = v["pluginPath"].toString();
    item.pluginState.fromBase64Encoding (v["pluginState"].toString());
    item.instances = juce::jlimit (1, 4, (int) v.getProperty ("instances", 1));
    item.smartPick = (bool) v["smartPick"];
    r.instrumentName = v["instrumentName"].toString();
    r.outputFolder = juce::File (v["outputFolder"].toString());

    auto p = v["plan"];
    r.plan.count = juce::jlimit (1, 2000, (int) p.getProperty ("count", 100));
    r.plan.key = p.getProperty ("key", "random").toString();
    r.plan.scale = p["scale"].toString();
    r.plan.seed = p["seed"].toString().getLargeIntValue();
    r.plan.prefix = p["prefix"].toString();
    r.plan.fixedRoot = (int) p.getProperty ("fixedRoot", -1);
    r.plan.engine = p["engine"].toString();
    r.plan.builtIn = (bool) p["builtIn"];
    r.plan.scaleBank = (bool) p["scaleBank"];
    r.plan.bankType = p["bankType"].toString();
    r.plan.shortNames = (bool) p["shortNames"];
    r.plan.progressionNumber = (int) p.getProperty ("progressionNumber", 0);
    r.plan.octaveLow = (int) p.getProperty ("octaveLow", -1);
    r.plan.octaveHigh = (int) p.getProperty ("octaveHigh", -1);
    r.plan.usePalette = (bool) p["usePalette"];
    r.plan.useCard = (bool) p["useCard"];
    r.plan.cardText = p["cardText"].toString();
    r.plan.cardIndex = (int) p.getProperty ("cardIndex", 0);
    r.plan.cardMpcNotation = (bool) p["cardMpcNotation"];
    r.plan.cardUseRecipeFx = (bool) p["cardUseRecipeFx"];
    if (auto* pal = p["palette"].getArray())
        for (auto& x : *pal) { PaletteChord c; if (PaletteChord::fromString (x.toString(), c)) r.plan.palette.push_back (c); }

    auto s = v["render"];
    r.render.sampleRate = (double) s.getProperty ("sampleRate", 44100.0);
    r.render.bitDepth = (int) s.getProperty ("bitDepth", 24);
    r.render.normalise = (bool) s.getProperty ("normalise", true);
    r.render.loudnessMatch = (bool) s.getProperty ("loudnessMatch", false);
    r.render.trimToSec = (double) s.getProperty ("trimToSec", -1.0);
    r.render.maxTailSec = (double) s.getProperty ("maxTailSec", 3.0);
    r.render.warmUpSec = (double) s.getProperty ("warmUpSec", 1.5);

    if (auto* regs = v["registers"].getArray())
        for (auto& x : *regs) r.registers.push_back ((int) x);
    r.variety = (int) v.getProperty ("variety", 1);
    r.splitBanks = (bool) v["splitBanks"];
    r.roundRobins = juce::jlimit (1, 4, (int) v.getProperty ("roundRobins", 1));
    if (item.status == "running") item.status = "waiting"; // the app closed while it ran
    return true;
}

juce::File defaultQueueFile()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("MPCSampleForge").getChildFile ("queue.json");
}

bool saveQueue (const juce::File& f, const std::vector<QueueItem>& items)
{
    juce::Array<juce::var> list;
    for (auto& i : items) list.add (queueItemToVar (i));
    f.getParentDirectory().createDirectory();
    return f.replaceWithText (juce::JSON::toString (juce::var (list), false));
}

std::vector<QueueItem> loadQueue (const juce::File& f, juce::String& error)
{
    std::vector<QueueItem> items;
    if (! f.existsAsFile()) return items;
    auto v = juce::JSON::parse (f);
    if (! v.isArray()) { error = f.getFileName() + " is not a queue file"; return items; }
    for (auto& entry : *v.getArray())
    {
        QueueItem item;
        juce::String e;
        if (queueItemFromVar (entry, item, e)) items.push_back (std::move (item));
        else error = e;
    }
    return items;
}
}
