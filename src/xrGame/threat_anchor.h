#pragma once

#include "ai/trader/ai_trader.h"

// CThreatAnchor -- a generic, AI-inert "combat target" bound to a parent object.
//
// It derives CAI_Trader purely to inherit a correct, lightweight CInventoryOwner (so it carries a real
// character community -- e.g. army -- and stalkers resolve hostility to it through the faction/goodwill
// system, exactly like any NPC). It is NOT used as a trader: it is invisible, never fights, never trades.
// Each frame it snaps to a point at a parent-local offset (so several anchors can ring a large parent and
// stay exposed to AI vision from every side); Hits it receives are forwarded (scaled) to the parent while
// it stays invulnerable.
//
// The community/profile come entirely from the spawn section (a soldier character profile), so this class
// is generic -- it hard-codes nothing game-specific.
class CThreatAnchor : public CAI_Trader
{
	typedef CAI_Trader inherited;

private:
	u16     m_parent_id;     // object we glue to and forward damage to (u16(-1) = unbound)
	float   m_damage_scale;  // forwarded Hit power is multiplied by this
	Fvector m_local_offset;  // offset from the parent origin, in the parent's local frame (rotates with it).
	                         // Push it clear of the parent's collision hull or AI vision is occluded by the
	                         // hull itself (a point inside the hull is never seen).

public:
	CThreatAnchor();
	virtual ~CThreatAnchor();

	virtual BOOL net_Spawn(CSE_Abstract* DC);
	virtual void UpdateCL();
	virtual void shedule_Update(u32 dt);
	virtual BOOL net_SaveRelevant() { return FALSE; }  // transient: never serialized

	virtual void Hit(SHit* pHDS);                       // forward to parent; stay invulnerable

	// inert: not a HUD/zone target, no scripted thinking of its own. UsedAI_Locations stays at the trader
	// default (TRUE) so a valid level vertex is maintained -- enemy selection rejects targets without one.
	virtual BOOL IsVisibleForHUD()   { return FALSE; }
	virtual bool IsVisibleForZones() { return false; }

	// script-facing control (driven by the script_game_object binding)
	void  bind_parent(u16 parent_id)         { m_parent_id = parent_id; }
	void  set_damage_scale(float k)          { m_damage_scale = k; }
	void  set_local_offset(const Fvector& o) { m_local_offset = o; }
	u16   parent_id() const                  { return m_parent_id; }
};
