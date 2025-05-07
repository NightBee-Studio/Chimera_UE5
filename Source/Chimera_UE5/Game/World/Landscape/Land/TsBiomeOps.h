
#include "TsAreaMap.h"



// -------------------------------- Operators --------------------------------
struct TsBiomeOp {
	union Val {
		TsBiomeOp*		op;
		float			f;
		int				i;
		TsBiomeMatFunc*	task;

		Val(float         _v) : f   (_v) {}
		Val(int           _v) : i   (_v) {}
		Val(TsBiomeOp*    _v) : op  (_v) {}
		Val(TsBiomeMatFunc* _v) : task(_v) {}
	};
	TArray<Val>				mArgs;

	TsBiomeOp(TArray<Val> args) : mArgs(args) {}
	virtual ~TsBiomeOp() {}

	virtual bool Is(const FVector2D& p) const { return true; }	//tex-coord

	static TsMaterialValue	gMatResult;
	static bool				gResultDone;
};

#ifdef	DECLARE
TsMaterialValue	TsBiomeOp::gMatResult = TsMaterialValue();
bool			TsBiomeOp::gResultDone = false;
#endif	// DECLARE



struct TsBiomeOp_Or : public TsBiomeOp {
	TsBiomeOp_Or(TsBiomeOp* a, TsBiomeOp* b) : TsBiomeOp({ Val(a), Val(b) }) {}
	virtual ~TsBiomeOp_Or() {}
	bool Is(const FVector2D& p) const override {
		return mArgs[0].op->Is(p) || mArgs[1].op->Is(p);
	}
};

struct TsBiomeOp_And : public TsBiomeOp {
	TsBiomeOp_And(TsBiomeOp* a, TsBiomeOp* b) : TsBiomeOp({ Val(a), Val(b) }) {}
	virtual ~TsBiomeOp_And() {}
	bool Is(const FVector2D& p) const override {
		return mArgs[0].op->Is(p) && mArgs[1].op->Is(p);
	}
};

struct TsBiomeOp_Not : public TsBiomeOp {
	TsBiomeOp_Not(TsBiomeOp* a) : TsBiomeOp({ Val(a) }) {}
	virtual ~TsBiomeOp_Not() {}
	bool Is(const FVector2D& p) const override {
		return !mArgs[0].op->Is(p);
	}
};

struct TsBiomeOp_PixelGt : public TsBiomeOp {
	TsBiomeOp_PixelGt(EBiomeMapType id, float t) : TsBiomeOp({ Val(id), Val(t) }) {}
	virtual ~TsBiomeOp_PixelGt() {}
	bool Is(const FVector2D& p) const override {
		return TsBiomeMap::GetBiomeMap((EBiomeMapType)mArgs[0].i)->GetValue(p) > mArgs[1].f;
	}
};

struct TsBiomeOp_PixelLt : public TsBiomeOp {
	TsBiomeOp_PixelLt(EBiomeMapType id, float t) : TsBiomeOp({ Val(id), Val(t) }) {}
	virtual ~TsBiomeOp_PixelLt() {}
	bool Is(const FVector2D& p) const override {
		return TsBiomeMap::GetBiomeMap((EBiomeMapType)mArgs[0].i)->GetValue(p) < mArgs[1].f;
	}
};

struct TsBiomeOp_PixelRange : public TsBiomeOp {
	TsBiomeOp_PixelRange(EBiomeMapType id, float min, float max) : TsBiomeOp({ Val(id), Val(min), Val(max) }) {}
	virtual ~TsBiomeOp_PixelRange() {}
	bool Is(const FVector2D& p) const override {
		return TsBiomeMap::GetBiomeMap((EBiomeMapType)mArgs[0].i)->GetValue(p) < mArgs[1].f;
	}
};

struct TsBiomeOp_ExecTask : public TsBiomeOp {
	TsBiomeOp_ExecTask(TsBiomeMatFunc* task) : TsBiomeOp({ Val(task) }) {}
	virtual ~TsBiomeOp_ExecTask() {}
	bool Is(const FVector2D& p) const override {
		gMatResult = mArgs[0].task->GetMaterial(p);
		gResultDone = gMatResult.Num() != 0;
		return true;
	}
};
