#pragma once

#include "../../Util/TsUtility.h"

namespace TsSurfUtil {
	static inline
	bool	is_done(TArray<TsBiome*>& done_list, TsBiome* b)
	{
		for (auto done : done_list) {
			if (b == done) return true;
		}
		return false;
	}

	static inline
	float	RecurseGetHeight(TsBiome* now, const FVector2D& p, TArray<TsBiome*>& done_list, float steep)
	{
		done_list.Add(now);		// Let 'now' included into 'done_list'

		float	h = 100000;		// initial height might be overwritten...
		now->ForeachEdge(
			[&](const TsVoronoi::Edge& e) {
				float hc = e.GetDistance(p) / steep;				// height.will be the distance to the edge.

				if (e.mShared && e.mShared->mOwner) {		// check the adjecent voronois.
					TsBiome* nxt = (TsBiome*)e.mShared->mOwner;
					if (nxt->GetSType() == now->GetSType()) {
						if (!is_done(done_list, nxt)) {		// if the same biome-type continue to calc
							hc = RecurseGetHeight(nxt, p, done_list, steep);
						} else {							// cancel overwriting the result 
							hc = h;
						}
					}
				}
				h = FMath::Min(hc, h);
			});

		return h;
	}
}
