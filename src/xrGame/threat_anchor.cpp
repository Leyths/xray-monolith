#include "pch_script.h"
#include "threat_anchor.h"
#include "Level.h"

CThreatAnchor::CThreatAnchor()
{
	m_parent_id    = u16(-1);
	m_damage_scale = 1.0f;
	m_local_offset.set(0.f, 2.0f, 0.f);  // default: lifted above the parent origin; tune via set_local_offset()
}

CThreatAnchor::~CThreatAnchor()
{
}

BOOL CThreatAnchor::net_Spawn(CSE_Abstract* DC)
{
	// CAI_Trader::net_Spawn wires up the CInventoryOwner / CharacterInfo (community comes from the section's
	// character_profile) and the skeleton visual we need for an AI line-of-sight CFORM.
	if (!inherited::net_Spawn(DC))
		return (FALSE);

	setVisible(FALSE);                          // not rendered; AI still perceives us (CFORM + flag below)
	SpatialComponent->spatial.type |= STYPE_VISIBLEFORAI;

	// Guarantee g_Alive() == true (a dead entity is dropped by the enemy filter). We never lose health --
	// Hit() forwards instead of decrementing self -- so this stays full.
	if (GetfHealth() <= 0.f)
		SetfHealth(1.0f);

	shedule.t_min = 100;
	shedule.t_max = 1000;
	return (TRUE);
}

void CThreatAnchor::shedule_Update(u32 dt)
{
	inherited::shedule_Update(dt);              // trader Think() is empty; inventory bookkeeping is harmless
	if (GetfHealth() <= 0.f)
		SetfHealth(1.0f);
}

void CThreatAnchor::UpdateCL()
{
	// Skip CAI_Trader::UpdateCL (trader animation / sound / look-at-actor); run the plain alive update,
	// then snap to the parent's centre.
	CEntityAlive::UpdateCL();

	if (m_parent_id == u16(-1))
		return;

	CObject* parent = Level().Objects.net_Find(m_parent_id);
	if (!parent)
		return;

	// Set the transform directly (ForceTransform is a no-op outside the physics hierarchy), then
	// spatial_move() re-registers in the spatial DB and refreshes the AI level vertex used by selection.
	// The offset is in the parent's local frame (rotated by its orientation) so it tracks turns; it must
	// sit clear of the parent's collision hull or AI line-of-sight to the anchor is occluded by the hull.
	Fvector world_offset;
	parent->XFORM().transform_dir(world_offset, m_local_offset);
	Fvector pos = parent->Position();
	pos.add(world_offset);
	XFORM().c.set(pos);
	spatial_move();
}

void CThreatAnchor::Hit(SHit* pHDS)
{
	// Stay invulnerable: do NOT call inherited::Hit, so our own health is never touched. Forward a scaled
	// copy to the parent -- this is how MELEE reaches it (ranged bullets already strike the parent hull).
	if (m_parent_id == u16(-1))
		return;

	CObject* parent = Level().Objects.net_Find(m_parent_id);
	CEntity* pe = smart_cast<CEntity*>(parent);
	if (!pe)
		return;

	SHit fwd = *pHDS;
	fwd.power *= m_damage_scale;
	pe->Hit(&fwd);
}
