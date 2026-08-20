// YummyLife: the editor links the game's shared banks (objectBank, animationBank)
// and minorGems' gameGraphicsGL, and those reach into a handful of HetuwMod
// statics.  Pulling in hetuwmod.cpp to get them would drag the whole client
// along, so the editor defines just the members it actually references.
//
// Every one of those use sites is guarded, so these values give the editor
// stock OHOL behaviour:
//
//   objectBank.cpp      if (HetuwMod::objectDrawScale && inScale == 1.0)
//   animationBank.cpp   if (HetuwMod::objectDrawScale)
//                       if (HetuwMod::filterSprites)
//   gameGraphicsGL.cpp  if (HetuwMod::drawColorAlpha != 1.0f)
//
// If a shared file starts using another HetuwMod member, the editor fails to
// link naming that symbol; add it here.

#include "hetuwmod.h"

double *HetuwMod::objectDrawScale = NULL;
bool HetuwMod::filterSprites = false;
vector<int> HetuwMod::filteredSprites;
float HetuwMod::drawColorAlpha = 1.0f;


// Not a HetuwMod member: this is one of the callbacks every program built on
// the minorGems game layer has to provide (see minorGems/game/game.h). game.cpp
// answers it for the client, from a key the player can rebind. editor.cpp is
// upstream code that predates the setting, so answer it here with the '%' that
// gameSDL.cpp used before the key was made configurable.
char hetuwGetConfirmExitKey() {
	return '%';
}


// Same story: the editor has no HetuwMod overlays, so escape is never ours.
char yumEscapeConsumed() {
	return false;
}
