export type IntelligenceOpportunity = {
  rank: number;
  id: string;
  title: string;
  businessBenefit: string;
  buildPriority: "immediate" | "next" | "later";
};

export type IntelligenceSnapshot = {
  generatedAt: string;
  agent: {
    id: string;
    prefix: string;
    displayName: string;
    source: string;
    sourceUrl: string;
    skill: string;
    doc: string;
  };
  categories: { id: string; label: string; watchFor: string }[];
  opportunities: IntelligenceOpportunity[];
  factoryMilestone: {
    id: string;
    title: string;
    complete: boolean;
    stories: {
      id: string;
      title: string;
      passes: boolean;
      milestone: string | null;
    }[];
  };
  techRadar: {
    generated_at: string | null;
    feed_error: string | null;
    feed_url: string;
    inbox_pending: boolean;
    latest_issue: {
      title: string;
      link: string | null;
      pubDate: string | null;
    } | null;
    recent_items: { title: string; link: string | null }[];
  };
  quickPrompts: string[];
};

export const FALLBACK_INTELLIGENCE: IntelligenceSnapshot = {
  generatedAt: "1970-01-01T00:00:00.000Z",
  agent: {
    id: "radar",
    prefix: "RADAR",
    displayName: "Technology Intelligence",
    source: "AI Dev Signals",
    sourceUrl: "https://www.aidevsignals.com/",
    skill: "tech_radar_signals",
    doc: "disklordz-agent-stack/TECH-RADAR.md",
  },
  categories: [],
  opportunities: [],
  factoryMilestone: {
    id: "factory-m1",
    title: "Factory milestone M1",
    complete: false,
    stories: [],
  },
  techRadar: {
    generated_at: null,
    feed_error: null,
    feed_url: "https://rss.beehiiv.com/feeds/f5e63dd4-5653-4f09-83e2-321a8b1ba526.xml",
    inbox_pending: false,
    latest_issue: null,
    recent_items: [],
  },
  quickPrompts: ["RADAR: review latest AI Dev Signals issue"],
};

export async function fetchIntelligenceSnapshot(): Promise<IntelligenceSnapshot> {
  const res = await fetch(`/intelligence-snapshot.json?ts=${Date.now()}`);
  if (!res.ok) throw new Error(`intelligence snapshot ${res.status}`);
  return (await res.json()) as IntelligenceSnapshot;
}