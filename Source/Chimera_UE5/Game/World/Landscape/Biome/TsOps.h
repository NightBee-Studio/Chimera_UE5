
#include "TsBiomeMap.h"



// -------------------------------- Operators --------------------------------
struct TsOp {
	union Val {
		TsOp*		op;
		float			f;
		int				i;
		TsBiomeMatFunc*	task;

		Val(float         _v) : f   (_v) {}
		Val(int           _v) : i   (_v) {}
		Val(TsOp*    _v) : op  (_v) {}
		Val(TsBiomeMatFunc* _v) : task(_v) {}
	};
	TArray<Val>				mArgs;

	TsOp(TArray<Val> args) : mArgs(args) {}
	virtual ~TsOp() {}

	virtual bool Is(const FVector2D& p) const { return true; }	//tex-coord

	static TsMaterialValue	gMatResult;
	static bool				gResultDone;
};

#ifdef	DECLARE
TsMaterialValue	TsOp::gMatResult = TsMaterialValue();
bool			TsOp::gResultDone = false;
#endif	// DECLARE



struct TsOp_Or : public TsOp {
	TsOp_Or(TsOp* a, TsOp* b) : TsOp({ Val(a), Val(b) }) {}
	virtual ~TsOp_Or() {}
	bool Is(const FVector2D& p) const override {
		return mArgs[0].op->Is(p) || mArgs[1].op->Is(p);
	}
};

struct TsOp_And : public TsOp {
	TsOp_And(TsOp* a, TsOp* b) : TsOp({ Val(a), Val(b) }) {}
	virtual ~TsOp_And() {}
	bool Is(const FVector2D& p) const override {
		return mArgs[0].op->Is(p) && mArgs[1].op->Is(p);
	}
};

struct TsOp_Not : public TsOp {
	TsOp_Not(TsOp* a) : TsOp({ Val(a) }) {}
	virtual ~TsOp_Not() {}
	bool Is(const FVector2D& p) const override {
		return !mArgs[0].op->Is(p);
	}
};

struct TsOp_PixelGt : public TsOp {
	TsOp_PixelGt(EBiomeMapType id, float t) : TsOp({ Val(id), Val(t) }) {}
	virtual ~TsOp_PixelGt() {}
	bool Is(const FVector2D& p) const override {
		return TsBiomeMap::GetBiomeMap((EBiomeMapType)mArgs[0].i)->GetValue(p) > mArgs[1].f;
	}
};

struct TsOp_PixelLt : public TsOp {
	TsOp_PixelLt(EBiomeMapType id, float t) : TsOp({ Val(id), Val(t) }) {}
	virtual ~TsOp_PixelLt() {}
	bool Is(const FVector2D& p) const override {
		return TsBiomeMap::GetBiomeMap((EBiomeMapType)mArgs[0].i)->GetValue(p) < mArgs[1].f;
	}
};

struct TsOp_PixelRange : public TsOp {
	TsOp_PixelRange(EBiomeMapType id, float min, float max) : TsOp({ Val(id), Val(min), Val(max) }) {}
	virtual ~TsOp_PixelRange() {}
	bool Is(const FVector2D& p) const override {
		return TsBiomeMap::GetBiomeMap((EBiomeMapType)mArgs[0].i)->GetValue(p) < mArgs[1].f;
	}
};

struct TsOp_ExecTask : public TsOp {
	TsOp_ExecTask(TsBiomeMatFunc* task) : TsOp({ Val(task) }) {}
	virtual ~TsOp_ExecTask() {}
	bool Is(const FVector2D& p) const override {
		gMatResult = mArgs[0].task->GetMaterial(p);
		gResultDone = gMatResult.Num() != 0;
		return true;
	}
};
